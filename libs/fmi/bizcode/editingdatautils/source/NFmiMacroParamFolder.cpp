#include "NFmiMacroParamFolder.h"
#include "NFmiMacroParam.h"
#include "NFmiMacroParamFunctions.h"
#include "NFmiFileSystem.h"
#include "NFmiDrawParam.h"
#include "NFmiFileString.h"
#include <boost\algorithm\string.hpp>

using namespace std;

struct MacroParamFinder
{
    MacroParamFinder(const string& theName) :itsName(theName) {};
    bool operator()(boost::shared_ptr<NFmiMacroParam> &theItem)
    {
        return boost::iequals(theItem->Name(), itsName);
    }

    string itsName;
};

NFmiMacroParamFolder::NFmiMacroParamFolder(const std::string& thePath, const std::string& theRootPath)
:itsPath(thePath)
,itsRootPath(theRootPath)
,itsMacroParams()
,itsCurrentIndex(-1)
,fInitialized(false)
{
}

NFmiMacroParamFolder::~NFmiMacroParamFolder(void)
{
	Clear(); // tätä ei oikeasti tarvitsisi kutsua, koska ei ole mitään dynaamista dataa tuhottavana (tällä hetkellä)
}

void NFmiMacroParamFolder::Clear(void)
{
	itsMacroParams.clear();
}

// lukee kaikki harmoonisaattorit uudestaan itsPath-hakemistosta
void NFmiMacroParamFolder::RefreshMacroParams(void)
{
	Clear();
	std::vector<string> names(GetNames(itsPath));
	std::vector<string>::iterator it = names.begin();
	for( ; it != names.end(); ++it)
	{
		Load(*it);
	}
	fInitialized = true;
}

// etsii nimellä macroparamia ja asettaa currentin osoittamaan siihen
bool NFmiMacroParamFolder::Find(const std::string &theName)
{
	std::vector<boost::shared_ptr<NFmiMacroParam> >::iterator it = std::find_if(itsMacroParams.begin(), itsMacroParams.end(), MacroParamFinder(theName));
	if(it != itsMacroParams.end())
	{
		itsCurrentIndex = static_cast<int>(it - itsMacroParams.begin());
		return true;
	}
	itsCurrentIndex = -1;
	return false;
}

int NFmiMacroParamFolder::GetIndex(const std::string &theName)
{
	Find(theName);
	return itsCurrentIndex;
}

// etsii indeksillä (=prioriteetti) macroparamia ja asettaa currentin osoittamaan siihen
bool NFmiMacroParamFolder::Find(int theIndex)
{
	itsCurrentIndex = -1;
	if(theIndex >= 0 && theIndex < static_cast<int>(itsMacroParams.size()))
	{
		itsCurrentIndex = theIndex;
		return true;
	}
	return false;
}

// tällä voi säädellä Find:illa etsityn macroParamin arvoja ja asetuksia
boost::shared_ptr<NFmiMacroParam> NFmiMacroParamFolder::Current(void)
{
	if(itsCurrentIndex >= 0 && itsCurrentIndex < static_cast<int>(itsMacroParams.size()))
		return itsMacroParams[itsCurrentIndex];
	throw runtime_error("NFmiMacroParamFolder::Current: ei ole asetettu mitään macroParamia currentiksi.");
}

// poistaa nimetyn macroParamin (= listalta ja tiedostot)
bool NFmiMacroParamFolder::Remove(const std::string &theName)
{
	std::vector<NFmiMacroParam>::size_type oldSize = itsMacroParams.size();
	itsMacroParams.erase(std::remove_if(itsMacroParams.begin(), itsMacroParams.end(), MacroParamFinder(theName)), itsMacroParams.end());
	std::vector<NFmiMacroParam>::size_type newSize = itsMacroParams.size();
	RemoveFiles(theName);
    return oldSize != newSize;
}

bool NFmiMacroParamFolder::RemoveFiles(const std::string &theName)
{
	string drawParamFileName;
	string macroFileName;

	// poista ensin normaali makrot
	MacroParam::GetFileNames(itsPath, theName, drawParamFileName, macroFileName, NFmiInfoData::kMacroParam);
	NFmiFileSystem::RemoveFile(drawParamFileName);
	NFmiFileSystem::RemoveFile(macroFileName);

	// poista sitten q3 makrot
	MacroParam::GetFileNames(itsPath, theName, drawParamFileName, macroFileName, NFmiInfoData::kQ3MacroParam);
	NFmiFileSystem::RemoveFile(drawParamFileName);
	NFmiFileSystem::RemoveFile(macroFileName);
	return true;
}

// tallettaa nimetyn macroparamin (on listoilla) arvot tiedostoihin
bool NFmiMacroParamFolder::Save(const std::string & /* theName */ )
{
	return false;
}

// tallettaa macroparamin arvot tiedostoihin ja lisää listoihin (=save as, jos saman nimistä ei löydy)
// HUOM! Uusi juttu talletetaan aina prioriteetti listan viimeiseksi
bool NFmiMacroParamFolder::Save(boost::shared_ptr<NFmiMacroParam> theMacroParam)
{
	if(Find(theMacroParam->Name()))
		Current() = theMacroParam;
	else
		itsMacroParams.push_back(theMacroParam);

	return theMacroParam->Store(itsPath, string(""));
}

// palauttaa macroParam-dialogin listaa varten stringi-listan, jossa näkyy kunkin
// macroparamin nimi ja skripti
std::vector<std::string> NFmiMacroParamFolder::GetDialogListStrings(bool getQ3Macros)
{
	std::vector<string> strList;
	std::vector<NFmiMacroParam>::size_type ssize = itsMacroParams.size();
	std::vector<NFmiMacroParam>::size_type counter = 0;
	for( ; counter < ssize; counter++)
	{
		if(itsMacroParams[counter]->IsMacroParamDirectory() == false) // jos kyseessä ei ollut hakemisto, tarkistetaan makron tyyppi
		{
			if(getQ3Macros && itsMacroParams[counter]->DrawParam()->DataType() != NFmiInfoData::kQ3MacroParam)
				continue; // skipataan, jos haluttiin q3-makroja ja kyseeessä ei ollut sellainen
			if(getQ3Macros == false && itsMacroParams[counter]->DrawParam()->DataType() != NFmiInfoData::kMacroParam)
				continue; // skipataan, jos haluttiin macroParameja, eikä kyseessä ollut sellainen
		}

		string macroName = itsMacroParams[counter]->Name();
		if(itsMacroParams[counter]->ErrorInMacro())
			macroName += " (ERROR)";
		strList.push_back(macroName);
	}
	return strList;
}

// palauttaa listan macroparameita halutusta hakemistosta
// ja myös siinä olevat hakemistot
std::vector<std::string> NFmiMacroParamFolder::GetNames(const std::string &thePath)
{
	std::vector<string> returnVector;

	// listataan alkuun hakemistot ja jos ei olla rootissa, laitetaan vielä ..-hakemisto mukaan
	std::list<std::string> directories = NFmiFileSystem::Directories(thePath);
	std::list<std::string>::iterator itDir = directories.begin();
	std::list<std::string>::iterator itEndDir = directories.end();
	for( ; itDir != itEndDir; ++itDir)
	{
		// CVS -hakemistoa ei laiteta
		if(*itDir == "CVS")
			continue;
		// "this"-hakemistoa ei laiteta
		if(*itDir == ".")
			continue;
		// jos ollaan ns. root hakemistossa, ei up-hakemistoa laiteta
		if(itsRootPath == thePath && *itDir == "..")
			continue;
		std::string name("<");
		name += *itDir;
		name += ">";
		returnVector.push_back(name);
	}

	string pattern(thePath);
	pattern += "*.st"; // haetaan vain macro-tiedostot, koska nimi saadaan myös siitä (tai haluttaessa drawParam-tiedostosta)
	std::list<std::string> macroParamFiles(NFmiFileSystem::PatternFiles(pattern));
	std::list<std::string>::iterator it = macroParamFiles.begin();
	NFmiFileString fileString;
	for( ; it != macroParamFiles.end() ; ++it)
	{
		fileString = NFmiString(*it);
		returnVector.push_back(static_cast<char*>(fileString.Header()));
	}

	// lisätään myös q3-makrot listaan
	pattern = thePath;
	pattern += "*.q3"; // haetaan vain macro-tiedostot, koska nimi saadaan myös siitä (tai haluttaessa drawParam-tiedostosta)
	macroParamFiles = NFmiFileSystem::PatternFiles(pattern);
	it = macroParamFiles.begin();
	for( ; it != macroParamFiles.end() ; ++it)
	{
		fileString = NFmiString(*it);
		returnVector.push_back(static_cast<char*>(fileString.Header()));
	}


	return returnVector;
}

// lataa nimetyn harminisaattorin ja lisää listaan
bool NFmiMacroParamFolder::Load(const std::string &theName)
{
	boost::shared_ptr<NFmiMacroParam> tmp(new NFmiMacroParam);
	tmp->Name(theName);
	NFmiDataIdent tmpPar(tmp->DrawParam()->Param());
	tmpPar.GetParam()->SetName("MacroParam");
	tmp->DrawParam()->Param(tmpPar);
	tmp->DrawParam()->DataType(NFmiInfoData::kMacroParam);
	try
	{
		if(tmp->Load(itsPath, theName))
		{
			tmp->DrawParam()->MacroParamRelativePath(MacroParam::GetRelativePath(itsRootPath, itsPath));
			itsMacroParams.push_back(tmp);
			return true;
		}
	}
	catch(...)
	{
	}
	tmp->ErrorInMacro(true);
	itsMacroParams.push_back(tmp);
	return false;
}

void NFmiMacroParamFolder::StoreMacroParams(void)
{
	std::vector<NFmiMacroParam>::size_type ssize = itsMacroParams.size();
	std::vector<NFmiMacroParam>::size_type i = 0;
	for( ; i < ssize; i++)
		itsMacroParams[i]->Store(itsPath, "");
}

bool operator==(const NFmiMacroParamFolder &obj1, const NFmiMacroParamFolder &obj2)
{
	if(obj1.Path() == obj2.Path() && obj1.RootPath() == obj2.RootPath())
	{
		if(MacroParam::CheckSharedPtrContainerEquality(obj1.MacroParams(), obj2.MacroParams()))
			return true;
	}
	return false;
}
