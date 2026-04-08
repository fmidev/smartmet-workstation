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
#include "NFmiFileSystem.h"
#include "NFmiPathUtils.h"
#include "NFmiSmartInfo.h"
#include "NFmiBetaProductSystem.h"
#include "NFmiTimeList.h"
#include "NFmiValueString.h"
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

    std::size_t length_between_two(std::size_t a, std::size_t b)
    {
        if(b < a) return 0;

        return b - a;
    }

    std::string text_between_two(std::string s, std::string a, std::string b)
    {
        std::size_t aPos = s.find(a);
        std::size_t bPos = s.find(b);
        if(aPos == std::string::npos || bPos == std::string::npos)
            return "";

        // The + a.length() is necessary here so we don't get the text that makes up a
        std::size_t length = ::length_between_two(aPos + a.length(), bPos);

        // See previous comment.
        std::string sub = s.substr(aPos + a.length(), length);
        return sub;
    }

    bool isNumeric(const std::string& str) 
    {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    }

    NFmiParam MakeWantedParam(const std::string& part1, const std::string& part2)
    {
        auto usedParamIdStr = part1;
        FmiInterpolationMethod usedInterpolation = kLinearly;
        std::string interpolationMethodStr = ::text_between_two(usedParamIdStr, "{", "}");
        if(!interpolationMethodStr.empty())
        {
            auto interpolationNumber = (FmiInterpolationMethod)std::stoi(interpolationMethodStr);
            if(interpolationNumber >= kLinearly && interpolationNumber <= kNearestPoint)
            {
                usedInterpolation = interpolationNumber;
            }
            else
            {
                throw std::runtime_error(std::string("Illegal value for parameter's interpolation method: ") + interpolationMethodStr);
            }
            auto bracesStartPos = usedParamIdStr.find('{');
            usedParamIdStr = usedParamIdStr.substr(0, bracesStartPos);
        }

        if(!::isNumeric(usedParamIdStr))
        {
            throw std::runtime_error(std::string("Illegal value for parameter's id: ") + usedParamIdStr);
        }
        unsigned long paramId = std::stoul(usedParamIdStr);

        return NFmiParam(paramId, boost::trim_copy(part2), kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", usedInterpolation);
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
            logMessageStart += std::to_string(keepMaxFiles);
            logMessageStart += ",\nDeleted files:\n";
            CtrlViewUtils::DeleteFilesWithPatternAndLog(theFilePattern, logMessageStart, CatLog::Severity::Debug, CatLog::Category::Data, keepMaxFiles);
        }
        catch(...)
        {
            // ei tehd� toistaiseksi mit��n...
        }
    }

    std::string FixBaseDataGridScaleString(const std::string& newValue)
    {
        // Korjataan annettu newValue NFmiMacroParamDataInfo olion tarkastelujen kautta
        NFmiMacroParamDataInfo tmpData;
        tmpData.BaseDataGridScaleString(newValue);
        return tmpData.BaseDataGridScaleString();
    }
}

// ***********************************************************
// ************** NFmiMacroParamDataInfo *********************
// ***********************************************************

NFmiMacroParamDataInfo::NFmiMacroParamDataInfo() = default;

NFmiMacroParamDataInfo::NFmiMacroParamDataInfo(const std::string& baseDataParamProducerLevelString, const std::string& usedProducerString, const std::string& dataGeneratingSmarttoolPathListString, const std::string& usedParameterListString, const std::string& dataStorageFileFilter, const std::string& dataTriggerList, int maxGeneratedFilesKept, const std::string& baseDataGridScaleString)
:mBaseDataParamProducerString(baseDataParamProducerLevelString)
,mUsedProducerString(usedProducerString)
,mDataGeneratingSmarttoolPathListString(dataGeneratingSmarttoolPathListString)
,mUsedParameterListString(usedParameterListString)
,mDataStorageFileFilter(dataStorageFileFilter)
,mDataTriggerList(dataTriggerList)
,mMaxGeneratedFilesKept(maxGeneratedFilesKept)
,mBaseDataGridScaleString(baseDataGridScaleString)
{
    CorrectMaxGeneratedFilesKeptValue();
    BaseDataGridScaleString(baseDataGridScaleString);
}

void NFmiMacroParamDataInfo::MaxGeneratedFilesKept(int newValue)
{ 
    mMaxGeneratedFilesKept = FixMaxGeneratedFilesKeptValue(newValue);
}

void NFmiMacroParamDataInfo::BaseDataGridScaleString(const std::string& newValue)
{
    auto checkResult = CalcBaseDataGridScaleValues(newValue);
    mBaseDataGridScaleValues = checkResult.second;
    if(checkResult.first)
    {
        mBaseDataGridScaleString = newValue;
    }
    else
    {
        // Jos tarkastelu tuotti virheen ja tuli joku default/korjattu 
        // arvo, korjataan perusstringi sen mukaan
        mBaseDataGridScaleString = MakeBaseDataGridScaleString(mBaseDataGridScaleValues);
    }
}

void NFmiMacroParamDataInfo::CorrectBaseDataGridScaleValue()
{
    BaseDataGridScaleString(mBaseDataGridScaleString);
}

static std::pair<bool, double> FixBaseDataGridScaleValue(double newValue)
{
    if(newValue < 1)
        return std::make_pair(false, 1);
    else if(newValue > 10)
        return std::make_pair(false, 10);

    return std::make_pair(true, newValue);
}


std::pair<bool, NFmiPoint> NFmiMacroParamDataInfo::CalcBaseDataGridScaleValues(const std::string& baseDataGridScaleString)
{
    auto scaleStrings = ::GetSplittedAndTrimmedStrings(baseDataGridScaleString, ",");
    if(scaleStrings.empty())
        return std::make_pair(false, gDefaultBaseDataGridScaleValues);
    try
    {
        auto checkResultX = ::FixBaseDataGridScaleValue(std::stod(scaleStrings[0]));
        auto status = checkResultX.first;
        double scaleX = checkResultX.second;
        double scaleY = scaleX;
        if(scaleStrings.size() > 1)
        {
            auto checkResultY = ::FixBaseDataGridScaleValue(std::stod(scaleStrings[1]));
            status &= checkResultY.first;
            scaleY = checkResultY.second;
        }
        return std::make_pair(status, NFmiPoint(scaleX, scaleY));
    }
    catch(...)
    {}
    return std::make_pair(false, gDefaultBaseDataGridScaleValues);
}

std::string NFmiMacroParamDataInfo::MakeBaseDataGridScaleString(NFmiPoint baseDataGridScaleValues)
{
    const int maxDecimals = 5;
    baseDataGridScaleValues.X(::FixBaseDataGridScaleValue(baseDataGridScaleValues.X()).second);
    baseDataGridScaleValues.Y(::FixBaseDataGridScaleValue(baseDataGridScaleValues.Y()).second);
    if(baseDataGridScaleValues.X() == baseDataGridScaleValues.Y())
    {
        return std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(baseDataGridScaleValues.X(), maxDecimals));
    }

    std::string str(NFmiValueString::GetStringWithMaxDecimalsSmartWay(baseDataGridScaleValues.X(), maxDecimals).CharPtr());
    str += ",";
    str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(baseDataGridScaleValues.Y(), maxDecimals).CharPtr();
    return str;
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
        auto checkResult = NFmiMacroParamDataInfo::CheckDataGeneratingSmarttoolPathListString(mDataGeneratingSmarttoolPathListString);
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

    {
        auto checkResult = NFmiMacroParamDataInfo::CheckBaseDataGridScaleString(mBaseDataGridScaleString);
        if(!checkResult.first.empty())
        {
            itsCheckShortStatusStr = "Grid scale error";
            return false;
        }
        mBaseDataGridScaleValues = checkResult.second;
    }

    itsCheckShortStatusStr = "Ok";
    return true;
}

std::string NFmiMacroParamDataInfo::MakeDataStorageFilePath(const std::string& dataStorageFileFilter)
{
    // Tehd��n lokaali ajasta aikaleima, jossa mukana my�s sekunnit
    NFmiTime makeTime;
    std::string timeStampStr(makeTime.ToStr(kYYYYMMDDHHMMSS).CharPtr());
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
    else
    {
        try
        {
            const auto& producerIdStr = parts[0];
            if(!::isNumeric(producerIdStr))
            {
                throw std::runtime_error(std::string("Illegal value for producer's id: ") + producerIdStr);
            }

            unsigned long producerId = std::stoul(parts[0]);
        }
        catch(std::exception&)
        {
            checkStr = std::string("CheckUsedProducerString: unable to convert produced id part to integer number in '" + usedProducerString + "', string should be in format of 'producer-id-integer,name-string'");
        }
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
    auto filename = std::filesystem::path(dataStorageFileFilter).stem().string();
    // 1. Tiedoston nimess� pit�� olla '*' merkki aikaleimaa varten.
    if(filename.find('*') == std::string::npos)
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output filename '" + filename + "' (in " + dataStorageFileFilter + ") doesn't have ' * ' character place marker for file's creation time stamp. To store MacroParam data you need to have that asterisk in filename.";
    }

    // 2. Tiedoston nimess� pit�� olla v�hint��n 3 alpha-numeerista merkki�, jotta esim. pelkk� path\*.sqd ei kelpaa
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
    // 3. Tiedoston extension pit�� olla sqd tyyppinen
    if(!boost::iequals(fileExtension, gQueryDataFileExtension))
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output extension '" + fileExtension + "' (in " + dataStorageFileFilter + ") is invalid, extension must be '" + gQueryDataFileExtension + "' type.";
    }

    return "";
}

const std::string gSmarttoolFileExtension = ".st";

std::string NFmiMacroParamDataInfo::CheckDataGeneratingSmarttoolPathListString(const std::string& dataGeneratingSmarttoolPathListString)
{
    auto paths = ::GetSplittedAndTrimmedStrings(dataGeneratingSmarttoolPathListString, ",");
    bool anyPathExist = false;
    for(const auto& pathStr : paths)
    {
        if(!::FileExists(pathStr))
        {
            auto absoluteSmarttoolPath = PathUtils::getAbsoluteFilePath(pathStr, NFmiMacroParamDataGenerator::RootSmarttoolDirectory());
            if(!::FileExists(absoluteSmarttoolPath))
            {
                return std::string("CheckDataGeneratingSmarttoolPathString") + ": Given smarttool file path '" + pathStr + "' doesn't exists, make sure that given path for MacroParam data generating smarttool was correct!";
            }
        }

        auto fileExtension = ::GetFileExtension(pathStr);
        // 3. Tiedoston extension pit�� olla st tyyppinen
        if(!boost::iequals(fileExtension, gSmarttoolFileExtension))
        {
            return std::string("CheckDataGeneratingSmarttoolPathString") + ": Given data output extension '" + fileExtension + "' (in " + pathStr + ") is invalid, extension must be '" + gSmarttoolFileExtension + "' type.";
        }
        anyPathExist = true;
    }

    if(!anyPathExist)
    {
        return std::string("CheckDataGeneratingSmarttoolPathString") + ": no valid path was given with input '" + dataGeneratingSmarttoolPathListString + "'";
    }

    return "";
}

std::pair<std::string, std::vector<NFmiDefineWantedData>> NFmiMacroParamDataInfo::CheckDataTriggerListString(const std::string& dataTriggerListString)
{
    const std::vector<NFmiDefineWantedData> dummyEmptyData;

    auto parts = ::GetSplittedAndTrimmedStrings(dataTriggerListString, ",");
    // Tyhj� on ok
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

static bool IsOneDimensionalGridScaleCase(const std::string& baseDataGridScaleString, const std::string& correctedString)
{
    auto scaleParts = ::GetSplittedAndTrimmedStrings(baseDataGridScaleString, ",");
    if(scaleParts.size() != 2)
        return false;

    return (scaleParts[0] == correctedString) && (scaleParts[1] == correctedString);
}

std::pair<std::string, NFmiPoint> NFmiMacroParamDataInfo::CheckBaseDataGridScaleString(const std::string& baseDataGridScaleString)
{
    NFmiMacroParamDataInfo tmpData;
    tmpData.BaseDataGridScaleString(baseDataGridScaleString);
    auto correctedValue = tmpData.BaseDataGridScaleString();
    if(baseDataGridScaleString != correctedValue)
    {
        if(!::IsOneDimensionalGridScaleCase(baseDataGridScaleString, correctedValue))
        {
            auto errorString = std::string("CheckBaseDataGridScaleString") + ": given value '" + baseDataGridScaleString + "' had invalid form, should be like '1' or '1.5' or '1.5,2.0'";
            return std::make_pair(errorString, NFmiPoint::gMissingLatlon);
        }
    }

    return std::make_pair("", tmpData.BaseDataGridScaleValues());
}

std::string NFmiMacroParamDataInfo::MakeShortStatusErrorString()
{
    CheckData();
    return itsCheckShortStatusStr;
}


static const std::string gJsonName_BaseDataParamProducer = "BaseDataParamProducer";
static const std::string gJsonName_UsedProducer = "UsedProducer";
static const std::string gJsonName_DataGeneratingSmarttoolPathList = "DataGeneratingSmarttoolPathList";
static const std::string gJsonName_UsedParameterList = "UsedParameterList";
static const std::string gJsonName_DataStorageFileFilter = "DataStorageFileFilter";
static const std::string gJsonName_DataTriggerList = "DataTriggerList";
static const std::string gJsonName_MaxGeneratedFilesKept = "mMaxGeneratedFilesKept";
static const std::string gJsonName_BaseDataGridScale = "BaseDataGridScale";

json_spirit::Object NFmiMacroParamDataInfo::MakeJsonObject(const NFmiMacroParamDataInfo& macroParamDataInfo)
{
    json_spirit::Object jsonObject;
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.BaseDataParamProducerString(), gJsonName_BaseDataParamProducer, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.UsedProducerString(), gJsonName_UsedProducer, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.DataGeneratingSmarttoolPathListString(), gJsonName_DataGeneratingSmarttoolPathList, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.UsedParameterListString(), gJsonName_UsedParameterList, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.DataStorageFileFilter(), gJsonName_DataStorageFileFilter, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.DataTriggerList(), gJsonName_DataTriggerList, jsonObject);
    jsonObject.push_back(json_spirit::Pair(gJsonName_MaxGeneratedFilesKept, macroParamDataInfo.MaxGeneratedFilesKept()));
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.BaseDataGridScaleString(), gJsonName_BaseDataGridScale, jsonObject);

    return jsonObject;
}

void NFmiMacroParamDataInfo::ParseJsonPair(json_spirit::Pair& thePair)
{
    // T�ss� puretaan NFmiMacroParamDataInfo luokan p��tason pareja.
    if(thePair.name_ == gJsonName_BaseDataParamProducer)
        mBaseDataParamProducerString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_UsedProducer)
        mUsedProducerString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_DataGeneratingSmarttoolPathList)
        mDataGeneratingSmarttoolPathListString = thePair.value_.get_str();
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
    else if(thePair.name_ == gJsonName_BaseDataGridScale)
    {
        mBaseDataGridScaleString = thePair.value_.get_str();
        CorrectBaseDataGridScaleValue();
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
        return std::filesystem::path(itsMacroParamDataAutomationPath).stem().string();
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
NFmiUserWorkAutomationContainer NFmiMacroParamDataAutomationList::GetOnDemandAutomations(int selectedAutomationIndex, bool doOnlyEnabled)
{
    NFmiUserWorkAutomationContainer onDemandAutomations;
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
            // Kopsataan my�h�stetty automaatio ajettavien listalle
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

NFmiUserWorkAutomationContainer NFmiMacroParamDataAutomationList::GetDueAutomations(const NFmiMetTime& theCurrentTime, const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer)
{
    NFmiUserWorkAutomationContainer dueAutomations;

    for(auto& listItem : mAutomationVector)
    {
        if(listItem->fEnable)
        {
            if(listItem->GetErrorStatus() == MacroParamDataStatus::kFmiListItemOk)
            {
                int postponeTriggerInMinutes = 0;
                if(NFmiBetaProductAutomation::NFmiTriggerModeInfo::HasDataTriggerBeenLoaded(listItem->itsMacroParamDataAutomation->WantedDataTriggerList(), loadedDataTriggerList, infoOrganizer, listItem->AutomationName(), postponeTriggerInMinutes, false))
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
const std::string NFmiMacroParamDataGenerator::itsGeneratedDataFileExtension = "sqd";
const std::string NFmiMacroParamDataGenerator::itsGeneratedDataFileFilter = "Generated data Files (*." + NFmiMacroParamDataGenerator::itsGeneratedDataFileExtension + ")|*." + NFmiMacroParamDataGenerator::itsGeneratedDataFileExtension + "|All Files (*.*)|*.*||";

NFmiStopFunctor NFmiMacroParamDataGenerator::itsStopFunctor;
NFmiThreadCallBacks NFmiMacroParamDataGenerator::itsThreadCallBacks = NFmiThreadCallBacks(&NFmiMacroParamDataGenerator::itsStopFunctor, nullptr);

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
#ifndef UNIX
    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;
    // Beta product section
    std::string macroParamDataGeneratorSectionName = "\\MacroParamDataGenerator";
    mDialogBaseDataParamProducerString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\BaseDataParamProducerLevel", usedKey, "T_ec");
    mDialogUsedProducerString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\UsedProducer", usedKey, "3001,ProducerName");
    mDialogDataGeneratingSmarttoolPathListString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\DataGeneratingSmarttoolPathList", usedKey, "smarttool.st");
    mDialogUsedParameterListString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\UsedParameterListString", usedKey, "6201, Param1, 6202, Param2");
    mDialogDataStorageFileFilter = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\DataStorageFileFilter", usedKey, "C:\\data\\*_mydata.sqd");
    mMacroParamDataInfoSaveInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\MacroParamDataInfoSaveInitialPath", usedKey, "C:\\data\\");
    mMacroParamDataAutomationListSaveInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\MacroParamDataAutomationListSaveInitialPath", usedKey, "C:\\data\\");
    mDialogDataTriggerList = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\DataTriggerList", usedKey, "");
    mDialogMaxGeneratedFilesKept = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\MaxGeneratedFilesKept", usedKey, 2);
    // MaxGeneratedFilesKept arvo pit�� korjata varmuuden vuoksi
    *mDialogMaxGeneratedFilesKept = NFmiMacroParamDataInfo::FixMaxGeneratedFilesKeptValue(*mDialogMaxGeneratedFilesKept);
    mAutomationModeOn = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\AutomationModeOn", usedKey, false);
    mAutomationPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\AutomationPath", usedKey, "");
    mAutomationListPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\AutomationListPath", usedKey, "");
    mMacroParamDataAutomationAddSmarttoolInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\AddSmarttoolInitialPath", usedKey, mRootSmarttoolDirectory);
    mDialogBaseDataGridScaleString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\BaseDataGridScale", usedKey, gDefaultBaseDataGridScaleString);
    // BaseDataGridScaleString pit�� korjata varmuuden vuoksi
    *mDialogBaseDataGridScaleString = ::FixBaseDataGridScaleString(*mDialogBaseDataGridScaleString);
    mDialogCpuUsagePercentage = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\CpuUsagePercentage", usedKey, NFmiSmartToolModifier::DefaultUsedCpuCapacityPercentageInCalculations);
    // CpuUsagePercentage pit�� korjata varmuuden vuoksi
    *mDialogCpuUsagePercentage = NFmiSmartToolModifier::FixCpuCapacityPercentageInCalculations(*mDialogCpuUsagePercentage);
    mGeneratedDataStorageInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\GeneratedDataStorageInitialPath", usedKey, "C:\\data\\");
#endif // !UNIX

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

static NFmiVPlaceDescriptor MakeCorrectVPlaceDescriptor(std::shared_ptr<NFmiFastQueryInfo>& foundInfo)
{
    if(foundInfo->SizeLevels() <= 1)
        return foundInfo->VPlaceDescriptor();

    // Level datasta pit�� tehd� surfacedata tyyppinen
    NFmiLevelBag levelBag(kFmiAnyLevelType, 0, 0, 1);
    return NFmiVPlaceDescriptor(levelBag);
}

static NFmiHPlaceDescriptor MakeScaledHPlaceDescriptor(std::shared_ptr<NFmiFastQueryInfo>& foundInfo, const NFmiPoint& scale)
{
    if(scale == gDefaultBaseDataGridScaleValues)
        return foundInfo->HPlaceDescriptor();

    auto usedGridSizeX = boost::math::iround(foundInfo->GridXNumber() / scale.X());
    auto usedGridSizeY = boost::math::iround(foundInfo->GridYNumber() / scale.Y());
    NFmiGrid usedGrid(foundInfo->Area(), usedGridSizeX, usedGridSizeY);
    return NFmiHPlaceDescriptor(usedGrid);
}

// Oletus: foundInfo on jo tarkistettu ettei se ole nullptr
static std::shared_ptr<NFmiQueryData> MakeWantedEmptyData(std::shared_ptr<NFmiFastQueryInfo>& foundInfo, const NFmiParamDescriptor& wantedParamDescriptor, const NFmiPoint &scale)
{
    auto correctVPlaceDescriptor = ::MakeCorrectVPlaceDescriptor(foundInfo);
    auto scaledHPlaceDescriptor = ::MakeScaledHPlaceDescriptor(foundInfo, scale);
    NFmiQueryInfo metaInfo(wantedParamDescriptor, foundInfo->TimeDescriptor(), scaledHPlaceDescriptor, correctVPlaceDescriptor);
    return std::shared_ptr<NFmiQueryData>(NFmiQueryDataUtil::CreateEmptyData(metaInfo));
}

static NFmiInfoOrganizer* GetInfoOrganizer()
{
    return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->InfoOrganizer();
}

// filePathList parametrissa on pilkulla eroteltu lista polkuja eri smarttool skripteihin.
// Luetaan kaikkien niiden sis�lt� ja laitetaan ne palautettavaan vector:iin.
// Jos yhdenkin polun kanssa tulee ongelmia, heitet��n poikkeus ja lopetetaan.
std::vector<std::string> NFmiMacroParamDataGenerator::ReadSmarttoolContentsFromFiles(const std::string& filePathList)
{
    std::vector<std::string> fileContentList;
    mUsedAbsoluteSmarttoolPathList.clear();
    auto filePaths = ::GetSplittedAndTrimmedStrings(filePathList, ",");
    for(const auto& filePath : filePaths)
    {
        auto usedAbsoluteSmarttoolPath = MakeUsedAbsoluteSmarttoolPathString(filePath);
        std::string fileContent;
        if(NFmiFileSystem::ReadFile2String(usedAbsoluteSmarttoolPath, fileContent))
        {
            fileContentList.push_back(fileContent);
            mUsedAbsoluteSmarttoolPathList.push_back(usedAbsoluteSmarttoolPath);
        }
        else
            throw std::runtime_error(std::string(__FUNCTION__) + " unable to read smarttool file content from given file: " + usedAbsoluteSmarttoolPath);
    }
    return fileContentList;
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
    tmpDataFilePath += std::filesystem::path(finalDataFilePath).filename().string();
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
    logStr += std::filesystem::path(fullAutomationPath).stem().string();
    logStr += "' MacroParam-data (";
    logStr += timeLasted;
    logStr += "), saved to file '";
    logStr += finalDataFilePath;
    logStr += "', from automation: ";
    logStr += fullAutomationPath;
    CatLog::logMessage(logStr, CatLog::Severity::Info, CatLog::Category::Operational);
}

// Oletus: macroParamDataPtr pit�� sis�ll��n talletettavan datan.
// T�t� funktiota ei keskeytet� (k�ytt�j�n pyynt�/ohjelman lopetus), koska tiedostojen 
// j�lkien siivonta tekisi koodista sotkuista. T��ll� on vain yksi hidas toimenpide eli qdata->Write jota ei voi keskeytt��.
// Tiedostojen kirjoitus on kuitenkin suht nopeaa (< 1 s useimmiten), varsinkin jos SSD k�yt�ss�.
bool NFmiMacroParamDataGenerator::StoreMacroParamData(std::shared_ptr<NFmiQueryData>& macroParamDataPtr, const std::string& dataStorageFileFilter, int keepMaxFiles, const std::string& fullAutomationPath, NFmiMilliSecondTimer &timer)
{
    // CheckDataStorageFileFilter funktio palauttaa virheilmoituksen, jos filefilterissa jotain vikaa.
    auto checkStr = NFmiMacroParamDataInfo::CheckDataStorageFileFilter(dataStorageFileFilter);
    if(!checkStr.empty())
    {
        throw std::runtime_error(checkStr);
    }

    // Jos dataStorageFileFilter:issa ei ole drive-letteria ja kyse on muuten 
    // absoluuttisesta polusta, lis�t��n siihen mMacroParamDataTmpDirectory drive-letter.
    auto usedDataStorageFileFilter = PathUtils::fixMissingDriveLetterToAbsolutePath(dataStorageFileFilter, mMacroParamDataTmpDirectory);
    auto finalDataFilePath = NFmiMacroParamDataInfo::MakeDataStorageFilePath(usedDataStorageFileFilter);
    auto tmpDataFilePath = ::MakeTmpDataFilePath(finalDataFilePath, mMacroParamDataTmpDirectory);

    // Talletetaan ensin tmp hakemistoon, ja sitten vasta rename:lla laitetaan lopulliseen hakemistoon.
    // Tarkoitus on ett� data siirtyy Dropboxiin valmiina ja t�ll�in Dropbox ei ala jakamaan 
    // keskener�isesti viel� kirjoituksessa olevaa dataa.
    // NFmiQueryData::Write heitt�� poikkeuksia, jos tulee ongelmia.
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
    ::CleanFilePattern(usedDataStorageFileFilter, keepMaxFiles);
    return true;
}

static void CalcMultiLevelSmarttoolData(NFmiSmartToolModifier& smartToolModifier, std::shared_ptr<NFmiFastQueryInfo>& editedInfoCopy)
{
    auto usedTimeDescriptor(editedInfoCopy->TimeDescriptor());
    for(editedInfoCopy->ResetLevel(); editedInfoCopy->NextLevel();)
    {
        // jos kyseess� on level-data, pit�� l�pik�yt�v� leveli ottaa talteen, 
        // ett� smartToolModifier osaa luoda siihen osoittavia fastInfoja.
        std::shared_ptr<NFmiLevel> theLevel(new NFmiLevel(*editedInfoCopy->Level()));
        smartToolModifier.ModifiedLevel(theLevel);

        smartToolModifier.ModifyData_ver2(&usedTimeDescriptor, false, false, nullptr);
    }
}

// Oletus: wantedMacroParamInfoPtr on tarkistettu jo ulkopuolella ett� ei ole nullptr
bool NFmiMacroParamDataGenerator::CalculateDataWithSmartTool(std::shared_ptr<NFmiFastQueryInfo>& wantedMacroParamInfoPtr, NFmiInfoOrganizer* infoOrganizer, const std::vector<std::string>& smartToolContentList, NFmiThreadCallBacks* threadCallBacks)
{
    itsSmarttoolCalculationLogStr.clear();
    if(smartToolContentList.size() != mUsedAbsoluteSmarttoolPathList.size())
    {
        throw std::runtime_error("MacroParam-data generation error: smarttool script content and used smarttool path counts are different sizes, stopping...");
    }

    bool status = false;
    for(size_t index = 0; index < smartToolContentList.size(); index++)
    {
        auto usedSmarttoolPath = mUsedAbsoluteSmarttoolPathList[index];
        try
        {
            // Riitt�� ett� yhdenkin smarttoolin laskut menev�t l�pi, jotta palautetaan 
            // true ja data talletetaan.
            status |= CalculateDataWithSmartTool(wantedMacroParamInfoPtr, infoOrganizer, smartToolContentList[index], usedSmarttoolPath, threadCallBacks);
        }
        catch(std::exception& e)
        {
            // Lokitetaan normi virheet t�ss� ja jatketaan
            std::string errorStr = "CalculateDataWithSmartTool failed: ";
            errorStr += e.what();
            CatLog::logMessage(errorStr, CatLog::Severity::Error, CatLog::Category::Operational, true);
        }
    }
    return status;
}

// Oletus: wantedMacroParamInfoPtr on tarkistettu jo ulkopuolella ett� ei ole nullptr
bool NFmiMacroParamDataGenerator::CalculateDataWithSmartTool(std::shared_ptr<NFmiFastQueryInfo>& wantedMacroParamInfoPtr, NFmiInfoOrganizer* infoOrganizer, const std::string& smartToolContent, const std::string& usedSmartToolPath, NFmiThreadCallBacks *threadCallBacks)
{
    NFmiSmartToolModifier smartToolModifier(infoOrganizer);
    try // ensin tulkitaan macro
    {
        smartToolModifier.SetGriddingHelper(CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetGriddingHelper());
        smartToolModifier.IncludeDirectory(mRootSmarttoolDirectory);
        smartToolModifier.InitSmartTool(smartToolContent);
#ifndef UNIX
        smartToolModifier.SetFixedEditedData(wantedMacroParamInfoPtr);
        smartToolModifier.UsedCpuCapacityPercentageInCalculations(DialogCpuUsagePercentage());
#endif
    }
    catch(std::exception& e)
    {
        itsSmarttoolCalculationLogStr = e.what();
        std::string errorString = e.what();
        errorString += ", in macro '";
        errorString += usedSmartToolPath;
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
        throw std::runtime_error(std::string("Used script '") + usedSmartToolPath + "' was macroParam type. You have to use smarttool scripts when generating MacroParam data, not macroParam scripts, even though data's name might suggest otherwise");
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
            errorString += usedSmartToolPath;
            errorString += "'";
            throw std::runtime_error(errorString);
        }
    }
    catch(std::exception& e)
    {
        itsSmarttoolCalculationLogStr = e.what();
        std::string errorString = e.what();
        errorString += ", in macro '";
        errorString += usedSmartToolPath;
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

std::string NFmiMacroParamDataGenerator::DialogDataGeneratingSmarttoolPathListString() const
{
    return *mDialogDataGeneratingSmarttoolPathListString;
}

void NFmiMacroParamDataGenerator::DialogDataGeneratingSmarttoolPathListString(const std::string& newValue)
{
    *mDialogDataGeneratingSmarttoolPathListString = newValue;
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

std::string NFmiMacroParamDataGenerator::MacroParamDataAutomationAddSmarttoolInitialPath() const
{
    return *mMacroParamDataAutomationAddSmarttoolInitialPath;
}

void NFmiMacroParamDataGenerator::MacroParamDataAutomationAddSmarttoolInitialPath(const std::string& newValue)
{
    *mMacroParamDataAutomationAddSmarttoolInitialPath = newValue;
}

std::string NFmiMacroParamDataGenerator::DialogBaseDataGridScaleString() const
{
    return *mDialogBaseDataGridScaleString;
}

void NFmiMacroParamDataGenerator::DialogBaseDataGridScaleString(const std::string& newValue)
{
    *mDialogBaseDataGridScaleString = ::FixBaseDataGridScaleString(newValue);
}

double NFmiMacroParamDataGenerator::DialogCpuUsagePercentage() const
{
    return *mDialogCpuUsagePercentage;
}

void NFmiMacroParamDataGenerator::DialogCpuUsagePercentage(double newValue) const
{
#ifndef UNIX
    *mDialogCpuUsagePercentage = NFmiSmartToolModifier::FixCpuCapacityPercentageInCalculations(newValue);
#else
    *mDialogCpuUsagePercentage = newValue;
#endif
}

std::string NFmiMacroParamDataGenerator::GeneratedDataStorageInitialPath() const
{
    return *mGeneratedDataStorageInitialPath;
}

void NFmiMacroParamDataGenerator::GeneratedDataStorageInitialPath(const std::string& newValue)
{
    *mGeneratedDataStorageInitialPath = newValue;
}

NFmiMacroParamDataInfo NFmiMacroParamDataGenerator::MakeDataInfo() const
{
    return NFmiMacroParamDataInfo(DialogBaseDataParamProducerString(), DialogUsedProducerString(), DialogDataGeneratingSmarttoolPathListString(), DialogUsedParameterListString(), DialogDataStorageFileFilter(), DialogDataTriggerList(), DialogMaxGeneratedFilesKept(), DialogBaseDataGridScaleString());
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
        // K�ytt�j� haluaa lopettaa
        std::string errorStr = "Generating MacroParam-data was canceled due the user has pressed the Cancel button.";
        CatLog::logMessage(errorStr, CatLog::Severity::Error, CatLog::Category::Operational, true);
    }
    else if(smartmetIsBeenClosed)
    {
        // Smartmetin sulkeminen vaatii ett� datan generointi lopetetaan
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

// K�ytt�j�ll� on nyky��n kolme on-demand -nappia, joista voi k�ynnist�� halutun setin automaatioita ty�stett�v�ksi:
// 1. Jos selectedAutomationIndex:iss� on positiivinen numero, ajetaan vain sen osoittama automaatio.
// 2. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on true, ajetaan kaikki listalle olevat enbloidut automaatiot.
// 3. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on false, ajetaan kaikki listalle olevat automaatiot.
// selectedAutomationIndex -parametri on 1:st� alkava indeksi ja -1 tarkoitti siis ett� k�yd��n koko listaa l�pi.
bool NFmiMacroParamDataGenerator::DoOnDemandBetaAutomations(int selectedAutomationIndex, bool doOnlyEnabled, NFmiThreadCallBacks* threadCallBacks)
{
    try
    {
        DataGenerationIsOnHandler dataGenerationIsOnHandler(fDataGenerationIsOn);
        bool status = false;
        auto onDemandAutomations = itsUsedMacroParamDataAutomationList.GetOnDemandAutomations(selectedAutomationIndex, doOnlyEnabled);
        status = GenerateAutomationsData(onDemandAutomations, threadCallBacks);
        if(threadCallBacks)
        {
            if(status)
                threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FINISHED);
            else
                threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FAILED);
        }
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

bool NFmiMacroParamDataGenerator::GenerateAutomationsData(const NFmiUserWorkAutomationContainer &automations, NFmiThreadCallBacks* threadCallBacks)
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
        auto status = GenerateMacroParamData(dataInfo, AutomationPath(), threadCallBacks);
        if(threadCallBacks)
        {
            if(status)
                threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FINISHED);
            else
                threadCallBacks->DoPostMessage(ID_MACRO_PARAM_DATA_GENERATION_FAILED);
        }
        return status;
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
    logStr += std::filesystem::path(fullAutomationPath).stem().string();
    logStr += "' MacroParam-data from automation: ";
    logStr += fullAutomationPath;
    CatLog::logMessage(logStr, CatLog::Severity::Info, CatLog::Category::Operational);
}

static void ReportDataGenerationFailure(const std::string& fullAutomationPath)
{
    std::string logStr = "Unable to generate '";
    logStr += std::filesystem::path(fullAutomationPath).stem().string();
    logStr += "' MacroParam-data (see error log(s) above) from automation: ";
    logStr += fullAutomationPath;
    CatLog::logMessage(logStr, CatLog::Severity::Error, CatLog::Category::Operational, true);
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

        // NFmiExtraMacroParamData::FindWantedInfo metodin 3. parametri (false) tarkoittaa ett� pohjadataksi 
        // ei kelpuuteta asemadataa, vain hiladata kelpaa.
        auto wantedInfo = NFmiExtraMacroParamData::FindWantedInfo(*infoOrganizer, baseDataCheck.second, false);
        if(!wantedInfo.foundInfo_)
        {
            throw std::runtime_error(std::string("GenerateMacroParamData: unable to find requested base data with '") + dataInfo.BaseDataParamProducerString() + "' parameter, unable to generate wanted macroParam data");
        }

        auto wantedParamDescriptor = ::MakeWantedParamDescriptor(dataInfo);
        NFmiQueryDataUtil::CheckIfStopped(threadCallBacks);
        auto wantedMacroParamDataPtr = ::MakeWantedEmptyData(wantedInfo.foundInfo_, wantedParamDescriptor, dataInfo.BaseDataGridScaleValues());
        if(!wantedMacroParamDataPtr)
        {
            throw std::runtime_error("GenerateMacroParamData: Unable to generate actual macroParam querydata, unknown error in system");
        }
        if(!wantedMacroParamDataPtr->IsGrid())
        {
            throw std::runtime_error("GenerateMacroParamData: Given base data wasn't a grid data, stopping MacroParam-data generation");
        }
        std::shared_ptr<NFmiFastQueryInfo> wantedMacroParamInfoPtr(new NFmiFastQueryInfo(wantedMacroParamDataPtr.get()));

        auto smarttoolContentList = ReadSmarttoolContentsFromFiles(dataInfo.DataGeneratingSmarttoolPathListString());
        NFmiQueryDataUtil::CheckIfStopped(threadCallBacks);

        if(CalculateDataWithSmartTool(wantedMacroParamInfoPtr, infoOrganizer, smarttoolContentList, threadCallBacks))
        {
            auto status = StoreMacroParamData(wantedMacroParamDataPtr, dataInfo.DataStorageFileFilter(), dataInfo.MaxGeneratedFilesKept(), fullAutomationPath, timer);
            if(status)
            {
                return true;
            }
        }
        ::ReportDataGenerationFailure(fullAutomationPath);
    }
    catch(std::exception& e)
    {
        // Normi poikkeukset lokitetaan yhden MacroParam-datan generoinnin tasolla
        CatLog::logMessage(e.what(), CatLog::Severity::Error, CatLog::Category::Operational, true);
    }

    return false;
}

// SmartMetin CMainFrm::OnTimer kutsuu t�t� funktiota kerran minuutissa ja p��ttelee onko teht�v� mit��n 
// itsUsedAutomationList:alla olevaa tuotantoa.
void NFmiMacroParamDataGenerator::DoNeededMacroParamDataAutomations(const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer)
{
    if(itsStopFunctor.Stop())
        return;
    if(!AutomationModeOn())
        return;
    if(loadedDataTriggerList.empty())
        return;

    NFmiMetTime currentTime(1); // Otetaan talteen nyky UTC hetki minuutin tarkkuudella
    auto dueAutomations = itsUsedMacroParamDataAutomationList.GetDueAutomations(currentTime, loadedDataTriggerList, infoOrganizer);
    auto launchWorkerThread = AddTriggeredWorksToFifoAndCheckIfNewWorkerThreadMustBeLaunched(dueAutomations);
    if(launchWorkerThread)
    {
        std::thread t(&NFmiMacroParamDataGenerator::LaunchGenerateAutomationsData, this, &NFmiMacroParamDataGenerator::itsThreadCallBacks);
        t.detach();
    }
}

// T�nne tullaan vain MacroParam-data automaatioiden kautta, eli ei k�ytt�j�n toimesta.
// Kun Automaatiosysteemi on k�yt�ss�, on kaikki k�ytt�j�n generoinnit estetty.
// Eli t��ll� ei tarvitse laittaa generointi lippuja p��lle, tai l�hetell� viestej�, 
// kun on valmista, tai canceloitu.
void NFmiMacroParamDataGenerator::LaunchGenerateAutomationsData(NFmiThreadCallBacks* threadCallBacks)
{
    try
    {
        std::string fullAutomationPath;
        do
        {
            NFmiQueryDataUtil::CheckIfStopped(threadCallBacks);
            auto automationWork = PopWorkFromFifo();
            if(!automationWork)
            {
                throw std::runtime_error("MacroParam-data trigger based automation loop error: Empty work-data, error in programs logic, stopping...");
            }
            fullAutomationPath = automationWork->FullAutomationPath();
            bool status = GenerateMacroParamData(*(automationWork->itsMacroParamDataAutomation), fullAutomationPath, threadCallBacks);
        } while(MarkLastAutomationWorkAsDoneAndCheckIfMoreWorkLeft(fullAutomationPath));
    }
    catch(NFmiStopThreadException&)
    {
        // T�ss� lopetus tulee aina Smartmetin sulkemisesta
        ::LogDataGenerationInterruption(false, true);
        CatLog::logMessage("Automated MacroParam-data generation has been stopped due Smartmet is been closed", CatLog::Severity::Debug, CatLog::Category::Operational);
        ClearAutomationWorksLeftToProcessCounter();
    }
    catch(...)
    {
        ::LogDataGenerationInterruption(false, false);
    }
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

// Lis�t��n thread-turvallisesti uudet trigger�idyt ty�t automaatio-fifo work-listaan.
// Mutta sit� ennen pit�� tarkistaa ett� onko edellinen worker-thread viel� hommissa.
// Palauttaa true, jos uusi thread pit�� k�ynnist��, muuten false.
// T�t� kutsutaan kun automaatio on saanut uusi trigger�ityj� t�it� ja uudet ty�t annetaan
// joko vanhalle threadille tai ett� pit��k� k�ynnist�� uusi worker-thread.
bool NFmiMacroParamDataGenerator::AddTriggeredWorksToFifoAndCheckIfNewWorkerThreadMustBeLaunched(const NFmiUserWorkAutomationContainer& automationWorkList)
{
    if(automationWorkList.empty())
        return false;

    std::lock_guard<std::mutex> lock(mAutomationWorkFifoMutex);
    // Ensin pit�� tarkistaa onko joku vanha threadi ty�skentelem�ss� vai ei.
    bool launchNewWorkerThread = mAutomationWorksLeftToProcessCounter.load() <= 0;
    // Sitten lis�t�� uudet ty�t listaan.
    mAutomationWorkFifo.insert(mAutomationWorkFifo.end(), automationWorkList.begin(), automationWorkList.end());
    mAutomationWorksLeftToProcessCounter += (int)automationWorkList.size();
    // Lokitetaan tilannetta
    std::string message = "Adding " + std::to_string(automationWorkList.size()) + " new triggered MacroParam-data automation to the working queue";
    message += ", total count of works is now " + std::to_string(mAutomationWorksLeftToProcessCounter.load());
    message += launchNewWorkerThread ? ", must launch new worker thread" : ", continuing with existing working thread";
    CatLog::logMessage(message, CatLog::Severity::Debug, CatLog::Category::Operational);
    // Lopuksi palautetaan status
    return launchNewWorkerThread;
}

// Otetaan listan ensimm�inen automaatio-work ja palautetaan se.
// Huom! t�ll�in ei viel� pienennet� mAutomationWorksLeftToProcessCounter:ia,
// se tehd��n vasta MarkLastWorkAsDoneAndCheckIfMoreWorkLeft metodissa.
std::shared_ptr<NFmiMacroParamDataAutomationListItem> NFmiMacroParamDataGenerator::PopWorkFromFifo()
{
    std::lock_guard<std::mutex> lock(mAutomationWorkFifoMutex);
    if(mAutomationWorkFifo.empty())
        return nullptr;

    auto automationWork = mAutomationWorkFifo.front();
    mAutomationWorkFifo.pop_front();
    return automationWork;
}

// Kun worker-thread on tehnyt automaation, merkit��n se tehdyksi v�hent�m�ll�
// mAutomationWorksLeftToProcessCounter:ia 1:ll�.
// Palauttaa true, jos viel� t�it� j�ljell�.
// Palauttaa false, jos mAutomationWorksLeftToProcessCounter menee 0:aan,
// t�ll�in worker-thread lopettaa saman tien.
bool NFmiMacroParamDataGenerator::MarkLastAutomationWorkAsDoneAndCheckIfMoreWorkLeft(const std::string &fullAutomationPath)
{
    std::lock_guard<std::mutex> lock(mAutomationWorkFifoMutex);
    mAutomationWorksLeftToProcessCounter--;
    auto workLeft = (mAutomationWorksLeftToProcessCounter.load() > 0);

    // Lokitetaan tilannetta
    std::string message = "Marking latest automation work as done (";
    message += std::filesystem::path(fullAutomationPath).stem().string();
    message += "), automations left in the work list " + std::to_string(mAutomationWorksLeftToProcessCounter.load()) + ", ";
    message += workLeft ? "keep on going with working thread" : "no more work to do, stop working thread";
    CatLog::logMessage(message, CatLog::Severity::Debug, CatLog::Category::Operational);

    // Lopuksi palautetaan status
    return workLeft;
}

// T�t� kutsutaan nollaamaan mAutomationWorksLeftToProcessCounter, kun ty�skentely
// on keskeytetty, kun Smartmetia halutaan sulkea.
void NFmiMacroParamDataGenerator::ClearAutomationWorksLeftToProcessCounter()
{
    std::lock_guard<std::mutex> lock(mAutomationWorkFifoMutex);
    mAutomationWorksLeftToProcessCounter = 0;
}

void NFmiMacroParamDataGenerator::StopAutomationDueClosingApplication()
{
    itsStopFunctor.Stop(true);
}

bool NFmiMacroParamDataGenerator::WaitForAutomationWorksToStop(double waitForSeconds)
{
    NFmiNanoSecondTimer waitTimer;
    for(;;)
    {
        {
            std::lock_guard<std::mutex> lock(mAutomationWorkFifoMutex);
            if(mAutomationWorksLeftToProcessCounter.load() <= 0)
                return true;

            if(waitTimer.elapsedTimeInSeconds() > waitForSeconds)
                return false;
        }
    }
    return false;
}
