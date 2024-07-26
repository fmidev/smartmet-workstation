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
#include <numeric>

namespace
{
    // Jos ohjelma halutaan lopettaa ulkoapäin, tälle gStopFunctorPtr:ille asetetaan tieto siitä CloseNow funktion kautta.
    std::shared_ptr<NFmiStopFunctor> gStopFunctorPtr;
    // Jos jotain datoja ei löydy serveriltä, halutaan siitä raportoida kerran lokiin.
    // Lisäksi tämä pitää jatkuvaa kirjaa siitä kuinka monta dataa löytyy löytyy sieltä serveriltä ja
    // tämä raportoi siitä Smartmetin tarvittaessa statusbarissa olevaan "operationaaliset ongelmat" ledikontrolliin.
    std::shared_ptr<NFmiMissingDataOnServerReporter> gMissingDataOnServerReporterPtr;
    // Tämän olion avulla working thread osaa lukea/kopioida haluttuja datoja
    LocalCacheHelpDataSystem gLocalCacheHelpDataSystem;
    // Tämän avulla tiedetään, suljettaessa ohjelmaa että onko working-thread 
    // lopettanut työnsä ja sen puolesta voidaan ohjelman sulkemista jatkaa.
    std::timed_mutex gThreadRunningMutex;
    std::string gThreadName = "Main-qdata-file-loader-thread";
    // Tämän avulla kontrolloidaan sitä kuinka monta single-data-loader threadia 
    // voi olla käynnissä kerrallaan.
    LocalCacheFutureWaitingSystem gLocalCacheFutureWaitingSystem;
    // Käyttäjä voi lisätä työlistoja, millä nopeutetaan jonkin datan latauksen
    // aloittamista esim, kun smartmetia käynnistetään tauon jälkeen ja on paljon 
    // ladattavaa. Nämä työt siis ohittavat normityölistan ja annetuista datoista 
    // luetaan vain viimeisimmät datat.
    std::list<std::string> gPrioritizedDataLoadWorkList;
    std::mutex gPrioritizedDataLoadWorkListMutex;

    NFmiMilliSecondTimer gDoWorkTimer;

    // Tämä heittää erikois poikkeuksen, jos käyttäjä on halunnut sulkea ohjelman.
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
            // Jos ollaan 1. kertaa loopissa, jatketaan heti ja nollataan siihen liittyvä firstTime flag
            *firstTime = false;
            return true;
        }

        // Jos datan läpikäynnissä oli kopioitu mitään dataa, tehdään uusi kierros samantein
        if(status != kFmiNoCopyNeeded)
            return true;

        // Tarkistetaan minuutin välein onko tullut uusia datoja palvelimelle kopioitavaksi
        return gDoWorkTimer.CurrentTimeDiffInMSeconds() > (60 * 1000);
    }

    // Tehdään lokaali data cachen siivous halutulla väleillä (10 minuuttia)
    void DoPossibleLocalCacheCleaning(std::shared_ptr<NFmiHelpDataInfoSystem> theHelpDataInfoSystemPtr)
    {
        auto& usedHelpDataInfoSystem = *theHelpDataInfoSystemPtr;
        LocalCacheCleaning::DoPossibleLocalCacheCleaning(usedHelpDataInfoSystem);
    }

    // Ei missään tilanteessa haluta ladata dataa serverilta lokaali cacheen, 
    // jos se on disabloitu tai se on merkitty CaseStudy legacy dataksi.
    bool IsDataUsed(const NFmiHelpDataInfo& helpDataInfo)
    {
        return helpDataInfo.IsEnabled() && !helpDataInfo.CaseStudyLegacyOnly();
    }

    const NFmiHelpDataInfo* GetNextPossiblePrioritizedDataWork(NFmiHelpDataInfoSystem& helpDataInfoSystem)
    {
        std::lock_guard<std::mutex> lock(gPrioritizedDataLoadWorkListMutex);
        do
        {
            // Jos lista oli tyhjä, lopetetaan ikilooppi
            if(gPrioritizedDataLoadWorkList.empty())
                return nullptr;

            // Otetaan ensimmäinen filefiltteri listasta kokonaan pois
            auto prioritizedFileFilter = gPrioritizedDataLoadWorkList.front();
            gPrioritizedDataLoadWorkList.pop_front();

            // Jos filtteriä vastaava helpDataInfo löytyi, palautetaan se
            auto* helpDataInfo = helpDataInfoSystem.FindHelpDataInfo(prioritizedFileFilter);
            if(helpDataInfo)
                return helpDataInfo;
        } while(true);
    }

    const NFmiHelpDataInfo& GetNextDataWork(NFmiHelpDataInfoSystem& helpDataInfoSystem, size_t* helpInfoIndexInOut)
    {
        const auto* prioritizedHelpDataInfo = ::GetNextPossiblePrioritizedDataWork(helpDataInfoSystem);
        if(prioritizedHelpDataInfo)
        {
            std::string debugMessage = "Starting prioritized work with ";
            debugMessage += prioritizedHelpDataInfo->FileNameFilter();
            CatLog::logMessage(debugMessage, CatLog::Severity::Debug, CatLog::Category::Data, true);

            // helpInfoIndexInOut:ia ei saa kasvattaa, kun tehdään priorisoituja töitä välissä.
            return *prioritizedHelpDataInfo;
        }

        // Muuten palautetaan normityö ja juoksutus indeksiä pitää kasvattaa
        auto usedHelpDataIndex = *helpInfoIndexInOut;
        // Kasvatetaan helpInfoIndexInOut juoksutusta
        (*helpInfoIndexInOut)++;
        // Tehdään vielä tarkistus ettei mennä DynamicHelpDataInfos vector:in rajojen yli mitenkään
        const auto& helpDataInfos = helpDataInfoSystem.DynamicHelpDataInfos();
        if(usedHelpDataIndex >= helpDataInfos.size())
            throw std::runtime_error("Error in QueryDataToLocalCacheLoaderThread - GetNextDataWork: Given helpInfoIndex was out of bounds, logical error in program");

        return helpDataInfos[usedHelpDataIndex];
    }

    // Käy läpi kaikki dynaamiset helpdatat ja tekee tarvittavat cache-kopioinnit.
    // Jos ei löytynyt mitään kopioitavaa koko kierroksella, palauttaa kFmiNoCopyNeeded, joka tarkoittaa
    // että worker-threadi voi pitää taukoa.
    // Jos palauttaa kFmiCopyWentOk:n, tarkoittaa että jotain kopiointi on tapahtunut ja on 
    // syytä tehdä uusi kierros saman tien.
    CFmiCopyingStatus GoThroughAllHelpDataInfos(std::shared_ptr<NFmiHelpDataInfoSystem> theHelpDataInfoSystemPtr)
    {
        // Tänne annettu shared_ptr on tarkoituksella kopio, ja siitä otetaan käytettävä referenssi ulos tässä.
        NFmiHelpDataInfoSystem& usedHelpDataInfoSystem = *theHelpDataInfoSystemPtr;
        for(size_t helpInfoIndex = 0; helpInfoIndex < usedHelpDataInfoSystem.DynamicHelpDataInfos().size(); )
        {
            CheckIfProgramWantsToStop();
            // Huom! helpInfoIndex:ia juoksutetaan GetNextDataWork funktiossa, joten sen kutsun jälkeen 
            // siinä voi olla jo seuraavan kierroksen arvo.
            const NFmiHelpDataInfo& helpDataInfo = ::GetNextDataWork(usedHelpDataInfoSystem, &helpInfoIndex);
            if(::IsDataUsed(helpDataInfo))
            {
                gLocalCacheFutureWaitingSystem.AddFuture(std::async(std::launch::async, LocalCacheSingleFileLoaderThread::CopyQueryDataToCache, helpDataInfo, usedHelpDataInfoSystem));
            }

            // Pitää mahdollisesti odotella jos kaikki worker-threadit ovat jo käytössä
            gLocalCacheFutureWaitingSystem.WaitForFuturesToExpire();
        }

        gMissingDataOnServerReporterPtr->mainWorkerThreadCompletesCycle();
        return gLocalCacheFutureWaitingSystem.GetAndResetDataCycleStatus();
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

    // Ei tarvii tehdä mitään threadi turvallista std::call_once juttua, koska 
    // pää-working-thread kutsuu tätä normiloopissa, eli static lippu riittää.
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
        gLocalCacheFutureWaitingSystem.InitWaitingSystem(maxSingleDataCopyThreads, gStopFunctorPtr);

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

        // Tässä on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehdään yhdistelmä datoja SmartMetin luettavaksi.
        // Lisäksi pitää tarkkailla, onko tullut lopetus käsky, joloin pitää siivota ja lopettaa.
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
                        // Tämä oli joku 'tavallinen' virhe tilanne,
                        // jatketaan vain loopitusta.
                    }

                    // Tarkastellaan myös pitääkö lokaali cachehakemisto tyhjennellä
                    ::DoPossibleLocalCacheCleaning(gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
                    ::StartHistoryDataThreadOnce();
                    // Aloitetaan taas uuden kierroksen ajanlasku
                    gDoWorkTimer.StartTimer();
                }


                CheckIfProgramWantsToStop();
                // Nukutaan aina lyhyitä aikoja (0.5 s), että osataan tutkia usein, josko pääohjelma haluaa jo sulkea
                Sleep(500); 
            }
        }
        catch(...)
        {
            // tämä oli luultavasti StopThreadException, lopetetaan joka tapauksessa
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

        // Jokin oli pielessä, ei voi mitään....
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

    void AddPrioritizedDataLoadWork(const std::list<std::string>& prioritizedDataLoadWorkList)
    {
        if(prioritizedDataLoadWorkList.empty())
            return;

        std::string debugMessage = "AddPrioritizedDataLoadWork: adding new prioritized work to list: ";
        debugMessage += std::accumulate(
            std::next(prioritizedDataLoadWorkList.begin()), // Start from the second element
            prioritizedDataLoadWorkList.end(),              // End of the list
            *prioritizedDataLoadWorkList.begin(),           // Initial value (first element)
            [](const std::string& a, const std::string& b) {
                return a + ", " + b;     // Concatenate with a comma
            }
        );

        CatLog::logMessage(debugMessage, CatLog::Severity::Debug, CatLog::Category::Data, true);

        std::lock_guard<std::mutex> lock(gPrioritizedDataLoadWorkListMutex);
        gPrioritizedDataLoadWorkList.insert(gPrioritizedDataLoadWorkList.begin(), prioritizedDataLoadWorkList.begin(), prioritizedDataLoadWorkList.end());
    }

} // QueryDataToLocalCacheLoaderThread

