#include "NFmiMacroParamDataGenerator.h"
#include "NFmiPathUtils.h"

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

bool NFmiMacroParamDataGenerator::Init(const std::string& theBaseRegistryPath, const std::string& rootSmarttoolsDirectory)
{
    mBaseRegistryPath = theBaseRegistryPath;
    mRootSmarttoolsDirectory = ::FixRootPath(rootSmarttoolsDirectory);
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

// Jos annettu polku on suhteellinen, lis‰t‰‰n siihen smarttools juuri hakemisto.
// Korjataan polku kaikin puolin oikeaan formaattiin (erottimet ja mahd. .. tai . jutut).
std::string NFmiMacroParamDataGenerator::MakeUsedAbsoluteSmarttoolPathString(const std::string& smarttoolPath) const
{
    return PathUtils::getAbsoluteFilePath(smarttoolPath, mRootSmarttoolsDirectory);
}

// Jos annettu polku on absoluuttinen, poistetaan siit‰ smarttools juuri hakemisto osio alusta.
// Korjataan polku kaikin puolin oikeaan formaattiin (erottimet ja mahd. .. tai . jutut).
std::string NFmiMacroParamDataGenerator::MakeCleanedSmarttoolPathString(const std::string& smarttoolPath) const
{
    auto tmpPath = PathUtils::getRelativePathIfPossible(smarttoolPath, mRootSmarttoolsDirectory);
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
