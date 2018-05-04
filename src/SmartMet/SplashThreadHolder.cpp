#include "stdafx.h"
#include "SplashThreadHolder.h"
#include "splasher.h"

CSplashThreadHolder::CSplashThreadHolder(CSplashThread* theSplashThreadPtr)
:itsSplashThreadPtr(theSplashThreadPtr)
,fHidden(false)
{
}

CSplashThreadHolder::~CSplashThreadHolder(void)
{
	HideSplash();
}

// tällä voi halutessa piilottaa sen heti kun haluaa
void CSplashThreadHolder::HideSplash(void)
{
	if(fHidden == false && itsSplashThreadPtr) // HUOM! ei tarvitse deletoida, ilmeisesti MFC huolehtii siivouksesta automaattisesti
		itsSplashThreadPtr->HideSplash();
	fHidden = true;
}
