// FmiCombineDataThread.cpp

#include "FmiCombineDataThread.h"
#include "afxmt.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFileString.h"
#include "NFmiFileSystem.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiQueryData.h"
#include "NFmiStreamQueryData.h"
#include "NFmiSoundingIndexCalculator.h"
#include "FmiDataLoadingThread2.h"
#include "catlog/catlog.h"

#include <vector>
#include <thread>
#include "NFmiSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static bool gUseDebugLog = false;

static void DebugCombineDataThread(const std::string &theLogStr, CatLog::Severity severity)
{
	if(gUseDebugLog || severity > CatLog::Severity::Debug) // eli jos t‰st‰ poikkeava, niin lokita
        CatLog::logMessage(theLogStr, severity, CatLog::Category::Data);
}

namespace
{
	CSemaphore gCombineDataThreadRunning; // t‰m‰n avulla yritet‰‰n lopettaan jatkuvasti pyˆriv‰ working thread 'siististi'
	CSemaphore gSoundingIndexDataThreadRunning; // t‰m‰n avulla yritet‰‰n lopettaan jatkuvasti pyˆriv‰ working thread 'siististi'

	NFmiStopFunctor gCombineDataStopFunctor, gSoundingIndexDataStopFunctor;
	bool gCheckForCombinedDataRebuild; // cache-threadi laittaa t‰m‰n flagin p‰‰lle, kun se on saanut kopsattua historia datan lokaali cacheen. 
										// T‰llˆin pit‰‰ tarkistaa, voidaanko yhdistelm‰ data rakentaa t‰ydellisill‰ tiedoilla.
	int gCombineDataStartUpDelayInMS = 0; // t‰m‰n avulla voidaan s‰‰t‰‰ kuinka kauan alussa odotellaan, ennen kuin tehd‰‰n tyˆt ensimm‰isen kerran
	int gSoundingIndexStartUpDelayInMS = 0; // t‰m‰n avulla voidaan s‰‰t‰‰ kuinka kauan alussa odotellaan, ennen kuin tehd‰‰n tyˆt ensimm‰isen kerran

/*
	int gClosingTime = 0; // main-thread ilmoittaa t‰ll‰, kun se haluaa lopettaa ohjelman, working threadilla
					// on jonkin aikaa lopettaa omat juttunsa (mutta jos kesken qdatan luvun, ei t‰t‰ voi testata)
					// Aluksi arvo nolla. Kun main-thread haluaa lopettaa, asettaa t‰m‰n ykkˆseksi.
					// Sen j‰lkeen yritt‰‰ saada lukkoon tietyn ajan gThreadRunning-semaphorea,
					// jos onnistuu, lopetetaan ohjelma halitusti. Jos se ei onnistu
					// (eli worker tekee datan lukua liian pitk‰‰n), main-thread lopettaa v‰kivaltaisesti.
*/
	// Tietorakenne, miss‰ tarvittavat tiedot datan yhdist‰misest‰ ja hakemistoista ja file-filttereist‰
	class DataCombineInfo
	{
	public:
		std::string itsSourceDirectory; // johdettu itsSourceFileFilter:ist‰
		std::string itsSourceFileFilter; // polkuineen filefilttereineen
		std::string itsTargetDirectory; // johdettu itsTargetFileFilter:ist‰
		std::string itsTargetFileFilter; // yhdistelm‰ tiedosto polkuineen ja filefilttereineen. nimess‰ pit‰‰ olla timestamppi minuutteja myˆten, aikaleima laitetaan *-merkin tilalle
		int itsMaximumTimeSteps; // kuinka monta aika-askelta kootussa datassa saa maksimissaan olla
	};

	std::vector<DataCombineInfo> gDataCombineInfos;
	CSemaphore gChangeCombineDataSettings; // t‰m‰n avulla muutetaan combine-data asetuksia thread-safesty
	std::vector<DataCombineInfo> gMediatorDataCombineInfos; // t‰t‰ k‰ytet‰‰n kun asetuksia muutetaan ja pit‰‰ rakentaa uusi info-setti thread-safesty
	bool gDataCombineSettingsChanged; 

	// Tietorakenne, miss‰ tarvittavat tiedot sounding-index datan tekemisest‰ ja hakemistoista ja file-filttereist‰
	class SoundingIndexDataInfo
	{
	public:
		bool IsNewDataNeeded(void)
		{ // tarkistaa l‰hdedatoista ja tulosdatoista uusimman tiedoston aikeleiman ja sen perusteella voidaan p‰‰tell‰ 
		  // onko tullut uutta dataa, josta pit‰‰ tehd‰ soundingindex-dataa
			return NFmiFileSystem::NewestPatternFileTime(itsSourceFileFilter) > NFmiFileSystem::NewestPatternFileTime(itsTargetFileFilter);
		}

		std::string itsSourceDirectory; // johdettu itsSourceFileFilter:ist‰
		std::string itsSourceFileFilter; // polkuineen filefilttereineen
		std::string itsTargetDirectory; // johdettu itsTargetFileFilter:ist‰
		std::string itsTargetFileFilter; // yhdistelm‰ tiedosto polkuineen ja filefilttereineen. nimess‰ pit‰‰ olla timestamppi minuutteja myˆten, aikaleima laitetaan *-merkin tilalle
        std::string itsRequiredGroundDataFileFilter; // Jos vaaditaan ground dataa laskuihin, t‰ss‰ pit‰‰ olla ei-tyhj‰ tiedosto file-filter
    };

	std::vector<SoundingIndexDataInfo> gSoundingIndexDataInfos;
}

void CFmiCombineDataThread::SetFirstTimeDelay(int theStartUpDelayInMS)
{
	gCombineDataStartUpDelayInMS = theStartUpDelayInMS;
}

void CFmiSoundingIndexDataThread::SetFirstTimeDelay(int theStartUpDelayInMS)
{
	gSoundingIndexStartUpDelayInMS = theStartUpDelayInMS;
}

static bool LetGoAfterFirstTimeDelaying(NFmiMilliSecondTimer &theTimer, bool theFirstTimeflag, int theDelayTimeInMS)
{
	if(theFirstTimeflag)
	{
		if(theTimer.CurrentTimeDiffInMSeconds() >= theDelayTimeInMS)
			return true;
	}
	return false;
}

void CFmiCombineDataThread::CheckForCombinedDataRebuild(void)
{
	gCheckForCombinedDataRebuild = true;
}

// main-thread kertoo t‰ll‰ funtiolla, ett‰ nyt on aika lopettaa
void CFmiCombineDataThread::CloseNow(void)
{
	gCombineDataStopFunctor.Stop(true);
}

// katsoo saako lukituksi gThreadRunning-semaphoren, jos saa, voidaan lopettaa ohjelma
// kuten haluttiinkin.
// Palauttaa 1 jos homma ok ja voidaan lopettaa.
// Palauttaa 0 jos ei saanut viel‰ semaphorea haltuunsa.
int CFmiCombineDataThread::WaitToClose(int theMilliSecondsToWait)
{
	CSingleLock singleLock(&gCombineDataThreadRunning);
	if(singleLock.Lock(theMilliSecondsToWait))
		return 1;
	return 0;
}

static std::string GetDirectory(const std::string &theFileFilter)
{
	NFmiFileString fileStr(theFileFilter);
	NFmiString str;
	str += fileStr.Device();
	str += fileStr.Path();

	return static_cast<char*>(str);
}

#ifdef CreateDirectory
#undef CreateDirectory // win32 systeemit m‰‰rittelev‰t CreateDirectory-makron, joka otetaan t‰ss‰ pois k‰ytˆst‰ varmuuden vuoksi
#endif

static void MakeCombineDataInfos(const NFmiHelpDataInfoSystem &theHelpDataInfoSystem, std::vector<DataCombineInfo> &theDataCombineInfos)
{
	theDataCombineInfos.clear();
	const checkedVector<NFmiHelpDataInfo> &helpDataInfoVector = theHelpDataInfoSystem.DynamicHelpDataInfos();
	for(size_t i = 0; i < helpDataInfoVector.size(); i++)
	{
		const NFmiHelpDataInfo &helpDataInfo = helpDataInfoVector[i];
		if(helpDataInfo.IsEnabled() && helpDataInfo.IsCombineData())
		{ // jos kyseess‰ oli datatiedostoista koottava data, lis‰t‰‰n se listaan
			DataCombineInfo dataCombineInfo;
			dataCombineInfo.itsMaximumTimeSteps = helpDataInfo.CombineDataMaxTimeSteps();
			dataCombineInfo.itsTargetFileFilter = helpDataInfo.CombineDataPathAndFileName();
			dataCombineInfo.itsTargetDirectory = ::GetDirectory(dataCombineInfo.itsTargetFileFilter);
			dataCombineInfo.itsSourceFileFilter = helpDataInfo.UsedFileNameFilter(theHelpDataInfoSystem);
			dataCombineInfo.itsSourceDirectory = ::GetDirectory(dataCombineInfo.itsSourceFileFilter);
			theDataCombineInfos.push_back(dataCombineInfo);
		}
	}
}

void CFmiCombineDataThread::InitCombineDataInfos(const NFmiHelpDataInfoSystem &theHelpDataInfoSystem)
{
	CSingleLock singleLock(&gChangeCombineDataSettings); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(singleLock.Lock(3000)) // Attempt to lock the shared resource, 3000 means 3 sec wait
	{
		::MakeCombineDataInfos(theHelpDataInfoSystem, gMediatorDataCombineInfos);
		gDataCombineSettingsChanged = true;
	}
}

static void ApplyCombineDataInfoSettings(void)
{
	if(gDataCombineSettingsChanged)
	{
		CSingleLock singleLock(&gChangeCombineDataSettings); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
		if(singleLock.Lock(3000)) // Attempt to lock the shared resource, 3000 means 3 sec wait
		{
			gDataCombineInfos.swap(gMediatorDataCombineInfos);
			gMediatorDataCombineInfos.clear();
			gDataCombineSettingsChanged = false;
		}
	}
}

static void	DoCombinationWork(bool fDoRebuildCheck)
{
	::ApplyCombineDataInfoSettings(); // aina kierroksen aluksi kokeillaan tarvitseeko asetuksia p‰ivitt‰‰...

	// 1. k‰y l‰pi kaikki tarkasteltavat hakemistot, ja tutki onko niihin ilmestynyt uusia datatiedostoja
	//	- kumpaa aikaleimaa pit‰‰ tarkastella cache-hakiksen viimeisint‰ vai lahde hakemiston viimeisint‰
	//  - jos useita SmartMeteja k‰ynniss‰, ei ehk‰ kuin yhden tarvitsisi tehd‰ yhdistelm‰ data, 
	//    jos tarkastellaan cache-hakemiston viimeisint‰ tiedostoa ja verataan sit‰ l‰hde-hakemiston viimeiseen
	//		- jos cache-hakis sis‰lt‰‰ tuoreemman, ei tehd‰ mit‰‰n!!
	for(size_t i = 0; i < gDataCombineInfos.size(); i++)
	{
		DataCombineInfo &combinedDataInfo = gDataCombineInfos[i];
		NFmiQueryDataUtil::CheckIfStopped(&gCombineDataStopFunctor);
        ::DebugCombineDataThread(std::string("Checking if data combination is needed from directories:\n") + combinedDataInfo.itsSourceDirectory + std::string(" and ") + combinedDataInfo.itsTargetDirectory, CatLog::Severity::Debug);
//		if(NFmiFileSystem::NewestFileTime(combinedDataInfo.itsSourceDirectory) > NFmiFileSystem::NewestFileTime(combinedDataInfo.itsTargetDirectory))
		if(fDoRebuildCheck || NFmiFileSystem::NewestPatternFileTime(combinedDataInfo.itsSourceFileFilter) > NFmiFileSystem::NewestPatternFileTime(combinedDataInfo.itsTargetFileFilter))
		{
            ::DebugCombineDataThread(std::string("Starting doing combine data from directory: ") + combinedDataInfo.itsSourceDirectory, CatLog::Severity::Debug);
	// 2. jos on, niin tee yhdistely 
			try
			{
                std::unique_ptr<NFmiQueryData> data(NFmiQueryDataUtil::CombineQueryDatas(fDoRebuildCheck, combinedDataInfo.itsTargetFileFilter, combinedDataInfo.itsSourceFileFilter, true, combinedDataInfo.itsMaximumTimeSteps, &gCombineDataStopFunctor)); // true = tehd‰‰n aika-askel yhdistelty‰ dataa
                if(data)
                {
                    // varmistetaan ett‰ kohda hakemisto on olemassa
                    NFmiFileSystem::CreateDirectory(combinedDataInfo.itsTargetDirectory);
                    // 3. tarkista ensin ett‰ ja talleta queryData tiedostoon oikeaan cache-hakemistoon, mutta v‰‰r‰ll‰ nimell‰ ja lopuksi tee rename jolloin tiedosto on halutun niminen
                    std::string outputFileName = combinedDataInfo.itsTargetFileFilter;
                    NFmiStaticTime currentTime;
                    std::string timeStampStr = static_cast<char*>(currentTime.ToStr(kYYYYMMDDHHMM)); // tehd‰‰n minuutin tarkkuudella aikaleima, t‰llˆin jos toinen k‰ynniss‰ oleva SmartMet tekee jo ennenmin tiedoston, t‰t‰ ei tarvitse tallentaa
                    NFmiStringTools::ReplaceAll(outputFileName, "*", timeStampStr);
                    std::string tmpOutputFileName = outputFileName + "_TMP_FILE_DELETE_THIS_";

                    if(NFmiFileSystem::FileExists(outputFileName) == false && NFmiFileSystem::FileExists(tmpOutputFileName) == false)
                    {
                        ::DebugCombineDataThread(std::string("Storing data to file: ") + tmpOutputFileName, CatLog::Severity::Debug);
                        NFmiStreamQueryData sQueryData;
                        sQueryData.WriteData(tmpOutputFileName, data.get(), static_cast<long>(data->InfoVersion()));
                        if(NFmiFileSystem::FileExists(outputFileName) == false)
                        {
                            NFmiFileSystem::RenameFile(tmpOutputFileName, outputFileName);
                            ::DebugCombineDataThread(std::string("renaming to final data-file: ") + outputFileName, CatLog::Severity::Debug);
                            CFmiDataLoadingThread2::LoadDataNow(); // laitetaan tietoa data-loading threadille ett‰ on tullut uutta dataa
                        }
                        else
                        { // jos sinne on nyt ilmestynyt saman niminen tiedosto, poistetaan v‰liaikainen tiedosto, joka ehdittiin tallentaa
                            NFmiFileSystem::RemoveFile(tmpOutputFileName);
                            ::DebugCombineDataThread(std::string("Removing temporary file because final data-file allready exist: ") + outputFileName, CatLog::Severity::Debug);
                        }

                        // siivotaan hakemistoa aina kun lis‰t‰‰n tiedostoja
                        double maxFileAgeInHours = 1;
                        if(data->Info()->TimeResolution() > 0)
                            maxFileAgeInHours = data->Info()->TimeResolution() * 4.2 / 60.; // yritet‰‰n tehd‰ niin pitk‰ ik‰‰ntymis aika, ett‰ 4 tiedostoa j‰‰
                        NFmiFileSystem::CleanDirectory(combinedDataInfo.itsTargetDirectory, maxFileAgeInHours); // tuhotaan tuntia vanhemmat tiedostot, ei voida k‰ytt‰‰ fileFilter siivousta, t‰llˆin hakemistoon j‰isi mahdolliset tmp-tiedostot
                        NFmiFileSystem::CleanFilePattern(combinedDataInfo.itsTargetFileFilter, 3); // tuhotaan viel‰ muut paitsi 3 viimaist‰ tiedostoa, jotka osuvat filefilteriin
                    }
                    else
                    {
                        ::DebugCombineDataThread(std::string("Not storing the data, all ready exist?: ") + outputFileName, CatLog::Severity::Debug);
                        // siell‰ oli jo sen niminen tiedosto, oletetaan ett‰ toinen SmartMet teki sen juuri, eik‰ tehd‰ mit‰‰n
                    }
                }
                else
                    ::DebugCombineDataThread(std::string("Unable to create the combined data"), CatLog::Severity::Debug);
            }
			catch(NFmiStopThreadException & /* e */ )
			{
				return ; // aika lopettaa threadi
			}
			catch(...)
			{ // t‰m‰ oli joku muu 'normaali' virhe
				continue; // jatketaan for-looppia
			}
		}
	}
}

UINT CFmiCombineDataThread::DoThread(LPVOID /* pParam */)
{
	gCheckForCombinedDataRebuild = false;

    ::DebugCombineDataThread("CFmiCombineDataThread::DoThread - starting combineData-thread.", CatLog::Severity::Debug);

	CSingleLock singleLock(&gCombineDataThreadRunning); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(5000)) // Attempt to lock the shared resource, 5000 means 5 sec wait, 0 wait resulted sometimes to wait for next minute for unknown reason
	{
//		doc->LogMessage("CFmiCombineDataThread::DoThread oli jo lukittuna, lopetetaan...", Info);
		return 1;
	}

	NFmiMilliSecondTimer timer;
	bool firstTime = true;

	// T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
	// Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
	int counter = 0;
	try
	{
		for( ; ; counter++)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gCombineDataStopFunctor);
			bool localDoRebuildCheckFlag = 	gCheckForCombinedDataRebuild; // tehd‰‰n rebuild-check lokaalin muuttujan kautta, ett‰ DoCombinationWork-funktiossa ei aloiteta sit‰ kesken kierroksen (jonka lopuksi lippu laitetaan sitten pois p‰‰lt‰)

			if(localDoRebuildCheckFlag || ::LetGoAfterFirstTimeDelaying(timer, firstTime, gCombineDataStartUpDelayInMS) || timer.CurrentTimeDiffInMSeconds() > (30 * 1000))
			{ // jos on kulunut tarpeeksi aikaa, tarkastetaan, onko jonnekin tullut uusia datatiedostoja jotka pit‰‰ yhdist‰‰
				firstTime = false;
				try
				{
					::DoCombinationWork(localDoRebuildCheckFlag);
				}
				catch(NFmiStopThreadException & /* e */ )
				{
					return 0;
				}
				catch(...)
				{
					// t‰m‰ oli joku 'tavallinen' virhe tilanne,
					// jatketaan vain loopitusta
				}
				if(localDoRebuildCheckFlag)
					gCheckForCombinedDataRebuild = false; // t‰ss‰ on varmistettu, ett‰ lippua ei aseteta ennen kuin kaikki datat on k‰yty l‰pi
				timer.StartTimer(); // aloitetaan taas uusi ajan lasku
			}
			else
			{
				if(counter%15 == 0)
					::DebugCombineDataThread("CFmiCombineDataThread::DoThread - do nothing (report every 15th time)...", CatLog::Severity::Debug);
			}

			NFmiQueryDataUtil::CheckIfStopped(&gCombineDataStopFunctor);
			Sleep(1*1000); // nukutaan aina lyhyit‰ aikoja (1 s), ett‰ osataan tutkia usein, joska p‰‰ohjelma haluaa jo sulkea
		}
	}
	catch(...)
	{
		// t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
	}

    return 0;   // thread completed successfully
}

// **********************************************************************
// CFmiSoundingIndexDataThread-namespaceen liittyv‰t jutut
// **********************************************************************

static void MakeSoundingIndexBasePathWarning(bool baseSoundingIndexDirectoryStringEmpty, const std::string &sourceDataFileFilter)
{
    static const std::string warningStr = "Neither 'soundingIndexBasePath' nor 'dataPath4' were not set in configurations.\nSoundingIndex data is set to be generated and it'll be stored to root directory.\nFollowing data generates soundingIndex data:\n";
    static const std::string warningTitleStr = "soundingIndex data path warning";

    if(baseSoundingIndexDirectoryStringEmpty)
    {
        CatLog::logMessage(warningStr + sourceDataFileFilter, CatLog::Severity::Warning, CatLog::Category::Data);
    }
}


// T‰t‰ initialisointi funktiota pit‰‰ kutsua ennen kuin itse threadi k‰ynnistet‰‰n. T‰m‰ lis‰‰
// luettavia datoja NFmiHelpInfoDataSystem:iin.
void CFmiSoundingIndexDataThread::InitSoundingIndexDataInfos(NFmiHelpDataInfoSystem &theHelpDataInfoSystem, const std::string &autoGeneratedSoundingIndexBasePath)
{
	gSoundingIndexDataInfos.clear();
	std::string cacheBaseDir = autoGeneratedSoundingIndexBasePath;
    bool baseDirStringEmpty = cacheBaseDir.empty();
	cacheBaseDir += kFmiDirectorySeparator;
	std::string extraName = "_soundingIndex"; // t‰m‰ lis‰t‰‰n cache hakemiston ja tiedoston nimeen.

	const checkedVector<NFmiHelpDataInfo> &helpDataInfoVector = theHelpDataInfoSystem.DynamicHelpDataInfos();
	checkedVector<NFmiHelpDataInfo> newHelpDataInfos; // t‰h‰n lis‰t‰‰n uudet datat, jotka pit‰‰ rekisterˆid‰
	for(size_t i = 0; i < helpDataInfoVector.size(); i++)
	{
		const NFmiHelpDataInfo &helpDataInfo = helpDataInfoVector[i];
		if(helpDataInfo.IsEnabled() && helpDataInfo.MakeSoundingIndexData())
		{ // jos halutaan ett‰ datasta tehd‰‰n soundingIndex-dataa, lis‰t‰‰n se listaan
			SoundingIndexDataInfo soundIndexInfo;
			soundIndexInfo.itsSourceFileFilter = helpDataInfo.UsedFileNameFilter(theHelpDataInfoSystem);
			soundIndexInfo.itsSourceDirectory = ::GetDirectory(soundIndexInfo.itsSourceFileFilter);
            ::MakeSoundingIndexBasePathWarning(baseDirStringEmpty, soundIndexInfo.itsSourceFileFilter);

			// tee target hakemistot ja filefilterit automaattisesti (miss‰ rekisterˆid‰‰n ne luettaviksi infoOrganizeriin???, pit‰isikˆ t‰m‰ luoda GenDocin init:iss‰?)
			soundIndexInfo.itsTargetDirectory = cacheBaseDir + helpDataInfo.Name() + extraName;

			soundIndexInfo.itsTargetFileFilter = soundIndexInfo.itsTargetDirectory;
			soundIndexInfo.itsTargetFileFilter += kFmiDirectorySeparator;
			soundIndexInfo.itsTargetFileFilter += "*_" + helpDataInfo.Name() + extraName + ".sqd";
            if(!helpDataInfo.RequiredGroundDataFileFilterForSoundingIndexCalculations().empty())
            {
                auto groundDataHelpInfo = theHelpDataInfoSystem.FindHelpDataInfo(helpDataInfo.RequiredGroundDataFileFilterForSoundingIndexCalculations());
                if(groundDataHelpInfo)
                {
                    soundIndexInfo.itsRequiredGroundDataFileFilter = groundDataHelpInfo->UsedFileNameFilter(theHelpDataInfoSystem);;
                }
            }

			gSoundingIndexDataInfos.push_back(soundIndexInfo);

			// Lis‰ksi luodut data pit‰‰ rekisterˆid‰, ett‰ SmartMet lukee ne normaali k‰yt‰nnˆn mukaisesti.
			// Lis‰t‰‰n ne listaan ensin ja lopuksi lis‰t‰‰n ne kerralla HelpDataInfoSystem:iin.
			NFmiHelpDataInfo hInfo;
			hInfo.Name(helpDataInfo.Name() + extraName);
			hInfo.FileNameFilter(soundIndexInfo.itsTargetFileFilter, true); // t‰m‰ taitaa olla ainoa tilanne kun force laitetaan p‰‰lle
			hInfo.DataType(static_cast<NFmiInfoData::Type>(NFmiInfoData::kModelHelpData));
//			hInfo.DataType(static_cast<NFmiInfoData::Type>(NFmiInfoData::kSoundingParameterData + helpDataInfo.DataType())); // T‰m‰ on ik‰v‰ j‰‰nne vanhasta tavasta laskea lennossa indeksej‰, jos en tee t‰t‰ samoin 
//																					// kuin ennen, eiv‰t jo luodut n‰yttˆmakrot toimi n‰iden kanssa en‰‰. Tekisi mieli tehd‰ t‰m‰ toisin...
			newHelpDataInfos.push_back(hInfo);
		}
	}
	// rekiterˆid‰‰n uudet datat
	for(size_t j = 0; j < newHelpDataInfos.size(); j++)
        theHelpDataInfoSystem.AddDynamic(newHelpDataInfos[j]);
}

void CFmiSoundingIndexDataThread::CloseNow(void)
{
	gSoundingIndexDataStopFunctor.Stop(true);
}

int CFmiSoundingIndexDataThread::WaitToClose(int theMilliSecondsToWait)
{
	CSingleLock singleLock(&gSoundingIndexDataThreadRunning);
	if(singleLock.Lock(theMilliSecondsToWait))
		return 1;
	return 0;
}

static void	DoSoundingIndexDataWork()
{
    // Jos koneessa on liian v‰h‰n ytimi‰, ei kannata jakaa laskuja useampaan s‰ikeeseen
	bool useOnlyOneThread = std::thread::hardware_concurrency() < 4; 

	// 1. k‰y l‰pi kaikki tarkasteltavat infodatat, ja tutki, onko jossain qDatassa tuoreempi aikaleima kuin sit‰ vastaavassa soundingDatassa
	//  - jos useita SmartMeteja k‰ynniss‰, ei ehk‰ kuin yhden tarvitsisi tehd‰ yhdistelm‰ data, 
	//    jos tarkastellaan cache-hakemiston viimeisint‰ tiedostoa ja verrataan sit‰ l‰hde-hakemiston viimeiseen
	//		- jos cache-hakis sis‰lt‰‰ tuoreemman, ei tehd‰ mit‰‰n!!
	// TODO soundingIndex datojea voisi myˆs p‰ivitt‰‰, kun tulee uutta mesa analyysej‰ ja havaintoja, jolloin niille ajoille voisi laskea havaintojen avulla tietyt parametrit uudestaan
	for(size_t i = 0; i < gSoundingIndexDataInfos.size(); i++)
	{
		SoundingIndexDataInfo &dataInfo = gSoundingIndexDataInfos[i];
		NFmiQueryDataUtil::CheckIfStopped(&gSoundingIndexDataStopFunctor);
        ::DebugCombineDataThread(std::string("Checking if soundingIndexData making is needed from directories:\n") + dataInfo.itsSourceDirectory + std::string(" and ") + dataInfo.itsTargetDirectory, CatLog::Severity::Debug);
		if(dataInfo.IsNewDataNeeded())
		{
            CatLog::logMessage(std::string("Starting doing soundingIndex data from: ") + dataInfo.itsSourceFileFilter, CatLog::Severity::Debug, CatLog::Category::Data);
			NFmiMilliSecondTimer debugTimer;
			debugTimer.StartTimer();
	// 2. tee sounding index data
			boost::shared_ptr<NFmiQueryData> data;
			try
			{
				std::string producerNameStr; // TODO: laita t‰h‰n kuvaava nimi
                int maxUsedThreads = FmiRound(boost::thread::hardware_concurrency() * 0.3); // otetaan k‰yttˆˆn n. 1/3 s‰ikeist‰
				data = NFmiSoundingIndexCalculator::CreateNewSoundingIndexData(dataInfo.itsSourceFileFilter, producerNameStr, dataInfo.itsRequiredGroundDataFileFilter, false, &gSoundingIndexDataStopFunctor, useOnlyOneThread, maxUsedThreads);
				debugTimer.StopTimer();
			}
			catch(NFmiStopThreadException & /* e */ )
			{
				return ; // aika lopettaa threadi
			}
			catch(std::exception &e)
			{ 
                // t‰m‰ oli joku raportoitava virhe
                CatLog::Severity usedLogLevel = CatLog::Severity::Error;
                std::string exceptionMessage = e.what();
                if(exceptionMessage.find(NFmiSoundingIndexCalculator::itsReadCompatibleGroundData_functionName) != std::string::npos)
                    usedLogLevel = CatLog::Severity::Debug;
                CatLog::logMessage(std::string("DoSoundingIndexDataWork: ") + exceptionMessage, usedLogLevel, CatLog::Category::Data);
				continue; // jatketaan for-looppia
			}
			catch(...)
			{ // t‰m‰ oli joku muu 'normaali' virhe
                CatLog::logMessage("DoSoundingIndexDataWork: unknown problem", CatLog::Severity::Error, CatLog::Category::Data);
				continue; // jatketaan for-looppia
			}

			if(data)
			{
				{
					std::string debugStr("Making ");
					debugStr += dataInfo.itsTargetDirectory;
					debugStr += " \ndata lasted ";
					debugStr += debugTimer.EasyTimeDiffStr();
                    CatLog::logMessage(debugStr, CatLog::Severity::Debug, CatLog::Category::Data);
				}

				// varmistetaan ett‰ kohde hakemisto on olemassa
				NFmiFileSystem::CreateDirectory(dataInfo.itsTargetDirectory);
			// 3. tarkista ensin ett‰ ja talleta queryData tiedostoon oikeaan cache-hakemistoon, mutta v‰‰r‰ll‰ nimell‰ ja lopuksi tee rename jolloin tiedosto on halutun niminen
				std::string outputFileName = dataInfo.itsTargetFileFilter;
				NFmiStaticTime currentTime;
				std::string timeStampStr = static_cast<char*>(currentTime.ToStr(kYYYYMMDDHHMM)); // tehd‰‰n minuutin tarkkuudella aikaleima, t‰llˆin jos toinen k‰ynniss‰ oleva SmartMet tekee jo ennenmin tiedoston, t‰t‰ ei tarvitse tallentaa
				NFmiStringTools::ReplaceAll(outputFileName, "*", timeStampStr);
				std::string tmpOutputFileName = outputFileName + "_TMP_FILE_DELETE_THIS_";

				if(NFmiFileSystem::FileExists(outputFileName) == false && NFmiFileSystem::FileExists(tmpOutputFileName) == false)
				{
                    ::DebugCombineDataThread(std::string("finished doing soundingIndex data from: ") + dataInfo.itsSourceFileFilter, CatLog::Severity::Info);
                    ::DebugCombineDataThread(std::string("Storing soundingIndex-data to file: ") + tmpOutputFileName, CatLog::Severity::Debug);
					NFmiStreamQueryData sQueryData;
					sQueryData.WriteData(tmpOutputFileName, data.get(), static_cast<long>(data->InfoVersion()));
					if(NFmiFileSystem::FileExists(outputFileName) == false)
					{
						NFmiFileSystem::RenameFile(tmpOutputFileName, outputFileName);
                        ::DebugCombineDataThread(std::string("renaming to final soundingIndexData-file: ") + outputFileName, CatLog::Severity::Debug);
						CFmiDataLoadingThread2::LoadDataNow(); // laitetaan tietoa data-loading threadille ett‰ on tullut uutta dataa
					}
					else
					{ // jos sinne on nyt ilmestynyt saman niminen tiedosto, poistetaan v‰liaikainen tiedosto, joka ehdittiin tallentaa
						NFmiFileSystem::RemoveFile(tmpOutputFileName);
                        ::DebugCombineDataThread(std::string("Removing temporary file because final soundingIndexData-file allready exist: ") + outputFileName, CatLog::Severity::Debug);
					}

					// siivotaan hakemistoa aina kun lis‰t‰‰n tiedostoja
					double maxFileAgeInHours = 13;
					NFmiFileSystem::CleanDirectory(dataInfo.itsTargetDirectory, maxFileAgeInHours); // tuhotaan tuntia vanhemmat tiedostot, ei voida k‰ytt‰‰ fileFilter siivousta, t‰llˆin hakemistoon j‰isi mahdolliset tmp-tiedostot
				}
				else
				{
                    ::DebugCombineDataThread(std::string("Not storing the soundingIndexData, all ready exist?: ") + outputFileName, CatLog::Severity::Debug);
					// siell‰ oli jo sen niminen tiedosto, oletetaan ett‰ toinen SmartMet teki sen juuri, eik‰ tehd‰ mit‰‰n
				}
			}
			else
                ::DebugCombineDataThread(std::string("Unable to create the soundingIndexData from: ") + dataInfo.itsSourceFileFilter, CatLog::Severity::Info);
		}
	}
}

UINT CFmiSoundingIndexDataThread::DoThread(LPVOID /* pParam */ )
{
	::DebugCombineDataThread("CFmiSoundingIndexDataThread::DoThread - starting soundingIndexData-thread.", CatLog::Severity::Debug);

	CSingleLock singleLock(&gSoundingIndexDataThreadRunning); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(5000)) // Attempt to lock the shared resource, 5000 means 5 sec wait, 0 wait resulted sometimes to wait for next minute for unknown reason
	{
        ::DebugCombineDataThread("CFmiSoundingIndexDataThread::DoThread oli jo lukittuna, lopetetaan...", CatLog::Severity::Debug);
		return 1;
	}

	if(gSoundingIndexDataInfos.size() == 0)
	{
        ::DebugCombineDataThread("CFmiSoundingIndexDataThread::DoThread - there were no data registered to work with, ending thread.", CatLog::Severity::Debug);
		return 0;
	}

	NFmiMilliSecondTimer timer;
	bool firstTime = true;

	// T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n soundingIndex-datoja SmartMetin luettavaksi.
	// Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
	int counter = 0;
	try
	{
		for( ; ; counter++)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gSoundingIndexDataStopFunctor);

			if(::LetGoAfterFirstTimeDelaying(timer, firstTime, gSoundingIndexStartUpDelayInMS) || timer.CurrentTimeDiffInMSeconds() > (60 * 1000))
			{ // jos on kulunut tarpeeksi aikaa, tarkastetaan, onko jonnekin tullut uusia datatiedostoja jotka pit‰‰ yhdist‰‰
				firstTime = false;
				try
				{
					DoSoundingIndexDataWork();
				}
				catch(NFmiStopThreadException & /* e */ )
				{
					return 0;
				}
				catch(...)
				{
					// t‰m‰ oli joku 'tavallinen' virhe tilanne,
					// jatketaan vain loopitusta
				}

				timer.StartTimer(); // aloitetaan taas uusi ajan lasku
			}
			else
			{
				if(counter%30 == 0)
                    ::DebugCombineDataThread("CFmiSoundingIndexDataThread::DoThread - do nothing (report every 15th time)...", CatLog::Severity::Debug);
			}

			NFmiQueryDataUtil::CheckIfStopped(&gSoundingIndexDataStopFunctor);
			Sleep(1*1000); // nukutaan aina lyhyit‰ aikoja (1 s), ett‰ osataan tutkia usein, joska p‰‰ohjelma haluaa jo sulkea
		}
	}
	catch(...)
	{
		// t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
	}

    return 0;   // thread completed successfully
}


