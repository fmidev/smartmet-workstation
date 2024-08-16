
#include "FmiMacroParamUpdateThread.h"
#include "SmartMetThreads_resource.h"
#include "afxmt.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMilliSecondTimer.h"
#include "CtrlViewDocumentInterface.h"
#include <mutex>

namespace
{
	CSemaphore gThreadRunning; // t‰m‰n avulla yritet‰‰n lopettaan jatkuvasti pyˆriv‰ working thread 'siististi'
	NFmiStopFunctor gStopFunctor;
	bool fDoUpdateNow; // t‰ll‰ voidaan pakottaa macroParam update v‰littˆm‰sti
	int gStartUpDelayInMS = 0; // t‰m‰n avulla voidaan s‰‰t‰‰ kuinka kauan alussa odotellaan, ennen kuin tehd‰‰n tyˆt ensimm‰isen kerran

	std::unique_ptr<NFmiMacroParamSystem> gWorkerMacroParamSystemPtr; // T‰m‰n olion avulla p‰ivitet‰‰n working threadissa makroParam listaa.

	// N‰ill‰ on suojattu macroParamien v‰lityspaikka. T‰h‰n ei saa koskea kuin 
	// AddMacroParams ja GetMacroParams -metodien kautta (mieti miten t‰m‰n saisi tehty‰ fiksusti).
	std::unique_ptr<NFmiMacroParamSystem> gMediatorMacroParamSystemPtr; // T‰m‰n v‰litt‰j‰-olion avulla siirret‰‰n macroParameja working-threadin ja p‰‰ohjelman v‰lill‰.
	std::mutex gDataTranferMutex; // t‰m‰ on lukko, jolla estet‰‰n samanaikainen luku/kirjoitus.
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

static void AddMacroParams(std::unique_ptr<NFmiMacroParamSystem>& theMacroParamSystemPtr)
{
	std::lock_guard<std::mutex> lockGuard(gDataTranferMutex);
	gMediatorMacroParamSystemPtr.swap(theMacroParamSystemPtr);
}

static void	RebuildMacroParams(void)
{
	NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
	// 1. rebuild makros in worker object
	gWorkerMacroParamSystemPtr->Rebuild(&gStopFunctor);
	NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);
	// 2. Put rebuild-macrParams in mediator-object
	AddMacroParams(gWorkerMacroParamSystemPtr);
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
	int usedUpdateTimeIntervalInMilliSeconds = 4 * 60 * 1000; // tehd‰‰n tunnin v‰lein

	// T‰ss‰ on iki-looppi, jossa vahditaan onko tullut uusia datoja, jolloin tehd‰‰n yhdistelm‰ datoja SmartMetin luettavaksi.
	// Lis‰ksi pit‰‰ tarkkailla, onko tullut lopetus k‰sky, joloin pit‰‰ siivota ja lopettaa.
	int counter = 0;
	try
	{
		for( ; ; counter++)
		{
			NFmiQueryDataUtil::CheckIfStopped(&gStopFunctor);

			if(::LetGoAfterFirstTimeDelaying(timer, firstTime, gStartUpDelayInMS) || fDoUpdateNow || timer.CurrentTimeDiffInMSeconds() > usedUpdateTimeIntervalInMilliSeconds)
			{ 
				// jos on kulunut tarpeeksi aikaa, tarkastetaan, onko jonnekin tullut uusia datatiedostoja jotka pit‰‰ yhdist‰‰
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
void CFmiMacroParamUpdateThread::InitMacroParamSystem(std::shared_ptr<NFmiMacroParamSystem> theMacroParamSystemPtr)
{
	gWorkerMacroParamSystemPtr = std::make_unique<NFmiMacroParamSystem>();
	gWorkerMacroParamSystemPtr->Init(*theMacroParamSystemPtr);

	{
		std::lock_guard<std::mutex> lockGuard(gDataTranferMutex);
		gMediatorMacroParamSystemPtr = std::make_unique<NFmiMacroParamSystem>();
		gMediatorMacroParamSystemPtr->Init(*theMacroParamSystemPtr);
	}
}

void CFmiMacroParamUpdateThread::CloseNow(void)
{
	gStopFunctor.Stop(true);
}

void CFmiMacroParamUpdateThread::ForceUpdate()
{
	fDoUpdateNow = true;
}

int CFmiMacroParamUpdateThread::WaitToClose(int theMilliSecondsToWait)
{
	CSingleLock singleLock(&gThreadRunning);
	if(singleLock.Lock(theMilliSecondsToWait))
		return 1;
	return 0;
}

bool CFmiMacroParamUpdateThread::MakePossibleMacroParamSystemUpdateInDoc(std::shared_ptr<NFmiMacroParamSystem> theMacroParamSystemPtr)
{
	std::lock_guard<std::mutex> lockGuard(gDataTranferMutex);

	if(theMacroParamSystemPtr->IsUpdateNeeded(*gMediatorMacroParamSystemPtr))
	{
		// Siirret‰‰n p‰ivitetty macroParamSystem otus tmp-shared_ptr muuttujalle
		std::shared_ptr<NFmiMacroParamSystem> updatedMacroParamSystemPtr(gMediatorMacroParamSystemPtr.release());
		// Luodaan uusi pohja mediator otukselle
		gMediatorMacroParamSystemPtr = std::make_unique<NFmiMacroParamSystem>();
		gMediatorMacroParamSystemPtr->Init(*theMacroParamSystemPtr);
		// Asetetaan p‰ivitetty macroParamSystem otus dokumentille k‰yttˆˆn
		CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateMacroParamSystemContent(std::move(updatedMacroParamSystemPtr));
		return true;
	}
	return false;
}
