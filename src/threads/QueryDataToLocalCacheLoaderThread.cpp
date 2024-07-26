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
    // Jos ohjelma halutaan lopettaa ulkoap�in, t�lle gStopFunctorPtr:ille asetetaan tieto siit� CloseNow funktion kautta.
    std::shared_ptr<NFmiStopFunctor> gStopFunctorPtr;
    // Jos jotain datoja ei l�ydy serverilt�, halutaan siit� raportoida kerran lokiin.
    // Lis�ksi t�m� pit�� jatkuvaa kirjaa siit� kuinka monta dataa l�ytyy l�ytyy sielt� serverilt� ja
    // t�m� raportoi siit� Smartmetin tarvittaessa statusbarissa olevaan "operationaaliset ongelmat" ledikontrolliin.
    std::shared_ptr<NFmiMissingDataOnServerReporter> gMissingDataOnServerReporterPtr;
    // T�m�n olion avulla working thread osaa lukea/kopioida haluttuja datoja
    LocalCacheHelpDataSystem gLocalCacheHelpDataSystem;
    // T�m�n avulla tiedet��n, suljettaessa ohjelmaa ett� onko working-thread 
    // lopettanut ty�ns� ja sen puolesta voidaan ohjelman sulkemista jatkaa.
    std::timed_mutex gThreadRunningMutex;
    std::string gThreadName = "Main-qdata-file-loader-thread";
    // T�m�n avulla kontrolloidaan sit� kuinka monta single-data-loader threadia 
    // voi olla k�ynniss� kerrallaan.
    LocalCacheFutureWaitingSystem gLocalCacheFutureWaitingSystem;
    // K�ytt�j� voi lis�t� ty�listoja, mill� nopeutetaan jonkin datan latauksen
    // aloittamista esim, kun smartmetia k�ynnistet��n tauon j�lkeen ja on paljon 
    // ladattavaa. N�m� ty�t siis ohittavat normity�listan ja annetuista datoista 
    // luetaan vain viimeisimm�t datat.
    std::list<std::string> gPrioritizedDataLoadWorkList;
    std::mutex gPrioritizedDataLoadWorkListMutex;

    NFmiMilliSecondTimer gDoWorkTimer;

    // T�m� heitt�� erikois poikkeuksen, jos k�ytt�j� on halunnut sulkea ohjelman.
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
            // Jos ollaan 1. kertaa loopissa, jatketaan heti ja nollataan siihen liittyv� firstTime flag
            *firstTime = false;
            return true;
        }

        // Jos datan l�pik�ynniss� oli kopioitu mit��n dataa, tehd��n uusi kierros samantein
        if(status != kFmiNoCopyNeeded)
            return true;

        // Tarkistetaan minuutin v�lein onko tullut uusia datoja palvelimelle kopioitavaksi
        return gDoWorkTimer.CurrentTimeDiffInMSeconds() > (60 * 1000);
    }

    // Tehd��n lokaali data cachen siivous halutulla v�leill� (10 minuuttia)
    void DoPossibleLocalCacheCleaning(std::shared_ptr<NFmiHelpDataInfoSystem> theHelpDataInfoSystemPtr)
    {
        auto& usedHelpDataInfoSystem = *theHelpDataInfoSystemPtr;
        LocalCacheCleaning::DoPossibleLocalCacheCleaning(usedHelpDataInfoSystem);
    }

    // Ei miss��n tilanteessa haluta ladata dataa serverilta lokaali cacheen, 
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
            // Jos lista oli tyhj�, lopetetaan ikilooppi
            if(gPrioritizedDataLoadWorkList.empty())
                return nullptr;

            // Otetaan ensimm�inen filefiltteri listasta kokonaan pois
            auto prioritizedFileFilter = gPrioritizedDataLoadWorkList.front();
            gPrioritizedDataLoadWorkList.pop_front();

            // Jos filtteri� vastaava helpDataInfo l�ytyi, palautetaan se
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

            // helpInfoIndexInOut:ia ei saa kasvattaa, kun tehd��n priorisoituja t�it� v�liss�.
            return *prioritizedHelpDataInfo;
        }

        // Muuten palautetaan normity� ja juoksutus indeksi� pit�� kasvattaa
        auto usedHelpDataIndex = *helpInfoIndexInOut;
        // Kasvatetaan helpInfoIndexInOut juoksutusta
        (*helpInfoIndexInOut)++;
        // Tehd��n viel� tarkistus ettei menn� DynamicHelpDataInfos vector:in rajojen yli mitenk��n
        const auto& helpDataInfos = helpDataInfoSystem.DynamicHelpDataInfos();
        if(usedHelpDataIndex >= helpDataInfos.size())
            throw std::runtime_error("Error in QueryDataToLocalCacheLoaderThread - GetNextDataWork: Given helpInfoIndex was out of bounds, logical error in program");

        return helpDataInfos[usedHelpDataIndex];
    }

    // K�y l�pi kaikki dynaamiset helpdatat ja tekee tarvittavat cache-kopioinnit.
    // Jos ei l�ytynyt mit��n kopioitavaa koko kierroksella, palauttaa kFmiNoCopyNeeded, joka tarkoittaa
    // ett� worker-threadi voi pit�� taukoa.
    // Jos palauttaa kFmiCopyWentOk:n, tarkoittaa ett� jotain kopiointi on tapahtunut ja on 
    // syyt� tehd� uusi kierros saman tien.
    CFmiCopyingStatus GoThroughAllHelpDataInfos(std::shared_ptr<NFmiHelpDataInfoSystem> theHelpDataInfoSystemPtr)
    {
        // T�nne annettu shared_ptr on tarkoituksella kopio, ja siit� otetaan k�ytett�v� referenssi ulos t�ss�.
        NFmiHelpDataInfoSystem& usedHelpDataInfoSystem = *theHelpDataInfoSystemPtr;
        for(size_t helpInfoIndex = 0; helpInfoIndex < usedHelpDataInfoSystem.DynamicHelpDataInfos().size(); )
        {
            CheckIfProgramWantsToStop();
            // Huom! helpInfoIndex:ia juoksutetaan GetNextDataWork funktiossa, joten sen kutsun j�lkeen 
            // siin� voi olla jo seuraavan kierroksen arvo.
            const NFmiHelpDataInfo& helpDataInfo = ::GetNextDataWork(usedHelpDataInfoSystem, &helpInfoIndex);
            if(::IsDataUsed(helpDataInfo))
            {
                gLocalCacheFutureWaitingSystem.AddFuture(std::async(std::launch::async, LocalCacheSingleFileLoaderThread::CopyQueryDataToCache, helpDataInfo, usedHelpDataInfoSystem));
            }

            // Pit�� mahdollisesti odotella jos kaikki worker-threadit ovat jo k�yt�ss�
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

    // Ei tarvii tehd� mit��n threadi turvallista std::call_once juttua, koska 
    // p��-working-thread kutsuu t�t� normiloopissa, eli static lippu riitt��.
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

        // T�ss� on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd��n yhdistelm� datoja SmartMetin luettavaksi.
        // Lis�ksi pit�� tarkkailla, onko tullut lopetus k�sky, joloin pit�� siivota ja lopettaa.
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
                        // T�m� oli joku 'tavallinen' virhe tilanne,
                        // jatketaan vain loopitusta.
                    }

                    // Tarkastellaan my�s pit��k� lokaali cachehakemisto tyhjennell�
                    ::DoPossibleLocalCacheCleaning(gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
                    ::StartHistoryDataThreadOnce();
                    // Aloitetaan taas uuden kierroksen ajanlasku
                    gDoWorkTimer.StartTimer();
                }


                CheckIfProgramWantsToStop();
                // Nukutaan aina lyhyit� aikoja (0.5 s), ett� osataan tutkia usein, josko p��ohjelma haluaa jo sulkea
                Sleep(500); 
            }
        }
        catch(...)
        {
            // t�m� oli luultavasti StopThreadException, lopetetaan joka tapauksessa
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

        // Jokin oli pieless�, ei voi mit��n....
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

