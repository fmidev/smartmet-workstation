#include "NFmiSymbolTextMapping.h"
#include "NFmiStringTools.h"
#include "NFmiCommentStripper.h"
#include "catlog/catlog.h"

#include "boost/algorithm/string.hpp"

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

bool NFmiRangedValueText::parseRangedValue(const std::string& strippedValue, const std::string& strippedValueText)
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
        if(valueParts.size() != 2)
            throw std::runtime_error("Value part didn't have 2 comma (',') separated values");

        value1_ = std::stof(valueParts[0]);
        value2_ = std::stof(valueParts[1]);
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
    // Haku tehdään seuraavalla priorisoinnilla:
    // 1) Jos initialisointi on epäonnistunut, palautetaan aina varoituksena initialisoinnissa syntynyt virheilmoitus.
    if(!wasInitializationOk())
        return initializationMessage_;

    // 2) Missing arvon teksti.
    if(symbolValue == kFloatMissing)
    {
        if(missingValueText_.empty())
        {
            static const std::string defaultMissingValueText = "missing value";
            return defaultMissingValueText;
        }
        else
            return missingValueText_;
    }

    // 3) Single value teksti, jos sellainen löytyy singleValueTextMap_:ista.
    auto singleValueIter = singleValueTextMap_.find(symbolValue);
    if(singleValueIter != singleValueTextMap_.end())
        return singleValueIter->second;

    // 4) Ranged value teksti, jos sellainen arvoväli löytyy rangedValueTextList_:ista.
    auto rangedValueIter = std::find_if(rangedValueTextList_.begin(), rangedValueTextList_.end(),
        [=](const auto& rangedValueText) {return rangedValueText.isMatch(symbolValue); });
    if(rangedValueIter != rangedValueTextList_.end())
        return rangedValueIter->valueText();

    // 5) Default value, oli se tyhjä tai ei.
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
// 2. key-value;string-value; (eli puolipiste on rivin lopussa)  ==> lineParts.size() == 3 and lineParts[2] on tyhjä stringi
// Palauta true, jos saatiin parsittua arvo pari
// Palauta false, jos ei ollut parsittavaa
// Heitä poikkeus, jos parsiminen meni pieleen
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
// Heitä poikkeus, jos parsiminen meni pieleen
bool NFmiSymbolTextMapping::parseLineNewStyle(const std::string& line)
{
    const std::string defaultValueStr = "default";
    const std::string missValueStr1 = "miss";
    const std::string missValueStr2 = "missing";

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

        if(boost::iequals(strippedValue, defaultValueStr))
        {
            defaultValueText_ = strippedValueText;
            return true;
        }
        if(boost::iequals(strippedValue, missValueStr1) || boost::iequals(strippedValue, missValueStr2))
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
    if(rangedValueText.parseRangedValue(strippedValue, strippedValueText))
    {
        rangedValueTextList_.push_back(rangedValueText);
        return true;
    }

    return false;
}

void NFmiSymbolTextMapping::parseLine(const std::string& line, const std::string& totalFilePath)
{
    // Yritetään ensin old-style parserointia (heittää poikkeuksen, jos jokin meni pieleen)
    try
    {
        if(parseLineOldStyle(line))
            return;
    }
    catch(...)
    { }

    // Yritetään sitten uuden tyylisiä formaatteja
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
// ***********  NFmiSymbolTextMappingCache  ******************
// ***********************************************************

NFmiSymbolTextMappingCache::NFmiSymbolTextMappingCache() = default;

std::string NFmiSymbolTextMappingCache::getPossibleMacroParamSymbolText(float value, const std::string &possibleSymbolTooltipFile)
{
    if(!possibleSymbolTooltipFile.empty())
    {
        std::string str = " (";
        // Katsotaan löytyykö haluttu tiedosto jo luettuna cache:en
        auto iter = symbolTextMapCache_.find(possibleSymbolTooltipFile);
        if(iter != symbolTextMapCache_.end())
            str += iter->second.getSymbolText(value);
        else
        {
            // Jos ei löytynyt, luodaan uusi cache otus, alustetaan se ja palautetaan siitä haluttu arvo
            auto iter = symbolTextMapCache_.insert(std::make_pair(possibleSymbolTooltipFile, NFmiSymbolTextMapping()));
            iter.first->second.initialize(possibleSymbolTooltipFile);
            str += iter.first->second.getSymbolText(value);
        }
        str += ")";
        return str;
    }
    return "";
}

