#pragma once

#include "catlog/catlog.h"
#include <mutex> 
#include <set> 
#include <string> 

class NFmiHelpDataInfoSystem;
class NFmiHelpDataInfo;

struct CachedDataFileInfo
{
	CachedDataFileInfo();
	static bool IsDataCached(const NFmiHelpDataInfo& theDataInfo);

	std::string itsTotalServerFileName; // t‰ss‰ on serverill‰ olevan tiedoston polku
	bool fFilePacked = false; // tieto onko serverill‰ oleva tiedosto pakattu vai ei
	std::string itsTotalCacheFileName; // t‰m‰ on datatiedoston lopullinen polku lokaali cachessa
	std::string itsTotalCacheTmpFileName; // t‰m‰ on tiedoston lokaali tmp hakemiston polku, mist‰ se rename:lla siirret‰‰n lopulliseen paikkaan nimeen
	std::string itsTotalCacheTmpPackedFileName; // jos tiedosto oli pakattu, t‰m‰ on lokaali tmp hakemiston nimi pakatulle tiedostolle, joka sitten puretaan itsTotalCacheTmpFileName:ksi
	double itsFileSizeInMB = 0; // tiedoston koko levyll‰ serverill‰ [MB] (joko pakattu tai ei pakattu koko)
};

class NFmiMissingDataOnServerReporter
{
	// Kuinka monta normaali worker-threadia on tˆiss‰ (ei lasketa historia threadia mukaan).
	int workerThreadCount_ = 0;
	// Kuinka monesta queryDatasta ollaan kiinnostuneita serverill‰.
	int queryDataOnServerCount_ = 0;
	// Pidet‰‰n kirjaa server-to-local-cache loader worker-thread:eista, 
	// jotka ovat menneet l‰pi kierroksen kokonaan. Jos kaikki threadit 
	// ovat tehneet niin, voidaan raportoida lˆydettyjen datojen m‰‰rist‰.
    std::set<std::string> loaderThreadsThatHaveRunThroughCycle_;
	// Edell‰ olevaa listaa pit‰‰ vartioida mutexilla, koska nimi laitetaan siihen eri threadeista
	std::mutex loaderThreadsThatHaveRunThroughCycleMutex_;
	// Kun kaikki threadit ovat p‰‰sseet l‰pi 1. kierroksen kokonaan,
	// voidaan historia-data-lataus -thread laittaa p‰‰lle.
	// Lis‰ksi ei en‰‰ raportoida lokiin, mitk‰ datat uupuvat serverilt‰,
	// vain led-light-status systeemiin raportoidaan jatkossa tilannetta.
	bool allThreadsHaveRunThroughFirstCycle_ = false;
	// Pidet‰‰n kirjaa kuinka moni ja mitk‰ datat eiv‰t lˆydy ollenkaan
	// queryData serverilt‰.
    std::set<std::string> fileFiltersWithNoServerDataFiles_;
	// fileFiltersWithNoServerDataFiles tietorakennetta pit‰‰ suojella mutex:illa,
	// koska sit‰ p‰ivitet‰‰n useasta threadista rinnakkain.
    std::mutex fileFiltersWithNoServerDataFilesMutex_;
	// Kun saadaan kaikkien threadien kierros p‰‰tˆseen, pit‰‰ teht‰v‰t rutiinit lukita t‰ll‰ mutex:illa
	std::mutex cycleCompletionMutex_;
	bool initialized_ = false;

public:
	NFmiMissingDataOnServerReporter();
	bool initialize(NFmiHelpDataInfoSystem &helpDataSystem, int workerThreadCount);
	bool initialized() const {return initialized_;}

	void doReportIfFileFilterHasNoRelatedDataOnServer(const CachedDataFileInfo &cachedDataFileInfo, const std::string &fileFilter);
	void workerThreadCompletesCycle(const std::string& workerThreadName);

private:
	void startHistoryLoaderThreadOnce();
	void doLedChannelReportOfMissingServerData();
	void clearThreadCycleData();
	bool insertMissingDataFileFilter(const std::string& fileFilter);
	size_t completedCycleThreadCount();
	size_t missingDataOnServerCount();
	bool isCycleCompletedForAllThreads();
};

