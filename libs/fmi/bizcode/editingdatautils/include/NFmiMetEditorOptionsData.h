//**********************************************************
// C++ Class Name : NFmiMetEditorOptionsData
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: NFmiMetEditorOptionsData.h
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

#pragma once

// Luokka, joka pitää tiedot Meteorologin Editorin eri optioista.

#include <string>

const double MinViewCacheSizeInMB = 5; // tätä alemmaksi ei voi näytön välimuistin kokoa asettaa

class NFmiMetEditorOptionsData
{
public:
	NFmiMetEditorOptionsData(void);
	~NFmiMetEditorOptionsData(void);

	void InitFromSettings(const std::string &theBaseNameSpace);
	void StoreToSettings(void);

	bool ShowToolTipsOnMapView(void){return fShowToolTipsOnMapView;}
	void ShowToolTipsOnMapView(bool newState){fShowToolTipsOnMapView = newState;}
	int UndoRedoDepth(void){return itsUndoRedoDepth;}
	void UndoRedoDepth(int newValue);

	bool MakeDataValiditationAtSendingToDB(void);
	void MakeDataValiditationAtSendingToDB(bool newState){fMakeDataValiditationAtSendingToDB = newState;};
	bool MakeDataValiditationAutomatic(void);
	void MakeDataValiditationAutomatic(bool newState){fMakeDataValiditationAutomatic = newState;};
	bool UseDataValiditation_PrForm_T(void);
	void UseDataValiditation_PrForm_T(bool newState){fUseDataValiditation_PrForm_T = newState;};
	float SnowTemperatureLimit(void){return itsDataValiditation_SnowTemperatureLimit;};
	void SnowTemperatureLimit(float newValue){itsDataValiditation_SnowTemperatureLimit = newValue;};
	float RainTemperatureLimit(void){return itsDataValiditation_RainTemperatureLimit;};
	void RainTemperatureLimit(float newValue){itsDataValiditation_RainTemperatureLimit = newValue;};
	bool UseDataValiditation_T_DP(void);
	void UseDataValiditation_T_DP(bool newState){fUseDataValiditation_T_DP = newState;};
	bool DefaultPrecipitationTypeIsLargeScale(void){return fDefaultPrecipitationTypeIsLargeScale;};
	void DefaultPrecipitationTypeIsLargeScale(bool newState);

	bool UseAutoSave(void){return fUseAutoSave;};
	void UseAutoSave(bool newState){fUseAutoSave = newState;};
	int AutoSaveFrequensInMinutes(void){return itsAutoSaveFrequensInMinutes;};
	void AutoSaveFrequensInMinutes(int newValue);
	bool ControlPointMode(void){return fControlPointMode;};
	void ControlPointMode(bool newState){fControlPointMode = newState;};

	bool DataValidationEnabled(void) const { return fDataValidationEnabled;	}
	void DataValidationEnabled(bool newValue) {	fDataValidationEnabled = newValue; }
	bool DisableWindowManipulations(void) const {return fDisableWindowManipulations;}
	void DisableWindowManipulations(bool newValue) {fDisableWindowManipulations = newValue;}

private:
	bool fDataValidationEnabled; // tämä päättää onko datavalidaatio systeemi käytössä ja näkyvissä ollenkaan (tätä ei alusteta Init:issä, vaan NFmiGenDocissa se alustetaan settingseistä)
	bool fShowToolTipsOnMapView; // näytetäänkö kartan päällä tooltippejä vai ei?
	int itsUndoRedoDepth; // moninkertainen undu/redo toiminto on (0 = ei ole toimintoa)

// Tee oma luokka datan validoinnista
	bool fMakeDataValiditationAtSendingToDB;
	bool fMakeDataValiditationAutomatic; // tehdään tarkastukset aina kun muokataan dataa, paitsi siveltimen yhteydessä
	bool fUseDataValiditation_PrForm_T;
	float itsDataValiditation_SnowTemperatureLimit; // alle tämän olevissa lämpötiloissa sateet muuttuvat lumisateiksi
							// näiden väliset alueet ovat räntää
	float itsDataValiditation_RainTemperatureLimit; // yli tämän olevissa lämpötiloissa vesisateet muuttuvat lumisateiksi
	bool fUseDataValiditation_T_DP;
	bool fDefaultPrecipitationTypeIsLargeScale;

	bool fUseAutoSave;
	int itsAutoSaveFrequensInMinutes;

	bool fControlPointMode; // true=tila päällä ja false = ei ole päällä
	bool fDisableWindowManipulations;

	std::string itsBaseNameSpace;
};

