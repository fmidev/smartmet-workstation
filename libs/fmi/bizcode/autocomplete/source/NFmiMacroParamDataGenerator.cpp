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
#include "jsonutils.h"
#include "catlog/catlog.h"
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <regex>
#include <numeric>

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
        return NFmiParam(paramId, boost::trim_copy(part2));
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

    // Tarkistaa onko inputissa joko pelkk� T_ec tyyppinen teksti
    // tai vaihtoehtoisesti siin� voi olla vaikka T_ec[0.5h]
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
            logMessageStart += NFmiStringTools::Convert(keepMaxFiles);
            logMessageStart += ",\nDeleted files:\n";
            CtrlViewUtils::DeleteFilesWithPatternAndLog(theFilePattern, logMessageStart, CatLog::Severity::Debug, CatLog::Category::Data, keepMaxFiles);
        }
        catch(...)
        {
            // ei tehd� toistaiseksi mit��n...
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
        auto checkResult = NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(mBaseDataParamProducerString, false);
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
        if(!checkResult.empty())
        {
            itsCheckShortStatusStr = "Trigger list error";
            return false;
        }
    }

    itsCheckShortStatusStr = "Macro param data ok";
    return true;
}

std::string NFmiMacroParamDataInfo::MakeDataStorageFilePath(const std::string& dataStorageFileFilter)
{
    // Tehd��n lokaali ajasta aikaleima, jossa mukana my�s sekunnit
    NFmiTime makeTime;
    std::string timeStampStr = makeTime.ToStr(kYYYYMMDDHHMMSS);
    auto finalFilePath = dataStorageFileFilter;
    boost::replace_all(finalFilePath, "*", timeStampStr);
    return finalFilePath;
}

// Perus datan valintaan liittyvan par+prod stringin tarkistus. Palauttaa selityksen virheest�, jos siin� on jotain vikaa.
// Jos kaikki on ok, palauttaa tyhj�n stringin.
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

// Talletettavan datan filefiltterin tarkistus. Palauttaa selityksen virheest�, jos siin� on jotain vikaa.
// Jos kaikki on ok, palauttaa tyhj�n string:in.
std::string NFmiMacroParamDataInfo::CheckDataStorageFileFilter(const std::string& dataStorageFileFilter)
{
    // 1. Polku ei kelpaa, jos talletushakemistoa ei ole.
    // T�ll� tarkoitus est�� datojen vahinkotalletuksia v��riin paikkoihin.
    auto directory = PathUtils::getPathSectionFromTotalFilePath(dataStorageFileFilter);
    if(!::DirectoryExists(directory))
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output directory '" + directory + "' doesn't exists, make sure that given path for MacroParam data was correct!";
    }

    auto filename = PathUtils::getFilename(dataStorageFileFilter, false);
    // 2. Tiedoston nimess� pit�� olla '*' merkki aikaleimaa varten.
    if(filename.find('*') == std::string::npos)
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output filename '" + filename + "' (in " + dataStorageFileFilter + ") doesn't have ' * ' character place marker for file's creation time stamp. To store MacroParam data you need to have that asterisk in filename.";
    }

    // 3. Tiedoston nimess� pit�� olla v�hint��n 3 alpha-numeerista merkki�, jotta esim. pelkk� path\*.sqd ei kelpaa
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
    // 4. Tiedoston extension pit�� olla sqd tyyppinen
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
    // 3. Tiedoston extension pit�� olla st tyyppinen
    if(!boost::iequals(fileExtension, gSmarttoolFileExtension))
    {
        return std::string("CheckDataGeneratingSmarttoolPathString") + ": Given data output extension '" + fileExtension + "' (in " + dataGeneratingSmarttoolPathString + ") is invalid, extension must be '" + gSmarttoolFileExtension + "' type.";
    }

    return "";
}

std::string NFmiMacroParamDataInfo::CheckDataTriggerListString(const std::string& dataTriggerListString)
{
    auto parts = ::GetSplittedAndTrimmedStrings(dataTriggerListString, ",");
    // Tyhj� on ok
    if(parts.empty())
        return "";

    for(const auto& paramDataStr : parts)
    {
        auto checkData = CheckBaseDataParamProducerString(paramDataStr, true);
        if(!checkData.first.empty())
        {
            return checkData.first;
        }

        if(!::DoParamDataCheckWithOptionalDelayString(paramDataStr))
        {
            return std::string("CheckDataTriggerListString") + ": Given data parameter '" + paramDataStr + "' (in " + dataTriggerListString + ") had invalid form, should be like 'T_ec' or 'T_ec[0.5h]'";
        }
    }

    return "";
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
    // T�ss� puretaan NFmiMacroParamDataInfo luokan p��tason pareja.
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

// T�t� kutsutaan kun esim. luetaan data tiedostosta ja tehd��n t�ysi tarkistus kaikille osille
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

// T�m� tallentaa json objektiin vain enable -lipun ja polun k�ytettyyn MacroParam data -automaatioon. 
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

// ***********************************************************
// ************ NFmiMacroParamDataAutomationList ******************
// ***********************************************************

bool NFmiMacroParamDataAutomationList::Add(const std::string& theBetaAutomationPath)
{
    auto listItem = std::make_shared<NFmiMacroParamDataAutomationListItem>(theBetaAutomationPath);
    if(PrepareListItemAfterJsonRead(*listItem)) // Voidaan k�ytt�� t�t� metodia, vaikka listItemia ei olekaan luettu json-tiedostosta
    {
        listItem->fEnable = true;
        // Lis�tt�ess� listaan laitetaan CalcNextDueTime funktiolla true optio p��lle.
//        listItem->itsNextRunTime = listItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(NFmiMetTime(1), true);
        mAutomationVector.push_back(listItem);
        return true;
    }

    return false;
}

// Rivi indeksi pit�� antaa 0-kantaisena indeksin�, oikeasti rivit grid-controllissa alkavat yhdest�, koska siin� on mukana otsikko rivi.
bool NFmiMacroParamDataAutomationList::Remove(size_t theZeroBasedRowIndex)
{
    if(theZeroBasedRowIndex < mAutomationVector.size())
    {
        mAutomationVector.erase(mAutomationVector.begin() + theZeroBasedRowIndex);
        return true;
    }
    else
        return false; // indeksi ei osoittanut mihink��n vektorin olioon
}

// ListItem on luettu json-tiedostosta NFmiBetaProductAutomationList -luvun yhteydess�, 
// sille pit�� tehd� seuraavia asioita ennen kuin se voidaan lis�t� itsAutomationVector:iin:
// 1. S��d� polut (abs vs relative)
// 2. Luo dynaamisesti oletus Beta-automation olio
// 3. Lue Beta-automation olio k�ytt��n
bool NFmiMacroParamDataAutomationList::PrepareListItemAfterJsonRead(NFmiMacroParamDataAutomationListItem& theListItem)
{
    MakeListItemPathSettings(theListItem);
    theListItem.itsMacroParamDataAutomation = std::make_shared<NFmiMacroParamDataInfo>();
    std::string errorString;
    return NFmiMacroParamDataInfo::ReadInJsonFormat(*theListItem.itsMacroParamDataAutomation, theListItem.itsMacroParamDataAutomationAbsolutePath, errorString);
}

// Oletus: theListItem.itsBetaProductAutomationPath:iin on jo laitettu k�ytetty polku.
// 1. Tarkistaa onko annettu polku absoluuttinen vai suhteellinen
// 2. Jos suhteellinen, laske absoluuttinen polku beta-product-base-directoryn mukaan
// 3. Jos absoluuttinen polku, laske suhteellinen polku em. base-directoryn mukaan.
// 4. Kokeillaan l�ytyyk� saatu absoluuttinen polku
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
    // Kokeillaan ensin l�ytyyk� automaatio-tiedosto k�ytt�en mahdollista suhteellista polkua vastaan (k�ytt�j�n antama polku, joka voi olla suhteellinen)
    if(!MakeListItemPathSettingsCheck(tmpItem, tmpItem.itsMacroParamDataAutomationPath))
    { 
        // jos ei l�ytynyt, kokeillaan viel� l�ytyyk� absoluuttisella polulla mit��n
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

// Rivi indeksi pit�� antaa 0-kantaisena indeksin�, oikeasti rivit grid-controllissa alkavat yhdest�, koska siin� on mukana otsikko rivi.
NFmiMacroParamDataAutomationListItem& NFmiMacroParamDataAutomationList::Get(size_t theZeroBasedRowIndex)
{
    if(theZeroBasedRowIndex < mAutomationVector.size())
        return *mAutomationVector[theZeroBasedRowIndex];
    else
        return gDummyMacroParamDataAutomationListItem; // indeksi ei osoittanut mihink��n vektorin olioon
}

// Rivi indeksi pit�� antaa 0-kantaisena indeksin�, oikeasti rivit grid-controllissa alkavat yhdest�, koska siin� on mukana otsikko rivi.
const NFmiMacroParamDataAutomationListItem& NFmiMacroParamDataAutomationList::Get(size_t theZeroBasedRowIndex) const
{
    if(theZeroBasedRowIndex < mAutomationVector.size())
        return *mAutomationVector[theZeroBasedRowIndex];
    else
        return gDummyMacroParamDataAutomationListItem; // indeksi ei osoittanut mihink��n vektorin olioon
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
    { // t�ytet��n objekti vain jos l�ytyi yht��n talletettavaa dataa
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
                mAutomationVector.push_back(listItem); // Lis�t��n virheellisestikin luetut listItemit, jotta k�ytt�j� saisi palautetta
            }
        }
    }
}

// T�t� kutsutaan kun esim. luetaan data tiedostosta ja tehd��n t�ysi tarkistus kaikille osille
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

    // Sitten tehd��n yhteis tarkastuksia: 1. onko samoja automaatioita useita listassa
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
    if(PrepareListItemAfterJsonRead(*listItemFromFile)) // Voidaan k�ytt�� t�t� metodia, vaikka listItemia ei olekaan luettu json-tiedostosta
    {
        // Jos luku meni hyvin, sijoitetaan annettu beta-automaatio p�ivitett�v��n otukseen
        automationListItem->itsMacroParamDataAutomation.swap(listItemFromFile->itsMacroParamDataAutomation);
        // p�ivitet��n viel� seuraava ajoaika
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
    theBetaProductAutomationList.DoFullChecks(true); // Tehd��n t��ll� tarkastelut automaatiomoodi p��ll�, my�hemmin (t�t� funktiota kutsuvassa systeemiss�) tarkastelut on teht�v� uudestaan kun oikeasti tiedet��n miss� moodissa ollaan
    return status;
}

// K�ytt�j�ll� on nyky��n kolme on-demand -nappia, joista voi k�ynnist�� halutun setin automaatioita ty�stett�v�ksi:
// 1. Jos selectedAutomationIndex:iss� on positiivinen numero, ajetaan vain sen osoittama automaatio.
// 2. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on true, ajetaan kaikki listalle olevat enbloidut automaatiot.
// 3. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on false, ajetaan kaikki listalle olevat automaatiot.
// selectedAutomationIndex -parametri on 1:st� alkava indeksi ja -1 tarkoitti siis ett� k�yd��n koko listaa l�pi.
NFmiMacroParamDataAutomationList::AutomationContainer NFmiMacroParamDataAutomationList::GetOnDemandAutomations(int selectedAutomationIndex, bool doOnlyEnabled)
{
    AutomationContainer onDemandAutomations;
    if(selectedAutomationIndex > 0)
    {
        auto actualIndex = selectedAutomationIndex - 1;
        if(actualIndex < mAutomationVector.size())
        {
            const auto& automationItem = mAutomationVector[actualIndex];
            // Automaation pit�� kuitenkin olla ilman virheit�, ett� se kelpuutetaan ajettavaksi
            if(automationItem->GetErrorStatus() == MacroParamDataStatus::kFmiListItemOk)
                onDemandAutomations.push_back(automationItem);
        }
    }
    else
    {
        for(const auto& listItem : mAutomationVector)
        {
            // Automaation pit�� kuitenkin olla ilman virheit�, ett� se kelpuutetaan ajettavaksi
            if(listItem->GetErrorStatus() == MacroParamDataStatus::kFmiListItemOk)
            {
                if(!doOnlyEnabled || listItem->fEnable)
                    onDemandAutomations.push_back(listItem);
            }
        }
    }

    return onDemandAutomations;
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

bool NFmiMacroParamDataGenerator::Init(const std::string& theBaseRegistryPath, const std::string& rootSmarttoolDirectory, const std::string& rootMacroParamDataDirectory)
{
    mBaseRegistryPath = theBaseRegistryPath;
    mRootSmarttoolDirectory = ::FixRootPath(rootSmarttoolDirectory);
    mRootMacroParamDataDirectory = ::FixRootPath(rootMacroParamDataDirectory);
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

// Lataa k�ytt��n annetusta polusta automaatio-listan.
// Asettaa polun my�s mUsedAutomationListPathString:in arvoksi.
bool NFmiMacroParamDataGenerator::LoadUsedAutomationList(const std::string& thePath)
{
    AutomationListPath(thePath);
    std::string fullPath = PathUtils::getTrueFilePath(thePath, RootMacroParamDataDirectory(), NFmiMacroParamDataGenerator::MacroParamDataListFileExtension());
    std::string errorString;
    itsUsedMacroParamDataAutomationList = NFmiMacroParamDataAutomationList(); // Nollataan k�yt�ss� ollut lista ennen uuden lukua
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

// Oletus: foundInfo on jo tarkistettu ettei se ole nullptr
static boost::shared_ptr<NFmiQueryData> MakeWantedEmptyData(boost::shared_ptr<NFmiFastQueryInfo>& foundInfo, const NFmiParamDescriptor& wantedParamDescriptor)
{
    NFmiQueryInfo metaInfo(wantedParamDescriptor, foundInfo->TimeDescriptor(), foundInfo->HPlaceDescriptor(), foundInfo->VPlaceDescriptor());
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

// Oletus: macroParamDataPtr pit�� sis�ll��n talletettavan datan.
bool NFmiMacroParamDataGenerator::StoreMacroParamData(boost::shared_ptr<NFmiQueryData>& macroParamDataPtr, const std::string& dataStorageFileFilter, int keepMaxFiles)
{
    // CheckDataStorageFileFilter funktio palauttaa virheilmoituksen, jos filefilterissa jotain vikaa.
    auto checkStr = NFmiMacroParamDataInfo::CheckDataStorageFileFilter(dataStorageFileFilter);
    if(!checkStr.empty())
    {
        throw std::runtime_error(checkStr);
    }

    auto finalDataFilePath = NFmiMacroParamDataInfo::MakeDataStorageFilePath(dataStorageFileFilter);
    // NFmiQueryData::Write heitt�� poikkeuksia, jos tulee ongelmia.
    macroParamDataPtr->Write(finalDataFilePath);
    ::CleanFilePattern(dataStorageFileFilter, keepMaxFiles);
    return true;
}

static void CalcMultiLevelSmarttoolData(NFmiSmartToolModifier& smartToolModifier, boost::shared_ptr<NFmiFastQueryInfo>& editedInfoCopy)
{
    auto usedTimeDescriptor(editedInfoCopy->TimeDescriptor());
    for(editedInfoCopy->ResetLevel(); editedInfoCopy->NextLevel();)
    {
        // jos kyseess� on level-data, pit�� l�pik�yt�v� leveli ottaa talteen, 
        // ett� smartToolModifier osaa luoda siihen osoittavia fastInfoja.
        boost::shared_ptr<NFmiLevel> theLevel(new NFmiLevel(*editedInfoCopy->Level()));
        smartToolModifier.ModifiedLevel(theLevel);

        smartToolModifier.ModifyData_ver2(&usedTimeDescriptor, false, false, nullptr);
    }
}

// Oletus: wantedMacroParamInfoPtr on tarkistettu jo ulkopuolella ett� ei ole nullptr
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

    // Jos kyseess� on ns. macroParam eli skriptiss� on lausekkeit� jossa sijoitetaan 
    // RETURN muuttujaan, t�ll�in ei haluta ajaa makro muutoksia, koska tarkoitus 
    // on vain katsella macroParam:ia karttan�yt�ll�.
    // T�nne asti tullaan ett� saadaan virhe ilmoitus tulkinta vaiheessa talteen.
    // Mutta suoritus vaiheen virheet menev�t t�ll� hetkell� vain loki tiedostoon.
    if(smartToolModifier.IsInterpretedSkriptMacroParam())
    {
        throw std::runtime_error(std::string("Used script '") + mUsedAbsoluteSmarttoolPath + "' was macroParam type. You have to use smarttool scripts when generating MacroParam data, not macroParam scripts, even though data's name might suggest otherwise");
    }

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

// Jos annettu polku on suhteellinen, lis�t��n siihen smarttool juuri hakemisto.
// Korjataan polku kaikin puolin oikeaan formaattiin (erottimet ja mahd. .. tai . jutut).
std::string NFmiMacroParamDataGenerator::MakeUsedAbsoluteSmarttoolPathString(const std::string& smarttoolPath) const
{
    return PathUtils::getAbsoluteFilePath(smarttoolPath, mRootSmarttoolDirectory);
}

// Jos annettu polku on absoluuttinen, poistetaan siit� smarttool juuri hakemisto osio alusta.
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

// K�ytt�j�ll� on nyky��n kolme on-demand -nappia, joista voi k�ynnist�� halutun setin automaatioita ty�stett�v�ksi:
// 1. Jos selectedAutomationIndex:iss� on positiivinen numero, ajetaan vain sen osoittama automaatio.
// 2. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on true, ajetaan kaikki listalle olevat enbloidut automaatiot.
// 3. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on false, ajetaan kaikki listalle olevat automaatiot.
// selectedAutomationIndex -parametri on 1:st� alkava indeksi ja -1 tarkoitti siis ett� k�yd��n koko listaa l�pi.
bool NFmiMacroParamDataGenerator::DoOnDemandBetaAutomations(int selectedAutomationIndex, bool doOnlyEnabled, NFmiThreadCallBacks* threadCallBacks)
{
    DataGenerationIsOnHandler dataGenerationIsOnHandler(fDataGenerationIsOn);
    bool status = false;
    auto onDemandAutomations = itsUsedMacroParamDataAutomationList.GetOnDemandAutomations(selectedAutomationIndex, doOnlyEnabled);
    if(!onDemandAutomations.empty())
    {
        for(const auto& automationItem : onDemandAutomations)
        {
            status |= GenerateMacroParamData(*automationItem->itsMacroParamDataAutomation, threadCallBacks);
        }
    }
    if(threadCallBacks)
        threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FINISHED);
    return status;
}

bool NFmiMacroParamDataGenerator::GenerateMacroParamData(NFmiThreadCallBacks* threadCallBacks)
{
    DataGenerationIsOnHandler dataGenerationIsOnHandler(fDataGenerationIsOn);
    auto dataInfo = MakeDataInfo();
    auto result = GenerateMacroParamData(dataInfo, threadCallBacks);
    if(threadCallBacks)
        threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FINISHED);
    return result;
}

bool NFmiMacroParamDataGenerator::GenerateMacroParamData(const NFmiMacroParamDataInfo& dataInfo, NFmiThreadCallBacks* threadCallBacks)
{
    try
    {
        auto baseDataCheck = NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(dataInfo.BaseDataParamProducerString(), false);
        if(!baseDataCheck.first.empty())
        {
            throw std::runtime_error(baseDataCheck.first);
        }

        auto* infoOrganizer = ::GetInfoOrganizer();
        if(!infoOrganizer)
        {
            throw std::runtime_error("GenerateMacroParamData: Can't get proper InfoOrganizer into works, unknown error in the system");
        }

        // NFmiExtraMacroParamData::FindWantedInfo metodin 3. parametri (false) tarkoittaa ett� pohjadataksi 
        // ei kelpuuteta asemadataa, vain hiladata kelpaa.
        auto wantedInfo = NFmiExtraMacroParamData::FindWantedInfo(*infoOrganizer, baseDataCheck.second, false);
        if(!wantedInfo.foundInfo_)
        {
            throw std::runtime_error(std::string("GenerateMacroParamData: unable to fing requested base data with '") + dataInfo.BaseDataParamProducerString() + "' parameter, unable to generate wanted macroParam data");
        }

        auto wantedParamDescriptor = ::MakeWantedParamDescriptor(dataInfo);
        auto wantedMacroParamDataPtr = ::MakeWantedEmptyData(wantedInfo.foundInfo_, wantedParamDescriptor);
        if(!wantedMacroParamDataPtr)
        {
            throw std::runtime_error("GenerateMacroParamData: Unable to generate actual macroParam querydata, unknown error in system");
        }
        boost::shared_ptr<NFmiFastQueryInfo> wantedMacroParamInfoPtr(new NFmiFastQueryInfo(wantedMacroParamDataPtr.get()));

        auto smarttoolContent = ReadSmarttoolContentFromFile(dataInfo.DataGeneratingSmarttoolPathString());

        if(CalculateDataWithSmartTool(wantedMacroParamInfoPtr, infoOrganizer, smarttoolContent, threadCallBacks))
        {
            return StoreMacroParamData(wantedMacroParamDataPtr, dataInfo.DataStorageFileFilter(), dataInfo.MaxGeneratedFilesKept());
        }
    }
    catch(std::exception&)
    {
        // Raportoi ongelmasta
    }

    return false;
}
