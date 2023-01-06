#pragma once

// OptionsDlg.h : header file
//
#include "SmartMetDialogs_resource.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog
class SmartMetDocumentInterface;

class COptionsDlg : public CDialog
{
// Construction
public:
	COptionsDlg(CWnd* pParent = NULL);   // standard constructor
	void SetData(SmartMetDocumentInterface *smartMetDocumentInterface){itsSmartMetDocumentInterface = smartMetDocumentInterface;}

// Dialog Data
	//{{AFX_DATA(COptionsDlg)
	enum { IDD = IDD_DIALOG_OPTIONS };
	BOOL	fStationPlot;
    CString	itsTimeStepStrU_;
	BOOL	fShowToolTip;
	BOOL	fUseSpacingOut;
	BOOL	fAutoZoom;
    CString	itsViewCacheSizeStrU_;
    CString	itsUndoRedoDepthStrU_;
	BOOL	fUseAutoSave;
    CString	itsAnimationFrameDelayInMSecStrU_;
    CString	itsAutoSaveFrequensInMinutesStrU_;
// onko editori näyttö moodissa vai ei
	BOOL fUseViewMode;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;

	// Generated message map functions
	//{{AFX_MSG(COptionsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonValidationDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DisableControls(void);
	void InitDialogTexts(void);
    void InitLogLevelComboBox();
    void SetLogLevelOnOk();
	void AdjustRestartTextControl();
public:
	afx_msg void OnBnClickedCheckUseViewMode();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonBrowseLocalCacheDirectory();
	afx_msg void OnEnChangeEditLocalCacheDirectory();
private:
    CString itsQ2ServerUrlStrU_;
    CString itsQ3ServerUrlStrU_;
	BOOL fQ2ServerUsed;
	BOOL fQ2ServerZipUsed;
	BOOL fLogQ2Requests;
	int itsQ2ServerGridsizeX;
	int itsQ2ServerGridsizeY;
	int itsStationDataGridsizeX;
	int itsStationDataGridsizeY;
	int itsQ2ServerDecimalCount;
	BOOL fDoAutoLoadDataAtStartUp;
	double itsDrawObjectScaleFactor;
	BOOL fSmartOrientationPrint;
	BOOL fFitToPagePrint;
	BOOL fLowMemoryPrint;
	int itsMaxRangeInPrint;
	BOOL fUseLocalCache;
	BOOL fDoCleanLocalCache;
	double itsCacheKeepFilesMaxDays;
	int itsSatelDataUpdateFrequenceInMinutes;
	BOOL fAllowSending;
    CString itsSysInfoDbUrlU_;
    // Lataako Smartmet automaattisesti querydataa lokaaliin cacheen ja siivoaa sitä
    BOOL fAutoLoadNewCacheData;
    double itsLocationFinderTimeoutInSeconds;
    BOOL fShowLastSendTimeOnMapView;
    BOOL fUseCombinedMapMode;
    CComboBox itsLogLevelComboBox;
    BOOL fDroppedDataEditable;
    double itsIsolineMinimumLengthFactor;
    BOOL fGenerateTimeCombinationData;
    BOOL fUseForcedLinearInterpolationOption;
	float itsHatchingToolmasterEpsilonFactor = 1.f;
	CString itsLocalCacheDirectoryPathOriginal;
	CString itsLocalCacheDirectoryPath;
	bool fLocalCacheDirectoryChanged = false;
	CStatic itsRestartSmartMetStr;
	BOOL fUseLedLightStatusSystem;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

