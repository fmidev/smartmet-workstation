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
	CSemaphore gThreadRunning; // t‰m‰n avulla yritet‰‰n lopettaan jatkuvasti pyˆriv‰ working thread 'siististi'
	NFmiStopFunctor gStopFunctor;

	NFmiHelpDataInfoSystem gWorkerHelpDataSystem; // T‰m‰n olion avulla working thread osaa lukea haluttuja datoja
    NFmiDataNotificationSettingsWinRegistry gWorkerDataNotificationSettings; // T‰m‰n avulla tehd‰‰n eri datoille halutunlaisia notifikaatio tekstej‰
    FmiLanguage gWorkerUsedLanguage; // Mink‰ kielisi‰ sanakirjaja k‰ytet‰‰n kun tehd‰‰n notifikaatio tekstej‰

	CSemaphore gSettingsChanged; // t‰m‰n avulla p‰ivitet‰‰n datan luku asetuksia thread safetysti
	NFmiHelpDataInfoSystem gMediatorHelpDataSystem; // t‰m‰n avulla s‰‰det‰‰n threadin asetuksia thread safetysti
	bool gSettingsHaveChanged;

	CSemaphore gDataExchange; // t‰m‰n avulla lis‰t‰‰n ja luetaan qdatoja listasta data-threadin ja p‰‰ohjelman v‰lill‰ thread-safesti
	std::vector<LoadedQueryDataHolder> gLoadedDatas;

	bool gDoDataCheckNow = false; // t‰m‰n avulla voidaan pakottaa threadi tarkistamaan datan lukua heti
	bool gResetTimeStamps = false; // t‰m‰n avulla voidaan pakottaa NFmiHelpDataSystem:in kaikkien dynaamisten
	bool gApplyHelpDataInfos = false; // t‰t‰ k‰ytet‰‰n mm. CaseStudy-datan yhteydess‰, t‰ll‰ varmistetaan ett‰ uudet data polut tulevat k‰yttˆˆn
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

// t‰ll‰ p‰‰ohjelma pyyt‰‰ ladattuja datoja (funktiossa tehd‰‰n vector swap!!)
// palauttaa true, jos uutta dataa tuli k‰yttˆˆn, muuten jos lukitus failaa, tai ei ollut dataa, palautetaan false.
bool CFmiDataLoadingThread2::GetLoadedDatas(std::vector<LoadedQueryDataHolder> &theLoadedDatasOut)
{
	CSingleLock singleLock(&gDataExchange); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(singleLock.Lock(1000))
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
	if(singleLock.Lock(3000)) // annetaan 3 sekuntia lukolle aikaa, muuten data menetet‰‰n (pit‰isi riitt‰‰ kevyesti aina)
	{
        NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
        gLoadedDatas.push_back(std::move(theLoadedData));
		AfxGetMainWnd()->PostMessage(ID_MESSAGE_WORKING_THREAD_DATA_READ2); // l‰hetet‰‰n tieto ett‰ nyt on data luettu k‰ytett‰v‰ksi
	}
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
			fileFilter = theDataInfo.CombineDataPathAndFileName(); // jos kyse yhdistelem‰ll‰ rakennetusta datasta, k‰ytet‰‰n t‰t‰ lataamaan data (FileNameFilter:in avulla haettiin koottavat tiedostot)
		time_t timeStamp = 0;
		time_t latestTimeStamp = theDataInfo.LatestFileTimeStamp();
		string latestFileName;
		try
		{
			std::unique_ptr<NFmiQueryData> data = QueryDataReading::ReadLatestDataWithFileFilterAfterTimeStamp(fileFilter, latestTimeStamp, latestFileName, timeStamp);
            if(data)
            {
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
		if(dataInfo.IsEnabled())
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
	bool firstTime = true;
	int loadingStatus = 0;

	// T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
	// Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
	int counter = 0;
	try
	{
		for( ; ; counter++)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);

			if(firstTime || gDoDataCheckNow || loadingStatus || timer.CurrentTimeDiffInMSeconds() > (30 * 1000)) // tarkistetaan v‰hint‰‰n puolen minuutin v‰lein onko tullut uusia datoja
			{
				firstTime = false;
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

