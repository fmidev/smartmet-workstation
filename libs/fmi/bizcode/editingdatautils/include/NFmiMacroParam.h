#pragma once

#include "boost/shared_ptr.hpp"

class NFmiDrawParam;

// NFmiMacroParam-luokka pit‰‰ sis‰ll‰‰n vain skriptin, nimen ja drawparamin.
// Sill‰ ei ole mit‰‰n korkeampia toiminoja kuin asetus ja palautus funktiot.
// Paitsi osaa lukea ja tallettaa itsens‰ kahteen tiedostoon kun on annettu
// haluttu hakemisto, mihin macroparam halutaan tallettaa.
class NFmiMacroParam
{
public:
	NFmiMacroParam(void);

	const std::string& Name(void) const {return itsName;}
	void Name(const std::string &newValue) {itsName = newValue;}
	const std::string& MacroText(void) const {return itsMacroText;}
	void MacroText(const std::string &newValue) {itsMacroText = newValue;}
	boost::shared_ptr<NFmiDrawParam> DrawParam(void) {return itsDrawParam;}
	void DrawParam(boost::shared_ptr<NFmiDrawParam> &newValue) {itsDrawParam = newValue;}
	bool IsMacroParamDirectory(void) const {return fMacroParamDirectory;}
	void SetMacroParamDirectory(bool newState) {fMacroParamDirectory = newState;}
	const std::string& MacroParaDirectoryPath(void) const {return itsMacroParaDirectoryPath;}
	void MacroParaDirectoryPath(const std::string &thePath) {itsMacroParaDirectoryPath = thePath;}
	bool ErrorInMacro(void) const {return fErrorInMacro;}
	void ErrorInMacro(bool newValue) {fErrorInMacro = newValue;}

	bool Store(const std::string &thePath, const std::string &theName);
	bool Load(const std::string &thePath, const std::string &theName);
private:

	std::string itsName;
	std::string itsMacroText; // smarttool skripti
	boost::shared_ptr<NFmiDrawParam> itsDrawParam; // piirto-ominaisuudet
	std::string itsMacroParaDirectoryPath; // jos kyseess‰ on hakemisto, laitetaan t‰h‰n talteen koko polku
	bool fMacroParamDirectory; // jos t‰m‰ on true, valinta dialogissa toimitaan erilailla
							  // lis‰ksi t‰ll‰ist‰ makroa ei talleteta tiedostoon
							  // T‰m‰n avulla on tarkoitus laittaa kansio systeemi macroParameihin
	bool fErrorInMacro;
};
bool operator==(const NFmiMacroParam &obj1, const NFmiMacroParam &obj2);
