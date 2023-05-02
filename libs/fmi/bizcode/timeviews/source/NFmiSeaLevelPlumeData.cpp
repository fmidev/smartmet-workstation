#include "NFmiSeaLevelPlumeData.h"
#include "NFmiDataIdent.h"
#include "NFmiSettings.h"
#include "NFmiInfoData.h"

namespace
{
    NFmiProducer getProducerFromSettings(const std::string& producerKey)
    {
        auto id = NFmiSettings::Require<unsigned long>(producerKey + "::Id");
        auto name = NFmiSettings::Require<std::string>(producerKey + "::Name");
        return NFmiProducer(id, name);
    }

    std::vector<NFmiParam> getFractileParametersFromSettings(const std::string& parametersKey)
    {
        auto idsStr = NFmiSettings::Require<std::string>(parametersKey + "::Ids");
        auto idsVector = NFmiStringTools::Split<std::vector<int>>(idsStr, ",");
        auto namesStr = NFmiSettings::Require<std::string>(parametersKey + "::Names");
        auto namesVector = NFmiStringTools::Split<std::vector<std::string>>(namesStr, ",");
        auto parameterCount = std::min(idsVector.size(), namesVector.size());
        if(parameterCount == 0)
            throw std::runtime_error(std::string("Sea-level configurations: getFractileParametersFromSettings failed, no parameters found"));

        std::vector<NFmiParam> parameters;
        for(size_t index = 0; index < parameterCount; index++)
        {
            parameters.push_back(NFmiParam(idsVector[index], namesVector[index]));
        }

        return parameters;
    }

    NFmiColor getColorFromString(const std::string& colorsString)
    {
        auto rgbValues = NFmiStringTools::Split<std::vector<float>>(colorsString, ",");
        if(rgbValues.size() < 3)
            throw std::runtime_error(std::string("Sea-level configurations: getColorFromString failed, not enough RGB values found"));
        return NFmiColor(rgbValues[0], rgbValues[1], rgbValues[2]);
    }

    std::vector<NFmiColor> getColorsFromSettings(const std::string& colorsKey)
    {
        auto colorsStr = NFmiSettings::Require<std::string>(colorsKey);
        auto colorsStrVector = NFmiStringTools::Split<std::vector<std::string>>(colorsStr, ";");
        if(colorsStrVector.empty())
            throw std::runtime_error(std::string("Sea-level configurations: getColorsFromSettings failed, no Fractile-Param-Colors found"));

        std::vector<NFmiColor> colors;
        for(const auto &colorStr : colorsStrVector)
        {
            colors.push_back(::getColorFromString(colorStr));
        }

        return colors;
    }

    NFmiSeaLevelProbData getSingleProbabilityStationDataFromSettings(const std::string& probabilityStationBaseKey, const std::string& stationName)
    {
        auto probabilityStationKey = probabilityStationBaseKey + "::" + stationName;
        auto id = NFmiSettings::Require<long>(probabilityStationKey + "::Id");
        auto latlonStr = NFmiSettings::Require<std::string>(probabilityStationKey + "::Latlon");
        auto latlonValues = NFmiStringTools::Split<std::vector<float>>(latlonStr, ",");
        if(latlonValues.size() != 2)
            throw std::runtime_error(std::string("Sea-level configurations: getSingleProbabilityStationDataFromSettings failed, latlonValues had not 2 values"));

        NFmiStation station(id, stationName, latlonValues[0], latlonValues[1]);
        auto probabilityLimitsStr = NFmiSettings::Require<std::string>(probabilityStationKey + "::ProbabilityLimits");
        auto probLimits = NFmiStringTools::Split<std::vector<float>>(probabilityLimitsStr, ",");
        if(probLimits.size() < 4)
            throw std::runtime_error(std::string("Sea-level configurations: getSingleProbabilityStationDataFromSettings failed, probabilityLimitValues had less than 4 values"));

        return NFmiSeaLevelProbData(station, probLimits[0], probLimits[1], probLimits[2], probLimits[3]);
    }

    std::vector<NFmiSeaLevelProbData> getProbabilityStationDataFromSettings(const std::string& probabilityStationKey)
    {
        auto stationNameList = NFmiSettings::ListChildren(probabilityStationKey);
        if(stationNameList.empty())
            throw std::runtime_error(std::string("Sea-level configurations: getProbabilityStationDataFromSettings failed, no probability-Stations found"));

        std::vector<NFmiSeaLevelProbData> probabilityStations;
        for(const auto& stationName : stationNameList)
        {
            probabilityStations.push_back(::getSingleProbabilityStationDataFromSettings(probabilityStationKey, stationName));
        }
        return probabilityStations;
    }
}

// *********************************************************************
// ***************** NFmiSingleSeaLevelPlumeData ***********************
// *********************************************************************

NFmiSingleSeaLevelPlumeData::NFmiSingleSeaLevelPlumeData() = default;

void NFmiSingleSeaLevelPlumeData::InitFromSettings(const std::string & baseKey)
{
    if(!initialized_)
    {
        initialized_ = true;
        settingsBaseKey_ = baseKey;
        auto childrenCheckList = NFmiSettings::ListChildren(baseKey);
        if(childrenCheckList.empty())
            return;

        try
        {
            foundAnySettings_ = true;

            seaLevelParameterId_ = NFmiSettings::Require<unsigned long>(settingsBaseKey_ + "::SeaLevelParameterId");

            auto producerKey = settingsBaseKey_ + "::FractileProducer";
            producer_ = ::getProducerFromSettings(producerKey);
            dataType_ = static_cast<NFmiInfoData::Type>(NFmiSettings::Require<int>(producerKey + "::DataType"));

            auto fractileParametersKey = settingsBaseKey_ + "::FractileParameters";
            fractileParams_ = ::getFractileParametersFromSettings(fractileParametersKey);
            fractileParamColors_ = ::getColorsFromSettings(fractileParametersKey + "::Colors");

            auto probabilityParametersKey = settingsBaseKey_ + "::ProbabilityParameters";
            probLimitParams_ = ::getFractileParametersFromSettings(probabilityParametersKey);
            probabilityLineColors_ = ::getColorsFromSettings(probabilityParametersKey + "::Colors");

            auto probabilityStationDataKey = settingsBaseKey_ + "::Stations";
            probabilityStationData_ = ::getProbabilityStationDataFromSettings(probabilityStationDataKey);
            dataOk_ = true;
        }
        catch(std::exception &e)
        { 
            configurationErrorMessage_ = e.what();
        }
    }
}

bool NFmiSingleSeaLevelPlumeData::dataOk() const
{ 
    return dataOk_; 
}

const NFmiSeaLevelProbData* NFmiSingleSeaLevelPlumeData::FindSeaLevelProbabilityStationData(const NFmiLocation* location, const NFmiPoint& latlon) const
{
    if(location && location->GetIdent() != 0)
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

bool NFmiSingleSeaLevelPlumeData::IsSeaLevelPlumeParam(unsigned long checkedParameterId) const
{
    return seaLevelParameterId_ == checkedParameterId;
}

bool NFmiSingleSeaLevelPlumeData::IsSeaLevelProbLimitParam(const NFmiDataIdent& dataIdent) const
{
    auto iter = std::find_if(probLimitParams_.begin(), probLimitParams_.end(), 
        [&](const auto& param) {return param.GetIdent() == dataIdent.GetParamIdent(); }
    );
    return iter != probLimitParams_.end();
}

// *********************************************************************
// ***************** NFmiSeaLevelPlumeData *****************************
// *********************************************************************

NFmiSeaLevelPlumeData::NFmiSeaLevelPlumeData() = default;

void NFmiSeaLevelPlumeData::InitFromSettings(const std::string& baseKey)
{
    if(!initialized_)
    {
        initialized_ = true;
        settingsBaseKey_ = baseKey;
        auto childrenCheckList = NFmiSettings::ListChildren(baseKey);
        if(childrenCheckList.empty())
        {
            baseConfigurationMessage_ = "No configurations for Sea-level help data";
            return;
        }

        baseConfigurationMessage_ = "Sea-level help data configurations count is " + std::to_string(childrenCheckList.size()) + ": ";
        for(const auto &childrenName : childrenCheckList)
        {
            auto seaLevelPlumeDataKey = settingsBaseKey_ + "::" + childrenName;
            NFmiSingleSeaLevelPlumeData seaLevelPlumeData;
            seaLevelPlumeData.InitFromSettings(seaLevelPlumeDataKey);
            if(!baseConfigurationMessage_.empty())
            {
                baseConfigurationMessage_ += ", ";
            }

            if(seaLevelPlumeData.dataOk())
            {
                plumeDataContainer_.push_back(seaLevelPlumeData);
                baseConfigurationMessage_ += childrenName + " Ok";
            }
            else
            {
                baseConfigurationMessage_ += childrenName + " Error";
                if(!configurationErrorMessage_.empty())
                {
                    configurationErrorMessage_ += ", ";
                }
                configurationErrorMessage_ += seaLevelPlumeData.configurationErrorMessage();
            }
        }
    }
}

const NFmiSingleSeaLevelPlumeData* NFmiSeaLevelPlumeData::getSeaLevelPlumeData(unsigned long checkedParameterId) const
{
    auto iter = std::find_if(plumeDataContainer_.begin(), plumeDataContainer_.end(),
        [&](const auto& singlePlumeData) {return singlePlumeData.IsSeaLevelPlumeParam(checkedParameterId); }
    );

    if(iter != plumeDataContainer_.end())
    {
        return &(*iter);
    }
    else
    {
        return nullptr;
    }
}
