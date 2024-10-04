#include "NFmiMacroParamDataGenerator.h"
#include "NFmiPathUtils.h"
#include "NFmiSmartToolIntepreter.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiExtraMacroParamData.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiParamDescriptor.h"
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiFileSystem.h"
#include "NFmiPathUtils.h"
#include "NFmiSmartInfo.h"
#include "NFmiBetaProductSystem.h"
#include "NFmiTimeList.h"
#include "jsonutils.h"
#include "catlog/catlog.h"
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <regex>
#include <numeric>
#include <thread>

namespace
{
    bool DirectoryExists(const std::string& directoryPath)
    {
        return (std::filesystem::exists(directoryPath) && std::filesystem::is_directory(directoryPath));
    }

    bool FileExists(const std::string& filePath)
    {
        return (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath));
    }

    std::string GetFileExtension(const std::string& filePath)
    {
        std::filesystem::path originalPath(filePath);
        return originalPath.extension().string();
    }

    NFmiParam MakeWantedParam(const std::string& part1, const std::string& part2)
    {
        unsigned long paramId = std::stoul(part1);
        return NFmiParam(paramId, boost::trim_copy(part2), kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly);
    }

    std::vector<std::string> GetSplittedAndTrimmedStrings(const std::string& str, const std::string& splitter)
    {
        std::vector<std::string> rawParts;
        boost::split(rawParts, str, boost::is_any_of(splitter));
        std::vector<std::string> parts;
        for(auto& part : rawParts)
        {
            boost::trim(part);
            if(!part.empty())
                parts.push_back(part);
        }
        return parts;
    }

    // Tarkistaa onko inputissa joko pelkkä T_ec tyyppinen teksti
    // tai vaihtoehtoisesti siinä voi olla vaikka T_ec[0.5h]
    bool DoParamDataCheckWithOptionalDelayString(const std::string& paramDataStr)
    {
        // Regular expression pattern to match any string (a-z,0-9,_) optionally followed by a bracketed number with h or H
        std::regex pattern(R"(^[A-Za-z0-9_]+(\[([0-9]*\.?[0-9]+[hH])\])?$)");
        return std::regex_match(paramDataStr, pattern);
    }

    void CleanFilePattern(const std::string& theFilePattern, int keepMaxFiles)
    {
        try
        {
            std::string logMessageStart("NFmiMacroParamDataGenerator-CleanFilePattern: ");
            logMessageStart += theFilePattern;
            logMessageStart += ",\nkeep-max-files = ";
            logMessageStart += std::to_string(keepMaxFiles);
            logMessageStart += ",\nDeleted files:\n";
            CtrlViewUtils::DeleteFilesWithPatternAndLog(theFilePattern, logMessageStart, CatLog::Severity::Debug, CatLog::Category::Data, keepMaxFiles);
        }
        catch(...)
        {
            // ei tehdä toistaiseksi mitään...
        }
    }
}

// ***********************************************************
// ************** NFmiMacroParamDataInfo *********************
// ***********************************************************

NFmiMacroParamDataInfo::NFmiMacroParamDataInfo() = default;

NFmiMacroParamDataInfo::NFmiMacroParamDataInfo(const std::string& baseDataParamProducerLevelString, const std::string& usedProducerString, const std::string& dataGeneratingSmarttoolPathString, const std::string& usedParameterListString, const std::string& dataStorageFileFilter, const std::string& dataTriggerList, int maxGeneratedFilesKept)
:mBaseDataParamProducerString(baseDataParamProducerLevelString)
,mUsedProducerString(usedProducerString)
,mDataGeneratingSmarttoolPathString(dataGeneratingSmarttoolPathString)
,mUsedParameterListString(usedParameterListString)
,mDataStorageFileFilter(dataStorageFileFilter)
,mDataTriggerList(dataTriggerList)
,mMaxGeneratedFilesKept(maxGeneratedFilesKept)
{
    CorrectMaxGeneratedFilesKeptValue();
}

void NFmiMacroParamDataInfo::CorrectMaxGeneratedFilesKeptValue()
{
    mMaxGeneratedFilesKept = FixMaxGeneratedFilesKeptValue(mMaxGeneratedFilesKept);
}

int NFmiMacroParamDataInfo::FixMaxGeneratedFilesKeptValue(int newValue)
{
    if(newValue < 1)
        return 1;
    else if(newValue > 10)
        return 10;

    return newValue;
}

bool NFmiMacroParamDataInfo::CheckData()
{
    {
        auto checkResult = NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(mBaseDataParamProducerString, true);
        if(!checkResult.first.empty())
        {
            itsCheckShortStatusStr = "Base data param-producer error";
            return false;
        }
    }

    {
        auto checkResult = NFmiMacroParamDataInfo::CheckUsedProducerString(mUsedProducerString);
        if(!checkResult.first.empty())
        {
            itsCheckShortStatusStr = "Used producer error";
            return false;
        }
    }

    {
        NFmiProducer dummyProducer;
        auto checkResult = NFmiMacroParamDataInfo::CheckUsedParameterListString(mUsedParameterListString, dummyProducer);
        if(!checkResult.first.empty())
        {
            itsCheckShortStatusStr = "Param list error";
            return false;
        }
    }

    {
        auto checkResult = NFmiMacroParamDataInfo::CheckDataStorageFileFilter(mDataStorageFileFilter);
        if(!checkResult.empty())
        {
            itsCheckShortStatusStr = "Storage file filter error";
            return false;
        }
    }

    {
        auto checkResult = NFmiMacroParamDataInfo::CheckDataGeneratingSmarttoolPathString(mDataGeneratingSmarttoolPathString);
        if(!checkResult.empty())
        {
            itsCheckShortStatusStr = "Smarttool path error";
            return false;
        }
    }

    {
        auto checkResult = NFmiMacroParamDataInfo::CheckDataTriggerListString(mDataTriggerList);
        if(!checkResult.first.empty())
        {
            itsCheckShortStatusStr = "Trigger list error";
            return false;
        }
        mWantedDataTriggerList = checkResult.second;
    }

    itsCheckShortStatusStr = "Macro param data ok";
    return true;
}

std::string NFmiMacroParamDataInfo::MakeDataStorageFilePath(const std::string& dataStorageFileFilter)
{
    // Tehdään lokaali ajasta aikaleima, jossa mukana myös sekunnit
    NFmiTime makeTime;
    std::string timeStampStr = makeTime.ToStr(kYYYYMMDDHHMMSS);
    auto finalFilePath = dataStorageFileFilter;
    boost::replace_all(finalFilePath, "*", timeStampStr);
    return finalFilePath;
}

// Perus datan valintaan liittyvan par+prod stringin tarkistus. Palauttaa selityksen virheestä, jos siinä on jotain vikaa.
// Jos kaikki on ok, palauttaa tyhjän stringin.
std::pair<std::string, NFmiDefineWantedData> NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(const std::string& baseDataParamProducerString, bool allowLevelData)
{
    std::string checkStr;
    std::pair<bool, NFmiDefineWantedData> variableDataType;
    try
    {
        variableDataType = NFmiSmartToolIntepreter::CheckForVariableDataType(baseDataParamProducerString);
    } 
    catch(std::exception& e)
    {
        checkStr = "CheckBaseDataParamProducerString: Illegal base data parameter '" + baseDataParamProducerString + "' was given with error '" + e.what() + "'.";
        return std::make_pair(checkStr, variableDataType.second);
    }

    if((!variableDataType.first))
    {
        checkStr = "CheckBaseDataParamProducerString: Given base data parameter '" + baseDataParamProducerString + "' doesn't match any of existing querydata in the system.";
    }
    else if(variableDataType.second.levelPtr_ != nullptr)
    {
        if(!allowLevelData)
        {
            checkStr = "CheckBaseDataParamProducerString: Given base data parameter '" + baseDataParamProducerString + "' did have level information, and level data aren't allowed to be macroParam data.";
        }
    }

    return std::make_pair(checkStr, variableDataType.second);
}

// Producer stringin tarkistus, palauttaa mahdollisen virheilmoituksen ja vectorin, jossa id+name palaset.
std::pair<std::string, std::vector<std::string>> NFmiMacroParamDataInfo::CheckUsedProducerString(const std::string& usedProducerString)
{
    auto parts = ::GetSplittedAndTrimmedStrings(usedProducerString, ",");
    std::string checkStr;
    if(parts.size() < 2)
    {
        checkStr = std::string("CheckUsedProducerString: given string (" + usedProducerString + ") had too few producer parts(< 2), string should be in format '123,name'");
    }

    try
    {
        unsigned long producerId = std::stoul(parts[0]);
    }
    catch(std::exception& )
    {
        checkStr = std::string("CheckUsedProducerString: unable to convert produced id part to integer number in '" + usedProducerString + "', string should be in format of 'producer-id-integer,name-string'");
    }
    return std::make_pair(checkStr, parts);
}

std::pair<std::string, NFmiParamBag> NFmiMacroParamDataInfo::CheckUsedParameterListString(const std::string usedParameterListString, const NFmiProducer& wantedProducer)
{
    auto parts = ::GetSplittedAndTrimmedStrings(usedParameterListString, ",");
    std::string checkStr;
    NFmiParamBag wantedParams;
    if(parts.size() < 2)
    {
        checkStr = std::string("CheckUsedParameterListString: '") + usedParameterListString + "' had too few parameter (list) parts (< 2), should be in format '123,paramName1[,124,paramName2,...]')";
    }
    else
    {
        size_t totalPartCounter = 0;
        for(; totalPartCounter < parts.size() - 1; )
        {
            NFmiParam wantedParam;
            auto paramIdStr = parts[totalPartCounter];
            try
            {
                wantedParam = ::MakeWantedParam(paramIdStr, parts[totalPartCounter + 1]);
            }
            catch(std::exception&)
            {
                checkStr = std::string("CheckUsedParameterListString: '") + usedParameterListString + "' had an illegal parameter id (" + paramIdStr + ") that couldn't be converted to integer)";
                break;
            }
            wantedParams.Add(NFmiDataIdent(wantedParam, wantedProducer));
            totalPartCounter += 2;
        }
    }
    return std::make_pair(checkStr, wantedParams);
}

const std::string gQueryDataFileExtension = ".sqd";

// Talletettavan datan filefiltterin tarkistus. Palauttaa selityksen virheestä, jos siinä on jotain vikaa.
// Jos kaikki on ok, palauttaa tyhjän string:in.
std::string NFmiMacroParamDataInfo::CheckDataStorageFileFilter(const std::string& dataStorageFileFilter)
{
    auto filename = PathUtils::getFilename(dataStorageFileFilter, false);
    // 1. Tiedoston nimessä pitää olla '*' merkki aikaleimaa varten.
    if(filename.find('*') == std::string::npos)
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output filename '" + filename + "' (in " + dataStorageFileFilter + ") doesn't have ' * ' character place marker for file's creation time stamp. To store MacroParam data you need to have that asterisk in filename.";
    }

    // 2. Tiedoston nimessä pitää olla vähintään 3 alpha-numeerista merkkiä, jotta esim. pelkkä path\*.sqd ei kelpaa
    int alnumCount = std::accumulate(filename.begin(), filename.end(), 0,
        [](int total, char c) {
            // Check if the character is alphanumeric (alphabet or number)
            return total + (std::isalnum(c) ? 1 : 0);
        });
    if(alnumCount < 3)
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output filename '" + filename + "' (in " + dataStorageFileFilter + ") doesn't have enough alpha-numeric characters (< 3) and it maybe hazardous when system starts cleaning result data file routines.";
    }

    auto fileExtension = ::GetFileExtension(dataStorageFileFilter);
    // 3. Tiedoston extension pitää olla sqd tyyppinen
    if(!boost::iequals(fileExtension, gQueryDataFileExtension))
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output extension '" + fileExtension + "' (in " + dataStorageFileFilter + ") is invalid, extension must be '" + gQueryDataFileExtension + "' type.";
    }

    return "";
}

const std::string gSmarttoolFileExtension = ".st";

std::string NFmiMacroParamDataInfo::CheckDataGeneratingSmarttoolPathString(const std::string& dataGeneratingSmarttoolPathString)
{
    if(!::FileExists(dataGeneratingSmarttoolPathString))
    {
        auto absoluteSmarttoolPath = PathUtils::getAbsoluteFilePath(dataGeneratingSmarttoolPathString, NFmiMacroParamDataGenerator::RootSmarttoolDirectory());
        if(!::FileExists(absoluteSmarttoolPath))
        {
            return std::string("CheckDataGeneratingSmarttoolPathString") + ": Given smarttool file path '" + dataGeneratingSmarttoolPathString + "' doesn't exists, make sure that given path for MacroParam data generating smarttool was correct!";
        }
    }

    auto fileExtension = ::GetFileExtension(dataGeneratingSmarttoolPathString);
    // 3. Tiedoston extension pitää olla st tyyppinen
    if(!boost::iequals(fileExtension, gSmarttoolFileExtension))
    {
        return std::string("CheckDataGeneratingSmarttoolPathString") + ": Given data output extension '" + fileExtension + "' (in " + dataGeneratingSmarttoolPathString + ") is invalid, extension must be '" + gSmarttoolFileExtension + "' type.";
    }

    return "";
}

std::pair<std::string, std::vector<NFmiDefineWantedData>> NFmiMacroParamDataInfo::CheckDataTriggerListString(const std::string& dataTriggerListString)
{
    const std::vector<NFmiDefineWantedData> dummyEmptyData;

    auto parts = ::GetSplittedAndTrimmedStrings(dataTriggerListString, ",");
    // Tyhjä on ok
    if(parts.empty())
        return std::make_pair("", dummyEmptyData);

    std::vector<NFmiDefineWantedData> triggerDataList;

    for(const auto& paramDataStr : parts)
    {
        auto checkData = CheckBaseDataParamProducerString(paramDataStr, true);
        if(!checkData.first.empty())
        {
            return std::make_pair(checkData.first, dummyEmptyData);
        }

        if(!::DoParamDataCheckWithOptionalDelayString(paramDataStr))
        {
            auto errorString = std::string("CheckDataTriggerListString") + ": Given data parameter '" + paramDataStr + "' (in " + dataTriggerListString + ") had invalid form, should be like 'T_ec' or 'T_ec[0.5h]'";
            return std::make_pair(errorString, dummyEmptyData);
        }
        triggerDataList.push_back(checkData.second);
    }

    return std::make_pair("", triggerDataList);
}

std::string NFmiMacroParamDataInfo::MakeShortStatusErrorString()
{
    CheckData();
    return itsCheckShortStatusStr;
}


static const std::string gJsonName_BaseDataParamProducer = "BaseDataParamProducer";
static const std::string gJsonName_UsedProducer = "UsedProducer";
static const std::string gJsonName_DataGeneratingSmarttoolPath = "DataGeneratingSmarttoolPath";
static const std::string gJsonName_UsedParameterList = "UsedParameterList";
static const std::string gJsonName_DataStorageFileFilter = "DataStorageFileFilter";
static const std::string gJsonName_DataTriggerList = "DataTriggerList";
static const std::string gJsonName_MaxGeneratedFilesKept = "mMaxGeneratedFilesKept";

json_spirit::Object NFmiMacroParamDataInfo::MakeJsonObject(const NFmiMacroParamDataInfo& macroParamDataInfo)
{
    json_spirit::Object jsonObject;
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.BaseDataParamProducerString(), gJsonName_BaseDataParamProducer, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.UsedProducerString(), gJsonName_UsedProducer, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.DataGeneratingSmarttoolPathString(), gJsonName_DataGeneratingSmarttoolPath, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.UsedParameterListString(), gJsonName_UsedParameterList, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.DataStorageFileFilter(), gJsonName_DataStorageFileFilter, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.DataTriggerList(), gJsonName_DataTriggerList, jsonObject);
    jsonObject.push_back(json_spirit::Pair(gJsonName_MaxGeneratedFilesKept, macroParamDataInfo.MaxGeneratedFilesKept()));

    return jsonObject;
}

void NFmiMacroParamDataInfo::ParseJsonPair(json_spirit::Pair& thePair)
{
    // Tässä puretaan NFmiMacroParamDataInfo luokan päätason pareja.
    if(thePair.name_ == gJsonName_BaseDataParamProducer)
        mBaseDataParamProducerString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_UsedProducer)
        mUsedProducerString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_DataGeneratingSmarttoolPath)
        mDataGeneratingSmarttoolPathString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_UsedParameterList)
        mUsedParameterListString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_DataStorageFileFilter)
        mDataStorageFileFilter = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_DataTriggerList)
        mDataTriggerList = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_MaxGeneratedFilesKept)
    {
        mMaxGeneratedFilesKept = thePair.value_.get_int();
        CorrectMaxGeneratedFilesKeptValue();
    }
}

bool NFmiMacroParamDataInfo::StoreInJsonFormat(const NFmiMacroParamDataInfo& macroParamDataInfo, const std::string& theFilePath, std::string& theErrorStringOut)
{
    return JsonUtils::StoreObjectInJsonFormat(macroParamDataInfo, theFilePath, "MacroParamDataInfo", theErrorStringOut);
}

bool NFmiMacroParamDataInfo::ReadInJsonFormat(NFmiMacroParamDataInfo& macroParamDataInfoOut, const std::string& theFilePath, std::string& theErrorStringOut)
{
    return JsonUtils::ReadObjectInJsonFormat(macroParamDataInfoOut, theFilePath, "MacroParamDataInfo", theErrorStringOut);
}

// ********************************************************************
// ***  NFmiMacroParamDataAutomationListItem osio alkaa  *****************
// ********************************************************************

NFmiMacroParamDataAutomationListItem::NFmiMacroParamDataAutomationListItem()
    :fEnable(false)
    , itsMacroParamDataAutomationPath()
    , itsMacroParamDataAutomationAbsolutePath()
    , itsMacroParamDataAutomation()
    , itsLastRunTime(1) // Otetaan viime ajoajaksi nyky utc hetki 1 minuutin tarkkuudella
    , fProductsHaveBeenGenerated(false)
//    , itsNextRunTime(NFmiMetTime::gMissingTime)
{
}

NFmiMacroParamDataAutomationListItem::NFmiMacroParamDataAutomationListItem(const std::string& theBetaAutomationPath)
    :fEnable(false)
    , itsMacroParamDataAutomationPath(theBetaAutomationPath)
    , itsMacroParamDataAutomationAbsolutePath()
    , itsMacroParamDataAutomation()
    , itsLastRunTime(1) // Otetaan viime ajoajaksi nyky utc hetki 1 minuutin tarkkuudella
    , fProductsHaveBeenGenerated(false)
//    , itsNextRunTime(NFmiMetTime::gMissingTime)
{
}

// Tätä kutsutaan kun esim. luetaan data tiedostosta ja tehdään täysi tarkistus kaikille osille
void NFmiMacroParamDataAutomationListItem::DoFullChecks(bool automationModeOn)
{
    itsStatus = MacroParamDataStatus::kFmiListItemOk;
    if(itsMacroParamDataAutomation)
    {
        if(GetErrorStatus() == MacroParamDataStatus::kFmiListItemOk) // Jos automaatio tuote on ok, lasketaan aina seuraava ajoaika valmiiksi
        {
//            itsNextRunTime = itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(itsLastRunTime, automationModeOn);
        }
    }
    else
        itsStatus = MacroParamDataStatus::kFmiListItemReadError;

}

bool NFmiMacroParamDataAutomationListItem::IsEmpty() const
{
    if(itsMacroParamDataAutomation)
        return false;
    else
        return true;
}

std::string NFmiMacroParamDataAutomationListItem::AutomationName() const
{
    try
    {
        return PathUtils::getFilename(itsMacroParamDataAutomationPath, false);
    }
    catch(...)
    {
        return "FileName-error";
    }
}

std::string NFmiMacroParamDataAutomationListItem::ShortStatusText() const
{
    if(itsStatus == MacroParamDataStatus::kFmiListItemReadError)
        return "Automation read error";
    else if(itsStatus == MacroParamDataStatus::kFmiListItemSameAutomationsInList)
        return "Same automation multiple times";
    else if(itsMacroParamDataAutomation)
        return itsMacroParamDataAutomation->CheckShortStatusStr();
    else
        return "Empty-Automation-error";
}

MacroParamDataStatus NFmiMacroParamDataAutomationListItem::GetErrorStatus() const
{
    if(itsStatus == MacroParamDataStatus::kFmiListItemReadError)
        return itsStatus;
    else if(itsMacroParamDataAutomation)
    {
        if(!itsMacroParamDataAutomation->CheckData())
            return MacroParamDataStatus::kFmiListItemAutomationError;
    }

    return itsStatus;
}

std::string NFmiMacroParamDataAutomationListItem::FullAutomationPath() const
{
    return itsMacroParamDataAutomationAbsolutePath;
}


static const std::string gJsonName_MacroParDataAutomationListItemEnable = "Enable";
static const std::string gJsonName_MacroParDataAutomationListItemPath = "MacroParamData-automation path";
static const std::string gJsonName_MacroParDataAutomationListItemAbsolutePath = "MacroParamData-automation absolute path";

// Tämä tallentaa json objektiin vain enable -lipun ja polun käytettyyn MacroParam data -automaatioon. 
// Mutta ei itse MacroParam data -automaatio otusta!!
json_spirit::Object NFmiMacroParamDataAutomationListItem::MakeJsonObject(const NFmiMacroParamDataAutomationListItem& listItem)
{
    NFmiMacroParamDataAutomationListItem defaultListItem;

    json_spirit::Object jsonObject;
    if(defaultListItem.fEnable != listItem.fEnable)
        jsonObject.push_back(json_spirit::Pair(gJsonName_MacroParDataAutomationListItemEnable, listItem.fEnable));
    JsonUtils::AddNonEmptyStringJsonPair(listItem.itsMacroParamDataAutomationPath, gJsonName_MacroParDataAutomationListItemPath, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(listItem.itsMacroParamDataAutomationAbsolutePath, gJsonName_MacroParDataAutomationListItemAbsolutePath, jsonObject);

    return jsonObject;
}

void NFmiMacroParamDataAutomationListItem::ParseJsonPair(json_spirit::Pair& thePair)
{
    if(thePair.name_ == gJsonName_MacroParDataAutomationListItemEnable)
        fEnable = thePair.value_.get_bool();
    else if(thePair.name_ == gJsonName_MacroParDataAutomationListItemPath)
        itsMacroParamDataAutomationPath = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_MacroParDataAutomationListItemAbsolutePath)
        itsMacroParamDataAutomationAbsolutePath = thePair.value_.get_str();
}

// ********************************************************************
// *******  NFmiPostponedMacroParamDataAutomation osio alkaa  *******************
// ********************************************************************

NFmiPostponedMacroParamDataAutomation::NFmiPostponedMacroParamDataAutomation(std::shared_ptr<NFmiMacroParamDataAutomationListItem>& postponedDataTriggeredAutomation, int postponeTimeInMinutes)
    :itsPostponeTimer()
    , itsPostponedDataTriggeredAutomation(postponedDataTriggeredAutomation)
    , itsPostponeTimeInMinutes(postponeTimeInMinutes)
{}

bool NFmiPostponedMacroParamDataAutomation::IsPostponeTimeOver()
{
    return itsPostponeTimer.CurrentTimeDiffInMSeconds() >= (itsPostponeTimeInMinutes * 1000 * 60);
}

// ***********************************************************
// ************ NFmiMacroParamDataAutomationList ******************
// ***********************************************************

bool NFmiMacroParamDataAutomationList::Add(const std::string& theBetaAutomationPath)
{
    auto listItem = std::make_shared<NFmiMacroParamDataAutomationListItem>(theBetaAutomationPath);
    if(PrepareListItemAfterJsonRead(*listItem)) // Voidaan käyttää tätä metodia, vaikka listItemia ei olekaan luettu json-tiedostosta
    {
        listItem->fEnable = true;
        // Lisättäessä listaan laitetaan CalcNextDueTime funktiolla true optio päälle.
//        listItem->itsNextRunTime = listItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(NFmiMetTime(1), true);
        mAutomationVector.push_back(listItem);
        return true;
    }

    return false;
}

// Rivi indeksi pitää antaa 0-kantaisena indeksinä, oikeasti rivit grid-controllissa alkavat yhdestä, koska siinä on mukana otsikko rivi.
bool NFmiMacroParamDataAutomationList::Remove(size_t theZeroBasedRowIndex)
{
    if(theZeroBasedRowIndex < mAutomationVector.size())
    {
        mAutomationVector.erase(mAutomationVector.begin() + theZeroBasedRowIndex);
        return true;
    }
    else
        return false; // indeksi ei osoittanut mihinkään vektorin olioon
}

// ListItem on luettu json-tiedostosta NFmiBetaProductAutomationList -luvun yhteydessä, 
// sille pitää tehdä seuraavia asioita ennen kuin se voidaan lisätä itsAutomationVector:iin:
// 1. Säädä polut (abs vs relative)
// 2. Luo dynaamisesti oletus Beta-automation olio
// 3. Lue Beta-automation olio käyttöön
bool NFmiMacroParamDataAutomationList::PrepareListItemAfterJsonRead(NFmiMacroParamDataAutomationListItem& theListItem)
{
    MakeListItemPathSettings(theListItem);
    theListItem.itsMacroParamDataAutomation = std::make_shared<NFmiMacroParamDataInfo>();
    std::string errorString;
    return NFmiMacroParamDataInfo::ReadInJsonFormat(*theListItem.itsMacroParamDataAutomation, theListItem.itsMacroParamDataAutomationAbsolutePath, errorString);
}

// Oletus: theListItem.itsBetaProductAutomationPath:iin on jo laitettu käytetty polku.
// 1. Tarkistaa onko annettu polku absoluuttinen vai suhteellinen
// 2. Jos suhteellinen, laske absoluuttinen polku beta-product-base-directoryn mukaan
// 3. Jos absoluuttinen polku, laske suhteellinen polku em. base-directoryn mukaan.
// 4. Kokeillaan löytyykö saatu absoluuttinen polku
bool NFmiMacroParamDataAutomationList::MakeListItemPathSettingsCheck(NFmiMacroParamDataAutomationListItem& theListItem, const std::string& theCheckedPath)
{
    std::string givenPath = theCheckedPath;
    if(givenPath.empty())
        return false;
    if(NFmiFileSystem::IsAbsolutePath(givenPath))
    {
        theListItem.itsMacroParamDataAutomationAbsolutePath = givenPath;
        theListItem.itsMacroParamDataAutomationPath = PathUtils::getRelativePathIfPossible(givenPath, NFmiMacroParamDataGenerator::RootMacroParamDataDirectory());
    }
    else
    {
        theListItem.itsMacroParamDataAutomationAbsolutePath = PathUtils::getTrueFilePath(givenPath, NFmiMacroParamDataGenerator::RootMacroParamDataDirectory(), NFmiMacroParamDataGenerator::MacroParamDataInfoFileExtension());
        theListItem.itsMacroParamDataAutomationPath = givenPath;
    }
    return NFmiFileSystem::FileExists(theListItem.itsMacroParamDataAutomationAbsolutePath);
}

bool NFmiMacroParamDataAutomationList::MakeListItemPathSettings(NFmiMacroParamDataAutomationListItem& theListItem)
{
    auto tmpItem = theListItem;
    // Kokeillaan ensin löytyykö automaatio-tiedosto käyttäen mahdollista suhteellista polkua vastaan (käyttäjän antama polku, joka voi olla suhteellinen)
    if(!MakeListItemPathSettingsCheck(tmpItem, tmpItem.itsMacroParamDataAutomationPath))
    { 
        // jos ei löytynyt, kokeillaan vielä löytyykö absoluuttisella polulla mitään
        if(theListItem.itsMacroParamDataAutomationAbsolutePath.empty())
        {
            theListItem = tmpItem; // Jos jsonista ei saatu ollenkaan absoluuttista polkua, palautetaan suhteellisen polun tarkastelun tilanne ja palautetaan false
            return false;
        }
        else
            return MakeListItemPathSettingsCheck(theListItem, theListItem.itsMacroParamDataAutomationPath);
    }
    theListItem = tmpItem;
    return true;
}

static NFmiMacroParamDataAutomationListItem gDummyMacroParamDataAutomationListItem;

// Rivi indeksi pitää antaa 0-kantaisena indeksinä, oikeasti rivit grid-controllissa alkavat yhdestä, koska siinä on mukana otsikko rivi.
NFmiMacroParamDataAutomationListItem& NFmiMacroParamDataAutomationList::Get(size_t theZeroBasedRowIndex)
{
    if(theZeroBasedRowIndex < mAutomationVector.size())
        return *mAutomationVector[theZeroBasedRowIndex];
    else
        return gDummyMacroParamDataAutomationListItem; // indeksi ei osoittanut mihinkään vektorin olioon
}

// Rivi indeksi pitää antaa 0-kantaisena indeksinä, oikeasti rivit grid-controllissa alkavat yhdestä, koska siinä on mukana otsikko rivi.
const NFmiMacroParamDataAutomationListItem& NFmiMacroParamDataAutomationList::Get(size_t theZeroBasedRowIndex) const
{
    if(theZeroBasedRowIndex < mAutomationVector.size())
        return *mAutomationVector[theZeroBasedRowIndex];
    else
        return gDummyMacroParamDataAutomationListItem; // indeksi ei osoittanut mihinkään vektorin olioon
}

static const std::string gJsonName_MacroParamDataAutomationListHeader = "MacroParData-automation list";

json_spirit::Object NFmiMacroParamDataAutomationList::MakeJsonObject(const NFmiMacroParamDataAutomationList& theMacroParamDataAutomationList)
{
    json_spirit::Array dataArray;
    const auto& dataVector = theMacroParamDataAutomationList.AutomationVector();
    for(size_t i = 0; i < dataVector.size(); i++)
    {
        json_spirit::Object tmpObject = NFmiMacroParamDataAutomationListItem::MakeJsonObject(*dataVector[i]);
        if(tmpObject.size())
        {
            json_spirit::Value tmpVal(tmpObject);
            dataArray.push_back(tmpVal);
        }
    }

    json_spirit::Object jsonObject; // luodaan ns. null-objekti
    if(dataArray.size())
    { // täytetään objekti vain jos löytyi yhtään talletettavaa dataa
        jsonObject.push_back(json_spirit::Pair(gJsonName_MacroParamDataAutomationListHeader, dataArray));
    }
    return jsonObject;
}

void NFmiMacroParamDataAutomationList::ParseJsonPair(json_spirit::Pair& thePair)
{
    if(thePair.name_ == gJsonName_MacroParamDataAutomationListHeader)
    {
        json_spirit::Array dataFileArray = thePair.value_.get_array();
        if(dataFileArray.size())
        {
            for(json_spirit::Array::iterator it = dataFileArray.begin(); it != dataFileArray.end(); ++it)
            {
                auto listItem = std::make_shared<NFmiMacroParamDataAutomationListItem>();
                JsonUtils::ParseJsonValue(*listItem, *it);
                if(!PrepareListItemAfterJsonRead(*listItem))
                    listItem->itsStatus = MacroParamDataStatus::kFmiListItemReadError;
                mAutomationVector.push_back(listItem); // Lisätään virheellisestikin luetut listItemit, jotta käyttäjä saisi palautetta
            }
        }
    }
}

// Tätä kutsutaan kun esim. luetaan data tiedostosta ja tehdään täysi tarkistus kaikille osille
MacroParamDataStatus NFmiMacroParamDataAutomationList::DoFullChecks(bool fAutomationModeOn)
{
    auto status = MacroParamDataStatus::kFmiListItemOk;
    // Tutkitaan ensin erikseen jokainen automaatio-olio
    for(auto& listItem : mAutomationVector)
    {
        listItem->DoFullChecks(fAutomationModeOn);
        if(listItem->GetErrorStatus() != MacroParamDataStatus::kFmiListItemOk)
            return listItem->GetErrorStatus();
    }

    // Sitten tehdään yhteis tarkastuksia: 1. onko samoja automaatioita useita listassa
    if(mAutomationVector.size() > 1)
    {
        for(size_t j = 0; j < mAutomationVector.size() - 1; j++)
        {
            for(size_t i = j + 1; i < mAutomationVector.size(); i++)
            {
                if(mAutomationVector[j]->FullAutomationPath() == mAutomationVector[i]->FullAutomationPath() && !mAutomationVector[j]->FullAutomationPath().empty())
                {
                    mAutomationVector[j]->itsStatus = MacroParamDataStatus::kFmiListItemSameAutomationsInList;
                    return MacroParamDataStatus::kFmiListItemSameAutomationsInList;
                }
            }
        }
    }

    return MacroParamDataStatus::kFmiListItemOk;
}

void NFmiMacroParamDataAutomationList::RefreshAutomationList()
{
    for(auto& betaAutomation : mAutomationVector)
    {
        RefreshAutomationIfNeeded(betaAutomation);
    }
}

void NFmiMacroParamDataAutomationList::RefreshAutomationIfNeeded(std::shared_ptr<NFmiMacroParamDataAutomationListItem>& automationListItem)
{
    // Lue annettu beta-automaatio uudestaan tiedostosta uuteen olioon
    auto listItemFromFile = std::make_shared<NFmiMacroParamDataAutomationListItem>(automationListItem->itsMacroParamDataAutomationAbsolutePath);
    if(PrepareListItemAfterJsonRead(*listItemFromFile)) // Voidaan käyttää tätä metodia, vaikka listItemia ei olekaan luettu json-tiedostosta
    {
        // Jos luku meni hyvin, sijoitetaan annettu beta-automaatio päivitettävään otukseen
        automationListItem->itsMacroParamDataAutomation.swap(listItemFromFile->itsMacroParamDataAutomation);
        // päivitetään vielä seuraava ajoaika
//        automationListItem->itsNextRunTime = automationListItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(NFmiMetTime(1), true);
    }
}

bool NFmiMacroParamDataAutomationList::IsOk() const
{
    for(auto& listItem : mAutomationVector)
    {
        if(listItem->GetErrorStatus() != MacroParamDataStatus::kFmiListItemOk)
            return false;
    }
    return true;
}

std::set<std::string> NFmiMacroParamDataAutomationList::GetUniqueFilePathSet() const
{
    std::set<std::string> uniqueFilePaths;
    for(auto& listItem : mAutomationVector)
        uniqueFilePaths.insert(listItem->FullAutomationPath());
    return uniqueFilePaths;
}

bool NFmiMacroParamDataAutomationList::ContainsAutomationMoreThanOnce() const
{
    std::set<std::string> uniqueFilePaths = GetUniqueFilePathSet();
    return mAutomationVector.size() > uniqueFilePaths.size();
}

bool NFmiMacroParamDataAutomationList::HasAutomationAlready(const std::string& theFullFilePath) const
{
    std::set<std::string> uniqueFilePaths = GetUniqueFilePathSet();
    std::set<std::string>::iterator pos = uniqueFilePaths.find(theFullFilePath);
    return pos != uniqueFilePaths.end();
}

static const std::string gMacroParDataAutomationListName = "MacroParData-automation list";

bool NFmiMacroParamDataAutomationList::StoreInJsonFormat(const NFmiMacroParamDataAutomationList& theBetaProductAutomationList, const std::string& theFilePath, std::string& theErrorStringOut)
{
    return JsonUtils::StoreObjectInJsonFormat(theBetaProductAutomationList, theFilePath, gMacroParDataAutomationListName, theErrorStringOut);
}

bool NFmiMacroParamDataAutomationList::ReadInJsonFormat(NFmiMacroParamDataAutomationList& theBetaProductAutomationList, const std::string& theFilePath, std::string& theErrorStringOut)
{
    bool status = JsonUtils::ReadObjectInJsonFormat(theBetaProductAutomationList, theFilePath, gMacroParDataAutomationListName, theErrorStringOut);
    theBetaProductAutomationList.DoFullChecks(true); // Tehdään täällä tarkastelut automaatiomoodi päällä, myöhemmin (tätä funktiota kutsuvassa systeemissä) tarkastelut on tehtävä uudestaan kun oikeasti tiedetään missä moodissa ollaan
    return status;
}

// Käyttäjällä on nykyään kolme on-demand -nappia, joista voi käynnistää halutun setin automaatioita työstettäväksi:
// 1. Jos selectedAutomationIndex:issä on positiivinen numero, ajetaan vain sen osoittama automaatio.
// 2. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on true, ajetaan kaikki listalle olevat enbloidut automaatiot.
// 3. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on false, ajetaan kaikki listalle olevat automaatiot.
// selectedAutomationIndex -parametri on 1:stä alkava indeksi ja -1 tarkoitti siis että käydään koko listaa läpi.
NFmiAutomationContainer NFmiMacroParamDataAutomationList::GetOnDemandAutomations(int selectedAutomationIndex, bool doOnlyEnabled)
{
    NFmiAutomationContainer onDemandAutomations;
    if(selectedAutomationIndex > 0)
    {
        auto actualIndex = selectedAutomationIndex - 1;
        if(actualIndex < mAutomationVector.size())
        {
            const auto& automationItem = mAutomationVector[actualIndex];
            // Automaation pitää kuitenkin olla ilman virheitä, että se kelpuutetaan ajettavaksi
            if(automationItem->GetErrorStatus() == MacroParamDataStatus::kFmiListItemOk)
                onDemandAutomations.push_back(automationItem);
        }
    }
    else
    {
        for(const auto& listItem : mAutomationVector)
        {
            // Automaation pitää kuitenkin olla ilman virheitä, että se kelpuutetaan ajettavaksi
            if(listItem->GetErrorStatus() == MacroParamDataStatus::kFmiListItemOk)
            {
                if(!doOnlyEnabled || listItem->fEnable)
                    onDemandAutomations.push_back(listItem);
            }
        }
    }

    return onDemandAutomations;
}

static void AddPostponedAutomationsToDueList(std::vector<std::shared_ptr<NFmiMacroParamDataAutomationListItem>>& dueAutomationsInOut, std::list<NFmiPostponedMacroParamDataAutomation>& postponedDataTriggeredAutomations)
{
    for(auto it = postponedDataTriggeredAutomations.begin(); it != postponedDataTriggeredAutomations.end(); )
    {
        // Check condition for removal
        if(it->IsPostponeTimeOver())
        {
            std::string debugTriggerMessage = "Postponed MacroParam-data automation '";
            debugTriggerMessage += it->itsPostponedDataTriggeredAutomation->AutomationName();
            debugTriggerMessage += "' is now triggered after ";
            debugTriggerMessage += std::to_string(it->itsPostponeTimeInMinutes);
            debugTriggerMessage += " minutes";
            CatLog::logMessage(debugTriggerMessage, CatLog::Severity::Debug, CatLog::Category::Operational);
            // Kopsataan myöhästetty automaatio ajettavien listalle
            dueAutomationsInOut.push_back(it->itsPostponedDataTriggeredAutomation);
            // Erase the current element and get the iterator to the next element
            it = postponedDataTriggeredAutomations.erase(it);
        }
        else
        {
            // Move iterator to the next element
            ++it;
        }
    }
}

std::vector<std::shared_ptr<NFmiMacroParamDataAutomationListItem>> NFmiMacroParamDataAutomationList::GetDueAutomations(const NFmiMetTime& theCurrentTime, const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer)
{
    std::vector<std::shared_ptr<NFmiMacroParamDataAutomationListItem>> dueAutomations;

    for(auto& listItem : mAutomationVector)
    {
        if(listItem->fEnable)
        {
            if(listItem->GetErrorStatus() == MacroParamDataStatus::kFmiListItemOk)
            {
                int postponeTriggerInMinutes = 0;
                if(NFmiBetaProductAutomation::NFmiTriggerModeInfo::HasDataTriggerBeenLoaded(listItem->itsMacroParamDataAutomation->WantedDataTriggerList(), loadedDataTriggerList, infoOrganizer, listItem->AutomationName(), postponeTriggerInMinutes))
                {
                    if(postponeTriggerInMinutes <= 0)
                        dueAutomations.push_back(listItem);
                    else
                        itsPostponedDataTriggeredAutomations.push_back(NFmiPostponedMacroParamDataAutomation(listItem, postponeTriggerInMinutes));
                }
            }
        }
    }

    ::AddPostponedAutomationsToDueList(dueAutomations, itsPostponedDataTriggeredAutomations);

    return dueAutomations;
}

// ***********************************************************
// ************ NFmiMacroParamDataGenerator ******************
// ***********************************************************

std::string NFmiMacroParamDataGenerator::mRootSmarttoolDirectory = "";
std::string NFmiMacroParamDataGenerator::mRootMacroParamDataDirectory = "";
const std::string NFmiMacroParamDataGenerator::itsMacroParamDataInfoFileExtension = "mpd";
const std::string NFmiMacroParamDataGenerator::itsMacroParamDataInfoFileFilter = "MacroParam data info Files (*." + NFmiMacroParamDataGenerator::itsMacroParamDataInfoFileExtension + ")|*." + NFmiMacroParamDataGenerator::itsMacroParamDataInfoFileExtension + "|All Files (*.*)|*.*||";
const std::string NFmiMacroParamDataGenerator::itsMacroParamDataListFileExtension = "mpdlist";
const std::string NFmiMacroParamDataGenerator::itsMacroParamDataListFileFilter = "MacroParam data list Files (*." + NFmiMacroParamDataGenerator::itsMacroParamDataListFileExtension + ")|*." + NFmiMacroParamDataGenerator::itsMacroParamDataListFileExtension + "|All Files (*.*)|*.*||";

NFmiMacroParamDataGenerator::NFmiMacroParamDataGenerator() = default;

namespace
{
    std::string FixRootPath(std::string pathString)
    {
        pathString = PathUtils::simplifyWindowsPath(pathString);
        PathUtils::addDirectorySeparatorAtEnd(pathString);
        return pathString;
    }
}

bool NFmiMacroParamDataGenerator::Init(const std::string& theBaseRegistryPath, const std::string& rootSmarttoolDirectory, const std::string& rootMacroParamDataDirectory, const std::string& localDataBaseDirectory)
{
    mBaseRegistryPath = theBaseRegistryPath;
    mRootSmarttoolDirectory = ::FixRootPath(rootSmarttoolDirectory);
    mRootMacroParamDataDirectory = ::FixRootPath(rootMacroParamDataDirectory);
    mLocalDataBaseDirectory = localDataBaseDirectory;
    InitMacroParamDataTmpDirectory();
    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;
    // Beta product section
    std::string macroParamDataGeneratorSectionName = "\\MacroParamDataGenerator";
    mDialogBaseDataParamProducerString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\BaseDataParamProducerLevel", usedKey, "T_ec");
    mDialogUsedProducerString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\UsedProducer", usedKey, "3001,ProducerName");
    mDialogDataGeneratingSmarttoolPathString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\DataGeneratingSmarttoolPath", usedKey, "smarttool.st");
    mDialogUsedParameterListString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\UsedParameterListString", usedKey, "6201, Param1, 6202, Param2");
    mDialogDataStorageFileFilter = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\DataStorageFileFilter", usedKey, "C:\\data\\*_mydata.sqd");
    mMacroParamDataInfoSaveInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\MacroParamDataInfoSaveInitialPath", usedKey, "C:\\data\\");
    mMacroParamDataAutomationListSaveInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\MacroParamDataAutomationListSaveInitialPath", usedKey, "C:\\data\\");
    mDialogDataTriggerList = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\DataTriggerList", usedKey, "");
    mDialogMaxGeneratedFilesKept = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\MaxGeneratedFilesKept", usedKey, 2);
    mAutomationModeOn = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\AutomationModeOn", usedKey, false);
    mAutomationPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\AutomationPath", usedKey, "");
    mAutomationListPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\AutomationListPath", usedKey, "");

    LoadUsedAutomationList(AutomationListPath());

    return true;
}

// Lataa käyttöön annetusta polusta automaatio-listan.
// Asettaa polun myös mUsedAutomationListPathString:in arvoksi.
bool NFmiMacroParamDataGenerator::LoadUsedAutomationList(const std::string& thePath)
{
    AutomationListPath(thePath);
    std::string fullPath = PathUtils::getTrueFilePath(thePath, RootMacroParamDataDirectory(), NFmiMacroParamDataGenerator::MacroParamDataListFileExtension());
    std::string errorString;
    itsUsedMacroParamDataAutomationList = NFmiMacroParamDataAutomationList(); // Nollataan käytössä ollut lista ennen uuden lukua
    bool status = NFmiMacroParamDataAutomationList::ReadInJsonFormat(itsUsedMacroParamDataAutomationList, fullPath, errorString);
    UsedMacroParamDataAutomationList().DoFullChecks(AutomationModeOn());
    return status;
}

static NFmiProducer MakeWantedProducer(const NFmiMacroParamDataInfo& dataInfo)
{
    auto producerStringCheck = NFmiMacroParamDataInfo::CheckUsedProducerString(dataInfo.UsedProducerString());
    if(!producerStringCheck.first.empty())
    {
        throw std::runtime_error(producerStringCheck.first);
    }

    unsigned long producerId = std::stoul(producerStringCheck.second[0]);
    return NFmiProducer(producerId, boost::trim_copy(producerStringCheck.second[1]));
}

static NFmiParamDescriptor MakeWantedParamDescriptor(const NFmiMacroParamDataInfo& dataInfo)
{
    auto producer = ::MakeWantedProducer(dataInfo);
    auto paramListStringCheck = NFmiMacroParamDataInfo::CheckUsedParameterListString(dataInfo.UsedParameterListString(), producer);
    if(!paramListStringCheck.first.empty())
    {
        throw std::runtime_error(paramListStringCheck.first);
    }

    return NFmiParamDescriptor(paramListStringCheck.second);
}

static NFmiVPlaceDescriptor MakeCorrectVPlaceDescriptor(boost::shared_ptr<NFmiFastQueryInfo>& foundInfo)
{
    if(foundInfo->SizeLevels() <= 1)
        return foundInfo->VPlaceDescriptor();

    // Level datasta pitää tehdä surfacedata tyyppinen
    NFmiLevelBag levelBag(kFmiAnyLevelType, 0, 0, 1);
    return NFmiVPlaceDescriptor(levelBag);
}

static NFmiHPlaceDescriptor MakeScaledHPlaceDescriptor(boost::shared_ptr<NFmiFastQueryInfo>& foundInfo, double scale)
{
    if(scale <= 1)
        return foundInfo->HPlaceDescriptor();

    auto usedGridSizeX = boost::math::iround(foundInfo->GridXNumber() / scale);
    auto usedGridSizeY = boost::math::iround(foundInfo->GridYNumber() / scale);
    NFmiGrid usedGrid(foundInfo->Area(), usedGridSizeX, usedGridSizeY);
    return NFmiHPlaceDescriptor(usedGrid);
}

// Oletus: foundInfo on jo tarkistettu ettei se ole nullptr
static boost::shared_ptr<NFmiQueryData> MakeWantedEmptyData(boost::shared_ptr<NFmiFastQueryInfo>& foundInfo, const NFmiParamDescriptor& wantedParamDescriptor)
{
    auto correctVPlaceDescriptor = ::MakeCorrectVPlaceDescriptor(foundInfo);
    auto scaledHPlaceDescriptor = ::MakeScaledHPlaceDescriptor(foundInfo, 1);
    NFmiQueryInfo metaInfo(wantedParamDescriptor, foundInfo->TimeDescriptor(), scaledHPlaceDescriptor, correctVPlaceDescriptor);
    return boost::shared_ptr<NFmiQueryData>(NFmiQueryDataUtil::CreateEmptyData(metaInfo));
}

static NFmiInfoOrganizer* GetInfoOrganizer()
{
    return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->InfoOrganizer();
}

std::string NFmiMacroParamDataGenerator::ReadSmarttoolContentFromFile(const std::string& filePath)
{
    mUsedAbsoluteSmarttoolPath = MakeUsedAbsoluteSmarttoolPathString(filePath);
    std::string fileContent;
    if(NFmiFileSystem::ReadFile2String(mUsedAbsoluteSmarttoolPath, fileContent))
    {
        return fileContent;
    }

    throw std::runtime_error(std::string(__FUNCTION__) + " unable to read smarttool file content from given file: " + mUsedAbsoluteSmarttoolPath);
}

class DataGenerationIsOnHandler
{
    bool& dataGenerationIsOn_;
public:
    DataGenerationIsOnHandler(bool& dataGenerationIsOn)
        :dataGenerationIsOn_(dataGenerationIsOn)
    {
        dataGenerationIsOn_ = true;
    }
    ~DataGenerationIsOnHandler()
    {
        dataGenerationIsOn_ = false;
    }
};

static std::string MakeTmpDataFilePath(const std::string& finalDataFilePath, const std::string& tmpDataDirectory)
{
    auto tmpDataFilePath = tmpDataDirectory;
    tmpDataFilePath += PathUtils::getFilename(finalDataFilePath, true);
    return tmpDataFilePath;
}

static bool EnsureFilePathDirectoryExists(const std::string &totalFilePath, const std::string& pathDescription)
{
    auto directoryPath = PathUtils::getPathSectionFromTotalFilePath(totalFilePath);

    if(!NFmiFileSystem::CreateDirectory(directoryPath))
    {
        std::string errorStr = "MacroParam-data creation: unable to create directory '";
        errorStr += directoryPath + "' for " + pathDescription + " (unknown reason).";
        throw std::runtime_error(errorStr);
    }

    return true;
}

static void LogDataGenerationCompletes(const std::string& fullAutomationPath, const std::string& finalDataFilePath, const std::string& timeLasted)
{
    std::string logStr = "Finished to generate '";
    logStr += PathUtils::getFilename(fullAutomationPath, false);
    logStr += "' MacroParam-data (";
    logStr += timeLasted;
    logStr += "), saved to file '";
    logStr += finalDataFilePath;
    logStr += "', from automation: ";
    logStr += fullAutomationPath;
    CatLog::logMessage(logStr, CatLog::Severity::Info, CatLog::Category::Operational);
}

// Oletus: macroParamDataPtr pitää sisällään talletettavan datan.
// Tätä funktiota ei keskeytetä (käyttäjän pyyntö/ohjelman lopetus), koska tiedostojen 
// jälkien siivonta tekisi koodista sotkuista. Täällä on vain yksi hidas toimenpide eli qdata->Write jota ei voi keskeyttää.
// Tiedostojen kirjoitus on kuitenkin suht nopeaa (< 1 s useimmiten), varsinkin jos SSD käytössä.
bool NFmiMacroParamDataGenerator::StoreMacroParamData(boost::shared_ptr<NFmiQueryData>& macroParamDataPtr, const std::string& dataStorageFileFilter, int keepMaxFiles, const std::string& fullAutomationPath, NFmiMilliSecondTimer &timer)
{
    // CheckDataStorageFileFilter funktio palauttaa virheilmoituksen, jos filefilterissa jotain vikaa.
    auto checkStr = NFmiMacroParamDataInfo::CheckDataStorageFileFilter(dataStorageFileFilter);
    if(!checkStr.empty())
    {
        throw std::runtime_error(checkStr);
    }

    auto finalDataFilePath = NFmiMacroParamDataInfo::MakeDataStorageFilePath(dataStorageFileFilter);
    auto tmpDataFilePath = ::MakeTmpDataFilePath(finalDataFilePath, mMacroParamDataTmpDirectory);

    // Talletetaan ensin tmp hakemistoon, ja sitten vasta rename:lla laitetaan lopulliseen hakemistoon.
    // Tarkoitus on että data siirtyy Dropboxiin valmiina ja tällöin Dropbox ei ala jakamaan 
    // keskeneräisesti vielä kirjoituksessa olevaa dataa.
    // NFmiQueryData::Write heittää poikkeuksia, jos tulee ongelmia.
    macroParamDataPtr->Write(tmpDataFilePath);
    ::EnsureFilePathDirectoryExists(finalDataFilePath, "MacroParam-data directory");
    if(!NFmiFileSystem::RenameFile(tmpDataFilePath, finalDataFilePath))
    {
        std::string errorStr = __FUNCTION__;
        errorStr += ": Moving generated MacroParam-data tmp file '" + tmpDataFilePath + "' to '" + finalDataFilePath + "' failed for unknown reason.";
        throw std::runtime_error(errorStr);
    }
    timer.StopTimer();
    auto timeLastedStr = timer.TimeDiffInMSeconds() < 10000 ? timer.EasyTimeDiffStr(false, true) : timer.EasyTimeDiffStr(true, true);
    LastGeneratedDataMakeTime(timeLastedStr);
    ::LogDataGenerationCompletes(fullAutomationPath, finalDataFilePath, timeLastedStr);
    ::CleanFilePattern(dataStorageFileFilter, keepMaxFiles);
    return true;
}

static void CalcMultiLevelSmarttoolData(NFmiSmartToolModifier& smartToolModifier, boost::shared_ptr<NFmiFastQueryInfo>& editedInfoCopy)
{
    auto usedTimeDescriptor(editedInfoCopy->TimeDescriptor());
    for(editedInfoCopy->ResetLevel(); editedInfoCopy->NextLevel();)
    {
        // jos kyseessä on level-data, pitää läpikäytävä leveli ottaa talteen, 
        // että smartToolModifier osaa luoda siihen osoittavia fastInfoja.
        boost::shared_ptr<NFmiLevel> theLevel(new NFmiLevel(*editedInfoCopy->Level()));
        smartToolModifier.ModifiedLevel(theLevel);

        smartToolModifier.ModifyData_ver2(&usedTimeDescriptor, false, false, nullptr);
    }
}

// Oletus: wantedMacroParamInfoPtr on tarkistettu jo ulkopuolella että ei ole nullptr
bool NFmiMacroParamDataGenerator::CalculateDataWithSmartTool(boost::shared_ptr<NFmiFastQueryInfo>& wantedMacroParamInfoPtr, NFmiInfoOrganizer* infoOrganizer, const std::string& smartToolContent, NFmiThreadCallBacks *threadCallBacks)
{
    itsSmarttoolCalculationLogStr.clear();

    NFmiSmartToolModifier smartToolModifier(infoOrganizer);
    try // ensin tulkitaan macro
    {
        smartToolModifier.SetGriddingHelper(CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetGriddingHelper());
        smartToolModifier.IncludeDirectory(mRootSmarttoolDirectory);
        smartToolModifier.InitSmartTool(smartToolContent);
        smartToolModifier.SetFixedEditedData(wantedMacroParamInfoPtr);
    }
    catch(std::exception& e)
    {
        itsSmarttoolCalculationLogStr = e.what();
        std::string errorString = e.what();
        errorString += ", in macro '";
        errorString += mUsedAbsoluteSmarttoolPath;
        errorString += "'";
        throw std::runtime_error(errorString);
    }

    // Jos kyseessä on ns. macroParam eli skriptissä on lausekkeitä jossa sijoitetaan 
    // RETURN muuttujaan, tällöin ei haluta ajaa makro muutoksia, koska tarkoitus 
    // on vain katsella macroParam:ia karttanäytöllä.
    // Tänne asti tullaan että saadaan virhe ilmoitus tulkinta vaiheessa talteen.
    // Mutta suoritus vaiheen virheet menevät tällä hetkellä vain loki tiedostoon.
    if(smartToolModifier.IsInterpretedSkriptMacroParam())
    {
        throw std::runtime_error(std::string("Used script '") + mUsedAbsoluteSmarttoolPath + "' was macroParam type. You have to use smarttool scripts when generating MacroParam data, not macroParam scripts, even though data's name might suggest otherwise");
    }

    NFmiQueryDataUtil::CheckIfStopped(threadCallBacks);
    // suoritetaan macro sitten
    try
    {
        auto usedTimeDescriptor(wantedMacroParamInfoPtr->TimeDescriptor());
        //if(wantedMacroParamInfoPtr->SizeLevels() > 1)
        //{
        //    auto editedInfoCopy = NFmiSmartInfo::CreateShallowCopyOfHighestInfo(wantedMacroParamInfoPtr);
        //    CalcMultiLevelSmarttoolData(smartToolModifier, editedInfoCopy);
        //}
        //else
        //{
        smartToolModifier.ModifyData_ver2(&usedTimeDescriptor, false, false, threadCallBacks);
        //}

        if(!smartToolModifier.LastExceptionMessageFromThreads().empty())
        {
            itsSmarttoolCalculationLogStr = smartToolModifier.LastExceptionMessageFromThreads();
            std::string errorString = smartToolModifier.LastExceptionMessageFromThreads();
            errorString += ", in macro '";
            errorString += mUsedAbsoluteSmarttoolPath;
            errorString += "'";
            throw std::runtime_error(errorString);
        }
    }
    catch(std::exception& e)
    {
        itsSmarttoolCalculationLogStr = e.what();
        std::string errorString = e.what();
        errorString += ", in macro '";
        errorString += mUsedAbsoluteSmarttoolPath;
        errorString += "'";
        throw std::runtime_error(errorString);
    }

    return true;
}


std::string NFmiMacroParamDataGenerator::DialogBaseDataParamProducerString() const
{
    return *mDialogBaseDataParamProducerString;
}

void NFmiMacroParamDataGenerator::DialogBaseDataParamProducerString(const std::string& newValue)
{
    *mDialogBaseDataParamProducerString = newValue;
}

std::string NFmiMacroParamDataGenerator::DialogDataGeneratingSmarttoolPathString() const
{
    return *mDialogDataGeneratingSmarttoolPathString;
}

void NFmiMacroParamDataGenerator::DialogDataGeneratingSmarttoolPathString(const std::string& newValue)
{
    *mDialogDataGeneratingSmarttoolPathString = newValue;
}

// Jos annettu polku on suhteellinen, lisätään siihen smarttool juuri hakemisto.
// Korjataan polku kaikin puolin oikeaan formaattiin (erottimet ja mahd. .. tai . jutut).
std::string NFmiMacroParamDataGenerator::MakeUsedAbsoluteSmarttoolPathString(const std::string& smarttoolPath) const
{
    return PathUtils::getAbsoluteFilePath(smarttoolPath, mRootSmarttoolDirectory);
}

// Jos annettu polku on absoluuttinen, poistetaan siitä smarttool juuri hakemisto osio alusta.
// Korjataan polku kaikin puolin oikeaan formaattiin (erottimet ja mahd. .. tai . jutut).
std::string NFmiMacroParamDataGenerator::MakeCleanedSmarttoolPathString(const std::string& smarttoolPath) const
{
    auto tmpPath = PathUtils::getRelativePathIfPossible(smarttoolPath, mRootSmarttoolDirectory);
    return PathUtils::simplifyWindowsPath(tmpPath);
}

std::string NFmiMacroParamDataGenerator::DialogUsedParameterListString() const
{
    return *mDialogUsedParameterListString;
}

void NFmiMacroParamDataGenerator::DialogUsedParameterListString(const std::string& newValue)
{
    *mDialogUsedParameterListString = newValue;
}

std::string NFmiMacroParamDataGenerator::DialogUsedProducerString() const
{
    return *mDialogUsedProducerString;
}

void NFmiMacroParamDataGenerator::DialogUsedProducerString(const std::string& newValue)
{
    *mDialogUsedProducerString = newValue;
}

std::string NFmiMacroParamDataGenerator::DialogDataStorageFileFilter() const
{
    return *mDialogDataStorageFileFilter;
}

void NFmiMacroParamDataGenerator::DialogDataStorageFileFilter(const std::string& newValue)
{
    *mDialogDataStorageFileFilter = newValue;
}

std::string NFmiMacroParamDataGenerator::DialogDataTriggerList() const
{
    return *mDialogDataTriggerList;
}

void NFmiMacroParamDataGenerator::DialogDataTriggerList(const std::string& newValue)
{
    *mDialogDataTriggerList = newValue;
}

int NFmiMacroParamDataGenerator::DialogMaxGeneratedFilesKept() const
{
    return *mDialogMaxGeneratedFilesKept;
}

void NFmiMacroParamDataGenerator::DialogMaxGeneratedFilesKept(int newValue)
{
    *mDialogMaxGeneratedFilesKept = NFmiMacroParamDataInfo::FixMaxGeneratedFilesKeptValue(newValue);
}

bool NFmiMacroParamDataGenerator::AutomationModeOn() const
{
    return *mAutomationModeOn;
}

void NFmiMacroParamDataGenerator::AutomationModeOn(bool newState)
{
    *mAutomationModeOn = newState;
}

std::string NFmiMacroParamDataGenerator::AutomationPath() const
{
    return *mAutomationPath;
}

void NFmiMacroParamDataGenerator::AutomationPath(const std::string& newValue) const
{
    *mAutomationPath = newValue;
}

std::string NFmiMacroParamDataGenerator::AutomationListPath() const
{
    return *mAutomationListPath;
}

void NFmiMacroParamDataGenerator::AutomationListPath(const std::string& newValue)
{
    *mAutomationListPath = newValue;
}

std::string NFmiMacroParamDataGenerator::LastGeneratedDataMakeTime() const
{
    std::lock_guard<std::mutex> lockGuard(mLastGeneratedDataMakeTimeMutex);
    return mLastGeneratedDataMakeTime;
}

void NFmiMacroParamDataGenerator::LastGeneratedDataMakeTime(std::string newValue)
{
    std::lock_guard<std::mutex> lockGuard(mLastGeneratedDataMakeTimeMutex);
    mLastGeneratedDataMakeTime = newValue;
}

NFmiMacroParamDataInfo NFmiMacroParamDataGenerator::MakeDataInfo() const
{
    return NFmiMacroParamDataInfo(DialogBaseDataParamProducerString(), DialogUsedProducerString(), DialogDataGeneratingSmarttoolPathString(), DialogUsedParameterListString(), DialogDataStorageFileFilter(), DialogDataTriggerList(), DialogMaxGeneratedFilesKept());
}

std::string NFmiMacroParamDataGenerator::MacroParamDataInfoSaveInitialPath()
{
    return *mMacroParamDataInfoSaveInitialPath;
}

void NFmiMacroParamDataGenerator::MacroParamDataInfoSaveInitialPath(const std::string& newValue)
{
    *mMacroParamDataInfoSaveInitialPath = newValue;
}

std::string NFmiMacroParamDataGenerator::MacroParamDataAutomationListSaveInitialPath() const
{
    return *mMacroParamDataAutomationListSaveInitialPath;
}

void NFmiMacroParamDataGenerator::MacroParamDataAutomationListSaveInitialPath(const std::string& newValue)
{
    *mMacroParamDataAutomationListSaveInitialPath = newValue;
}

static void LogDataGenerationInterruption(bool userHasStopped, bool smartmetIsBeenClosed)
{
    if(userHasStopped)
    {
        // Käyttäjä haluaa lopettaa
        std::string errorStr = "Generating MacroParam-data was canceled due the user has pressed the Cancel button.";
        CatLog::logMessage(errorStr, CatLog::Severity::Error, CatLog::Category::Operational, true);
    }
    else if(smartmetIsBeenClosed)
    {
        // Smartmetin sulkeminen vaatii että datan generointi lopetetaan
        std::string errorStr = "Generating MacroParam-data was canceled due the Smartmet application is being closed.";
        CatLog::logMessage(errorStr, CatLog::Severity::Error, CatLog::Category::Operational, true);
    }
    else
    {
        // Tuntematon poikkeus keskeytti datan generoinnin
        std::string errorStr = "Generating MacroParam-data was canceled due unknown exception or error.";
        CatLog::logMessage(errorStr, CatLog::Severity::Error, CatLog::Category::Operational, true);
    }
}

// Käyttäjällä on nykyään kolme on-demand -nappia, joista voi käynnistää halutun setin automaatioita työstettäväksi:
// 1. Jos selectedAutomationIndex:issä on positiivinen numero, ajetaan vain sen osoittama automaatio.
// 2. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on true, ajetaan kaikki listalle olevat enbloidut automaatiot.
// 3. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on false, ajetaan kaikki listalle olevat automaatiot.
// selectedAutomationIndex -parametri on 1:stä alkava indeksi ja -1 tarkoitti siis että käydään koko listaa läpi.
bool NFmiMacroParamDataGenerator::DoOnDemandBetaAutomations(int selectedAutomationIndex, bool doOnlyEnabled, NFmiThreadCallBacks* threadCallBacks)
{
    try
    {
        DataGenerationIsOnHandler dataGenerationIsOnHandler(fDataGenerationIsOn);
        bool status = false;
        auto onDemandAutomations = itsUsedMacroParamDataAutomationList.GetOnDemandAutomations(selectedAutomationIndex, doOnlyEnabled);
        status = GenerateAutomationsData(onDemandAutomations, threadCallBacks);
        if(threadCallBacks)
            threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FINISHED);
        return status;
    }
    catch(NFmiStopThreadException&)
    {
        ::LogDataGenerationInterruption(true, false);
    }
    catch(...)
    {
        ::LogDataGenerationInterruption(false, false);
    }
    if(threadCallBacks)
        threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_CANCELED);
    return false;
}

bool NFmiMacroParamDataGenerator::GenerateAutomationsData(const NFmiAutomationContainer &automations, NFmiThreadCallBacks* threadCallBacks)
{
    bool status = false;
    if(!automations.empty())
    {
        for(const auto& automationItem : automations)
        {
            status |= GenerateMacroParamData(*(automationItem->itsMacroParamDataAutomation), automationItem->FullAutomationPath(), threadCallBacks);
        }
    }
    return status;
}

bool NFmiMacroParamDataGenerator::GenerateMacroParamData(NFmiThreadCallBacks* threadCallBacks)
{
    try
    {
        DataGenerationIsOnHandler dataGenerationIsOnHandler(fDataGenerationIsOn);
        auto dataInfo = MakeDataInfo();
        auto result = GenerateMacroParamData(dataInfo, AutomationPath(), threadCallBacks);
        if(threadCallBacks)
            threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FINISHED);
        return result;
    }
    catch(NFmiStopThreadException &)
    {
        ::LogDataGenerationInterruption(true, false);
    }
    catch(...)
    {
        ::LogDataGenerationInterruption(false, false);
    }
    if(threadCallBacks)
        threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_CANCELED);
    return false;
}

static void LogDataGenerationStarts(const std::string& fullAutomationPath)
{
    std::string logStr = "Starting to generate '";
    logStr += PathUtils::getFilename(fullAutomationPath, false);
    logStr += "' MacroParam-data from automation: ";
    logStr += fullAutomationPath;
    CatLog::logMessage(logStr, CatLog::Severity::Info, CatLog::Category::Operational);
}

bool NFmiMacroParamDataGenerator::GenerateMacroParamData(const NFmiMacroParamDataInfo& dataInfo, const std::string& fullAutomationPath, NFmiThreadCallBacks* threadCallBacks)
{
    try
    {
        NFmiMilliSecondTimer timer;
        ::LogDataGenerationStarts(fullAutomationPath);

        if(!EnsureTmpDirectoryExists())
            return false;

        NFmiQueryDataUtil::CheckIfStopped(threadCallBacks);
        auto baseDataCheck = NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(dataInfo.BaseDataParamProducerString(), true);
        if(!baseDataCheck.first.empty())
        {
            throw std::runtime_error(baseDataCheck.first);
        }

        auto* infoOrganizer = ::GetInfoOrganizer();
        if(!infoOrganizer)
        {
            throw std::runtime_error("GenerateMacroParamData: Can't get proper InfoOrganizer into works, unknown error in the system");
        }

        // NFmiExtraMacroParamData::FindWantedInfo metodin 3. parametri (false) tarkoittaa että pohjadataksi 
        // ei kelpuuteta asemadataa, vain hiladata kelpaa.
        auto wantedInfo = NFmiExtraMacroParamData::FindWantedInfo(*infoOrganizer, baseDataCheck.second, false);
        if(!wantedInfo.foundInfo_)
        {
            throw std::runtime_error(std::string("GenerateMacroParamData: unable to find requested base data with '") + dataInfo.BaseDataParamProducerString() + "' parameter, unable to generate wanted macroParam data");
        }

        auto wantedParamDescriptor = ::MakeWantedParamDescriptor(dataInfo);
        NFmiQueryDataUtil::CheckIfStopped(threadCallBacks);
        auto wantedMacroParamDataPtr = ::MakeWantedEmptyData(wantedInfo.foundInfo_, wantedParamDescriptor);
        if(!wantedMacroParamDataPtr)
        {
            throw std::runtime_error("GenerateMacroParamData: Unable to generate actual macroParam querydata, unknown error in system");
        }
        if(!wantedMacroParamDataPtr->IsGrid())
        {
            throw std::runtime_error("GenerateMacroParamData: Given base data wasn't a grid data, stopping MacroParam-data generation");
        }
        boost::shared_ptr<NFmiFastQueryInfo> wantedMacroParamInfoPtr(new NFmiFastQueryInfo(wantedMacroParamDataPtr.get()));

        auto smarttoolContent = ReadSmarttoolContentFromFile(dataInfo.DataGeneratingSmarttoolPathString());
        NFmiQueryDataUtil::CheckIfStopped(threadCallBacks);

        if(CalculateDataWithSmartTool(wantedMacroParamInfoPtr, infoOrganizer, smarttoolContent, threadCallBacks))
        {
            return StoreMacroParamData(wantedMacroParamDataPtr, dataInfo.DataStorageFileFilter(), dataInfo.MaxGeneratedFilesKept(), fullAutomationPath, timer);
        }
    }
    catch(std::exception& e)
    {
        // Normi poikkeukset lokitetaan yhden MacroParam-datan generoinnin tasolla
        CatLog::logMessage(e.what(), CatLog::Severity::Error, CatLog::Category::Operational, true);
    }

    return false;
}

// SmartMetin CMainFrm::OnTimer kutsuu tätä funktiota kerran minuutissa ja päättelee onko tehtävä mitään 
// itsUsedAutomationList:alla olevaa tuotantoa.
void NFmiMacroParamDataGenerator::DoNeededMacroParamDataAutomations(const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer)
{
    if(!AutomationModeOn())
        return;

    NFmiMetTime currentTime(1); // Otetaan talteen nyky UTC hetki minuutin tarkkuudella
    auto dueAutomations = itsUsedMacroParamDataAutomationList.GetDueAutomations(currentTime, loadedDataTriggerList, infoOrganizer);
    std::thread t(&NFmiMacroParamDataGenerator::LaunchGenerateAutomationsData, this, dueAutomations, nullptr);
    t.detach();
}

// Tänne tullaan vain MacroParam-data automaatioiden kautta, eli ei käyttäjän toimesta.
// Kun Automaatiosysteemi on käytössä, on kaikki käyttäjän generoinnit estetty.
// Eli täällä ei tarvitse laittaa generointi lippuja päälle, tai lähetellä viestejä, 
// kun on valmista, tai canceloitu.
void NFmiMacroParamDataGenerator::LaunchGenerateAutomationsData(const NFmiAutomationContainer& automations, NFmiThreadCallBacks* threadCallBacks)
{
    try
    {
        GenerateAutomationsData(automations, nullptr);
        // Ei saa lähetellä viestejä dialogille!
        //if(threadCallBacks)
        //    threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FINISHED);
    }
    catch(NFmiStopThreadException&)
    {
        // Tässä lopetus tulee aina Smartmetin sulkemisesta
        ::LogDataGenerationInterruption(false, true);
    }
    catch(...)
    {
        ::LogDataGenerationInterruption(false, false);
    }

    // Ei saa lähetellä viestejä dialogille!
    //if(threadCallBacks)
    //    threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_CANCELED);
}

void NFmiMacroParamDataGenerator::InitMacroParamDataTmpDirectory()
{
    mMacroParamDataTmpDirectory = mLocalDataBaseDirectory + "\\MacroParamDataTmp\\";
    mMacroParamDataTmpDirectory = PathUtils::simplifyWindowsPath(mMacroParamDataTmpDirectory);
}

bool NFmiMacroParamDataGenerator::EnsureTmpDirectoryExists()
{
    if(!NFmiFileSystem::CreateDirectory(mMacroParamDataTmpDirectory))
    {
        std::string errorStr = "MacroParam-data creation: unable to create directory '";
        errorStr += mMacroParamDataTmpDirectory + "' for temporary querydata file (unknown reason).";
        throw std::runtime_error(errorStr);
    }

    return true;
}
