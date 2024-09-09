#pragma once

#include "boost/shared_ptr.hpp"
#include <vector>
#include "NFmiMacroParamItem.h"

class NFmiStopFunctor;
class NFmiMacroParamFolder;
class NFmiMacroParam;
class NFmiMacroParamItem;

// Luokka huolehtii NFmiMacroParam olioista, joita on talletettu Meteorologin
// Editorilla.Tätä luokka käytetään dokumentti - luokan kautta.
// MacroParam on smarttool - skripti, joka piirretään karttanäytölle.
// Taikasana on RESULT eli esim.RESULT = T - DP
// on macroParam joka piirtää näytölle lämpötilan ja kastepisteen eron.
// Miten se piirretään, riippuu macroParamiin liittyvästä DrawParamista.
// MacroParamit talletetaan haluttuun hakemistoon, joka annetaan
// MacroParamSysteemille ja se talletetaan kahdessa osassa.Skripti omaan
// tiedostoon ja drawParam omaan.Niitä linkittää macroParamin nimi joka
// tulee tiedoston nimiin.Eli esim.theta1 nimisestä macroparamista tulisi
// tiedosto theta1.st(skripti) ja theta1.dpa(drawparam) tiedostot.

class NFmiMacroParamSystem
{
    class CurrentMacroPointerData
    {
    public:
        bool wasMacroFound() const;

        std::string currentPath;
        int currentFolderIndex = -1;
        boost::shared_ptr<NFmiMacroParam> foundMacroParam;
    };

public:
	NFmiMacroParamSystem(void);
	void Init(NFmiMacroParamSystem &theOther);
	bool IsUpdateNeeded(NFmiMacroParamSystem &theOther);

	void RootPath(const std::string &newValue); // tämä on initialisointi metodi ja myös root hakis asetetaan
	const std::string& RootPath(void) const {return itsRootPath;}

    boost::shared_ptr<NFmiMacroParam> GetCurrentMacroParam(void) { return itsFoundMacroParam; }; // tällä saa edellä etsityn macroparamin
    boost::shared_ptr<NFmiMacroParamFolder> GetCurrentFolder(); // tällä voi säädellä Find:illa etsityn folder-otuksen arvoja ja asetuksia
    boost::shared_ptr<NFmiMacroParamFolder> GetFolder(int index) const;
	bool SetCurrentToWantedMacroPath(const std::string &theTotalFileName); // tietyn nimistä tiedosto nimeä
	bool FindMacroFromCurrentFolder(const std::string &theMacroName);
	bool FindMacroParamPath(const std::string& theRelativeMacroParamPath);
	int CurrentFolderIndex(void) const {return itsCurrentFolderIndex;}
    boost::shared_ptr<NFmiMacroParam> GetWantedMacro(const std::string& theTotalFileName) const;
	bool ReloadDrawParamFromFile(const std::string& theTotalFileName);
	const std::vector<std::string>& MacroParamSpeedSearchPathNames(bool updateList);
	const std::string& CurrentPath(void) const {return itsCurrentPath;}
	std::string RelativePath(void) const;
    void CurrentPath(const std::string& newValue); // tämä on alihakemistoon siirtymis metodi
    void SetCurrentPathByAbsolutePath(const std::string &absolutePath); // tämä on hakemistoon siirtymis metodi
	std::vector<NFmiMacroParamItem>& MacroParamItemTree(void) {return itsMacroItemTree;}
	void Rebuild(NFmiStopFunctor *theStopFunctor);
	void SwapMacroData(NFmiMacroParamSystem& theOther);
	void UpdateToWorkingData(NFmiMacroParamSystem &theCurrentWorkingData);
	bool UpdateMacroParamListView(void) {return fUpdateMacroParamListView;}
	void UpdateMacroParamListView(bool newValue) {fUpdateMacroParamListView = newValue;}
	void EnsureRootFolderInitialized();
private:

	void AddToMacroParamItemTree(std::vector<NFmiMacroParamItem> &theMacroItemList, const std::vector<boost::shared_ptr<NFmiMacroParam> >& theMacroParams, NFmiStopFunctor *theStopFunctor);
	int FindPath(const std::string &thePathName) const; // etsii folder-otusta polku nimellä
	int SeekMacroFolder(const std::string &theTotalFileName);
	void ClearMacros(void);
	void InitMacroParamTree(NFmiStopFunctor *theStopFunctor);
	void MakeMacroParamItemTree(NFmiStopFunctor *theStopFunctor);
    void SetWantedPath(const std::string& wantedPath);
    CurrentMacroPointerData FindCurrentMacroPointerData(const std::string& theTotalFileName) const;
	void InitializeRootFolder();
	void MakeMacroParamSpeedSearchPathNames();
	void UpdateCurrentPath(const std::string& newCurrentPath);

	std::string itsCurrentPath; // mihin paikkaan on macroparamit talletettu
	std::string itsRootPath; // tämän avulla saadaan kansio systeemille root kansio, josta ylöspäin ei voi mennä ..-hakiksen kautta
	std::vector<boost::shared_ptr<NFmiMacroParamFolder> > itsMacroParamFolders; // tässä on kaikki macroParamit omissa foldereissaa vectorissa, siis kaikki ali hakemistot omina foldereinaan, eikä ne ole hierarkkisessa systeemissä vaan ihan vektorissa vierekkäin
	std::vector<NFmiMacroParamItem> itsMacroItemTree; // tämän rakenteen avulla lisätään macroParamit mm. popup-valikkoon
	int itsCurrentFolderIndex; // currenttia macroparamia osoitetaan tällä indeksillä (=paikka itsMacroParamFolders vektorissa), alkaa 0:sta ja -1 kun ei osoita mihinkään
	boost::shared_ptr<NFmiMacroParam> itsFoundMacroParam; // Find(path, name)::lla löydetty macroParam
	bool fUpdateMacroParamListView; // tämän lipun avulla voidaan päivittää smartTool-dialogia ja sen päivityksiä
	std::vector<std::string> itsMacroParamSpeedSearchPathNames;
};

