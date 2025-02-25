#pragma once

/*
* T�m� threadi lukee querydata-tiedostoja verkkopalvelimelta SmartMetin lokaali 
* cache-hakemistoon. Siis jos SmartMet on laitettu ns. lokaali-levy moodiin.
* T�m� on siis vain hallinnoiva worker-thread, joka kutsuu erillisi� yhden datatiedoston
* kopioivia worker-threadeja tarpeen mukaan (max 3 kpl).
*/

#include <string>
#include <list>

class NFmiHelpDataInfoSystem;

namespace QueryDataToLocalCacheLoaderThread
{
    // InitHelpDataInfo funktiota pit�� kutsua ennen kuin luokan varsinainen thread (DoThread) k�ynnistet��n
    void InitHelpDataInfo(const NFmiHelpDataInfoSystem& helpDataInfoSystem, const std::string& smartMetBinariesDirectory, double cacheCleaningIntervalInHours, bool loadDataAtStartUp, bool autoLoadNewCacheDataMode, const std::string& smartMetWorkingDirectory, double maxDataFileSizeInMB);
    void DoThread();
    void CloseNow();
    bool WaitToClose(int milliSecondsToWait);
    void UpdateSettings(NFmiHelpDataInfoSystem& theHelpDataSystem, bool loadDataAtStartUp, bool autoLoadNewCacheDataMode);
    void AutoLoadNewCacheDataMode(bool newValue);
    void AddPrioritizedDataLoadWork(const std::list<std::string>& prioritizedDataLoadWorkList);
    void AddLoadOldDataWork(const std::list<std::string>& loadOldDataWorkList);
}

