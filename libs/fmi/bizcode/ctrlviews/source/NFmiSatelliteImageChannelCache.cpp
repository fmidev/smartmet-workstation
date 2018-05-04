#include "NFmiSatelliteImageChannelCache.h"
#include "NFmiSatelliteImageCache.h"
#include "NFmiStringTools.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFileSystem.h"
#include "CtrlViewGdiPlusFunctions.h"


NFmiSatelliteImageChannelCache::NFmiSatelliteImageChannelCache(const std::string &fileFilter, const boost::shared_ptr<NFmiArea> &imageArea, const NFmiDataIdent &imageDataIdent, int firstTimeUpdateDelayTimeInMS, int firstTimeLoadingWaitTimeMs, int imageLoadingFailedWaitTimeMs)
:mFileFilter(fileFilter)
,mImageDataIdent(imageDataIdent)
,mChannelShownOnView(false)
,mFilesCheckedTimer()
,mFirstTimeUpdateDelayTimeInMS(firstTimeUpdateDelayTimeInMS)
,mImageCacheSortedList()
,mCacheInitialized(false)
,mImageCacheSetMutex()
,mFirstTimeLoadingWaitTimeMs(firstTimeLoadingWaitTimeMs)
,mImageLoadingFailedWaitTimeMs(imageLoadingFailedWaitTimeMs)
,mImageArea(imageArea)
{
}

std::list<std::string> NFmiSatelliteImageChannelCache::GetFileList(const ImageCacheSortedList &imageCacheSortedList)
{
    std::list<std::string> fileList;
    for(const auto &imageCache : imageCacheSortedList)
        fileList.push_back(imageCache->ImageFileName());
    return fileList;
}

// Lis‰t‰‰n annetun file-listan kuvat cacheen.
// Jos cachen koko muuttui eli cache on muuttunut, palautetaan true, muuten false;
// Lista pit‰‰ sortata lopuksi.
// HUOM! NoLock metodin nimess‰ viittaa siihen ett‰ mImageCacheSet:ia ei lukita t‰‰ll‰ mutexin avulla, koska se on jo lukittuna t‰t‰ kutsuvassa funktiossa.
bool NFmiSatelliteImageChannelCache::InsertToImageCache_NoLock(const std::list<std::string> &fileListIn, ImageCacheUpdateData &updatedImagesOut)
{
    size_t origsize = mImageCacheSortedList.size();
    for(auto fileName : fileListIn)
    {
        try
        {
            mImageCacheSortedList.push_back(std::make_shared<NFmiSatelliteImageCache>(fileName, mFirstTimeLoadingWaitTimeMs, mImageLoadingFailedWaitTimeMs));
            updatedImagesOut.push_back(std::make_pair(mImageDataIdent, (*mImageCacheSortedList.rbegin())->ImageTime())); // NFmiSatelliteImageCache:n konstruktorissa laskettu aika otetaan update-dataan, HUOM! rbegin eli listan viimeinen itemi.
        }
        catch(...)
        { // jos annetun tiedoston nimess‰ ei ollut oikeanlaista aikaleimaa, poikkeus lent‰‰ NFmiSatelliteImageCache -luokan konstruktorista
        }
    }

    return origsize != mImageCacheSortedList.size();
}

// Poistetaan annetun file-listan kuvat cachesta.
// Jos cachen koko muuttui eli cache on muuttunut, palautetaan true, muuten false.
// Lista pit‰‰ sortata lopuksi.
// HUOM! NoLock metodin nimess‰ viittaa siihen ett‰ mImageCacheSet:ia ei lukita t‰‰ll‰ mutexin avulla, koska se on jo lukittuna t‰t‰ kutsuvassa funktiossa.
bool NFmiSatelliteImageChannelCache::RemoveFromImageCache_NoLock(const std::list<std::string> &fileListIn, ImageCacheUpdateData &updatedImagesOut)
{
    size_t origsize = mImageCacheSortedList.size();
    for(auto fileName : fileListIn)
    {
        auto foundIter = std::find_if(mImageCacheSortedList.begin(), mImageCacheSortedList.end(), [&fileName](const NFmiSatelliteImageChannelCache::ImageCacheSortedList::value_type &imageCache)->bool {return imageCache->ImageFileName() == fileName; });
        if(foundIter != mImageCacheSortedList.end())
        {
            updatedImagesOut.push_back(std::make_pair(mImageDataIdent, (*foundIter)->ImageTime()));
            mImageCacheSortedList.erase(foundIter);
        }
    }

    return origsize != mImageCacheSortedList.size();
}

// Hakee annetun fileFilterin avulla listan tiedostoja ja tekee niist‰ listan, miss‰
// on tiedostot polkuineen kaikkineen.
std::list<std::string> NFmiSatelliteImageChannelCache::GetFileList(const std::string &fileFilter)
{
    std::list<std::string> fileNameList = NFmiFileSystem::PatternFiles(fileFilter);
    std::string path = NFmiFileSystem::PathFromPattern(fileFilter);
    std::list<std::string> filePathList;
    for(const auto &fileName : fileNameList)
        filePathList.push_back(path + fileName);
    return filePathList;
}

// T‰t‰ funktiota voidaan kutsua joko piirto s‰ikeest‰ tai taustalla 
// pyˆriv‰st‰ p‰ivitys worker-threadista, siksi mImageCacheSet pit‰‰ synkronisoida.
// Palauttaa true, jos on tullut uusia kuvia k‰yttˆˆn, tai kuvia on poistunut.
// Jos listoihin tulee muutoksia, niist‰ tehd‰‰n listaa updatedImagesOut -listaan, ja lista annetaan clientille, jotta osataan p‰ivitt‰‰ ruutuja oikein.
ImageCacheUpdateData NFmiSatelliteImageChannelCache::UpdateCacheList(bool forceUpdate)
{
    ImageCacheUpdateData updatedImages;

    // 0. Ei tehd‰ tiedostolista p‰ivityst‰, ellei kyse ole pakotetusta p‰ivityksest‰, tai jos alustuksesta on kulunut tarpeeksi aikaa
    // Tarkoituksena on est‰‰ liiallinen trafiikki esim. serverille heti SmartMetin k‰ynnistyksen yhteydess‰
    if(forceUpdate || mCacheInitialized || mFilesCheckedTimer.CurrentTimeDiffInMSeconds() >= mFirstTimeUpdateDelayTimeInMS)
    {
        // 1. Hae tiedosto lista mFileFilter:ill‰ (t‰ss‰ voi kest‰‰ kauan, siksi laitoin sen mutex-lukon ulkopuolelle)
        std::list<std::string> fileListA = NFmiSatelliteImageChannelCache::GetFileList(mFileFilter);
        fileListA.sort(); // pakko sortata, ett‰ std::set_difference ja vastaavat toimivat!!

        std::lock_guard<std::mutex> lock(mImageCacheSetMutex);
        try
        {
            // 2. Listan avulla p‰ivit‰ mImageCacheSet:ia (vain lis‰‰ uudet ja poista h‰vinneet tiedostot toiminnot)
            if(fileListA.empty())
            {
                // Jos kaikki image tiedostot ovat yht‰kki‰ vain h‰vinneet, ei tyhjennet‰ cachea ainakaan viel‰, eli ei tehd‰ nyt mit‰‰n.
                // Kyse saattaa olla vaikka hetkellisest‰ yhteysongelmasta serveriin
            }
            else if(!fileListA.empty() && mImageCacheSortedList.empty())
                InsertToImageCache_NoLock(fileListA, updatedImages); // tiedostonimi-listassa oli jotain, mutta cache on tyhj‰, t‰yt‰ cache listan arvoilla
            else
            {
                std::list<std::string> fileListB = NFmiSatelliteImageChannelCache::GetFileList(mImageCacheSortedList);
                fileListB.sort(); // t‰m‰n olisi pit‰nyt olla jo j‰rjestyksess‰, mutta jostain syyst‰ n‰in ei aina ole, en tied‰ miksi, siksi pakko sortata

                // Jos tiedostonnimi on A:ssa mutta ei B:ss‰, lis‰‰ mImageCacheSet:iin
                std::list<std::string> result_A_minus_B;
                std::set_difference(fileListA.begin(), fileListA.end(), fileListB.begin(), fileListB.end(), std::inserter(result_A_minus_B, result_A_minus_B.begin()));
                InsertToImageCache_NoLock(result_A_minus_B, updatedImages);

                // Jos on B:ss‰ mutta ei A:ssa, poista mImageCacheSet:ist‰
                std::list<std::string> result_B_minus_A;
                std::set_difference(fileListB.begin(), fileListB.end(), fileListA.begin(), fileListA.end(), std::inserter(result_B_minus_A, result_B_minus_A.begin()));
                RemoveFromImageCache_NoLock(result_B_minus_A, updatedImages);

                // Jos on sek‰ A:ssa ett‰ B:ss‰ ‰l‰ tee mit‰‰n
            }
        }
        catch(...)
        {
        }
        mCacheInitialized = true;
        mFilesCheckedTimer.StartTimer();
        auto sortPredicate = [](const std::shared_ptr<NFmiSatelliteImageCache> &itemPtr1, const std::shared_ptr<NFmiSatelliteImageCache> &itemPtr2){return *itemPtr1 < *itemPtr2; };
        mImageCacheSortedList.sort(sortPredicate);
#ifdef ENABLE_LOGGING
        ::LogImageCacheTimes(mImageCacheSortedList, "UpdateCacheList-end");
#endif
    }
    return updatedImages;
}

// Katsoo onko t‰ll‰ channelilla yht‰‰n imagea loading-tilassa. 
// Jos on, tarkistaa tilanteen ett‰ onko jokin niist‰ valmistunut.
// Jos joku loading on valmistunut, lis‰t‰‰n sen imagen tiedot loadedImages -listaan, joka palautetaan funktiosta.
ImageCacheUpdateData NFmiSatelliteImageChannelCache::CheckOnCacheLoading()
{
    ImageCacheUpdateData loadedImages;
    std::lock_guard<std::mutex> lock(mImageCacheSetMutex);
    for(auto &imageCache : mImageCacheSortedList)
    {
        if(imageCache->CheckOnImageLoading(0))
            loadedImages.push_back(std::make_pair(mImageDataIdent, imageCache->ImageTime()));
    }

    return loadedImages;
}

void NFmiSatelliteImageChannelCache::DoPossibleForceUpdate()
{
    if(!mCacheInitialized)
        UpdateCacheList(true); // Jos ei ole kertaakaan alustettu cache-listaa, pit‰‰ se tehd‰ t‰ss‰. 
}

NFmiSatelliteImageChannelCache::ImageCacheItem NFmiSatelliteImageChannelCache::FindImageCache(const NFmiMetTime &wantedTime, int maxOffSetInMinutes)
{
    DoPossibleForceUpdate(); // HUOM! Pit‰‰ olla ennen scoped_lock:ia, koska DoPossibleForceUpdate:n sis‰ll‰ tapahtuu myˆs sama lukitus!!

    std::lock_guard<std::mutex> lock(mImageCacheSetMutex);
    auto foundIter = std::find_if(mImageCacheSortedList.begin(), mImageCacheSortedList.end(), [&wantedTime](const ImageCacheSortedList::value_type &imageCache)->bool {return imageCache->ImageTime() == wantedTime; });
    if(foundIter != mImageCacheSortedList.end())
        return *foundIter;
    else
    {   // Ei lˆytynyt tarkkaa aikaa, katsotaan viel‰ onko k‰ytˆss‰ aikahaarukka ja etsit‰‰n t‰llˆin l‰hint‰ aikaa
        if(maxOffSetInMinutes > 0)
        {
            long minOffset = 999999999;
            auto foundIter = mImageCacheSortedList.end();
            for(auto iter = mImageCacheSortedList.begin(); iter != mImageCacheSortedList.end(); ++iter)
            {
                long currentOffset = std::abs(wantedTime.DifferenceInMinutes((*iter)->ImageTime()));
                if(currentOffset < minOffset)
                {
                    minOffset = currentOffset;
                    foundIter = iter;
                }
            }
            if(foundIter != mImageCacheSortedList.end() && minOffset <= maxOffSetInMinutes)
                return *foundIter;
        }

        return ImageCacheItem();
    }
}

NFmiImageHolder NFmiSatelliteImageChannelCache::FindImage(const NFmiMetTime &wantedTime, int maxOffSetInMinutes)
{
    ImageCacheItem item = FindImageCache(wantedTime, maxOffSetInMinutes);
    if(item)
        return item->Image();
    else
        return NFmiImageHolder();
}

NFmiMetTime NFmiSatelliteImageChannelCache::GetLatestImageTime()
{
    DoPossibleForceUpdate(); // HUOM! Pit‰‰ olla ennen scoped_lock:ia, koska DoPossibleForceUpdate:n sis‰ll‰ tapahtuu myˆs sama lukitus!!

    std::lock_guard<std::mutex> lock(mImageCacheSetMutex);
    if(mImageCacheSortedList.size())
        return (*mImageCacheSortedList.rbegin())->ImageTime(); // rbegin palauttaa set:in viimeisen (= viimeisin aika) itemin iteraattorin
    else
        return NFmiMetTime::gMissingTime;
}

void NFmiSatelliteImageChannelCache::ResetImages()
{
    std::lock_guard<std::mutex> lock(mImageCacheSetMutex);
    for(auto &imageCache : mImageCacheSortedList)
        imageCache->ResetImage();
}

void NFmiSatelliteImageChannelCache::ResetFailedImages(ImageCacheUpdateData &resetedImagesOut)
{
    std::lock_guard<std::mutex> lock(mImageCacheSetMutex);
    for(auto &imageCache : mImageCacheSortedList)
    {
        if(imageCache->ImageState() == NFmiImageData::kErrorneus || imageCache->ImageState() == NFmiImageData::kErrorLoadingTookTooLong)
        {
            resetedImagesOut.push_back(std::make_pair(mImageDataIdent, imageCache->ImageTime()));
            imageCache->ResetImage();
        }
    }
}

void NFmiSatelliteImageChannelCache::Clear()
{
    std::lock_guard<std::mutex> lock(mImageCacheSetMutex);
    mImageCacheSortedList.clear();
}
