#pragma once

#include "NFmiDataIdent.h"
#include "NFmiLevel.h"
#include "NFmiInfoData.h"
#include <vector>

// Here is stored all the needed info about used parameters: dataIdent, their possible level info and if param was used with vertical function.
struct MacroParamDataInfo
{
    std::string variableName_;
    NFmiDataIdent dataIdent_;
    NFmiLevel level_;
    std::string possibleVerticalFunctionName_;
    NFmiInfoData::Type type_ = NFmiInfoData::kNoDataType;
    bool usedWithVerticalFunction_ = false;
};

// MacroParamDataChecker is used to check from given macroParam formula that
// which dataIdents (par+prod) are used there. With this information check
// if given queryData contains those dataIdents. 
// This can be used to check if there is need to do updates on map-views when
// new query-data is loaded to the system.
class MacroParamDataChecker
{
public:
    MacroParamDataChecker();

    std::vector<MacroParamDataInfo> getCalculationParametersFromMacroPram(const std::string &macroParamText);

private:
    std::vector<MacroParamDataInfo> getCalculationParametersFromFormula(const std::string &macroParamFormula);
};
