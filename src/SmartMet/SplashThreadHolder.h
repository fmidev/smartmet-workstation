#pragma once

class CSplashThread;

// tämä luokka varmistaa että kaikissa tapauksissa Applikaation
// InitInstance-metodissa kutsutaan lopuksi CSplashThread-luokan 
// HideSplash-metodia.
class CSplashThreadHolder
{
public:
	CSplashThreadHolder(CSplashThread* theSplashThreadPtr);
	~CSplashThreadHolder(void);

	void HideSplash(void); // tällä voi halutessa piilottaa sen heti kun haluaa

private:
	CSplashThread* itsSplashThreadPtr;
	bool fHidden;
};
