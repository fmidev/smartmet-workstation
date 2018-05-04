//© Ilmatieteenlaitos/Marko.
// Original 29.8.2006
// 
// Luokka pitää huolta helpEditorMoodin asioista
// eli usean editoijan systeemistä, missä yksi on pääeditoija
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
	// UsedHelpEditorStatus-metodi palauttaa onko editori help-moodissa ottaen huomioon että systeemin pitää olla myös käytössä
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
	bool fUse; // onko systeemi käytössä vai ei
	bool fHelpEditor; // onko tämä yksilö apu-editori, jos ei ole, niin on oletus arvoisesti pääeditori
	std::string itsDataPath; // minne datat talletetaan
	std::string itsFileNameBase; // esim. help_pal_data.sqd, jolloin kun siihen lisätään aikaleima, 
								// siitä tulee YYYYMMDDHHmmss_help_pal_data.sqd, joka talletetaan dataPath-polkuun.
	NFmiColor itsHelpColor; // tämä väri on ns. help-mode teemaväri, eli tätä väriä yritetään kayttämään tarvittavissa paikoissa
							// kertomaan käyttäjälle että nyt ollaan ns. help-modessa.
	std::string itsInitNameSpace; // init:issä talletetaan tähän halutta pohja namespace stringi
};

