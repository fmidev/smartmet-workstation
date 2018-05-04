#pragma once

class CSplashThread;

// t�m� luokka varmistaa ett� kaikissa tapauksissa Applikaation
// InitInstance-metodissa kutsutaan lopuksi CSplashThread-luokan 
// HideSplash-metodia.
class CSplashThreadHolder
{
public:
	CSplashThreadHolder(CSplashThread* theSplashThreadPtr);
	~CSplashThreadHolder(void);

	void HideSplash(void); // t�ll� voi halutessa piilottaa sen heti kun haluaa

private:
	CSplashThread* itsSplashThreadPtr;
	bool fHidden;
};
