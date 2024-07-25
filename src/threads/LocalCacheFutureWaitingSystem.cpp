#include "LocalCacheFutureWaitingSystem.h"
#include "NFmiQueryDataUtil.h"
#include "catlog/catlog.h"


void LocalCacheFutureWaitingSystem::InitWaitingSystem(size_t maxThreadCount, std::shared_ptr<NFmiStopFunctor> stopFunctorPtr)
{
	SetMaxFutureCount(maxThreadCount);
	if(maxFutureCount == 0)
		maxFutureCount = 1;
	gStopFunctorPtr = stopFunctorPtr;
	futuresFromThreads.clear();
}

// Jos lista oli jo yli rajan, heitetään poikkeus, koska tämä oli looginen virhe.
bool LocalCacheFutureWaitingSystem::AddFuture(std::future<CFmiCopyingStatus>&& futureFromThread)
{ 
	if(IsFuturesAtMaxLimit())
	{
		throw std::runtime_error("LocalCacheFutureWaitingSystem::AddFuture - futures were at max limit already, logic error in code...");
	}

	// Siirretään future listaan odottamaan valmistumista
	futuresFromThreads.emplace_back(std::move(futureFromThread));
	return true; 
}

// Tämä funktio looppaa niin kauan kunnes joku/jotkin future(t) valmistuvat,
// jolloin ne poistetaan listasta.
// Loopissa tarkistetaan myös haluaako pääohjelma lopettaa.
// Loopissa odotetaan pieni aika (0.01 s), jotta CPU:ta ei turhaan huudateta.
void LocalCacheFutureWaitingSystem::WaitForFuturesToExpire()
{
	while(IsFuturesAtMaxLimit())
	{
		CheckForExpiredFutures();
		CheckIfProgramWantsToStop();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void LocalCacheFutureWaitingSystem::CheckIfProgramWantsToStop() const
{
	NFmiQueryDataUtil::CheckIfStopped(gStopFunctorPtr.get());
}

bool LocalCacheFutureWaitingSystem::IsFuturesAtMaxLimit() const
{
	return size_t(maxFutureCount) <= futuresFromThreads.size();
}

void LocalCacheFutureWaitingSystem::SetMaxFutureCount(size_t maxThreadCount)
{
	maxFutureCount = maxThreadCount;
	if(maxFutureCount == 0)
		maxFutureCount = 1;
}

bool LocalCacheFutureWaitingSystem::IsFutureReady(std::future<CFmiCopyingStatus>& fut)
{
	static int debugCounter = 1;
	auto status = fut.wait_for(std::chrono::milliseconds(2));
	if(status == std::future_status::ready)
	{
		auto value = fut.get();
		if(value == kFmiCopyWentOk || value == kFmiUnpackIsDoneInSeparateProcess)
		{
			hasDataCopiesDoneOnThisCycle = true;
			std::string message = "IsFutureReady [";
			message += std::to_string(debugCounter++);
			message += "] file was ";
			message += (value == kFmiCopyWentOk) ? "copied to local cache" : "packed and it's now in unpacking process";
			CatLog::logMessage(message, CatLog::Severity::Debug, CatLog::Category::Data, true);
		}
		return true;
	}
	return false;
}

void LocalCacheFutureWaitingSystem::CheckForExpiredFutures()
{
	futuresFromThreads.remove_if(
		[this](auto& fut) 
		{return this->IsFutureReady(fut); }
	);
}

CFmiCopyingStatus LocalCacheFutureWaitingSystem::GetAndResetDataCycleStatus()
{
	auto returnedStatus = hasDataCopiesDoneOnThisCycle ? kFmiCopyWentOk : kFmiNoCopyNeeded;
	hasDataCopiesDoneOnThisCycle = false;
	return returnedStatus;
}
