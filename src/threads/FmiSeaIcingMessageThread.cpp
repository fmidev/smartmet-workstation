
#include "FmiSeaIcingMessageThread.h"
#include "SmartMetThreads_resource.h"
#include "afxmt.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiSeaIcingWarningSystem.h"
#include "NFmiMilliSecondTimer.h"

// TODO Mit‰ jos k‰ytt‰j‰ s‰‰t‰‰ sanomien lukua kesken ajon, pit‰‰kˆ ohjelma k‰ynnist‰‰ uudestaan, vai tehd‰‰nkˆ muutokset lennossa?

namespace
{
	CSemaphore gSeaIcingMessageThreadRunning; // t‰m‰n avulla yritet‰‰n lopettaan jatkuvasti pyˆriv‰ working thread 'siististi'
	NFmiStopFunctor gSeaIcingMessageThreadStopFunctor;
	int gStartUpDelayInMS = 0; // t‰m‰n avulla voidaan s‰‰t‰‰ kuinka kauan alussa odotellaan, ennen kuin tehd‰‰n tyˆt ensimm‰isen kerran

	NFmiSeaIcingWarningSystem gSeaIcingWarningSystem; // T‰m‰n olion avulla luetaan working threadissa uusia sanomia.

	// T‰ss‰ on suojattu sea-icing sanomien v‰lityspaikka. T‰h‰n ei saa koskea kuin 
	// AddNewSeaIcingMessages ja GetNewSeaIcingMessages -metodien kautta (mieti miten t‰m‰n saisi tehty‰ fiksusti).
	std::set<NFmiSeaIcingWarningMessage> gSeaIcingMessages;
	time_t gLatestSeaIcingTimeStamp; // otetaan t‰m‰kin varmuuden vuoksi talteen, mutta t‰ll‰ ei oikeasti tee mit‰‰n
	CSemaphore gSeaIcingMessagesSemaphore; // t‰m‰ on lukko, jolla estet‰‰n samanaikainen luku/kirjoitus gHAKEMessages-oliosta.
}

static UINT PostMessageThread(LPVOID pParam)
{
    return ::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ID_MESSAGE_NEW_HAKE_WARNING_AVAILABLE, 0, 0);
}

void CFmiHakeWarningMessages::UpdateApplicationAfterChanges()
{
    // HakeWarning systeemi pyˆrii c++11 std::thread:issa. Sille annetaan t‰m‰ updatefunktio callbackin‰.
    // Jostain syyst‰ std::thread:ista l‰hetetyt PostMessage:t joko kaatuvat tai eiv‰t mene perille CMainFrm:een.
    // Siksi on luotava erillinen CWinThread, joka tekee PostMessage kutsun, koska siit‰ sanomat menev‰t perille.
    CWinThread *postMessageThread = AfxBeginThread(::PostMessageThread, nullptr, THREAD_PRIORITY_NORMAL);
}

void CFmiSeaIcingMessageThread::SetFirstTimeDelay(int theStartUpDelayInMS)
{
	gStartUpDelayInMS = theStartUpDelayInMS;
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

static void	ReadNewSeaIcingWarnings(void)
{
	// 1. Read possible new sea-icing warning mwessages and store them in thread-safe storage.
	gSeaIcingWarningSystem.CheckForNewMessages(&gSeaIcingMessageThreadStopFunctor);
	// 2. Store messages to thread-safe container
	NFmiQueryDataUtil::CheckIfStopped(&gSeaIcingMessageThreadStopFunctor);
	if(gSeaIcingWarningSystem.GetMessages().size())
	{ // jos oli lˆytynyt uusia sanomia, laitetaan ne jakoon ja l‰hetet‰‰n p‰‰ohjelmalle viesti‰
        CFmiSeaIcingMessageThread::AddNewSeaIcingMessages(gSeaIcingWarningSystem);
		// 3. Clear messages from local worker object
		gSeaIcingWarningSystem.Clear(true);
		// 4. Send message to main program that there are new messages available
		NFmiQueryDataUtil::CheckIfStopped(&gSeaIcingMessageThreadStopFunctor);
		AfxGetMainWnd()->PostMessage(ID_MESSAGE_NEW_SEA_ICING_WARNING_AVAILABLE); // l‰hetet‰‰n tieto ett‰ nyt on data luettu k‰ytett‰v‰ksi
	}
}

UINT CFmiSeaIcingMessageThread::DoThread(LPVOID /* pParam */ )
{
	CSingleLock singleLock(&gSeaIcingMessageThreadRunning); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(5000)) // Attempt to lock the shared resource, 5000 means 5 sec wait, 0 wait resulted sometimes to wait for next minute for unknown reason
	{
		return 1;
	}

	if(!gSeaIcingWarningSystem.DoMessageSearching())
		return 1; // ei haluta lukea seaice-sanomia (tai niit‰ ei ole)

	NFmiMilliSecondTimer timer;
	bool firstTime = true;

	// T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
	// Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
	int counter = 0;
	try
	{
		for( ; ; counter++)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gSeaIcingMessageThreadStopFunctor);

			if(::LetGoAfterFirstTimeDelaying(timer, firstTime, gStartUpDelayInMS) || timer.CurrentTimeDiffInMSeconds() > (gSeaIcingWarningSystem.UpdateTimeStepInMinutes() * 60 * 1000))
			{ // jos on kulunut tarpeeksi aikaa, tarkastetaan, onko jonnekin tullut uusia datatiedostoja jotka pit‰‰ yhdist‰‰
				firstTime = false;
				try
				{
					::ReadNewSeaIcingWarnings();
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

			NFmiQueryDataUtil::CheckIfStopped(&gSeaIcingMessageThreadStopFunctor);
			Sleep(1*1000); // nukutaan aina lyhyit‰ aikoja (1 s), ett‰ osataan tutkia usein, joska p‰‰ohjelma haluaa jo sulkea
		}
	}
	catch(...)
	{
		// t‰m‰ oli luultavasti StopThreadException, lopetetaan joka tapauksessa
	}

    return 0;   // thread completed successfully
}

// T‰t‰ initialisointi funktiota pit‰‰ kutsua ennen kuin itse threadi k‰ynnistet‰‰n MainFramesta. 
void CFmiSeaIcingMessageThread::InitSeaIcingMessageInfo(const NFmiSeaIcingWarningSystem &seaIcingWarningSystem)
{
	gSeaIcingWarningSystem.Init(seaIcingWarningSystem, true); // otetaan asetukset talteen lokaaliin warningCenter-olioon
}

void CFmiSeaIcingMessageThread::CloseNow(void)
{
    gSeaIcingMessageThreadStopFunctor.Stop(true);
}

int CFmiSeaIcingMessageThread::WaitToClose(int theMilliSecondsToWait)
{
	CSingleLock singleLock(&gSeaIcingMessageThreadRunning);
	if(singleLock.Lock(theMilliSecondsToWait))
		return 1;
	return 0;
}

void CFmiSeaIcingMessageThread::AddNewSeaIcingMessages(NFmiSeaIcingWarningSystem &theSeaIcingSystem)
{
	CSingleLock singleLock(&gSeaIcingMessagesSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(1000)) // Attempt to lock the shared resource for 1 second
	{
		return ; // oli jo lukossa eik‰ auennut tarpeeksi nopeasti, lopetataan toistaiseksi
	}

	std::set<NFmiSeaIcingWarningMessage> &tmpMsgList = theSeaIcingSystem.GetMessages();
	gSeaIcingMessages.insert(tmpMsgList.begin(), tmpMsgList.end()); // talletetaan workerin lista jakelu listaan
	gLatestSeaIcingTimeStamp = theSeaIcingSystem.LatestReadFileTimeStamp();
	theSeaIcingSystem.Clear(true); // lopuksi tyhjennet‰‰n worker-olion lista sanomista
}

bool CFmiSeaIcingMessageThread::GetNewSeaIcingMessages(NFmiSeaIcingWarningSystem &theSeaIcingSystem)
{
	CSingleLock singleLock(&gSeaIcingMessagesSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(1000)) // Attempt to lock the shared resource for 1 second
	{
		return false; // oli jo lukossa eik‰ auennut tarpeeksi nopeasti, lopetataan toistaiseksi
	}

	bool status = gSeaIcingMessages.empty() == false;
	if(status)
	{
		theSeaIcingSystem.Add(gSeaIcingMessages, gLatestSeaIcingTimeStamp);
		gSeaIcingMessages.clear(); // kun sanomat on saatu p‰‰ohjelmaan, tuhotaan ne t‰‰lt‰
	}
	return status;
}
