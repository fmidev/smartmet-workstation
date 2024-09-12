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

// lis�t��n kaikki alihakemistot mukaan,
// jos niit� on ja niit� ei ole jo lis�tty,
// mutta ei initialisoida niit�, eli ei ladata niiden alihakemistoja tai macroparameja
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
		{ // jos hakista ei ollut aiemmin, voidaan se lis�t�
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

// t�m� on initialisointi metodi ja my�s root hakis asetetaan
void NFmiMacroParamSystem::RootPath(const std::string &newValue)
{
	ClearMacros();

	itsRootPath = itsCurrentPath = MacroParam::ConvertPathToOneUsedFormat(newValue);
}

// Tarkoitus on siirt�� this-oliosta theOther -olioon kaiki makroParam data.
// pit�� tehd� viel� niin ett� lopuksi this-olion current osoittaa oikeaan paikkaan
void NFmiMacroParamSystem::SwapMacroData(NFmiMacroParamSystem &theOther)
{
	fUpdateMacroParamListView = true;
	this->itsMacroParamFolders.swap(theOther.itsMacroParamFolders);
	this->itsMacroItemTree.swap(theOther.itsMacroItemTree);

	UpdateCurrentPath(itsCurrentPath);
}

// Kun this olio (johon ladattu uusimmat macroParam tiedosto/hakemisto rakenteet) 
// halutaan ottaa k�ytt��n dokumentissa, p�ivitet��n se t�ll� metodilla tarvittavilla 
// working-data asetuksilla.
void NFmiMacroParamSystem::UpdateToWorkingData(NFmiMacroParamSystem& theCurrentWorkingData)
{
	fUpdateMacroParamListView = true;
	this->itsRootPath = theCurrentWorkingData.itsRootPath;
	// itsMacroParamFolders ja itsMacroItemTree pidet��n this oliosta
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
	// Laita viel� osoittamaan oikeisiin current otuksiin
	int pathIndex = FindPath(itsCurrentPath);
	if(pathIndex >= 0)
	{
		if(itsFoundMacroParam.get())
			FindMacroFromCurrentFolder((*itsFoundMacroParam).Name());
		if(itsCurrentFolderIndex < 0 && itsMacroParamFolders.size() > 0)
			itsCurrentFolderIndex = 0; // jos on polku ok ja l�ytyy macroParam-hakemisto, laitetaan osoittamaan roottiin, jos muuten indksi osoittaa -1:een
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

// t�ss� this-olio tyhjennet��n ja alustetaan vain niin ett� root-polku otetaan
// theOther-otuksesta.
void NFmiMacroParamSystem::Init(NFmiMacroParamSystem &theOther)
{
	RootPath(theOther.RootPath());
}

// Kun worker-thread on p�ivitt�nyt macroParamit, on syyt� tarkistaa, ett�
// tarvitaanko p��ohjelman oliota oikeasti p�ivitt��, eli ovatko makro rakenteet jo samoja.
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

// etsii folder-otusta polku nimell�
// palauta sen indeksi, -1 jos ei l�ydy
// jos l�ytyy, ei aseta currentiksi folderiksi!
int NFmiMacroParamSystem::FindPath(const std::string &thePathName) const
{
	auto it = std::find_if(itsMacroParamFolders.begin(), itsMacroParamFolders.end(), PathFinderFunctor(thePathName));
	if(it == itsMacroParamFolders.end())
		return -1;
	else
		return static_cast<int>(it - itsMacroParamFolders.begin());
}

// t�ll� voi s��dell� Find:illa etsityn folder-otuksen arvoja ja asetuksia
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
// jos l�yt��, asettaa sen itsFoundMacroParam:iksi
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

// Etsii annetulla suhteellisella polulla olevaa macroParamia (k�ytet��n speed-search kontrollista).
// Jos l�yt��, asetetaan se itsFoundMacroParam:iksi.
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
    // 2. etsi itsMacroParamFolders-listasta folderi joka on t�m� polku
    int folderIndex = FindPath(pathStr);
    if(folderIndex != -1)
    {
        // 2.2 jos l�ytyy hakemisto
        // 2.2.1 initilaisoi folderi tarvittaessa
        auto macroParamFolder = GetFolder(folderIndex);
        if(macroParamFolder)
        {
            if(!macroParamFolder->Initialized())
                macroParamFolder->RefreshMacroParams();
        }

        // 2.2.2 irrota theTotalFileName:sta ns. lyhenne eli ilman polkua ja ilman extensiota
        std::string fileNameStr(fileStr.Header());
        // 2.2.3 etsi lyhennett� folderista
        if(macroParamFolder->Find(fileNameStr))
        {
            // 2.2.4 jos l�ytyy alusta CurrentMacroPointerData olio ja palauta se
            return CurrentMacroPointerData{ pathStr , folderIndex, macroParamFolder->Current() };
        }
    }
    // 2.1 jos ei l�ydy, palauta false
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
		// Pit�� ladata erikseen originaali drawParam asetukset omaan olioon ja sen avulla initialisoida k�yt�ss� olevan asetukset
		NFmiDrawParam origDrawParam;
		if(origDrawParam.Init(theTotalFileName))
		{
			foundMacroParam->DrawParam()->Init(&origDrawParam, true);
			return true;
		}
	}
	return false;
}

// T�m� on hakemistoon siirtymis metodi, t�t� k�ytet��n jos file-browserilla talletetaan
// uusi macroParam mahdollisesti t�ysin uuteen hakemistoon (file-browserilla tehty hakemisto).
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

// t�m� on alihakemistoon siirtymis metodi
void NFmiMacroParamSystem::CurrentPath(const std::string &newValue)
{
	// aivan aluksi lis�t��n varmuuden vuoksi kaikki hakemistot, mitk� ovat mahd. lis�tty currenttiin hakemistoon
	InsertAllSubdirectories(itsMacroParamFolders, itsCurrentPath, itsRootPath, 0);

	// t�m� on alihakemistoon siirtymis metodi
	// newValue on muotoa <hakis>, josta kulma sulut pit�� ottaa pois ja lis�t� nykyiseen polkuun
	// piatsi jos tulee <..>, jolloin pit�� menn� yksi hakemisto taso yl�s
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
        // Vasta kun on varmistunut ett� polku l�ytyy systeemist�, asetetaan luokan sis�inen tila vastaamaan sit�
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
	{ // HUOM! aluksi pit�� varmistaa, ett� kaikki hakemistot on alustettu kerran, muuten homma ei toimi!!
		NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
		if(!itsMacroParamFolders[i]->Initialized())
			itsMacroParamFolders[i]->RefreshMacroParams();
	}
}

void NFmiMacroParamSystem::MakeMacroParamItemTree(NFmiStopFunctor *theStopFunctor)
{
	if(itsMacroParamFolders.size() > 0)
	{ // HUOM! kaikki tasot on laitettu itsMacroParamFolders-vektoriin vain per�kk�in, 1. kohdassa on root taso
		// joten t�yt�mme vain root tason, ja etsimme siell� oleviin hakemistoihin niiden omat macroparam-listat
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
		if(theMacroParams[j]->Name() != "<..>") // emo hakemistoa ei laiteta t�h�n listaan
		{
			NFmiMacroParamItem item;
			item.itsMacroParam = theMacroParams[j];
			if(theMacroParams[j]->IsMacroParamDirectory())
			{
				int index = SeekMacroFolder(theMacroParams[j]->MacroParamDirectoryPath());
				if(index != -1)
				{
					// HUOM! aluksi pit�� varmistaa, ett� kaikki hakemistot on alustettu kerran, muuten homma ei toimi!!
					InsertAllSubdirectories(itsMacroParamFolders, theMacroParams[j]->MacroParamDirectoryPath(), itsRootPath, theStopFunctor);
					if(!itsMacroParamFolders[index]->Initialized())
						itsMacroParamFolders[index]->RefreshMacroParams();
					AddToMacroParamItemTree(item.itsDirectoryItems, itsMacroParamFolders[index]->MacroParams(), theStopFunctor);
				}
				else
				{
					// t�h�n ei saisi menn�, pit�isi heitt�� poikkeus!!!
				}
			}
			theMacroItemList.push_back(item);
		}
	}
	// sotrtataan macroParamit nimen perusteella
	std::sort(theMacroItemList.begin(), theMacroItemList.end(), MacroParamLessThan);
}

// etsii macroParamin total nimen avulla sen macroParamFolderin indeksin ja paluttaa sen.
// jos indeksi on -1, ei l�ytynyt. Ei aseta currentteja asetuksia l�ytyneeseen.
int NFmiMacroParamSystem::SeekMacroFolder(const std::string &theTotalFileName)
{
	// 1. irrota polku theTotalFileName:sta
	NFmiFileString fileStr(theTotalFileName);
	std::string pathStr(fileStr.Device());
	pathStr += fileStr.Path();
	pathStr = MacroParam::ConvertPathToOneUsedFormat(pathStr);
	// 2. etsi itsMacroParamFolders-listasta folderi joka on t�m� polku
	return FindPath(pathStr);
}
