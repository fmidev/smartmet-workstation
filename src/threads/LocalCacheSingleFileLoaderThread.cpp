#include "stdafx.h"
#include "LocalCacheSingleFileLoaderThread.h"
#include "NFmiCachedDataFileInfo.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiMissingDataOnServerReporter.h"
#include "NFmiOnceLoadedDataFiles.h"
#include "NFmiValueString.h"
#include "NFmiLedLightStatus.h"
#include "execute-command-in-separate-process.h"
#include "FmiDataLoadingThread2.h"

namespace
{
    std::string gThreadName = "Qdata-loader-thread";
    // Jos ohjelma halutaan lopettaa ulkoap‰in, t‰lle gStopFunctorPtr:ille asetetaan tieto siit‰ CloseNow funktion kautta.
    std::shared_ptr<NFmiStopFunctor> gStopFunctorPtr;
    // Jos jotain datoja ei lˆydy serverilt‰, halutaan siit‰ raportoida eri paikkoihin, 
    // paremmat kuvaukset QueryDataToLocalCacheLoaderThread:in vastaavan muuttujan kohdalla.
    std::shared_ptr<NFmiMissingDataOnServerReporter> gMissingDataOnServerReporterPtr;
    // Tuetut pakkausformaatit
    const std::vector<std::string> g_ZippedFileExtensions{ ".7z", ".zip", ".bz2" }; //, ".gz" , ".tar" , ".xz" , ".wim" };
    // Jos tiedosto on isompi kuin t‰ss‰ oleva raja, sit‰ ei kopioida ollenkaan
    double gMaxDataFileSizeInMB;
    // T‰m‰ pit‰‰ kirjaa tiedostoista, joita on jo kertaalleen yritetty ladata, jotta voidaan
    // v‰ltt‰‰ tietynlaisten ongelmatiedostojen uudelleen ja uudelleen lataaminen.
    NFmiOnceLoadedDataFiles gOnceLoadedDataFiles;
    // T‰m‰n avulla CopyFileEx-funktio voidaan keskeytt‰‰ kun sen arvo muutetaan TRUE:ksi.
    BOOL gCopyFileExCancel = FALSE;
    // SmartMetin bin‰‰ri-hakemistoa tarvitaan ainakin kun tehd‰‰n tiedostojen purkua erillisess‰ prosessissa (purku ohjelma sijaitsee siell‰ miss‰ smartmetin exe)
    std::string gSmartMetBinDirectory;
    // SmartMetin Working-hakemistoa tarvitaan kun rakennetaan polkua 7-zip ohjelmalle (purku ohjelma sijaitsee sen utils-hakemistossa)
    std::string gSmartMetWorkingDirectory;

    // T‰m‰ heitt‰‰ erikoispoikkeuksen, jos k‰ytt‰j‰ on halunnut sulkea ohjelman.
    void CheckIfProgramWantsToStop()
    {
        NFmiQueryDataUtil::CheckIfStopped(gStopFunctorPtr.get());
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
        logStr += "' from server to local cache lasted: ";
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
        commandStr += LocalCacheSingleFileLoaderThread::MakeDailyUnpackLogFilePath();
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
        CheckIfProgramWantsToStop();
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
                    // Laitetaan tietoa data-loading threadille ett‰ on tullut uutta dataa
                    CFmiDataLoadingThread2::LoadDataNow();
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

    // 1. onko sen nimist‰ tiedostoa jo cachessa
    // 2. tee cache kopiointia varten tmp-nimi tiedostosta (joka kopioinnin j‰lkeen renametaan oikeaksi)
    // 3. onko tmp-nimi jo cachessa (t‰llˆin mahd. toisen SmartMetin kopio on jo k‰ynniss‰)
    // 4. tee varsinainen tiedosto kopio cacheen
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

} // nameless namespace ends

namespace LocalCacheSingleFileLoaderThread
{
    void InitSingleFileLoader(std::shared_ptr<NFmiStopFunctor>& stopFunctorPtr, std::shared_ptr<NFmiMissingDataOnServerReporter>& missingDataOnServerReporterPtr, double maxDataFileSizeInMB, const std::string& smartMetBinDirectory, const std::string& smartMetWorkingDirectory)
    {
        gStopFunctorPtr = stopFunctorPtr;
        gMissingDataOnServerReporterPtr = missingDataOnServerReporterPtr;
        gMaxDataFileSizeInMB = maxDataFileSizeInMB;
        gSmartMetBinDirectory = smartMetBinDirectory;
        gSmartMetWorkingDirectory = smartMetWorkingDirectory;
    }

    // Funktio tutkii annetun theDataInfo:n avulla onko kyseess‰ cacheen ladattava data
    // ja onko levypalvelimella uudempaa tiedostoa kuin paikallisessa cachessa.
    // Paluu arvot:
    // kFmiNoCopyNeeded = ei ollut mit‰‰n luettavaa
    // kFmiCopyWentOk = oli luettavaa ja se on luettu ilman ongelmia cacheen
    // kFmiCopyNotSuccessfull = oli luettavaa, mutta ei voitu kopioida tiedostoa (yksi mahd. syy on ett‰ 
    // toinen SmartMet on juuri kopioimassa sit‰), t‰m‰ tulkitaan siten ett‰ ei ollut mit‰‰n luettavaa/kopioitavaa
    CFmiCopyingStatus CopyQueryDataToCache(const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem)
    {
        try
        {
            if(NFmiCachedDataFileInfo::IsDataCached(theDataInfo))
            {
                // 1. Mik‰ on uusimman file-filterin mukaisen tiedoston nimi, ja oliko kyse pakatusta tiedostosta
                std::string fileFilter = theDataInfo.FileNameFilter();
                NFmiCachedDataFileInfo cachedDataFileInfo;
                ::GetNewestFileInfo(fileFilter, cachedDataFileInfo);
                gMissingDataOnServerReporterPtr->doReportIfFileFilterHasNoRelatedDataOnServer(cachedDataFileInfo, fileFilter);

                CheckIfProgramWantsToStop();
                if(!cachedDataFileInfo.itsTotalServerFileName.empty())
                {
                    if(::DoesThisThreadCopyFile(cachedDataFileInfo))
                    {
                        ::MakeRestOfTheFileNames(cachedDataFileInfo, theDataInfo, theHelpDataSystem);
                        return ::CopyFileToLocalCache(cachedDataFileInfo, theDataInfo);
                    }
                }
            }
        }
        catch(...)
        {
            // t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
        }

        return kFmiNoCopyNeeded;
    }

    void CloseNow()
    {
        gCopyFileExCancel = TRUE;
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

} // LocalCacheSingleFileLoaderThread namespace ends
