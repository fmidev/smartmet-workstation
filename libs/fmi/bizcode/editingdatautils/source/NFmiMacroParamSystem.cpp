#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParamFolder.h"
#include "NFmiMacroParamItem.h"
#include "NFmiMacroParam.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiMacroParamFunctions.h"
#include "NFmiPathUtils.h"
#include "NFmiDrawParam.h"
#include "boost/algorithm/string.hpp"
#include <list>

using namespace std;

struct PathFinderFunctor
{
	PathFinderFunctor(const std::string &thePath):itsPath(thePath){}

	bool operator()(boost::shared_ptr<NFmiMacroParamFolder> theFolder)
	{
		return boost::iequals(theFolder->Path(), itsPath);
	}

	std::string itsPath;
};

// lisätään kaikki alihakemistot mukaan,
// jos niitä on ja niitä ei ole jo lisätty,
// mutta ei initialisoida niitä, eli ei ladata niiden alihakemistoja tai macroparameja
static void InsertAllSubdirectories(std::vector<boost::shared_ptr<NFmiMacroParamFolder> > &theMacroParamFolders, const std::string &thePath, const std::string &theRootPath, NFmiStopFunctor *theStopFunctor)
{
	std::list<std::string> directories = NFmiFileSystem::Directories(thePath);
	std::list<std::string>::iterator itDir = directories.begin();
	std::list<std::string>::iterator itEndDir = directories.end();
	for( ; itDir != itEndDir; ++itDir)
	{
		NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
		if(*itDir == "..")
			continue;
		if(*itDir == ".")
			continue;
		if(*itDir == "CVS")
			continue;

		std::string totalPath(thePath);
		totalPath += *itDir;
		totalPath = MacroParam::ConvertPathToOneUsedFormat(totalPath);
		std::vector<boost::shared_ptr<NFmiMacroParamFolder> >::iterator it = std::find_if(theMacroParamFolders.begin(), theMacroParamFolders.end(), PathFinderFunctor(totalPath));
		if(it == theMacroParamFolders.end())
		{ // jos hakista ei ollut aiemmin, voidaan se lisätä
			boost::shared_ptr<NFmiMacroParamFolder> tmp(new NFmiMacroParamFolder(totalPath, theRootPath));
			theMacroParamFolders.push_back(tmp);
		}
	}
}

NFmiMacroParamSystem::NFmiMacroParamSystem(void)
:itsCurrentPath()
,itsRootPath()
,itsMacroParamFolders()
,itsCurrentFolderIndex(-1)
,itsFoundMacroParam()
,fUpdateMacroParamListView(true)
{
}

// tämä on initialisointi metodi ja myös root hakis asetetaan
void NFmiMacroParamSystem::RootPath(const std::string &newValue)
{
	ClearMacros();

	itsRootPath = itsCurrentPath = MacroParam::ConvertPathToOneUsedFormat(newValue);
}

// Tarkoitus on siirtää this-oliosta theOther -olioon kaiki makroParam data.
// pitää tehdä vielä niin että lopuksi this-olion current osoittaa oikeaan paikkaan
void NFmiMacroParamSystem::SwapMacroData(NFmiMacroParamSystem &theOther)
{
	fUpdateMacroParamListView = true;
	this->itsMacroParamFolders.swap(theOther.itsMacroParamFolders);
	this->itsMacroItemTree.swap(theOther.itsMacroItemTree);

	UpdateCurrentPath(itsCurrentPath);
}

// Kun this olio (johon ladattu uusimmat macroParam tiedosto/hakemisto rakenteet) 
// halutaan ottaa käyttöön dokumentissa, päivitetään se tällä metodilla tarvittavilla 
// working-data asetuksilla.
void NFmiMacroParamSystem::UpdateToWorkingData(NFmiMacroParamSystem& theCurrentWorkingData)
{
	fUpdateMacroParamListView = true;
	this->itsRootPath = theCurrentWorkingData.itsRootPath;
	// itsMacroParamFolders ja itsMacroItemTree pidetään this oliosta
	auto currentMacroParam = theCurrentWorkingData.GetCurrentMacroParam();
	if(currentMacroParam && !currentMacroParam->IsMacroParamDirectory())
	{
		std::string totalMacroParamPath = theCurrentWorkingData.itsCurrentPath + currentMacroParam->Name() + ".st";
		SetCurrentToWantedMacroPath(totalMacroParamPath);
	}
	else
	{
		UpdateCurrentPath(theCurrentWorkingData.itsCurrentPath);
	}
	MakeMacroParamSpeedSearchPathNames();
}

void NFmiMacroParamSystem::UpdateCurrentPath(const std::string &newCurrentPath)
{
	itsCurrentPath = newCurrentPath;
	// Laita vielä osoittamaan oikeisiin current otuksiin
	int pathIndex = FindPath(itsCurrentPath);
	if(pathIndex >= 0)
	{
		if(itsFoundMacroParam.get())
			FindMacroFromCurrentFolder((*itsFoundMacroParam).Name());
		if(itsCurrentFolderIndex < 0 && itsMacroParamFolders.size() > 0)
			itsCurrentFolderIndex = 0; // jos on polku ok ja löytyy macroParam-hakemisto, laitetaan osoittamaan roottiin, jos muuten indksi osoittaa -1:een
	}
}

void NFmiMacroParamSystem::InitializeRootFolder()
{
	boost::shared_ptr<NFmiMacroParamFolder> tmp(new NFmiMacroParamFolder(itsCurrentPath, itsRootPath));
	tmp->RefreshMacroParams();
	itsMacroParamFolders.push_back(tmp);
	itsCurrentFolderIndex = 0;
}

void NFmiMacroParamSystem::EnsureRootFolderInitialized()
{
	if(itsMacroParamFolders.empty())
	{
		InitializeRootFolder();
	}
	else if(itsCurrentFolderIndex < 0)
	{
		itsCurrentFolderIndex = 0;
	}
}

// Rakennetaan kaikki macroParamit ja polut uudestaan
void NFmiMacroParamSystem::Rebuild(NFmiStopFunctor *theStopFunctor)
{
	ClearMacros();
	NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
	InitializeRootFolder();
	NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
	InsertAllSubdirectories(itsMacroParamFolders, itsCurrentPath, itsRootPath, theStopFunctor);
	NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
	InitMacroParamTree(theStopFunctor);
	NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
	MakeMacroParamItemTree(theStopFunctor);
}

// tässä this-olio tyhjennetään ja alustetaan vain niin että root-polku otetaan
// theOther-otuksesta.
void NFmiMacroParamSystem::Init(NFmiMacroParamSystem &theOther)
{
	RootPath(theOther.RootPath());
}

// Kun worker-thread on päivittänyt macroParamit, on syytä tarkistaa, että
// tarvitaanko pääohjelman oliota oikeasti päivittää, eli ovatko makro rakenteet jo samoja.
// Palauttaa true, jos rakenteet ovat jotenkin erilaisia.
bool NFmiMacroParamSystem::IsUpdateNeeded(NFmiMacroParamSystem &theOther)
{
	if(itsRootPath == theOther.itsRootPath)
	{
		if(MacroParam::CheckSharedPtrContainerEquality(itsMacroParamFolders, theOther.itsMacroParamFolders))
		{
			if(itsMacroItemTree == theOther.itsMacroItemTree)
				return false;
		}
	}
	return true;
}

// etsii folder-otusta polku nimellä
// palauta sen indeksi, -1 jos ei löydy
// jos löytyy, ei aseta currentiksi folderiksi!
int NFmiMacroParamSystem::FindPath(const std::string &thePathName) const
{
	auto it = std::find_if(itsMacroParamFolders.begin(), itsMacroParamFolders.end(), PathFinderFunctor(thePathName));
	if(it == itsMacroParamFolders.end())
		return -1;
	else
		return static_cast<int>(it - itsMacroParamFolders.begin());
}

// tällä voi säädellä Find:illa etsityn folder-otuksen arvoja ja asetuksia
boost::shared_ptr<NFmiMacroParamFolder> NFmiMacroParamSystem::GetCurrentFolder()
{
    return GetFolder(itsCurrentFolderIndex);
}

boost::shared_ptr<NFmiMacroParamFolder> NFmiMacroParamSystem::GetFolder(int index) const
{
    if(index >= 0 && index < itsMacroParamFolders.size())
        return itsMacroParamFolders[index];
    else
        return boost::shared_ptr<NFmiMacroParamFolder>();
}

// etsii currentista folderista haluttua makroa
// jos löytää, asettaa sen itsFoundMacroParam:iksi
bool NFmiMacroParamSystem::FindMacroFromCurrentFolder(const std::string &theMacroName)
{
	boost::shared_ptr<NFmiMacroParamFolder> currentFolder = GetCurrentFolder();
	if(currentFolder && currentFolder->Find(theMacroName))
	{
		itsFoundMacroParam = currentFolder->Current();
		return true;
	}
	return false;
}

// Etsii annetulla suhteellisella polulla olevaa macroParamia (käytetään speed-search kontrollista).
// Jos löytää, asetetaan se itsFoundMacroParam:iksi.
bool NFmiMacroParamSystem::FindMacroParamPath(const std::string& theRelativeMacroParamPath)
{
	auto totalMacroParamPath = itsRootPath + theRelativeMacroParamPath;
	return SetCurrentToWantedMacroPath(totalMacroParamPath);
}

const std::vector<std::string>& NFmiMacroParamSystem::MacroParamSpeedSearchPathNames(bool updateList)
{
	if(updateList)
	{
//		Rebuild(nullptr);
		MakeMacroParamSpeedSearchPathNames();
	}
	return itsMacroParamSpeedSearchPathNames;
}

void NFmiMacroParamSystem::MakeMacroParamSpeedSearchPathNames()
{
	std::vector<std::string> pathNames;
	for(const auto& folder : itsMacroParamFolders)
	{
		auto relativePath = PathUtils::getRelativePathIfPossible(folder->Path(), folder->RootPath());
		const auto& macroParams = folder->MacroParams();
		for(const auto& macroParam : macroParams)
		{
			if(!macroParam->IsMacroParamDirectory())
			{
				std::string pathName = relativePath + macroParam->Name();
				pathNames.push_back(pathName);
			}
		}
	}
	itsMacroParamSpeedSearchPathNames = pathNames;
}

bool NFmiMacroParamSystem::CurrentMacroPointerData::wasMacroFound() const
{
    if(currentPath.empty() || currentFolderIndex < 0 || foundMacroParam == nullptr)
        return false;
    else
        return true;
}

NFmiMacroParamSystem::CurrentMacroPointerData NFmiMacroParamSystem::FindCurrentMacroPointerData(const std::string& theTotalFileName) const
{
    // 1. irrota polku theTotalFileName:sta
    NFmiFileString fileStr(theTotalFileName);
    std::string pathStr(fileStr.Device());
    pathStr += fileStr.Path();
    pathStr = MacroParam::ConvertPathToOneUsedFormat(pathStr);
    // 2. etsi itsMacroParamFolders-listasta folderi joka on tämä polku
    int folderIndex = FindPath(pathStr);
    if(folderIndex != -1)
    {
        // 2.2 jos löytyy hakemisto
        // 2.2.1 initilaisoi folderi tarvittaessa
        auto macroParamFolder = GetFolder(folderIndex);
        if(macroParamFolder)
        {
            if(!macroParamFolder->Initialized())
                macroParamFolder->RefreshMacroParams();
        }

        // 2.2.2 irrota theTotalFileName:sta ns. lyhenne eli ilman polkua ja ilman extensiota
        std::string fileNameStr(fileStr.Header());
        // 2.2.3 etsi lyhennettä folderista
        if(macroParamFolder->Find(fileNameStr))
        {
            // 2.2.4 jos löytyy alusta CurrentMacroPointerData olio ja palauta se
            return CurrentMacroPointerData{ pathStr , folderIndex, macroParamFolder->Current() };
        }
    }
    // 2.1 jos ei löydy, palauta false
    return CurrentMacroPointerData{};
}

bool NFmiMacroParamSystem::SetCurrentToWantedMacroPath(const std::string &theTotalFileName)
{
	itsFoundMacroParam.reset();
    auto currentMacroPointerData = FindCurrentMacroPointerData(theTotalFileName);
    if(currentMacroPointerData.wasMacroFound())
    {
        itsCurrentPath = currentMacroPointerData.currentPath;
        itsCurrentFolderIndex = currentMacroPointerData.currentFolderIndex;
        itsFoundMacroParam = currentMacroPointerData.foundMacroParam;
        return true;
    }

	return false;
}

boost::shared_ptr<NFmiMacroParam> NFmiMacroParamSystem::GetWantedMacro(const std::string& theTotalFileName) const
{
    auto currentMacroPointerData = FindCurrentMacroPointerData(theTotalFileName);
    if(currentMacroPointerData.wasMacroFound())
        return currentMacroPointerData.foundMacroParam;
    else
        return nullptr;
}

// theTotalFileName parametri viittaa drawParam tiedostoon.
bool NFmiMacroParamSystem::ReloadDrawParamFromFile(const std::string& theTotalFileName)
{
	auto foundMacroParam = GetWantedMacro(theTotalFileName);
	if(foundMacroParam)
	{
		// Pitää ladata erikseen originaali drawParam asetukset omaan olioon ja sen avulla initialisoida käytössä olevan asetukset
		NFmiDrawParam origDrawParam;
		if(origDrawParam.Init(theTotalFileName))
		{
			foundMacroParam->DrawParam()->Init(&origDrawParam, true);
			return true;
		}
	}
	return false;
}

// Tämä on hakemistoon siirtymis metodi, tätä käytetään jos file-browserilla talletetaan
// uusi macroParam mahdollisesti täysin uuteen hakemistoon (file-browserilla tehty hakemisto).
void NFmiMacroParamSystem::SetCurrentPathByAbsolutePath(const std::string& absolutePath)
{
    if(!absolutePath.empty())
    {
        Rebuild(nullptr);
        NFmiFileString fileString(absolutePath);
        std::string wantedPath = fileString.Device() + fileString.Path();
        SetWantedPath(wantedPath);
    }
}

// tämä on alihakemistoon siirtymis metodi
void NFmiMacroParamSystem::CurrentPath(const std::string &newValue)
{
	// aivan aluksi lisätään varmuuden vuoksi kaikki hakemistot, mitkä ovat mahd. lisätty currenttiin hakemistoon
	InsertAllSubdirectories(itsMacroParamFolders, itsCurrentPath, itsRootPath, 0);

	// tämä on alihakemistoon siirtymis metodi
	// newValue on muotoa <hakis>, josta kulma sulut pitää ottaa pois ja lisätä nykyiseen polkuun
	// piatsi jos tulee <..>, jolloin pitää mennä yksi hakemisto taso ylös
	std::string usedDirectoryName(newValue);
	NFmiStringTools::TrimL(usedDirectoryName, '<');
	NFmiStringTools::TrimR(usedDirectoryName, '>');

    std::string wantedPath = itsCurrentPath;
	if(usedDirectoryName == "..")
	{
		MacroParam::RemoveLastPartOfDirectory(wantedPath);
	}
	else
	{
		wantedPath += usedDirectoryName;
	}

    SetWantedPath(wantedPath);
}

void NFmiMacroParamSystem::SetWantedPath(const std::string& wantedPath)
{
    auto currentPath = MacroParam::ConvertPathToOneUsedFormat(wantedPath);
    auto currentFolderIndex = FindPath(currentPath);
    boost::shared_ptr<NFmiMacroParamFolder> currentFolder = GetFolder(currentFolderIndex);
    if(currentFolder)
    {
        // Vasta kun on varmistunut että polku löytyy systeemistä, asetetaan luokan sisäinen tila vastaamaan sitä
        itsCurrentPath = currentPath;
        itsCurrentFolderIndex = currentFolderIndex;
        if(!currentFolder->Initialized())
            currentFolder->RefreshMacroParams();
        InsertAllSubdirectories(itsMacroParamFolders, itsCurrentPath, itsRootPath, 0);
    }
}

void NFmiMacroParamSystem::ClearMacros(void)
{
	itsCurrentPath = itsRootPath;
	itsMacroParamFolders.clear();
    itsCurrentFolderIndex = -1;
	itsFoundMacroParam.reset();
	itsMacroItemTree.clear();
}


std::string NFmiMacroParamSystem::RelativePath(void) const
{
	return MacroParam::GetRelativePath(itsRootPath, itsCurrentPath);
}

void NFmiMacroParamSystem::InitMacroParamTree(NFmiStopFunctor *theStopFunctor)
{
	for(size_t i=0 ; i < itsMacroParamFolders.size(); i++)
	{ // HUOM! aluksi pitää varmistaa, että kaikki hakemistot on alustettu kerran, muuten homma ei toimi!!
		NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
		if(!itsMacroParamFolders[i]->Initialized())
			itsMacroParamFolders[i]->RefreshMacroParams();
	}
}

void NFmiMacroParamSystem::MakeMacroParamItemTree(NFmiStopFunctor *theStopFunctor)
{
	if(itsMacroParamFolders.size() > 0)
	{ // HUOM! kaikki tasot on laitettu itsMacroParamFolders-vektoriin vain peräkkäin, 1. kohdassa on root taso
		// joten täytämme vain root tason, ja etsimme siellä oleviin hakemistoihin niiden omat macroparam-listat
		const std::vector<boost::shared_ptr<NFmiMacroParam> >& macroParams = itsMacroParamFolders[0]->MacroParams(); // 0 eli root-taso
		AddToMacroParamItemTree(itsMacroItemTree, macroParams, theStopFunctor);
	}
}

bool MacroParamLessThan(const NFmiMacroParamItem &item1, const NFmiMacroParamItem &item2)
{
    return boost::algorithm::lexicographical_compare(item1.itsMacroParam->Name(), item2.itsMacroParam->Name(), boost::algorithm::is_iless());
}

void NFmiMacroParamSystem::AddToMacroParamItemTree(std::vector<NFmiMacroParamItem> &theMacroItemList, const std::vector<boost::shared_ptr<NFmiMacroParam> >& theMacroParams, NFmiStopFunctor *theStopFunctor)
{
	int mpSize = static_cast<int>(theMacroParams.size());
	for(int j=0 ; j<mpSize; j++)
	{
		NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
		if(theMacroParams[j]->Name() != "<..>") // emo hakemistoa ei laiteta tähän listaan
		{
			NFmiMacroParamItem item;
			item.itsMacroParam = theMacroParams[j];
			if(theMacroParams[j]->IsMacroParamDirectory())
			{
				int index = SeekMacroFolder(theMacroParams[j]->MacroParamDirectoryPath());
				if(index != -1)
				{
					// HUOM! aluksi pitää varmistaa, että kaikki hakemistot on alustettu kerran, muuten homma ei toimi!!
					InsertAllSubdirectories(itsMacroParamFolders, theMacroParams[j]->MacroParamDirectoryPath(), itsRootPath, theStopFunctor);
					if(!itsMacroParamFolders[index]->Initialized())
						itsMacroParamFolders[index]->RefreshMacroParams();
					AddToMacroParamItemTree(item.itsDirectoryItems, itsMacroParamFolders[index]->MacroParams(), theStopFunctor);
				}
				else
				{
					// tähän ei saisi mennä, pitäisi heittää poikkeus!!!
				}
			}
			theMacroItemList.push_back(item);
		}
	}
	// sotrtataan macroParamit nimen perusteella
	std::sort(theMacroItemList.begin(), theMacroItemList.end(), MacroParamLessThan);
}

// etsii macroParamin total nimen avulla sen macroParamFolderin indeksin ja paluttaa sen.
// jos indeksi on -1, ei löytynyt. Ei aseta currentteja asetuksia löytyneeseen.
int NFmiMacroParamSystem::SeekMacroFolder(const std::string &theTotalFileName)
{
	// 1. irrota polku theTotalFileName:sta
	NFmiFileString fileStr(theTotalFileName);
	std::string pathStr(fileStr.Device());
	pathStr += fileStr.Path();
	pathStr = MacroParam::ConvertPathToOneUsedFormat(pathStr);
	// 2. etsi itsMacroParamFolders-listasta folderi joka on tämä polku
	return FindPath(pathStr);
}
