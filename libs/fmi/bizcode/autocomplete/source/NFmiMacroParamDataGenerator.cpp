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
#include <boost/algorithm/string.hpp>
#include <filesystem>

namespace
{
    bool DirectoryExists(const std::string& directoryPath)
    {
        return (std::filesystem::exists(directoryPath) && std::filesystem::is_directory(directoryPath));
    }

    std::string GetFileExtension(const std::string& filePath)
    {
        std::filesystem::path originalPath(filePath);
        return originalPath.extension().string();
    }
}

// ***********************************************************
// ************** NFmiMacroParamDataInfo *********************
// ***********************************************************

NFmiMacroParamDataInfo::NFmiMacroParamDataInfo() = default;

NFmiMacroParamDataInfo::NFmiMacroParamDataInfo(const std::string& baseDataParamProducerLevelString, const std::string& usedProducerString, const std::string& dataGeneratingSmarttoolPathString, const std::string& usedParameterListString, const std::string& dataStorageFileFilter)
:mBaseDataParamProducerLevelString(baseDataParamProducerLevelString)
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

const std::string gWantedFileExtension = ".sqd";

// Talletettavan datan filefiltterin tarkistus. Heitt‰‰ poikkeuksen, jos siin‰ on jotain vikaa.
void NFmiMacroParamDataInfo::CheckDataStorageFileFilter(const std::string& dataStorageFileFilter)
{
    // 1. Polku ei kelpaa, jos talletushakemistoa ei ole.
    // T‰ll‰ tarkoitus est‰‰ datojen vahinkotalletuksia v‰‰riin paikkoihin.
    auto directory = PathUtils::getPathSectionFromTotalFilePath(dataStorageFileFilter);
    if(!::DirectoryExists(directory))
    {
        throw std::runtime_error(std::string(__FUNCTION__) + " given data output directory '" + directory + "' doesn't exists, make sure that given path for MacroParam data was correct!");
    }

    auto filename = PathUtils::getFilename(dataStorageFileFilter);
    // 2. Tiedoston nimess‰ pit‰‰ olla '*' merkki aikaleimaa varten.
    if(filename.find('*') == std::string::npos)
    {
        throw std::runtime_error(std::string(__FUNCTION__) + " given data output filename '" + filename + "' (in " + dataStorageFileFilter + ") doesn't have ' * ' character place marker for file's creation time stamp. To store MacroParam data you need to have that asterisk in filename.");
    }

    auto fileExtension = ::GetFileExtension(dataStorageFileFilter);
    // 3. Tiedoston extension pit‰‰ olla sqd tyyppinen
    if(!boost::iequals(fileExtension, gWantedFileExtension))
    {
        throw std::runtime_error(std::string(__FUNCTION__) + " given data output extension '" + fileExtension + "' (in " + dataStorageFileFilter + ") is invalid, extension must be '" + gWantedFileExtension + ".sqd' type.");
    }
}

// ***********************************************************
// ************ NFmiMacroParamDataGenerator ******************
// ***********************************************************

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
    mDialogBaseDataParamProducerLevelString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\BaseDataParamProducerLevel", usedKey, "T_ec");
    mDialogUsedProducerString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\UsedProducer", usedKey, "3001,ProducerName");
    mDialogDataGeneratingSmarttoolPathString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\DataGeneratingSmarttoolPath", usedKey, "smarttool.st");
    mDialogUsedParameterListString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\UsedParameterListString", usedKey, "6201, Param1, 6202, Param2");
    mDialogDataStorageFileFilter = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, macroParamDataGeneratorSectionName, "\\DataStorageFileFilter", usedKey, "C:\\data\\*_mydata.sqd");

    return true;
}

static NFmiProducer MakeWantedProducer(const NFmiMacroParamDataInfo& dataInfo)
{
    std::vector<std::string> parts;
    boost::split(parts, dataInfo.UsedProducerString(), boost::is_any_of(","));
    if(parts.size() < 2)
    {
        throw std::runtime_error(std::string(__FUNCTION__) + " had too few producer parts (< 2, should be like \"123,producerName\")");
    }

    unsigned long producerId = std::stoul(parts[0]);
    return NFmiProducer(producerId, parts[1]);
}

static NFmiParam MakeWantedParam(const std::string &part1, const std::string& part2)
{
    unsigned long paramId = std::stoul(part1);
    return NFmiParam(paramId, part2);
}

static NFmiParamDescriptor MakeWantedParamDescriptor(const NFmiMacroParamDataInfo& dataInfo)
{
    auto producer = ::MakeWantedProducer(dataInfo);
    std::vector<std::string> parts;
    boost::split(parts, dataInfo.UsedParameterListString(), boost::is_any_of(","));
    if(parts.size() < 2)
    {
        throw std::runtime_error(std::string(__FUNCTION__) + " had too few parameter (list) parts (< 2, should be like \"123,paramName1[,124,paramName2,...]\")");
    }
    
    NFmiParamBag wantedParams;
    size_t totalPartCounter = 0;
    for( ; totalPartCounter < parts.size() - 1; )
    {
        auto wantedParam = ::MakeWantedParam(parts[totalPartCounter], parts[totalPartCounter + 1]);
        wantedParams.Add(NFmiDataIdent(wantedParam, producer));
        totalPartCounter += 2;
    }
    return NFmiParamDescriptor(wantedParams);
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


bool NFmiMacroParamDataGenerator::GenerateMacroParamData()
{
    auto dataInfo = MakeDataInfo();
    return GenerateMacroParamData(dataInfo);
}

bool NFmiMacroParamDataGenerator::GenerateMacroParamData(const NFmiMacroParamDataInfo& dataInfo)
{
    try
    {
        auto variableDataType = NFmiSmartToolIntepreter::CheckForVariableDataType(dataInfo.BaseDataParamProducerLevelString());
        if(!variableDataType.first)
        {
            // Raportoi huonosta syˆtteest‰
            return false;
        }

        auto* infoOrganizer = ::GetInfoOrganizer();
        if(!infoOrganizer)
        {
            // Raportoi ongelmasta infoOrganizer:in suhteen
            return false;
        }

        // NFmiExtraMacroParamData::FindWantedInfo metodin 3. parametri (false) tarkoittaa ett‰ pohjadataksi 
        // ei kelpuuteta asemadataa, vain hiladata kelpaa.
        auto wantedInfo = NFmiExtraMacroParamData::FindWantedInfo(*infoOrganizer, variableDataType.second, false);
        if(!wantedInfo.foundInfo_)
        {
            // Raportoi ongelmasta kun dataa ei lˆytynyt
            return false;
        }

        auto wantedParamDescriptor = ::MakeWantedParamDescriptor(dataInfo);
        auto wantedMacroParamDataPtr = ::MakeWantedEmptyData(wantedInfo.foundInfo_, wantedParamDescriptor);
        boost::shared_ptr<NFmiFastQueryInfo> wantedMacroParamInfoPtr(new NFmiFastQueryInfo(wantedMacroParamDataPtr.get()));
        if(!wantedMacroParamInfoPtr)
        {
            // Raportoi ongelmasta kun fastInfo on tyhj‰‰
            return false;
        }

        auto smarttoolContent = ReadSmarttoolContentFromFile(dataInfo.DataGeneratingSmarttoolPathString());
        if(smarttoolContent.empty())
        {
            // Raportoi smarttool tiedoston luku ongelmista
            return false;
        }

        if(CalculateDataWithSmartTool(wantedMacroParamInfoPtr, infoOrganizer, smarttoolContent))
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
    // CheckDataStorageFileFilter funktio heitt‰‰ poikkeuksen virheilmoituksineen, jos filefilterissa jotain vikaa.
    NFmiMacroParamDataInfo::CheckDataStorageFileFilter(dataStorageFileFilter);

    auto finalDataFilePath = NFmiMacroParamDataInfo::MakeDataStorageFilePath(dataStorageFileFilter);
    // NFmiQueryData::Write heitt‰‰ poikkeuksia, jos tulee ongelmia.
    macroParamDataPtr->Write(finalDataFilePath);
    return true;
}

// Oletus: wantedMacroParamInfoPtr on tarkistettu jo ulkopuolella ett‰ ei ole nullptr
bool NFmiMacroParamDataGenerator::CalculateDataWithSmartTool(boost::shared_ptr<NFmiFastQueryInfo>& wantedMacroParamInfoPtr, NFmiInfoOrganizer* infoOrganizer, const std::string& smartToolContent)
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
        NFmiThreadCallBacks* threadCallBacks = nullptr;
        auto usedTimeDescriptor(wantedMacroParamInfoPtr->TimeDescriptor());
        smartToolModifier.ModifyData_ver2(&usedTimeDescriptor, false, false, threadCallBacks);

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


std::string NFmiMacroParamDataGenerator::DialogBaseDataParamProducerLevelString() const
{
    return *mDialogBaseDataParamProducerLevelString;
}

void NFmiMacroParamDataGenerator::DialogBaseDataParamProducerLevelString(const std::string& newValue)
{
    *mDialogBaseDataParamProducerLevelString = newValue;
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
    return NFmiMacroParamDataInfo(DialogBaseDataParamProducerLevelString(), DialogUsedProducerString(), DialogDataGeneratingSmarttoolPathString(), DialogUsedParameterListString(), DialogDataStorageFileFilter());
}
