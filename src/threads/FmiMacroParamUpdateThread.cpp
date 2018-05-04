
#include "FmiMacroParamUpdateThread.h"
#include "SmartMetThreads_resource.h"
#include "afxmt.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMilliSecondTimer.h"

namespace
{
	CSemaphore gThreadRunning; // t‰m‰n avulla yritet‰‰n lopettaan jatkuvasti pyˆriv‰ working thread 'siististi'
	NFmiStopFunctor gStopFunctor;
	bool fDoUpdateNow; // t‰ll‰ voidaan pakottaa macroParam update v‰littˆm‰sti
	std::string gNewRootFolder;
	int gStartUpDelayInMS = 0; // t‰m‰n avulla voidaan s‰‰t‰‰ kuinka kauan alussa odotellaan, ennen kuin tehd‰‰n tyˆt ensimm‰isen kerran

	NFmiMacroParamSystem gWorkerMacroParamSystem; // T‰m‰n olion avulla p‰ivitet‰‰n working threadissa makroParam listaa.

	// N‰ill‰ on suojattu macroParamien v‰lityspaikka. T‰h‰n ei saa koskea kuin 
	// AddMacroParams ja GetMacroParams -metodien kautta (mieti miten t‰m‰n saisi tehty‰ fiksusti).
	NFmiMacroParamSystem gMediatorMacroParamSystem; // T‰m‰n v‰litt‰j‰-olion avulla siirret‰‰n macroParameja working-threadin ja p‰‰ohjelman v‰lill‰.
	CSemaphore gDataTranferSemaphore; // t‰m‰ on lukko, jolla estet‰‰n samanaikainen luku/kirjoitus gHAKEMessages-oliosta.
}

void CFmiMacroParamUpdateThread::SetFirstTimeDelay(int theStartUpDelayInMS)
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

static void	RebuildMacroParams(void)
{
	NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
	// 0. aseta mahdollinen root-hakemiston muutos
	if(gNewRootFolder.empty() == false) // jos p‰‰ohjelmalta on tullut uusi macroParam-polku, asetetaan se t‰ss‰.
	{
		gWorkerMacroParamSystem.RootPath(gNewRootFolder);
		CSingleLock singleLock(&gDataTranferSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
		if(singleLock.Lock(1*1000)) // Attempt to lock the shared resource for 1 second
			gMediatorMacroParamSystem.RootPath(gNewRootFolder);
		gNewRootFolder = ""; // pit‰‰ nollata arvo ett‰ ensi kerralla ei aseteta t‰t‰ taas uudestaan.
	}
	// 1. rebuild makros in worker object
	gWorkerMacroParamSystem.Rebuild(&gStopFunctor);
	NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
	// 2. Put rebuild-macrParams in mediator-object
	CFmiMacroParamUpdateThread::AddMacroParams(gWorkerMacroParamSystem);
	NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
	AfxGetMainWnd()->PostMessage(ID_MESSAGE_MACRO_PARAMS_UPDATE); // l‰hetet‰‰n tieto ett‰ nyt on data luettu k‰ytett‰v‰ksi
}

UINT CFmiMacroParamUpdateThread::DoThread(LPVOID /* pParam */ )
{
	CSingleLock singleLock(&gThreadRunning); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(5000)) // Attempt to lock the shared resource, 5000 means 5 sec wait, 0 wait resulted sometimes to wait for next minute for unknown reason
	{
		return 1;
	}

	NFmiMilliSecondTimer timer;
	bool firstTime = true;
	gNewRootFolder = "";

	// T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
	// Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
	int counter = 0;
	try
	{
		for( ; ; counter++)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);

			if(::LetGoAfterFirstTimeDelaying(timer, firstTime, gStartUpDelayInMS) || fDoUpdateNow || timer.CurrentTimeDiffInMSeconds() > (60 * 60 * 1000)) // tarkistetaan 60 minuutin v‰lein onko tullut muutoksia makroParameihin
			{ // jos on kulunut tarpeeksi aikaa, tarkastetaan, onko jonnekin tullut uusia datatiedostoja jotka pit‰‰ yhdist‰‰
				firstTime = false;
				fDoUpdateNow = false;
				try
				{
					::RebuildMacroParams();
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

// T‰t‰ initialisointi funktiota pit‰‰ kutsua ennen kuin itse threadi k‰ynnistet‰‰n MainFramesta. 
void CFmiMacroParamUpdateThread::InitMacroParamSystem(NFmiMacroParamSystem &theMacroParamSystem)
{
	gWorkerMacroParamSystem.Init(theMacroParamSystem);

	CSingleLock singleLock(&gDataTranferSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(singleLock.Lock(1*1000)) // Attempt to lock the shared resource for 1 second
		gMediatorMacroParamSystem.Init(theMacroParamSystem);
}

void CFmiMacroParamUpdateThread::CloseNow(void)
{
	gStopFunctor.Stop(true);
}

void CFmiMacroParamUpdateThread::ForceUpdate(const std::string *theRootFolder)
{
	fDoUpdateNow = true;
	if(theRootFolder)
		gNewRootFolder = *theRootFolder;
	else
		gNewRootFolder = "";
}

int CFmiMacroParamUpdateThread::WaitToClose(int theMilliSecondsToWait)
{
	CSingleLock singleLock(&gThreadRunning);
	if(singleLock.Lock(theMilliSecondsToWait))
		return 1;
	return 0;
}

	// T‰ss‰ on 'lukolliset' sanomien vaihto metodit. Add on worker-threadin k‰ytt‰m‰ ja
	// Get on p‰‰ ohjelman k‰ytt‰m‰.
void CFmiMacroParamUpdateThread::AddMacroParams(NFmiMacroParamSystem &theMacroParamSystem)
{
	CSingleLock singleLock(&gDataTranferSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(1000)) // Attempt to lock the shared resource for 1 second
	{
		return ; // oli jo lukossa eik‰ auennut tarpeeksi nopeasti, lopetataan toistaiseksi
	}

	gMediatorMacroParamSystem.SwapMacroData(theMacroParamSystem);
}

bool CFmiMacroParamUpdateThread::GetMacroParams(NFmiMacroParamSystem &theMacroParamSystem)
{
	CSingleLock singleLock(&gDataTranferSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(1000)) // Attempt to lock the shared resource for 1 second
	{
		return false; // oli jo lukossa eik‰ auennut tarpeeksi nopeasti, lopetataan toistaiseksi
	}

	if(theMacroParamSystem.IsUpdateNeeded(gMediatorMacroParamSystem))
	{
		theMacroParamSystem.SwapMacroData(gMediatorMacroParamSystem);
		return true;
	}
	return false;
}
