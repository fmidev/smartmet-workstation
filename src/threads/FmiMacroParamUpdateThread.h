#pragma once

#include <string>
#include <memory>

#ifndef UNIX
#include "stdafx.h"
#endif // UNIX

class NFmiMacroParamSystem;

namespace CFmiMacroParamUpdateThread
{
	// t�m� pit�� kutsua ennen kuin threadi (DoThread) k�ynnistet��n
	void InitMacroParamSystem(std::shared_ptr<NFmiMacroParamSystem> theMacroParamSystemPtr);
#ifndef UNIX
	UINT DoThread(LPVOID pParam);
#endif // UNIX
	void CloseNow(void);
	int WaitToClose(int theMilliSecondsToWait);
	void ForceUpdate();
	void SetFirstTimeDelay(int theStartUpDelayInMS);

	bool MakePossibleMacroParamSystemUpdateInDoc(std::shared_ptr<NFmiMacroParamSystem> theMacroParamSystemPtr);
}

