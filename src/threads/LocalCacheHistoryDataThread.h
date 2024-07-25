#pragma once

/*
* Tämä threadi lukee partialdata hakemiston querydata-tiedostoja 
* verkkopalvelimelta SmartMetin cache-hakemistoon.
* Siis jos SmartMet on laitettu ns. lokaali-levy moodiin.
*/

#include <memory>

class NFmiHelpDataInfoSystem;
class NFmiStopFunctor;

namespace LocalCacheHistoryDataThread
{
	void DoHistoryThread(std::shared_ptr<NFmiStopFunctor> &stopFunctorPtr, std::shared_ptr<NFmiHelpDataInfoSystem> helpDataInfoSystemPtr);
}
