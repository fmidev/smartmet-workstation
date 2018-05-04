//© Ilmatieteenlaitos/Marko.
//Original 7.2.2011
//
// Luokka tiet‰‰ makro-polku asetuksia ja tiet‰‰ cache polut ja muut.
//---------------------------------------------------------- NFmiMacroPathSettings.cpp

#include "NFmiMacroPathSettings.h"
#include "NFmiFileString.h"
#include "NFmiSettings.h"
#include "NFmiPathUtils.h"

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

static std::string MakeFixedPath(const std::string &theDirectory, const std::string &theSmartMetWorkingDirectory)
{
	NFmiFileString fileString(theDirectory);
    if(fileString.IsAbsolutePath())
    {
        return PathUtils::fixMissingDriveLetterToAbsolutePath(theDirectory, theSmartMetWorkingDirectory);
    }
	else // muuten pit‰‰ rakentaa absoluuttinen polku suhteessa editorin k‰ynnistys hakemistoon
	{
		std::string pathName(theSmartMetWorkingDirectory);
		pathName += kFmiDirectorySeparator;
		pathName += theDirectory;
		return pathName;
	}
}

static std::string GetPathFromSettings(const std::string &theSmartMetWorkingDirectory, const std::string &theKeyStr)
{
	std::string settingPath = NFmiSettings::Require<std::string>(theKeyStr);
	return ::MakeFixedPath(settingPath, theSmartMetWorkingDirectory);
}

void NFmiMacroPathSettings::InitFromSettings(const std::string &theInitNameSpace, const std::string &theWorkingDirectory)
{
	itsSmartMetWorkingDirectory = theWorkingDirectory;
	itsBaseNameSpace = theInitNameSpace;

	itsLocalCacheBasePath = NFmiSettings::Require<std::string>(theInitNameSpace + "::LocalDirectory");
	if(itsLocalCacheBasePath.empty() == false)
	{
		char lastChar = itsLocalCacheBasePath[itsLocalCacheBasePath.size() - 1];
		if(lastChar != '\\' && lastChar != '/')
			itsLocalCacheBasePath += kFmiDirectorySeparator; // varmistatetaan ett‰ base-polun lopussa on hakemisto-erotin
	}
	fUseLocalCache = NFmiSettings::Require<bool>(theInitNameSpace + "::UseLocalCache");
	itsSyncIntervalInMinutes = NFmiSettings::Require<int>(theInitNameSpace + "::SyncIntervalInMinutes");

	itsOrigSmartToolPath = ::GetPathFromSettings(itsSmartMetWorkingDirectory, "MetEditor::SmartTools::LoadDirectory");
	itsOrigMacroParamPath = ::GetPathFromSettings(itsSmartMetWorkingDirectory, "MetEditor::MacroParams::LoadDirectory");
	itsOrigDrawParamPath = ::GetPathFromSettings(itsSmartMetWorkingDirectory, "MetEditor::DrawParams::LoadDirectory");
	itsOrigViewMacroPath = ::GetPathFromSettings(itsSmartMetWorkingDirectory, "MetEditor::ViewMacro::LoadDirectory");
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

