
// SmartMet.h : main header file for the SmartMet application
//
#pragma once
#include "stdafx.h"

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include <map>
#include <string>
#include <gdiplus.h>

#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	typedef CWinApp CFmiUsedAppParent;
#else
	typedef CWinAppEx CFmiUsedAppParent;
#endif // FMI_DISABLE_MFC_FEATURE_PACK


// CSmartMetApp:
// See SmartMet.cpp for the implementation of this class
//
class NFmiEditMapGeneralDataDoc;
class CSplashThread;

class CSmartMetApp : public CFmiUsedAppParent
{
public:
    using ControlIdTextMap = std::map<UINT, std::string>;

	NFmiEditMapGeneralDataDoc* GeneralDocData(void);
	CSmartMetApp();
	virtual ~CSmartMetApp();
    CString GetToolTipString(UINT commandID, CString &theMagickWordU_);
    void UpdateCrashRptLogFile();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance();
    void InitApplicationInterface();
    bool AllowApplicationToClose() const { return fAllowApplicationToClose; }
    void AllowApplicationToClose(bool newValue) { fAllowApplicationToClose = newValue; }
    virtual int Run();

// Implementation
	BOOL  m_bHiColorIcons;

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	COleTemplateServer m_server;
		// Server object for document creation
	UINT  m_nAppLook;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

private:
	bool DoToolMasterInitialization(void);
	bool InitGdiplus(void);
	void TermGdiplus(void);
//	void CreateMenuDynamically(void);
	bool TakeControlPathInfo(void);
    void CloseToolMaster();
    // std::map<UINT, std::string>& NonDictionaryToolbarItems(void) {return itsMenuCreator.NonDictionaryToolbarItems();}
	CSplashThread* SplashStart(void);
	void LoadFileAtStartUp(CCommandLineInfo *theCmdInfo);
	bool InitGeneralDataDoc(void);
	void DoFinalInitializations(void);
    bool CrashRptInstall(void);
	void CrashRptUninstall(void);
    void InitNonDictionaryToolbarItems();
    void DoFirstTimeOnIdleInitializations();
    void SetWin32LocalesToEnglish();

	NFmiEditMapGeneralDataDoc* itsGeneralData; // UGLY but I couldn't put this in mainframe
	bool fUseToolMasterIfAvailable; // komentorivi argumentilla -n voidaan estää toolmasterin käyttö kokonaan, tämä on true, jos -n optiota ei ole käytetty
    bool fAllowApplicationToClose = false;
    Gdiplus::GdiplusStartupInput itsGdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;
	CSplashThread* itsSplashThread;
    ControlIdTextMap itsNonDictionaryToolbarItems;
};

extern CSmartMetApp theApp;
