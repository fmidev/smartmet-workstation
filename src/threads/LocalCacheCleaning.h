#pragma once

#include <memory>

class NFmiHelpDataInfoSystem;
class NFmiStopFunctor;

namespace LocalCacheCleaning
{
    void InitLocalCacheCleaning(bool loadDataAtStartUp, bool autoLoadNewCacheDataMode, double cacheCleaningIntervalInHours, std::shared_ptr<NFmiStopFunctor> &stopFunctorPtr, const NFmiHelpDataInfoSystem& helpDataSystem);
    void DoPossibleLocalCacheCleaning(NFmiHelpDataInfoSystem& helpDataSystem);

    bool LoadDataAtStartUp();
    void LoadDataAtStartUp(bool newValue);
    bool AutoLoadNewCacheDataMode();
    void AutoLoadNewCacheDataMode(bool newValue);
    bool IsDataCopyingRoutinesOn();
}
