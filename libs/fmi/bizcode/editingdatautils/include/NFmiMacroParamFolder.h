#pragma once

#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"

class NFmiMacroParam;

// t‰m‰ luokka pit‰‰ sis‰ll‰‰n yhden hakemiston makroparametrit
class NFmiMacroParamFolder
{
public:
	NFmiMacroParamFolder(const std::string& thePath, const std::string& theRootPath);
	~NFmiMacroParamFolder(void);

	void Clear(void);
	void RefreshMacroParams(void); // lukee kaikki macroparamit uudestaan itsPath-hakemistosta
	void StoreMacroParams(void); // tallettaa kaikki macroparamit tiedostoihinsa
	bool Find(const std::string &theName); // etsii nimell‰ macroparamia ja asettaa currentin osoittamaan siihen
	int GetIndex(const std::string &theName); // etsii nimell‰ macroparamia ja palauttaa sen indeksin eli prioriteetin
	bool Find(int theIndex);  // etsii indeksill‰ macroparamia ja asettaa currentin osoittamaan siihen
	boost::shared_ptr<NFmiMacroParam> Current(void); // t‰ll‰ voi s‰‰dell‰ Find:illa etsityn macroparamia arvoja ja asetuksia
	bool Remove(const std::string &theName); // poistaa nimetyn macroparamin (= listalta ja tiedostot)
	bool Save(const std::string &theName); // tallettaa nimetyn macroparamin (on listoilla) arvot tiedostoihin
	bool Save(boost::shared_ptr<NFmiMacroParam> theMacroParam); // tallettaa macroparamin arvot tiedostoihin ja lis‰‰ listoihin (=save as, jos saman nimist‰ ei lˆydy)
	std::vector<std::string> GetDialogListStrings(bool getQ3Macros); // palauttaa macroparam-dialogin listaa vatern stringi-listan, jossa n‰kyy kunkin macroparamin nimi

	const std::string& Path(void) const {return itsPath;}
	const std::string& RootPath(void) const {return itsRootPath;}
	int CurrentIndex(void) const {return itsCurrentIndex;}
	void CurrentIndex(int newValue) {itsCurrentIndex = newValue;}
	int Size(void) const {return static_cast<int>(itsMacroParams.size());}
	bool Initialized(void) const {return fInitialized;}
	void Initialized(bool newState) {fInitialized = newState;}
	const std::vector<boost::shared_ptr<NFmiMacroParam> >& MacroParams(void) const {return itsMacroParams;}
private:
	bool RemoveFiles(const std::string &theName);
	std::vector<std::string> GetNames(const std::string &thePath); // palauttaa listan macroparameja halutusta hakemistosta
	bool Load(const std::string &theName); // lataa nimetyn macroparamin ja lis‰‰ listaan

	std::string itsPath; // mihin paikkaan on macroparamit talletettu
	std::string itsRootPath;
	std::vector<boost::shared_ptr<NFmiMacroParam> > itsMacroParams;
	int itsCurrentIndex; // currenttia macroparamia osoitetaan t‰ll‰ indeksill‰ (=paikka itsMacroParams vektorissa), alkaa 0:sta ja -1 kun ei osoita mihink‰‰n
	bool fInitialized; // onko t‰m‰ 'hakemisto' jo luettu muistiin vai ei
};
bool operator==(const NFmiMacroParamFolder &obj1, const NFmiMacroParamFolder &obj2);
