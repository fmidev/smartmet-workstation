#include "NFmiSatelliteImageCacheSystem.h"
#include "NFmiSatelliteImageChannelCache.h"
#include "NFmiStringTools.h"
#include "NFmiHelpDataInfo.h"
#include "CtrlViewGdiPlusFunctions.h"

namespace
{ // päivitys threadi funktioihin liittyviä asetuksia

    // Pääohjelma ilmoittaa gStopWorking -lipun avulla että on aika sulkea Smartmet
    std::atomic<bool> gStopWorking = false;
    // Working-threadit ilmoittavat näillä lipuilla (gUpdateThreadRunning ja gLoadingThreadRunning) pääohjelmalle kun ne ovat lopettaneet toimintansa
    std::atomic<bool> gUpdateThreadRunning = false;
    std::atomic<bool> gLoadingThreadRunning = false;

    // En voi laittaa NFmiSatelliteImageCacheTotal::mSatelliteImageChannelCacheList:in käytön ympärille mutex-lukkoja, 
    // koska se olisi liian hidasta monissa tilanteissa.
    // Kun gTakeABreakInUpdating arvo on true, tällöin Update-funktiot eivät tee mitään työtä ja laittavat omat lippunsa päälle että ovat tauolla.
    // Tälläistä toimintaa tarvitaan kun ladataan CaseStudy tai kun palataan CaseStudysta takaisin normaaliin tilaan, jolloin 
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

// Alustaa vain eri satelkuvakanavat, mutta ei alusta vielä image tiedosto cacheja.
void NFmiSatelliteImageCacheSystem::Init(NFmiHelpDataInfoSystem &satelInfoSystem)
{
    Clear(); // Clear:issa on oma mutex lukitus

    int firstTimeUpdateDelayTimeInMS = 60 * 1000;
    int firstTimeLoadingWaitTimeMs = 300;
    int imageLoadingFailedWaitTimeMs = 2 * 60 * 1000;
    for(const auto &satelDataInfo : satelInfoSystem.DynamicHelpDataInfos())
    {
        if(satelDataInfo.DataType() == NFmiInfoData::kSatelData) // Otetaan tietenkin käyttöön vain satel tyyppiset datainfot
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
            // Joku uusi kuva on saatu cacheen, pitää kutsua päivitys callback-funktiota
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
            // Lisätään (splice = move) tämän kanavan päivitykset totaalilistaan
            totalLoadedImageList.splice(totalLoadedImageList.end(), loadedImageList);
        }
    }

    // Kun joku/jotain kuv(i)a on saatu ladattua, pitää kutsua päivitys callback-funktiota.
    // Tehdään tämä päivitys kutsu vain kerran kaikille ladatuille kuville.
    if((!totalLoadedImageList.empty()) && mLoadedCacheCallback)
        mLoadedCacheCallback(totalLoadedImageList);
}

// 'Resetoi' kaikkien kanavien kaikkien kuvien imageen liittyvän datan, jolloin ne on ladattava uudestaan, kun ruutuja päivitetään.
// Oletus, kun tätä on kutsuttu, on liattu kaikkien näyttöjen kaikki ajat, joten täältä ei tarvitse palauttaa mitään päivitys tietoa.
void NFmiSatelliteImageCacheSystem::ResetImages()
{
    for(auto &channelCacheItem : mSatelliteImageChannelCacheList)
        channelCacheItem->ResetImages();
}

// Jos kuvan lataus on jostain syystä epäonnistunut, tällä resetoidaan kyseiset kuvat uudelleen lukua varten.
// Palauttaa resetedImagesOut -parametrissa tiedot resetoiduista kucvista, jotta näyttöjä osataan kutsuvasta koodista päivittää.
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
        std::this_thread::sleep_for(std::chrono::milliseconds(stoppingCheckIntervalInMs)); // Halutuin välein tarkistetaan että haluaako SmartMet lopettaa...
    }
    runningStatus = false;
}

void NFmiSatelliteImageCacheSystem::StartUpdateThreads(std::shared_ptr<NFmiSatelliteImageCacheSystem> &satelliteImageCacheTotal)
{
    // Käynnistää kaksi totalCachee liittyvää päivitys threadia:
    // 1. Threadin joka tarkastelee n. minuutin välein, onko tullut uusia satel tiedostoja tai onko niitä poistettu levyltä.
    int updateIntervalInMs = 60 * 1000;
    int stoppingCheckIntervalInMs = 500;
    std::function<void(void)> fileCheckingMethod = std::bind(&NFmiSatelliteImageCacheSystem::DoCacheFileChecks, satelliteImageCacheTotal);
    std::thread{ ::FileUpdateFunction, fileCheckingMethod, updateIntervalInMs, stoppingCheckIntervalInMs, std::ref(gUpdateThreadRunning), std::ref(gUpdateThreadTakingABreak) }.detach();

    // 2. Threadi joka tutkii onko yhtään kuvia latauksessa ja tarkastelee onko lataus kestänyt liikaa ja joka latauksen loputtua ilmoittaa clientille että lataus on valmis.
    updateIntervalInMs = 1000;
    std::function<void(void)> loadingCheckingMethod = std::bind(&NFmiSatelliteImageCacheSystem::DoLoadingStatusChecks, satelliteImageCacheTotal);
    std::thread{ ::FileUpdateFunction, loadingCheckingMethod, updateIntervalInMs, stoppingCheckIntervalInMs, std::ref(gLoadingThreadRunning), std::ref(gLoadingThreadTakingABreak) }.detach();
}

// Kun client (SmartMet) halutaan sulkea, pitää tätä kutsua, jotta lopetus on hallittu. 
// HUOM! tämä ei siis jää odottamaan että työsäikeet lopettavat, tämä antaa niille vain käskyn lopettaa.
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
            return false; // ollaan odotettu yli maksimi odotusajan, tullaan ulos epäonnistuneesti
    }

    return false; // tänne ei pitäisi mennä millloinkaan, pitäisi heittää poikkeus, mutta en näe hyötyä siinä, nyt pitää lopettaa eikä heitellä poikkeuksia
}

// Tällä odotellaan että työsäikeet todella lopettavat, tai jos kestää enemmän 
// kuin annettu maxWaitTimeInMS, niin sitten on pakko lopettaa epähallitusti.
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

