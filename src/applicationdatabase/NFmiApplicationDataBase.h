#pragma once

#include "NFmiGlobals.h"

#include <vector>
#include "stdafx.h" // GUID-struct

class NFmiInfoOrganizer;

class NFmiApplicationDataBase
{
public:
	// t‰m‰ typedef on string:in ja bool:in yhdistelm‰. string on exen nimi ja bool kertoo otetaanko
	// kyseisen exen versio numero myˆs talteen
	typedef std::pair<std::string, bool>  AppSpyData;
	typedef std::vector<AppSpyData> AppSpyList;


	enum Action
	{
		kStart = 1,
		kClose = 2,
		kUpdate = 3
	};

	NFmiApplicationDataBase(void);
	void InitFromSettings(const std::string &theBaseNameSpace);
	void StoreToSettings(void) const;

	std::string MakeUrlParamString(void);
	const std::string& BaseUrlString(void) const {return itsBaseUrlString;}
	void BaseUrlString(const std::string &newValue) {itsBaseUrlString = newValue;}
	bool UseDataSending(void) const {return fUseDataSending;}
	void UseDataSending(bool newValue) {fUseDataSending = newValue;}
    double UpdateIntervalInHours(void) const {return itsUpdateIntervalInHours;}
	void UpdateIntervalInHours(double newValue) {itsUpdateIntervalInHours = newValue;}
	const std::string& ControlBasePath(void) const {return itsControlBasePath;}
	void ControlBasePath(const std::string &newValue) {itsControlBasePath = newValue;}
    // infoOrganizer -parametri voi olla puuttuva
	void CollectSmartMetData(NFmiApplicationDataBase::Action theAction, FmiLanguage applicationLanguage, int applicationRunningTimeInSeconds, bool toolMasterAvailable, NFmiInfoOrganizer *infoOrganizer); 
	static bool SendSmartMetDataToDB(NFmiApplicationDataBase theData, std::string &theResponceStr); // HUOM! tahallaan kopio, koska dataa voidaan l‰hett‰‰ teoriassa kahta samaan aikaan (esim. jos k‰ynnist‰‰ ja sammuttaa editorin muutaman sekunnin sis‰ll‰)
	static std::string GetApplicationName(void); // vain nimi ilman polkuja ja p‰‰tteit‰
	static std::string GetFullApplicationName(void); // exe:n nimi polkuineen p‰‰tteineen
	static std::string GetFileVersionOfApplication(const std::string &theFullAppName);
	static int CountProcessCount(const NFmiApplicationDataBase::AppSpyData &theAppData, std::string &theAppVersionsStrOut);
    static std::string GetProcessPathAndName(DWORD processID);
    const std::string& GuidStr(void) const;
    float GetApplicationHardDriveFreeSpaceInMB(void) const;
	std::string GetDecodedApplicationDirectory() const;

	// guid
	GUID guid;
	std::string guidstring;
	// checksum
	std::string checksum;
	// Action-enum
	Action itsAction;
	// action
	std::string action;
	// appname
	std::string appname;
	// appversion
	std::string appversion;
	// confname
	std::string confname;
	// exebits
	int exebits;
	// applang
	std::string applang;
	// apppath
	std::string apppath;
	// appuptime [s]
	int appuptime;
	// avstmstatus
	std::string avstmstatus;
	// avstmversion
	std::string avstmversion;
	// avsexpstatus
	std::string avsexpstatus;
	// avsexpversion
	std::string avsexpversion;
	// pcname
	std::string pcname;
	// username
	std::string username;
	// domain
	std::string domain;
	// ipaddress
	std::string ipaddress;
	// pcuptime
	int pcuptime;
	// sendtime [UTC + zone]
	std::string sendtime;
	// osname
	std::string osname;
	// osrevision
	std::string osrevision;
	// osspinfo
	std::string osspinfo;
	// oskernelversion
	std::string oskernelversion;
	// osbits
	int osbits;
	// oslang
	std::string oslang;
	// corecount
	int corecount;
	// cpubits
	int cpubits;
	// memtotal [MB]
	float memtotal;
	// memavailable [MB]
	float memavailable;
	// appqdatamem [MB]
	float appqdatamem;
	// appqdatacount
	int appqdatacount;
	// pcdisplaycount
	int pcdisplaycount;
	// pcdisplayreso
	std::string pcdisplayreso;
	// cdriveserial
	std::string cdriveserial;
	// cdrivefree [MB]
	float cdrivefree;
	// appdrivefree [MB]
	float appdrivefree;
	// sysappmemusage [MB]
	float sysappmemusage;
	// runappcount e.g. [1:0:2]
	std::string runappcount;
	// runningversions e.g. [5.4.0.1;5.6.1.0;5.7.10.1]
	std::string runningversions;
	// gcname
	std::string gcname;
	// gcdriverversion
	std::string gcdriverversion;
	// gcmemory [MB]
	float gcmemory;

private:
	std::string itsBaseUrlString;
	bool fUseDataSending;
	double itsUpdateIntervalInHours;
	std::string itsControlBasePath; // T‰ss‰ on haluttu ohjaus-hakemisto, mista tietyt ohjaus tiedostot luetaan, polku on suhteellinen.


	std::string itsBaseNameSpace;
};
