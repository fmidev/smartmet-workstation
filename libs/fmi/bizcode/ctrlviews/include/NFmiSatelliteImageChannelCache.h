#pragma once

#include "NFmiMetTime.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiDataIdent.h"
#include "NFmiSatelliteImageCacheHelpers.h"

#include <list>
#include <future>
#include <mutex> 

#include <boost/noncopyable.hpp>

class NFmiHelpDataInfoSystem;
class NFmiArea;
class NFmiSatelliteImageCache;

// T‰ss‰ on yhden satelliitin yhden kanavan kuva cache.
class NFmiSatelliteImageChannelCache : public boost::noncopyable
{
public:
    typedef std::shared_ptr<NFmiSatelliteImageCache> ImageCacheItem;
    typedef std::list<ImageCacheItem> ImageCacheSortedList;

    NFmiSatelliteImageChannelCache(const std::string &fileFilter, const boost::shared_ptr<NFmiArea> &imageArea, const NFmiDataIdent &imageDataIdent, int firstTimeUpdateDelayTimeInMS, int firstTimeLoadingWaitTimeMs, int imageLoadingFailedWaitTimeMs);

    ImageCacheUpdateData UpdateCacheList(bool forceUpdate);
    ImageCacheUpdateData CheckOnCacheLoading();

    const std::string& FileFilter() const {return mFileFilter;}
    const NFmiDataIdent& ImageDataIdent() const {return mImageDataIdent;}
    bool ChannelShownOnView() const {return mChannelShownOnView;}
    void ChannelShownOnView(bool newValue) {mChannelShownOnView = newValue;}
    int FirstTimeUpdateDelayTimeInMS() const { return mFirstTimeUpdateDelayTimeInMS; }

    ImageCacheItem FindImageCache(const NFmiMetTime &wantedTime, int maxOffSetInMinutes = 0);
    NFmiImageHolder FindImage(const NFmiMetTime &wantedTime, int maxOffSetInMinutes = 0);
    NFmiMetTime GetLatestImageTime();
    boost::shared_ptr<NFmiArea> ImageArea() { return mImageArea; }
    void ResetImages();
    void ResetFailedImages(ImageCacheUpdateData &resetedImagesOut);
    void Clear();

private:
    static std::list<std::string> GetFileList(const ImageCacheSortedList &imageCacheSortedList);
    static std::list<std::string> GetFileList(const std::string &fileFilter);
    bool InsertToImageCache_NoLock(const std::list<std::string> &fileListIn, ImageCacheUpdateData &updatedImagesOut);
    bool RemoveFromImageCache_NoLock(const std::list<std::string> &fileListIn, ImageCacheUpdateData &updatedImagesOut);
    void DoPossibleForceUpdate();

	const std::string mFileFilter; // N‰it‰ tiedostoja cachetetaan t‰ss‰ oliossa (esim. P:\meteosat9\HRV\scandinavia_1008x1118\*_8bit-msg-scandinavia_HRV.png)
    const NFmiDataIdent mImageDataIdent; // T‰m‰ on avain, mill‰ etsit‰‰n oikeaa kuva kanavaa DrawParamin param-asetuksen mukaan (par-id ja prod-id pari nimineen)
    bool mChannelShownOnView; // Onko kyseinen satel-kanava valittuna jollekin karttan‰ytˆlle, jos on, t‰llˆin tehd‰‰n cache-tiedosto p‰ivityksi‰ (tausta s‰ikeess‰) useammin
    NFmiMilliSecondTimer mFilesCheckedTimer; // Koska on viimeksi tehty tiedosto lista p‰ivitys -timer
    int mFirstTimeUpdateDelayTimeInMS; // Kun SmartMet k‰ynnistet‰‰n, ei ole tarkoitus ett‰ jokaisen satelliitin jokaista kanavaa aletaan heti tutkimaan verkkolevyilta, jokaisella kanavalle annetaan jonkinlainen odottelu aika, ennen kuin p‰ivitys rumba laitetaan k‰yntiin.

    ImageCacheSortedList mImageCacheSortedList; // T‰ss‰ on palvelimelta lˆytyv‰t satel-cache kuvat aikaj‰rjestyksess‰, t‰t‰ on pakko synkronoida mutexin avulla, koska t‰t‰ p‰ivitet‰‰n ja k‰ytet‰‰n eri s‰ikeist‰
    mutable std::mutex mImageCacheSetMutex;
    bool mCacheInitialized; // Onko cache k‰ynyt tiedostolistan l‰pi ainakin kerran
    int mFirstTimeLoadingWaitTimeMs; // T‰m‰n kanavan kuville: Kun lataus k‰ynnistet‰‰n 1. kerran, kuinka kauan odotetaan siin‰ paikassa valmistumista, enenen kuin palautetaan tyhj‰‰
    int mImageLoadingFailedWaitTimeMs; // T‰m‰n kanavan kuville: Kuinka kauan yritet‰‰n ladata imagea maksimissaan, ennen kuin sen lataus todetaan lopullisesti virheelliseksi
    boost::shared_ptr<NFmiArea> mImageArea; // Satel-kuvan konffeissa on m‰‰r‰tty alue, jonka kuvan alue peitt‰‰
};

