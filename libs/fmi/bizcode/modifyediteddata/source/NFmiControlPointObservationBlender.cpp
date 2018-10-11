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
    // 1. Mik� on sallittu aikahaarukka? esim. 10.00 - 10.20 Utc
    auto allowedTimes = ::CalcAllowedTimes(itsActualFirstTime, NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes());
    // 3. Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisess� CP-pisteess� on 0 (merkit��n missing arvolla).
    std::vector<float> xValues, yValues, zValues;
    if(GetObservationsToChangeValueFields(xValues, yValues, zValues, allowedTimes))
    {
        // Laitetaan editoitu data osoittamaan 1. muokattavaan aikaan
        if(!itsInfo->Time(theActiveTimes.FirstTime()))
            itsInfo->FirstTime();
        // 4. T�ydenn� CP-pisteiden arvoja seuraavasti, jos CP-pisteess� on puuttuva, otetaan editoidusta datasta 0-hetkelt� arvo siihen (0-muutos)
        FillZeroChangeValuesForMissingCpPoints(zValues);
        // 5. Laske CP-pisteiden avulla 0-hetken 'analyysikentt�'
        DoBlendingDataGridding(xValues, yValues, zValues);
        // 6. Laske analyysikent�n ja editoidun datan 0-hetken kent�n avulla muutoskentt�
        itsBlendingDataHelper.changeField = CalcChangeField(GetUsedGridData());
        // 7. Blendaa muutoskentt� editoituun dataan liu'uttamalla
        return MakeBlendingOperation(theActiveTimes);
        // 8. Lasketaanko 'analyysit' ja niiden sijoitus editoituun dataan my�s 0-hetke� edelt�ville editoidun datan ajoille?
    }
    return false;
}

bool NFmiControlPointObservationBlender::DoBlendingDataGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues)
{
    // Mik� oli k�ytetty griddaus functio?
    FmiGriddingFunction griddingFunction = itsCPManager->CPGriddingProperties().Function();
    if(griddingFunction != kFmiErrorGriddingFunction)
    {
        NFmiDataParamControlPointModifier::DoDataGridding(xValues, yValues, zValues, static_cast<int>(xValues.size()), GetUsedGridData(), itsGridCropRelativeRect, griddingFunction, itsObsDataGridding, kFloatMissing);

        return true;
    }
    return false;
}

// Blendataan annettu muutoskentt� editoituun parametriin annetun blendingTimes:in aikarajoissa.
bool NFmiControlPointObservationBlender::MakeBlendingOperation(NFmiTimeDescriptor &blendingTimes)
{
    auto status = false;
    itsBlendingDataHelper.limitChecker = NFmiLimitChecker(static_cast<float>(itsDrawParam->AbsoluteMinValue()), static_cast<float>(itsDrawParam->AbsoluteMaxValue()), static_cast<FmiParameterName>(itsInfo->Param().GetParamIdent()));
    for(itsInfo->ResetLevel(); itsInfo->NextLevel(); ) // Tuskin ikin� p��st��n 3D editointiin, mutta varaudutaan silti siihen
    {
        itsBlendingDataHelper.blendingTimeSize = blendingTimes.Size();
        for(blendingTimes.Reset(); blendingTimes.Next(); )
        {
            itsBlendingDataHelper.blendingTimeIndex = blendingTimes.Index();
            // Blendaus menee niin ett� alkuhetkell� muutos kentt� otetaan kertoimella 1. ja viimeisell� se otetaan kertoimella 0. (eli viimeinen aika-askel voidaan skipata)
            if(itsBlendingDataHelper.blendingTimeIndex + 1 >= itsBlendingDataHelper.blendingTimeSize)
                break;
            if(itsInfo->Time(blendingTimes.Time())) // Ajan pit�isi aina l�yty�, koska blendingtimes on rakennettu editoidun datan aikojen perusteella
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
        itsInfo->Values(changeField); // otetaan originaali kentt� changeField:iin
    changeField -= analysisField; // matriisi laskuoperaatiot ottavat huomioon puuttuvat arvot

    // Zoomed crop tapauksessa pit�� muutos liu'uttaa nolliin kohti zoomatubn alueen reunoja
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
    // Jos tietyss� cp-pisteess� on jo et�isyys minimi eli 0 km ja cp-arvo ei ole puuttuvaa, 
    // t�ll�in ei en�� tarvitse etsi� parempia asemia tai arvoja.
    if(cpDistanceToStationInKm[cpLocationIndex] == 0 && cpValues[cpLocationIndex] != kFloatMissing)
        return false;
    else
        return true;
}

// Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisess� CP-pisteess� on 0 (merkit��n missing arvolla).
bool NFmiControlPointObservationBlender::GetObservationsToChangeValueFields(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, const NFmiTimeDescriptor &allowedTimeRange)
{
    // 1. Haetaan k�ytetyst� CP-managerista pohjat, eli x- ja y-koordinaatit ja jotkut pohja-arvot z eli muutoskent�lle.
    if(GetChangeValues(xValues, yValues, zValues))
    {
        // 1.1. Laita z-kentt��n puuttuvaa arvoa
        zValues = std::vector<float>(zValues.size(), kFloatMissing);
        // 1.2. Vektori, jossa minimi et�isyydet CP-pisteiden ja l�ydettyjen asemien v�lill�
        std::vector<double> cpDistanceToStationInKm(zValues.size(), g_missingDistanceToStationInKm);
        // 1.3. Varsinaiset CP-pisteiden lokaatiot, joiden avulla et�isyyksi� lasketaan
        const auto &cpLatlonPoints = itsCPManager->CPLocationVector();
        // 2. K�y kaikki datat l�pi
        for(auto &info : itsObservationInfos)
        {
            // 3. K�y l�pi datan sallitut ajat uusimmasta taaksep�in
            if(info->FindNearestTime(allowedTimeRange.LastTime(), kBackward))
            {
                for( ; allowedTimeRange.IsInside(info->Time()); )
                {
                    // 4. K�y l�pi CP-pisteet ja etsi jokaiseen l�hin asema. 
                    for(size_t cpLocationIndex = 0; cpLocationIndex < cpLatlonPoints.size(); cpLocationIndex++)
                    {
                        if(::NeedToLookForObservationValuesForCpPoint(zValues, cpDistanceToStationInKm, cpLocationIndex))
                        {
                            NFmiLocation cpLocation(cpLatlonPoints[cpLocationIndex]);
                            if(info->NearestLocation(cpLocation, NFmiControlPointObservationBlendingData::MaxAllowedDistanceToStationInKm() * 1000))
                            {
                                auto distanceInKm = cpLocation.Distance(info->LatLonFast());
                                // Jos on l�ytynyt uusi l�hin asema, pit�� ensin muutos arvo nollata mahdollisesta edellisen datan l�hipisteen arvosta
                                if(distanceInKm < cpDistanceToStationInKm[cpLocationIndex])
                                {
                                    zValues[cpLocationIndex] = kFloatMissing;
                                    // 5. Laita aseman et�isyys pisteeseen joka CP-pisteeseen talteen, jotta voidaan etsi� toisista datoista l�hempi� asemia
                                    cpDistanceToStationInKm[cpLocationIndex] = distanceInKm;
                                }
                                // T�ss� etsit��n my�h�isimm�n sallitun ajan ei-puuttuvaa arvoa
                                if(distanceInKm == cpDistanceToStationInKm[cpLocationIndex])
                                {
                                    // 6. Miten CP-pisteiden arvoja rankataan t�rkeysj�rjestykseen?
                                    // 6.1. Ensin ratkaisee ett� on l�hin asema (vaikka datalle ei l�ytyisi sopivia aikoja)
                                    // 6.2. Sitten ratkaiseen viimeisimm�n sallitun ajan ei-puuttuva arvo
                                    if(zValues[cpLocationIndex] == kFloatMissing)
                                    {
                                        zValues[cpLocationIndex] = info->FloatValue();
                                    }
                                }
                            }
                        }
                    }
                    if(!info->PreviousTime())
                        break; // Jos edellist� aikaa ei ole datassa, voidaan my�s lopettaa
                }
            }
        }
        return true;
    }

    return false;
}
