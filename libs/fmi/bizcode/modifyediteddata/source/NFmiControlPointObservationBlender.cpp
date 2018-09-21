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

    // Mikä oli käytetty griddaus functio?
    FmiGriddingFunction griddingFunction = itsCPManager->CPGriddingProperties().Function();
    if(griddingFunction != kFmiErrorGriddingFunction)
    {
        // 1. Mikä on sallittu aikahaarukka? esim. 10.00 - 10.20 Utc
        auto allowedTimes = ::CalcAllowedTimes(itsActualFirstTime, expirationTimeInMinutes);
        // 2. Mikä on maksimi etäisyys CP-pisteestä lähimpään havaintoasemaan
        const double maxAllowedDistanceToStationInKm = 10.;
        // 3. Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisessä CP-pisteessä on 0 (merkitään missing arvolla).
        std::vector<float> xValues, yValues, zValues;
        if(GetObservationsToChangeValueFields(xValues, yValues, zValues, allowedTimes, maxAllowedDistanceToStationInKm))
        {
            // 4. Täydennä CP-pisteiden arvoja seuraavasti, jos CP-pisteessä on puuttuva, otetaan editoidusta datasta 0-hetkeltä arvo siihen (0-muutos)
            FillZeroChangeValuesForMissingCpPoints(zValues);
            // 5. Laske CP-pisteiden avulla 0-hetken 'analyysikenttä'
            NFmiDataParamControlPointModifier::DoDataGridding(xValues, yValues, zValues, static_cast<int>(xValues.size()), itsGridData, itsGridCropRelativeRect, griddingFunction, itsObsDataGridding, kFloatMissing);
            // 6. Laske analyysikentän ja editoidun datan 0-hetken kentän avulla muutoskenttä
            // 7. Blendaa muutoskenttä editoituun dataan liu'uttamalla
            // 8. Lasketaanko 'analyysit' ja niiden sijoitus editoituun dataan myös 0-hetkeä edeltäville editoidun datan ajoille?
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


// Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisessä CP-pisteessä on 0 (merkitään missing arvolla).
bool NFmiControlPointObservationBlender::GetObservationsToChangeValueFields(std::vector<float> xValues, std::vector<float> yValues, std::vector<float> zValues, const NFmiTimeDescriptor &allowedTimeRange, double maxAllowedDistanceToStationInKm)
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
                        NFmiLocation cpLocation(cpLatlonPoints[cpLocationIndex]);
                        if(info->NearestLocation(cpLocation, maxAllowedDistanceToStationInKm * 1000))
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
                    if(!info->PreviousTime())
                        break; // Jos edellistä aikaa ei ole datassa, voidaan myös lopettaa
                }
            }
        }
        return true;
    }

    return false;
}
