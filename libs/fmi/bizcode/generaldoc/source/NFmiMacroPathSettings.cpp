//© Ilmatieteenlaitos/Marko.
//Original 7.2.2011
//
// Luokka tiet‰‰ makro-polku asetuksia ja tiet‰‰ cache polut ja muut.
//---------------------------------------------------------- NFmiMacroPathSettings.cpp

#include "NFmiMacroPathSettings.h"
#include "NFmiFileString.h"
#include "NFmiSettings.h"
#include "NFmiPathUtils.h"
#include "NFmiBetaProductHelperFunctions.h"

NFmiMacroPathSettings::NFmiMacroPathSettings(void)
:fUseLocalCache(false)
,itsLocalCacheBasePath()
,itsSyncIntervalInMinutes(0)
,itsSmartMetWorkingDirectory()
,itsOrigSmartToolPath()
,itsOrigViewMacroPath()
,itsOrigMacroParamPath()
,itsOrigDrawParamPath()

,itsBaseNameSpace()
{
}

NFmiMacroPathSettings::NFmiMacroPathSettings(const NFmiMacroPathSettings &theOther)
:fUseLocalCache(theOther.fUseLocalCache)
,itsLocalCacheBasePath(theOther.itsLocalCacheBasePath)
,itsSyncIntervalInMinutes(theOther.itsSyncIntervalInMinutes)
,itsSmartMetWorkingDirectory(theOther.itsSmartMetWorkingDirectory)
,itsOrigSmartToolPath(theOther.itsOrigSmartToolPath)
,itsOrigViewMacroPath(theOther.itsOrigViewMacroPath)
,itsOrigMacroParamPath(theOther.itsOrigMacroParamPath)
,itsOrigDrawParamPath(theOther.itsOrigDrawParamPath)

,itsBaseNameSpace(theOther.itsBaseNameSpace)
{
}

const NFmiMacroPathSettings& NFmiMacroPathSettings::operator=(const NFmiMacroPathSettings &theOther)
{
	fUseLocalCache = theOther.fUseLocalCache;
	itsLocalCacheBasePath = theOther.itsLocalCacheBasePath;
	itsSyncIntervalInMinutes = theOther.itsSyncIntervalInMinutes;
	itsSmartMetWorkingDirectory = theOther.itsSmartMetWorkingDirectory;
	itsOrigSmartToolPath = theOther.itsOrigSmartToolPath;
	itsOrigViewMacroPath = theOther.itsOrigViewMacroPath;
	itsOrigMacroParamPath = theOther.itsOrigMacroParamPath;
	itsOrigDrawParamPath = theOther.itsOrigDrawParamPath;

	itsBaseNameSpace = theOther.itsBaseNameSpace;
	return *this;
}

void NFmiMacroPathSettings::InitFromSettings(const std::string &theInitNameSpace, const std::string &theWorkingDirectory)
{
	itsSmartMetWorkingDirectory = theWorkingDirectory;
	itsBaseNameSpace = theInitNameSpace;

	itsLocalCacheBasePath = PathUtils::getFixedAbsolutePathFromSettings(theInitNameSpace + "::LocalDirectory", itsSmartMetWorkingDirectory, true);
	fUseLocalCache = NFmiSettings::Require<bool>(theInitNameSpace + "::UseLocalCache");
	itsSyncIntervalInMinutes = NFmiSettings::Require<int>(theInitNameSpace + "::SyncIntervalInMinutes");

	itsOrigSmartToolPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::SmartTools::LoadDirectory", itsSmartMetWorkingDirectory);
	itsOrigMacroParamPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::MacroParams::LoadDirectory", itsSmartMetWorkingDirectory);
	itsOrigDrawParamPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::DrawParams::LoadDirectory", itsSmartMetWorkingDirectory);
	itsOrigViewMacroPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::ViewMacro::LoadDirectory", itsSmartMetWorkingDirectory);
    LogMacroPaths();
}

void NFmiMacroPathSettings::LogMacroPaths()
{
    CatLog::logMessage(std::string("MacroPathSettings::UseLocalCache = ") + (fUseLocalCache ? "true" : "false"), CatLog::Severity::Info, CatLog::Category::Configuration, true);
    if(fUseLocalCache)
        CatLog::logMessage(std::string("MacroPathSettings::LocalCacheBasePath = ") + itsLocalCacheBasePath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::SmartMetWorkingDirectory = ") + itsSmartMetWorkingDirectory, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::SmartToolPath = ") + itsOrigSmartToolPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::ViewMacroPath = ") + itsOrigViewMacroPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::MacroParamPath = ") + itsOrigMacroParamPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::DrawParamPath = ") + itsOrigDrawParamPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
}


void NFmiMacroPathSettings::StoreToSettings(void)
{
	if(itsBaseNameSpace.empty() == false)
	{
		// HUOM! t‰ss‰ on toistaiseksi vain cacheen liittyvien muutosten talletukset
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::LocalDirectory"), itsLocalCacheBasePath, true);
		NFmiSettings::Set(itsBaseNameSpace + "::UseLocalCache", NFmiStringTools::Convert(fUseLocalCache), true);
		NFmiSettings::Set(itsBaseNameSpace + "::SyncIntervalInMinutes", NFmiStringTools::Convert(itsSyncIntervalInMinutes), true);
	}
	else
		throw std::runtime_error("Error in NFmiMacroPathSettings::StoreToSettings, unable to store setting.");
}

// Palauttaa puhtaan hakemisto osan originaali polusta (siis ilman drive-letteri‰).
// Esim. P:\settings\kepa\DrawParam3 -> settings\kepa\DrawParam3
static std::string GetDirectoryPart(const std::string &thePath)
{
	if(thePath.empty())
		return thePath;
	NFmiFileString pathStr = thePath;
	pathStr.NormalizeDelimiter();
	if(pathStr[pathStr.GetLen()] == kFmiDirectorySeparator)
		pathStr.TrimR(kFmiDirectorySeparator); // jos lopussa oli polku-erotin kenoviiva, poistetaan se
	std::string tmpPath = pathStr;
	size_t pos = tmpPath.find(kFmiDirectorySeparator);
	if(pos != std::string::npos)
	{
		return std::string(tmpPath.begin() + pos + 1, tmpPath.end());
	}
	return std::string();
}

static std::string MakeWantedPath(const std::string &theBasePath, const std::string &theFinalPathDir)
{
	std::string wantedPath = theBasePath;
	wantedPath += theFinalPathDir;
	return wantedPath;
}

// fGetUserPath = true tarkoittaa ett‰ halutaan polku mit‰ oikeasti k‰ytet‰‰n 'tyˆss‰' eli 
// se on lokaali cache-polku jos niin asetuksissa on m‰‰r‰tty. Jos lokaali cachea ei k‰ytet‰
// palautetaan originaali(server)-polku.
// fGetUserPath = false tarkoittaa ett‰ halutaan polku 'server' osoitteeseen, t‰m‰ voi 
// siis olla vain 'originaali' polku.
std::string NFmiMacroPathSettings::SmartToolPath(bool fGetUserPath)
{
	if(fGetUserPath && fUseLocalCache)
		return ::MakeWantedPath(itsLocalCacheBasePath, ::GetDirectoryPart(itsOrigSmartToolPath));
	else
		return itsOrigSmartToolPath;
}

std::string NFmiMacroPathSettings::ViewMacroPath(bool fGetUserPath)
{
	if(fGetUserPath && fUseLocalCache)
		return ::MakeWantedPath(itsLocalCacheBasePath, ::GetDirectoryPart(itsOrigViewMacroPath));
	else
		return itsOrigViewMacroPath;
}

std::string NFmiMacroPathSettings::MacroParamPath(bool fGetUserPath)
{
	if(fGetUserPath && fUseLocalCache)
		return ::MakeWantedPath(itsLocalCacheBasePath, ::GetDirectoryPart(itsOrigMacroParamPath));
	else
		return itsOrigMacroParamPath;
}

std::string NFmiMacroPathSettings::DrawParamPath(bool fGetUserPath)
{
	if(fGetUserPath && fUseLocalCache)
		return ::MakeWantedPath(itsLocalCacheBasePath, ::GetDirectoryPart(itsOrigDrawParamPath));
	else
		return itsOrigDrawParamPath;
}

