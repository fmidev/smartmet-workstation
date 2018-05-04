#include "MacroParamDataChecker.h"
#include "NFmiCommentStripper.h"
#include "NFmiSmartToolIntepreter.h"
#include "NFmiAreaMaskInfo.h"

#include "boost/algorithm/string.hpp"

MacroParamDataChecker::MacroParamDataChecker()
{
}

std::vector<MacroParamDataInfo> MacroParamDataChecker::getCalculationParametersFromMacroPram(const std::string &macroParamText)
{
    NFmiCommentStripper commentStripper;
    commentStripper.SetString(macroParamText);
    commentStripper.Strip();
    return getCalculationParametersFromFormula(commentStripper.GetString());
}

// It's useful to check only words with underscores, because they could have the needed parmeter and producer information (like T_ec)
static bool containsUnderScore(const std::string &word)
{
    if(word.empty())
        return false;
    return word.find('_') != std::string::npos;
}

static bool containsStringCaseInsensitive(const std::string &searchThis, const std::string &findThis)
{
    auto findRange = boost::algorithm::ifind_first(searchThis, findThis);
    return !findRange.empty();
}

// Fearly easily recognizable vertical functions are like: vertp_max(par, p1, p2).
// So previousActualWord would be vertp_max and parameter must be alone before first comma separator.
// We can't interpret maxz type functions easily, because they are given like this: maxz(h1, h2, formula). 
// There are two parameters between function name and formula. And formula can contain any number of parameters.
// That's why we will skip max/min/avg/sum-z functions all together.
static bool wasPreviousWordVerticalFuction(const std::string &previousActualWord)
{
    // There has to be 'vert' in function name, because there are also non-vertical functions in this function map.
    if(containsStringCaseInsensitive(previousActualWord, "vert"))
    {
        std::string lowerCaseName = previousActualWord;
        boost::algorithm::to_lower(lowerCaseName);
        auto &vertFunctions = NFmiSmartToolIntepreter::GetTokenVertFunctions();
        auto iter = std::find_if(vertFunctions.begin(), vertFunctions.end(), [&lowerCaseName](const auto &functionItem) {return functionItem.first == lowerCaseName; });
        if(iter != vertFunctions.end())
            return true;
    }
    return false;
}

static bool checkPossibleVariable(const std::string &possibleVariableWord, MacroParamDataInfo &macroParamDataInfo, const std::string &previousActualWord)
{
    boost::shared_ptr<NFmiAreaMaskInfo> maskInfo(new NFmiAreaMaskInfo());
    if(NFmiSmartToolIntepreter::InterpretVariableForChecking(possibleVariableWord, maskInfo))
    {
        macroParamDataInfo.variableName_ = possibleVariableWord;
        macroParamDataInfo.dataIdent_ = maskInfo->GetDataIdent();
        if(maskInfo->GetLevel())
            macroParamDataInfo.level_ = *(maskInfo->GetLevel());
        if(::wasPreviousWordVerticalFuction(previousActualWord))
        {
            macroParamDataInfo.usedWithVerticalFunction_ = true;
            macroParamDataInfo.possibleVerticalFunctionName_ = previousActualWord;
        }
        return true;
    }
    return false;
}

// macroParamFormula has no comments, just pure macroParam formula.
std::vector<MacroParamDataInfo> MacroParamDataChecker::getCalculationParametersFromFormula(const std::string &macroParamFormula)
{
    std::string delimiters = NFmiSmartToolIntepreter::GetFullDelimiterChars();
    std::vector<std::string> words;
    boost::split(words, macroParamFormula, boost::is_any_of(delimiters));

    // Store pervious non-empty word here, this is used to find out, if current variable is used in any vertical functions
    std::string previousActualWord;
    std::vector<MacroParamDataInfo> usedParamList;
    for(const auto &word : words)
    {
        if(containsUnderScore(word))
        {
            MacroParamDataInfo macroParamDataInfo;
            if(::checkPossibleVariable(word, macroParamDataInfo, previousActualWord))
            {
                usedParamList.push_back(macroParamDataInfo);
            }
            previousActualWord = word;
        }
    }

    return usedParamList;
}
