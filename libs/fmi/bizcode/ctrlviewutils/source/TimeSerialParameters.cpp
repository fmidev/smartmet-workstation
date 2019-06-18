#include "TimeSerialParameters.h"
#include "NFmiSettings.h"
#include "catlog/catlog.h"
#include <boost/algorithm/string.hpp>

// In configurations following settings form connected parameters:
// SmartMet::TimeSerialParameters::WindGust = 467,417
// So here is said that Wind-gust parameter 467 is connected to parameter 417 and vice verse.
void TimeSerialParameters::initializeFromConfigurations()
{
    if(!initialized_)
    {
        initialized_ = true;
        const std::string baseConfigurationName = "SmartMet::TimeSerialParameters";
        try
        {
            initializeFromConfigurations(baseConfigurationName, NFmiSettings::ListChildren(baseConfigurationName));
        }
        catch(std::exception& e)
        {
            std::string errorMessage = "Error in ";
            errorMessage += __FUNCTION__;
            errorMessage += ": ";
            errorMessage += e.what();
            throw std::runtime_error(errorMessage);
        }
        catch(...)
        {
            std::string errorMessage = "Unknown error in ";
            errorMessage += __FUNCTION__;
            throw std::runtime_error(errorMessage);
        }
    }
}

TimeSerialParameters::OptionalValue TimeSerialParameters::getComparisonParameters(FmiParameterName parameter)
{
    auto iter = comparisonParameterMap_.find(parameter);
    if(iter == comparisonParameterMap_.end())
        return nullptr;  // nullopt;
    else
    {
//        return TimeSerialParameters::OptionalValue{ iter->second };
        return &iter->second;
    }
}

static TimeSerialParameters::ParamVector makeParameterRawValue(const std::string& configurationValue, const std::string& configurationName)
{
    std::vector<std::string> paramIdStrings;
    boost::split(paramIdStrings, configurationValue, boost::is_any_of(","));
    TimeSerialParameters::ParamVector paramVector;
    std::transform(paramIdStrings.begin(), paramIdStrings.end(), std::back_inserter(paramVector),
        [](const std::string& str) { return static_cast<FmiParameterName>(std::stoi(str)); });
    return paramVector;
}

static std::set<FmiParameterName> getUniqueParameters(const TimeSerialParameters::ParamRawMap& parameterRawMap)
{
    std::set<FmiParameterName> uniqueParameters;
    for(const auto& parameterRawMapData : parameterRawMap)
    {
        uniqueParameters.insert(parameterRawMapData.second.begin(), parameterRawMapData.second.end());
    }
    return uniqueParameters;
}

static TimeSerialParameters::ParamVector makeRelatedParametersVector(FmiParameterName parameter, const TimeSerialParameters::ParamRawMap& parameterRawMap)
{
    // Haetaan ensin kaikki uniikit parametrit, jotka liittyvät annettuun parametriin (voi olla yhteyksiä eri parametri listoissa)
    std::set<FmiParameterName> uniqueRelatedParameters;
    for(const auto& parameterRawMapData : parameterRawMap)
    {
        const auto& paramVector = parameterRawMapData.second;
        auto iter = std::find(paramVector.begin(), paramVector.end(), parameter);
        if(iter != paramVector.end())
            uniqueRelatedParameters.insert(paramVector.begin(), paramVector.end());
    }

    // Poistetaan itse haettu parametri
    uniqueRelatedParameters.erase(parameter);

    // Muutetaan lopulliseen muotoon vektoriksi
    return TimeSerialParameters::ParamVector(uniqueRelatedParameters.begin(), uniqueRelatedParameters.end());
}

// Searches all the connected parameters for each parameter on parameterRawMap.
// Creates new map where key is FmiParameter for those each separate parameter and data is vector of related FMiParameterName's.
static TimeSerialParameters::ComparisonMap makeFinalComparisonParameterMap(const TimeSerialParameters::ParamRawMap &parameterRawMap)
{
    auto uniqueParameters = ::getUniqueParameters(parameterRawMap);
    TimeSerialParameters::ComparisonMap finalComparisonMap;
    for(auto parameter : uniqueParameters)
    {
        finalComparisonMap.insert(std::make_pair(parameter, ::makeRelatedParametersVector(parameter, parameterRawMap)));
    }
    return finalComparisonMap;
}

void TimeSerialParameters::initializeFromConfigurations(const std::string& baseConfigurationName, const std::vector<std::string>& configurationNames)
{
    if(configurationNames.empty())
        return;
    ParamRawMap parameterRawMap;
    for(const auto& name : configurationNames)
    {
        auto finalConfigurationName = baseConfigurationName + "::" + name;
        try
        {
            auto configurationValue = NFmiSettings::Require<std::string>(finalConfigurationName);
            parameterRawMap.insert(std::make_pair(name, ::makeParameterRawValue(configurationValue, finalConfigurationName)));
        }
        catch(std::exception& e)
        {
            // Don't stop operation if any of the configuration lines is illegal, only log error
            std::string errorMessage = "Error in ";
            errorMessage += __FUNCTION__;
            errorMessage += " with param-name '" + finalConfigurationName;
            errorMessage += "': ";
            errorMessage += e.what();
            CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
        }
    }

    comparisonParameterMap_ = ::makeFinalComparisonParameterMap(parameterRawMap);
}
