#pragma once

#include "stdafx.h"

class NFmiSeaIcingWarningSystem;

namespace CFmiHakeWarningMessages
{
    void UpdateApplicationAfterChanges();
}

namespace CFmiSeaIcingMessageThread
{
	void InitSeaIcingMessageInfo(const NFmiSeaIcingWarningSystem &seaIcingWarningSystem); // t‰m‰ pit‰‰ kutsua ennen kuin threadi (DoThread) k‰ynnistet‰‰n
	UINT DoThread(LPVOID pParam);
	void CloseNow(void);
	int WaitToClose(int theMilliSecondsToWait);
	void SetFirstTimeDelay(int theStartUpDelayInMS);

	// t‰ss‰ viel‰ lukolliset sea-icing sanomien v‰liset mekanismit
	void AddNewSeaIcingMessages(NFmiSeaIcingWarningSystem &theSeaIcingSystem);
	bool GetNewSeaIcingMessages(NFmiSeaIcingWarningSystem &theSeaIcingSystem);
}

