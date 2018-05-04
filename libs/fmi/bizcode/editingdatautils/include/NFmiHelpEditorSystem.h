//� Ilmatieteenlaitos/Marko.
// Original 29.8.2006
// 
// Luokka pit�� huolta helpEditorMoodin asioista
// eli usean editoijan systeemist�, miss� yksi on p��editoija
// ja muita metkuja toisilla editori-koneilla voi osallistua 
// apueditoijina.
//---------------------------------------------------------- NFmiHelpEditorSystem.h

#pragma once

#include "NFmiColor.h"

class NFmiHelpEditorSystem
{
public:

	NFmiHelpEditorSystem(void);
	~NFmiHelpEditorSystem(void);

	void InitFromSettings(const std::string &theInitNameSpace);
	bool Use(void) const {return fUse;}
	void Use(bool newValue) {fUse = newValue;}
	// UsedHelpEditorStatus-metodi palauttaa onko editori help-moodissa ottaen huomioon ett� systeemin pit�� olla my�s k�yt�ss�
	bool UsedHelpEditorStatus(void) const {return fUse ? fHelpEditor : false;}
	bool HelpEditor(void) const {return fHelpEditor;}
	void HelpEditor(bool newValue) {fHelpEditor = newValue;}
	const std::string& DataPath(void) const {return itsDataPath;}
	void DataPath(const std::string &newValue) {itsDataPath = newValue;}
	const std::string& FileNameBase(void) const {return itsFileNameBase;}
	void FileNameBase(const std::string &newValue) {itsFileNameBase = newValue;}
	const NFmiColor& HelpColor(void) const {return itsHelpColor;}
	void HelpColor(const NFmiColor &newValue) {itsHelpColor = newValue;}
	void StoreSettings(bool fStoreHelpEditorStatus);
private:
	bool fUse; // onko systeemi k�yt�ss� vai ei
	bool fHelpEditor; // onko t�m� yksil� apu-editori, jos ei ole, niin on oletus arvoisesti p��editori
	std::string itsDataPath; // minne datat talletetaan
	std::string itsFileNameBase; // esim. help_pal_data.sqd, jolloin kun siihen lis�t��n aikaleima, 
								// siit� tulee YYYYMMDDHHmmss_help_pal_data.sqd, joka talletetaan dataPath-polkuun.
	NFmiColor itsHelpColor; // t�m� v�ri on ns. help-mode teemav�ri, eli t�t� v�ri� yritet��n kaytt�m��n tarvittavissa paikoissa
							// kertomaan k�ytt�j�lle ett� nyt ollaan ns. help-modessa.
	std::string itsInitNameSpace; // init:iss� talletetaan t�h�n halutta pohja namespace stringi
};

