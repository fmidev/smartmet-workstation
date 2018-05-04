//© Ilmatieteenlaitos/Marko.
// Original 19.2.2008
// 
// Luokka hanskaa käsitemallit eli rintamat sun muut. Tähän on
// koottu niin asetukset, että datan luku tietokannasta ja niiden tulkinta.
//---------------------------------------------------------- NFmiConceptualModelData.h

#pragma once

#include <string>

class NFmiConceptualModelData
{
public:

	NFmiConceptualModelData(void);
	~NFmiConceptualModelData(void);
	void InitFromSettings(const std::string &theInitNameSpace);

	bool Use(void) const {return fUse;}
	void Use(bool newValue) {fUse = newValue;}
	const std::string& DefaultUserName(void) const {return itsDefaultUserName;}
	void DefaultUserName(const std::string &newValue) {itsDefaultUserName = newValue;}
	const std::string& DataBaseUrl(void) const {return itsDataBaseUrl;}
	void DataBaseUrl(const std::string &newValue) {itsDataBaseUrl = newValue;}

	bool UseFileData(void) const {return fUseFileData;}
	void UseFileData(bool newValue) {fUseFileData = newValue;}
	bool UseWOML(void) const {return fUseWOML;}
	void UseWOML(bool newValue) {fUseWOML = newValue;}
	const std::string& FileFilter(void) const {return itsFileFilter;}
	void FileFilter(const std::string &newValue) {itsFileFilter = newValue;}

private:
	bool fUse; // onko systeemi käytössä vai ei
	std::string itsDefaultUserName; // minkä nimiseltä käyttäjältä luetaan datoja tietokannasta oletuksena
	std::string itsDataBaseUrl; // Mistä osoitteesta haetaan datoja

	bool fUseFileData; // is data read from server via http-calls or from files from certain directory (this dictates if itsDataBaseUrl or itsFileFilter is used)
	bool fUseWOML; // is new WOML- or old dataformat used
	std::string itsFileFilter; //  file-filter of used data-files with path and pattern

};


