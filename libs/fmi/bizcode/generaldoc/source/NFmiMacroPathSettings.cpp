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
:itsSmartMetWorkingDirectory()
,itsOrigSmartToolPath()
,itsOrigViewMacroPath()
,itsOrigMacroParamPath()
,itsOrigDrawParamPath()

,itsBaseNameSpace()
{
}

NFmiMacroPathSettings::NFmiMacroPathSettings(const NFmiMacroPathSettings& theOther) = default;

const NFmiMacroPathSettings& NFmiMacroPathSettings::operator=(const NFmiMacroPathSettings &theOther)
{
	if(this != &theOther)
	{
		itsSmartMetWorkingDirectory = theOther.itsSmartMetWorkingDirectory;
		itsOrigSmartToolPath = theOther.itsOrigSmartToolPath;
		itsOrigViewMacroPath = theOther.itsOrigViewMacroPath;
		itsOrigMacroParamPath = theOther.itsOrigMacroParamPath;
		itsOrigDrawParamPath = theOther.itsOrigDrawParamPath;
		itsOrigMacroParamDataPath = theOther.itsOrigMacroParamDataPath;

		itsBaseNameSpace = theOther.itsBaseNameSpace;
	}
	return *this;
}

void NFmiMacroPathSettings::InitFromSettings(const std::string &theInitNameSpace, const std::string &theWorkingDirectory)
{
	itsSmartMetWorkingDirectory = theWorkingDirectory;
	itsBaseNameSpace = theInitNameSpace;

	itsOrigSmartToolPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::SmartTools::LoadDirectory", itsSmartMetWorkingDirectory);
	itsOrigMacroParamPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::MacroParams::LoadDirectory", itsSmartMetWorkingDirectory);
	itsOrigDrawParamPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::DrawParams::LoadDirectory", itsSmartMetWorkingDirectory);
	itsOrigViewMacroPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::ViewMacro::LoadDirectory", itsSmartMetWorkingDirectory);
	itsOrigMacroParamDataPath = PathUtils::getFixedAbsolutePathFromSettings("MetEditor::MacroParamData::LoadDirectory", itsSmartMetWorkingDirectory);
	LogMacroPaths();
}

void NFmiMacroPathSettings::LogMacroPaths()
{
    CatLog::logMessage(std::string("MacroPathSettings::SmartMetWorkingDirectory = ") + itsSmartMetWorkingDirectory, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::SmartToolPath = ") + itsOrigSmartToolPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::ViewMacroPath = ") + itsOrigViewMacroPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::MacroParamPath = ") + itsOrigMacroParamPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("MacroPathSettings::DrawParamPath = ") + itsOrigDrawParamPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
	CatLog::logMessage(std::string("MacroPathSettings::MacroParamDataPath = ") + itsOrigMacroParamDataPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
}

std::string NFmiMacroPathSettings::SmartToolPath()
{
	return itsOrigSmartToolPath;
}

std::string NFmiMacroPathSettings::ViewMacroPath()
{
	return itsOrigViewMacroPath;
}

std::string NFmiMacroPathSettings::MacroParamPath()
{
	return itsOrigMacroParamPath;
}

std::string NFmiMacroPathSettings::DrawParamPath()
{
	return itsOrigDrawParamPath;
}

std::string NFmiMacroPathSettings::MacroParamDataPath()
{
	return itsOrigMacroParamDataPath;
}
