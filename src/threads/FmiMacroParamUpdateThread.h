#pragma once

#include "stdafx.h"
#include <string>
#include <memory>

class NFmiMacroParamSystem;

namespace CFmiMacroParamUpdateThread
{
	// t‰m‰ pit‰‰ kutsua ennen kuin threadi (DoThread) k‰ynnistet‰‰n
	void InitMacroParamSystem(std::shared_ptr<NFmiMacroParamSystem> theMacroParamSystemPtr);
	UINT DoThread(LPVOID pParam);
	void CloseNow(void);
	int WaitToClose(int theMilliSecondsToWait);
	void ForceUpdate();
	void SetFirstTimeDelay(int theStartUpDelayInMS);

	bool MakePossibleMacroParamSystemUpdateInDoc(std::shared_ptr<NFmiMacroParamSystem> theMacroParamSystemPtr);
}

