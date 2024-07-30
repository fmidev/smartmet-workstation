#pragma once

/*
* Tämä threadi lukee partialdata hakemiston querydata-tiedostoja 
* verkkopalvelimelta SmartMetin cache-hakemistoon.
* Siis jos SmartMet on laitettu ns. lokaali-levy moodiin.
*/

#include "FmiCopyingStatus.h"
#include <memory>

class NFmiHelpDataInfoSystem;
class NFmiHelpDataInfo;
class NFmiStopFunctor;

namespace LocalCacheHistoryDataThread
{
	void DoHistoryThread(std::shared_ptr<NFmiStopFunctor> &stopFunctorPtr, std::shared_ptr<NFmiHelpDataInfoSystem> helpDataInfoSystemPtr);
	CFmiCopyingStatus CollectOldModelRunDataToCache(const NFmiHelpDataInfo& theDataInfo, std::shared_ptr<NFmiHelpDataInfoSystem> helpDataInfoSystemPtr);
}
