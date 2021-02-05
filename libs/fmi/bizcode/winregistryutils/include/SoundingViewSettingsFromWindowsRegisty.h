#pragma once

// Windows registry:issä on oikeasti tallettu nämä muuttujat, jotka sitten kuljetetaan sieltä
// NFmiMTATempSystem luokan käyttöön tässä erillisessä rakenteessa.
class SoundingViewSettingsFromWindowsRegisty
{
	// Luotausnäytössä olevan tekstiosion voi nyt laittaa menemään yläreunasta alkaen joko alhaalta ylös tai päinvastoin (ennen oli vain alhaalta ylös eli nurinpäin suhteessä luotaus käyriin)
	bool fSoundingTextUpward = false; 
	// Luotausnäytössä voi olla nyt aikalukko päällä, jolloin luotausten ajat sidotaan pääkarttanäyttöön, eli niitä säädetään jos karttanäytöllä vaihdetaan aikaa
	bool fSoundingTimeLockWithMapView = false;
	// Onko luotausnäytössä stabiilisuusindeksitaulukko auki vai ei
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
