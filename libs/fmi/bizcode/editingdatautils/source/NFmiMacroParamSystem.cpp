#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParamFolder.h"
#include "NFmiMacroParamItem.h"
#include "NFmiMacroParam.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiMacroParamFunctions.h"
#include <list>

using namespace std;

struct PathFinderFunctor
{
	PathFinderFunctor(const std::string &thePath):itsPath(thePath){}

	bool operator()(boost::shared_ptr<NFmiMacroParamFolder> theFolder)
	{
		return theFolder->Path() == itsPath;
	}

	std::string itsPath;
};

// lisätään kaikki alihakemistot mukaan,
// jos niitä on ja niitä ei ole jo lisätty,
// mutta ei initialisoida niitä, eli ei ladata niiden alihakemistoja tai macroparameja
static void InsertAllSubdirectories(checkedVector<boost::shared_ptr<NFmiMacroParamFolder> > &theMacroParamFolders, const std::string &thePath, const std::string &theRootPath, NFmiStopFunctor *theStopFunctor)
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
		checkedVector<boost::shared_ptr<NFmiMacroParamFolder> >::iterator it = std::find_if(theMacroParamFolders.begin(), theMacroParamFolders.end(), PathFinderFunctor(totalPath));
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
,itsCurrentIndexPois(-1)
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

	// laita osoittamaan vielä oikeisiin current otuksiin
	int pathIndex = FindPath(itsCurrentPath);
	if(pathIndex >= 0)
	{
		if(itsFoundMacroParam.get())
			FindMacro((*itsFoundMacroParam).Name());
		if(itsCurrentIndexPois < 0 && itsMacroParamFolders.size() > 0)
			itsCurrentIndexPois = 0; // jos on polku ok ja löytyy macroParam-hakemisto, laitetaan osoittamaan roottiin, jos muuten indksi osoittaa -1:een
	}
}

// Rakennetaan kaikki macroParamit ja polut uudestaan
void NFmiMacroParamSystem::Rebuild(NFmiStopFunctor *theStopFunctor)
{
	ClearMacros();
	NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);

	boost::shared_ptr<NFmiMacroParamFolder> tmp(new NFmiMacroParamFolder(itsCurrentPath, itsRootPath));
	tmp->RefreshMacroParams();
	itsMacroParamFolders.push_back(tmp);
	itsCurrentIndexPois = 0;
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
int NFmiMacroParamSystem::FindPath(const std::string &thePathName)
{
	checkedVector<boost::shared_ptr<NFmiMacroParamFolder> >::iterator it = std::find_if(itsMacroParamFolders.begin(), itsMacroParamFolders.end(), PathFinderFunctor(thePathName));
	if(it == itsMacroParamFolders.end())
		return -1;
	else
		return static_cast<int>(it - itsMacroParamFolders.begin());
}

// tällä voi säädellä Find:illa etsityn folder-otuksen arvoja ja asetuksia
boost::shared_ptr<NFmiMacroParamFolder> NFmiMacroParamSystem::GetCurrent(void)
{
	if(itsCurrentIndexPois >= 0 && itsCurrentIndexPois < itsMacroParamFolders.size())
		return itsMacroParamFolders[itsCurrentIndexPois];
	else
		return boost::shared_ptr<NFmiMacroParamFolder>();
}

// etsii currentista folderista haluttua makroa
// jos löytää, asettaa sen itsFoundMacroParam:iksi
bool NFmiMacroParamSystem::FindMacro(const std::string &theMacroName)
{
	boost::shared_ptr<NFmiMacroParamFolder> currentFolder = GetCurrent();
	if(currentFolder && currentFolder->Find(theMacroName))
	{
		itsFoundMacroParam = currentFolder->Current();
		return true;
	}
	return false;
}

bool NFmiMacroParamSystem::FindTotal(const std::string &theTotalFileName)
{
	itsFoundMacroParam.reset();
	// 1. irrota polku theTotalFileName:sta
	NFmiFileString fileStr(theTotalFileName);
	std::string pathStr(fileStr.Device());
	pathStr += fileStr.Path();
	pathStr = MacroParam::ConvertPathToOneUsedFormat(pathStr);
	// 2. etsi itsMacroParamFolders-listasta folderi joka on tämä polku
	int ind = FindPath(pathStr);
	if(ind != -1)
	{
	// 2.2 jos löytyy
		// 2.2.1 initilaisoi folderi tarvittaessa
		if(!itsMacroParamFolders[ind]->Initialized())
			itsMacroParamFolders[ind]->RefreshMacroParams();

		// 2.2.2 irrota theTotalFileName:sta ns. lyhenne eli ilman polkua ja ilman extensiota
		std::string fileNameStr(fileStr.Header());
		// 2.2.3 etsi lyhennettä folderista
		if(itsMacroParamFolders[ind]->Find(fileNameStr))
		{
			// 2.2.4 jos löytyy aseta itsFoundMacroParam ja palauta true,
			itsFoundMacroParam = itsMacroParamFolders[ind]->Current();
			return true;
		}
	}
	// 2.1 jos ei löydy, palauta false
	return false;
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

	if(usedDirectoryName == "..")
	{
		MacroParam::RemoveLastPartOfDirectory(itsCurrentPath);
	}
	else
	{
		itsCurrentPath += usedDirectoryName;
	}

	itsCurrentPath = MacroParam::ConvertPathToOneUsedFormat(itsCurrentPath);
	itsCurrentIndexPois = FindPath(itsCurrentPath);
	boost::shared_ptr<NFmiMacroParamFolder> currentFolder = GetCurrent();
	if(currentFolder)
	{
		if(!currentFolder->Initialized())
			currentFolder->RefreshMacroParams();
		InsertAllSubdirectories(itsMacroParamFolders, itsCurrentPath, itsRootPath, 0);
	}
}

void NFmiMacroParamSystem::ClearMacros(void)
{
	itsCurrentPath = itsRootPath;
	itsMacroParamFolders.clear();
	itsCurrentIndexPois = -1;
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
		const checkedVector<boost::shared_ptr<NFmiMacroParam> >& macroParams = itsMacroParamFolders[0]->MacroParams(); // 0 eli root-taso
		AddToMacroParamItemTree(itsMacroItemTree, macroParams, theStopFunctor);
	}
}

bool MacroParamLessThan(const NFmiMacroParamItem &item1, const NFmiMacroParamItem &item2)
{
	std::string name1 = item1.itsMacroParam->Name();
	std::string name2 = item2.itsMacroParam->Name();
	NFmiStringTools::LowerCase(name1);
	NFmiStringTools::LowerCase(name2);
	return name1 < name2;
}

void NFmiMacroParamSystem::AddToMacroParamItemTree(std::vector<NFmiMacroParamItem> &theMacroItemList, const checkedVector<boost::shared_ptr<NFmiMacroParam> >& theMacroParams, NFmiStopFunctor *theStopFunctor)
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
