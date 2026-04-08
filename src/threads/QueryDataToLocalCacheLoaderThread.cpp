#ifndef UNIX
#include "stdafx.h"
#endif // UNIX
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
#include "NFmiInfoDataCompat.h"

#include <thread>
#include <mutex>
#include <numeric>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace
{
    enum class ThreadDataType
    {
        LatestData,
        OldData,
        PrioritizedData,
        HistoryData
    };

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
    // Vanhojen datojen lataus ty�lista
    std::list<std::string> gLoadOldDataWorkList;
    // T�m� mutex on molemmille ty�listoille
    std::mutex gWorkDataLoadMutex;
    // Lippu sille ett� uutta work listaa on tullut, eik� kannata odotella
    // normity�-timerin kanssa kokonaista minuuttia odotusloopissa.
    bool gNewDataLoadingWorkReceived = false;
    // Eri working-threadeille pit�� saada uniikki nimi, jotta Smartmetin ledi-indikaattori systeemi
    // osaisi raportoida kaikki erilliset k�ynniss� olevat jutut sen tooltipissa.
    // Nyt on kolme erityyppist� datanhakua, ja niille on erilliset laskurit, joista
    // tuotetaan lopuksi nimet tiettyjen s��nt�jen mukaan.
    size_t gThreadNameIndexForLatestData = 1;
    size_t gThreadNameIndexForPrioritizedData = 1;
    size_t gThreadNameIndexForOldData = 1;

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

    std::string GetThreadName(ThreadDataType dataType)
    {
        switch(dataType)
        {
        case ThreadDataType::LatestData:
            return std::string("LatestData-") + std::to_string(gThreadNameIndexForLatestData++);
        case ThreadDataType::OldData:
            return std::string("OldData-") + std::to_string(gThreadNameIndexForOldData++);
        case ThreadDataType::PrioritizedData:
            return std::string("PrioritizedData-") + std::to_string(gThreadNameIndexForPrioritizedData++);
        default:
        {
            CatLog::logMessage("Unknown data type in GetThreadName, logical error in program", CatLog::Severity::Error, CatLog::Category::Data, true);
            return std::string("UnknownData-1");
        }
        }
    }

    bool IsTimeToCheckForNewData(bool* firstTime, CFmiCopyingStatus status)
    {
        if(*firstTime)
        {
            // Jos ollaan 1. kertaa loopissa, jatketaan heti ja nollataan siihen liittyv� firstTime flag
            *firstTime = false;
            return true;
        }

        if(gNewDataLoadingWorkReceived)
        {
            gNewDataLoadingWorkReceived = false;
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

    const NFmiHelpDataInfo* GetNextPossibleDataWork(NFmiHelpDataInfoSystem& helpDataInfoSystem, std::list<std::string> &workList, bool oldDataCase)
    {
        std::lock_guard<std::mutex> lock(gWorkDataLoadMutex);
        do
        {
            // Jos lista oli tyhj�, lopetetaan ikilooppi
            if(workList.empty())
                return nullptr;

            // Otetaan ensimm�inen filefiltteri listasta kokonaan pois
            auto prioritizedFileFilter = workList.front();
            workList.pop_front();

            // Jos filtteri� vastaava helpDataInfo l�ytyi, palautetaan se
            auto* helpDataInfo = helpDataInfoSystem.FindHelpDataInfo(prioritizedFileFilter);
            if(helpDataInfo)
            {
                if(!oldDataCase)
                {
                    // Jos kyse priorisoidusta datasta, ei ole muita tarkastuksia
                    return helpDataInfo;
                }
                if(oldDataCase && NFmiInfoData::IsModelRunBasedData(helpDataInfo->DataType()))
                {
                    // Old data tapauksessa, datan pit�� olla malliajo tyyppist�
                    return helpDataInfo;
                }
            }
        } while(true);
    }

    const NFmiHelpDataInfo* GetNextPossiblePrioritizedDataWork(NFmiHelpDataInfoSystem& helpDataInfoSystem)
    {
        return ::GetNextPossibleDataWork(helpDataInfoSystem, gPrioritizedDataLoadWorkList, false);
    }

    const NFmiHelpDataInfo* GetNextPossibleLoadOldDataWork(NFmiHelpDataInfoSystem& helpDataInfoSystem)
    {
        return ::GetNextPossibleDataWork(helpDataInfoSystem, gLoadOldDataWorkList, true);
    }

    const NFmiHelpDataInfo* GetNextPossibleWorkWithLogging(NFmiHelpDataInfoSystem& helpDataInfoSystem, bool doLoadOldData, const std::string& baseLogMessage)
    {
        const auto* workHelpDataInfo = doLoadOldData ? ::GetNextPossibleLoadOldDataWork(helpDataInfoSystem) : ::GetNextPossiblePrioritizedDataWork(helpDataInfoSystem);
        if(workHelpDataInfo)
        {
            std::string debugMessage = baseLogMessage;
            debugMessage += workHelpDataInfo->FileNameFilter();
            CatLog::logMessage(debugMessage, CatLog::Severity::Debug, CatLog::Category::Data, true);
        }
        return workHelpDataInfo;
    }

    // Palautetaan kopiointi ty�h�n liittyv� HelpDataInfo ja tieto mink� tyyppisest� datan hausta on kyse
    std::pair<const NFmiHelpDataInfo*, ThreadDataType> GetNextDataWork(NFmiHelpDataInfoSystem& helpDataInfoSystem, size_t* helpInfoIndexInOut)
    {
        const auto* prioritizedHelpDataInfo = ::GetNextPossibleWorkWithLogging(helpDataInfoSystem, false, "Starting prioritized work with ");
        if(prioritizedHelpDataInfo)
        {
            // Huom: ei kasvateta helpInfoIndexInOut:ia!
            gNewDataLoadingWorkReceived = false; // nollataan ty�saatu lippu my�s
            return std::make_pair(prioritizedHelpDataInfo, ThreadDataType::PrioritizedData);
        }

        const auto* loadOldDataHelpDataInfo = ::GetNextPossibleWorkWithLogging(helpDataInfoSystem, true, "Starting load-old-data work with ");
        if(loadOldDataHelpDataInfo)
        {
            // Huom: ei kasvateta helpInfoIndexInOut:ia!
            gNewDataLoadingWorkReceived = false; // nollataan ty�saatu lippu my�s
            return std::make_pair(loadOldDataHelpDataInfo, ThreadDataType::OldData);
        }

        // Muuten palautetaan normity� ja juoksutus indeksi� pit�� kasvattaa
        auto usedHelpDataIndex = (*helpInfoIndexInOut)++;
        // Tehd��n viel� tarkistus ettei menn� DynamicHelpDataInfos vector:in rajojen yli mitenk��n
        const auto& helpDataInfos = helpDataInfoSystem.DynamicHelpDataInfos();
        if(usedHelpDataIndex >= helpDataInfos.size())
            throw std::runtime_error("Error in QueryDataToLocalCacheLoaderThread - GetNextDataWork: Given helpInfoIndex was out of bounds, logical error in program");

        return std::make_pair(&helpDataInfos[usedHelpDataIndex], ThreadDataType::LatestData);
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
            auto helpDataInfoOldDataPair = ::GetNextDataWork(usedHelpDataInfoSystem, &helpInfoIndex);
            if(::IsDataUsed(*helpDataInfoOldDataPair.first))
            {
                try
                {
                    auto doOldDataCase = (helpDataInfoOldDataPair.second == ThreadDataType::OldData);
                    auto usedThreadName = ::GetThreadName(helpDataInfoOldDataPair.second);
                    if(doOldDataCase)
                        gLocalCacheFutureWaitingSystem.AddFuture(std::async(std::launch::async, LocalCacheHistoryDataThread::CollectOldModelRunDataToCache, *helpDataInfoOldDataPair.first, theHelpDataInfoSystemPtr, usedThreadName));
                    else
                        gLocalCacheFutureWaitingSystem.AddFuture(std::async(std::launch::async, LocalCacheSingleFileLoaderThread::CopyQueryDataToCache, *helpDataInfoOldDataPair.first, theHelpDataInfoSystemPtr, usedThreadName));
                }
                catch(std::exception& e)
                {
                    std::string errorMessage = "Error in GoThroughAllHelpDataInfos: ";
                    errorMessage += e.what();
                    CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Data, true);
                }
            }

            // Pit�� mahdollisesti odotella jos kaikki worker-threadit ovat jo k�yt�ss�
            gLocalCacheFutureWaitingSystem.WaitForFuturesToExpire();
        }

        // T�ss� lopussa pit�� odotella ett� kaikki ty�t tulevat tehdyiksi, eli tehd��n
        // yksi kokonainen kierros ja mahdolliset erilliset old-data/history-data ty�t 
        // j�mptisti loppuun, ennen kuin rynn�t��n seuraavalle kierrokselle.
        gLocalCacheFutureWaitingSystem.WaitForAllFuturesToExpire();
        gMissingDataOnServerReporterPtr->mainWorkerThreadCompletesCycle();
        return gLocalCacheFutureWaitingSystem.GetAndResetDataCycleStatus();
    }

    std::string JoinStringList(const std::list<std::string>& stringList)
    {
        return std::accumulate( std::next(stringList.begin()), // Start from the second element
            stringList.end(),              // End of the list
            *stringList.begin(),           // Initial value (first element)
            [](const std::string& a, const std::string& b) {
                return a + ", " + b;     // Concatenate with a comma
            }
        );
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
            if(LocalCacheCleaning::IsDataCopyingRoutinesOn())
            {
                std::thread t(LocalCacheHistoryDataThread::DoHistoryThread, std::ref(gStopFunctorPtr), gLocalCacheHelpDataSystem.GetHelpDataInfoSystemPtr());
                t.detach(); // Detach the thread
            }
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
        const size_t maxSingleDataCopyThreads = 2;
        gLocalCacheFutureWaitingSystem.InitWaitingSystem(maxSingleDataCopyThreads, gStopFunctorPtr);

        LocalCacheCleaning::InitLocalCacheCleaning(loadDataAtStartUp, autoLoadNewCacheDataMode, cacheCleaningIntervalInHours, gStopFunctorPtr, helpDataInfoSystem);
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
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
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

        std::string debugMessage = "AddPrioritizedDataLoadWork: adding new prioritized-data-loading work to list: ";
        debugMessage += ::JoinStringList(prioritizedDataLoadWorkList);
        CatLog::logMessage(debugMessage, CatLog::Severity::Debug, CatLog::Category::Data, true);

        {
            std::lock_guard<std::mutex> lock(gWorkDataLoadMutex);
            gPrioritizedDataLoadWorkList.insert(gPrioritizedDataLoadWorkList.begin(), prioritizedDataLoadWorkList.begin(), prioritizedDataLoadWorkList.end());
        }
        gNewDataLoadingWorkReceived = true;
    }

    void AddLoadOldDataWork(const std::list<std::string>& loadOldDataWorkList)
    {
        if(loadOldDataWorkList.empty())
            return;

        std::string debugMessage = "AddLoadOldDataWork: adding new load-old-model-run-data work to list: ";
        debugMessage += ::JoinStringList(loadOldDataWorkList);
        CatLog::logMessage(debugMessage, CatLog::Severity::Debug, CatLog::Category::Data, true);

        {
            std::lock_guard<std::mutex> lock(gWorkDataLoadMutex);
            gLoadOldDataWorkList.insert(gLoadOldDataWorkList.begin(), loadOldDataWorkList.begin(), loadOldDataWorkList.end());
        }
        gNewDataLoadingWorkReceived = true;
    }

} // QueryDataToLocalCacheLoaderThread

