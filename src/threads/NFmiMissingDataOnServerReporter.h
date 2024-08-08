#pragma once

#include "catlog/catlog.h"
#include <mutex> 
#include <set> 
#include <string> 

class NFmiHelpDataInfoSystem;
class NFmiHelpDataInfo;
struct NFmiCachedDataFileInfo;

class NFmiMissingDataOnServerReporter
{
	// Kuinka monesta queryDatasta ollaan kiinnostuneita serverill‰.
	int queryDataOnServerCount_ = 0;
	// Kun p‰‰-worker-thread on p‰‰sseet l‰pi 1. kierroksen kokonaan,
	// laitetaan t‰m‰ lippu p‰‰lle.
	// Lis‰ksi ei en‰‰ raportoida lokiin, mitk‰ datat uupuvat serverilt‰,
	// vain led-light-status systeemiin raportoidaan jatkossa tilannetta.
	bool haveRunThroughFirstCycle_ = false;
	// Pidet‰‰n kirjaa kuinka moni ja mitk‰ datat eiv‰t lˆydy ollenkaan
	// queryData serverilt‰.
    std::set<std::string> fileFiltersWithNoServerDataFiles_;
	// fileFiltersWithNoServerDataFiles tietorakennetta pit‰‰ suojella mutex:illa,
	// koska sit‰ p‰ivitet‰‰n useasta yksitt‰isen datan lataus threadista rinnakkain.
    std::mutex fileFiltersWithNoServerDataFilesMutex_;
	bool initialized_ = false;

public:
	NFmiMissingDataOnServerReporter();
	bool initialize(NFmiHelpDataInfoSystem &helpDataSystem);
	bool initialized() const {return initialized_;}

	void doReportIfFileFilterHasNoRelatedDataOnServer(const NFmiCachedDataFileInfo &cachedDataFileInfo, const std::string &fileFilter);
	void mainWorkerThreadCompletesCycle();

private:
	void doLedChannelReportOfMissingServerData();
	void clearThreadCycleData();
	bool insertMissingDataFileFilter(const std::string& fileFilter);
	size_t missingDataOnServerCount();
};

