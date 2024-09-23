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
#include <boost/algorithm/string.hpp>
#include <filesystem>

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
}

// ***********************************************************
// ************** NFmiMacroParamDataInfo *********************
// ***********************************************************

NFmiMacroParamDataInfo::NFmiMacroParamDataInfo() = default;

NFmiMacroParamDataInfo::NFmiMacroParamDataInfo(const std::string& baseDataParamProducerLevelString, const std::string& usedProducerString, const std::string& dataGeneratingSmarttoolPathString, const std::string& usedParameterListString, const std::string& dataStorageFileFilter)
:mBaseDataParamProducerString(baseDataParamProducerLevelString)
,mUsedProducerString(usedProducerString)
,mDataGeneratingSmarttoolPathString(dataGeneratingSmarttoolPathString)
,mUsedParameterListString(usedParameterListString)
,mDataStorageFileFilter(dataStorageFileFilter)
{

}

bool NFmiMacroParamDataInfo::CheckData()
{
    return false;
}

std::string NFmiMacroParamDataInfo::MakeDataStorageFilePath(const std::string& dataStorageFileFilter)
{
    // Tehd‰‰n lokaali ajasta aikaleima, jossa mukana myˆs sekunnit
    NFmiTime makeTime;
    std::string timeStampStr = makeTime.ToStr(kYYYYMMDDHHMMSS);
    auto finalFilePath = dataStorageFileFilter;
    boost::replace_all(finalFilePath, "*", timeStampStr);
    return finalFilePath;
}

// Perus datan valintaan liittyvan par+prod stringin tarkistus. Palauttaa selityksen virheest‰, jos siin‰ on jotain vikaa.
// Jos kaikki on ok, palauttaa tyhj‰n stringin.
std::pair<std::string, NFmiDefineWantedData> NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(const std::string& baseDataParamProducerString)
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
        checkStr = "CheckBaseDataParamProducerString: Given base data parameter '" + baseDataParamProducerString + "' did have level information, and level data aren't allowed to be macroParam data.";
    }

    return std::make_pair(checkStr, variableDataType.second);
}

// Producer stringin tarkistus, palauttaa mahdollisen virheilmoituksen ja vectorin, jossa id+name palaset.
std::pair<std::string, std::vector<std::string>> NFmiMacroParamDataInfo::CheckUsedProducerString(const std::string& usedProducerString)
{
    std::vector<std::string> parts;
    boost::split(parts, usedProducerString, boost::is_any_of(","));
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
    std::vector<std::string> parts;
    boost::split(parts, usedParameterListString, boost::is_any_of(","));
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

// Talletettavan datan filefiltterin tarkistus. Palauttaa selityksen virheest‰, jos siin‰ on jotain vikaa.
// Jos kaikki on ok, palauttaa tyhj‰n string:in.
std::string NFmiMacroParamDataInfo::CheckDataStorageFileFilter(const std::string& dataStorageFileFilter)
{
    // 1. Polku ei kelpaa, jos talletushakemistoa ei ole.
    // T‰ll‰ tarkoitus est‰‰ datojen vahinkotalletuksia v‰‰riin paikkoihin.
    auto directory = PathUtils::getPathSectionFromTotalFilePath(dataStorageFileFilter);
    if(!::DirectoryExists(directory))
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output directory '" + directory + "' doesn't exists, make sure that given path for MacroParam data was correct!";
    }

    auto filename = PathUtils::getFilename(dataStorageFileFilter);
    // 2. Tiedoston nimess‰ pit‰‰ olla '*' merkki aikaleimaa varten.
    if(filename.find('*') == std::string::npos)
    {
        return std::string("CheckDataStorageFileFilter") + ": Given data output filename '" + filename + "' (in " + dataStorageFileFilter + ") doesn't have ' * ' character place marker for file's creation time stamp. To store MacroParam data you need to have that asterisk in filename.";
    }

    auto fileExtension = ::GetFileExtension(dataStorageFileFilter);
    // 3. Tiedoston extension pit‰‰ olla sqd tyyppinen
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
    // 3. Tiedoston extension pit‰‰ olla st tyyppinen
    if(!boost::iequals(fileExtension, gSmarttoolFileExtension))
    {
        return std::string("CheckDataGeneratingSmarttoolPathString") + ": Given data output extension '" + fileExtension + "' (in " + dataGeneratingSmarttoolPathString + ") is invalid, extension must be '" + gSmarttoolFileExtension + "' type.";
    }

    return "";
}

static const std::string gJsonName_BaseDataParamProducer = "BaseDataParamProducer";
static const std::string gJsonName_UsedProducer = "UsedProducer";
static const std::string gJsonName_DataGeneratingSmarttoolPath = "DataGeneratingSmarttoolPath";
static const std::string gJsonName_UsedParameterList = "UsedParameterList";
static const std::string gJsonName_DataStorageFileFilter = "DataStorageFileFilter";

json_spirit::Object NFmiMacroParamDataInfo::MakeJsonObject(const NFmiMacroParamDataInfo& macroParamDataInfo)
{
    json_spirit::Object jsonObject;
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.BaseDataParamProducerString(), gJsonName_BaseDataParamProducer, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.UsedProducerString(), gJsonName_UsedProducer, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.DataGeneratingSmarttoolPathString(), gJsonName_DataGeneratingSmarttoolPath, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.UsedParameterListString(), gJsonName_UsedParameterList, jsonObject);
    JsonUtils::AddNonEmptyStringJsonPair(macroParamDataInfo.DataStorageFileFilter(), gJsonName_DataStorageFileFilter, jsonObject);

    return jsonObject;
}

void NFmiMacroParamDataInfo::ParseJsonPair(json_spirit::Pair& thePair)
{
    // T‰ss‰ puretaan NFmiMacroParamDataInfo luokan p‰‰tason pareja.
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
}

bool NFmiMacroParamDataInfo::StoreInJsonFormat(const NFmiMacroParamDataInfo& macroParamDataInfo, const std::string& theFilePath, std::string& theErrorStringOut)
{
    return JsonUtils::StoreObjectInJsonFormat(macroParamDataInfo, theFilePath, "MacroParamDataInfo", theErrorStringOut);
}

bool NFmiMacroParamDataInfo::ReadInJsonFormat(NFmiMacroParamDataInfo& macroParamDataInfoOut, const std::string& theFilePath, std::string& theErrorStringOut)
{
    return JsonUtils::ReadObjectInJsonFormat(macroParamDataInfoOut, theFilePath, "MacroParamDataInfo", theErrorStringOut);
}

// ***********************************************************
// ************ NFmiMacroParamDataGenerator ******************
// ***********************************************************

std::string NFmiMacroParamDataGenerator::mRootSmarttoolDirectory = "";
const std::string NFmiMacroParamDataGenerator::itsMacroParamDataInfoFileExtension = "mpd";
const std::string NFmiMacroParamDataGenerator::itsMacroParamDataInfoFileFilter = "MacroParam data info Files (*." + NFmiMacroParamDataGenerator::itsMacroParamDataInfoFileExtension + ")|*." + NFmiMacroParamDataGenerator::itsMacroParamDataInfoFileExtension + "|All Files (*.*)|*.*||";

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

bool NFmiMacroParamDataGenerator::Init(const std::string& theBaseRegistryPath, const std::string& rootSmarttoolDirectory)
{
    mBaseRegistryPath = theBaseRegistryPath;
    mRootSmarttoolDirectory = ::FixRootPath(rootSmarttoolDirectory);
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

    return true;
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


bool NFmiMacroParamDataGenerator::GenerateMacroParamData(NFmiThreadCallBacks* threadCallBacks)
{
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
        auto baseDataCheck = NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(dataInfo.BaseDataParamProducerString());
        if(!baseDataCheck.first.empty())
        {
            throw std::runtime_error(baseDataCheck.first);
        }

        auto* infoOrganizer = ::GetInfoOrganizer();
        if(!infoOrganizer)
        {
            throw std::runtime_error("GenerateMacroParamData: Can't get proper InfoOrganizer into works, unknown error in the system");
        }

        // NFmiExtraMacroParamData::FindWantedInfo metodin 3. parametri (false) tarkoittaa ett‰ pohjadataksi 
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
            return StoreMacroParamData(wantedMacroParamDataPtr, dataInfo.DataStorageFileFilter());
        }
    }
    catch(std::exception& )
    {
        // Raportoi ongelmasta
    }

    return false;
}

// Oletus: macroParamDataPtr pit‰‰ sis‰ll‰‰n talletettavan datan.
bool NFmiMacroParamDataGenerator::StoreMacroParamData(boost::shared_ptr<NFmiQueryData>& macroParamDataPtr, const std::string& dataStorageFileFilter)
{
    // CheckDataStorageFileFilter funktio palauttaa virheilmoituksen, jos filefilterissa jotain vikaa.
    auto checkStr = NFmiMacroParamDataInfo::CheckDataStorageFileFilter(dataStorageFileFilter);
    if(!checkStr.empty())
    {
        throw std::runtime_error(checkStr);
    }

    auto finalDataFilePath = NFmiMacroParamDataInfo::MakeDataStorageFilePath(dataStorageFileFilter);
    // NFmiQueryData::Write heitt‰‰ poikkeuksia, jos tulee ongelmia.
    macroParamDataPtr->Write(finalDataFilePath);
    return true;
}

static void CalcMultiLevelSmarttoolData(NFmiSmartToolModifier& smartToolModifier, boost::shared_ptr<NFmiFastQueryInfo>& editedInfoCopy)
{
    auto usedTimeDescriptor(editedInfoCopy->TimeDescriptor());
    for(editedInfoCopy->ResetLevel(); editedInfoCopy->NextLevel();)
    {
        // jos kyseess‰ on level-data, pit‰‰ l‰pik‰yt‰v‰ leveli ottaa talteen, 
        // ett‰ smartToolModifier osaa luoda siihen osoittavia fastInfoja.
        boost::shared_ptr<NFmiLevel> theLevel(new NFmiLevel(*editedInfoCopy->Level()));
        smartToolModifier.ModifiedLevel(theLevel);

        smartToolModifier.ModifyData_ver2(&usedTimeDescriptor, false, false, nullptr);
    }
}

// Oletus: wantedMacroParamInfoPtr on tarkistettu jo ulkopuolella ett‰ ei ole nullptr
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

    // Jos kyseess‰ on ns. macroParam eli skriptiss‰ on lausekkeit‰ jossa sijoitetaan 
    // RETURN muuttujaan, t‰llˆin ei haluta ajaa makro muutoksia, koska tarkoitus 
    // on vain katsella macroParam:ia karttan‰ytˆll‰.
    // T‰nne asti tullaan ett‰ saadaan virhe ilmoitus tulkinta vaiheessa talteen.
    // Mutta suoritus vaiheen virheet menev‰t t‰ll‰ hetkell‰ vain loki tiedostoon.
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

// Jos annettu polku on suhteellinen, lis‰t‰‰n siihen smarttool juuri hakemisto.
// Korjataan polku kaikin puolin oikeaan formaattiin (erottimet ja mahd. .. tai . jutut).
std::string NFmiMacroParamDataGenerator::MakeUsedAbsoluteSmarttoolPathString(const std::string& smarttoolPath) const
{
    return PathUtils::getAbsoluteFilePath(smarttoolPath, mRootSmarttoolDirectory);
}

// Jos annettu polku on absoluuttinen, poistetaan siit‰ smarttool juuri hakemisto osio alusta.
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

NFmiMacroParamDataInfo NFmiMacroParamDataGenerator::MakeDataInfo() const
{
    return NFmiMacroParamDataInfo(DialogBaseDataParamProducerString(), DialogUsedProducerString(), DialogDataGeneratingSmarttoolPathString(), DialogUsedParameterListString(), DialogDataStorageFileFilter());
}

std::string NFmiMacroParamDataGenerator::MacroParamDataInfoSaveInitialPath()
{
    return *mMacroParamDataInfoSaveInitialPath;
}

void NFmiMacroParamDataGenerator::MacroParamDataInfoSaveInitialPath(const std::string& newValue)
{
    *mMacroParamDataInfoSaveInitialPath = newValue;
}
