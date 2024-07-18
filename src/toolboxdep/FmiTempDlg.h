#pragma once

// FmiTempDlg.h : header file
//
#include "SmartMetToolboxDep_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiTempDlg dialog

#include "CheckComboBox.h"
#include "NFmiProducer.h"
#include "afxcmn.h"
#include "NFmiViewPosRegistryInfo.h"

class CFmiTempView;
class SmartMetDocumentInterface;
class NFmiProducerInfo;

class CFmiTempDlg : public CDialog
{
private: // näillä talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

    // Construction
	DECLARE_DYNAMIC(CFmiTempDlg)

    CFmiTempDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiTempDlg(void);

	void Update(void);
	CBitmap* ViewMemoryBitmap(void);
	void SetDefaultValues(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
    CFmiTempView* GetTempView() { return itsView; }

// Dialog Data
	//{{AFX_DATA(CFmiTempDlg)
	enum { IDD = IDD_TEMP_DIALOG };
	CButton	itsPrintButtom;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiTempDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnCancel();
    virtual void OnOK();
    //}}AFX_VIRTUAL

// Implementation
private:
	int CalcControlAreaHeight(void);
	CRect CalcClientArea(void);
	CRect CalcOtherArea(void);
    void UpdateControlsFromDocument();
    void UpdateProducerList(void);
    void DoWhenClosing(void);
    void InitDialogTexts(void);
    void SelectProducer(int producerIndex);
    void SetSelectedProducer(void);
    void SetSelectedProducersFromViewMacro(void);
    BOOL InitTooltipControl(void);
    void InitControlsTooltip(int theControlId, const char *theDictionaryStr);

// Attributes:
	CFmiTempView *itsView;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	CToolTipCtrl itsTooltipCtrl;
    BOOL fSkewTModeOn;
    // Näytetäänkö indeksi taulukko vain luotaus tekstinä
    BOOL fShowStabilityIndexSideView;
    BOOL fShowHodograf;
    // lista luotaus tuottajista, joilta löytyi dataa, tällä täytetään yllä oleva combolista.
    std::vector<NFmiProducer> itsProducerListWithData; 
    // Tämä on lippu, jolla teemme dialogista käyttäjä ystävällisen eli jos kukaan ei ole vielä käynyt säätämässä tuottaja listaa, voidaan
    // listan oletus arvo laittaa osoittamaan havaittuja luotauksia jos niitä löytyy, muuten laitetaan se osoittamaan 1. tuottajaa. 
    // Muista että tämä lista elää kun dataa ladataan koko ajan sisään.
    bool fProducerSelectorUsedYet; 
    BOOL fShowMapMarkers;
    CCheckComboBox itsMultiProducerSelector;
    HACCEL m_hAccel; // accelerator hanskaus menee tämän kautta
    BOOL fShowTextualSoundingDataSideView;
    int itsModelRunCount;
    CSpinButtonCtrl itsModelRunSpinner;
    BOOL fSoundingTimeLockWithMapView;
    BOOL fSoundingTextUpward;
    BOOL fShowSecondaryDataView;
	CString itsAvgRangeInKmStr;
	CString itsAvgTimeRange1Str;
	CString itsAvgTimeRange2Str;

	// Generated message map functions
	//{{AFX_MSG(CFmiTempDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonPrint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedShowMapMarkers();
	afx_msg void OnBnClickedCheckTempSkewtMode();
	afx_msg void OnBnClickedButtonSettings();
	afx_msg void OnBnClickedButtonResetSoundingData();
	afx_msg void OnBnClickedCheckShowStabilityIndexiesSideView();
	afx_msg void OnBnClickedCheckShowHodograf();
	afx_msg void OnBnClickedShowTxtSoundingData();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnBnClickedButtonResetScales();
	afx_msg void OnBnClickedTempCode();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCbnSelchangeComboProducerSelection();
	afx_msg void OnCbnCloseUp();

	afx_msg void OnBnClickedCheckShowTextualSoundingDataSideView();
	afx_msg void OnEnChangeEditModelRunCount();
    afx_msg void OnBnClickedCheckUseMapTimeWithSoundings();
    afx_msg void OnBnClickedCheckPutSoundingTextsUpward();
    afx_msg void OnBnClickedCheckShowSecondaryDataView();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnAcceleratorSoundingProducer1();
	afx_msg void OnAcceleratorSoundingProducer2();
	afx_msg void OnAcceleratorSoundingProducer3();
	afx_msg void OnAcceleratorSoundingProducer4();
	afx_msg void OnAcceleratorSoundingProducer5();
	afx_msg void OnAcceleratorSoundingProducer6();
	afx_msg void OnAcceleratorSoundingProducer7();
	afx_msg void OnAcceleratorSoundingProducer8();
	afx_msg void OnAcceleratorSoundingProducer9();
	afx_msg void OnAcceleratorSoundingProducer10();
	afx_msg void OnAcceleratorSoundingOpenLogViewer();
	afx_msg void OnAcceleratorSoundingSpaceOutWinds();
	afx_msg void OnAcceleratorSoundingToggleSideView2();
	afx_msg void OnAcceleratorSoundingToggleStabilityIndexView();
	afx_msg void OnAcceleratorSoundingToggleTextView();
	afx_msg void OnAcceleratorSoundingToggleTooltip();
	afx_msg void OnEnChangeEditAvgRangeInKm();
	afx_msg void OnEnChangeEditAvgTimeRange1();
	afx_msg void OnEnChangeEditAvgTimeRange2();
	afx_msg void OnBnClickedButtonClearAvgControls();
	afx_msg void OnAcceleratorSoundingAutoAdjustScales();
	afx_msg void OnAcceleratorSoundingToggleVirtualTimeMode();
};


