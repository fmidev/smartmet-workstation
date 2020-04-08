//© Ilmatieteenlaitos/Marko.
//Original 7.2.2011
//
// Luokka tietää makro-polku asetuksia ja tietää cache polut ja muut.
//---------------------------------------------------------- NFmiMacroPathSettings.h

#pragma once

#include <string>

class NFmiMacroPathSettings
{
public:
	NFmiMacroPathSettings(void);
	NFmiMacroPathSettings(const NFmiMacroPathSettings &theOther);
	const NFmiMacroPathSettings& operator=(const NFmiMacroPathSettings &theOther);
	void InitFromSettings(const std::string &theInitNameSpace, const std::string &theWorkingDirectory);
	void StoreToSettings(void);

	std::string SmartToolPath(bool fGetUserPath);
	std::string ViewMacroPath(bool fGetUserPath);
	std::string MacroParamPath(bool fGetUserPath);
	std::string DrawParamPath(bool fGetUserPath);

	const std::string& LocalCacheBasePath(void) const {return itsLocalCacheBasePath;}
	void LocalCacheBasePath(const std::string &newValue) {itsLocalCacheBasePath = newValue;}
	bool UseLocalCache(void) const {return fUseLocalCache;}
	int SyncIntervalInMinutes(void) const {return itsSyncIntervalInMinutes;}

private:
    void LogMacroPaths();

	bool fUseLocalCache;
	std::string itsLocalCacheBasePath;
	int itsSyncIntervalInMinutes;
	std::string itsSmartMetWorkingDirectory; // editorin työ hakemisto

	std::string itsOrigSmartToolPath;
	std::string itsOrigViewMacroPath;
	std::string itsOrigMacroParamPath;
	std::string itsOrigDrawParamPath;

	std::string itsBaseNameSpace;
};
