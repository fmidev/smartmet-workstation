#include "NFmiSymbolTextMapping.h"
#include "NFmiStringTools.h"
#include "NFmiCommentStripper.h"
#include "catlog/catlog.h"

#include "boost/algorithm/string.hpp"

namespace
{
    std::string MakeDifferentDimensionsWithMultiParameters(const std::string& reporter, const std::string& object1, int dimensions1, const std::string& object2, int dimensions2)
    {
        std::string errorStr = reporter;
        errorStr += ": Parameter count different with current ";
        errorStr += object1;
        errorStr += " (";
        errorStr += std::to_string(dimensions1);
        errorStr += ") and given ";
        errorStr += object2;
        errorStr += " (";
        errorStr += std::to_string(dimensions2);
        errorStr += ")";

        return errorStr;
    }

}

// ***********************************************************
// ***********  NFmiSingleValueText  *************************
// ***********************************************************

NFmiRangedValueText::NFmiRangedValueText() = default;

NFmiRangedValueText::NFmiRangedValueText(float value1, float value2, const std::string& valueText, bool openStart, bool openEnd)
:value1_(value1)
,value2_(value2)
,valueText_(valueText)
,openStart_(openStart)
,openEnd_(openEnd)
{}

bool NFmiRangedValueText::parseRangedValue(const std::string& strippedValue, const std::string& strippedValueText, bool allowSingleValue)
{
    auto startChar = strippedValue.front();
    auto endChar = strippedValue.back();
    if((startChar == '[' || startChar == ']') && (endChar == '[' || endChar == ']'))
    {
        openStart_ = startChar == ']';
        openEnd_ = endChar == '[';
        std::string strippedValueStr(strippedValue.begin()+1, strippedValue.end()-1);
        if(strippedValueStr.empty())
            throw std::runtime_error("Value part was empty after '[' and ']' characters were removed");

        auto valueParts = NFmiStringTools::Split(strippedValueStr, ",");
        if(valueParts.size() == 1 && allowSingleValue)
        {
            // Laitetaan molempiin rajoihin vain se yksi ja sama arvo
            value2_ = value1_ = std::stof(valueParts[0]);
        }
        else
        {
            if(valueParts.size() != 2)
                throw std::runtime_error("Value part didn't have 2 comma (',') separated values");

            value1_ = std::stof(valueParts[0]);
            value2_ = std::stof(valueParts[1]);
        }

        if(value1_ == kFloatMissing || value2_ == kFloatMissing)
            throw std::runtime_error("In ranged setting either of comma separated values can't be missing values (32700)");

        valueText_ = strippedValueText;
        return true;
    }
    else
        throw std::runtime_error("Value part didn't contain correct '[' and ']' characters");
}

bool NFmiRangedValueText::isMatch(float value) const
{
    if(openStart_ && openEnd_)
    {
        return value > value1_ && value < value2_;
    }
    else if(openStart_)
    {
        return value > value1_ && value <= value2_;
    }
    else if(openEnd_)
    {
        return value >= value1_ && value < value2_;
    }
    else
    {
        return value >= value1_ && value <= value2_;
    }
}

const std::string& NFmiRangedValueText::valueText() const
{
    return valueText_;
}

// ***********************************************************
// ***********  NFmiSymbolTextMapping  ***********************
// ***********************************************************

NFmiSymbolTextMapping::NFmiSymbolTextMapping() = default;

bool NFmiSymbolTextMapping::initialize(const std::string &totalFilePath)
{
    return initialize_impl(totalFilePath);
}

bool NFmiSymbolTextMapping::wasInitializationOk() const
{
    return initializationMessage_.empty();
}

std::string NFmiSymbolTextMapping::getSymbolText(float symbolValue) const
{
    // Haku tehd‰‰n seuraavalla priorisoinnilla:
    // 1) Jos initialisointi on ep‰onnistunut, palautetaan aina varoituksena initialisoinnissa syntynyt virheilmoitus.
    if(!wasInitializationOk())
        return initializationMessage_;

    // 2) Missing arvon teksti.
    if(symbolValue == kFloatMissing)
    {
        return missingValueText_;
    }

    // 3) Single value teksti, jos sellainen lˆytyy singleValueTextMap_:ista.
    auto singleValueIter = singleValueTextMap_.find(symbolValue);
    if(singleValueIter != singleValueTextMap_.end())
        return singleValueIter->second;

    // 4) Ranged value teksti, jos sellainen arvov‰li lˆytyy rangedValueTextList_:ista.
    auto rangedValueIter = std::find_if(rangedValueTextList_.begin(), rangedValueTextList_.end(),
        [=](const auto& rangedValueText) {return rangedValueText.isMatch(symbolValue); });
    if(rangedValueIter != rangedValueTextList_.end())
        return rangedValueIter->valueText();

    // 5) Default value, oli se tyhj‰ tai ei.
    return defaultValueText_;
}

bool NFmiSymbolTextMapping::hasAnyValues() const
{
    if(!defaultValueText_.empty())
        return true;
    if(!missingValueText_.empty())
        return true;
    if(!singleValueTextMap_.empty())
        return true;
    if(!rangedValueTextList_.empty())
        return true;

    return false;
}

// Seuraavat 'vanhan' tyyliset line formaatit ovat ok:
// 1. key-value;string-value  ==> lineParts.size() == 2
// 2. key-value;string-value; (eli puolipiste on rivin lopussa)  ==> lineParts.size() == 3 and lineParts[2] on tyhj‰ stringi
// Palauta true, jos saatiin parsittua arvo pari
// Palauta false, jos ei ollut parsittavaa
// Heit‰ poikkeus, jos parsiminen meni pieleen
bool NFmiSymbolTextMapping::parseLineOldStyle(const std::string& line)
{
    auto strippedLine = line;
    NFmiStringTools::TrimAll(strippedLine);
    if(strippedLine.empty())
        return false;
    auto lineParts = NFmiStringTools::Split(strippedLine, ";");
    if(lineParts.size() == 2 || (lineParts.size() == 3 && lineParts[2].empty()))
    {
        float value = std::stof(lineParts[0]);
        auto strippedValueText = lineParts[1];
        NFmiStringTools::TrimAll(strippedValueText);
        singleValueTextMap_.insert(std::make_pair(value, strippedValueText));
        return true;
    }
    throw std::runtime_error("Line didn't match old style format");
}

const std::string g_DefaultValueStr = "default";
const std::string g_MissValueStr1 = "miss";
const std::string g_MissValueStr2 = "missing";

// Seuraavat 'uuden' tyyliset line formaatit ovat ok:
// 1. value = valueText
// 2. default = valueText
// 3. miss/missing = valueText
// 4. [value1,value2] = valueText    (suljettu-raja1,suljettu-raja2)
// 5. [value1,value2[ = valueText    (suljettu-raja1,avoin-raja2)
// 6. ]value1,value2] = valueText    (avoin-raja1,suljettu-raja2)
// 7. ]value1,value2[ = valueText    (avoin-raja1,avoin-raja2)
// Palauta true, jos saatiin parsittua arvo pari
// Palauta false, jos ei ollut parsittavaa
// Heit‰ poikkeus, jos parsiminen meni pieleen
bool NFmiSymbolTextMapping::parseLineNewStyle(const std::string& line)
{
    auto strippedLine = line;
    NFmiStringTools::TrimAll(strippedLine);
    if(strippedLine.empty())
        return false;
    auto lineParts = NFmiStringTools::Split(strippedLine, "=");
    if(lineParts.size() == 2)
    {
        auto strippedValue = lineParts[0];
        NFmiStringTools::TrimAll(strippedValue);
        if(strippedValue.empty())
            throw std::runtime_error("Line didn't have actual value part (value=text)");
        auto strippedValueText = lineParts[1];
        NFmiStringTools::TrimAll(strippedValueText);
        if(strippedValueText.empty())
            throw std::runtime_error("Line didn't have actual text part (value=text)");

        if(boost::iequals(strippedValue, g_DefaultValueStr))
        {
            defaultValueText_ = strippedValueText;
            return true;
        }
        if(boost::iequals(strippedValue, g_MissValueStr1) || boost::iequals(strippedValue, g_MissValueStr2))
        {
            missingValueText_ = strippedValueText;
            return true;
        }

        return doFinalParseLineNewStyle(strippedValue, strippedValueText);
    }
    throw std::runtime_error("Line didn't match new style format");
}

bool NFmiSymbolTextMapping::doFinalParseLineNewStyle(const std::string& strippedValue, const std::string& strippedValueText)
{
    try
    {
        float value = std::stof(strippedValue);
        singleValueTextMap_.insert(std::make_pair(value, strippedValueText));
        return true;
    }
    catch(...)
    { }

    NFmiRangedValueText rangedValueText;
    if(rangedValueText.parseRangedValue(strippedValue, strippedValueText, false))
    {
        rangedValueTextList_.push_back(rangedValueText);
        return true;
    }

    return false;
}

void NFmiSymbolTextMapping::parseLine(const std::string& line, const std::string& totalFilePath)
{
    // Yritet‰‰n ensin old-style parserointia (heitt‰‰ poikkeuksen, jos jokin meni pieleen)
    try
    {
        if(parseLineOldStyle(line))
            return;
    }
    catch(...)
    { }

    // Yritet‰‰n sitten uuden tyylisi‰ formaatteja
    try
    {
        parseLineNewStyle(line);
        return;
    }
    catch(std::exception &e)
    {
        std::string parsingErrorMessage = "Unable to parse line: '";
        parsingErrorMessage += line;
        parsingErrorMessage += "', error message: ";
        parsingErrorMessage += e.what();
        parsingErrorMessage += ", in file: ";
        parsingErrorMessage += totalFilePath;
        CatLog::logMessage(parsingErrorMessage, CatLog::Severity::Warning, CatLog::Category::Configuration);
    }
}

bool NFmiSymbolTextMapping::initialize_impl(const std::string &totalFilePath)
{
    initializationMessage_.clear();
    totalFilePath_ = totalFilePath;
    singleValueTextMap_.clear();
    rangedValueTextList_.clear();
    NFmiCommentStripper commentStripper;
    if(commentStripper.ReadAndStripFile(totalFilePath_))
    {
        std::istringstream in(commentStripper.GetString());
        std::string line;
        do
        {
            std::getline(in, line);
            parseLine(line, totalFilePath);
        } while(in);

        if(hasAnyValues())
            return true;
        else
            initializationMessage_ = std::string("Unable to parse any viable symbol text mappings from file: ") + totalFilePath_;
    }
    else
    {
        initializationMessage_ = std::string("Unable to read symbol text file: ") + totalFilePath_;
    }

    return false;
}

// ***********************************************************
// ***********  MultiParamRangedValueText  *******************
// ***********************************************************

MultiParamRangedValueText::MultiParamRangedValueText() = default;

MultiParamRangedValueText::MultiParamRangedValueText(const std::vector<NFmiRangedValueText>& multiParamRanges,
    const std::string& valueText)
    :multiParamRanges_(multiParamRanges)
    ,valueText_(valueText)
{}

bool MultiParamRangedValueText::isMatch(const std::vector<float>& multiParamValues) const
{
    if(multiParamRanges_.size() != multiParamValues.size())
    {
        std::string errorStr = ::MakeDifferentDimensionsWithMultiParameters("MultiParamRangedValueText", "multiParamRanges", (int)multiParamRanges_.size(), "multiParamValues vector", (int)multiParamValues.size());
        throw std::runtime_error(errorStr);
    }

    for(size_t index = 0; index < multiParamRanges_.size(); index++)
    {
        if(!multiParamRanges_[index].isMatch(multiParamValues[index]))
        {
            return false;
        }
    }
    return true;
}

// ***********************************************************
// ***********  NFmiMultiParamTextMapping  *******************
// ***********************************************************


NFmiMultiParamTextMapping::NFmiMultiParamTextMapping() = default;

bool NFmiMultiParamTextMapping::initialize(const std::string& totalFilePath)
{
    return initialize_impl(totalFilePath);
}

bool NFmiMultiParamTextMapping::wasInitializationOk() const
{
    return initializationMessage_.empty();
}

std::string NFmiMultiParamTextMapping::getSymbolText(const std::vector<float>& multiParamValues) const
{
    // Haku tehd‰‰n seuraavalla priorisoinnilla:

    // 1) Jos initialisointi on ep‰onnistunut, palautetaan aina varoituksena initialisoinnissa syntynyt virheilmoitus.
    if(!wasInitializationOk())
        return initializationMessage_;

    // 2) Tarkistetaan onko multi-param count sama (multimaptooltip tiedostosta luetttu 
    // taulukkokoko vs multiParamValues vektorin koko).
    if(multiParamCount_ != multiParamValues.size())
    {
        std::string errorStr = ::MakeDifferentDimensionsWithMultiParameters("NFmiMultiParamTextMapping", "MultiParamTextMapping", multiParamCount_, "multiParamValues vector", (int)multiParamValues.size());
        throw std::runtime_error(errorStr);
    }

    // 3) Missing arvon teksti.
    if(allMissingValues(multiParamValues))
    {
        return missingValueText_;
    }

    // 4) Ranged value teksti, jos sellainen arvov‰li lˆytyy rangedValueTextList_:ista.
    auto multiParamRangedValueIter = std::find_if(multiParamRangedValueTextList_.begin(), multiParamRangedValueTextList_.end(),
        [=](const auto& multiParamRangedValueText) {return multiParamRangedValueText.isMatch(multiParamValues); });
    if(multiParamRangedValueIter != multiParamRangedValueTextList_.end())
        return multiParamRangedValueIter->valueText();

    // 5) Default value, oli se tyhj‰ tai ei.
    return defaultValueText_;
}

bool NFmiMultiParamTextMapping::allMissingValues(const std::vector<float>& multiParamValues) const
{
    auto nonMissingValueIter = std::find_if(multiParamValues.begin(), multiParamValues.end(),
        [](const auto& paramValue) {return paramValue != kFloatMissing; });
    return nonMissingValueIter == multiParamValues.end();
}

bool NFmiMultiParamTextMapping::hasAnyValues() const
{
    if(!defaultValueText_.empty())
        return true;
    if(!missingValueText_.empty())
        return true;
    if(!multiParamRangedValueTextList_.empty())
        return true;

    return false;
}

static std::pair<std::string, std::string> SplitStringByFirstOccuranceOfDelimiter(const std::string& splittedString, char delimiter)
{
    auto pos = splittedString.find(delimiter);
    if(pos == std::string::npos)
    {
        std::string errorString = "Multi-param set line didn't have assingment '=' in it, use correct format e.g. [1,5][3,4] = tooltip-text)";
        throw std::runtime_error(errorString);
    }
    auto startStr = splittedString.substr(0, pos);
    auto endStr = splittedString.substr(pos + 1, std::string::npos);
    return std::make_pair(startStr, endStr);
}

void NFmiMultiParamTextMapping::parseLine(const std::string& line, const std::string& totalFilePath)
{
    try
    {
        auto strippedLine = line;
        NFmiStringTools::TrimAll(strippedLine);
        if(strippedLine.empty())
            return;
        auto lineParts = ::SplitStringByFirstOccuranceOfDelimiter(strippedLine, '=');
        auto strippedValuePart = lineParts.first;
        NFmiStringTools::TrimAll(strippedValuePart);
        if(strippedValuePart.empty())
            throw std::runtime_error("Line didn't have actual value-part (value-part = text-part)");
        auto strippedTextPart = lineParts.second;
        NFmiStringTools::TrimAll(strippedTextPart);
        if(strippedTextPart.empty())
            throw std::runtime_error("Line didn't have actual text-part (value-part = text-part)");

        if(boost::iequals(strippedValuePart, g_DefaultValueStr))
        {
            defaultValueText_ = strippedValuePart;
            return;
        }
        if(boost::iequals(strippedValuePart, g_MissValueStr1) || boost::iequals(strippedValuePart, g_MissValueStr2))
        {
            missingValueText_ = strippedTextPart;
            return;
        }

        doFinalParseLine(strippedValuePart, strippedTextPart);
    }
    catch(std::exception& e)
    {
        std::string parsingErrorMessage = "Unable to parse line: '";
        parsingErrorMessage += line;
        parsingErrorMessage += "', error message: ";
        parsingErrorMessage += e.what();
        parsingErrorMessage += ", in file: ";
        parsingErrorMessage += totalFilePath;
        CatLog::logMessage(parsingErrorMessage, CatLog::Severity::Warning, CatLog::Category::Configuration);
    }
}

static std::vector<std::string> GetRangeStrings(const std::string& strippedValuePart)
{
    std::string rangeMarkers = "[]";
    size_t startingOffset = 0;
    std::vector<std::string> rangeStrings;
    for(;;)
    {
        // Etsi seuraavan rangen alku
        auto rangeStartPos = strippedValuePart.find_first_of(rangeMarkers, startingOffset);
        if(rangeStartPos == std::string::npos)
            break;
        // Etsi seuraavan rangen loppu
        auto rangeEndPos = strippedValuePart.find_first_of(rangeMarkers, rangeStartPos + 1);
        if(rangeEndPos == std::string::npos)
        {
            throw std::runtime_error("Uneven number of multi-param range-block markers ([ and ] characters)");
        }
        auto usedLength = rangeEndPos - rangeStartPos + 1;
        rangeStrings.push_back(std::string(strippedValuePart, rangeStartPos, usedLength));
        startingOffset += usedLength;
    }
    return rangeStrings;
}

static std::vector<NFmiRangedValueText> GetMultiParamRanges(const std::string& strippedValuePart)
{
    // Jaetaan value-part n kpl rangeja
    auto rangeStrings = ::GetRangeStrings(strippedValuePart);
    std::vector<NFmiRangedValueText> ranges;
    for(const auto& rangeStr : rangeStrings)
    {
        NFmiRangedValueText rangedValueText;
        if(rangedValueText.parseRangedValue(rangeStr, "", true))
        {
            ranges.push_back(rangedValueText);
        }
    }
    return ranges;
}

void NFmiMultiParamTextMapping::doFinalParseLine(const std::string& strippedValuePart, const std::string& strippedTextPart)
{
    auto multiParamRanges = ::GetMultiParamRanges(strippedValuePart);
    if(multiParamCount_ != 0 && multiParamCount_ != (int)multiParamRanges.size())
    {
        std::string errorString = "Multi-param set line had different dimension size (";
        errorString += std::to_string(multiParamRanges.size());
        errorString += ") than previously defined lines (";
        errorString += std::to_string(multiParamCount_);
        errorString += ")";
        throw std::runtime_error(errorString);
    }
    multiParamRangedValueTextList_.push_back(MultiParamRangedValueText(multiParamRanges, strippedTextPart));
    if(multiParamCount_ == 0)
    {
        multiParamCount_ = (int)multiParamRanges.size();
    }
}

bool NFmiMultiParamTextMapping::initialize_impl(const std::string& totalFilePath)
{
    initializationMessage_.clear();
    totalFilePath_ = totalFilePath;
    multiParamRangedValueTextList_.clear();
    NFmiCommentStripper commentStripper;
    if(commentStripper.ReadAndStripFile(totalFilePath_))
    {
        std::istringstream in(commentStripper.GetString());
        std::string line;
        do
        {
            std::getline(in, line);
            parseLine(line, totalFilePath);
        } while(in);

        if(hasAnyValues())
            return true;
        else
            initializationMessage_ = std::string("Unable to parse any viable multi-param text mappings from file: ") + totalFilePath_;
    }
    else
    {
        initializationMessage_ = std::string("Unable to read multi-param text mapping file: ") + totalFilePath_;
    }

    return false;
}

// ***********************************************************
// ***********  NFmiSymbolTextMappingCache  ******************
// ***********************************************************

NFmiSymbolTextMappingCache::NFmiSymbolTextMappingCache() = default;

std::string NFmiSymbolTextMappingCache::getPossibleMacroParamSymbolText(float value, const std::string &possibleSymbolTooltipFile)
{
    if(!possibleSymbolTooltipFile.empty())
    {
        std::string valueStr;
        // Katsotaan lˆytyykˆ haluttu tiedosto jo luettuna cache:en
        auto iter = symbolTextMapCache_.find(possibleSymbolTooltipFile);
        if(iter != symbolTextMapCache_.end())
            valueStr += iter->second.getSymbolText(value);
        else
        {
            // Jos ei lˆytynyt, luodaan uusi cache otus, alustetaan se ja palautetaan siit‰ haluttu arvo
            auto iter = symbolTextMapCache_.insert(std::make_pair(possibleSymbolTooltipFile, NFmiSymbolTextMapping()));
            iter.first->second.initialize(possibleSymbolTooltipFile);
            valueStr += iter.first->second.getSymbolText(value);
        }
        return valueStr;
    }
    return "";
}

std::string NFmiSymbolTextMappingCache::getPossibleMacroParamMultiParamText(const std::vector<float>& multiParamValues, const std::string& possibleMultiParamTooltipFile)
{
    if(!possibleMultiParamTooltipFile.empty())
    {
        std::string valueStr;
        // Katsotaan lˆytyykˆ haluttu tiedosto jo luettuna cache:en
        auto iter = multiParamTextMapCache_.find(possibleMultiParamTooltipFile);
        if(iter != multiParamTextMapCache_.end())
            valueStr += iter->second.getSymbolText(multiParamValues);
        else
        {
            // Jos ei lˆytynyt, luodaan uusi cache otus, alustetaan se ja palautetaan siit‰ haluttu arvo
            auto iter = multiParamTextMapCache_.insert(std::make_pair(possibleMultiParamTooltipFile, NFmiMultiParamTextMapping()));
            iter.first->second.initialize(possibleMultiParamTooltipFile);
            valueStr += iter.first->second.getSymbolText(multiParamValues);
        }
        return valueStr;
    }
    return "";
}
