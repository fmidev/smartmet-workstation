// FmiDataLoadingThread.cpp

#include "FmiDataLoadingThread2.h"
#include "SmartMetThreads_resource.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiQueryData.h"
#include "NFmiDataNotificationSettingsWinRegistry.h"
#include "NFmiMilliSecondTimer.h"
#include "catlog/catlog.h"
#include "QueryDataReading.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFileSystem.h"
#include "CtrlViewFunctions.h"

// MSVC++ 2010 Beta 2 k‰‰nt‰j‰ ei k‰‰nn‰ ellei t‰m‰ ole muiden headereiden per‰ss‰
#include "afxmt.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static bool gUseDebugLog = true;

namespace
{
	// T‰m‰n avulla yritet‰‰n lopettaan jatkuvasti pyˆriv‰ working thread 'siististi'
	CSemaphore gThreadRunning; 
	NFmiStopFunctor gStopFunctor;
	// T‰m‰n olion avulla working thread osaa lukea haluttuja datoja
	NFmiHelpDataInfoSystem gWorkerHelpDataSystem; 
	// T‰m‰n avulla tehd‰‰n eri datoille halutunlaisia notifikaatio tekstej‰
    NFmiDataNotificationSettingsWinRegistry gWorkerDataNotificationSettings; 
	// Mink‰ kielisi‰ sanakirjaja k‰ytet‰‰n kun tehd‰‰n notifikaatio tekstej‰
	FmiLanguage gWorkerUsedLanguage; 
	// T‰m‰n avulla p‰ivitet‰‰n datan luku asetuksia thread safetysti
	CSemaphore gSettingsChanged; 
	// T‰m‰n avulla s‰‰det‰‰n threadin asetuksia thread safetysti
	NFmiHelpDataInfoSystem gMediatorHelpDataSystem; 
	bool gSettingsHaveChanged;
	// T‰m‰n avulla lis‰t‰‰n ja luetaan qdatoja listasta data-threadin ja p‰‰ohjelman v‰lill‰ thread-safesti
	CSemaphore gDataExchange; 
	std::vector<LoadedQueryDataHolder> gLoadedDatas;
	// t‰m‰n avulla voidaan pakottaa threadi tarkistamaan datan lukua heti
	bool gDoDataCheckNow = false; 
	// t‰m‰n avulla voidaan pakottaa NFmiHelpDataSystem:in kaikkien dynaamisten
	bool gResetTimeStamps = false; 
	// t‰t‰ k‰ytet‰‰n mm. CaseStudy-datan yhteydess‰, t‰ll‰ varmistetaan ett‰ uudet data polut tulevat k‰yttˆˆn
	bool gApplyHelpDataInfos = false; 
	// Tiedet‰‰n onko datan luku looppi menossa 1. kertaa vai ei, luettuihin datoihin merkit‰‰n kyseinen tieto.
	bool gFirstTimeGoingThrough = true;
	// Tietyt data poistuvat k‰ytˆst‰, mutta niiden k‰yttˆ‰ pit‰‰ tukea, kun ollaan CaseStudy moodissa,
	// koska mik‰ tahansa data on voitu joskus tallettaa johonkin CaseStudy pakettiin.
	// Eli jos t‰m‰ on false (SmartMetin normi moodi), ei t‰ll‰isi poistettuja datoja edes yritet‰ lukea.
	// Jos true, kaikkia datoja yritet‰‰n lukea.
	bool gIsInCaseStudyMode = false;
}

// T‰t‰ initialisointi funktiota pit‰‰ kutsua ennen kuin itse threadi k‰ynnistet‰‰n MainFramesta. 
void CFmiDataLoadingThread2::InitDynamicHelpDataInfo(const NFmiHelpDataInfoSystem &helpDataInfoSystem, const NFmiDataNotificationSettingsWinRegistry &dataNotificationSettings, FmiLanguage usedLanguage)
{
	gWorkerHelpDataSystem.InitSettings(helpDataInfoSystem, true);
    gWorkerDataNotificationSettings = dataNotificationSettings;
    gWorkerUsedLanguage = usedLanguage;
	gSettingsHaveChanged = false;
	gDoDataCheckNow = true;
	gMediatorHelpDataSystem.InitSettings(gWorkerHelpDataSystem, true);
}

void CFmiDataLoadingThread2::SettingsChanged(const NFmiHelpDataInfoSystem &helpDataInfoSystem, bool fDoHelpDataInfo)
{
	CSingleLock singleLock(&gSettingsChanged);
	if(singleLock.Lock(2000))
	{
		gMediatorHelpDataSystem.InitSettings(helpDataInfoSystem, fDoHelpDataInfo);
		gSettingsHaveChanged = true;
		gApplyHelpDataInfos = fDoHelpDataInfo;
	}
}

static void ApplyChangedSettings(void)
{
	if(gSettingsHaveChanged)
	{
		CSingleLock singleLock(&gSettingsChanged);
		if(singleLock.Lock(1000))
		{
			gWorkerHelpDataSystem.InitSettings(gMediatorHelpDataSystem, gApplyHelpDataInfos);
			gApplyHelpDataInfos = false;
			if(gResetTimeStamps)
				gWorkerHelpDataSystem.ResetAllDynamicDataTimeStamps();
			gSettingsHaveChanged = false;
		}
	}
}

void CFmiDataLoadingThread2::CloseNow(void)
{
	gStopFunctor.Stop(true);
}

int CFmiDataLoadingThread2::WaitToClose(int theMilliSecondsToWait)
{
	CSingleLock singleLock(&gThreadRunning);
	if(singleLock.Lock(theMilliSecondsToWait))
		return 1;
	return 0;
}

static void MakeDataNotificationTexts(const NFmiDataNotificationSettingsWinRegistry &dataNotificationSettings, FmiLanguage usedLanguage, LoadedQueryDataHolder &theData, NFmiHelpDataInfo &theDataInfo)
{
    if(theDataInfo.NotifyOnLoad())
    {
        std::string notificationStr;
        if(theDataInfo.NotificationLabel().empty())
        { // laitetaan tiedoston nimi
            notificationStr += theData.itsDataFileName;
        }
        else
            notificationStr += theDataInfo.NotificationLabel();

        notificationStr += "\n";
        notificationStr += theData.itsQueryData->Info()->OriginTime().ToStr(dataNotificationSettings.OriginTimeFormat(), usedLanguage);
        notificationStr += " ";
        NFmiTime aTime;
        notificationStr += aTime.UTCTime().ToStr(dataNotificationSettings.CurrentTimeFormat(), usedLanguage);
        theData.itsNotificationStr = notificationStr;
    }
}

// Annetaan semaphoren lukitukselle aina t‰m‰ aika kokeilla lukkoa, ennen 
// kuin ep‰onnistuu ja jolloin data menetet‰‰n, 3 sekuntia pit‰isi riitt‰‰ ruhtinaallisesti.
const int gDataExchangeTimeInMS = 3000;

// t‰ll‰ p‰‰ohjelma pyyt‰‰ ladattuja datoja (funktiossa tehd‰‰n vector swap!!)
// palauttaa true, jos uutta dataa tuli k‰yttˆˆn, muuten jos lukitus failaa, tai ei ollut dataa, palautetaan false.
bool CFmiDataLoadingThread2::GetLoadedDatas(std::vector<LoadedQueryDataHolder> &theLoadedDatasOut)
{
	CSingleLock singleLock(&gDataExchange); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(singleLock.Lock(gDataExchangeTimeInMS))
	{
		theLoadedDatasOut.swap(gLoadedDatas);
		gLoadedDatas.clear(); // tyhjennet‰‰n viel‰ varmuuden vuoksi
		return theLoadedDatasOut.empty() == false;
	}
	return false;
}

static void AddLoadedData(LoadedQueryDataHolder &&theLoadedData)
{
	CSingleLock singleLock(&gDataExchange); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(singleLock.Lock(gDataExchangeTimeInMS))
	{
        NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
        gLoadedDatas.push_back(std::move(theLoadedData));
		AfxGetMainWnd()->PostMessage(ID_MESSAGE_WORKING_THREAD_DATA_READ2); // l‰hetet‰‰n tieto ett‰ nyt on data luettu k‰ytett‰v‰ksi
	}
}

static std::list<std::pair<std::string, std::time_t>> GetLatestTimeSortedFilenames(const std::string& fileFilter, time_t limitingTimeStamp)
{
	auto filesWithTimes = NFmiFileSystem::PatternFiles(fileFilter, limitingTimeStamp);
	if(filesWithTimes.empty())
		return filesWithTimes;

	// J‰rjestet‰‰n tiedostot ajan suhteen laskevassa j‰rjestyksess‰, jolloin uusimmat ovat listan k‰rjess‰
	return CtrlViewUtils::TimeSortFiles(filesWithTimes);
}

static std::unique_ptr<NFmiQueryData> TryReadingFirstGoodBackupData(const std::list<std::pair<std::string, std::time_t>>& backupSortedDataFilenames, const std::string& fileFilter, std::string& fileNameOut)
{
	for(const auto& filenameTimeStampPair : backupSortedDataFilenames)
	{
		try
		{
			const auto& dataFilename = filenameTimeStampPair.first;
			auto data = QueryDataReading::ReadDataFromFile(dataFilename, true);
			if(data)
			{
				std::string logMessage = "Reading older data file '";
				logMessage += dataFilename;
				logMessage += "' for backup (file-filter ";
				logMessage += fileFilter;
				logMessage += ")";
				CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Data);
				fileNameOut = dataFilename;
				return data;
			}
		}
		catch(...)
		{
		}
	}

	std::string logMessage = "Couldn't find any readable older data file for backup to file-filter: ";
	logMessage += fileFilter;
	CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Data);
	return nullptr;
}

static std::unique_ptr<NFmiQueryData> ReadLatestAcceptableDataWithFileFilterAfterTimeStamp(const std::string& fileFilter, time_t limitingTimeStamp, std::string& fileNameOut, time_t& timeStampOut)
{
	try
	{
		// Yritet‰‰n lukea viimeisin data annetulla fileFilter:ill‰ ja palautetaan data
		return QueryDataReading::ReadLatestDataWithFileFilterAfterTimeStamp(fileFilter, limitingTimeStamp, fileNameOut, timeStampOut);
	}
	catch(...)
	{
		// Jos viimeisin datatiedosto oli korruptoitunut ja ollaan gFirstTimeGoingThrough moodissa, yritet‰‰n lukea joku vanhempi data tiedostoista
		if(gFirstTimeGoingThrough)
		{
			// Haetaan kaikki fileFilter:iin liittyneet tiedostot j‰rjestettyn‰ uudesta vanhaan
			auto timeSortedFiles = ::GetLatestTimeSortedFilenames(fileFilter, limitingTimeStamp);
			if(timeSortedFiles.size() > 1)
			{
				// Jos tiedostoja oli enemm‰n kuin 1, kokeillaan lukea jotain vanhempaa jos ne datat eiv‰t olisi korruptoituneita
				// Poistetaan viimeisin, koska sit‰ on jo yritetty lukea
				timeSortedFiles.pop_front();
				auto data = TryReadingFirstGoodBackupData(timeSortedFiles, fileFilter, fileNameOut);
				// Vain jos backup data saatiin oikeasti luettua, palautetaan se, muuten menn‰‰n poikkeusk‰sittelyihin
				if(data)
					return data;
			}
		}

		// Jos backup tiedoston luku ei onnistunut tai ei olla moodissa miss‰ 
		// niit‰ yritet‰‰n lukea, heitet‰‰n vain jo heitetty poikkeus uudestaan
		throw;
	}
}

// Kaatumisraportit ovat kertoneet ett‰ latlon cachen alustus on kaatanut smartmetia usein.
// Lokitetaan ett‰ mik‰ data ja kuinka iso vector<NFmiPoint> on pit‰nyt alustaa, kaatuu vector<NFmiPoint>::reserve kutsuun.
static void LogLatlonCacheInitialization(NFmiQueryData& data, const std::string & latestFileName)
{
	std::string logMessage = "Starting to initialize latlon-cache after reading queryData file ";
	logMessage += latestFileName;
	logMessage += " with location count of ";
	logMessage += std::to_string(data.Info()->SizeLocations());
	CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Data, true);
}

// Lukee qdatan, jos on tullut uusi.
// Laittaa sen luettujen datojen listaan.
// Palauttaa 0, jos ei ollut uutta dataa.
// Palauttaa 1:n, jos oli uutta dataa ja luku onnistui.
// Plauttaa 2:n jos oli uutta dataa, mutta oli jotain ongelmia datan luvun kanssa (muisti ei riit‰, data korruptoitunut jne.)
static int ReadData(NFmiHelpDataInfo &theDataInfo, const NFmiHelpDataInfoSystem &theHelpDataSystem)
{
	int status = 0;
	if(theDataInfo.DataType() != NFmiInfoData::kSatelData) // satelliitti data on kuva, eik‰ sit‰ ladata t‰‰ll‰
	{
		string fileFilter(theDataInfo.UsedFileNameFilter(theHelpDataSystem));
		if(theDataInfo.IsCombineData())
			fileFilter = theDataInfo.CombinedResultDataFileFilter(); // jos kyse yhdistelem‰ll‰ rakennetusta datasta, k‰ytet‰‰n t‰t‰ lataamaan data (FileNameFilter:in avulla haettiin koottavat tiedostot)
		time_t timeStamp = 0;
		time_t latestTimeStamp = theDataInfo.LatestFileTimeStamp();
		string latestFileName;
		try
		{
			std::unique_ptr<NFmiQueryData> data = ::ReadLatestAcceptableDataWithFileFilterAfterTimeStamp(fileFilter, latestTimeStamp, latestFileName, timeStamp);
            if(data)
            {
				::LogLatlonCacheInitialization(*data, latestFileName);
                data->LatLonCache(); // T‰m‰ alustaa latlon cachen worker threadissa, jotta se olisi sitten k‰ytˆss‰ SmartMetissa multi-thread koodeissa
                if(theDataInfo.FakeProducerId() != 0)
                {
                    data->Info()->First();
                    NFmiProducer kepaProducer(*data->Info()->Producer());
                    kepaProducer.SetIdent(theDataInfo.FakeProducerId());
                    data->Info()->SetProducer(kepaProducer);
                }

                LoadedQueryDataHolder tmp;
                tmp.itsQueryData = std::move(data);
                tmp.itsDataFileName = latestFileName;
                tmp.itsDataFilePattern = fileFilter;
                tmp.itsDataType = theDataInfo.DataType();

                theDataInfo.LatestFileTimeStamp(timeStamp);
                theDataInfo.LatestFileName(latestFileName);
                ::MakeDataNotificationTexts(gWorkerDataNotificationSettings, gWorkerUsedLanguage, tmp, theDataInfo);
                status = 1;
                ::AddLoadedData(std::move(tmp));
            }
		}
        catch(...)
        {
            status = 2;
            // QueryDatan luku ep‰onnistunut syyst‰ tai toisesta.
            // Ongelmasta on jo lokitettu tarkempi analyysi QueryDataReading::ReadLatestDataWithFileFilterAfterTimeStamp funktiossa.
            // Datatiedostoa ei haluta siirt‰‰ pois viallisena error-hakemistoon, eik‰ siit‰ haluta raportoida minuutin
            // v‰lein eli laitetaan viallisen tiedoston timestamp talteen.
            theDataInfo.LatestFileTimeStamp(timeStamp);
        }
	}
	return status;
}

// Yrit‰‰ lukea kaikki dynaamiset data (jos tullut uusia).
// Palauttaa jos, jos ei lukenut yht‰‰n mit‰‰n, muuten palauttaa 1:n.
static int GoThroughAllHelpDataInfos(NFmiHelpDataInfoSystem &theHelpDataSystem)
{
	::ApplyChangedSettings(); // katsotaan onko asetuksia muutettu
	int status = 0;
	for(size_t i=0; i < theHelpDataSystem.DynamicHelpDataInfos().size(); i++)
	{
		NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
		NFmiHelpDataInfo &dataInfo = theHelpDataSystem.DynamicHelpDataInfo(static_cast<int>(i));
		if(dataInfo.IsEnabled() && dataInfo.IsDataUsedCaseStudyChecks(gIsInCaseStudyMode))
		{
			if(::ReadData(dataInfo, theHelpDataSystem) == 1)
				status = 1;
		}
	}

	return status;
}

// t‰m‰ pakottaa ett‰ datan luku tehd‰‰n heti
void CFmiDataLoadingThread2::LoadDataNow(void)
{
	gDoDataCheckNow = true;
}

void CFmiDataLoadingThread2::ResetTimeStamps(void)
{
	gResetTimeStamps = true;
	gSettingsHaveChanged = true;
}

UINT CFmiDataLoadingThread2::DoThread(LPVOID /* pParam */ )
{
	CSingleLock singleLock(&gThreadRunning); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(5000)) // Attempt to lock the shared resource, 5000 means 5 sec wait, 0 wait resulted sometimes to wait for next minute for unknown reason
	{
//		DebugThread(logger, "CFmiDataLoadingThread2::DoThread was allready running, stopping...");
		return 1;
	}

	NFmiMilliSecondTimer timer;
	int loadingStatus = 0;

	// T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
	// Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
	int counter = 0;
	try
	{
		for( ; ; counter++)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);

			if(gFirstTimeGoingThrough || gDoDataCheckNow || loadingStatus || timer.CurrentTimeDiffInMSeconds() > (30 * 1000)) // tarkistetaan v‰hint‰‰n puolen minuutin v‰lein onko tullut uusia datoja
			{
				NFmiInfoOrganizer::MarkLoadedDataAsOld(gFirstTimeGoingThrough);
				gDoDataCheckNow = false;
				try
				{
					loadingStatus = ::GoThroughAllHelpDataInfos(gWorkerHelpDataSystem);
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

				if(gFirstTimeGoingThrough)
				{
					// Jos oltiin tekem‰ss‰ 1. ajo kertaa, nukutaan v‰h‰n aikaa, ennen kuin laitetaan lippu 
					// pois p‰‰lt‰, jotta systeemi ehtii ottaa kierroksella tulleet datat k‰yttˆˆn ennen moodin vaihtoa,
					Sleep(1 * 1000);
				}
				gFirstTimeGoingThrough = false;
				NFmiInfoOrganizer::MarkLoadedDataAsOld(gFirstTimeGoingThrough);
				timer.StartTimer(); // aloitetaan taas uusi ajan lasku
			}

			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
			Sleep(1*1000); // nukutaan aina lyhyit‰ aikoja (1 s), ett‰ osataan tutkia usein, joska p‰‰ohjelma haluaa jo sulkea
		}
	}
	catch(...)
	{
		// t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
	}

    return 0;   // thread completed successfully
}

void CFmiDataLoadingThread2::ResetFirstTimeGoingThroughState()
{
	gFirstTimeGoingThrough = true;
}

void CFmiDataLoadingThread2::SetCaseStudyMode(bool isInCaseStudyMode)
{
	gIsInCaseStudyMode = isInCaseStudyMode;
}
