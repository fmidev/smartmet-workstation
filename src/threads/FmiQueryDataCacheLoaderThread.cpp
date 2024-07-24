
#include "stdafx.h"
#include "SmartMetThreads_resource.h"
#include "FmiQueryDataCacheLoaderThread.h"
#include "afxmt.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "FmiCacheLoaderData.h"
#include "FmiDataLoadingThread2.h"
#include "FmiCombineDataThread.h"
#include "execute-command-in-separate-process.h"
#include "NFmiSettings.h"
#include "catlog/catlog.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiCaseStudySystem.h"
#include "NFmiLedLightStatus.h"
#include "NFmiValueString.h"
#include "NFmiMissingDataOnServerReporter.h"
#include "NFmiCachedDataFileInfo.h"
#include "NFmiOnceLoadedDataFiles.h"

#include <boost/filesystem.hpp> 
#include <mutex> 

#ifdef _MSC_VER
#pragma warning (disable : 4996)
#endif


namespace
{
	// HUOM! stopperi ja cancel ovat yhteisi‰ kaikille 3 eri threadille, runiing-semaphore piti laittaa threadi kohtaiseksi.
	NFmiStopFunctor gStopFunctor;
	BOOL gCopyFileExCancel; // t‰m‰n avulla CopyFileEx-funktio voidaan keskeytt‰‰.

	NFmiHelpDataInfoSystem gWorkerHelpDataSystem; // T‰m‰n olion avulla working thread osaa lukea/kopioida haluttuja datoja
    std::string gSmartMetBinDirectory; // SmartMetin bin‰‰ri-hakemistoa tarvitaan ainakin kun tehd‰‰n tiedostojen purkua erillisess‰ prosessissa (purku ohjelma sijaitsee siell‰ miss‰ smartmetin exe)
    std::string gSmartMetWorkingDirectory; // SmartMetin Working-hakemistoa tarvitaan kun rakennetaan polkua 7-zip ohjelmalle (purku ohjelma sijaitsee sen utils-hakemistossa)

	CSemaphore gSettingsChanged; // t‰m‰n avulla p‰ivitet‰‰n datan luku asetuksia thread safetysti
	NFmiHelpDataInfoSystem gMediatorHelpDataSystem; // t‰m‰n avulla s‰‰det‰‰n threadin asetuksia thread safetysti
	bool gSettingsHaveChanged;
    bool gLoadDataAtStartUp = true; // Jos t‰m‰ on false, se est‰‰ datan latauksen cacheen ja hakemistojen siivouksen
    bool gAutoLoadNewCacheDataMode = true; // Onko SmartMet ns. operatiivisessa moodissa, eli silloin se automaattisesti latailee uusia datoja cacheen ja silloin myˆs vanhoja pit‰‰ siivoilla pois.
                            // Joskus halutaan ett‰ uutta dataa ei lueta eik‰ vanhoja siivota, silloin t‰m‰ moodi pit‰‰ laittaa pois p‰‰lt‰.
                            // Myˆs jos ollaan ns. no-data/tiputus moodissa, t‰llˆin ei saa tuhota vanhoja tiedostoja.

    bool gUseCopyFileExFunction; // jos true, k‰ytet‰‰n tiedoston kopioinnissa CopyFileEx:‰‰, muuten k‰ytet‰‰n FileCopyMarko-funktiota
    size_t gUsedChunckSize; // Jos k‰ytet‰‰n FileCopyMarko:a, t‰m‰n kokoisia puskureita k‰ytet‰‰n kopioinnissa
    double gCacheCleaningIntervalInHours = 0.16; // T‰h‰n otetaan GeneralDocista siivousintervalli, oletus 0.16 [h] eli n. 10 minuutin v‰lein.

    NFmiMissingDataOnServerReporter gMissingDataOnServerReporter;
    NFmiOnceLoadedDataFiles gOnceLoadedDataFiles;

	enum CFmiCopyingStatus
	{
		kFmiNoCopyNeeded = 0,
		kFmiCopyWentOk = 1,
		kFmiCopyNotSuccessfull = 2,
		kFmiGoOnWithCopying = 3,
		kFmiUnpackIsDoneInSeparateProcess = 4
	};
}

static void MakeCacheDirectories(void)
{
	// Make sure that cache and tmp directories exist
	NFmiFileSystem::CreateDirectory(gWorkerHelpDataSystem.LocalDataLocalDirectory());
	NFmiFileSystem::CreateDirectory(gWorkerHelpDataSystem.LocalDataTmpDirectory());
	NFmiFileSystem::CreateDirectory(gWorkerHelpDataSystem.LocalDataPartialDirectory());
}

void CFmiQueryDataCacheLoaderThread::UpdateSettings(NFmiHelpDataInfoSystem &theHelpDataSystem)
{
	CSingleLock singleLock(&gSettingsChanged);
	if(singleLock.Lock(2000))
	{
		gMediatorHelpDataSystem.InitSettings(theHelpDataSystem, false);
		gSettingsHaveChanged = true;
	}
}

void CFmiQueryDataCacheLoaderThread::LoadDataAtStartUp(bool newState)
{
    gLoadDataAtStartUp = newState;
}

void CFmiQueryDataCacheLoaderThread::AutoLoadNewCacheDataMode(bool newState)
{
    gAutoLoadNewCacheDataMode = newState;
}

static void ApplyChangedSettings(void)
{
	if(gSettingsHaveChanged)
	{
		CSingleLock singleLock(&gSettingsChanged);
		if(singleLock.Lock(1000))
		{
			gWorkerHelpDataSystem.InitSettings(gMediatorHelpDataSystem, false);
			gSettingsHaveChanged = false;
			MakeCacheDirectories();
		}
	}
}

static void LogCopySuccess(const std::string &theTotalFileName, NFmiMilliSecondTimer &theTimer)
{
	std::string logStr("Copying QData file '");
	logStr += theTotalFileName; 
	logStr += "' to cache lasted: ";
	logStr += theTimer.EasyTimeDiffStr();
    CatLog::logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Data);
}

static void LogRenameFailure(const std::string &theTotalCacheTmpFileName, const std::string &theTotalCacheFileName)
{
	std::string logStr("Copying QData file was success but renaming tmp file failed\n");
	logStr += theTotalCacheTmpFileName;
	logStr += "\n -> \n";
	logStr += theTotalCacheFileName;
    CatLog::logMessage(logStr, CatLog::Severity::Warning, CatLog::Category::Data);
}

static void LogCopyFailure(const std::string &theTotalFileName, const std::string &theTotalCacheTmpFileName)
{
	std::string logStr("Copying QData file to cache tmp file failed\n");
	logStr += theTotalFileName;
	logStr += "\n -> \n";
	logStr += theTotalCacheTmpFileName;
    CatLog::logMessage(logStr, CatLog::Severity::Error, CatLog::Category::Data);
}

static void EnsureCacheDirectoryForPartialData(const std::string &theTotalCacheFileName, const NFmiHelpDataInfo &theDataInfo)
{
	if(theDataInfo.IsCombineData())
	{ // partial-datoille on varmistettava hakemisto, koska ne laitetaan cachessakin eri hakemistoihin
		NFmiFileString fileStr(theTotalCacheFileName);
		std::string partialDataDirectory = static_cast<char*>(fileStr.Device());
		partialDataDirectory += static_cast<char*>(fileStr.Path());
		NFmiFileSystem::CreateDirectory(partialDataDirectory);
	}
}

// Kun catlog ja sen speedlog systeemit otettiin k‰yttˆˆn, ovat smartmetin
// lokitiedostot lukossa ja niihin ei voi menn‰ ulkopuoliset loggerit lis‰‰m‰‰n mit‰‰n.
// Siksi luodaan oma unpack lokitiedosto. Jokaiselle p‰iv‰lle oma jotta niit‰ saadaan v‰h‰n niputettua.
std::string CFmiQueryDataCacheLoaderThread::MakeDailyUnpackLogFilePath()
{
    auto basicLogFile = CatLog::currentLogFilePath();
    NFmiFileString fileString = basicLogFile;
    std::string dailyLogFilePath = fileString.Device();
    dailyLogFilePath += fileString.Path();
    dailyLogFilePath += "unpacking_data_daily_log_";
    NFmiTime atime;
    dailyLogFilePath += atime.ToStr(kYYYYMMDD);
    dailyLogFilePath += ".txt";
    return dailyLogFilePath;
}

// Katso miten haluttu komentorivi pit‰‰ rakentaa smartmet_workstation\src\unpackdatafilesexe\UnpackSmartMetDataFilesMain.cpp
// tiedoston main -funktion alusta, kun virhetilanteessa laitetaan ohjeita cout:iin.
static std::string MakeUnpackCommand(NFmiCachedDataFileInfo &theCachedDataFileInfo)
{
    // HUOM! laitetaan kaikki k‰skyn osat lainausmerkkeihin, jos polut sattuisivat sis‰lt‰m‰‰n spaceja
    std::string commandStr("\"");
    // 1. ajettava exe
    commandStr += gSmartMetBinDirectory;
    commandStr += "\\UnpackSmartMetDataFilesExe\"";
    commandStr += " \"";
    // 2. pakattu tmp tiedosto
    commandStr += theCachedDataFileInfo.itsTotalCacheTmpPackedFileName;
    commandStr += "\" \"";
    // 3. purettu tiedosto siirrettyn‰ lokaali cacheen
    commandStr += theCachedDataFileInfo.itsTotalCacheFileName;
    // 4. pakattu tiedosto deletoidaan = 1
    commandStr += "\" 1 ";
    // 5. k‰ytetty 7-zip exe polku
    commandStr += CFmiProcessHelpers::Make7zipExePath(gSmartMetWorkingDirectory);
    // 6. k‰ytetty lokitiedosto
    commandStr += " \"";
    commandStr += CFmiQueryDataCacheLoaderThread::MakeDailyUnpackLogFilePath();
    commandStr += "\"";

    return commandStr;
}

static CFmiCopyingStatus DoFileUnpacking(NFmiCachedDataFileInfo &theCachedDataFileInfo)
{
    if(theCachedDataFileInfo.fFilePacked) // jos oli pakattu tiedosto
    {
        // 1.1. Puretaan 7z, zip tai bz2 pakattu tiedosto
        try
        {
            // Tehd‰‰n purku aina omassa erillisess‰ prosessissa, koska siihen pit‰‰ k‰ytt‰‰ erillista 7z.exe ohjelmaa ja purku voi kestaa minuutteja
            std::string unpackCommandStr = ::MakeUnpackCommand(theCachedDataFileInfo);
            bool status = CFmiProcessHelpers::ExecuteCommandInSeparateProcess(unpackCommandStr, true, false, SW_HIDE, false, NORMAL_PRIORITY_CLASS);
            if(status)
                return kFmiUnpackIsDoneInSeparateProcess;
            else
                return kFmiCopyNotSuccessfull;
        }
        catch(std::exception &e)
        {
            std::string errStr(std::string("Error in ") + __FUNCTION__ + " when trying to unpack file '" + theCachedDataFileInfo.itsTotalCacheFileName + "': ");
            errStr += e.what();
            CatLog::logMessage(errStr, CatLog::Severity::Error, CatLog::Category::Data);
            return kFmiCopyNotSuccessfull;
        }
        catch(...)
        {
            std::string errStr(std::string("Unknown error in ") + __FUNCTION__ + " when trying to unpack file '" + theCachedDataFileInfo.itsTotalCacheFileName + "'");
            CatLog::logMessage(errStr, CatLog::Severity::Error, CatLog::Category::Data);
            return kFmiCopyNotSuccessfull;
        }
    }
    return kFmiGoOnWithCopying;
}

static bool CopyFileCancelled(int *copyFileExCancel = 0)
{
    if(copyFileExCancel && *copyFileExCancel)
        return true;
    else
        return false;
}

// Windows:in CopyFileEx -funktio on ollut aiemmin k‰ytˆss‰ SmartMetissa, kun
// datatiedostoja kopioitiin serverilt‰ lokaali levylle. Funktiossa oli mm.
// keskeytys toiminto ja muita ominaisuuksia. 
// Ongelmia tuli kun viuhti-serverista tehtiin Linux pohjainen (oli windows).
// Alkoi tulla outoja lukituksia, ja tiedostoja voitiin kopioida yhdest‰ hakemistosta
// vain yksi kerrallaan. Aiemmin SmartMet pystyi kopioimaan vaikka kolmea tiedostoa rinnakkain.
// ------------------------------------------------------------------------------
// Tee kopiointi funktio seuraavilla ominaisuuksilla:
// 1. Tekee bin‰‰ri kopion (tiedostot avataan bin‰‰risin‰)
// 2. Mahdollisimman nopea (C:n FILE systeemi valittu web esimerkkien perusteella)
// 3. Kopio tehd‰‰n osissa (voidaan tehd‰ keskeytys tarkasteluja kopioinnin lomassa)
// 4. Jos kohde tiedosto on jo olemassa, ‰l‰ tee mit‰‰n
// 5. Argumentteina inputPath, outputPath, errorStr, keskeytysFlag-pointteri
// 6. Jos tulee keskeytys, deletoi kohde tiedosto pois
// 7. Varmista ett‰ uuden tiedoston aikaleima otetaan source:sta
// 8. Jos luku/kirjoitus vaiheessa kohdataan virhetilanne, jatketaan normaalista 
// ja virheellinen tulostiedosto j‰tet‰‰n k‰yttˆˆn. T‰m‰ siksi ett‰ muuten SmartMet 
// yritt‰‰ kopsata tiedostoa uudestaan ja uudestaan.
static bool FileCopyMarko(const std::string &inputPath, const std::string &outputPath, size_t chuckSizeInBytes, int *copyFileCancel = 0)
{
    FILE* source = ::fopen(inputPath.c_str(), "rb");
    if(source)
    {
        if(NFmiFileSystem::FileExists(outputPath) == false)
        {
            // HUOM! vaikka tiedosto olisi avattuna muualla, se ei est‰ sen uudelleen avaamista, siksi yritet‰‰n tutkia ensin onko tiedosto olemassa.
            FILE* dest = ::fopen(outputPath.c_str(), "wb"); // Oletus, jos toinen SmartMet on juuri kopioimassa samaa tiedostoa, t‰m‰ avaus ep‰onnistuu
            if(dest)
            {
                std::vector<char> buf(chuckSizeInBytes);
                size_t size = 0;

                int readErrorStatus = 0;
                int writeErrorStatus = 0;
                while ((size = ::fread(buf.data(), 1, buf.size(), source)) != 0) 
                {
                    if(::CopyFileCancelled(copyFileCancel))
                        break;
                    readErrorStatus = ferror(source);
                    ::fwrite(buf.data(), 1, size, dest);
                    writeErrorStatus = ferror(dest);

                    if(readErrorStatus || writeErrorStatus)
                        break; // ei jatketa virhetilanteissa
                }

                std::time_t modifiedTimeStamp = boost::filesystem::last_write_time(inputPath);  // otetaan source-tiedoston aikaleima varmuuden vuoksi jo t‰ss‰ (sit‰ ei voi deletoida nyt kun se on lukossa)
                ::fclose(source);
                ::fclose(dest);

                if(::CopyFileCancelled(copyFileCancel) || readErrorStatus || writeErrorStatus)
                {
                    NFmiFileSystem::RemoveFile(outputPath);
                    return false;
                }
                else
                {
                    // Laitetaan viel‰ luodun tiedoston modified aikeleima kuntoon
                    boost::filesystem::last_write_time(outputPath, modifiedTimeStamp); 

                    return true;
                }
            }
        }
    }

    // Jos vain source aukesi, pit‰‰ se sulkea viel‰
    if(source)
        ::fclose(source);

    return false;
}


// T‰m‰ on copy-rename k‰ytt‰en win32:en CopyFileEx funktiota. 
// T‰ss‰ on keskeytys ja progres-seuranta mahdollisuudet.
// Jos server-datatiedosto oli pakattu (theNewestFileInfo.second == true),
// pit‰‰ tiedosto kopioida normaalisti tmp-hakemistoon, mutta sen j‰lkeen riippuen 
// pakatun tiedoston koosta, tiedosto pit‰‰ joko purkaa t‰‰ll‰ tai jos se on iso
// tiedosto, pit‰‰ k‰ynnist‰‰ erillinen prosessi, joka purkaa ja siirt‰‰ tiedoston oikeaan 
// paikkaan. Oma purku-prosessi on pakko k‰ynnist‰‰, koska purkua ei voi keskeytt‰‰ ja se voi kest‰‰ 
// vaikka 3 minuuttia (esim. nykyinen arome mallipintadata). Jos k‰ytt‰j‰ sulkee SmartMetin, 
// ei voi odottaa 3 minuuttia, vaan purku keskeytett‰isiin v‰kivaltaisesti, eik‰ seuraamuksia tiedet‰ viel‰.
static CFmiCopyingStatus CopyFileEx_CopyRename(NFmiCachedDataFileInfo &theCachedDataFileInfo)
{
	NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
	NFmiMilliSecondTimer timer;
	timer.StartTimer();
    std::string totalFileStdString = theCachedDataFileInfo.itsTotalServerFileName;
    CString totFileStrU_ = CA2T(totalFileStdString.c_str());
    // totCacheTmpFileStr arvo riippuu onko kyse pakatusta tiedostosta vai ei
    CString totCacheTmpFileStrU_ = CA2T(theCachedDataFileInfo.itsTotalCacheTmpFileName.c_str());
    if(theCachedDataFileInfo.fFilePacked)
        totCacheTmpFileStrU_ = CA2T(theCachedDataFileInfo.itsTotalCacheTmpPackedFileName.c_str());
    std::string totalCacheTmpFileStdString = CT2A(totCacheTmpFileStrU_);

	// 1. kopioi data tmp-tiedostoon
    DWORD dwCopyFlags = COPY_FILE_FAIL_IF_EXISTS; // & COPY_FILE_NO_BUFFERING;
    bool copyStatus = gUseCopyFileExFunction ? (CopyFileEx(totFileStrU_, totCacheTmpFileStrU_, 0, 0, &gCopyFileExCancel, dwCopyFlags) == TRUE) : FileCopyMarko(totalFileStdString, totalCacheTmpFileStdString, gUsedChunckSize, &gCopyFileExCancel);
    if(copyStatus)
//	if(CopyFileEx(totFileStr, totCacheTmpFileStr, 0, 0, &gCopyFileExCancel, dwCopyFlags))
//    if(NFmiFileSystem::CopyFile((LPCSTR)totFileStr, (LPCSTR)totCacheTmpFileStr))
	{
		timer.StopTimer();
        ::LogCopySuccess(theCachedDataFileInfo.itsTotalServerFileName, timer); // pit‰‰ laittaa lokiin tiedoston kopion lokaalilevylle kesto ennen mahdollista bzip2 purkua

        CFmiCopyingStatus tmpFileStatus = ::DoFileUnpacking(theCachedDataFileInfo);
        if(tmpFileStatus == kFmiGoOnWithCopying)
        {
    	    // 2. jos onnistui renamea data-tiedosto lopulliseen muotoon ja hakemistoon.
            if(NFmiFileSystem::RenameFile(theCachedDataFileInfo.itsTotalCacheTmpFileName, theCachedDataFileInfo.itsTotalCacheFileName))
		    {
			    return kFmiCopyWentOk;
		    }
		    else
		    {
                ::LogRenameFailure(theCachedDataFileInfo.itsTotalCacheTmpFileName, theCachedDataFileInfo.itsTotalCacheFileName);
			    // jos rename ep‰onnistui, ei poisteta tmp-tiedostoa ainakaan viel‰, ett‰ j‰‰ jotain n‰yttˆ‰ ongelmista
		    }
        }
        else
            return tmpFileStatus;
	}
	else
	{
		if(gCopyFileExCancel == FALSE) // raportoidaan ep‰onnistumisesta vain jos threadia ei oltu canceloitu
		{
            ::LogCopyFailure(theCachedDataFileInfo.itsTotalServerFileName, theCachedDataFileInfo.itsTotalCacheTmpFileName);
			return kFmiCopyNotSuccessfull;
		}
		else
			return kFmiNoCopyNeeded; // jos threadi haluttiin lopettaa, palautetaan no-copy-needed status, eik‰ virhett‰
	}

	return kFmiCopyNotSuccessfull;
}

const double gKiloByte = 1024;
const double gMegaByte = gKiloByte * gKiloByte;
// t‰ss‰ tarkastetaan kuuluuko kyseinen data-tiedosto t‰lle threadille, eli tiedoston koon
// pit‰‰ menn‰ rajojen sis‰‰n.
static bool DoesThisThreadCopyFile(NFmiCachedDataFileInfo &theCachedDataFileInfo, CFmiCacheLoaderData *theCacheLoaderData)
{
    theCachedDataFileInfo.itsFileSizeInMB = NFmiFileSystem::FileSize(theCachedDataFileInfo.itsTotalServerFileName) / gMegaByte;
    if(theCachedDataFileInfo.itsFileSizeInMB <= 0)
		return false; // jostain syyst‰ tiedoston kokoa ei saatu
    else if(theCachedDataFileInfo.itsFileSizeInMB >= theCacheLoaderData->itsMinDataSizeInMB && theCachedDataFileInfo.itsFileSizeInMB < theCacheLoaderData->itsMaxDataSizeInMB)
		return true;
	else
		return false;
}

// Tarkistetaan tmp-tiedoston tila. Jos sit‰ ei ole, voidaan kopiointiproseduuria jatkaa.
// Jos se on olemassa, mutta sen voi poistaa (j‰‰nyt virhetilanteessa ja siin‰ ei ole nyt mik‰‰n kiinni?), voidaan jatkaa.
// Jos sit‰ ei voi poistaa, ei jatketa eteenp‰in, koska joku muu threadi tai prosessi on luultavasti tekem‰ss‰ sille jotain.
static CFmiCopyingStatus CheckTmpFileStatus(const std::string &theTmpFileName)
{
	if(NFmiFileSystem::FileExists(theTmpFileName))
	{
		// kokeillaan, voiko tmp-tiedoston deletoida, jos voi (eli kukaan ei ole sin‰ kiinni), se on j‰‰nyt jostain virhetilanteesta ja se voidaan deletoida ja aloittaa kopio uudestaan
		if(NFmiFileSystem::RemoveFile(theTmpFileName) == false)
			return kFmiNoCopyNeeded;
	}
    return kFmiGoOnWithCopying;
}

static std::string MakeFileSizeString(double fileSizeInMB)
{
    if(fileSizeInMB <= 0.1)
    {
        std::string str = NFmiValueString::GetStringWithMaxDecimalsSmartWay(fileSizeInMB * gKiloByte, 1);
        str += " kB";
        return str;
    }

    if(fileSizeInMB >= 900)
    {
        std::string str = NFmiValueString::GetStringWithMaxDecimalsSmartWay(fileSizeInMB / gKiloByte, 2);
        str += " GB";
        return str;
    }

    std::string str = NFmiValueString::GetStringWithMaxDecimalsSmartWay(fileSizeInMB, 1);
    str += " MB";
    return str;
}

static std::string MakeLedChannelStartLoadingString(const NFmiCachedDataFileInfo& cachedDataFileInfo)
{
    std::string reportStr = "Loading file from server (";
    reportStr += ::MakeFileSizeString(cachedDataFileInfo.itsFileSizeInMB);
    reportStr += "):\n";
    NFmiFileString fileStr(cachedDataFileInfo.itsTotalCacheFileName);
    reportStr += fileStr.FileName();
    return reportStr;
}

static CFmiCopyingStatus CopyFileToLocalCache(NFmiCachedDataFileInfo& theCachedDataFileInfo, CFmiCacheLoaderData* theCacheLoaderData, const NFmiHelpDataInfo& theDataInfo)
{
    if(NFmiFileSystem::FileExists(theCachedDataFileInfo.itsTotalCacheFileName))
    {
        return kFmiNoCopyNeeded;
    }

    if(gOnceLoadedDataFiles.checkIfFileHasBeenLoadedEarlier(theCachedDataFileInfo))
    {
        return kFmiNoCopyNeeded;
    }

    CFmiCopyingStatus tmpFileStatus = ::CheckTmpFileStatus(theCachedDataFileInfo.itsTotalCacheTmpFileName);
    CFmiCopyingStatus tmpPackedFileStatus = ::CheckTmpFileStatus(theCachedDataFileInfo.itsTotalCacheTmpPackedFileName);

    if(tmpFileStatus == kFmiGoOnWithCopying && tmpPackedFileStatus == kFmiGoOnWithCopying)
    {
        NFmiLedLightStatusBlockReporter blockReporter(NFmiLedChannel::QueryData, theCacheLoaderData->itsThreadName, ::MakeLedChannelStartLoadingString(theCachedDataFileInfo));
        ::EnsureCacheDirectoryForPartialData(theCachedDataFileInfo.itsTotalCacheFileName, theDataInfo);
        return ::CopyFileEx_CopyRename(theCachedDataFileInfo);
    }
    else
    {
        if(CatLog::doTraceLevelLogging())
        {
            std::string traceLoggingStr = theCacheLoaderData->itsThreadName;
            traceLoggingStr += ": tmp file status was no go with: ";
            traceLoggingStr += theCachedDataFileInfo.itsTotalServerFileName;
            CatLog::logMessage(traceLoggingStr, CatLog::Severity::Trace, CatLog::Category::Data, true);
        }
    }

    return kFmiNoCopyNeeded;
}

static NFmiFileString MakeFileStringWithoutCompressionFileExtension(const NFmiCachedDataFileInfo &theCachedDataFileInfo)
{
    NFmiFileString fileStr(theCachedDataFileInfo.itsTotalServerFileName);
    if(theCachedDataFileInfo.fFilePacked)
        fileStr = fileStr.Device() + fileStr.Path() + fileStr.Header(); // pakatun tiedoston tapauksessa j‰tetaan pois tiedoston viimeinen extensio
    return fileStr;
}

static std::string MakeFinalTargetFileName(const NFmiCachedDataFileInfo &theCachedDataFileInfo, const NFmiHelpDataInfo &theDataInfo, const NFmiHelpDataInfoSystem &theHelpDataSystem)
{
	// datatiedoston target polku+nimi saadaan k‰ytt‰m‰ll‰ NFmiHelpDataInfo:n UsedFileFilter:in polkua ja source-filen nimi osaa
    NFmiFileString fileStr = ::MakeFileStringWithoutCompressionFileExtension(theCachedDataFileInfo);
	NFmiString fileNameStr = fileStr.FileName();
	NFmiFileString usedFileFilterStr = theDataInfo.UsedFileNameFilter(theHelpDataSystem);
	std::string totalCacheFileName = static_cast<char*>(usedFileFilterStr.Device());
	totalCacheFileName += static_cast<char*>(usedFileFilterStr.Path());
	totalCacheFileName += static_cast<char*>(fileNameStr);
	return totalCacheFileName;
}

static std::string MakeFinalTmpFileName(const NFmiCachedDataFileInfo &theCachedDataFileInfo, const NFmiHelpDataInfo &theDataInfo, const NFmiHelpDataInfoSystem &theHelpDataSystem, bool fGetPackedName)
{
    NFmiFileString fileStr = fGetPackedName ? NFmiFileString(theCachedDataFileInfo.itsTotalServerFileName) : ::MakeFileStringWithoutCompressionFileExtension(theCachedDataFileInfo);
	NFmiString fileNameStr = fileStr.FileName();
	std::string totalCacheTmpFileName = theHelpDataSystem.LocalDataTmpDirectory();
    if(!theCachedDataFileInfo.fFilePacked)
    {
        // Etu TMP-liite laitetaan vain ei pakattuihin datoihin.
        // SYY: Jostain syyst‰ bzip2 tyyppi puretaan niin ett‰ purettuun datatiedostoon tulee mukaan pakatun tiedoston etiliite, jos purkaus tehd‰‰n 7-zip ohjelmalla.
        // Jos purku tehd‰‰n zip tai 7zip pakattuihin datoihin, etuliitett‰ ei tule purettuun tiedostoon (this behaviour really sucks!!!).
    	totalCacheTmpFileName += theHelpDataSystem.CacheTmpFileNameFix() + "_"; // laitetaan tmp-nimi fixi tiedosto nimen alkuun ja loppuun
    }
	totalCacheTmpFileName += static_cast<char*>(fileNameStr);
	totalCacheTmpFileName += "_";
	totalCacheTmpFileName += theHelpDataSystem.CacheTmpFileNameFix();
	return totalCacheTmpFileName;
}

static const std::vector<std::string> g_ZippedFileExtensions{ ".7z", ".zip", ".bz2" }; //, ".gz" , ".tar" , ".xz" , ".wim" };

// Kokeillaan eri pakkaus p‰‰tteit‰ prioriteetti j‰rjestyksess‰, heti kun lˆytyy jotain jollain p‰‰tteell‰, etsint‰ loppuu.
static std::string TryToFindNewestPackedFileName(const std::string& theFileFilter)
{
    for(auto& zipExtension : g_ZippedFileExtensions)
    {
        auto totalPackedFileName = NFmiFileSystem::NewestPatternFileName(theFileFilter + zipExtension);
        if(!totalPackedFileName.empty())
            return totalPackedFileName;
    }
    return "";
}

// Etsii uusimman tiedoston, joka vastaa annettua fileFilteri‰ ja lˆytyy server puolelta 
// ja palauttaa sen koko nimen polkuineen.
// Tutkii ensin lˆytyykˆ tiedostosta 7z, zip tai bz2-p‰‰tteist‰ versiota, koska pakatut tiedostot ovat 
// prioriteetissa ensin. Jos oli pakattu tiedosto, asetetaan pair:in second-arvoon true, 
// muuten se on false.
static void GetNewestFileInfo(const std::string &theFileFilter, NFmiCachedDataFileInfo &theCachedDataFileInfoOut)
{
    std::string totalPackedFileName = TryToFindNewestPackedFileName(theFileFilter);
    if(!totalPackedFileName.empty())
    {
        theCachedDataFileInfoOut.itsTotalServerFileName = totalPackedFileName;
        theCachedDataFileInfoOut.fFilePacked = true;
    }
    else
    {
        std::string totalFileName = NFmiFileSystem::NewestPatternFileName(theFileFilter);
        theCachedDataFileInfoOut.itsTotalServerFileName = totalFileName;
        theCachedDataFileInfoOut.fFilePacked = false;
    }
}

// Kokeillaan eri pakkaus p‰‰tteit‰ prioriteetti j‰rjestyksess‰, heti kun lˆytyy jotain jollain p‰‰tteell‰, etsint‰ loppuu.
static std::list<std::string> TryToFindPackedFileNameListWithFileFilter(const std::string& theFileFilter)
{
    for(auto& zipExtension : g_ZippedFileExtensions)
    {
        auto packedFileNameList = NFmiFileSystem::PatternFiles(theFileFilter + zipExtension);
        if(!packedFileNameList.empty())
            return packedFileNameList;
    }
    return std::list<std::string>();
}

// Sama kuin edell‰ GetNewestFileInfo-funktiossa, mutta haetaan joko pakattujen tiedostojen listaa
// tai ei pakattujen tiedostojen listaa.
static std::pair<std::list<std::string>, bool> GetNewestFileInfoList(const std::string &theFileFilter)
{
    std::list<std::string> packedFileList = ::TryToFindPackedFileNameListWithFileFilter(theFileFilter);
    if(packedFileList.empty() == false)
        return std::make_pair(packedFileList, true);
    else
    {
        std::list<std::string> fileList = NFmiFileSystem::PatternFiles(theFileFilter);
        return std::make_pair(fileList, false);
    }
}

// Sample results from MakeRestOfTheFileNames function:
// Original data path: p:\\data\\in\\202001021141_gfs_scandinavia_pressure.sqd.bz2
// itsTotalCacheFileName: D:\\smartmet\\wrk\\data\\local\\202001021141_gfs_scandinavia_pressure.sqd
// itsTotalCacheTmpFileName: D:\\smartmet\\wrk\\data\\tmp\\202001021141_gfs_scandinavia_pressure.sqd_TMP
// itsTotalCacheTmpPackedFileName: D:\\smartmet\\wrk\\data\\tmp\\202001021141_gfs_scandinavia_pressure.sqd.bz2_TMP
static void MakeRestOfTheFileNames(NFmiCachedDataFileInfo &theCachedDataFileInfoInOut, const NFmiHelpDataInfo &theDataInfo, const NFmiHelpDataInfoSystem &theHelpDataSystem)
{
    theCachedDataFileInfoInOut.itsTotalCacheFileName = ::MakeFinalTargetFileName(theCachedDataFileInfoInOut, theDataInfo, theHelpDataSystem);
    theCachedDataFileInfoInOut.itsTotalCacheTmpFileName = ::MakeFinalTmpFileName(theCachedDataFileInfoInOut, theDataInfo, theHelpDataSystem, false);
    theCachedDataFileInfoInOut.itsTotalCacheTmpPackedFileName = ::MakeFinalTmpFileName(theCachedDataFileInfoInOut, theDataInfo, theHelpDataSystem, true);
}

// Funktio tutkii annetun theDataInfo:n avulla onko kyseess‰ cacheen ladattava data
// ja onko levypalvelimella uudempaa tiedostoa kuin paikallisessa cachessa.
// Paluu arvot:
// kFmiNoCopyNeeded = ei ollut mit‰‰n luettavaa
// kFmiCopyWentOk = oli luettavaa ja se on luettu ilman ongelmia cacheen
// kFmiCopyNotSuccessfull = oli luettavaa, mutta ei voitu kopioida tiedostoa (yksi mahd. syy on ett‰ 
//		toinen SmartMet on juuri kopioimassa sit‰), t‰m‰ tulkitaan siten ett‰ ei ollut mit‰‰n luettavaa/kopioitavaa
static CFmiCopyingStatus CopyQueryDataToCache(const NFmiHelpDataInfo &theDataInfo, const NFmiHelpDataInfoSystem &theHelpDataSystem, CFmiCacheLoaderData *theCacheLoaderData)
{
	if(NFmiCachedDataFileInfo::IsDataCached(theDataInfo))
	{
		// 1. Mik‰ on uusimman file-filterin mukaisen tiedoston nimi, ja oliko kyse pakatusta tiedostosta
        std::string fileFilter = theDataInfo.FileNameFilter();
        NFmiCachedDataFileInfo cachedDataFileInfo;
        ::GetNewestFileInfo(fileFilter, cachedDataFileInfo);
        gMissingDataOnServerReporter.doReportIfFileFilterHasNoRelatedDataOnServer(cachedDataFileInfo, fileFilter);

		NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
        if(!cachedDataFileInfo.itsTotalServerFileName.empty())
		{
            if(::DoesThisThreadCopyFile(cachedDataFileInfo, theCacheLoaderData))
            {
                ::MakeRestOfTheFileNames(cachedDataFileInfo, theDataInfo, theHelpDataSystem);
                // 2. onko sen nimist‰ tiedostoa jo cachessa
                // 3. tee cache kopiointia varten tmp-nimi tiedostosta (joka kopioinnin j‰lkeen renametaan oikeaksi)
                // 4. onko tmp-nimi jo cachessa (t‰llˆin mahd. toisen SmartMetin kopio on jo k‰ynniss‰)
                // 5. tee varsinainen tiedosto kopio cacheen
                return ::CopyFileToLocalCache(cachedDataFileInfo, theCacheLoaderData, theDataInfo);
            }
		}
	}

	return kFmiNoCopyNeeded;
}

static bool LetGoAfterFirstTimeDelaying(NFmiMilliSecondTimer &theTimer, bool theFirstTimeflag, int theDelayTimeInMS)
{
	if(theFirstTimeflag)
	{
		if(theTimer.CurrentTimeDiffInMSeconds() >= theDelayTimeInMS)
			return true;
	}
	return false;
}

static std::string DeletedFilesStr(const std::list<std::string> &theDeletedFiles)
{
	std::string str;
	for(auto filePath : theDeletedFiles)
	{
        if(!str.empty())
            str += "\n";
        boost::replace_all(filePath, "\\/", "\\");
		str += filePath;
	}
	return str;
}

static void CleanDirectory(const std::string &theDirectory, double theKeepHours)
{
	try
	{
		std::list<std::string> deletedFiles;
		NFmiFileSystem::CleanDirectory(theDirectory, theKeepHours, &deletedFiles);
		if(deletedFiles.size())
		{
			std::string logStr("CFmiQueryDataCacheLoaderThread - CleanDirectory: ");
			logStr += theDirectory;
			logStr += ", keep-hour: ";
			logStr += NFmiStringTools::Convert(theKeepHours);
			logStr += "\nDeleted files:\n";
			logStr += ::DeletedFilesStr(deletedFiles);
			CatLog::logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Data);
		}
	}
	catch(...)
	{
		// ei tehd‰ toistaiseksi mit‰‰n...
	}
}

void CleanFilePattern(const std::string &theFilePattern, int theKeepMaxFiles)
{
	try
	{
        std::string logMessageStart("CFmiQueryDataCacheLoaderThread - CleanFilePattern: ");
        logMessageStart += theFilePattern;
        logMessageStart += ",\nkeep-max-files = ";
        logMessageStart += NFmiStringTools::Convert(theKeepMaxFiles);
        logMessageStart += ",\nDeleted files:\n";
        CtrlViewUtils::DeleteFilesWithPatternAndLog(theFilePattern, logMessageStart, CatLog::Severity::Debug, CatLog::Category::Data, theKeepMaxFiles);
	}
	catch(...)
	{
		// ei tehd‰ toistaiseksi mit‰‰n...
	}
}

// Siivotaan combine-data cachet.
// N‰m‰ olivat omissa alihakemistoissaan ja niille oli omat keep-files m‰‰r‰t.
// siivotaan kuitenkin jokaista ali-hakemistoa yhteisen keepMaxDays:in mukaan.
static void CleanCombineDataCache(void)
{
	if(gWorkerHelpDataSystem.DoCleanCache())
	{
		NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
		for(size_t i=0; i < gWorkerHelpDataSystem.DynamicHelpDataInfos().size(); i++)
		{
			NFmiHelpDataInfo &helpData = gWorkerHelpDataSystem.DynamicHelpDataInfo(static_cast<int>(i));
			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
			if(NFmiCachedDataFileInfo::IsDataCached(helpData) && helpData.IsCombineData())
			{
				double keepDays = gWorkerHelpDataSystem.CacheFileKeepMaxDays();
				if(keepDays > 0)
				{
					NFmiFileString usedFileFilterStr = helpData.UsedFileNameFilter(gWorkerHelpDataSystem);
					std::string cacheDir = static_cast<char*>(usedFileFilterStr.Device());
					cacheDir += static_cast<char*>(usedFileFilterStr.Path());
					::CleanDirectory(cacheDir, keepDays * 24);
				}
				::CleanFilePattern(helpData.UsedFileNameFilter(gWorkerHelpDataSystem), helpData.CombineDataMaxTimeSteps() + 3);
			}
		}
	}
}

static int CalcMaxKeepFileCount(const NFmiHelpDataInfo &helpDataInfo, NFmiCaseStudySystem & caseStudySystem)
{
    NFmiInfoData::Type dataType = helpDataInfo.DataType();
    if(dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kSingleStationRadarData || dataType == NFmiInfoData::kTrajectoryHistoryData || dataType == NFmiInfoData::kFlashData || dataType == NFmiInfoData::kAnalyzeData)
    {
        return 1; // n‰it‰ datatyyppeja on turhaa s‰ilˆ‰ 1 enemp‰‰
    }
    else
    {
        auto localCacheFileCount = 3;
        auto *caseStudyDataFile = caseStudySystem.FindCaseStudyDataFile(helpDataInfo.Name());
        if(caseStudyDataFile)
            localCacheFileCount = caseStudyDataFile->DataFileWinRegValues().LocalCacheDataCount();
        return localCacheFileCount;
    }
}

// HUOM! siivouksessa ei tarkisteta onko jokin data k‰ytˆss‰ vai ei (NFmiHelpDataInfo:n IsEnabled-metodi tarkistus), vanhoja tiedostoja ei 
// j‰tet‰ levyille lojumaan, vaikka joku data on joskus otettu pois k‰ytˆst‰.
static void CleanCache(void)
{
    if(gWorkerHelpDataSystem.DoCleanCache())
    {
        NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
        // 1. siivotaan ensin pois kaikki yli halutun aikam‰‰reen olevat tiedostot
        if(gWorkerHelpDataSystem.CacheFileKeepMaxDays() > 0)
            ::CleanDirectory(gWorkerHelpDataSystem.LocalDataLocalDirectory(), gWorkerHelpDataSystem.CacheFileKeepMaxDays() * 24);
        // 2. siivotaan tmp-hakemistosta kaikki yli puoli tuntia vanhemmat tiedostot (jos ne eiv‰t lukossa), oletetaan
        // ett‰ yhden tiedoston kopiointi ei kest‰ yli puolta tuntia, vaan kyse on jostain virheest‰.
        NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
        ::CleanDirectory(gWorkerHelpDataSystem.LocalDataTmpDirectory(), 0.5);

        auto& caseStudySystem = SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->CaseStudySystem();

        // 3. siivotaan pois file-pattern -kohtaisesti ylim‰‰r‰iset tiedostot n-kpl
        for(size_t i = 0; i < gWorkerHelpDataSystem.DynamicHelpDataInfos().size(); i++)
        {
            NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
            NFmiHelpDataInfo& helpDataInfo = gWorkerHelpDataSystem.DynamicHelpDataInfo(static_cast<int>(i));
            // HUOM! yhdistelm‰ datoja ei siivota t‰ss‰ yleisill‰ asetuksilla, vaan ne 
            // pit‰‰ siivota eri lailla ja sit‰ varten on oma funktio (CleanCombineDataCache).
            if(NFmiCachedDataFileInfo::IsDataCached(helpDataInfo) && helpDataInfo.IsCombineData() == false)
                ::CleanFilePattern(helpDataInfo.UsedFileNameFilter(gWorkerHelpDataSystem), ::CalcMaxKeepFileCount(helpDataInfo, caseStudySystem));
        }
    }
    ::CleanCombineDataCache();
}

static bool IsCacheCleaningDoneAtAll(CFmiCacheLoaderData &cacheLoaderData)
{
    if(gLoadDataAtStartUp && gAutoLoadNewCacheDataMode && cacheLoaderData.itsThreadPriority == 1)
        return true;
    else
        return false;
}

static bool IsDataUsed(const NFmiHelpDataInfo& helpDataInfo)
{
    return helpDataInfo.IsEnabled() && !helpDataInfo.CaseStudyLegacyOnly();
}

// K‰y l‰pi kaikki dynaamiset helpdatat ja tekee tarvittavat cache-kopioinnit.
// Jos ei lˆytynyt mit‰‰n kopioitavaa koko kierroksella, palauttaa 0, joka tarkoittaa
// ett‰ worker-threadi voi pit‰‰ taukoa.
// Jos palauttaa 1:n, tarkoittaa ett‰ jotain kopiointi on tapahtunut ja on syyt‰ tehd‰ uusi kierros saman tien.
static int GoThroughAllHelpDataInfos(const NFmiHelpDataInfoSystem &theHelpDataSystem, CFmiCacheLoaderData *theCacheLoaderData)
{
    ::ApplyChangedSettings(); // katsotaan onko asetuksia muutettu
    CFmiCopyingStatus status = kFmiNoCopyNeeded;
    const auto &helpInfos = theHelpDataSystem.DynamicHelpDataInfos();
    for(size_t i = 0; i < helpInfos.size(); i++)
    {
        NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
        const NFmiHelpDataInfo &helpDataInfo = helpInfos[i];
        // Ei miss‰‰n tilanteessa haluta ladata dataa serverilta lokaali cacheen, 
        // jos se on disabloitu tai se on merkitty CaseStudy legacy dataksi.
        if(::IsDataUsed(helpDataInfo))
        {
            CFmiCopyingStatus tmpStatus = CopyQueryDataToCache(helpDataInfo, theHelpDataSystem, theCacheLoaderData);
            if(tmpStatus == kFmiCopyWentOk)
            {
                status = kFmiCopyWentOk;
                CFmiDataLoadingThread2::LoadDataNow(); // laitetaan tietoa data-loading threadille ett‰ on tullut uutta dataa
            }
        }
    }

    gMissingDataOnServerReporter.mainWorkerThreadCompletesCycle();
    return status;
}

static void LogGeneralMessage(const std::string &theThreadNameStr, const std::string &theStartMessage, const std::string &theEndMessage, CatLog::Severity logLevel)
{
	std::string logStr(theStartMessage);
	logStr += " ";
	logStr += theThreadNameStr;
	logStr += " ";
	logStr += theEndMessage;
    CatLog::logMessage(logStr, logLevel, CatLog::Category::Data);
}

UINT CFmiQueryDataCacheLoaderThread::DoThread(LPVOID pParam)
{
	CFmiCacheLoaderData *cacheLoaderData = (CFmiCacheLoaderData *)pParam;
	if(cacheLoaderData == 0 || cacheLoaderData->itsThreadRunning == 0)
	{
        CatLog::logMessage("CFmiQueryDataCacheLoaderThread::DoThread error CFmiCacheLoaderData-was invalid application error, stopping...", CatLog::Severity::Error, CatLog::Category::Data);
		return 1;
	}
	std::string threadNameStr = cacheLoaderData->itsThreadName;
	CSingleLock singleLock(cacheLoaderData->itsThreadRunning); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(2000)) // Attempt to lock the shared resource, 2000 means 2 sec wait, 0 wait resulted sometimes to wait for next minute for unknown reason
	{
		::LogGeneralMessage(threadNameStr, "CFmiQueryDataCacheLoaderThread::DoThread with", "was allready running, stopping...", CatLog::Severity::Warning);
		return 1;
	}
	else
		::LogGeneralMessage(threadNameStr, "CFmiQueryDataCacheLoaderThread::DoThread with", "was started...", CatLog::Severity::Debug);

	MakeCacheDirectories();

	NFmiMilliSecondTimer timer;
	NFmiMilliSecondTimer cleanerTimer;
	bool firstTime = true;

	// T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
	// Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
	int counter = 0;
	int loadingStatus = 0;
	try
	{
		for( ; ; counter++)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);

            if(::LetGoAfterFirstTimeDelaying(timer, firstTime, cacheLoaderData->itsStartUpWaitTimeInMS) || loadingStatus || timer.CurrentTimeDiffInMSeconds() > (60 * 1000)) // tarkistetaan v‰hint‰‰n minuutin v‰lein onko tullut uusia datoja palvelimelle
            {
                firstTime = false;
                try
                {
                    if(gLoadDataAtStartUp && gAutoLoadNewCacheDataMode)
                        loadingStatus = ::GoThroughAllHelpDataInfos(gWorkerHelpDataSystem, cacheLoaderData);
                    else
                        loadingStatus = kFmiNoCopyNeeded;
                }
                catch(NFmiStopThreadException & /* e */)
                {
                    return 0; // SmartMet haluaa lopettaa, tullaan ulos thred funktiosta
                }
                catch(...)
                {
                    // t‰m‰ oli joku 'tavallinen' virhe tilanne,
                    // jatketaan vain loopitusta
                }

                timer.StartTimer(); // aloitetaan taas uusi ajan lasku
            }

			// tehd‰‰n tunnin v‰lein datojen cachen siivous 
			// HUOM! 1. kerran tunnin kuluttua koneen k‰ynnistymisest‰, ett‰ ei heti tuhota vanhaa dataa pois.
			// HUOM! n‰it‰ worker-threadeja k‰ynnistet‰‰n 3-4 kpl, joten vain yksi niist‰ siivoaa..
            if(::IsCacheCleaningDoneAtAll(*cacheLoaderData) && cleanerTimer.CurrentTimeDiffInMSeconds() > (gCacheCleaningIntervalInHours * 60 * 60 * 1000))
			{
                ::CleanCache();
				cleanerTimer.StartTimer(); // aloitetaan taas uusi siivous ajan lasku
			}

			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
			Sleep(1*1000); // nukutaan aina lyhyit‰ aikoja (1 s), ett‰ osataan tutkia usein, josko p‰‰ohjelma haluaa jo sulkea
		}
	}
	catch(...)
	{
		// t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
	}

//	::LogGeneralMessage(threadNameStr, "CFmiQueryDataCacheLoaderThread::DoThread with", "is now stopped as requested...");

    return 0;   // thread completed successfully
}

// T‰t‰ initialisointi funktiota pit‰‰ kutsua ennen kuin itse threadi k‰ynnistet‰‰n MainFramesta. 
void CFmiQueryDataCacheLoaderThread::InitHelpDataInfo(const NFmiHelpDataInfoSystem &helpDataInfoSystem, const std::string &smartMetBinariesDirectory, double cacheCleaningIntervalInHours, const std::string& smartMetWorkingDirectory, int usedDataLoaderThreadCount)
{
	gWorkerHelpDataSystem = helpDataInfoSystem;
    gSmartMetBinDirectory = smartMetBinariesDirectory;
    gSmartMetWorkingDirectory = smartMetWorkingDirectory;

	gCopyFileExCancel = FALSE;
	gSettingsHaveChanged = false; // t‰m‰ tarkoittaa sit‰ ett‰ asetukset ovat jo gWorkerHelpDataSystem-oliossa (ei mediator-oliossa)

    gUseCopyFileExFunction = NFmiSettings::Optional("SmartMet::UseCopyFileExFunction", true);
    gUsedChunckSize = NFmiSettings::Optional("SmartMet::UsedChunckSizeInKB", 512) * 1024;
    gCacheCleaningIntervalInHours = cacheCleaningIntervalInHours;
    gMissingDataOnServerReporter.initialize(gWorkerHelpDataSystem);
}

void CFmiQueryDataCacheLoaderThread::CloseNow(void)
{
	gStopFunctor.Stop(true);
	gCopyFileExCancel = TRUE;
}

int CFmiQueryDataCacheLoaderThread::WaitToClose(int theMilliSecondsToWait, CFmiCacheLoaderData *theCacheLoaderData)
{
	if(theCacheLoaderData && theCacheLoaderData->itsThreadRunning)
	{
		CSingleLock singleLock(theCacheLoaderData->itsThreadRunning);
		if(singleLock.Lock(theMilliSecondsToWait))
			return 1; // onnellinen lopetus saatu aikaan....
	}
	return 0; // jokin oli pieless‰, ei voi mit‰‰n....
}

// CollectHistoryDataToCache palauttaa true, jos yksikin tiedosto kopioitiin serverilt‰ lokaaliin hakemistoon.
static bool CollectHistoryDataToCache(const NFmiHelpDataInfo &theDataInfo, const NFmiHelpDataInfoSystem &theHelpDataSystem, CFmiCacheLoaderData *theCacheLoaderData)
{
    bool anythingCopied = false;
	// aluksi tehd‰‰n vain combine-datojen historiat
	if(theDataInfo.IsCombineData())
	{
        if(CatLog::doTraceLevelLogging())
        {
            std::string debugStr = theCacheLoaderData->itsThreadName;
            debugStr += ": starting to load history data for: ";
            debugStr += theDataInfo.UsedFileNameFilter(theHelpDataSystem);
            CatLog::logMessage(debugStr, CatLog::Severity::Trace, CatLog::Category::Data);
        }

		std::string usedPattern = theDataInfo.FileNameFilter();
		std::string usedPath = NFmiFileSystem::PathFromPattern(usedPattern);
        std::pair<std::list<std::string>, bool> fileInfoList = ::GetNewestFileInfoList(usedPattern);
        std::list<std::string> &fileList = fileInfoList.first;
		fileList.sort(); // sortataan
		fileList.reverse(); // k‰‰nnet‰‰n j‰rjestys, jolloin jos k‰ytetty YYYYMMDDHHmmss aikaleimaa tiedoston alussa, tulee uusimmat tiedostot alkuun
		size_t counter = 0;
		for(std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
			std::string totalFileName = usedPath;
			totalFileName += *it;
            NFmiCachedDataFileInfo cachedDataFileInfo;
            cachedDataFileInfo.itsTotalServerFileName = totalFileName;
            cachedDataFileInfo.fFilePacked = fileInfoList.second;
            ::MakeRestOfTheFileNames(cachedDataFileInfo, theDataInfo, theHelpDataSystem);
            if(::CopyFileToLocalCache(cachedDataFileInfo, theCacheLoaderData, theDataInfo) == kFmiCopyWentOk)
            {
                anythingCopied = true;
            }
			counter++;
			if(counter > theDataInfo.CombineDataMaxTimeSteps())
				break;
		}
	}
    return anythingCopied;
}

static void CollectAllHistoryDatas(const NFmiHelpDataInfoSystem& theHelpDataSystem, CFmiCacheLoaderData* theCacheLoaderData)
{
    ::ApplyChangedSettings(); // katsotaan onko asetuksia muutettu
    const auto& helpDataInfos = theHelpDataSystem.DynamicHelpDataInfos();
    for(const auto &helpDataInfo : helpDataInfos)
    {
        NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
        if(::IsDataUsed(helpDataInfo))
        {
            ::CollectHistoryDataToCache(helpDataInfo, theHelpDataSystem, theCacheLoaderData);
        }
    }
}

// T‰m‰ apu threadi kopioi lokaali cacheen kaiken tarvittavan historia datan.
// Se k‰ynnistet‰‰n alemmalla prioriteetilla ja k‰y l‰pi ensin kaikki combine-datat. 
// TODO kopioi myˆs n kpl malli ajoja.
UINT CFmiQueryDataCacheLoaderThread::DoHistoryThread(LPVOID pParam)
{
	CFmiCacheLoaderData *cacheLoaderData = (CFmiCacheLoaderData *)pParam;
	if(cacheLoaderData == 0 || cacheLoaderData->itsThreadRunning == 0)
	{
        CatLog::logMessage("CFmiQueryDataCacheLoaderThread::DoHistoryThread error CFmiCacheLoaderData-was invalid application error, stopping...", CatLog::Severity::Error, CatLog::Category::Data);
		return 1;
	}

	std::string threadNameStr = cacheLoaderData->itsThreadName; // otetaan threadin nimi talteen, koska kun ohjelma lopetetaan, ja threadin lopetuksesta raportoidaan, saattaa tietyiss‰ tapauksissa 
																// cacheLoaderData -olio olla jo tuhottuna, joten nimi otetaan heti talteen ett‰ sit‰ voidaan k‰ytt‰‰ lokituksessa myˆs lopussa
	CSingleLock singleLock(cacheLoaderData->itsThreadRunning); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(2000)) // Attempt to lock the shared resource, 2000 means 2 sec wait, 0 wait resulted sometimes to wait for next minute for unknown reason
	{
		::LogGeneralMessage(threadNameStr, "CFmiQueryDataCacheLoaderThread::DoHistoryThread with", "was allready running, stopping...", CatLog::Severity::Warning);
		return 1;
	}
	else
		::LogGeneralMessage(threadNameStr, "CFmiQueryDataCacheLoaderThread::DoHistoryThread with", "was started...", CatLog::Severity::Debug);

	MakeCacheDirectories();

	NFmiMilliSecondTimer timer;
	bool firstTime = true;
	int counter = 0;
	try
	{
		for( ; ; counter++)
		{
            NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);

            if(::LetGoAfterFirstTimeDelaying(timer, firstTime, cacheLoaderData->itsStartUpWaitTimeInMS))
            {
                firstTime = false;

                try
                {
                    // t‰m‰ historia-threadi k‰yd‰‰n siis vain kerran l‰pi
                    NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
                    ::CollectAllHistoryDatas(gWorkerHelpDataSystem, cacheLoaderData);
                }
                catch(...)
                {
                    // t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
                }

                ::LogGeneralMessage(threadNameStr, "CFmiQueryDataCacheLoaderThread::DoHistoryThread with", "is now stopped as requested...", CatLog::Severity::Debug);
                return 0;   // thread completed successfully
            }
			Sleep(1*1000); // nukutaan aina sekunnin p‰tki‰, ett‰ voidaan tarkkailla lopetus merkki‰
		}
	}
	catch(...)
	{
	}
	return 0;   // thread completed successfully
}
