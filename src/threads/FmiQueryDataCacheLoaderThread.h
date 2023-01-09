#pragma once

/*
* T‰m‰ threadi lukee querydata-tiedostoja verkkopalvelimelta SmartMetin cache-hakemistoon.
* Siis jos SmartMet on laitettu ns. lokaali-levy moodiin.
*/

#include "stdafx.h"
#include <string>

class NFmiHelpDataInfoSystem;
class CFmiCacheLoaderData;

namespace CFmiQueryDataCacheLoaderThread
{
    // t‰m‰ pit‰‰ kutsua ennen kuin threadi (DoThread) k‰ynnistet‰‰n
	void InitHelpDataInfo(const NFmiHelpDataInfoSystem &helpDataInfoSystem, const std::string &smartMetBinariesDirectory, double cacheCleaningIntervalInHours, const std::string & smartMetWorkingDirectory, int usedDataLoaderThreadCount);
	UINT DoThread(LPVOID pParam);
	void CloseNow(void);
	int WaitToClose(int theMilliSecondsToWait, CFmiCacheLoaderData *theCacheLoaderData);
    void UpdateSettings(NFmiHelpDataInfoSystem &theHelpDataSystem);
    void LoadDataAtStartUp(bool newState);
    void AutoLoadNewCacheDataMode(bool newState);
	std::string MakeDailyUnpackLogFilePath();

	UINT DoHistoryThread(LPVOID pParam);
}

