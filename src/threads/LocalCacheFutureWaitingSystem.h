#pragma once

#include "FmiCopyingStatus.h"
#include <future>
#include <list>

class NFmiStopFunctor;

class LocalCacheFutureWaitingSystem
{
	size_t maxFutureCount = 1;
	std::list<std::future<CFmiCopyingStatus>> futuresFromThreads;
	// Jos ohjelma halutaan lopettaa ulkoapäin, tälle gStopFunctorPtr:ille asetetaan tieto siitä.
	std::shared_ptr<NFmiStopFunctor> gStopFunctorPtr;
	// Halutaan palauttaa kokonaisen datakierroksen jälkeen joko kFmiCopyWentOk,
	// jos oli yhtään onnistunutta kopiointia serveriltä lokaali cacheen tai
	// muuten kFmiNoCopyNeeded status.
	bool hasDataCopiesDoneOnThisCycle = false;
public:
	void InitWaitingSystem(size_t maxThreadCount, std::shared_ptr<NFmiStopFunctor> stopFunctorPtr);
	bool AddFuture(std::future<CFmiCopyingStatus>&& futureFromThread);
	void WaitForFuturesToExpire();
	void WaitForAllFuturesToExpire();
	CFmiCopyingStatus GetAndResetDataCycleStatus();
private:
	void CheckIfProgramWantsToStop() const;
	bool IsFuturesAtMaxLimit() const;
	void SetMaxFutureCount(size_t maxCount);
	void CheckForExpiredFutures();
	bool IsFutureReady(std::future<CFmiCopyingStatus>& fut);
};
