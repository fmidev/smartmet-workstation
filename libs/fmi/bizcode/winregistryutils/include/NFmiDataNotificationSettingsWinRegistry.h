// NFmiDataNotificationSettingsWinRegistry.h
//
// NFmiDataNotificationSettings-luokka tietää yleiset datan lataus notifikaatio asetukset.
// Luokkaan on laitettu myös System Tray Icon perus asetuksia.
// Asetukset ovat windows rekisterissä, mutta alkuarvaukset voidaan myös lukea SmartMet konffeista. 

#pragma once

#include "NFmiCachedRegistryValue.h"

class NFmiDataNotificationSettingsWinRegistry
{
public:
	NFmiDataNotificationSettingsWinRegistry(void);
	~NFmiDataNotificationSettingsWinRegistry(void);

	bool Init(const std::string &baseRegistryPath, const std::string &fullAppVer);

	bool Use(void) const;
	void Use(bool newValue);
	int TimeOutInSeconds(void) const;
	void TimeOutInSeconds(int newValue);
	std::string OriginTimeFormat(void) const;
	void OriginTimeFormat(const std::string &newValue);
	std::string CurrentTimeFormat(void) const;
	void CurrentTimeFormat(const std::string &newValue);
	bool UseSound(void) const;
	void UseSound(bool newValue);
	std::string ToolTip(void) const;
	void ToolTip(const std::string &newValue);
	bool ToolTipUseTitle(void) const;
	void ToolTipUseTitle(bool newValue);
	std::string BalloonHeader(void) const;
	void BalloonHeader(const std::string &newValue);
	bool ShowIcon(void) const;
	void ShowIcon(bool newValue);

private:
    bool mInitialized = false; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath;
    std::string mSectionName; // tässä on  \\DataNotifications

    boost::shared_ptr<CachedRegBool> mUse;
    boost::shared_ptr<CachedRegInt> mTimeOutInSeconds;
    boost::shared_ptr<CachedRegString> mOriginTimeFormat;
	boost::shared_ptr<CachedRegString> mCurrentTimeFormat;
    boost::shared_ptr<CachedRegBool> mUseSound;

	// system tray icon asetuksia
	boost::shared_ptr<CachedRegString> mToolTip;
	boost::shared_ptr<CachedRegBool> mToolTipUseTitle; // käytetäänkö tooltipissa itsToolTip arvoa, vaiko SmartMetin titleä eli otsikon tekstiä
	boost::shared_ptr<CachedRegString> mBalloonHeader;
	boost::shared_ptr<CachedRegBool> mShowIcon;
};


