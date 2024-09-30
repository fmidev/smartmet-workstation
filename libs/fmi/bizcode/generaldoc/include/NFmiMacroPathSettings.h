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

	std::string SmartToolPath();
	std::string ViewMacroPath();
	std::string MacroParamPath();
	std::string DrawParamPath();
	std::string MacroParamDataPath();

private:
    void LogMacroPaths();

	std::string itsSmartMetWorkingDirectory; // editorin työ hakemisto

	std::string itsOrigSmartToolPath;
	std::string itsOrigViewMacroPath;
	std::string itsOrigMacroParamPath;
	std::string itsOrigDrawParamPath;
	std::string itsOrigMacroParamDataPath;

	std::string itsBaseNameSpace;
};
