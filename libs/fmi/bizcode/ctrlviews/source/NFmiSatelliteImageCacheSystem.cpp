#include "NFmiSatelliteImageCacheSystem.h"
#include "NFmiSatelliteImageChannelCache.h"
#include "NFmiStringTools.h"
#include "NFmiHelpDataInfo.h"
#include "CtrlViewGdiPlusFunctions.h"
#include <thread>
#include <chrono>

namespace
{ // p�ivitys threadi funktioihin liittyvi� asetuksia

    // P��ohjelma ilmoittaa gStopWorking -lipun avulla ett� on aika sulkea Smartmet
    std::atomic<bool> gStopWorking = false;
    // Working-threadit ilmoittavat n�ill� lipuilla (gUpdateThreadRunning ja gLoadingThreadRunning) p��ohjelmalle kun ne ovat lopettaneet toimintansa
    std::atomic<bool> gUpdateThreadRunning = false;
    std::atomic<bool> gLoadingThreadRunning = false;

    // En voi laittaa NFmiSatelliteImageCacheTotal::mSatelliteImageChannelCacheList:in k�yt�n ymp�rille mutex-lukkoja, 
    // koska se olisi liian hidasta monissa tilanteissa.
    // Kun gTakeABreakInUpdating arvo on true, t�ll�in Update-funktiot eiv�t tee mit��n ty�t� ja laittavat omat lippunsa p��lle ett� ovat tauolla.
    // T�ll�ist� toimintaa tarvitaan kun ladataan CaseStudy tai kun palataan CaseStudysta takaisin normaaliin tilaan, jolloin 
    // ImageCachet on initialisoitava uudestaan.
    std::atomic<bool> gTakeABreakInUpdating = false;
    std::atomic<bool> gUpdateThreadTakingABreak = false;
    std::atomic<bool> gLoadingThreadTakingABreak = false;
}

class StopWorkingException
{
};

class GoOnBreakException
{
};

static void DoThreadStoppageCheck()
{
    if(gStopWorking)
        throw StopWorkingException();
    if(gTakeABreakInUpdating)
        throw GoOnBreakException();
}


NFmiSatelliteImageCacheSystem::NFmiSatelliteImageCacheSystem()
:mSatelliteImageChannelCacheList()
,mUpdatedCacheCallback()
,mLoadedCacheCallback()
{
}

// Alustaa vain eri satelkuvakanavat, mutta ei alusta viel� image tiedosto cacheja.
void NFmiSatelliteImageCacheSystem::Init(NFmiHelpDataInfoSystem &satelInfoSystem)
{
    Clear(); // Clear:issa on oma mutex lukitus

    int firstTimeUpdateDelayTimeInMS = 60 * 1000;
    int firstTimeLoadingWaitTimeMs = 300;
    int imageLoadingFailedWaitTimeMs = 2 * 60 * 1000;
    for(const auto &satelDataInfo : satelInfoSystem.DynamicHelpDataInfos())
    {
        if(satelDataInfo.DataType() == NFmiInfoData::kSatelData) // Otetaan tietenkin k�ytt��n vain satel tyyppiset datainfot
        {
            std::unique_ptr<NFmiSatelliteImageChannelCache> tmp(new NFmiSatelliteImageChannelCache(satelDataInfo.FileNameFilter(), satelDataInfo.ImageArea(), satelDataInfo.ImageDataIdent(), firstTimeUpdateDelayTimeInMS, firstTimeLoadingWaitTimeMs, imageLoadingFailedWaitTimeMs));
            mSatelliteImageChannelCacheList.push_back(std::move(tmp));
        }
    }
}

NFmiSatelliteImageCacheSystem::ChannelCacheItem& NFmiSatelliteImageCacheSystem::FindChannelCache(const NFmiDataIdent &wantedDataIdent)
{
    auto foundIter = std::find_if(mSatelliteImageChannelCacheList.begin(), mSatelliteImageChannelCacheList.end(), [&wantedDataIdent](const ChannelCacheList::value_type &channelCache)->bool {return channelCache->ImageDataIdent() == wantedDataIdent; });
    if(foundIter != mSatelliteImageChannelCacheList.end())
        return *foundIter;
    else
    {
        static ChannelCacheItem emptyItem;
        return emptyItem;
    }
}

NFmiImageHolder NFmiSatelliteImageCacheSystem::FindImage(const NFmiDataIdent &wantedDataIdent, const NFmiMetTime &wantedTime, int maxOffSetInMinutes)
{
    ChannelCacheItem &channelCache = FindChannelCache(wantedDataIdent);
    if(channelCache)
        return channelCache->FindImage(wantedTime, maxOffSetInMinutes);
    else
        return NFmiImageHolder();
}

NFmiMetTime NFmiSatelliteImageCacheSystem::GetLatestImageTime(const NFmiDataIdent &wantedDataIdent)
{
    ChannelCacheItem &channelCache = FindChannelCache(wantedDataIdent);
    if(channelCache)
        return channelCache->GetLatestImageTime();
    else
        return NFmiMetTime::gMissingTime;
}

boost::shared_ptr<NFmiArea> NFmiSatelliteImageCacheSystem::ImageChannelArea(const NFmiDataIdent &wantedDataIdent)
{
    ChannelCacheItem &channelCache = FindChannelCache(wantedDataIdent);
    if(channelCache)
        return channelCache->ImageArea();
    else
        return boost::shared_ptr<NFmiArea>();
}

void NFmiSatelliteImageCacheSystem::DoCacheFileChecks()
{
    for(auto &channelCacheItem : mSatelliteImageChannelCacheList)
    {
        ::DoThreadStoppageCheck();
        ImageCacheUpdateData updatedImageList = channelCacheItem->UpdateCacheList(false);
        if(updatedImageList.size())
        {
            ::DoThreadStoppageCheck();
            // Joku uusi kuva on saatu cacheen, pit�� kutsua p�ivitys callback-funktiota
            if(mUpdatedCacheCallback)
                mUpdatedCacheCallback(updatedImageList);
        }
    }
}

void NFmiSatelliteImageCacheSystem::DoLoadingStatusChecks()
{
    ImageCacheUpdateData totalLoadedImageList;
    for(auto &channelCacheItem : mSatelliteImageChannelCacheList)
    {
        ::DoThreadStoppageCheck();
        ImageCacheUpdateData loadedImageList = channelCacheItem->CheckOnCacheLoading();
        ::DoThreadStoppageCheck();
        if(loadedImageList.size())
        {
            // Lis�t��n (splice = move) t�m�n kanavan p�ivitykset totaalilistaan
            totalLoadedImageList.splice(totalLoadedImageList.end(), loadedImageList);
        }
    }

    // Kun joku/jotain kuv(i)a on saatu ladattua, pit�� kutsua p�ivitys callback-funktiota.
    // Tehd��n t�m� p�ivitys kutsu vain kerran kaikille ladatuille kuville.
    if((!totalLoadedImageList.empty()) && mLoadedCacheCallback)
        mLoadedCacheCallback(totalLoadedImageList);
}

// 'Resetoi' kaikkien kanavien kaikkien kuvien imageen liittyv�n datan, jolloin ne on ladattava uudestaan, kun ruutuja p�ivitet��n.
// Oletus, kun t�t� on kutsuttu, on liattu kaikkien n�ytt�jen kaikki ajat, joten t��lt� ei tarvitse palauttaa mit��n p�ivitys tietoa.
void NFmiSatelliteImageCacheSystem::ResetImages()
{
    for(auto &channelCacheItem : mSatelliteImageChannelCacheList)
        channelCacheItem->ResetImages();
}

// Jos kuvan lataus on jostain syyst� ep�onnistunut, t�ll� resetoidaan kyseiset kuvat uudelleen lukua varten.
// Palauttaa resetedImagesOut -parametrissa tiedot resetoiduista kucvista, jotta n�ytt�j� osataan kutsuvasta koodista p�ivitt��.
void NFmiSatelliteImageCacheSystem::ResetFailedImages(ImageCacheUpdateData &resetedImagesOut)
{
    for(auto &channelCacheItem : mSatelliteImageChannelCacheList)
        channelCacheItem->ResetFailedImages(resetedImagesOut);
}

void NFmiSatelliteImageCacheSystem::Clear()
{
    for(auto &channelCacheItem : mSatelliteImageChannelCacheList)
        channelCacheItem->Clear();
    mSatelliteImageChannelCacheList.clear();
}

void NFmiSatelliteImageCacheSystem::SetCallbacks(ImageUpdateCallbackFunction &updatedCacheCallback, ImageUpdateCallbackFunction &loadedCacheCallback)
{
    mUpdatedCacheCallback = updatedCacheCallback;
    mLoadedCacheCallback = loadedCacheCallback;
}

static void FileUpdateFunction(std::function<void(void)> workingMethod, int updateIntervalInMs, int stoppingCheckIntervalInMs, std::atomic<bool> &runningStatus, std::atomic<bool> &onBreakStatus)
{
    runningStatus = true;
    NFmiMilliSecondTimer timer;
    for(;;)
    {
        try
        {
            ::DoThreadStoppageCheck();
            if(timer.CurrentTimeDiffInMSeconds() > updateIntervalInMs)
            {
                workingMethod();
                timer.StartTimer();
            }
        }
        catch(StopWorkingException &)
        {
            break;
        }
        catch(GoOnBreakException &)
        {
            onBreakStatus = true;
        }
        catch(...)
        {
            timer.StartTimer();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(stoppingCheckIntervalInMs)); // Halutuin v�lein tarkistetaan ett� haluaako SmartMet lopettaa...
    }
    runningStatus = false;
}

void NFmiSatelliteImageCacheSystem::StartUpdateThreads(std::shared_ptr<NFmiSatelliteImageCacheSystem> &satelliteImageCacheTotal)
{
    // K�ynnist�� kaksi totalCachee liittyv�� p�ivitys threadia:
    // 1. Threadin joka tarkastelee n. minuutin v�lein, onko tullut uusia satel tiedostoja tai onko niit� poistettu levylt�.
    int updateIntervalInMs = 60 * 1000;
    int stoppingCheckIntervalInMs = 500;
    std::function<void(void)> fileCheckingMethod = std::bind(&NFmiSatelliteImageCacheSystem::DoCacheFileChecks, satelliteImageCacheTotal);
    std::thread{ ::FileUpdateFunction, fileCheckingMethod, updateIntervalInMs, stoppingCheckIntervalInMs, std::ref(gUpdateThreadRunning), std::ref(gUpdateThreadTakingABreak) }.detach();

    // 2. Threadi joka tutkii onko yht��n kuvia latauksessa ja tarkastelee onko lataus kest�nyt liikaa ja joka latauksen loputtua ilmoittaa clientille ett� lataus on valmis.
    updateIntervalInMs = 1000;
    std::function<void(void)> loadingCheckingMethod = std::bind(&NFmiSatelliteImageCacheSystem::DoLoadingStatusChecks, satelliteImageCacheTotal);
    std::thread{ ::FileUpdateFunction, loadingCheckingMethod, updateIntervalInMs, stoppingCheckIntervalInMs, std::ref(gLoadingThreadRunning), std::ref(gLoadingThreadTakingABreak) }.detach();
}

// Kun client (SmartMet) halutaan sulkea, pit�� t�t� kutsua, jotta lopetus on hallittu. 
// HUOM! t�m� ei siis j�� odottamaan ett� ty�s�ikeet lopettavat, t�m� antaa niille vain k�skyn lopettaa.
void NFmiSatelliteImageCacheSystem::StopUpdateThreads()
{
    gStopWorking = true;
}

static bool AllThreadsStopped()
{
    return !gUpdateThreadRunning && !gLoadingThreadRunning;
}

static bool WaitingToHappenFunction(int maxWaitTimeInMS, int sleepPeriodInMS, std::function<bool()> &checkFunction)
{
    int totalWaitTime = 0;
    for(;;)
    {
        if(checkFunction())
            return true; // kaikki working threadit ovat lopettaneet, tullaan ulos onnistuneesti

        std::this_thread::sleep_for(std::chrono::milliseconds(sleepPeriodInMS));
        totalWaitTime += sleepPeriodInMS;
        if(totalWaitTime > maxWaitTimeInMS)
            return false; // ollaan odotettu yli maksimi odotusajan, tullaan ulos ep�onnistuneesti
    }

    return false; // t�nne ei pit�isi menn� millloinkaan, pit�isi heitt�� poikkeus, mutta en n�e hy�ty� siin�, nyt pit�� lopettaa eik� heitell� poikkeuksia
}

// T�ll� odotellaan ett� ty�s�ikeet todella lopettavat, tai jos kest�� enemm�n 
// kuin annettu maxWaitTimeInMS, niin sitten on pakko lopettaa ep�hallitusti.
// Palauttaa true, jos alasajo oli hallittu, muuten palautuu  false.
bool NFmiSatelliteImageCacheSystem::WaitUpdateThreadsToStop(int maxWaitTimeInMS)
{
    std::function<bool()> checkFunction = ::AllThreadsStopped;
    return ::WaitingToHappenFunction(maxWaitTimeInMS, 50, checkFunction);
}

static bool AllWorkingThreadsAreOnBreak()
{
    return gUpdateThreadTakingABreak && gLoadingThreadTakingABreak;
}

bool NFmiSatelliteImageCacheSystem::WaitUpdateThreadsToTakeABreak(int maxWaitTimeInMS)
{
    gTakeABreakInUpdating = true;

    std::function<bool()> checkFunction = ::AllWorkingThreadsAreOnBreak;
    return ::WaitingToHappenFunction(maxWaitTimeInMS, 50, checkFunction);
}

void NFmiSatelliteImageCacheSystem::TellUpdateThreadsToStopTheBreak()
{
    gTakeABreakInUpdating = false;
    gUpdateThreadTakingABreak = false;
    gLoadingThreadTakingABreak = false;
}

