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

// Luokka, joka pit�� tiedot Meteorologin Editorin eri optioista.

#include <string>

const double MinViewCacheSizeInMB = 5; // t�t� alemmaksi ei voi n�yt�n v�limuistin kokoa asettaa

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
	bool fDataValidationEnabled; // t�m� p��tt�� onko datavalidaatio systeemi k�yt�ss� ja n�kyviss� ollenkaan (t�t� ei alusteta Init:iss�, vaan NFmiGenDocissa se alustetaan settingseist�)
	bool fShowToolTipsOnMapView; // n�ytet��nk� kartan p��ll� tooltippej� vai ei?
	int itsUndoRedoDepth; // moninkertainen undu/redo toiminto on (0 = ei ole toimintoa)

// Tee oma luokka datan validoinnista
	bool fMakeDataValiditationAtSendingToDB;
	bool fMakeDataValiditationAutomatic; // tehd��n tarkastukset aina kun muokataan dataa, paitsi siveltimen yhteydess�
	bool fUseDataValiditation_PrForm_T;
	float itsDataValiditation_SnowTemperatureLimit; // alle t�m�n olevissa l�mp�tiloissa sateet muuttuvat lumisateiksi
							// n�iden v�liset alueet ovat r�nt��
	float itsDataValiditation_RainTemperatureLimit; // yli t�m�n olevissa l�mp�tiloissa vesisateet muuttuvat lumisateiksi
	bool fUseDataValiditation_T_DP;
	bool fDefaultPrecipitationTypeIsLargeScale;

	bool fUseAutoSave;
	int itsAutoSaveFrequensInMinutes;

	bool fControlPointMode; // true=tila p��ll� ja false = ei ole p��ll�
	bool fDisableWindowManipulations;

	std::string itsBaseNameSpace;
};

