#include "NFmiSeaLevelPlumeData.h"
#include "NFmiDataIdent.h"

NFmiSeaLevelPlumeData::NFmiSeaLevelPlumeData() = default;

void NFmiSeaLevelPlumeData::InitFromSettings(const std::string & baseKey)
{
    if(probabilityStationData_.empty())
    {
        producer_ = NFmiProducer(2168, "Hansen EPS");
        dataType_ = NFmiInfoData::kViewable;
        fractileParams_ = std::vector<FmiParameterName>{ static_cast<FmiParameterName>(1309), static_cast<FmiParameterName>(1310), static_cast<FmiParameterName>(1311) , static_cast<FmiParameterName>(1312) , static_cast<FmiParameterName>(1313) , static_cast<FmiParameterName>(1314) , static_cast<FmiParameterName>(1315) };
        fractileParamLabels_ = std::vector<std::string>{ "F95", "F90", "F75", "F50", "F25", "F10", "F5" };
        fractileParamColors_ = std::vector<NFmiColor>{ NFmiColor(1.f, 0.f, 0.f), NFmiColor(1.f, 0.25f, 0.25f), NFmiColor(1.f, 0.5f, 0.5f) , NFmiColor(0.f, 0.5f, 0.f), NFmiColor(0.4f, 0.4f, 1.f), NFmiColor(0.25f, 0.25f, 1.f), NFmiColor(0.f, 0.f, 1.f) };

        // Hamina kuuluu Suomenlahden it‰osaan (prob arvot sielt‰)
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100051, "Hamina", 27.2, 60.56), -70, 110, 145, 175));
        // Porvoo ja Helsinki (Suomenlahden l‰nsiosaan)
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100066, "Porvoo", 25.6251, 60.2058), -60, 80, 115, 130));
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100052, "Helsinki", 24.95, 60.13), -60, 80, 115, 130));
        // Turku ja Hanko (Saaristomeri ja Suomenlahden l‰nsiosa)
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100054, "Turku", 22.22, 60.44), -50, 70, 95, 110));
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100053, "Hanko", 22.95, 59.82), -50, 70, 95, 110));
        // Degerby (Ahvenanmeri ja Saaristomeri)
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100055, "Degerby", 20.38, 60.03), -50, 65, 85, 100));
        // Rauma, M‰ntyluoto ja Kaskinen (Selk‰meri)
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100056, "Rauma", 21.49, 61.13), -50, 75, 100, 120));
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100057, "M‰ntyluoto", 21.49, 61.59), -50, 75, 100, 120));
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100058, "Kaskinen", 21.21, 62.34), -50, 75, 100, 120));
        // Vaasa (Merenkurkku)
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100059, "Vaasa", 21.61, 63.1), -50, 85, 110, 130));
        // Pietarsaari (Per‰meren etel‰osa)
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100060, "Pietarsaari", 22.67, 63.68), -65, 85, 110, 130));
        // Raahe, Oulu ja Kemi (Per‰meren pohjoisosa)
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100061, "Raahe", 24.48, 64.68), -65, 85, 110, 130));
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100062, "Oulu", 25.49, 65.02), -80, 115, 140, 170));
        probabilityStationData_.push_back(NFmiSeaLevelProbData(NFmiStation(100063, "Kemi", 24.53, 65.68), -80, 115, 140, 170));

        probLimitParams_ = std::vector<FmiParameterName>{ static_cast<FmiParameterName>(1305), static_cast<FmiParameterName>(1306), static_cast<FmiParameterName>(1307) , static_cast<FmiParameterName>(1308) };
        probabilityLineColors_ = std::vector<NFmiColor>{ NFmiColor(0.7f, 0.7f, 0.f), NFmiColor(0.7f, 0.7f, 0.f), NFmiColor(1.f, 0.5f, 0.f) , NFmiColor(1.f, 0.f, 0.f) };

        initializationOk_ = true;
    }
}

const NFmiSeaLevelProbData* NFmiSeaLevelPlumeData::FindSeaLevelProbabilityStationData(const NFmiLocation* location, const NFmiPoint& latlon)
{
    if(location)
    {
        // Etsi joko tarkka location-id pohjaisen aseman data
        for(const auto& probStationData : probabilityStationData_)
        {
            if(location->GetIdent() == probStationData.station_.GetIdent())
                return &probStationData;
        }
    }
    else
    {
        // Tai etsi l‰himm‰n latlon pisteesen liittyv‰n aseman data, edellytt‰en ett‰ latlon piste on g_SeaLevelProbabilityMaxSearchRangeInMetres sis‰ll‰.
        // T‰t‰ k‰ytet‰‰n, jos seaLevel data sattuu olemaan hiladataa.
        double minDistanceInMetres = 999999999;
        const NFmiSeaLevelProbData* minDistanceProbDataPtr = nullptr;
        NFmiLocation searchedLocation(latlon);
        for(const auto& probStationData : probabilityStationData_)
        {
            auto currentDistanceInMeters = searchedLocation.Distance(probStationData.station_);
            if(currentDistanceInMeters < minDistanceInMetres)
            {
                minDistanceInMetres = currentDistanceInMeters;
                minDistanceProbDataPtr = &probStationData;
            }
        }
        if(minDistanceInMetres <= probabilityMaxSearchRangeInMetres_)
            return minDistanceProbDataPtr;
    }
    return nullptr;
}

bool NFmiSeaLevelPlumeData::IsSeaLevelPlumeParam(const NFmiDataIdent& param)
{
    return param.GetParamIdent() == kFmiSeaLevel;
}

bool NFmiSeaLevelPlumeData::IsSeaLevelProbLimitParam(const NFmiDataIdent& param)
{
    auto iter = std::find(probLimitParams_.begin(), probLimitParams_.end(), param.GetParamIdent());
    return iter != probLimitParams_.end();
}
