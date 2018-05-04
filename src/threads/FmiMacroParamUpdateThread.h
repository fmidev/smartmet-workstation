#pragma once

#include "stdafx.h"
#include <string>

class NFmiMacroParamSystem;

namespace CFmiMacroParamUpdateThread
{
	void InitMacroParamSystem(NFmiMacroParamSystem &theMacroParamSystem); // t‰m‰ pit‰‰ kutsua ennen kuin threadi (DoThread) k‰ynnistet‰‰n
	UINT DoThread(LPVOID pParam);
	void CloseNow(void);
	int WaitToClose(int theMilliSecondsToWait);
	void ForceUpdate(const std::string *theRootFolder = 0);
	void SetFirstTimeDelay(int theStartUpDelayInMS);

	// T‰ss‰ on 'lukolliset' macroParamien vaihto metodit. Add on worker-threadin k‰ytt‰m‰ (talleta ett‰ p‰‰ohjelma voi hakea ne)
	// ja Get on p‰‰ ohjelman k‰ytt‰m‰ (hae worker threadin tekem‰ lista k‰yttˆˆn).
	void AddMacroParams(NFmiMacroParamSystem &theMacroParamSystem);
	bool GetMacroParams(NFmiMacroParamSystem &theMacroParamSystem);
}

