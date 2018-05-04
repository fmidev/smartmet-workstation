// NFmiDataNotificationSettingsWinRegistry.cpp 
//
// NFmiDataNotificationSettingsWinRegistry-luokan toteutus.

#ifdef _MSC_VER
#pragma warning( disable : 4800 ) // tämä estää pitkän varoituksen joka tulee kun käytetään CachedRegBool -> registry_int<bool> -tyyppistä dataa, siellä operator T -metodissa DWORD muuttuu bool:iksi
#endif

#include "NFmiDataNotificationSettingsWinRegistry.h"
#include "NFmiSettings.h"
#include <boost/regex.hpp>

NFmiDataNotificationSettingsWinRegistry::NFmiDataNotificationSettingsWinRegistry(void)
:mUse(false)
,mTimeOutInSeconds()
,mOriginTimeFormat()
,mCurrentTimeFormat()
,mUseSound()
,mToolTip()
,mToolTipUseTitle()
,mBalloonHeader()
,mShowIcon()
{
}

NFmiDataNotificationSettingsWinRegistry::~NFmiDataNotificationSettingsWinRegistry(void)
{
}

static void FixVersionNumber(boost::shared_ptr<CachedRegString> &cachedValue, const std::string &fullAppVer)
{
    try
    {
        std::string origString = *cachedValue; 
        boost::regex re("[\\d+\\.]+[\\d+]{1}"); // versio numero, jossa on väh. x.x, missä x voi sisältää useita numeroita esim. 2.12
        std::string fixedStr = boost::regex_replace(origString, re, fullAppVer);
        *cachedValue = fixedStr;
    }
    catch(...)
    {
        // ei kiinnosta jos jokin meni pieleen
    }
}

bool NFmiDataNotificationSettingsWinRegistry::Init(const std::string &baseRegistryPath, const std::string &fullAppVer)
{
    if(mInitialized)
        std::runtime_error("NFmiDataNotificationSettingsWinRegistry::Init: all ready initialized.");

    mInitialized = true;
    mBaseRegistryPath = baseRegistryPath;
    mSectionName = "\\DataNotifications";

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

	mUse = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, mSectionName, "\\Use", usedKey, true, "SmartMet::DataNotification::Use");
	mTimeOutInSeconds = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionName, "\\TimeOutInSeconds", usedKey, 10, "SmartMet::DataNotification::TimeOutInSeconds");
	mOriginTimeFormat = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\OriginTimeFormat", usedKey, "Time: DD.MM.YYYY HH UTC", "SmartMet::DataNotification::OriginTimeFormat");
	mCurrentTimeFormat = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\CurrentTimeFormat", usedKey, "", "SmartMet::DataNotification::CurrentTimeFormat");
	mUseSound = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, mSectionName, "\\UseSound", usedKey, false, "SmartMet::DataNotification::UseSound");
    mToolTipUseTitle = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, mSectionName, "\\ToolTipUseTitle", usedKey, true, "SmartMet::DataNotification::SystemTrayIcon::ToolTipUseTitle");
    mShowIcon = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, mSectionName, "\\ShowIcon", usedKey, true, "SmartMet::DataNotification::SystemTrayIcon::Show");

    // Kahdelle seuraavalle arvolle pitää tehdä vielä versio numeron korvaus:
    // Jos arvosta löytyy versio numero joka on muotoa x.x.x.x tai "${SmartMetVersion}", korvaa se nykyisellä versionumerolla
    mToolTip = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\ToolTip", usedKey, "SmartMet 5.9.3.0 System Tray Icon", "SmartMet::DataNotification::SystemTrayIcon::ToolTip");
    ::FixVersionNumber(mToolTip, fullAppVer);
    mBalloonHeader = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\BalloonHeader", usedKey, "SmartMet 5.9.3.0 data loaded", "SmartMet::DataNotification::SystemTrayIcon::BalloonHeader");
    ::FixVersionNumber(mBalloonHeader, fullAppVer);

    return true;
}

bool NFmiDataNotificationSettingsWinRegistry::Use(void) const 
{
    return *mUse;
}

void NFmiDataNotificationSettingsWinRegistry::Use(bool newValue) 
{
    *mUse = newValue;
}

int NFmiDataNotificationSettingsWinRegistry::TimeOutInSeconds(void) const 
{
    return *mTimeOutInSeconds;
}

void NFmiDataNotificationSettingsWinRegistry::TimeOutInSeconds(int newValue) 
{
    *mTimeOutInSeconds = newValue;
}

std::string NFmiDataNotificationSettingsWinRegistry::OriginTimeFormat(void) const 
{
    return *mOriginTimeFormat;
}

void NFmiDataNotificationSettingsWinRegistry::OriginTimeFormat(const std::string &newValue) 
{
    *mOriginTimeFormat = newValue;
}

std::string NFmiDataNotificationSettingsWinRegistry::CurrentTimeFormat(void) const 
{
    return *mCurrentTimeFormat;
}

void NFmiDataNotificationSettingsWinRegistry::CurrentTimeFormat(const std::string &newValue) 
{
    *mCurrentTimeFormat = newValue;
}

bool NFmiDataNotificationSettingsWinRegistry::UseSound(void) const 
{
    return *mUseSound;
}

void NFmiDataNotificationSettingsWinRegistry::UseSound(bool newValue) 
{
    *mUseSound = newValue;
}

std::string NFmiDataNotificationSettingsWinRegistry::ToolTip(void) const 
{
    return *mToolTip;
}

void NFmiDataNotificationSettingsWinRegistry::ToolTip(const std::string &newValue) 
{
    *mToolTip = newValue;
}

bool NFmiDataNotificationSettingsWinRegistry::ToolTipUseTitle(void) const 
{
    return *mToolTipUseTitle;
}

void NFmiDataNotificationSettingsWinRegistry::ToolTipUseTitle(bool newValue) 
{
    *mToolTipUseTitle = newValue;
}

std::string NFmiDataNotificationSettingsWinRegistry::BalloonHeader(void) const 
{
    return *mBalloonHeader;
}

void NFmiDataNotificationSettingsWinRegistry::BalloonHeader(const std::string &newValue) 
{
    *mBalloonHeader = newValue;
}

bool NFmiDataNotificationSettingsWinRegistry::ShowIcon(void) const 
{
    return *mShowIcon;
}

void NFmiDataNotificationSettingsWinRegistry::ShowIcon(bool newValue) 
{
    *mShowIcon = newValue;
}

