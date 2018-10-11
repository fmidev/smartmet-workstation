#include "NFmiControlPointObservationBlender.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiAreaMaskList.h"
#include "NFmiDrawParam.h"
#include "NFmiAnalyzeToolData.h"

NFmiControlPointObservationBlender::BlendingDataHelper::BlendingDataHelper()
:changeField()
,limitChecker(kFloatMissing, kFloatMissing, kFmiBadParameter)
{

}


NFmiControlPointObservationBlender::NFmiControlPointObservationBlender(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
    unsigned long theAreaMask, boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, const NFmiRect &theCPGridCropRect,
    bool theUseGridCrop, const NFmiPoint &theCropMarginSize, checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, const NFmiMetTime &actualFirstTime)
    :NFmiDataParamControlPointModifier(theInfo, theDrawParam, theMaskList,
        theAreaMask, theCPManager, theCPGridCropRect,
        theUseGridCrop, theCropMarginSize)
    , itsObservationInfos(observationInfos)
    , itsActualFirstTime(actualFirstTime)
{
}

static NFmiTimeDescriptor CalcAllowedTimes(const NFmiMetTime &actualFirstTime, long expirationTimeInMinutes)
{
    auto allowedRangeStartTime = actualFirstTime;
    allowedRangeStartTime.SetTimeStep(1, false);
    allowedRangeStartTime.ChangeByMinutes(-expirationTimeInMinutes);
    return NFmiTimeDescriptor(actualFirstTime, NFmiTimeBag(allowedRangeStartTime, actualFirstTime, expirationTimeInMinutes));
}

bool NFmiControlPointObservationBlender::ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks)
{
    // 1. Mikä on sallittu aikahaarukka? esim. 10.00 - 10.20 Utc
    auto allowedTimes = ::CalcAllowedTimes(itsActualFirstTime, NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes());
    // 3. Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisessä CP-pisteessä on 0 (merkitään missing arvolla).
    std::vector<float> xValues, yValues, zValues;
    if(GetObservationsToChangeValueFields(xValues, yValues, zValues, allowedTimes))
    {
        // Laitetaan editoitu data osoittamaan 1. muokattavaan aikaan
        if(!itsInfo->Time(theActiveTimes.FirstTime()))
            itsInfo->FirstTime();
        // 4. Täydennä CP-pisteiden arvoja seuraavasti, jos CP-pisteessä on puuttuva, otetaan editoidusta datasta 0-hetkeltä arvo siihen (0-muutos)
        FillZeroChangeValuesForMissingCpPoints(zValues);
        // 5. Laske CP-pisteiden avulla 0-hetken 'analyysikenttä'
        DoBlendingDataGridding(xValues, yValues, zValues);
        // 6. Laske analyysikentän ja editoidun datan 0-hetken kentän avulla muutoskenttä
        itsBlendingDataHelper.changeField = CalcChangeField(GetUsedGridData());
        // 7. Blendaa muutoskenttä editoituun dataan liu'uttamalla
        return MakeBlendingOperation(theActiveTimes);
        // 8. Lasketaanko 'analyysit' ja niiden sijoitus editoituun dataan myös 0-hetkeä edeltäville editoidun datan ajoille?
    }
    return false;
}

bool NFmiControlPointObservationBlender::DoBlendingDataGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues)
{
    // Mikä oli käytetty griddaus functio?
    FmiGriddingFunction griddingFunction = itsCPManager->CPGriddingProperties().Function();
    if(griddingFunction != kFmiErrorGriddingFunction)
    {
        NFmiDataParamControlPointModifier::DoDataGridding(xValues, yValues, zValues, static_cast<int>(xValues.size()), GetUsedGridData(), itsGridCropRelativeRect, griddingFunction, itsObsDataGridding, kFloatMissing);

        return true;
    }
    return false;
}

// Blendataan annettu muutoskenttä editoituun parametriin annetun blendingTimes:in aikarajoissa.
bool NFmiControlPointObservationBlender::MakeBlendingOperation(NFmiTimeDescriptor &blendingTimes)
{
    auto status = false;
    itsBlendingDataHelper.limitChecker = NFmiLimitChecker(static_cast<float>(itsDrawParam->AbsoluteMinValue()), static_cast<float>(itsDrawParam->AbsoluteMaxValue()), static_cast<FmiParameterName>(itsInfo->Param().GetParamIdent()));
    for(itsInfo->ResetLevel(); itsInfo->NextLevel(); ) // Tuskin ikinä päästään 3D editointiin, mutta varaudutaan silti siihen
    {
        itsBlendingDataHelper.blendingTimeSize = blendingTimes.Size();
        for(blendingTimes.Reset(); blendingTimes.Next(); )
        {
            itsBlendingDataHelper.blendingTimeIndex = blendingTimes.Index();
            // Blendaus menee niin että alkuhetkellä muutos kenttä otetaan kertoimella 1. ja viimeisellä se otetaan kertoimella 0. (eli viimeinen aika-askel voidaan skipata)
            if(itsBlendingDataHelper.blendingTimeIndex + 1 >= itsBlendingDataHelper.blendingTimeSize)
                break;
            if(itsInfo->Time(blendingTimes.Time())) // Ajan pitäisi aina löytyä, koska blendingtimes on rakennettu editoidun datan aikojen perusteella
            {
                itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
                DoLocationGridCalculations(itsBlendingDataHelper.changeField);
                status = true;
            }
        }
    }
    return status;
}

void NFmiControlPointObservationBlender::DoCroppedPointCalculations(const NFmiDataMatrix<float> &usedData, size_t xIndex, size_t yIndex, float maskFactor)
{
    auto editedValue = itsInfo->FloatValue();
    auto changeValue = usedData[xIndex][yIndex];
    float value = NFmiControlPointObservationBlendingData::BlendData(editedValue, changeValue, maskFactor, itsBlendingDataHelper.blendingTimeSize, itsBlendingDataHelper.blendingTimeIndex, itsBlendingDataHelper.limitChecker);
    itsInfo->FloatValue(value);
}

void NFmiControlPointObservationBlender::DoNormalPointCalculations(const NFmiDataMatrix<float> &usedData, unsigned long locationIndex, float maskFactor)
{
    auto editedValue = itsInfo->FloatValue();
    auto changeValue = usedData.GetValue(itsInfo->LocationIndex(), kFloatMissing);
    float value = NFmiControlPointObservationBlendingData::BlendData(editedValue, changeValue, maskFactor, itsBlendingDataHelper.blendingTimeSize, itsBlendingDataHelper.blendingTimeIndex, itsBlendingDataHelper.limitChecker);
    itsInfo->FloatValue(value);
}

NFmiDataMatrix<float> NFmiControlPointObservationBlender::CalcChangeField(const NFmiDataMatrix<float> &analysisField)
{
    NFmiDataMatrix<float> changeField;
    if(fUseGridCrop)
        itsInfo->CroppedValues(changeField, boost::math::iround(itsCPGridCropRect.Left()), boost::math::iround(itsCPGridCropRect.Top()), boost::math::iround(itsCPGridCropRect.Right()), boost::math::iround(itsCPGridCropRect.Bottom()));
    else
        itsInfo->Values(changeField); // otetaan originaali kenttä changeField:iin
    changeField -= analysisField; // matriisi laskuoperaatiot ottavat huomioon puuttuvat arvot

    // Zoomed crop tapauksessa pitää muutos liu'uttaa nolliin kohti zoomatubn alueen reunoja
    if(fUseGridCrop)
        NFmiDataParamControlPointModifier::FixCroppedMatrixMargins(changeField, itsCropMarginSize);

    return changeField;
}

const double g_missingDistanceToStationInKm = 9999999999.;

void NFmiControlPointObservationBlender::FillZeroChangeValuesForMissingCpPoints(std::vector<float> &zValues)
{
    const auto &cpLatlonPoints = itsCPManager->CPLocationVector();
    for(size_t cpLocationIndex = 0; cpLocationIndex < zValues.size(); cpLocationIndex++)
    {
        if(zValues[cpLocationIndex] == kFloatMissing)
        {
            zValues[cpLocationIndex] = itsInfo->InterpolatedValue(cpLatlonPoints[cpLocationIndex]);
        }
    }
}

static bool NeedToLookForObservationValuesForCpPoint(const std::vector<float> &cpValues, const std::vector<double> &cpDistanceToStationInKm, size_t cpLocationIndex)
{
    // Jos tietyssä cp-pisteessä on jo etäisyys minimi eli 0 km ja cp-arvo ei ole puuttuvaa, 
    // tällöin ei enää tarvitse etsiä parempia asemia tai arvoja.
    if(cpDistanceToStationInKm[cpLocationIndex] == 0 && cpValues[cpLocationIndex] != kFloatMissing)
        return false;
    else
        return true;
}

// Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisessä CP-pisteessä on 0 (merkitään missing arvolla).
bool NFmiControlPointObservationBlender::GetObservationsToChangeValueFields(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, const NFmiTimeDescriptor &allowedTimeRange)
{
    // 1. Haetaan käytetystä CP-managerista pohjat, eli x- ja y-koordinaatit ja jotkut pohja-arvot z eli muutoskentälle.
    if(GetChangeValues(xValues, yValues, zValues))
    {
        // 1.1. Laita z-kenttään puuttuvaa arvoa
        zValues = std::vector<float>(zValues.size(), kFloatMissing);
        // 1.2. Vektori, jossa minimi etäisyydet CP-pisteiden ja löydettyjen asemien välillä
        std::vector<double> cpDistanceToStationInKm(zValues.size(), g_missingDistanceToStationInKm);
        // 1.3. Varsinaiset CP-pisteiden lokaatiot, joiden avulla etäisyyksiä lasketaan
        const auto &cpLatlonPoints = itsCPManager->CPLocationVector();
        // 2. Käy kaikki datat läpi
        for(auto &info : itsObservationInfos)
        {
            // 3. Käy läpi datan sallitut ajat uusimmasta taaksepäin
            if(info->FindNearestTime(allowedTimeRange.LastTime(), kBackward))
            {
                for( ; allowedTimeRange.IsInside(info->Time()); )
                {
                    // 4. Käy läpi CP-pisteet ja etsi jokaiseen lähin asema. 
                    for(size_t cpLocationIndex = 0; cpLocationIndex < cpLatlonPoints.size(); cpLocationIndex++)
                    {
                        if(::NeedToLookForObservationValuesForCpPoint(zValues, cpDistanceToStationInKm, cpLocationIndex))
                        {
                            NFmiLocation cpLocation(cpLatlonPoints[cpLocationIndex]);
                            if(info->NearestLocation(cpLocation, NFmiControlPointObservationBlendingData::MaxAllowedDistanceToStationInKm() * 1000))
                            {
                                auto distanceInKm = cpLocation.Distance(info->LatLonFast());
                                // Jos on löytynyt uusi lähin asema, pitää ensin muutos arvo nollata mahdollisesta edellisen datan lähipisteen arvosta
                                if(distanceInKm < cpDistanceToStationInKm[cpLocationIndex])
                                {
                                    zValues[cpLocationIndex] = kFloatMissing;
                                    // 5. Laita aseman etäisyys pisteeseen joka CP-pisteeseen talteen, jotta voidaan etsiä toisista datoista lähempiä asemia
                                    cpDistanceToStationInKm[cpLocationIndex] = distanceInKm;
                                }
                                // Tässä etsitään myöhäisimmän sallitun ajan ei-puuttuvaa arvoa
                                if(distanceInKm == cpDistanceToStationInKm[cpLocationIndex])
                                {
                                    // 6. Miten CP-pisteiden arvoja rankataan tärkeysjärjestykseen?
                                    // 6.1. Ensin ratkaisee että on lähin asema (vaikka datalle ei löytyisi sopivia aikoja)
                                    // 6.2. Sitten ratkaiseen viimeisimmän sallitun ajan ei-puuttuva arvo
                                    if(zValues[cpLocationIndex] == kFloatMissing)
                                    {
                                        zValues[cpLocationIndex] = info->FloatValue();
                                    }
                                }
                            }
                        }
                    }
                    if(!info->PreviousTime())
                        break; // Jos edellistä aikaa ei ole datassa, voidaan myös lopettaa
                }
            }
        }
        return true;
    }

    return false;
}
