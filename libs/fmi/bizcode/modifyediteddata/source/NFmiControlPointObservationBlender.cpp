#include "NFmiControlPointObservationBlender.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiAreaMaskList.h"
#include "NFmiDrawParam.h"
#include "NFmiAnalyzeToolData.h"

#include <boost/math/special_functions/round.hpp>

#include <fstream>

NFmiControlPointObservationBlender::BlendingDataHelper::BlendingDataHelper()
:changeField()
,limitChecker(kFloatMissing, kFloatMissing, kFmiBadParameter)
{

}


NFmiControlPointObservationBlender::NFmiControlPointObservationBlender(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
    unsigned long theAreaMask, boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, const NFmiRect &theCPGridCropRect,
    bool theUseGridCrop, const NFmiPoint &theCropMarginSize, checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, const NFmiMetTime &actualFirstTime, const NFmiGriddingProperties &griddingProperties)
    :NFmiDataParamControlPointModifier(theInfo, theDrawParam, theMaskList,
        theAreaMask, theCPManager, theCPGridCropRect,
        theUseGridCrop, theCropMarginSize, griddingProperties)
    , itsObservationInfos(observationInfos)
    , itsActualFirstTime(actualFirstTime)
{
}

NFmiTimeDescriptor NFmiControlPointObservationBlender::CalcAllowedObsBlenderTimes(const NFmiMetTime &actualFirstTime, const NFmiMetTime &firstEditedTime, long expirationTimeInMinutes)
{
    auto allowedRangeStartTime = (actualFirstTime > firstEditedTime) ? actualFirstTime : firstEditedTime;
    allowedRangeStartTime.SetTimeStep(1, false);
    allowedRangeStartTime.ChangeByMinutes(-expirationTimeInMinutes);
    return NFmiTimeDescriptor(actualFirstTime, NFmiTimeBag(allowedRangeStartTime, actualFirstTime, expirationTimeInMinutes));
}

bool NFmiControlPointObservationBlender::ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks)
{
    // assert(itsInfo->Param().GetParamIdent() == kFmiTemperature);
    // 1. Mik‰ on sallittu aikahaarukka? esim. 10.00 - 10.20 Utc
    auto allowedTimes = NFmiControlPointObservationBlender::CalcAllowedObsBlenderTimes(itsActualFirstTime, itsActualFirstTime, NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes());
    // 2. Laitetaan editoitu data osoittamaan 1. muokattavaan aikaan
    if(!itsInfo->Time(theActiveTimes.FirstTime()))
    {
        itsInfo->FirstTime();
    }
    auto firstModifiedTimeIndex = itsInfo->TimeIndex();

    // 3. Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisess‰ CP-pisteess‰ on 0 (merkit‰‰n missing arvolla).
    std::vector<float> xValues, yValues, zValues;
    if(NFmiControlPointObservationBlender::GetObservationsToChangeValueFields(itsCPManager, itsInfo, itsObservationInfos, xValues, yValues, zValues, allowedTimes))
    {
        // 4. T‰ydenn‰ CP-pisteiden arvoja seuraavasti, jos CP-pisteess‰ on puuttuva arvo, laitetaan siihen 0 muutos.
        FillZeroChangeValuesForMissingCpPoints(zValues);
        // 5. Laske CP-pisteiden avulla 0-hetken 'muutoskentt‰'
        DoBlendingDataGridding(xValues, yValues, zValues);
        // 6. Ota muutoskentt‰ talteen itsBlendingDataHelper:iin
        itsBlendingDataHelper.changeField = GetUsedGridData();
        // 7. Blendaa muutoskentt‰ editoituun dataan liu'uttamalla
        auto status = MakeBlendingOperation(theActiveTimes);
        // 8. Lasketaan 'analyysit' ja niiden sijoitus editoituun dataan myˆs 0-hetke‰ edelt‰ville editoidun datan ajoille
        MakeAnalysisModificationToStartOfEditedData(firstModifiedTimeIndex);
        return status;
    }
    return false;
}

void NFmiControlPointObservationBlender::MakeAnalysisModificationToStartOfEditedData(unsigned long firstModifiedTimeIndex)
{
    // Aloitus aikaindeksi ei saa olla 0 tai muu out-of-data indeksi
    if(firstModifiedTimeIndex > 0 && firstModifiedTimeIndex < itsInfo->SizeTimes())
    {
        // Siirret‰‰n timeIndex yksi askel taaksep‰in Obs-blender tyˆkalun aloitusajasta
        firstModifiedTimeIndex--;
        if(itsInfo->TimeIndex(firstModifiedTimeIndex))
        {
            // N‰ill‰ asetuksilla saadaan 'blendaus' toiminto k‰ytt‰m‰‰n analyysikentt‰‰ t‰ysill‰ arvoilla
            itsBlendingDataHelper.blendingTimeSize = 2; // 2 on pienin time-size mit‰ voi olla
            itsBlendingDataHelper.blendingTimeIndex = 0;
            do
            {
                // Sallitut havaintoajat ovat vain kulloinkin l‰pik‰yt‰v‰ ajan hetki editoidussa datassa
                const auto &currentTime = itsInfo->Time();
                NFmiTimeDescriptor allowedObservationTimes(currentTime, NFmiTimeBag(currentTime, currentTime, 60));
                std::vector<float> xValues, yValues, zValues;
                if(NFmiControlPointObservationBlender::GetObservationsToChangeValueFields(itsCPManager, itsInfo, itsObservationInfos, xValues, yValues, zValues, allowedObservationTimes))
                {
                    // 4. T‰ydenn‰ CP-pisteiden arvoja seuraavasti, jos CP-pisteess‰ on puuttuva, laitetaan siihen 0-muutos
                    FillZeroChangeValuesForMissingCpPoints(zValues);
                    // 5. Laske CP-pisteiden avulla 0-hetken 'analyysikentt‰'
                    DoBlendingDataGridding(xValues, yValues, zValues);
                    // 6. Ota muutoskentt‰ talteen itsBlendingDataHelper:iin
                    itsBlendingDataHelper.changeField = GetUsedGridData();
                    itsParamMaskList->SyncronizeMaskTime(currentTime);
                    DoLocationGridCalculations(itsBlendingDataHelper.changeField);
                }
            } while(itsInfo->PreviousTime());
        }
    }
}

bool NFmiControlPointObservationBlender::DoBlendingDataGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues)
{
    if(itsGriddingProperties.function() != kFmiErrorGriddingFunction)
    {
        auto obsBlenderCalculationRadiusRelative = static_cast<float>(NFmiGriddingProperties::ConvertLengthInKmToRelative(itsGriddingProperties.rangeLimitInKm(), itsInfo->Area()));
        NFmiDataParamControlPointModifier::DoDataGridding(xValues, yValues, zValues, static_cast<int>(xValues.size()), GetUsedGridData(), itsGridCropRelativeRect, itsGriddingProperties, itsObsDataGridding, obsBlenderCalculationRadiusRelative);
        if(fUseGridCrop)
            FixCroppedMatrixMargins(GetUsedGridData(), itsCropMarginSize);
        return true;
    }
    return false;
}

// Blendataan annettu muutoskentt‰ editoituun parametriin annetun blendingTimes:in aikarajoissa.
bool NFmiControlPointObservationBlender::MakeBlendingOperation(NFmiTimeDescriptor &blendingTimes)
{
    auto status = false;
    itsBlendingDataHelper.limitChecker = NFmiLimitChecker(static_cast<float>(itsDrawParam->AbsoluteMinValue()), static_cast<float>(itsDrawParam->AbsoluteMaxValue()), static_cast<FmiParameterName>(itsInfo->Param().GetParamIdent()));
    itsBlendingDataHelper.blendingTimeSize = blendingTimes.Size();
    for(blendingTimes.Reset(); blendingTimes.Next(); )
    {
        itsBlendingDataHelper.blendingTimeIndex = blendingTimes.Index();
        // Blendaus menee niin ett‰ alkuhetkell‰ muutos kentt‰ otetaan kertoimella 1. ja viimeisell‰ se otetaan kertoimella 0. (eli viimeinen aika-askel voidaan skipata)
        if(itsBlendingDataHelper.blendingTimeIndex + 1 >= itsBlendingDataHelper.blendingTimeSize)
            break;
        if(itsInfo->Time(blendingTimes.Time())) // Ajan pit‰isi aina lˆyty‰, koska blendingtimes on rakennettu editoidun datan aikojen perusteella
        {
            itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
            DoLocationGridCalculations(itsBlendingDataHelper.changeField);
            status = true;
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
        itsInfo->Values(changeField); // otetaan originaali kentt‰ changeField:iin
    changeField -= analysisField; // matriisi laskuoperaatiot ottavat huomioon puuttuvat arvot

    // Zoomed crop tapauksessa pit‰‰ muutos liu'uttaa nolliin kohti zoomatubn alueen reunoja
    if(fUseGridCrop)
        NFmiDataParamControlPointModifier::FixCroppedMatrixMargins(changeField, itsCropMarginSize);

    return changeField;
}

const double g_missingDistanceToStationInKm = 9999999999.;

void NFmiControlPointObservationBlender::FillZeroChangeValuesForMissingCpPoints(std::vector<float> &zValues)
{
    for(auto &value : zValues)
    {
        if(value == kFloatMissing)
        {
            value = 0;
        }
    }
}

bool NFmiControlPointObservationBlender::SeekClosestObsBlenderData(const NFmiLocation &cpLocation, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, const NFmiTimeDescriptor &allowedTimeRange, float &zValueOut, boost::shared_ptr<NFmiFastQueryInfo> &closestObsBlenderInfoOut)
{
    closestObsBlenderInfoOut.reset();
    double cpDistanceToStationInKm = g_missingDistanceToStationInKm;
    // 2. K‰y kaikki datat l‰pi
    for(auto &info : observationInfos)
    {
        // 3. K‰y l‰pi datan sallitut ajat uusimmasta taaksep‰in
        if(info->FindNearestTime(allowedTimeRange.LastTime(), kBackward))
        {
            for(; allowedTimeRange.IsInside(info->Time()); )
            {
                if(info->NearestLocation(cpLocation, NFmiControlPointObservationBlendingData::MaxAllowedDistanceToStationInKm() * 1000))
                {
                    auto distanceInKm = cpLocation.Distance(info->LatLonFast());
                    // Jos on lˆytynyt uusi l‰hin asema, pit‰‰ ensin muutos arvo nollata mahdollisesta edellisen datan l‰hipisteen arvosta
                    if(distanceInKm < cpDistanceToStationInKm)
                    {
                        // Nollataan ensin muutos, koska haluamme l‰himm‰lt‰ asemalta vaikka puuttuvaa
                        zValueOut = kFloatMissing;
                        // 5. Laita aseman et‰isyys pisteeseen joka CP-pisteeseen talteen, jotta voidaan etsi‰ toisista datoista l‰hempi‰ asemia
                        cpDistanceToStationInKm = distanceInKm;
                    }
                    // T‰ss‰ etsit‰‰n myˆh‰isimm‰n sallitun ajan ei-puuttuvaa arvoa (t‰m‰ erillinen ehto hanskaa mm. seuraavaa tilannetta, kahdessa eri datassa on sama asema ja toisessa on puuttuvia arvoja)
                    if(distanceInKm == cpDistanceToStationInKm)
                    {
                        // 6. Miten CP-pisteiden arvoja rankataan t‰rkeysj‰rjestykseen?
                        // 6.1. Ensin ratkaisee ett‰ on l‰hin asema (vaikka datalle ei lˆytyisi sopivia aikoja)
                        // 6.2. Sitten ratkaiseen viimeisimm‰n sallitun ajan ei-puuttuva arvo
                        if(zValueOut == kFloatMissing)
                        {
                            float obsValue = info->FloatValue();
                            float origValue = theInfo->InterpolatedValue(cpLocation.GetLocation());
                            if(obsValue != kFloatMissing && origValue != kFloatMissing)
                            {
                                zValueOut = origValue - obsValue;
                                // Otetaan talteen l‰himm‰n aseman data, jossa lokaatio on laitettu osoittamaan kyseiseen asemaan
                                closestObsBlenderInfoOut = info;
                            }
                        }
                    }
                }
                if(!info->PreviousTime())
                    break; // Jos edellist‰ aikaa ei ole datassa, lopetetaan aikalooppi
            }
        }
    }
    return closestObsBlenderInfoOut != nullptr;
}

// Hae muutosarvot originaali kent‰n arvoon sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisess‰ CP-pisteess‰ on 0 (merkit‰‰n missing arvolla).
bool NFmiControlPointObservationBlender::GetObservationsToChangeValueFields(boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, const NFmiTimeDescriptor &allowedTimeRange)
{
    // 1. Haetaan k‰ytetyst‰ CP-managerista pohjat, eli x- ja y-koordinaatit ja jotkut pohja-arvot z eli muutoskent‰lle.
    if(NFmiDataParamControlPointModifier::GetChangeValues(theCPManager, xValues, yValues, zValues))
    {
        // 1.1. Laita z-kentt‰‰n puuttuvaa arvoa
        zValues = std::vector<float>(zValues.size(), kFloatMissing);
        // 1.3. Varsinaiset CP-pisteiden lokaatiot, joiden avulla et‰isyyksi‰ lasketaan
        const auto &cpLatlonPoints = theCPManager->CPLocationVector();
        // 4. K‰y l‰pi CP-pisteet ja etsi jokaiseen l‰hin asema. 
        for(size_t cpLocationIndex = 0; cpLocationIndex < cpLatlonPoints.size(); cpLocationIndex++)
        {
            NFmiLocation cpLocation(cpLatlonPoints[cpLocationIndex]);
            boost::shared_ptr<NFmiFastQueryInfo> dummyInfo;
            if(NFmiControlPointObservationBlender::SeekClosestObsBlenderData(cpLocation, theInfo, observationInfos, allowedTimeRange, zValues[cpLocationIndex], dummyInfo))
            {
                // ei tarvii tehd‰ mit‰‰n t‰‰ll‰
            }
        }
        return true;
    }

    return false;
}
