#pragma once

// Windows registry:iss� on oikeasti tallettu n�m� muuttujat, jotka sitten kuljetetaan sielt�
// NFmiMTATempSystem luokan k�ytt��n t�ss� erillisess� rakenteessa.
class SoundingViewSettingsFromWindowsRegisty
{
	// Luotausn�yt�ss� olevan tekstiosion voi nyt laittaa menem��n yl�reunasta alkaen joko alhaalta yl�s tai p�invastoin (ennen oli vain alhaalta yl�s eli nurinp�in suhteess� luotaus k�yriin)
	bool fSoundingTextUpward = false; 
	// Luotausn�yt�ss� voi olla nyt aikalukko p��ll�, jolloin luotausten ajat sidotaan p��karttan�ytt��n, eli niit� s��det��n jos karttan�yt�ll� vaihdetaan aikaa
	bool fSoundingTimeLockWithMapView = false;
	// Onko luotausn�yt�ss� stabiilisuusindeksitaulukko auki vai ei
	bool fShowStabilityIndexSideView = false; 
	// Onko tekstimuotoinen luotausdatataulukko auki vai ei
	bool fShowTextualSoundingDataSideView = false; 
public:
	SoundingViewSettingsFromWindowsRegisty();
	~SoundingViewSettingsFromWindowsRegisty();
	SoundingViewSettingsFromWindowsRegisty(bool soundingTextUpward, bool soundingTimeLockWithMapView, bool showStabilityIndexSideView, bool showTextualSoundingDataSideView);

	bool SoundingTextUpward() const { return fSoundingTextUpward; }
	void SoundingTextUpward(bool newValue) { fSoundingTextUpward = newValue; }
	bool SoundingTimeLockWithMapView() const { return fSoundingTimeLockWithMapView; }
	void SoundingTimeLockWithMapView(bool newValue) { fSoundingTimeLockWithMapView = newValue; }
	bool ShowStabilityIndexSideView() const { return fShowStabilityIndexSideView; }
	void ShowStabilityIndexSideView(bool newValue) { fShowStabilityIndexSideView = newValue; }
	bool ShowTextualSoundingDataSideView() const { return fShowTextualSoundingDataSideView; }
	void ShowTextualSoundingDataSideView(bool newValue) { fShowTextualSoundingDataSideView = newValue; }
};
