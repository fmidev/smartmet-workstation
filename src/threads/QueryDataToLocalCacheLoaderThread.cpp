#include "stdafx.h"
#include "QueryDataToLocalCacheLoaderThread.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiMissingDataOnServerReporter.h"
#include "NFmiOnceLoadedDataFiles.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiSettings.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiCachedDataFileInfo.h"
#include "NFmiCaseStudySystem.h"
#include "FmiDataLoadingThread2.h"
#include "NFmiLedLightStatus.h"
#include "NFmiValueString.h"
#include "execute-command-in-separate-process.h"
#include "LocalCacheHelpDataSystem.h"

#include <thread>
#include <mutex>

namespace
{
    // Jos ohjelma halutaan lopettaa, t‰lle gStopFunctor:ille annetaan tieto siit‰ ulkoap‰in.
    NFmiStopFunctor gStopFunctor;
    // T‰m‰n avulla CopyFileEx-funktio voidaan keskeytt‰‰, arvo muutetaan TRUE:ksi.
    BOOL gCopyFileExCancel;
    // T‰m‰n olion avulla working thread osaa lukea/kopioida haluttuja datoja
    LocalCacheHelpDataSystem gLocalCacheHelpDataSystem;
    // SmartMetin bin‰‰ri-hakemistoa tarvitaan ainakin kun tehd‰‰n tiedostojen purkua erillisess‰ prosessissa (purku ohjelma sijaitsee siell‰ miss‰ smartmetin exe)
    std::string gSmartMetBinDirectory;
    // SmartMetin Working-hakemistoa tarvitaan kun rakennetaan polkua 7-zip ohjelmalle (purku ohjelma sijaitsee sen utils-hakemistossa)
    std::string gSmartMetWorkingDirectory;
    // T‰m‰n avulla tiedet‰‰n, suljettaessa ohjelmaa ett‰ onko working-thread 
    // lopettanut tyˆns‰ ja sen puolesta voidaan ohjelman sulkemista jatkaa.
    std::timed_mutex gThreadRunningMutex;
    // Jos t‰m‰ on false, se est‰‰ datan latauksen cacheen ja hakemistojen siivouksen
    bool gLoadDataAtStartUp = true;
    // Onko SmartMet ns. operatiivisessa moodissa, eli silloin se automaattisesti latailee uusia datoja cacheen ja silloin myˆs vanhoja pit‰‰ siivoilla pois.
    // Joskus halutaan ett‰ uutta dataa ei lueta eik‰ vanhoja siivota, silloin t‰m‰ moodi pit‰‰ laittaa pois p‰‰lt‰.
    // Myˆs jos ollaan ns. no-data/tiputus moodissa, t‰llˆin ei saa tuhota vanhoja tiedostoja.
    bool gAutoLoadNewCacheDataMode = true;
    // Jos k‰ytet‰‰n FileCopyMarko:a, t‰m‰n kokoisia puskureita k‰ytet‰‰n kopioinnissa
    size_t gUsedChunckSize;
    // T‰h‰n otetaan GeneralDocista siivousintervalli, oletus 0.16 [h] eli n. 10 minuutin v‰lein.
    double gCacheCleaningIntervalInHours = 0.16;
    // Jos tiedosto on isompi kuin t‰ss‰ oleva raja, sit‰ ei kopioida ollenkaan
    double gMaxDataFileSizeInMB;
    std::string gThreadName = "Main-thread";
    const std::vector<std::string> g_ZippedFileExtensions{ ".7z", ".zip", ".bz2" }; //, ".gz" , ".tar" , ".xz" , ".wim" };

    NFmiMissingDataOnServerReporter gMissingDataOnServerReporter;
    NFmiOnceLoadedDataFiles gOnceLoadedDataFiles;

    NFmiMilliSecondTimer gDoWorkTimer;
    NFmiMilliSecondTimer gLocalCacheCleanerTimer;

    enum CFmiCopyingStatus
    {
        kFmiNoCopyNeeded = 0,
        kFmiCopyWentOk = 1,
        kFmiCopyNotSuccessfull = 2,
        kFmiGoOnWithCopying = 3,
        kFmiUnpackIsDoneInSeparateProcess = 4
    };

    void MakeCacheDirectories(NFmiHelpDataInfoSystem& helpDataSystem)
    {
        // Make sure that cache and tmp directories exist
        NFmiFileSystem::CreateDirectory(helpDataSystem.LocalDataLocalDirectory());
        NFmiFileSystem::CreateDirectory(helpDataSystem.LocalDataTmpDirectory());
        NFmiFileSystem::CreateDirectory(helpDataSystem.LocalDataPartialDirectory());
    }

    bool IsTimeToCheckForNewData(bool* firstTime, CFmiCopyingStatus status)
    {
        if(firstTime)
        {
            // Jos ollaan 1. kertaa loopissa, jatketaan heti ja nollataan siihen liittyv‰ firstTime flag
            firstTime = false;
            return true;
        }

        // Jos datan l‰pik‰ynniss‰ oli kopioitu mit‰‰n dataa, tehd‰‰n uusi kierros samantein
        if(status != kFmiNoCopyNeeded)
            return true;

        // Tarkistetaan minuutin v‰lein onko tullut uusia datoja palvelimelle kopioitavaksi
        return gDoWorkTimer.CurrentTimeDiffInMSeconds() > (60 * 1000);
    }

    bool IsCacheCleaningDoneAtAll()
    {
        if(gLoadDataAtStartUp && gAutoLoadNewCacheDataMode)
            return true;
        else
            return false;
    }

    std::string DeletedFilesStr(const std::list<std::string>& theDeletedFiles)
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

    void CleanDirectory(const std::string& theDirectory, double theKeepHours)
    {
        try
        {
            std::list<std::string> deletedFiles;
            NFmiFileSystem::CleanDirectory(theDirectory, theKeepHours, &deletedFiles);
            if(deletedFiles.size())
            {
                std::string logStr("QueryDataToLocalCacheLoaderThread - CleanDirectory: ");
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

    void CleanFilePattern(const std::string& theFilePattern, int theKeepMaxFiles)
    {
        try
        {
            std::string logMessageStart("QueryDataToLocalCacheLoaderThread - CleanFilePattern: ");
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

    int CalcMaxKeepFileCount(const NFmiHelpDataInfo& helpDataInfo, NFmiCaseStudySystem& caseStudySystem)
    {
        NFmiInfoData::Type dataType = helpDataInfo.DataType();
        if(NFmiInfoData::IsLatestOnlyBasedData(dataType))
        {
            return 1; // n‰it‰ datatyyppeja on turhaa s‰ilˆ‰ 1 enemp‰‰
        }
        else
        {
            auto localCacheFileCount = 3;
            auto* caseStudyDataFile = caseStudySystem.FindCaseStudyDataFile(helpDataInfo.Name());
            if(caseStudyDataFile)
                localCacheFileCount = caseStudyDataFile->DataFileWinRegValues().LocalCacheDataCount();
            return localCacheFileCount;
        }
    }

    // Siivotaan combine-data cachet.
    // N‰m‰ olivat omissa alihakemistoissaan ja niille oli omat keep-files m‰‰r‰t.
    // siivotaan kuitenkin jokaista ali-hakemistoa yhteisen keepMaxDays:in mukaan.
    void CleanCombineDataCache(NFmiHelpDataInfoSystem& theHelpDataSystem)
    {
        if(theHelpDataSystem.DoCleanCache())
        {
            NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
            for(size_t i = 0; i < theHelpDataSystem.DynamicHelpDataInfos().size(); i++)
            {
                NFmiHelpDataInfo& helpData = theHelpDataSystem.DynamicHelpDataInfo(static_cast<int>(i));
                NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
                if(NFmiCachedDataFileInfo::IsDataCached(helpData) && helpData.IsCombineData())
                {
                    double keepDays = theHelpDataSystem.CacheFileKeepMaxDays();
                    if(keepDays > 0)
                    {
                        NFmiFileString usedFileFilterStr = helpData.UsedFileNameFilter(theHelpDataSystem);
                        std::string cacheDir = usedFileFilterStr.Device();
                        cacheDir += usedFileFilterStr.Path();
                        ::CleanDirectory(cacheDir, keepDays * 24);
                    }
                    ::CleanFilePattern(helpData.UsedFileNameFilter(theHelpDataSystem), helpData.CombineDataMaxTimeSteps() + 3);
                }
            }
        }
    }

    // HUOM! siivouksessa ei tarkisteta onko jokin data k‰ytˆss‰ vai ei (NFmiHelpDataInfo:n IsEnabled-metodi tarkistus), vanhoja tiedostoja ei 
    // j‰tet‰ levyille lojumaan, vaikka joku data on joskus otettu pois k‰ytˆst‰.
    void CleanCache(NFmiHelpDataInfoSystem& theHelpDataSystem)
    {
        if(theHelpDataSystem.DoCleanCache())
        {
            NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
            // 1. siivotaan ensin pois kaikki yli halutun aikam‰‰reen olevat tiedostot
            if(theHelpDataSystem.CacheFileKeepMaxDays() > 0)
                ::CleanDirectory(theHelpDataSystem.LocalDataLocalDirectory(), theHelpDataSystem.CacheFileKeepMaxDays() * 24);
            // 2. siivotaan tmp-hakemistosta kaikki yli puoli tuntia vanhemmat tiedostot (jos ne eiv‰t lukossa), oletetaan
            // ett‰ yhden tiedoston kopiointi ei kest‰ yli puolta tuntia, vaan kyse on jostain virheest‰.
            NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
            ::CleanDirectory(theHelpDataSystem.LocalDataTmpDirectory(), 0.5);

            auto& caseStudySystem = SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->CaseStudySystem();

            // 3. siivotaan pois file-pattern -kohtaisesti ylim‰‰r‰iset tiedostot n-kpl
            for(size_t i = 0; i < theHelpDataSystem.DynamicHelpDataInfos().size(); i++)
            {
                NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
                NFmiHelpDataInfo& helpDataInfo = theHelpDataSystem.DynamicHelpDataInfo(static_cast<int>(i));
                // HUOM! yhdistelm‰ datoja ei siivota t‰ss‰ yleisill‰ asetuksilla, vaan ne 
                // pit‰‰ siivota eri lailla ja sit‰ varten on oma funktio (CleanCombineDataCache).
                if(NFmiCachedDataFileInfo::IsDataCached(helpDataInfo) && helpDataInfo.IsCombineData() == false)
                    ::CleanFilePattern(helpDataInfo.UsedFileNameFilter(theHelpDataSystem), ::CalcMaxKeepFileCount(helpDataInfo, caseStudySystem));
            }
        }
        ::CleanCombineDataCache(theHelpDataSystem);
    }

    // Tehd‰‰n lokaali data cachen siivous halutulla v‰leill‰ (10 minuuttia)
    void DoPossibleLocalCacheCleaning(std::shared_ptr<NFmiHelpDataInfoSystem> theHelpDataSystemPtr)
    {
        // T‰nne annettu shared_ptr on tarkoituksella kopio, ja siit‰ otetaan k‰ytett‰v‰ referenssi ulos t‰ss‰.
        NFmiHelpDataInfoSystem& usedHelpDataSystem = *theHelpDataSystemPtr;

        if(!::IsCacheCleaningDoneAtAll())
        {
            return;
        }

        int cleaningIntervalInMS = int(gCacheCleaningIntervalInHours * 60 * 60 * 1000);
        if(gLocalCacheCleanerTimer.CurrentTimeDiffInMSeconds() > cleaningIntervalInMS)
        {
            ::CleanCache(usedHelpDataSystem);
            // Aloitetaan taas uusi siivous ajan lasku
            gLocalCacheCleanerTimer.StartTimer();
        }
    }

    // Ei miss‰‰n tilanteessa haluta ladata dataa serverilta lokaali cacheen, 
    // jos se on disabloitu tai se on merkitty CaseStudy legacy dataksi.
    bool IsDataUsed(const NFmiHelpDataInfo& helpDataInfo)
    {
        return helpDataInfo.IsEnabled() && !helpDataInfo.CaseStudyLegacyOnly();
    }

    // Kokeillaan eri pakkaus p‰‰tteit‰ prioriteetti j‰rjestyksess‰, heti kun lˆytyy jotain jollain p‰‰tteell‰, etsint‰ loppuu.
    std::list<std::string> TryToFindPackedFileNameListWithFileFilter(const std::string& theFileFilter)
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
    std::pair<std::list<std::string>, bool> GetNewestFileInfoList(const std::string& theFileFilter)
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

    NFmiFileString MakeFileStringWithoutCompressionFileExtension(const NFmiCachedDataFileInfo& theCachedDataFileInfo)
    {
        NFmiFileString fileStr(theCachedDataFileInfo.itsTotalServerFileName);
        if(theCachedDataFileInfo.fFilePacked)
            fileStr = fileStr.Device() + fileStr.Path() + fileStr.Header(); // pakatun tiedoston tapauksessa j‰tetaan pois tiedoston viimeinen extensio
        return fileStr;
    }

    std::string MakeFinalTargetFileName(const NFmiCachedDataFileInfo& theCachedDataFileInfo, const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem)
    {
        // datatiedoston target polku+nimi saadaan k‰ytt‰m‰ll‰ NFmiHelpDataInfo:n UsedFileFilter:in polkua ja source-filen nimi osaa
        NFmiFileString fileStr = ::MakeFileStringWithoutCompressionFileExtension(theCachedDataFileInfo);
        NFmiString fileNameStr = fileStr.FileName();
        NFmiFileString usedFileFilterStr = theDataInfo.UsedFileNameFilter(theHelpDataSystem);
        std::string totalCacheFileName = usedFileFilterStr.Device();
        totalCacheFileName += usedFileFilterStr.Path();
        totalCacheFileName += fileNameStr;
        return totalCacheFileName;
    }

    std::string MakeFinalTmpFileName(const NFmiCachedDataFileInfo& theCachedDataFileInfo, const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem, bool fGetPackedName)
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
        totalCacheTmpFileName += fileNameStr;
        totalCacheTmpFileName += "_";
        totalCacheTmpFileName += theHelpDataSystem.CacheTmpFileNameFix();
        return totalCacheTmpFileName;
    }

    // Sample results from MakeRestOfTheFileNames function:
    // Original data path: p:\\data\\in\\202001021141_gfs_scandinavia_pressure.sqd.bz2
    // itsTotalCacheFileName: D:\\smartmet\\wrk\\data\\local\\202001021141_gfs_scandinavia_pressure.sqd
    // itsTotalCacheTmpFileName: D:\\smartmet\\wrk\\data\\tmp\\202001021141_gfs_scandinavia_pressure.sqd_TMP
    // itsTotalCacheTmpPackedFileName: D:\\smartmet\\wrk\\data\\tmp\\202001021141_gfs_scandinavia_pressure.sqd.bz2_TMP
    void MakeRestOfTheFileNames(NFmiCachedDataFileInfo& theCachedDataFileInfoInOut, const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem)
    {
        theCachedDataFileInfoInOut.itsTotalCacheFileName = ::MakeFinalTargetFileName(theCachedDataFileInfoInOut, theDataInfo, theHelpDataSystem);
        theCachedDataFileInfoInOut.itsTotalCacheTmpFileName = ::MakeFinalTmpFileName(theCachedDataFileInfoInOut, theDataInfo, theHelpDataSystem, false);
        theCachedDataFileInfoInOut.itsTotalCacheTmpPackedFileName = ::MakeFinalTmpFileName(theCachedDataFileInfoInOut, theDataInfo, theHelpDataSystem, true);
    }

    // Kokeillaan eri pakkaus p‰‰tteit‰ prioriteetti j‰rjestyksess‰, heti kun lˆytyy jotain jollain p‰‰tteell‰, etsint‰ loppuu.
    std::string TryToFindNewestPackedFileName(const std::string& theFileFilter)
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
    void GetNewestFileInfo(const std::string& theFileFilter, NFmiCachedDataFileInfo& theCachedDataFileInfoOut)
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

    const double gKiloByte = 1024;
    const double gMegaByte = gKiloByte * gKiloByte;
    // t‰ss‰ tarkastetaan kuuluuko kyseinen data-tiedosto t‰lle threadille, eli tiedoston koon
    // pit‰‰ menn‰ rajojen sis‰‰n.
    bool DoesThisThreadCopyFile(NFmiCachedDataFileInfo& theCachedDataFileInfo)
    {
        theCachedDataFileInfo.itsFileSizeInMB = NFmiFileSystem::FileSize(theCachedDataFileInfo.itsTotalServerFileName) / gMegaByte;
        if(theCachedDataFileInfo.itsFileSizeInMB <= 0)
        {
            return false; // jostain syyst‰ tiedoston kokoa ei saatu
        }

        if(theCachedDataFileInfo.itsFileSizeInMB < gMaxDataFileSizeInMB)
        {
            return true;
        }

        return false;
    }

    // Tarkistetaan tmp-tiedoston tila. Jos sit‰ ei ole, voidaan kopiointiproseduuria jatkaa.
    // Jos se on olemassa, mutta sen voi poistaa (j‰‰nyt virhetilanteessa ja siin‰ ei ole nyt mik‰‰n kiinni?), voidaan jatkaa.
    // Jos sit‰ ei voi poistaa, ei jatketa eteenp‰in, koska joku muu threadi tai prosessi on luultavasti tekem‰ss‰ sille jotain.
    CFmiCopyingStatus CheckTmpFileStatus(const std::string& theTmpFileName)
    {
        if(NFmiFileSystem::FileExists(theTmpFileName))
        {
            // kokeillaan, voiko tmp-tiedoston deletoida, jos voi (eli kukaan ei ole sin‰ kiinni), se on j‰‰nyt jostain virhetilanteesta ja se voidaan deletoida ja aloittaa kopio uudestaan
            if(NFmiFileSystem::RemoveFile(theTmpFileName) == false)
                return kFmiNoCopyNeeded;
        }
        return kFmiGoOnWithCopying;
    }

    std::string MakeFileSizeString(double fileSizeInMB)
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

    std::string MakeLedChannelStartLoadingString(const NFmiCachedDataFileInfo& cachedDataFileInfo)
    {
        std::string reportStr = "Loading file from server (";
        reportStr += ::MakeFileSizeString(cachedDataFileInfo.itsFileSizeInMB);
        reportStr += "):\n";
        NFmiFileString fileStr(cachedDataFileInfo.itsTotalCacheFileName);
        reportStr += fileStr.FileName();
        return reportStr;
    }

    void EnsureCacheDirectoryForPartialData(const std::string& theTotalCacheFileName, const NFmiHelpDataInfo& theDataInfo)
    {
        if(theDataInfo.IsCombineData())
        { // partial-datoille on varmistettava hakemisto, koska ne laitetaan cachessakin eri hakemistoihin
            NFmiFileString fileStr(theTotalCacheFileName);
            std::string partialDataDirectory = fileStr.Device();
            partialDataDirectory += fileStr.Path();
            NFmiFileSystem::CreateDirectory(partialDataDirectory);
        }
    }

    void LogCopySuccess(const std::string& theTotalFileName, NFmiMilliSecondTimer& theTimer)
    {
        std::string logStr("Copying QData file '");
        logStr += theTotalFileName;
        logStr += "' to cache lasted: ";
        logStr += theTimer.EasyTimeDiffStr();
        CatLog::logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Data);
    }

    void LogRenameFailure(const std::string& theTotalCacheTmpFileName, const std::string& theTotalCacheFileName)
    {
        std::string logStr("Copying QData file was success but renaming tmp file failed\n");
        logStr += theTotalCacheTmpFileName;
        logStr += "\n -> \n";
        logStr += theTotalCacheFileName;
        CatLog::logMessage(logStr, CatLog::Severity::Warning, CatLog::Category::Data);
    }

    void LogCopyFailure(const std::string& theTotalFileName, const std::string& theTotalCacheTmpFileName)
    {
        std::string logStr("Copying QData file to cache tmp file failed\n");
        logStr += theTotalFileName;
        logStr += "\n -> \n";
        logStr += theTotalCacheTmpFileName;
        CatLog::logMessage(logStr, CatLog::Severity::Error, CatLog::Category::Data);
    }

    // Katso miten haluttu komentorivi pit‰‰ rakentaa smartmet_workstation\src\unpackdatafilesexe\UnpackSmartMetDataFilesMain.cpp
    // tiedoston main -funktion alusta, kun virhetilanteessa laitetaan ohjeita cout:iin.
    std::string MakeUnpackCommand(NFmiCachedDataFileInfo& theCachedDataFileInfo)
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
        commandStr += QueryDataToLocalCacheLoaderThread::MakeDailyUnpackLogFilePath();
        commandStr += "\"";

        return commandStr;
    }
    CFmiCopyingStatus DoFileUnpacking(NFmiCachedDataFileInfo& theCachedDataFileInfo)
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
            catch(std::exception& e)
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

    // T‰m‰ on copy-rename k‰ytt‰en win32:en CopyFileEx funktiota. 
    // T‰ss‰ on keskeytys ja progres-seuranta mahdollisuudet.
    // Jos server-datatiedosto oli pakattu (theNewestFileInfo.second == true),
    // pit‰‰ tiedosto kopioida normaalisti tmp-hakemistoon, mutta sen j‰lkeen riippuen 
    // pakatun tiedoston koosta, tiedosto pit‰‰ joko purkaa t‰‰ll‰ tai jos se on iso
    // tiedosto, pit‰‰ k‰ynnist‰‰ erillinen prosessi, joka purkaa ja siirt‰‰ tiedoston oikeaan 
    // paikkaan. Oma purku-prosessi on pakko k‰ynnist‰‰, koska purkua ei voi keskeytt‰‰ ja se voi kest‰‰ 
    // vaikka 3 minuuttia (esim. nykyinen arome mallipintadata). Jos k‰ytt‰j‰ sulkee SmartMetin, 
    // ei voi odottaa 3 minuuttia, vaan purku keskeytett‰isiin v‰kivaltaisesti, eik‰ seuraamuksia tiedet‰ viel‰.
    CFmiCopyingStatus CopyFileEx_CopyRename(NFmiCachedDataFileInfo& theCachedDataFileInfo)
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
        bool copyStatus = (CopyFileEx(totFileStrU_, totCacheTmpFileStrU_, 0, 0, &gCopyFileExCancel, dwCopyFlags) == TRUE);
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

    CFmiCopyingStatus CopyFileToLocalCache(NFmiCachedDataFileInfo& theCachedDataFileInfo, const NFmiHelpDataInfo& theDataInfo)
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
            NFmiLedLightStatusBlockReporter blockReporter(NFmiLedChannel::QueryData, gThreadName, ::MakeLedChannelStartLoadingString(theCachedDataFileInfo));
            ::EnsureCacheDirectoryForPartialData(theCachedDataFileInfo.itsTotalCacheFileName, theDataInfo);
            return ::CopyFileEx_CopyRename(theCachedDataFileInfo);
        }
        else
        {
            if(CatLog::doTraceLevelLogging())
            {
                std::string traceLoggingStr = gThreadName;
                traceLoggingStr += ": tmp file status was no go with: ";
                traceLoggingStr += theCachedDataFileInfo.itsTotalServerFileName;
                CatLog::logMessage(traceLoggingStr, CatLog::Severity::Trace, CatLog::Category::Data, true);
            }
        }

        return kFmiNoCopyNeeded;
    }

    // Funktio tutkii annetun theDataInfo:n avulla onko kyseess‰ cacheen ladattava data
    // ja onko levypalvelimella uudempaa tiedostoa kuin paikallisessa cachessa.
    // Paluu arvot:
    // kFmiNoCopyNeeded = ei ollut mit‰‰n luettavaa
    // kFmiCopyWentOk = oli luettavaa ja se on luettu ilman ongelmia cacheen
    // kFmiCopyNotSuccessfull = oli luettavaa, mutta ei voitu kopioida tiedostoa (yksi mahd. syy on ett‰ 
    //		toinen SmartMet on juuri kopioimassa sit‰), t‰m‰ tulkitaan siten ett‰ ei ollut mit‰‰n luettavaa/kopioitavaa
    CFmiCopyingStatus CopyQueryDataToCache(const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem)
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
                if(::DoesThisThreadCopyFile(cachedDataFileInfo))
                {
                    ::MakeRestOfTheFileNames(cachedDataFileInfo, theDataInfo, theHelpDataSystem);
                    // 2. onko sen nimist‰ tiedostoa jo cachessa
                    // 3. tee cache kopiointia varten tmp-nimi tiedostosta (joka kopioinnin j‰lkeen renametaan oikeaksi)
                    // 4. onko tmp-nimi jo cachessa (t‰llˆin mahd. toisen SmartMetin kopio on jo k‰ynniss‰)
                    // 5. tee varsinainen tiedosto kopio cacheen
                    return ::CopyFileToLocalCache(cachedDataFileInfo, theDataInfo);
                }
            }
        }

        return kFmiNoCopyNeeded;
    }

    // K‰y l‰pi kaikki dynaamiset helpdatat ja tekee tarvittavat cache-kopioinnit.
    // Jos ei lˆytynyt mit‰‰n kopioitavaa koko kierroksella, palauttaa kFmiNoCopyNeeded, joka tarkoittaa
    // ett‰ worker-threadi voi pit‰‰ taukoa.
    // Jos palauttaa kFmiCopyWentOk:n, tarkoittaa ett‰ jotain kopiointi on tapahtunut ja on 
    // syyt‰ tehd‰ uusi kierros saman tien.
    CFmiCopyingStatus GoThroughAllHelpDataInfos(std::shared_ptr<NFmiHelpDataInfoSystem> theHelpDataSystemPtr)
    {
        // T‰nne annettu shared_ptr on tarkoituksella kopio, ja siit‰ otetaan k‰ytett‰v‰ referenssi ulos t‰ss‰.
        NFmiHelpDataInfoSystem& usedHelpDataSystem = *theHelpDataSystemPtr;
        CFmiCopyingStatus status = kFmiNoCopyNeeded;
        const auto& helpInfos = theHelpDataSystemPtr->DynamicHelpDataInfos();
        for(size_t i = 0; i < helpInfos.size(); i++)
        {
            NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
            const NFmiHelpDataInfo& helpDataInfo = helpInfos[i];
            if(::IsDataUsed(helpDataInfo))
            {
                CFmiCopyingStatus tmpStatus = CopyQueryDataToCache(helpDataInfo, usedHelpDataSystem);
                if(tmpStatus == kFmiCopyWentOk)
                {
                    status = kFmiCopyWentOk;
                    // Laitetaan tietoa data-loading threadille ett‰ on tullut uutta dataa
                    CFmiDataLoadingThread2::LoadDataNow();
                }
            }
        }

        gMissingDataOnServerReporter.workerThreadCompletesCycle(gThreadName);
        return status;
    }

    void LogGeneralMessage(const std::string& theThreadNameStr, const std::string& theStartMessage, const std::string& theEndMessage, CatLog::Severity logLevel)
    {
        std::string logStr(theStartMessage);
        logStr += " ";
        logStr += theThreadNameStr;
        logStr += " ";
        logStr += theEndMessage;
        CatLog::logMessage(logStr, logLevel, CatLog::Category::Data);
    }

} // nameless namespace


namespace QueryDataToLocalCacheLoaderThread
{
    void InitHelpDataInfo(const NFmiHelpDataInfoSystem& helpDataInfoSystem, const std::string& smartMetBinariesDirectory, double cacheCleaningIntervalInHours, const std::string& smartMetWorkingDirectory, double maxDataFileSizeInMB)
    {
        gLocalCacheHelpDataSystem.InitHelpDataInfoSystem(helpDataInfoSystem);
        gSmartMetBinDirectory = smartMetBinariesDirectory;
        gSmartMetWorkingDirectory = smartMetWorkingDirectory;

        gCopyFileExCancel = FALSE;

        gUsedChunckSize = NFmiSettings::Optional("SmartMet::UsedChunckSizeInKB", 512) * 1024;
        gCacheCleaningIntervalInHours = cacheCleaningIntervalInHours;
        auto tmpHelpDataSystemPtr = gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr();
        gMissingDataOnServerReporter.initialize(*tmpHelpDataSystemPtr, 1);
        ::MakeCacheDirectories(*tmpHelpDataSystemPtr);
        gMaxDataFileSizeInMB = maxDataFileSizeInMB;
    }

    UINT DoThread(LPVOID pParam)
    {
        // Create lock without acquiring mutex, lock object will release mutex on any kind of exit
        std::unique_lock<std::timed_mutex> lock(gThreadRunningMutex, std::defer_lock);
        // Attempt to lock the shared resource for 2 seconds
        if(!lock.try_lock_for(std::chrono::seconds(2)))
        {
            ::LogGeneralMessage(gThreadName, "QueryDataToLocalCacheLoaderThread::DoThread with", "was allready running, stopping...", CatLog::Severity::Warning);
            return 1;
        }
        else
            ::LogGeneralMessage(gThreadName, "QueryDataToLocalCacheLoaderThread::DoThread with", "was started...", CatLog::Severity::Debug);

        bool firstTime = true;
        gDoWorkTimer.StartTimer();
        gLocalCacheCleanerTimer.StartTimer();

        // T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
        // Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
        CFmiCopyingStatus status = kFmiNoCopyNeeded;
        try
        {
            for(;;)
            {
                NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);

                if(::IsTimeToCheckForNewData(&firstTime, status))
                {
                    try
                    {
                        if(gLoadDataAtStartUp && gAutoLoadNewCacheDataMode)
                        {
                            status = GoThroughAllHelpDataInfos(gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
                        }
                    }
                    catch(NFmiStopThreadException& /* e */)
                    {
                        // SmartMet haluaa lopettaa, tullaan ulos thred funktiosta
                        return 0; 
                    }
                    catch(...)
                    {
                        // T‰m‰ oli joku 'tavallinen' virhe tilanne,
                        // jatketaan vain loopitusta.
                    }

                    // Aloitetaan taas uusi ajan lasku
                    gDoWorkTimer.StartTimer();
                }

                ::DoPossibleLocalCacheCleaning(gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());

                NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
                // Nukutaan aina lyhyit‰ aikoja (0.5 s), ett‰ osataan tutkia usein, josko p‰‰ohjelma haluaa jo sulkea
                Sleep(500); 
            }
        }
        catch(...)
        {
            // t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
        }

        ::LogGeneralMessage(gThreadName, "QueryDataToLocalCacheLoaderThread::DoThread with", "is now stopped as requested...", CatLog::Severity::Debug);
        return 0;
    }

    void CloseNow()
    {
        gStopFunctor.Stop(true);
        gCopyFileExCancel = TRUE;
    }

    bool WaitToClose(int milliSecondsToWait)
    {
        // Create lock without acquiring mutex, lock object will release mutex on any kind of exit
        std::unique_lock<std::timed_mutex> lock(gThreadRunningMutex, std::defer_lock);
        if(lock.try_lock_for(std::chrono::milliseconds(milliSecondsToWait)))
        {
            // Onnellinen lopetus saatu aikaan....
             return true; 
        }

        // Jokin oli pieless‰, ei voi mit‰‰n....
        return false; 
    }

    void UpdateSettings(NFmiHelpDataInfoSystem& theHelpDataSystem)
    {
        gLocalCacheHelpDataSystem.UpdateHelpDataInfoSystem(theHelpDataSystem);
        MakeCacheDirectories(*gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
    }

    void LoadDataAtStartUp(bool newState)
    {
        gLoadDataAtStartUp = newState;
    }

    void AutoLoadNewCacheDataMode(bool newState)
    {
        gAutoLoadNewCacheDataMode = newState;
    }

    // Kun catlog ja sen speedlog systeemit otettiin k‰yttˆˆn, ovat smartmetin
    // lokitiedostot lukossa ja niihin ei voi menn‰ ulkopuoliset loggerit lis‰‰m‰‰n mit‰‰n.
    // Siksi luodaan oma unpack lokitiedosto. Jokaiselle p‰iv‰lle oma jotta niit‰ saadaan v‰h‰n niputettua.
    std::string MakeDailyUnpackLogFilePath()
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

//    UINT DoCombinedDataHistoryThread(LPVOID pParam);

} // QueryDataToLocalCacheLoaderThread

