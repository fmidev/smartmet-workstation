#pragma once

#include "boost/shared_ptr.hpp"
#include <vector>
#include "NFmiMacroParamItem.h"

class NFmiStopFunctor;
class NFmiMacroParamFolder;
class NFmiMacroParam;
class NFmiMacroParamItem;

// Luokka huolehtii NFmiMacroParam olioista, joita on talletettu Meteorologin
// Editorilla.T�t� luokka k�ytet��n dokumentti - luokan kautta.
// MacroParam on smarttool - skripti, joka piirret��n karttan�yt�lle.
// Taikasana on RESULT eli esim.RESULT = T - DP
// on macroParam joka piirt�� n�yt�lle l�mp�tilan ja kastepisteen eron.
// Miten se piirret��n, riippuu macroParamiin liittyv�st� DrawParamista.
// MacroParamit talletetaan haluttuun hakemistoon, joka annetaan
// MacroParamSysteemille ja se talletetaan kahdessa osassa.Skripti omaan
// tiedostoon ja drawParam omaan.Niit� linkitt�� macroParamin nimi joka
// tulee tiedoston nimiin.Eli esim.theta1 nimisest� macroparamista tulisi
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

	void RootPath(const std::string &newValue); // t�m� on initialisointi metodi ja my�s root hakis asetetaan
	const std::string& RootPath(void) const {return itsRootPath;}

    boost::shared_ptr<NFmiMacroParam> GetCurrentMacroParam(void) { return itsFoundMacroParam; }; // t�ll� saa edell� etsityn macroparamin
    boost::shared_ptr<NFmiMacroParamFolder> GetCurrentFolder(); // t�ll� voi s��dell� Find:illa etsityn folder-otuksen arvoja ja asetuksia
    boost::shared_ptr<NFmiMacroParamFolder> GetFolder(int index) const;
	bool SetCurrentToWantedMacroPath(const std::string &theTotalFileName); // tietyn nimist� tiedosto nime�
	bool FindMacroFromCurrentFolder(const std::string &theMacroName);
	bool FindMacroParamPath(const std::string& theRelativeMacroParamPath);
	int CurrentFolderIndex(void) const {return itsCurrentFolderIndex;}
    boost::shared_ptr<NFmiMacroParam> GetWantedMacro(const std::string& theTotalFileName) const;
	const std::vector<std::string>& MacroParamSpeedSearchPathNames(bool updateList);
	const std::string& CurrentPath(void) const {return itsCurrentPath;}
	std::string RelativePath(void) const;
    void CurrentPath(const std::string& newValue); // t�m� on alihakemistoon siirtymis metodi
    void SetCurrentPathByAbsolutePath(const std::string &absolutePath); // t�m� on hakemistoon siirtymis metodi
	std::vector<NFmiMacroParamItem>& MacroParamItemTree(void) {return itsMacroItemTree;}
	void Rebuild(NFmiStopFunctor *theStopFunctor);
	void SwapMacroData(NFmiMacroParamSystem &theOther);
	bool UpdateMacroParamListView(void) {return fUpdateMacroParamListView;}
	void UpdateMacroParamListView(bool newValue) {fUpdateMacroParamListView = newValue;}
	void EnsureRootFolderInitialized();
private:

	void AddToMacroParamItemTree(std::vector<NFmiMacroParamItem> &theMacroItemList, const std::vector<boost::shared_ptr<NFmiMacroParam> >& theMacroParams, NFmiStopFunctor *theStopFunctor);
	int FindPath(const std::string &thePathName) const; // etsii folder-otusta polku nimell�
	int SeekMacroFolder(const std::string &theTotalFileName);
	void ClearMacros(void);
	void InitMacroParamTree(NFmiStopFunctor *theStopFunctor);
	void MakeMacroParamItemTree(NFmiStopFunctor *theStopFunctor);
    void SetWantedPath(const std::string& wantedPath);
    CurrentMacroPointerData FindCurrentMacroPointerData(const std::string& theTotalFileName) const;
	void InitializeRootFolder();
	void MakeMacroParamSpeedSearchPathNames();

	std::string itsCurrentPath; // mihin paikkaan on macroparamit talletettu
	std::string itsRootPath; // t�m�n avulla saadaan kansio systeemille root kansio, josta yl�sp�in ei voi menn� ..-hakiksen kautta
	std::vector<boost::shared_ptr<NFmiMacroParamFolder> > itsMacroParamFolders; // t�ss� on kaikki macroParamit omissa foldereissaa vectorissa, siis kaikki ali hakemistot omina foldereinaan, eik� ne ole hierarkkisessa systeemiss� vaan ihan vektorissa vierekk�in
	std::vector<NFmiMacroParamItem> itsMacroItemTree; // t�m�n rakenteen avulla lis�t��n macroParamit mm. popup-valikkoon
	int itsCurrentFolderIndex; // currenttia macroparamia osoitetaan t�ll� indeksill� (=paikka itsMacroParamFolders vektorissa), alkaa 0:sta ja -1 kun ei osoita mihink��n
	boost::shared_ptr<NFmiMacroParam> itsFoundMacroParam; // Find(path, name)::lla l�ydetty macroParam
	bool fUpdateMacroParamListView; // t�m�n lipun avulla voidaan p�ivitt�� smartTool-dialogia ja sen p�ivityksi�
	std::vector<std::string> itsMacroParamSpeedSearchPathNames;
};

