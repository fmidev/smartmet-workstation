#pragma once

#include "NFmiGlobals.h"
#include <string>
#include <map>
#include <list>

class NFmiRangedValueText
{
    float value1_ = kFloatMissing;
    float value2_ = kFloatMissing;
    std::string valueText_;
    bool openStart_ = false;
    bool openEnd_ = false;
public:
    NFmiRangedValueText();
    NFmiRangedValueText(float value1, float value2, const std::string &valueText, bool openStart, bool openEnd);

    bool parseRangedValue(const std::string& strippedValue, const std::string& strippedValueText);
    bool isMatch(float value) const;
    const std::string& valueText() const;
};


class NFmiSymbolTextMapping
{
    std::string totalFilePath_;
    std::map<float, std::string> singleValueTextMap_;
    std::list<NFmiRangedValueText> rangedValueTextList_;
    std::string missingValueText_ = "No 'missing' value text";
    std::string defaultValueText_ = "No 'default' value text";
    std::string initializationMessage_;
public:
    NFmiSymbolTextMapping();

    bool initialize(const std::string& totalFilePath);
    bool wasInitializationOk() const;
    std::string getSymbolText(float symbolValue) const;
    bool hasAnyValues() const;

private:
    void parseLine(const std::string& line, const std::string& totalFilePath);
    bool parseLineOldStyle(const std::string& line);
    bool parseLineNewStyle(const std::string& line);
    bool doFinalParseLineNewStyle(const std::string& strippedValue, const std::string& strippedValueText);
    bool initialize_impl(const std::string& totalFilePath);
};

class NFmiSymbolTextMappingCache
{
    std::map<std::string, NFmiSymbolTextMapping> symbolTextMapCache_;
public:
    NFmiSymbolTextMappingCache();
    std::string getPossibleMacroParamSymbolText(float value, const std::string& possibleSymbolTooltipFile);
};
