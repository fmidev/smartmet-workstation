#pragma once

// FmiTempDlg.h : header file
//
#include "SmartMetToolboxDep_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiTempDlg dialog

#include "CheckComboBox.h"
#include "NFmiProducer.h"
#include "NFmiDataMatrix.h"
#include "afxcmn.h"
#include "NFmiViewPosRegistryInfo.h"

class CFmiTempView;
class SmartMetDocumentInterface;
class NFmiProducerInfo;

class CFmiTempDlg : public CDialog
{
private: // n�ill� talletetaan sijainti ja koko rekisteriin
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
    // N�ytet��nk� indeksi taulukko vain luotaus tekstin�
    BOOL fShowIndexies;
    BOOL fShowHodograf;
    // lista luotaus tuottajista, joilta l�ytyi dataa, t�ll� t�ytet��n yll� oleva combolista.
    std::vector<NFmiProducer> itsProducerListWithData; 
    // T�m� on lippu, jolla teemme dialogista k�ytt�j� yst�v�llisen eli jos kukaan ei ole viel� k�ynyt s��t�m�ss� tuottaja listaa, voidaan
    // listan oletus arvo laittaa osoittamaan havaittuja luotauksia jos niit� l�ytyy, muuten laitetaan se osoittamaan 1. tuottajaa. 
    // Muista ett� t�m� lista el�� kun dataa ladataan koko ajan sis��n.
    bool fProducerSelectorUsedYet; 
    BOOL fShowMapMarkers;
    CCheckComboBox itsMultiProducerSelector;
    HACCEL m_hAccel; // accelerator hanskaus menee t�m�n kautta
    BOOL fShowSideView;
    int itsModelRunCount;
    CSpinButtonCtrl itsModelRunSpinner;
    BOOL fSoundingTimeLockWithMapView;
    BOOL fSoundingTextUpward;
    BOOL fShowSecondaryDataView;

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
	afx_msg void OnBnClickedCheckShowIndexies();
	afx_msg void OnBnClickedCheckShowHodograf();
	afx_msg void OnBnClickedShowTxtSoundingData();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnBnClickedButtonResetScales();
	afx_msg void OnBnClickedTempCode();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCbnSelchangeComboProducerSelection();
	afx_msg void OnCbnCloseUp();

	afx_msg void OnAcceleratorTempProducer1();
	afx_msg void OnAcceleratorTempProducer2();
	afx_msg void OnAcceleratorTempProducer3();
	afx_msg void OnAcceleratorTempProducer4();
	afx_msg void OnAcceleratorTempProducer5();
	afx_msg void OnAcceleratorTempProducer6();
	afx_msg void OnAcceleratorTempProducer7();
	afx_msg void OnAcceleratorTempProducer8();
	afx_msg void OnAcceleratorTempProducer9();
	afx_msg void OnAcceleratorTempProducer10();
	afx_msg void OnEditSpaceOut();
	afx_msg void OnBnClickedCheckShowSideView();
	afx_msg void OnAcceleratorSwapArea();
	afx_msg void OnAcceleratorToggleTooltip();
	afx_msg void OnEnChangeEditModelRunCount();
    afx_msg void OnBnClickedCheckUseMapTimeWithSoundings();
    afx_msg void OnBnClickedCheckPutSoundingTextsUpward();
    afx_msg void OnAcceleratorChangeMapTypeExtraMap();
    afx_msg void OnBnClickedCheckShowSecondaryDataView();
};


