//**********************************************************
// C++ Class Name : NFmiMetEditorOptionsData
// ---------------------------------------------------------
// Filetype: (source)
// Filepath: NFmiMetEditorOptionsData.cpp
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 4.0 )
//  - GD System Name    : nowcast starts planning
//  - GD View Type      : Class Diagram
//  - GD View Name      :
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Mon - Sep 4, 2000
//
//  Change Log     :
//
//**********************************************************
#ifdef _MSC_VER
// poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#pragma warning(disable : 4786)
#endif

#include "NFmiMetEditorOptionsData.h"
#include "NFmiWeatherAndCloudiness.h"
#include "NFmiSettings.h"

NFmiMetEditorOptionsData::NFmiMetEditorOptionsData(void)
:fDataValidationEnabled(true)
,fShowToolTipsOnMapView(true)
,itsViewCacheMaxSizeInMB(15)
,itsUndoRedoDepth(5)
,fMakeDataValiditationAtSendingToDB(false)
,fMakeDataValiditationAutomatic(true)
,fUseDataValiditation_PrForm_T(true)
,itsDataValiditation_SnowTemperatureLimit(0)
,itsDataValiditation_RainTemperatureLimit(1)
,fUseDataValiditation_T_DP(false)
,fDefaultPrecipitationTypeIsLargeScale(true)
,fUseAutoSave(false)
,itsAutoSaveFrequensInMinutes(0)
,fControlPointMode(false)
,fDisableWindowManipulations(false)
,itsBaseNameSpace()
{
}

NFmiMetEditorOptionsData::~NFmiMetEditorOptionsData(void)
{
}

void NFmiMetEditorOptionsData::InitFromSettings(const std::string &theBaseNameSpace)
{
	itsBaseNameSpace = theBaseNameSpace;

	fDataValidationEnabled = NFmiSettings::Require<bool>("MetEditor::DataValidationEnabled"); // HUOM! tässä on jostain syystä fiksattu namespace
	fShowToolTipsOnMapView = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::ShowToolTips"));
	fMakeDataValiditationAtSendingToDB = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::MakeDataValiditationAtSendingToDB"));
	fMakeDataValiditationAutomatic = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::MakeDataValiditationAutomatic"));
	fUseDataValiditation_PrForm_T = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseDataValiditation_PrForm_T"));
	fUseDataValiditation_T_DP = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseDataValiditation_T_DP"));
	itsDataValiditation_SnowTemperatureLimit = NFmiSettings::Require<float>(std::string(itsBaseNameSpace + "::DataValiditation_SnowTemperatureLimit"));
	itsDataValiditation_RainTemperatureLimit = NFmiSettings::Require<float>(std::string(itsBaseNameSpace + "::DataValiditation_RainTemperatureLimit"));
	fDefaultPrecipitationTypeIsLargeScale = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::DefaultPrecipitationTypeIsLargeScale"));
	itsViewCacheMaxSizeInMB = NFmiSettings::Require<float>(std::string(itsBaseNameSpace + "::ViewCacheMaxSizeInMB"));
	itsUndoRedoDepth = NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::UndoRedoDepth"));
	fUseAutoSave = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseAutoSave"));
	itsAutoSaveFrequensInMinutes = NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::AutoSaveFrequensInMinutes"));
	fDisableWindowManipulations = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::DisableWindowManipulations"));
}

void NFmiMetEditorOptionsData::StoreToSettings(void)
{
	if(itsBaseNameSpace.empty() == false)
	{
		NFmiSettings::Set("MetEditor::DataValidationEnabled", NFmiStringTools::Convert<bool>(fDataValidationEnabled), true); // HUOM! tässä on jostain syystä fiksattu namespace
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::ShowToolTips"), NFmiStringTools::Convert(fShowToolTipsOnMapView), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::MakeDataValiditationAtSendingToDB"), NFmiStringTools::Convert(fMakeDataValiditationAtSendingToDB), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::MakeDataValiditationAutomatic"), NFmiStringTools::Convert(fMakeDataValiditationAutomatic), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseDataValiditation_PrForm_T"), NFmiStringTools::Convert(fUseDataValiditation_PrForm_T), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseDataValiditation_T_DP"), NFmiStringTools::Convert(fUseDataValiditation_T_DP), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::DataValiditation_SnowTemperatureLimit"), NFmiStringTools::Convert(itsDataValiditation_SnowTemperatureLimit), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::DataValiditation_RainTemperatureLimit"), NFmiStringTools::Convert(itsDataValiditation_RainTemperatureLimit), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::DefaultPrecipitationTypeIsLargeScale"), NFmiStringTools::Convert(fDefaultPrecipitationTypeIsLargeScale), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::ViewCacheMaxSizeInMB"), NFmiStringTools::Convert(itsViewCacheMaxSizeInMB), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UndoRedoDepth"), NFmiStringTools::Convert(itsUndoRedoDepth), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseAutoSave"), NFmiStringTools::Convert(fUseAutoSave), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::AutoSaveFrequensInMinutes"), NFmiStringTools::Convert(itsAutoSaveFrequensInMinutes), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::DefaultPrecipitationTypeIsLargeScale"), NFmiStringTools::Convert(fDefaultPrecipitationTypeIsLargeScale), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::DisableWindowManipulations"), NFmiStringTools::Convert(fDisableWindowManipulations), true);
	}
	else
		throw std::runtime_error("Error in NFmiMetEditorOptionsData::StoreToSettings, unable to store setting.");
}

void NFmiMetEditorOptionsData::ViewCacheMaxSizeInMB(double newSize)
{
	if(newSize == 0.) // tällöin ei käytetä cache:a ollenkaan
		itsViewCacheMaxSizeInMB = newSize;
	else if(newSize >= MinViewCacheSizeInMB)
		itsViewCacheMaxSizeInMB = newSize;
	else
		itsViewCacheMaxSizeInMB = MinViewCacheSizeInMB;
}

void NFmiMetEditorOptionsData::DefaultPrecipitationTypeIsLargeScale(bool newState)
{
	fDefaultPrecipitationTypeIsLargeScale = newState;
	NFmiWeatherAndCloudiness temp;
	temp.DefaultPrecipitationTypeIsLargeScale(newState);
}

void NFmiMetEditorOptionsData::UndoRedoDepth(int newValue)
{
	if(newValue < 0)
		itsUndoRedoDepth = 0;
	else if(newValue > 10) // 10 on maksimi
		itsUndoRedoDepth = 10;
	else
		itsUndoRedoDepth = newValue;
}

void NFmiMetEditorOptionsData::AutoSaveFrequensInMinutes(int newValue)
{
	if(newValue < 1)
		newValue = 1;
	itsAutoSaveFrequensInMinutes = newValue;
}

bool NFmiMetEditorOptionsData::MakeDataValiditationAtSendingToDB(void)
{
	return (fDataValidationEnabled && fMakeDataValiditationAtSendingToDB);
}
bool NFmiMetEditorOptionsData::MakeDataValiditationAutomatic(void)
{
	return (fDataValidationEnabled && fMakeDataValiditationAutomatic);
}
bool NFmiMetEditorOptionsData::UseDataValiditation_PrForm_T(void)
{
	return (fDataValidationEnabled && fUseDataValiditation_PrForm_T);
}
bool NFmiMetEditorOptionsData::UseDataValiditation_T_DP(void)
{
	return (fDataValidationEnabled && fUseDataValiditation_T_DP);
}
