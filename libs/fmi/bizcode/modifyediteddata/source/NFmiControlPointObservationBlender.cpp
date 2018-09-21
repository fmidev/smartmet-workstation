#include "NFmiControlPointObservationBlender.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiFastQueryInfo.h"

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

void NFmiControlPointObservationBlender::DoTimeSerialModifications(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks)
{
    // Kuinka vanhoja havaintoja sallitaan mukaan suhteessa aloitusaikaan itsActualFirstTime
    const long expirationTimeInMinutes = 20;

    // Mik� oli k�ytetty griddaus functio?
    FmiGriddingFunction griddingFunction = itsCPManager->CPGriddingProperties().Function();
    if(griddingFunction != kFmiErrorGriddingFunction)
    {
        // 1. Mik� on sallittu aikahaarukka? esim. 10.00 - 10.20 Utc
        auto allowedTimes = ::CalcAllowedTimes(itsActualFirstTime, expirationTimeInMinutes);
        // 2. Mik� on maksimi et�isyys CP-pisteest� l�himp��n havaintoasemaan
        const double maxAllowedDistanceToStationInKm = 10.;
        // 3. Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisess� CP-pisteess� on 0 (merkit��n missing arvolla).
        std::vector<float> xValues, yValues, zValues;
        if(GetObservationsToChangeValueFields(xValues, yValues, zValues, allowedTimes, maxAllowedDistanceToStationInKm))
        {
            // 4. T�ydenn� CP-pisteiden arvoja seuraavasti, jos CP-pisteess� on puuttuva, otetaan editoidusta datasta 0-hetkelt� arvo siihen (0-muutos)
            FillZeroChangeValuesForMissingCpPoints(zValues);
            // 5. Laske CP-pisteiden avulla 0-hetken 'analyysikentt�'
            NFmiDataParamControlPointModifier::DoDataGridding(xValues, yValues, zValues, static_cast<int>(xValues.size()), itsGridData, itsGridCropRelativeRect, griddingFunction, itsObsDataGridding, kFloatMissing);
            // 6. Laske analyysikent�n ja editoidun datan 0-hetken kent�n avulla muutoskentt�
            // 7. Blendaa muutoskentt� editoituun dataan liu'uttamalla
            // 8. Lasketaanko 'analyysit' ja niiden sijoitus editoituun dataan my�s 0-hetke� edelt�ville editoidun datan ajoille?
        }
    }
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


// Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisess� CP-pisteess� on 0 (merkit��n missing arvolla).
bool NFmiControlPointObservationBlender::GetObservationsToChangeValueFields(std::vector<float> xValues, std::vector<float> yValues, std::vector<float> zValues, const NFmiTimeDescriptor &allowedTimeRange, double maxAllowedDistanceToStationInKm)
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
                        NFmiLocation cpLocation(cpLatlonPoints[cpLocationIndex]);
                        if(info->NearestLocation(cpLocation, maxAllowedDistanceToStationInKm * 1000))
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
                    if(!info->PreviousTime())
                        break; // Jos edellist� aikaa ei ole datassa, voidaan my�s lopettaa
                }
            }
        }
        return true;
    }

    return false;
}
