#pragma once

#include "NFmiMetTime.h"
#include "NFmiDataIdent.h"
#include "NFmiSatelliteImageCacheHelpers.h"
#include "NFmiSatelliteImageChannelCache.h"

#include <list>
#include <future>

#include <boost/noncopyable.hpp>

class NFmiHelpDataInfoSystem;
class NFmiArea;

// Luokka pit‰‰ kokonais cache kaikille eri satelliiteille ja niiden
// kaikille kanaville (ja alueille jos on eri alueita).
class NFmiSatelliteImageCacheSystem : public boost::noncopyable
{
public:
    typedef std::unique_ptr<NFmiSatelliteImageChannelCache> ChannelCacheItem;
    typedef std::list<ChannelCacheItem> ChannelCacheList;
    using ImageUpdateCallbackFunction = std::function<void(ImageCacheUpdateData&)>;

    NFmiSatelliteImageCacheSystem();
    void Init(NFmiHelpDataInfoSystem &satelInfoSystem);
    void SetCallbacks(ImageUpdateCallbackFunction &updatedCacheCallback, ImageUpdateCallbackFunction &loadedCacheCallback);

    NFmiImageHolder FindImage(const NFmiDataIdent &wantedDataIdent, const NFmiMetTime &wantedTime, int maxOffSetInMinutes = 0);
    ChannelCacheItem& FindChannelCache(const NFmiDataIdent &wantedDataIdent);
    NFmiMetTime GetLatestImageTime(const NFmiDataIdent &wantedDataIdent);
    boost::shared_ptr<NFmiArea> ImageChannelArea(const NFmiDataIdent &wantedDataIdent);
    void DoCacheFileChecks();
    void DoLoadingStatusChecks();
    void ResetImages();
    void ResetFailedImages(ImageCacheUpdateData &resetedImagesOut);
    void Clear();

    static void StartUpdateThreads(std::shared_ptr<NFmiSatelliteImageCacheSystem> &satelliteImageCacheTotal);
    static void StopUpdateThreads();
    static bool WaitUpdateThreadsToStop(int maxWaitTimeInMS);
    static bool WaitUpdateThreadsToTakeABreak(int maxWaitTimeInMS);
    static void TellUpdateThreadsToStopTheBreak();
private:
    bool mInitialized; // ei sallita tupla initialisointia, heitt‰‰ poikkeuksen ongelma tilanteissa
    ChannelCacheList mSatelliteImageChannelCacheList;
    ImageUpdateCallbackFunction mUpdatedCacheCallback;
    ImageUpdateCallbackFunction mLoadedCacheCallback;
};
