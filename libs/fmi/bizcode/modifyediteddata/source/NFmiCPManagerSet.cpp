
#include "NFmiCPManagerSet.h"
#include "NFmiSettings.h"
#include "NFmiTimeDescriptor.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "catlog/catlog.h"

#include <fstream>


NFmiCPManagerSet::NFmiCPManagerSet(void)
:itsOldSchoolCPManager(new NFmiEditorControlPointManager())
,itsOldSchoolCPManagerOrigPoints()
,itsCPManagers()
,itsCPManagersOrigPoints()
,itsIndex(0)
,itsCPManagerDirectory()
,itsControlDirectory()
,itsSpecialStorageDirectory()
,itsBaseNameSpace()
{
}

void NFmiCPManagerSet::InitFromSettings(const std::string &theBaseNameSpace, const std::string &theControlDirectory, const std::string &theSpecialStorageDirectory)
{
	itsBaseNameSpace = theBaseNameSpace;
	itsControlDirectory = theControlDirectory;
    itsSpecialStorageDirectory = theSpecialStorageDirectory;

	itsCPManagerDirectory = NFmiSettings::Optional<std::string>(std::string(itsBaseNameSpace + "::CPManagerDirectory"), "");
	if(itsCPManagerDirectory.empty() == false)
	{ // jos oli annettu CPManagerDirectory:lle arvo, rakennetaan se t�ss� loppuun (mm. '/' -merkit '\'-merkeiksi, suhteellinen polku absoluuttiseksi)
		NFmiFileString fileStr(itsCPManagerDirectory);
		if(fileStr.IsAbsolutePath() == false)
			itsCPManagerDirectory = itsControlDirectory + itsCPManagerDirectory;
		NFmiFileString fileStr2(itsCPManagerDirectory);
		fileStr2.NormalizeDelimiter();
		itsCPManagerDirectory = fileStr2;
	}

	itsIndex = NFmiSettings::Optional<size_t>(std::string(itsBaseNameSpace + "::CPManagerIndex"), 0);
	if(UseOldSchoolStyle())
		DoOldSchoolStyleInit();
	else
		DoMultiCPManagerInit();
}

void NFmiCPManagerSet::StoreToSettings(void) const
{
	if(itsBaseNameSpace.empty() == false)
	{
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::CPManagerIndex"), NFmiStringTools::Convert(itsIndex), true);
	}
	else
		throw std::runtime_error("Error in NFmiCPManagerSet::StoreToSettings, unable to store setting.");
}

void NFmiCPManagerSet::MakeSureCpManagerDirectoryExist()
{
    if(!NFmiFileSystem::DirectoryExists(itsCPManagerDirectory))
    {
        if(NFmiFileSystem::CreateDirectory(itsCPManagerDirectory))
        {
            CatLog::logMessage(std::string("Had to create directory for CPManager in '") + itsCPManagerDirectory + "'", CatLog::Severity::Warning, CatLog::Category::Configuration);
        }
        else
        {
            CatLog::logMessage(std::string("Couldn't create directory for CPManager in '") + itsCPManagerDirectory + "'", CatLog::Severity::Error, CatLog::Category::Configuration);
        }
    }
}

void NFmiCPManagerSet::ReadCPManagers(std::list<std::string> &fileList)
{
    NFmiTimeDescriptor times; // tehd��n joku feikki aikadescriptori
    NFmiParamBag params; // tehd��n joku feikki parambagi

    for(std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        std::string filePath = itsCPManagerDirectory;
        filePath += kFmiDirectorySeparator;
        filePath += *it;
        try
        {
            boost::shared_ptr<NFmiEditorControlPointManager> cpManager(new NFmiEditorControlPointManager());
            if(cpManager->Init(times, params, filePath, false, false))
            {
                itsCPManagers.push_back(cpManager);
            }
            else
            {
                CatLog::logMessage(std::string("Unable to read CPManeger file: ") + filePath, CatLog::Severity::Error, CatLog::Category::Configuration);
            }
        }
        catch(...)
        {
            CatLog::logMessage(std::string("Unknown exception while reading CPManeger file: ") + filePath, CatLog::Severity::Error, CatLog::Category::Configuration);
        }
    }
}

void NFmiCPManagerSet::MakeBackupStuffForCpManagers()
{
    if(itsCPManagers.size())
    {
        if(itsIndex >= itsCPManagers.size())
            itsIndex = 0;

        boost::shared_ptr<NFmiEditorControlPointManager> backupCPManager = LoadCPDataBackupIfNotTooOld(15); // katsotaan viel�, pit��k� CPbackup-data ladata aktiiviseen CPManageriin
        if(backupCPManager)
        {
            int index = Find(backupCPManager->Name());
            if(index >= 0)
            {
                backupCPManager->FilePath(itsCPManagers[index]->FilePath()); // polku pit�� viel� asettaa ennen kuin korvataan backup CPManagerilla
                itsCPManagers[index] = backupCPManager;
            }
            else if(backupCPManager->Name().empty() == false)
                itsCPManagers.push_back(backupCPManager);
        }
    }
    else
        throw std::runtime_error("Error in NFmiCPManagerSet::DoMultiCPManagerInit, no readable CP-manager files found.");
}

// Jos ei l�ytynyt yht��n tiedostoa, mutta CPManagerSet on kuitenkin k�yt�ss�, pit��
//luoda yksi default otus k�ytt��n. Teen t�m�n, jotta saadaan ownCloud systeemi toimimaan.
// Lokitetaan varoitus, jotta k�ytt�j�lle selvi�� mahdollinen virhe CPManager konfiguraatio virhe
void NFmiCPManagerSet::MakeDefaultCPManagerSet()
{
    checkedVector<NFmiPoint> newCPs;
    boost::shared_ptr<NFmiEditorControlPointManager> cpManager(new NFmiEditorControlPointManager());
    if(cpManager->Init(newCPs, false))
    {
        std::string fileName = itsCPManagerDirectory + "\\default.dat";
        cpManager->FilePath(fileName);
        cpManager->StoreCPs();
        CatLog::logMessage(std::string("Forced to create and store default CPManager in file: ") + fileName, CatLog::Severity::Warning, CatLog::Category::Configuration);
        itsCPManagers.push_back(cpManager);
    }
}

void NFmiCPManagerSet::DoMultiCPManagerInit()
{
    MakeSureCpManagerDirectoryExist();

    std::list<std::string> fileList = NFmiFileSystem::DirectoryFiles(itsCPManagerDirectory);
    if(fileList.size())
    {
        ReadCPManagers(fileList);
    }
    else
    {
        MakeDefaultCPManagerSet();
    }

    MakeBackupStuffForCpManagers();

    // Tehd��n kopiot originaal CP-pisteist�
    itsCPManagersOrigPoints.clear();
    for(int i = 0; i < itsCPManagers.size(); i++)
        itsCPManagersOrigPoints.push_back(itsCPManagers[i]->CPLocationVector());
}

int NFmiCPManagerSet::Find(const std::string &theName) const
{
	if(theName.empty() == false)
	{
		for(size_t i = 0; i < itsCPManagers.size(); i++)
		{
			if(itsCPManagers[i]->Name() == theName)
				return static_cast<int>(i);
		}
	}
	return -1;
}

void NFmiCPManagerSet::DoOldSchoolStyleInit(void)
{
	std::string fileName(itsSpecialStorageDirectory);
	fileName += "controlpoint.dat";

	boost::shared_ptr<NFmiEditorControlPointManager> backupCPManager = LoadCPDataBackupIfNotTooOld(60); // katsotaan ensin, pit��k� CPbackup-data ladata

	if(backupCPManager)
	{
		itsOldSchoolCPManager = backupCPManager;
		itsOldSchoolCPManager->FilePath(fileName); // t�m� asetetaan joka tapauksessa, vaikka data luettaisiin backup-tiedostosta (joka on siis eri kuin t�m� tiedosto)
	}
	else 
	{	// jos ei ladattua backupdataa, luetaan CP-pisteet tallesta.
		NFmiTimeDescriptor times; // tehd��n joku feikki aikadescriptori
		itsOldSchoolCPManager->Init(times, NFmiParamBag(), fileName, false, false);
	}

    itsOldSchoolCPManagerOrigPoints = itsOldSchoolCPManager->CPLocationVector();
}

// Antaa joko old school CP-managerin tai indeksi mukaisen CP-managerin listasta.
// K�ytt�j� voi vaatia old school CP-manageria laittamalla getOldSchoolCPManager -parametrin arvoksi true.
boost::shared_ptr<NFmiEditorControlPointManager> NFmiCPManagerSet::CPManager(bool getOldSchoolCPManager)
{
	if(getOldSchoolCPManager || UseOldSchoolStyle())
		return itsOldSchoolCPManager;
	else
	{
		if(itsIndex < itsCPManagers.size())
			return itsCPManagers[itsIndex];
		else
			throw std::runtime_error("Error in NFmiCPManagerSet::CPManager, no CPManagers available.");
	}
}

void NFmiCPManagerSet::SetCPManager(boost::shared_ptr<NFmiEditorControlPointManager> &newCPManager)
{
	if(UseOldSchoolStyle())
		itsOldSchoolCPManager = newCPManager;
	else
	{
		if(itsIndex < itsCPManagers.size())
			itsCPManagers[itsIndex] = newCPManager;
	}
}

static std::pair<bool, size_t> FindCpManagerByName(const std::vector<boost::shared_ptr<NFmiEditorControlPointManager>> &cPManagers, const std::string& managerName)
{
    for(size_t index = 0; index < cPManagers.size(); index++)
    {
        if(cPManagers[index]->Name() == managerName)
            return std::make_pair(true, index);
    }
    return std::make_pair(false, -1);
}

// Jos ollaan mult-set -moodissa tehd��n seuraavaa (old-school-moodissa ei mit��n):
// Jos l�ytyy "viewmacro" -niminen CPManager, laitetaan t�m� sen arvoksi.
// Jos ei l�ydy, lis�t��n t�m� sen nimisen� siihen.
// Laitetaan itsIndex osoittamaan t�h�n CPManager:iin.
void NFmiCPManagerSet::UpdateViewMacroCPManager(const checkedVector<NFmiPoint>& newCpLatlonPoints)
{
    static const std::string viewMacroName = "viewmacro";
    if(UseOldSchoolStyle() == false)
    {
        // 1. Katsotaan l�ytyyk� viewmacro CP-manageria ensin
        auto foundInfo = ::FindCpManagerByName(itsCPManagers, viewMacroName);
        if(foundInfo.first)
        {
            itsIndex = foundInfo.second;
            itsCPManagers[itsIndex]->Init(newCpLatlonPoints, true);
            return;
        }
        else
        {
            // pit�� luoda uusi viewmacro cp-manager
            boost::shared_ptr<NFmiEditorControlPointManager> newViewmacroCPManager(new NFmiEditorControlPointManager());
            std::string viewMacroFilePath = itsCPManagerDirectory + "\\" + viewMacroName + ".dat";
            newViewmacroCPManager->FilePath(viewMacroFilePath);
            // laitetaan t�m� heti talteen CPManager hakemistoon, ett� k�ytt�j� voi halutessaan tehd� kopion t�st� CPManagerista eri tiedostoon
            newViewmacroCPManager->StoreCPs(); 
            itsCPManagers.push_back(newViewmacroCPManager);
            itsIndex = itsCPManagers.size() - 1;
            return;
        }
    }
}

size_t NFmiCPManagerSet::CPSetSize(void) const
{
	if(UseOldSchoolStyle())
		return 0;
	else
		return itsCPManagers.size();
}

boost::shared_ptr<NFmiEditorControlPointManager> NFmiCPManagerSet::CPManagerFromSet(size_t theIndex)
{
	if(theIndex < itsCPManagers.size())
		return itsCPManagers[theIndex];
	else
		return boost::shared_ptr<NFmiEditorControlPointManager>();
}

void NFmiCPManagerSet::SetNewCPManagerSettings(size_t oldIndex, size_t newIndex)
{
	if(oldIndex != newIndex)
	{
		if(oldIndex < itsCPManagers.size() && newIndex < itsCPManagers.size())
		{
			itsCPManagers[newIndex]->Init(*itsCPManagers[oldIndex]);
		}
	}
}

void NFmiCPManagerSet::NextCPManager(void)
{
	if(UseOldSchoolStyle() == false)
	{
		size_t oldIndex = itsIndex;
		itsIndex++;
		if(itsIndex >= itsCPManagers.size())
			itsIndex = 0;
		SetNewCPManagerSettings(oldIndex, itsIndex);
	}
}

void NFmiCPManagerSet::PreviousCPManager(void)
{
	if(UseOldSchoolStyle() == false)
	{
		size_t oldIndex = itsIndex;
		if(itsIndex == 0)
			itsIndex = itsCPManagers.size() - 1;
		else
			itsIndex--;
		SetNewCPManagerSettings(oldIndex, itsIndex);
	}
}

void NFmiCPManagerSet::SetCPManager(size_t theIndex)
{
	if(UseOldSchoolStyle() == false)
	{
		size_t oldIndex = itsIndex;
		if(theIndex < itsCPManagers.size())
			itsIndex = theIndex;
		SetNewCPManagerSettings(oldIndex, itsIndex);
	}
}

// Jos on annettu konffeissa arvo itsCPManagerDirectory:lle, k�ytet��n kyseisest� kansiosta l�ytyvi� CPManagereita.
// Jos arvoa ei ole annettu, k�ytet��n ainoastaan yht� (vanhaan tyyliin) CPManageria (ja se my�s luetaan vanhasta paikasta).
bool NFmiCPManagerSet::UseOldSchoolStyle(void) const
{
	return itsCPManagerDirectory.empty();

}

bool NFmiCPManagerSet::HasCPManager(void) const
{
	if(UseOldSchoolStyle())
		return itsOldSchoolCPManager != 0;
	else
		return itsIndex < itsCPManagers.size();
}

std::string NFmiCPManagerSet::MakeCpDataFilePath() const
{
    std::string fileName(itsSpecialStorageDirectory);
    fileName += "cp_data.bak";
    return fileName;
}

// ControlPoint (CP) data pit�� lukea tietyiss� tilanteissa k�ynnistyksen
// yhteydess�. Sit� talletetaan automaattisen talletuksen yhteydess�
// jatkuvasti (esim. kerran minuutissa).
// Editoria k�ynnistett�ess� tarkistetaan pit��k� CPbackup-data ladata. Eli
// Jos editori k�ynnistet��n tietyn ajan sis�ll� edellisen ajon sulkemisesta
// (aika p��tell��n cp_data.bak tiedoston i�st�).
// T�m� tehd��n siksi, ett� jos editori on kaatunut tai se on suljettu, koska
// on lakannut toimimasta ja editori pit�� k�ynnist�� uudestaan, yritet��n
// dataa ladata.
// theMaxMinutes kertoo kuinka vanha tiedosto saa olla maksimissaan, ennen
// kuin backup dataa ladataan (liian vanha data ei kuulosta silt�, ett�
// editori olisi juuri kaatunut ja haluttaisiin muutos k�yr�t ladattua).
// Palauttaa ladatun CPManagerin, jos backup data ladattiin onnistuneesti ja tyhj�n shared_ptr:n, jos ei.
boost::shared_ptr<NFmiEditorControlPointManager> NFmiCPManagerSet::LoadCPDataBackupIfNotTooOld(int theMaxMinutes)
{
	std::string fileName = MakeCpDataFilePath();
	long fileAgeInSeconds = NFmiFileSystem::FileAge(fileName);
	if(fileAgeInSeconds != -1)
	{
		int fileAgeInMinutes = fileAgeInSeconds / 60;
		if(fileAgeInMinutes >= 0 && fileAgeInMinutes < theMaxMinutes)
		{
			std::ifstream in(fileName.c_str(), std::ios::binary);
			if(in)
			{
				boost::shared_ptr<NFmiEditorControlPointManager> cpManager(new NFmiEditorControlPointManager());
				cpManager->ReadBackup(in);
				return cpManager;
			}
		}
	}
	return boost::shared_ptr<NFmiEditorControlPointManager>();
}

// T�m� tallettaa currentin CPManagerin tiedot talteen. Sek� pelk�t CP-pisteet omaan tiedostoon ja 
// backup mieless� my�s toiseen tiedostoon kaiken mahdollisen tiedon kertoimineen kaikkineen (kaatumisten varalle).
bool NFmiCPManagerSet::StoreAllCurrentCPDataToFiles(void)
{
	bool status = true;
	// tehd��n t�ss� toistaiseksi talletukset ennen ohjelman sulkemista
	boost::shared_ptr<NFmiEditorControlPointManager> currentCPManager = CPManager();
	if(currentCPManager)
	{
		if(UseOldSchoolStyle())
        {
            if(currentCPManager->CPLocationVector() != itsOldSchoolCPManagerOrigPoints)
    			status = currentCPManager->StoreCPs();
        }
		else
		{
			for(size_t i = 0; i < itsCPManagers.size(); i++)
            {
                // itsCPManagers ja itsCPManagersOrigPoints vektoreiden koot saattavat olla eri suuruisia (FactorySettings ja viewmacro.dat -tapaus), 
                // joten pit�� tarkistaa l�ytyyk� itsCPManagersOrigPoints:ista tarpeeksi tavaraa.
                if(i < itsCPManagersOrigPoints.size())
                {
                    if(itsCPManagers[i]->CPLocationVector() != itsCPManagersOrigPoints[i])
                        status &= itsCPManagers[i]->StoreCPs();
                }
            }
		}

        std::string fileName2 = MakeCpDataFilePath();
        std::ofstream out2(fileName2.c_str(), std::ios::binary);
		if(out2)
		{
			currentCPManager->WriteBackup(out2); // T�ss� talletetaan virhetilanteiden varalle koko CP-roska muutosk�yrineen kaikkineen.
			out2.close();
		}
		else
			status = false;
	}
	else
		status = false;

	return status;
}
