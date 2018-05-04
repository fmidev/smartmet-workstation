
#include "FmiAppDataToDbThread.h"
#include "NFmiApplicationDataBase.h"
#include "SmartMetThreads_resource.h"
#include "catlog/catlog.h"
#include "afxmt.h"

namespace
{
	CSemaphore gThreadRunningStart;
	CSemaphore gThreadRunningUpdate;
	CSemaphore gThreadRunningStop;

//	NFmiStopFunctor gAppDataToDbThreadStopFunctor; // ei saa eik‰ voi lopettaa http-l‰hetyst‰!!

}

UINT CFmiAppDataToDbThread::DoThread(LPVOID pParam)
{
	NFmiApplicationDataBase *applicationDataBase = (NFmiApplicationDataBase *)pParam;
	if(applicationDataBase == 0)
		return 1;

	CSemaphore *usedSemaphore = 0;
	if(applicationDataBase->itsAction == NFmiApplicationDataBase::kStart) 
		usedSemaphore = &gThreadRunningStart;
	else if(applicationDataBase->itsAction == NFmiApplicationDataBase::kUpdate) 
		usedSemaphore = &gThreadRunningUpdate;
	else if(applicationDataBase->itsAction == NFmiApplicationDataBase::kClose) 
		usedSemaphore = &gThreadRunningStop;

	if(usedSemaphore == 0)
		return 1;

	CSingleLock singleLock(usedSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(!singleLock.Lock(5000)) // Attempt to lock the shared resource, 5000 means 5 sec wait, 0 wait resulted sometimes to wait for next minute for unknown reason
	{
//		DebugThread(logger, "CFmiAppDataToDbThread::DoThread oli jo lukittuna, lopetetaan...");
		return 1;
	}
	
	std::string responseStr;
	if(NFmiApplicationDataBase::SendSmartMetDataToDB(*applicationDataBase, responseStr))
	    return 0;   // thread completed successfully
    else
    {
        std::string logMsg = "Error while sending system data to database: ";
        logMsg += responseStr;
        CatLog::logMessage(logMsg, CatLog::Severity::Error, CatLog::Category::NetRequest);
        return 1;   // thread completed unsuccessfully
    }
}

void CFmiAppDataToDbThread::CloseNow(void)
{
//	gHAKEWarningThreadStopFunctor.Stop(true);
}

int CFmiAppDataToDbThread::WaitToClose(int theMilliSecondsToWait)
{
	CSingleLock singleLock1(&gThreadRunningStart);
	if(singleLock1.Lock(theMilliSecondsToWait))
	{
		CSingleLock singleLock2(&gThreadRunningUpdate);
		if(singleLock2.Lock(theMilliSecondsToWait))
		{
			CSingleLock singleLock3(&gThreadRunningStop);
			if(singleLock3.Lock(theMilliSecondsToWait))
			{
				return 1; // vain jos kaikki kolme eri mahdollista datan l‰hetyst‰ ovat loppuneet, on t‰m‰ onnistunut...
			}
		}
	}
	return 0; // jos mik‰ tahansa l‰hetyksist‰ on j‰‰nyt p‰‰lle, t‰m‰ ep‰onnistuu...
}

