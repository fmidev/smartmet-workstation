// NFmiApplicationDataBase.cpp

#include "stdafx.h"
#include "NFmiApplicationDataBase.h"
#include "NFmiStringTools.h"
#include "NFmiSettings.h"
#include "md5.h"
#include "NFmiString.h"
#include "NFmiFileString.h"
#include "Monitors.h"
#include "MultiMonitor.h"
#include "Psapi.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiQ2Client.h"
#include "NFmiValueString.h"
#include "Utf8ConversionFunctions.h"
#include "SettingsFunctions.h"
#include "NFmiMetTime.h"
#include <thread>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "version.lib")

NFmiApplicationDataBase::NFmiApplicationDataBase(void)
:guid()
,checksum()
,itsAction(kStart)
,action()
,appname()
,appversion()
,confname()
,exebits(0)
,applang()
,apppath()
,appuptime(0)
,avstmstatus()
,avstmversion()
,avsexpstatus()
,avsexpversion()
,pcname()
,username()
,domain()
,ipaddress()
,pcuptime(0)
,sendtime()
,osname()
,osrevision()
,osspinfo()
,oskernelversion()
,osbits(0)
,oslang()
,corecount(0)
,cpubits(0)
,memtotal(0)
,memavailable(0)
,appqdatamem(0)
,appqdatacount(0)
,pcdisplaycount(0)
,pcdisplayreso()
,cdriveserial()
,cdrivefree(0)
,appdrivefree(0)
,sysappmemusage(0)
,runappcount()
,runningversions()
,gcname()
,gcdriverversion()
,gcmemory(0)
,itsBaseUrlString()
,fUseDataSending(false)
,itsUpdateIntervalInHours(24)
,itsControlBasePath()
,itsBaseNameSpace()
{
}

void NFmiApplicationDataBase::InitFromSettings(const std::string &theBaseNameSpace)
{
	itsBaseNameSpace = theBaseNameSpace;

	fUseDataSending = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseDataSending"));
	itsBaseUrlString = SettingsFunctions::GetUrlFromSettings(std::string(itsBaseNameSpace + "::BaseUrlString"));
	itsUpdateIntervalInHours = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::UpdateIntervalInHours"));
}

void NFmiApplicationDataBase::StoreToSettings(void) const
{
	if(itsBaseNameSpace.empty() == false)
	{
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseDataSending"), NFmiStringTools::Convert(fUseDataSending), true);
        SettingsFunctions::SetUrlToSettings(std::string(itsBaseNameSpace + "::BaseUrlString"), itsBaseUrlString);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UpdateIntervalInHours"), NFmiStringTools::Convert(itsUpdateIntervalInHours), true);
	}
	else
		throw std::runtime_error("Error in NFmiApplicationDataBase::StoreToSettings, unable to store setting.");
}

static std::string MakeGuidStr(const GUID &guid)
{
    CString guidStrU_;
    guidStrU_.Format(_TEXT("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"), guid.Data1, guid.Data2, guid.Data3, UINT(guid.Data4[0]), UINT(guid.Data4[1]), UINT(guid.Data4[2]), UINT(guid.Data4[3]), UINT(guid.Data4[4]), UINT(guid.Data4[5]), UINT(guid.Data4[6]), UINT(guid.Data4[7]));
    std::string resultStr = CT2A(guidStrU_);
	return resultStr;
}

static std::string MakeDecimalNumberStr(float theValue, int maxDecimals)
{
	return std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValue, maxDecimals));
}

// HUOM! muutos pit‰‰ tehd‰ -> utf8 -> urlencode -j‰rjestyksess‰.
static std::string FixStringWithUtf8AndUrlEncode(const std::string &theString)
{
	std::string str = fromLocaleStringToUtf8(theString); // ensin muutetaan mahdolliset ‰‰kkˆset ja muut utf8:ksi
	str = NFmiStringTools::UrlEncode(str); // t‰ss‰ voisi olla spaceja ja muita vastaavia, siksi pit‰‰ varmuuden vuoksi encodata
	return str;
}

// Jos tekstiss‰ on vahingossa \u tai \U ilman XXXX- tai XXXXXXXX- osiota, pit‰‰ niit‰ muuttaa, muuten
// tulee ongelmia unicode tulkintojen kanssa.
// Nyt tuli ongelmia Turun siitepˆly editointi konffien kanssa, koska siin‰ oli control-polku seuraava:
// -p \smartmet\smartcloud\UTU\control
// eli tuossa tuo \UTU... aiheutti ongelman vastaanotto p‰‰ss‰.
// Teen korjauksen niin ett‰ muutan '\U'- ja '\u' -merkit '/U' tai '/u' :ksi
static std::string QuickFixPossibleUnicodeMixupWithPathStrings(const std::string &theString)
{
    std::string modifiedStr = theString;
    NFmiStringTools::ReplaceAll(modifiedStr, "\\u", "/u");
    return NFmiStringTools::ReplaceAll(modifiedStr, "\\U", "/U");
}

// HUOM! url-stringi‰ ei saa urlencodata lopuksi kerralla, eli eri kent‰t pit‰‰ urlencodata erikseen.
std::string NFmiApplicationDataBase::MakeUrlParamString(void)
{
	std::string resultStr;

	resultStr += "guid=";
	resultStr += guidstring;

	if(checksum.empty() == false)
	{
		resultStr += "&checksum=";
		resultStr += checksum;
	}

	if(action.empty() == false)
{
		resultStr += "&action=";
		resultStr += action;
	}

	if(appname.empty() == false)
	{
		resultStr += "&appname=";
		appname = ::FixStringWithUtf8AndUrlEncode(appname); // t‰ss‰ voisi olla spaceja, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += appname;
	}

	if(appversion.empty() == false)
	{
		resultStr += "&appversion=";
		resultStr += appversion;
	}

	if(confname.empty() == false)
	{
		resultStr += "&confname=";
        confname = ::QuickFixPossibleUnicodeMixupWithPathStrings(confname); // pit‰‰ tehd‰ ennen url-encodea
		confname = ::FixStringWithUtf8AndUrlEncode(confname); // t‰ss‰ voisi olla spaceja, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += confname;
	}

	if(exebits)
	{
		resultStr += "&exebits=";
		resultStr += NFmiStringTools::Convert(exebits);
	}

	if(applang.empty() == false)
	{
		resultStr += "&applang=";
		resultStr += applang;
	}

	if(apppath.empty() == false)
	{
		resultStr += "&apppath=";
		NFmiStringTools::TrimR(apppath, '\\'); // pit‰‰ ottaa mahdollinen viimeinen \-merkki pois, koska se sotkee vastaanotto puolen
        apppath = ::QuickFixPossibleUnicodeMixupWithPathStrings(apppath); // pit‰‰ tehd‰ ennen url-encodea
		apppath = ::FixStringWithUtf8AndUrlEncode(apppath); // t‰ss‰ voisi olla spaceja, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += apppath;
	}

	if(appuptime)
	{
		resultStr += "&appuptime=";
		resultStr += ::MakeDecimalNumberStr(static_cast<float>(appuptime), 0);
	}

	if(avstmstatus.empty() == false)
	{
		resultStr += "&avstmstatus=";
		resultStr += avstmstatus;
	}

	if(avstmversion.empty() == false)
	{
		resultStr += "&avstmversion=";
		resultStr += avstmversion;
	}

	if(avsexpstatus.empty() == false)
	{
		resultStr += "&avsexpstatus=";
		resultStr += avsexpstatus;
	}

	if(avsexpversion.empty() == false)
	{
		resultStr += "&avsexpversion=";
		resultStr += avsexpversion;
	}

	if(pcname.empty() == false)
	{
		resultStr += "&pcname=";
		pcname = ::FixStringWithUtf8AndUrlEncode(pcname); // t‰ss‰ voisi olla spaceja ja ‰‰kkˆsi‰, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += pcname;
	}

	if(username.empty() == false)
	{
		resultStr += "&username=";
		username = ::FixStringWithUtf8AndUrlEncode(username); // t‰ss‰ voisi olla spaceja ja ‰‰kkˆsi‰, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += username;
	}

	if(domain.empty() == false)
	{
		resultStr += "&domain=";
		resultStr += domain;
	}

	if(ipaddress.empty() == false)
	{
		resultStr += "&ipaddress=";
		resultStr += ipaddress;
	}

	if(pcuptime)
	{
		resultStr += "&pcuptime=";
		resultStr += ::MakeDecimalNumberStr(static_cast<float>(pcuptime), 0);
	}

	if(sendtime.empty() == false)
	{
		resultStr += "&sendtime=";
//		sendtime = ::FixStringWithUtf8AndUrlEncode(sendtime); // t‰ss‰ voisi olla spaceja tai +-merkki, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += sendtime;
	}

	if(osname.empty() == false)
	{
		resultStr += "&osname=";
		osname = ::FixStringWithUtf8AndUrlEncode(osname); // t‰ss‰ voisi olla spaceja, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += osname;
	}

	if(osrevision.empty() == false)
	{
		resultStr += "&osrevision=";
//		osrevision = ::FixStringWithUtf8AndUrlEncode(osrevision); // t‰ss‰ voisi olla spaceja, siksi pit‰‰ varmuuden vuoksi encodata
        if(osrevision.size() > 10)
    		osrevision.resize(10); // toistaiseksi koko on rajattu 10 merkkiin
		resultStr += osrevision;
	}

	if(osspinfo.empty() == false)
	{
		resultStr += "&osspinfo=";
		osspinfo = ::FixStringWithUtf8AndUrlEncode(osspinfo); // t‰ss‰ voisi olla spaceja, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += osspinfo;
	}

	if(oskernelversion.empty() == false)
	{
		resultStr += "&oskernelversion=";
		resultStr += oskernelversion;
	}

	if(osbits)
	{
		resultStr += "&osbits=";
		resultStr += NFmiStringTools::Convert(osbits);
	}

	if(oslang.empty() == false)
	{
		resultStr += "&oslang=";
		resultStr += oslang;
	}

	if(corecount)
	{
		resultStr += "&corecount=";
		resultStr += NFmiStringTools::Convert(corecount);
	}

	if(cpubits)
	{
		resultStr += "&cpubits=";
		resultStr += NFmiStringTools::Convert(cpubits);
	}

	if(memtotal)
	{
		resultStr += "&memtotal=";
		resultStr += ::MakeDecimalNumberStr(memtotal, 1);
	}

	if(memavailable)
	{
		resultStr += "&memavailable=";
		resultStr += ::MakeDecimalNumberStr(memavailable, 1);
	}

	if(appqdatamem)
	{
		resultStr += "&appqdatamem=";
		resultStr += ::MakeDecimalNumberStr(appqdatamem, 1);
	}

	if(appqdatacount)
	{
		resultStr += "&appqdatacount=";
		resultStr += NFmiStringTools::Convert(appqdatacount);
	}

	if(pcdisplaycount)
	{
		resultStr += "&pcdisplaycount=";
		resultStr += NFmiStringTools::Convert(pcdisplaycount);
	}

	if(pcdisplayreso.empty() == false)
	{
		resultStr += "&pcdisplayreso=";
		resultStr += pcdisplayreso;
	}

	if(cdriveserial.empty() == false)
	{
		resultStr += "&cdriveserial=";
		resultStr += cdriveserial;
	}

	if(cdrivefree)
	{
		resultStr += "&cdrivefree=";
		resultStr += ::MakeDecimalNumberStr(cdrivefree, 1);
	}

	if(appdrivefree)
	{
		resultStr += "&appdrivefree=";
		resultStr += ::MakeDecimalNumberStr(appdrivefree, 1);
	}

	if(sysappmemusage)
	{
		resultStr += "&sysappmemusage=";
		resultStr += ::MakeDecimalNumberStr(sysappmemusage, 1);
	}

	if(runappcount.empty() == false)
	{
		resultStr += "&runappcount=";
		resultStr += runappcount;
	}

	if(runningversions.empty() == false)
	{
		resultStr += "&runningversions=";
		resultStr += runningversions;
	}

	if(gcname.empty() == false)
	{
		resultStr += "&gcname=";
		gcname = ::FixStringWithUtf8AndUrlEncode(gcname); // t‰ss‰ voisi olla spaceja, siksi pit‰‰ varmuuden vuoksi encodata
		resultStr += gcname;
	}

	if(gcdriverversion.empty() == false)
	{
		resultStr += "&gcdriverversion=";
		resultStr += gcdriverversion;
	}

	if(gcmemory)
	{
		resultStr += "&gcmemory=";
		resultStr += ::MakeDecimalNumberStr(gcmemory, 1);
	}

	return resultStr;
}

char gSmetStr[] = "smet-4391@%{?#52§&!";

//give your application full path
static std::string GetFileVersionOfApplication(LPCTSTR theFileName)
{
     DWORD dwDummy;
     DWORD dwFVISize = ::GetFileVersionInfoSize(theFileName, &dwDummy);

     if(dwFVISize)
     {
         LPBYTE lpVersionInfo = new BYTE[dwFVISize];

         ::GetFileVersionInfo(theFileName , 0, dwFVISize, lpVersionInfo);

         UINT uLen;
         VS_FIXEDFILEINFO *lpFfi;

         ::VerQueryValue(lpVersionInfo, _T("\\"), (LPVOID *)&lpFfi, &uLen);

         DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
         DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;

         delete [] lpVersionInfo;

         DWORD dwLeftMost     = HIWORD(dwFileVersionMS);
         DWORD dwSecondLeft   = LOWORD(dwFileVersionMS);
         DWORD dwSecondRight  = HIWORD(dwFileVersionLS);
         DWORD dwRightMost    = LOWORD(dwFileVersionLS);

         CString strU_;
         strU_.Format(_TEXT("%d.%d.%d.%d"), dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
         return std::string(CT2A(strU_));
     }
     return "";
}

static CString GetFullAppName(const CString &theExeNameU_)
{
    CString fullNameU_ = theExeNameU_;
    fullNameU_.Append(_TEXT(".exe")); // Now has "MyExe.exe" (or "MyDll.dll").
    HMODULE hmod = ::GetModuleHandle(fullNameU_);
    CString fullPathU_;
    DWORD pathLen = ::GetModuleFileName(hmod, fullPathU_.GetBufferSetLength(MAX_PATH + 1), MAX_PATH); // hmod of zero gets the main EXE
    fullPathU_.ReleaseBuffer(pathLen); // Note that ReleaseBuffer doesn't need a +1 for the null byte.
    return fullPathU_;
}

// t‰t‰ ei ole jostain syyst‰ m‰‰ritelty winkkarin headereissa
#ifndef VER_SUITE_WH_SERVER
#define VER_SUITE_WH_SERVER 0x00008000
#endif

static std::string GetWindowsName(OSVERSIONINFOEX &osvi, SYSTEM_INFO &sys_info)
{
	std::string winName;
	if(osvi.dwMajorVersion == 6)
	{
		if(osvi.dwMinorVersion == 1)
		{
			if(osvi.wProductType == VER_NT_WORKSTATION)
				winName = "Windows 7";
			else
				winName = "Windows Server 2008 R2";
		}
		else if(osvi.dwMinorVersion == 0)
		{
			if(osvi.wProductType == VER_NT_WORKSTATION)
				winName = "Windows Vista";
			else
				winName = "Windows Server 2008";
		}
	}
	else if(osvi.dwMajorVersion == 5)
	{
		if(osvi.dwMinorVersion == 2)
		{
			if((osvi.wProductType == VER_NT_WORKSTATION) && (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))
				winName = "Windows XP Professional x64 Edition";
			else if(osvi.wSuiteMask & VER_SUITE_WH_SERVER)
				winName = "Windows Home Server";
			else if(::GetSystemMetrics(SM_SERVERR2) != 0)
				winName = "Windows Server 2003 R2";
			else
				winName = "Windows Server 2003";
		}
		else if(osvi.dwMinorVersion == 1)
		{
			winName = "Windows XP";
		}
		else if(osvi.dwMinorVersion == 0)
		{
			winName = "Windows 2000";
		}
	}

	return winName;
}

static std::string GetWindowsRevision(void)
{
	std::string revisionNumber;
	// haetaan winkkarin revisio rekisterist‰
	HKEY hKey = 0;
	char buf[255] = {0};
	DWORD dwType = 0;
	DWORD dwBufSize = sizeof(buf);
    if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"), NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwType = REG_SZ;
        if(::RegQueryValueEx(hKey, _TEXT("BuildLabEx"), 0, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS)
			revisionNumber = buf; // BuildLabEx:ista tulee hieman enemm‰n tavaraa, esim. omalla tyo koneellani Win7 tulee seuraava arvo:
									// "7600.16695.amd64fre.win7_gdr.101026-1503", 
									// miss‰ 7600 on buil-number
									// 16695 on revision number
									// sitten tulee ilmeisesti processori tietoa, k‰yttiksen nime‰ ja muuta s‰l‰‰....
		RegCloseKey(hKey);
	}
	return revisionNumber;
}

static std::string GetGraphicsAdapterName(void)
{
	std::string graphicsAdapter;
	// haetaan winkkarin rekisterist‰
	HKEY hKey = 0;
	char buf[255] = {0};
	DWORD dwType = 0;
	DWORD dwBufSize = sizeof(buf);
    if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winsat"), NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwType = REG_SZ;
        if(::RegQueryValueEx(hKey, _TEXT("PrimaryAdapterString"), 0, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS)
			graphicsAdapter = buf;
		RegCloseKey(hKey);
	}
	return graphicsAdapter;
}

const double kMegaByte = 1024*1024;

static float GetGraphicsAdapterMemoryMB(void)
{
	float memMB = 0;
	std::string memTotalStr;
	// haetaan winkkarin rekisterist‰
	HKEY hKey = 0;
	DWORD dwType = 0;
	DWORD value = 0;
	DWORD dwSize = sizeof(value);
    if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winsat"), NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwType = REG_DWORD;
        if(::RegQueryValueEx(hKey, _TEXT("VideoMemorySize"), 0, &dwType, (BYTE*)&value, &dwSize) == ERROR_SUCCESS)
		{
			try
			{
				memMB = static_cast<float>( value / kMegaByte );
			}
			catch(...)
			{
			}
		}
		RegCloseKey(hKey);
	}
	return memMB;
}

static std::string GetIPAddress(void)
{
	WSADATA WSAData;
	// Initialize winsock dll
	if(::WSAStartup(MAKEWORD(1, 0), &WSAData))
	{
		return "";
	}
	// Get local host name
	char szHostName[128] = "";
	if(::gethostname(szHostName, sizeof(szHostName)))
	{
		return "";
	}
	// Get local IP addresses
	struct sockaddr_in SocketAddress;
	struct hostent     *pHost        = 0;

	pHost = ::gethostbyname(szHostName);
	if(!pHost)
	{
		return "";
	}

	std::string ipAddress;
	for(int iCnt = 0; ((pHost->h_addr_list[iCnt]) && (iCnt < 10)); ++iCnt)
	{
		memcpy(&SocketAddress.sin_addr, pHost->h_addr_list[iCnt], pHost->h_length);
		ipAddress = inet_ntoa(SocketAddress.sin_addr);
		break; // otetaan vain ensimm‰inen
	}

	// Cleanup
	WSACleanup();

	return ipAddress;
}

// Otta sein‰kello ajan sekunnin tarkkuudella ja tekee siit‰ halutun formaalin mukaisen stringin.
// Formaatti: YYYY-MM-DDThh:mm:ssTZD
// Esim. 1997-07-16T19:20:30+01:00
static 	std::string GetCurrentTimeString(void)
{
	NFmiTime aTime; // otetaan sein‰kello aika
	NFmiTime utcTime = aTime.UTCTime(); // t‰ss‰ h‰ipyy sekunnit
	std::string timeStr = utcTime.ToStr("YYYY-MM-DDTHH:mm:");
	timeStr += aTime.ToStr("SS");
	if(aTime.GetZoneDifferenceHour() < 0) // en tied‰ voiko olla NFmiTime:n itsZoneDifferenceHour positiivinen, mutta varmuuden vuoksi tulee joko + tai -
		timeStr += "%2B"; // huom! NFmiTime:ssa itsZoneDifferenceHour etu merkki on toisin p‰in kuin itse tekstiin halutaan eli -2:n tapauksessa pit‰‰ olla +02:00 teksti
	else
		timeStr += "-";
	short absZoneValue = ::abs(aTime.GetZoneDifferenceHour());
	if(absZoneValue < 10)
	{
		timeStr += "0"; // etu nolla, jos liku oli pinempi kuin 10
		timeStr += NFmiStringTools::Convert(absZoneValue);
	}
	else
	{
		timeStr += NFmiStringTools::Convert(absZoneValue);
	}
	timeStr += ":00"; // NFmi*Time -luokassa ei ole tietoa esim. puolen tunnin siirroista, mit‰ jossain on, siksi pit‰‰ laittaa vain 00

	return timeStr;
}

#pragma warning( push )
#pragma warning( disable : 4996 )

// En voinut k‰ytt‰‰ CFmiGdiPlusHelpers::WStringToString -funktiota, koska
// jostain syyst‰ kyseisen headerin includointi tanne sekoittaa k‰‰nt‰j‰n pahasti. Joten kopsasin funktion t‰h‰n.
static std::string WStringToString(const std::wstring& s)
{
	std::locale loc;
	std::string ns;
	ns.resize(s.size());
	std::use_facet<std::ctype<wchar_t> >(loc).narrow(&s[0], &s[0]+s.size(), '?', &ns[0]);
	return ns;
}

// Undo the above warning disabling.
#pragma warning( pop )

static size_t GetAppMemoryUsage(void)
{
	DWORD currentProcessId = ::GetCurrentProcessId();
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, currentProcessId);
	if (NULL == hProcess)
		return 0;

	size_t memUsage = 0;
	if (GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)))
	{
		memUsage = pmc.WorkingSetSize;
	}
	CloseHandle( hProcess );
	return memUsage;
}

static float GetHardDriveFreSpaceMB(const std::string &theDrive)
{
    CString cDriveU_ = CA2T(theDrive.c_str());
	ULARGE_INTEGER freeBytes;
	ULARGE_INTEGER totalBytes;
	ULARGE_INTEGER totalFreeBytes;
	float memFreeMB = 0;
    if(::GetDiskFreeSpaceEx(cDriveU_, &freeBytes, &totalBytes, &totalFreeBytes))
	{
#ifdef _WIN64
		memFreeMB = static_cast<float>(totalFreeBytes.QuadPart / kMegaByte);
#else
		double totMemMB = totalFreeBytes.HighPart * 4096.; // pit‰‰ kertoa oikeasti 2 potenssiin 32 / megabyte -> 4096
		totMemMB += totalFreeBytes.LowPart / kMegaByte;
		memFreeMB = static_cast<float>(totMemMB);
#endif
	}
	return memFreeMB;
}

std::string NFmiApplicationDataBase::GetProcessPathAndName(DWORD processID)
{
    TCHAR szProcessPathName[MAX_PATH] = _TEXT("");
	std::string name;

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
    // Get the process name.
	if ( hProcess )
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
		{
            if(GetModuleFileNameEx(hProcess, hMod, szProcessPathName, static_cast<DWORD>(MAX_PATH)))
				name = CT2A(szProcessPathName);
		}
	}

	CloseHandle(hProcess); // vapauta handle k‰ytˆn j‰lkeen

	return name;
}

int NFmiApplicationDataBase::CountProcessCount(const NFmiApplicationDataBase::AppSpyData &theAppData, std::string &theAppVersionsStrOut)
{
	std::string lowerCaseAppName = theAppData.first;
	NFmiStringTools::LowerCase(lowerCaseAppName);
	int foundCounter = 0;
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
    char currentProcessPathName[MAX_PATH] = "";

	if(::EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
	    cProcesses = cbNeeded / sizeof(DWORD);
	    for(i = 0; i < cProcesses; i++)
		{
			std::string testedProcessNameStr(GetProcessPathAndName(aProcesses[i]));
			if(testedProcessNameStr.empty() == false)
			{
				NFmiFileString fileStr = testedProcessNameStr;
				NFmiString exeName = fileStr.FileName();
				std::string lowerCaseExeName = exeName;
				NFmiStringTools::LowerCase(lowerCaseExeName);
				if(lowerCaseExeName == lowerCaseAppName)
				{
					foundCounter++;
					if(theAppData.second)
					{ // t‰m‰n ohjelman versio numero halutaan theAppVersionsStrOut -stringiin lis‰tt‰v‰ksi
						if(theAppVersionsStrOut.empty() == false)
							theAppVersionsStrOut += ";";
						theAppVersionsStrOut += ::GetFileVersionOfApplication(CA2T(testedProcessNameStr.c_str()));
					}
				}
			}
		}
	}

	return foundCounter;
}

static std::string GetAppCountString(NFmiApplicationDataBase::AppSpyList &theAppSpyList, std::string &theAppVersionsStrOut)
{
	std::string resultStr;
	if(theAppSpyList.size())
	{
		for(size_t i=0; i < theAppSpyList.size(); i++)
		{
			if(i > 0)
				resultStr += ":";
			int appInstanceCount = NFmiApplicationDataBase::CountProcessCount(theAppSpyList[i], theAppVersionsStrOut);
			resultStr += NFmiStringTools::Convert<int>(appInstanceCount);
		}
	}
	return resultStr;
}

std::string NFmiApplicationDataBase::GetApplicationName(void)
{
	return std::string(CT2A(AfxGetApp()->m_pszExeName));
}

std::string NFmiApplicationDataBase::GetFullApplicationName(void)
{
    CString appFullNameU_ = ::GetFullAppName(AfxGetApp()->m_pszExeName);
    std::string appFullNameStr = CT2A(appFullNameU_);
    return appFullNameStr;
}

std::string NFmiApplicationDataBase::GetFileVersionOfApplication(const std::string &theFullAppName)
{
	return ::GetFileVersionOfApplication(CA2T(theFullAppName.c_str()));
}

const std::string& NFmiApplicationDataBase::GuidStr(void) const
{
    return guidstring;
}

static std::string GetOsLangName()
{
	// Kokeillaan ensin C++ tapaa, joka on toiminut joskus aiemmin, mutta ei en‰‰ tietyill‰ systeemeill‰ (en tied‰ johtuuko ongelma Windows vai Visual C++ versioista)
	std::locale loc("");
	std::string oslang = loc.name();
	if(oslang.empty())
	{
		WCHAR localeName[LOCALE_NAME_MAX_LENGTH] = { 0 };
		int ret = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);
		if(ret)
			oslang = CW2A(localeName);
	}

	if(oslang.size() > 2)
		oslang.resize(2); // vain kaksi ensimm‰ist‰ kirjainta kiinnostaa.... (fi, en, tms)
	return oslang;
}

// Windows versionin haku systeemist‰ on ihan rikki, MicroSoft on ryssinyt pahasti sen kanssa:
// 1. Joskus toiminut GetVersionEx((LPOSVERSIONINFO)&osvi); systeemi palauttaa nyky‰‰n vain versiota 6.2 eli Windows 8.0:aa.
// 2. Windows rekisteriss‰ on tieto versiosta Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\CurrentMajorVersionNumber ja CurrentMinorVersionNumber
//   - Mutta jostain syyst‰ niit‰ ei saa ulos normi kyselyill‰, muita saman paikan arvoja saa kyll‰ ulos (esim. CurrentVersion saa ulos, mutta siin‰ on arvo 6.3)
//   - Ongelma johtuu jotenkin WOW64 jutuista ja ett‰ niill‰ on omat rekisteri arvonsa ja n‰m‰ pit‰isi ehk‰ hakea sielt‰ (WOW o nWindows on Windows ja siin‰ voidaan ajaa eri bittisi‰ windows ohjelmia eri bittisiss‰ k‰yttiksiss‰)
// 3. T‰m‰ uuden GetWindowsVersion funktion jutut on haettu https://www.codeproject.com/Articles/5336372/Windows-Version-Detection
//   - Siin‰ otetaan joku randomin n‰kˆinen muistiosoite talteen ja siit‰ nysv‰t‰‰n sopivilla siirtymill‰ major/minor/build numerot
//   - Ainoa lohtu on ett‰ se toimii ainakin Windows 10:iss‰, muita koneita ei ole minulla k‰ytˆss‰
static std::string GetWindowsVersion()
{
	auto sharedUserData = (BYTE*)0x7FFE0000;
	int majorVersion = static_cast<int>(*(ULONG*)(sharedUserData + 0x26c));
	int minorVersion = static_cast<int>(*(ULONG*)(sharedUserData + 0x270));
//	DWORD buildNumber = static_cast<DWORD>(*(ULONG*)(sharedUserData + 0x260));
	std::string osVersionStr = std::to_string(majorVersion);
	osVersionStr += ".";
	osVersionStr += std::to_string(minorVersion);
	return osVersionStr;
}

void NFmiApplicationDataBase::CollectSmartMetData(NFmiApplicationDataBase::Action theAction, FmiLanguage applicationLanguage, int applicationRunningTimeInSeconds, bool toolMasterAvailable, NFmiInfoOrganizer *infoOrganizer)
{
	// guid
	// action
	itsAction = theAction;
	if(theAction == NFmiApplicationDataBase::kStart)
	{
		::CoCreateGuid(&guid);
		action = "start";

        this->guidstring = ::MakeGuidStr(guid);
		std::string guidStr = guidstring;
		guidStr += gSmetStr;

		checksum = MD5(guidStr).hexdigest();
	}
	else if(theAction == NFmiApplicationDataBase::kClose)
	{
		action = "stop";
	}
	else if(theAction == NFmiApplicationDataBase::kUpdate)
	{
		action = "update";
	}
	// appname
	appname = CT2A(AfxGetApp()->m_pszExeName);
	// appversion
	std::string appFullName = NFmiApplicationDataBase::GetFullApplicationName();
	appversion = NFmiApplicationDataBase::GetFileVersionOfApplication(appFullName);
	// confname
	confname = itsControlBasePath;
	// exebits
#ifdef _WIN64
	exebits = 64;
#else
	exebits = 32;
#endif
	// applang
    if(applicationLanguage == kFinnish)
        applang = "fi";
    else if(applicationLanguage == kEnglish)
        applang = "en";
    else
        applang = "??";

	// apppath (t‰m‰ on siis exe:n polku)
	NFmiFileString appPathAndName(appFullName);
	NFmiString appPath = appPathAndName.Device();
	appPath += appPathAndName.Path();
	apppath = appPath;
	// appuptime [s]
	appuptime = applicationRunningTimeInSeconds;

	// avstmversion is set outside of this class

    // avstmstatus
    if(toolMasterAvailable)
        avstmstatus = "ok";
    else
        avstmstatus = "error";


	// avsexpstatus  // AVS Express ei viel‰ k‰ytˆss‰
	// avsexpversion // AVS Express ei viel‰ k‰ytˆss‰

	// pcname
	TCHAR buffer[256] = TEXT("");
	DWORD dwSize = sizeof(buffer);
	if (::GetComputerNameEx(ComputerNameDnsHostname, buffer, &dwSize))
		pcname = CT2A(buffer);

	// username
	if (::GetUserName(buffer, &dwSize))
		username = CT2A(buffer);
	// domain
	if (::GetComputerNameEx(ComputerNameDnsDomain, buffer, &dwSize))
		domain = CT2A(buffer);
	else if (::GetComputerNameEx(ComputerNamePhysicalDnsDomain, buffer, &dwSize))
		domain = CT2A(buffer);
	// ipaddress
	ipaddress = GetIPAddress();
	// pcuptime [s]
	DWORD upTimeInMilliSeconds = ::GetTickCount(); // HUOM! t‰m‰ nollautuu aina 49.7 p‰iv‰n kuluttua
	pcuptime = upTimeInMilliSeconds/1000;

	// sendtime [UTC + zone]
	sendtime = ::GetCurrentTimeString();

	SYSTEM_INFO sys_info;
	::GetSystemInfo(&sys_info);

	SYSTEM_INFO native_sys_info;
	::GetNativeSystemInfo(&native_sys_info);

	OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
	::GetVersionEx((LPOSVERSIONINFO)&osvi);

	if(osvi.dwMajorVersion >= 6)
	{  // k‰ytet‰‰n t‰t‰ jos voidaan, t‰m‰ ei mene heti ymp‰ri, kuten GetTickCount() menee 49.7 vrk:ssa, kun on 64-bitti‰ k‰ytˆss‰ (liikkumavara on n. 0.5 mrd vuotta).
		// HUOM! t‰t‰ voi kutsua Vistasta (6.0) ja WinServer2008:sta (6.0) alkaen
		// HUOM! T‰t‰ koko GetTickCount64 -funktiota ei voi k‰ytt‰‰ ollenkaan, koska sit‰ ei lˆydy Kernel32.dll:st‰ esim. XP:ss‰ ja ohjelman suoritys tyss‰‰ heti k‰ynnnistyess‰, vaikka koko funktiota ei oikeasti k‰ytett‰isi
//		ULONGLONG upTimeInMilliSeconds64 = ::GetTickCount64();
//		pcuptime = upTimeInMilliSeconds/1000;
	}

	// osname
	osname = GetWindowsName(osvi, sys_info);
	// osrevision
	osrevision = ::GetWindowsRevision();
	// osspinfo
	osspinfo = CT2A(osvi.szCSDVersion); // sis‰lt‰‰ tekstin winkkarin viimeisest‰ asennetusta service packista (esim. "Service Pack 3" tai tyhj‰, jos ei ole asennettu ensimm‰ist‰k‰‰n)
	// oskernelversion
	oskernelversion = ::GetWindowsVersion();
	// osbits
	if(native_sys_info.wProcessorArchitecture >= 6 && native_sys_info.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_UNKNOWN)
		osbits = 64;
	else if(native_sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
		osbits = 32;
	else
		osbits = 0;

	// oslang
    oslang = ::GetOsLangName();

	// corecount
	corecount = std::thread::hardware_concurrency();
	// cpubits
	if(sys_info.wProcessorArchitecture >= 6 && sys_info.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_UNKNOWN)
		cpubits = 64;
	else if(sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
		cpubits = 32;
	else
		cpubits = 0;

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	::GlobalMemoryStatusEx(&statex);
	// memtotal [MB]
	memtotal = static_cast<float>(statex.ullTotalPhys/kMegaByte);
	// memavailable [MB]
	memavailable = static_cast<float>(statex.ullAvailPhys/kMegaByte);
	// appqdatamem [MB]
    if(infoOrganizer)
    {
        appqdatamem = static_cast<float>(infoOrganizer->CountDataSize() / kMegaByte);
        // appqdatacount 
        appqdatacount = infoOrganizer->CountData();
    }
	CMonitors monitors;
	// pcdisplaycount
	pcdisplaycount = monitors.GetCount();
	// pcdisplayreso
	CMonitor primaryMonitor = monitors.GetMonitor(0);
	CRect monitorRect;
	primaryMonitor.GetMonitorRect(&monitorRect);
    CString monitorResoStrU_;
    monitorResoStrU_.Format(_TEXT("%dx%d"), monitorRect.Width(), monitorRect.Height()); // formaatti: widthxheight [pixels]
    pcdisplayreso = CT2A(monitorResoStrU_);
	// cdriveserial
	TCHAR volumeName[MAX_PATH + 1] = { 0 };
	TCHAR fileSystemName[MAX_PATH + 1] = { 0 };
	DWORD serialNumber = 0;
	DWORD maxComponentLen = 0;
	DWORD fileSystemFlags = 0;
	if (GetVolumeInformation(_TEXT("C:\\"), volumeName, ARRAYSIZE(volumeName), &serialNumber, &maxComponentLen, &fileSystemFlags, fileSystemName, ARRAYSIZE(fileSystemName)))
	{
		int firstPart = serialNumber >> 16;  // otetaan 16 ylint‰ bitti‰ talteen
		int secondPart = serialNumber & 65535; // otetaan 16 alinta bitti‰ talteen
        CString serialStrU_;
        serialStrU_.Format(_TEXT("%X-%X"), firstPart, secondPart); // formaatti on 16 bitti‰ heksana ja viiva v‰liss‰ (esim. 23B1-5DA2)
        cdriveserial = CT2A(serialStrU_);
	}
	// cdrivefree [MB]
	std::string cDrive("C:\\");
	cdrivefree = ::GetHardDriveFreSpaceMB(cDrive);
	// appdrivefree [MB]
	appdrivefree = GetApplicationHardDriveFreeSpaceInMB();

	// sysappmemusage [MB]
	sysappmemusage =  static_cast<float>(::GetAppMemoryUsage() / kMegaByte);
	// runappcount
	AppSpyList appSpyList; // HUOM! kun tulee uusia tarkkailtavia ohjelmia, ne pit‰‰ laittaa listan alkuun!!!!!
    appSpyList.push_back(std::make_pair(std::string("msiexec.exe"), false)); // t‰m‰ pit‰‰ laittaa 7. viimeissen‰!
    appSpyList.push_back(std::make_pair(std::string("wmiprvse.exe"), false)); // t‰m‰ pit‰‰ laittaa 6. viimeissen‰!
    appSpyList.push_back(std::make_pair(std::string("explorer.exe"), false)); // t‰m‰ pit‰‰ laittaa 5. viimeissen‰!
	appSpyList.push_back(std::make_pair(std::string("javaw.exe"), false)); // t‰m‰ pit‰‰ laittaa 4. viimeissen‰! (Mirwa tai joku muu java ohjelma?!?!?)
	appSpyList.push_back(std::make_pair(std::string("firefox.exe"), false)); // t‰m‰ pit‰‰ laittaa 3. viimeissen‰!
	appSpyList.push_back(std::make_pair(std::string("zeditmap2.exe"), true)); // t‰m‰ pit‰‰ laittaa toiseksi viimeissen‰!
	appSpyList.push_back(std::make_pair(std::string("SmartMet.exe"), true)); // t‰m‰ pit‰‰ laittaa viimeissen‰!
	std::string runninAppVersions;
	runappcount = ::GetAppCountString(appSpyList, runninAppVersions);
	// runningversions
	runningversions = runninAppVersions;
	// gcname
	gcname = ::GetGraphicsAdapterName();
	// gcdriverversion
	// TƒHƒN kohtaan en lˆyt‰nyt nyt keinoa saada yksiselitteisesti arvoa, en rekisterist‰, enk‰ win32 API:sta

	// gcmemory [MB]
	gcmemory = ::GetGraphicsAdapterMemoryMB();

}

float NFmiApplicationDataBase::GetApplicationHardDriveFreeSpaceInMB(void) const
{
	std::string appFullName = NFmiApplicationDataBase::GetFullApplicationName();
	NFmiFileString appPathAndName(appFullName);
	std::string appDrive = appPathAndName.Device();
	appDrive += "\\";
	float appDriveFreeInMB = ::GetHardDriveFreSpaceMB(appDrive);
    return appDriveFreeInMB;
}

// HUOM! tahallaan kopio, koska dataa voidaan l‰hett‰‰ teoriassa kahta samaan aikaan 
// (esim. jos k‰ynnist‰‰ ja sammuttaa editorin muutaman sekunnin sis‰ll‰).
bool NFmiApplicationDataBase::SendSmartMetDataToDB(NFmiApplicationDataBase theData, std::string &theResponceStr) 
{
	theResponceStr = "";
	std::string urlParamStr = theData.MakeUrlParamString();
	std::string baseUrlStr = theData.BaseUrlString();
	NFmiQ2Client httpClient;
	std::string totalUrlStr = baseUrlStr + "?" + urlParamStr;
	std::string responseStr;
	try
	{
		httpClient.MakeHTTPRequest(totalUrlStr, responseStr, false); // false tarkoittaa ett‰ tehd‰‰n http-POST
		if(responseStr.empty() == false)
		{
			std::string lowerCaseResponseStr = responseStr;
			NFmiStringTools::LowerCase(lowerCaseResponseStr);
			std::string errorString("error");
			std::string::size_type pos = lowerCaseResponseStr.find(errorString);
			if(pos != std::string::npos)
				throw std::runtime_error("Error, this message won't go furher, but the responseStr is handled like error string");
		}
		return true;
	}
	catch(...)
	{
	}
	theResponceStr = responseStr;
	return false;
}

std::string NFmiApplicationDataBase::GetDecodedApplicationDirectory() const
{
	std::string tmpApppath = apppath;
	return NFmiStringTools::UrlDecode(tmpApppath);
}
