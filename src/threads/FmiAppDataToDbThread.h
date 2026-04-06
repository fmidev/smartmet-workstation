#pragma once

#ifndef UNIX
#include "stdafx.h"

namespace CFmiAppDataToDbThread
{
	UINT DoThread(LPVOID pParam);
	void CloseNow(void);
	int WaitToClose(int theMilliSecondsToWait);
}
#endif // UNIX

