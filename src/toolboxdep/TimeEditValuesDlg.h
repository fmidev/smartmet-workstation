#pragma once

// TimeEditValuesDlg.h : header file
//
#include "SmartMetToolboxDep_resource.h"
#include "afxwin.h"

#include <boost/function.hpp>
#include "boost/shared_ptr.hpp"
#include "NFmiPoint.h"
#include "NFmiViewPosRegistryInfo.h"
#include "NFmiDataMatrix.h"

/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesDlg dialog
class CTimeEditValuesView;
class NFmiLocationBag;
class NFmiDrawParam;
class SmartMetDocumentInterface;
class NFmiProducer;

class CTimeEditValuesDlg : public CDialog
{
private: // näillä talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

    // Construction
public:
	typedef std::function<bool (const std::string&) > SetByName;

	CTimeEditValuesDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);
	~CTimeEditValuesDlg(void);
	void Update(void);
	void SetDefaultValues(void);
	void EnableButtons();
	void RefreshApplicationViews(const std::string &reasonForUpdate);
	CBitmap* ViewMemoryBitmap(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
    CTimeEditValuesView* GetTimeEditValuesView() { return itsTimeEditValuesView; }

// Dialog Data
	//{{AFX_DATA(CTimeEditValuesDlg)
	enum { IDD = IDD_DIALOG_TIME_EDIT_VALUES };
	CSliderCtrl	itsSmootherSlider;
	CSliderCtrl	itsManualModifierLength;
	int		itsModifyMode;
	BOOL	fUseMaskInTimeSerialViews;
	BOOL	fUseZoomedAreaCP;
    CString	itsSmootherValueStrU_;
    CString	itsCPManagerStrU_;
	BOOL	fUseAnalyzeTool;
	BOOL	fUseControlPointObservationsBlending;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeEditValuesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK();
	virtual void OnCancel();
	
	// Generated message map functions
	//{{AFX_MSG(CTimeEditValuesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonToiminto();
	afx_msg void OnButtonClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonRedo();
	afx_msg void OnButtonUndo();
	afx_msg void OnClose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheckUseMasksInTimeSerialViews();
	afx_msg void OnCheckUseZoomedAreaCP();
	afx_msg void OnButtonClearAllCpValues();
	afx_msg void OnButtonPrint();
	afx_msg void OnCheckUseAnalyzeTool();
	afx_msg void OnCheckUseCpObsBlending();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DoWhenClosing(void);
	void SeViewModeButtonsSetup(void);
	void CheckIfAnalyzeToolModePossibleAndUpdateControls(void);
	bool EnableDlgItem(int theDlgId, bool fEnable, bool fShow = true);
	void UpdateControlsAfterAnalyzeMode(void);
	void UpdateControlsAfterMPCPMode(void);
	void UpdateSlider(void);
	void UpdateSmootherString(void);
	void UpdateCPManagerString(void);
	double CalcRelativeSliderValue();
	CRect CalcClientArea(void);
	void EditModeUpdate(void);
	void InitDialogTexts(void);
	void UpdateAnalyseActionControl(void);
    void UpdateProducerLists(void);
    void UpdateCpObsBlendProducerList(void);
	void UpdateProducerList(CComboBox &theProducerSelector, const NFmiProducer &theLastSessionProducer, const checkedVector<NFmiProducer> &producerList, bool isSelectionMadeYet);
	void OnComboSelectionChanged(CComboBox &theProducerSelector, SetByName setByName);
	NFmiPoint CalcEditedDataGriddingResolutionInKM(void);

    CTimeEditValuesView* itsTimeEditValuesView;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	HACCEL m_hAccel;
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnAcceleratorTimeViewToggleHelpData2();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnAcceleratorTimeViewToggleHelpData1();
	afx_msg void OnAcceleratorToggleTooltip();
	afx_msg void OnAcceleratorSwapArea();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	BOOL fDoAutoAdjust;
public:
	afx_msg void OnBnClickedCheckTimeSerialAutoAdjust();
	afx_msg void OnCbnSelchangeComboAnalyzeProducer1();
	afx_msg void OnCbnSelchangeComboAnalyzeProducer2();
	afx_msg void OnBnClickedCheckAnalyzeWithBothData();
private:
	CComboBox itsProducer1Selector;
	CComboBox itsProducer2Selector;
	BOOL fUseBothProducerData;
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    BOOL fUseMultiProcessCPCalc;
public:
    afx_msg void OnBnClickedCheckUseMultiProcessCpCalc();
private:
    BOOL fAllowRightClickSelection;
public:
    afx_msg void OnBnClickedCheckAllowRightClickSelection();
    afx_msg void OnAcceleratorTimeViewToggleHelpData3();
    afx_msg void OnAcceleratorTimeViewToggleHelpData4();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

