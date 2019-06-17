#pragma once

#include "NFmiParameterName.h"
#include <string>
#include <map>
#include <vector>
//#include <optional>

// With TimeSerialParameters you can define those matching parameter id's that can be used in time - serial - view with 
// each other as comparison data. Example from FMI : wind gust parameters can have parameter id 467 and 417.
// If you select from one data parameter 467 on time - serial view we want to compare that against other 
// producers 467 or 417 parameters which ever is found on each compared data source.
// With WindGust = 467, 417 - setting we declare that parameter named as WindGust can be either 467 or 417 on 
// time - serial view.You can have more id's in comma separated list.
class TimeSerialParameters
{
public:
    using ParamVector = std::vector<FmiParameterName>;
//    using OptionalValue = std::optional<std::reference_wrapper<ParamVector>>;
    using OptionalValue = ParamVector*;
    using ComparisonMap = std::map<FmiParameterName, ParamVector>;
    using ParamRawMap = std::map<std::string, ParamVector>;
private:
    // Map containing all the connected parameters to other parameters. Read from configurations.
    ComparisonMap comparisonParameterMap_;
    bool initialized_ = false;
public:
    TimeSerialParameters() = default;

    void initializeFromConfigurations();
    OptionalValue getComparisonParameters(FmiParameterName parameter);

private:
    void initializeFromConfigurations(const std::string &baseConfigurationName, const std::vector<std::string> &configurationNames);
};
