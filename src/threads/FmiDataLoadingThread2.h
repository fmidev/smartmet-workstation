#pragma once

// FmiDataLoadingThread2.h
// T‰m‰ on uudempi versio querydatojen luku threadista. T‰ss‰ on seuraavia ominaisuuksia:
// 1. Rakenne on muutettu saman tyyppiseksi kuin muissakin uudemmissa threadeissa, eli t‰ss‰ on namespace, jossa tuttuja funktioita
// 2. T‰m‰ kykenee hakemaan dataa joko originaali verkko palvelimelta tai sitten lokaali cachesta.
// 3. Dataa siirret‰‰n niin ett‰ ohjelmaa lopettaessa ei j‰‰ vuotoja kuten usein vanhalla tuntui j‰‰v‰n.
// 4. Datan v‰litykseen threadin ja p‰‰ohjelman v‰li‰ k‰ytet‰‰n nyt vektoria ja thread-safe semaphoreja.
// 5. P‰‰ohjelmalle ilmoitus vasta pieneen viiveen j‰lkeen, jolloin mahd. luetaan useita datoja siirtolistaan.

#include "NFmiInfoData.h"
#include "NFmiQueryData.h"
#include "stdafx.h"
#include <vector>
#include <memory>

class NFmiQueryData;
class NFmiHelpDataInfoSystem;
class NFmiDataNotificationSettingsWinRegistry;

// t‰m‰n tietorakenteen avulla v‰litet‰‰n ladattu qdata worker-threadista
// main-threadin gendocille.
struct LoadedQueryDataHolder
{
	LoadedQueryDataHolder(void)
	:itsQueryData()
	,itsDataFileName()
	,itsDataFilePattern()
	,itsDataType(NFmiInfoData::kNoDataType)
	,itsNotificationStr()
	{}
    LoadedQueryDataHolder(const LoadedQueryDataHolder&) = delete;
    LoadedQueryDataHolder& operator=(const LoadedQueryDataHolder&) = delete;

    LoadedQueryDataHolder(LoadedQueryDataHolder &&other)
    :itsQueryData(std::move(other.itsQueryData))
    ,itsDataFileName(std::move(other.itsDataFileName))
    ,itsDataFilePattern(std::move(other.itsDataFilePattern))
    ,itsDataType(std::move(other.itsDataType))
    ,itsNotificationStr(std::move(other.itsNotificationStr))
    {
    }

	bool HasData(void) const
	{
		return itsQueryData != 0;
	}

	std::unique_ptr<NFmiQueryData> itsQueryData;
	std::string itsDataFileName;
	std::string itsDataFilePattern;
	NFmiInfoData::Type itsDataType;
	std::string itsNotificationStr;
};

namespace CFmiDataLoadingThread2
{
	void InitDynamicHelpDataInfo(const NFmiHelpDataInfoSystem &helpDataInfoSystem, const NFmiDataNotificationSettingsWinRegistry &dataNotificationSettings, FmiLanguage usedLanguage); // t‰m‰ pit‰‰ kutsua ennen kuin threadi (DoThread) k‰ynnistet‰‰n
	void SettingsChanged(const NFmiHelpDataInfoSystem &helpDataInfoSystem, bool fDoHelpDataInfo); // p‰‰ohjelma kutsuu t‰t‰ kun se on muuttanut threadin k‰ynnistyksen j‰lkeen datan lukuun liittyvi‰ asetuksia (esim. verkko/lokaali asetus)
	UINT DoThread(LPVOID pParam);
	void CloseNow(void);
	int WaitToClose(int theMilliSecondsToWait);

	bool GetLoadedDatas(std::vector<LoadedQueryDataHolder> &theLoadedDatasOut); // t‰ll‰ p‰‰ohjelma pyyt‰‰ ladattuja datoja (funktiossa tehd‰‰n vector swap!!)
	void LoadDataNow(void); // t‰m‰ pakottaa ett‰ datan luku tehd‰‰n heti
	void ResetTimeStamps(void); // t‰m‰ asettaa NFmiHelpDataSystemin dynaamisten datojen aikaleimoiksi -1:en
}


