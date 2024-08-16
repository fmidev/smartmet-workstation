
// MainFrm.h : interface of the CMainFrame class
//

// Jos halutaan disabloida MFC-feature-packin käyttö, määrittele
// FMI_DISABLE_MFC_FEATURE_PACK
// Projektin asetuksissa C++ - Preprocessor definitions -kohdassa

#pragma once
#include <vector>
#include "NFmiViewPosRegistryInfo.h"
#include "PPToolTip.h"

#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	typedef CFrameWnd CFmiUsedFrameWndParent;
#else
	typedef CFrameWndEx CFmiUsedFrameWndParent;
#endif // FMI_DISABLE_MFC_FEATURE_PACK

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	#include "FileView.h"
	#include "ClassView.h"
	#include "OutputWnd.h"
	#include "PropertiesWnd.h"
#endif // FMI_DISABLE_MFC_FEATURE_PACK

class NFmiEditMapGeneralDataDoc;
enum class NFmiLedColor;
class NFmiLedLightStatusSystem;

const int kFmiAutoSaveTimer = 5;
const int kFmiCleanDataTimer = 6;
const int kFmiDebugDataSizeLoggerTimer = 8;
const int kFmiCheckAnimationLockedModeTimeBagsTimer = 9;
const int kFmiCheckForNewSatelDataTimer = 10;
const int kFmiAutoStartUpLoadTimer = 11;
const int kFmiToggleFlagButtonTimer = 12;
const int kFmiDataToDBUpdateTimer = 13;
const int kFmiCleanOldDataFromMemoryTimer = 14;
const int kFmiStoreViewPosToWinRegistryTimer = 16;
const int kFmiStoreCrashBackupViewMacroTimer = 17;
const int kFmiGenerateBetaProductsTimer = 18;
const int kFmiParameterSelectionSystemUpdateTimer = 19;
const int kFmiLoggingSystemManagementTimer = 20;
const int kFmiNewQueryDataReadUpdateViewsTimer = 21;
const int kFmiLedLightsActionTimer = 22;
const int kFmiWmsSupportRenewerTimer = 23;

class CStatusBarResize : public CStatusBar
{
	int wantedStretchPaneIndex_ = 0;
	int totalNumberOfPanes_ = 0;
	CPPToolTip itsTooltipCtrl;
	NFmiLedLightStatusSystem* ledLightStatusSystem_ = nullptr; // ei omista/tuhoa
public:
	CStatusBarResize();
	void Setup(NFmiLedLightStatusSystem* ledLightStatusSystem, int totalNumberOfPanes);
	BOOL InitTooltipControl();
protected:
	DECLARE_DYNCREATE(CStatusBarResize)
	DECLARE_MESSAGE_MAP()

	std::string GetTooltipText(const CPoint& cursorInClientSpace);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void NotifyDisplayTooltip(NMHDR* pNMHDR, LRESULT* result);
};


class CMainFrame : public CFmiUsedFrameWndParent
{
private:
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo() {return s_ViewPosRegistryInfo;}

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	void StartDataLoadingWorkingThread();
	void StartAdditionalWorkerThreads();
    void StartQDataCacheThreads();
    void StartHistoryDataCacheThread();
	void StopQDataCacheThreads();
	void WaitQDataCacheThreadsToStop();
	void SetNotificationMessage(const std::string &theNotificationMsgStr, const std::string &theNotificationTitle, int theStyle, int theTimeout, bool fNoSound);
	void SetDefaultValues() {}; // tämä vaaditaan DoWindowSizeSettings -template funktion takia, mutta se ei tee mitään, koska pää ikkunaa ei voi laittaa defaultti paikkaan
	void CheckForAutoLoadTimerStart();
	void PutWarningFlagTimerOn();
	void DoAppDataBaseCollection(int theAction);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
    void ParameterSelectionSystemUpdateTimerStart(int waitTimeInSeconds);

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	std::vector<UINT> indicatorVector;
	CStatusBarResize     m_wndStatusBar;
	CToolBar       m_wndToolBar;
protected:
#else
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBar       m_wndToolBar;
protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBarImages m_UserImages;
	CFileView         m_wndFileView;
	CClassView        m_wndClassView;
	COutputWnd        m_wndOutput;
	CPropertiesWnd    m_wndProperties;
	CMFCCaptionBar    m_wndCaptionBar;
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	UINT itsAutoSaveTimer;
	UINT itsCleanDataTimer;
	UINT itsDebugDataSizeLoggerTimer;
	UINT itsCheckAnimationLockedModeTimeBagsTimer;
	UINT itsCheckForNewSatelDataTimer;
	UINT itsAutoStartUpLoadTimer;
	UINT itsToggleFlagButtonTimer;
	UINT itsDataToDBUpdateTimer;
	UINT itsCleanOldDataFromMemoryTimer;
    UINT itsStoreViewPosToWinRegistryTimer;
    UINT itsStoreCrashBackupViewMacroTimer;
    UINT itsGenerateBetaProductsTimer;
    UINT itsParameterSelectionSystemUpdateTimer;
    UINT itsLoggingSystemManagementTimer;
	UINT itsLedLightsActionTimer;
	UINT itsWmsSupportRenewerTimer;

	NFmiEditMapGeneralDataDoc* itsDoc; // ei omista, ei tuhoa
	CBitmap *itsRedFlagBitmap;
	CBitmap *itsOrangeFlagBitmap;
	int itsFlagButtonIndex; // indeksi toolbarin dataQualityButtoniin (0 pohjainen indeksi, arvo on -1 jos nappi on piilotettu)
	int itsDisableThreadsVariable; // tämä on normaalisti 0, mutta jos testi mielessä haluaa tiettyjen threadien olevan pois käytöstä, tähän muuttujaan asetetaan tietyt bitit päällä, jolloin kyseiset threadit jätetään käynnistämättä.
// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnViewCaptionBar();
	afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnViewSetMainFramePlaceToDefault();
	afx_msg BOOL OnToolTipText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void OnWorkinThreadDataRead2();
	void GetNewWarningMessages();
	void DoMacroParamUpdate();

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	BOOL CreateCaptionBar();
#endif

public:
    virtual void GetMessageString(UINT nID, CString& rMessageU_) const;


private:
	void SetControlHiding(const std::string &confOption, int commandId);
	void ConfigureToolbarControls();
    void LocalizeMenuStrings();
    void UpdateCrashRptLogFile();
    void TrimmInMemoryLogMessages();
    void StartNewQueryDataLoadedUpdateTimer(const std::string &loadedFileNames);
    void StartSmartMetTimers();
	void SetupLedIndicatorsInStatusbar();
	void UpdateStatusBarIcons(int status, NFmiLedColor ledColor);
	void DoLedLightsActionUpdates();
	void MakeStatusBarIndicators();
	int CreateStatusBar();

	// 256 värisen ja vaihtuva kokoisen tolbarin teko vaatii näiden käyttöön oton
	HBITMAP itsToolBarBitmapHandle;
	CBitmap itsToolBarBitmap;
	CImageList itsToolBarImagelist;
public:
	afx_msg void OnEnablenotifications();
	afx_msg void OnUpdateEnablenotifications(CCmdUI *pCmdUI);
	afx_msg void OnDisablenotifications();
	afx_msg void OnUpdateDisablenotifications(CCmdUI *pCmdUI);
	afx_msg void OnNotificationsoundson();
	afx_msg void OnUpdateNotificationsoundson(CCmdUI *pCmdUI);
	afx_msg void OnNotificationsoundsoff();
	afx_msg void OnUpdateNotificationsoundsoff(CCmdUI *pCmdUI);
	afx_msg void OnShowSystraySymbol();
	afx_msg void OnUpdateShowSystraySymbol(CCmdUI *pCmdUI);
	afx_msg void OnHideSystraySymbol();
	afx_msg void OnUpdateHideSystraySymbol(CCmdUI *pCmdUI);

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	afx_msg void OnViewDemodockviews();
#endif // FMI_DISABLE_MFC_FEATURE_PACK

    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};


