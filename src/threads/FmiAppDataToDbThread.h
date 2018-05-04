#pragma once

#include "stdafx.h"

namespace CFmiAppDataToDbThread
{
	UINT DoThread(LPVOID pParam);
	void CloseNow(void);
	int WaitToClose(int theMilliSecondsToWait);
}

