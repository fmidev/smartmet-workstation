#include "stdafx.h"
#include "QueryDataToLocalCacheLoaderThread.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiMissingDataOnServerReporter.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFileSystem.h"
#include "LocalCacheHelpDataSystem.h"
#include "LocalCacheCleaning.h"
#include "LocalCacheSingleFileLoaderThread.h"
#include "NFmiMilliSecondTimer.h"
#include "LocalCacheFutureWaitingSystem.h"
#include "LocalCacheHistoryDataThread.h"

#include <thread>
#include <mutex>

namespace
{
    // Jos ohjelma halutaan lopettaa ulkoap‰in, t‰lle gStopFunctorPtr:ille asetetaan tieto siit‰ CloseNow funktion kautta.
    std::shared_ptr<NFmiStopFunctor> gStopFunctorPtr;
    // Jos jotain datoja ei lˆydy serverilt‰, halutaan siit‰ raportoida kerran lokiin.
    // Lis‰ksi t‰m‰ pit‰‰ jatkuvaa kirjaa siit‰ kuinka monta dataa lˆytyy lˆytyy sielt‰ serverilt‰ ja
    // t‰m‰ raportoi siit‰ Smartmetin tarvittaessa statusbarissa olevaan "operationaaliset ongelmat" ledikontrolliin.
    std::shared_ptr<NFmiMissingDataOnServerReporter> gMissingDataOnServerReporterPtr;
    // T‰m‰n olion avulla working thread osaa lukea/kopioida haluttuja datoja
    LocalCacheHelpDataSystem gLocalCacheHelpDataSystem;
    // T‰m‰n avulla tiedet‰‰n, suljettaessa ohjelmaa ett‰ onko working-thread 
    // lopettanut tyˆns‰ ja sen puolesta voidaan ohjelman sulkemista jatkaa.
    std::timed_mutex gThreadRunningMutex;
    std::string gThreadName = "Main-qdata-file-loader-thread";
    LocalCacheFutureWaitingSystem localCacheFutureWaitingSystem;

    NFmiMilliSecondTimer gDoWorkTimer;

    // T‰m‰ heitt‰‰ erikois poikkeuksen, jos k‰ytt‰j‰ on halunnut sulkea ohjelman.
    void CheckIfProgramWantsToStop()
    {
        NFmiQueryDataUtil::CheckIfStopped(gStopFunctorPtr.get());
    }

    void MakeCacheDirectories(NFmiHelpDataInfoSystem& helpDataSystem)
    {
        // Make sure that cache and tmp directories exist
        NFmiFileSystem::CreateDirectory(helpDataSystem.LocalDataLocalDirectory());
        NFmiFileSystem::CreateDirectory(helpDataSystem.LocalDataTmpDirectory());
        NFmiFileSystem::CreateDirectory(helpDataSystem.LocalDataPartialDirectory());
    }

    bool IsTimeToCheckForNewData(bool* firstTime, CFmiCopyingStatus status)
    {
        if(*firstTime)
        {
            // Jos ollaan 1. kertaa loopissa, jatketaan heti ja nollataan siihen liittyv‰ firstTime flag
            *firstTime = false;
            return true;
        }

        // Jos datan l‰pik‰ynniss‰ oli kopioitu mit‰‰n dataa, tehd‰‰n uusi kierros samantein
        if(status != kFmiNoCopyNeeded)
            return true;

        // Tarkistetaan minuutin v‰lein onko tullut uusia datoja palvelimelle kopioitavaksi
        return gDoWorkTimer.CurrentTimeDiffInMSeconds() > (60 * 1000);
    }

    // Tehd‰‰n lokaali data cachen siivous halutulla v‰leill‰ (10 minuuttia)
    void DoPossibleLocalCacheCleaning(std::shared_ptr<NFmiHelpDataInfoSystem> theHelpDataInfoSystemPtr)
    {
        auto& usedHelpDataInfoSystem = *theHelpDataInfoSystemPtr;
        LocalCacheCleaning::DoPossibleLocalCacheCleaning(usedHelpDataInfoSystem);
    }

    // Ei miss‰‰n tilanteessa haluta ladata dataa serverilta lokaali cacheen, 
    // jos se on disabloitu tai se on merkitty CaseStudy legacy dataksi.
    bool IsDataUsed(const NFmiHelpDataInfo& helpDataInfo)
    {
        return helpDataInfo.IsEnabled() && !helpDataInfo.CaseStudyLegacyOnly();
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
        const auto& helpInfos = theHelpDataSystemPtr->DynamicHelpDataInfos();
        for(size_t i = 0; i < helpInfos.size(); i++)
        {
            CheckIfProgramWantsToStop();
            const NFmiHelpDataInfo& helpDataInfo = helpInfos[i];
            if(::IsDataUsed(helpDataInfo))
            {
                localCacheFutureWaitingSystem.AddFuture(std::async(std::launch::async, LocalCacheSingleFileLoaderThread::CopyQueryDataToCache, helpDataInfo, usedHelpDataSystem));
            }

            // Pit‰‰ mahdollisesti odotella jos kaikki worker-threadit ovat jo k‰ytˆss‰
            localCacheFutureWaitingSystem.WaitForFuturesToExpire();
        }

        gMissingDataOnServerReporterPtr->mainWorkerThreadCompletesCycle();
        return localCacheFutureWaitingSystem.GetAndResetDataCycleStatus();
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

    // Ei tarvii tehd‰ mit‰‰n threadi turvallista std::call_once juttua, koska 
    // p‰‰-working-thread kutsuu t‰t‰ normiloopissa, eli static lippu riitt‰‰.
    void StartHistoryDataThreadOnce()
    {
        static bool hasRun = false;
        if(!hasRun)
        {
            hasRun = true;
            std::thread t(LocalCacheHistoryDataThread::DoHistoryThread, gStopFunctorPtr, gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
            t.detach(); // Detach the thread
        }
    }

} // nameless namespace


namespace QueryDataToLocalCacheLoaderThread
{
    void InitHelpDataInfo(const NFmiHelpDataInfoSystem& helpDataInfoSystem, const std::string& smartMetBinariesDirectory, double cacheCleaningIntervalInHours, bool loadDataAtStartUp, bool autoLoadNewCacheDataMode, const std::string& smartMetWorkingDirectory, double maxDataFileSizeInMB)
    {
        gStopFunctorPtr = std::make_shared<NFmiStopFunctor>();
        gMissingDataOnServerReporterPtr = std::make_shared<NFmiMissingDataOnServerReporter>();
        gLocalCacheHelpDataSystem.InitHelpDataInfoSystem(helpDataInfoSystem);
        auto tmpHelpDataSystemPtr = gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr();
        gMissingDataOnServerReporterPtr->initialize(*tmpHelpDataSystemPtr);
        ::MakeCacheDirectories(*tmpHelpDataSystemPtr);
        const size_t maxSingleDataCopyThreads = 3;
        localCacheFutureWaitingSystem.InitWaitingSystem(maxSingleDataCopyThreads, gStopFunctorPtr);

        LocalCacheCleaning::InitLocalCacheCleaning(loadDataAtStartUp, autoLoadNewCacheDataMode, cacheCleaningIntervalInHours, gStopFunctorPtr);
        LocalCacheSingleFileLoaderThread::InitSingleFileLoader(gStopFunctorPtr, gMissingDataOnServerReporterPtr, maxDataFileSizeInMB, smartMetBinariesDirectory, smartMetWorkingDirectory);
    }

    void DoThread()
    {
        // Create lock without acquiring mutex, lock object will release mutex on any kind of exit
        std::unique_lock<std::timed_mutex> lock(gThreadRunningMutex, std::defer_lock);
        // Attempt to lock the shared resource for 2 seconds
        if(!lock.try_lock_for(std::chrono::seconds(2)))
        {
            ::LogGeneralMessage(gThreadName, "QueryDataToLocalCacheLoaderThread::DoThread with", "was allready running, stopping...", CatLog::Severity::Warning);
            return ;
        }
        else
            ::LogGeneralMessage(gThreadName, "QueryDataToLocalCacheLoaderThread::DoThread with", "was started...", CatLog::Severity::Debug);

        bool firstTime = true;
        gDoWorkTimer.StartTimer();

        // T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
        // Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
        CFmiCopyingStatus status = kFmiNoCopyNeeded;
        try
        {
            for(;;)
            {
                CheckIfProgramWantsToStop();

                if(::IsTimeToCheckForNewData(&firstTime, status))
                {
                    try
                    {
                        if(LocalCacheCleaning::IsDataCopyingRoutinesOn())
                        {
                            status = GoThroughAllHelpDataInfos(gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
                        }
                    }
                    catch(NFmiStopThreadException& /* e */)
                    {
                        // SmartMet haluaa lopettaa, tullaan ulos thred funktiosta
                        return ; 
                    }
                    catch(...)
                    {
                        // T‰m‰ oli joku 'tavallinen' virhe tilanne,
                        // jatketaan vain loopitusta.
                    }

                    // Tarkastellaan myˆs pit‰‰kˆ lokaali cachehakemisto tyhjennell‰
                    ::DoPossibleLocalCacheCleaning(gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
                    ::StartHistoryDataThreadOnce();
                    // Aloitetaan taas uuden kierroksen ajanlasku
                    gDoWorkTimer.StartTimer();
                }


                CheckIfProgramWantsToStop();
                // Nukutaan aina lyhyit‰ aikoja (0.5 s), ett‰ osataan tutkia usein, josko p‰‰ohjelma haluaa jo sulkea
                Sleep(500); 
            }
        }
        catch(...)
        {
            // t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
        }

        ::LogGeneralMessage(gThreadName, "QueryDataToLocalCacheLoaderThread::DoThread with", "is now stopped as requested...", CatLog::Severity::Debug);
    }

    void CloseNow()
    {
        gStopFunctorPtr->Stop(true);
        LocalCacheSingleFileLoaderThread::CloseNow();
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

    void UpdateSettings(NFmiHelpDataInfoSystem& theHelpDataSystem, bool loadDataAtStartUp, bool autoLoadNewCacheDataMode)
    {
        gLocalCacheHelpDataSystem.UpdateHelpDataInfoSystem(theHelpDataSystem);
        LocalCacheCleaning::LoadDataAtStartUp(loadDataAtStartUp);
        LocalCacheCleaning::AutoLoadNewCacheDataMode(autoLoadNewCacheDataMode);
        MakeCacheDirectories(*gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
    }

    void AutoLoadNewCacheDataMode(bool newValue)
    {
        LocalCacheCleaning::AutoLoadNewCacheDataMode(newValue);
    }

//    UINT DoCombinedDataHistoryThread(LPVOID pParam);

} // QueryDataToLocalCacheLoaderThread

