#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "DialogEx.h"
#include "TableWnd.h"
#include "NFmiViewPosRegistryInfo.h"

class CFmiExtraMapView;
class SmartMetDocumentInterface;
class NFmiPoint;

// CFmiExtraMapViewDlg dialog

class CFmiExtraMapViewDlg : public CDialogEx2 //CDialog
{
private: // näillä talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiExtraMapViewDlg)

public:
	CFmiExtraMapViewDlg(SmartMetDocumentInterface *smartMetDocumentInterface, unsigned int theMapViewDescTopIndex, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiExtraMapViewDlg();

	void Update(bool fUpdateHiddenView = false);
	void SetDefaultValues(void);
	CFmiExtraMapView* MapView(void) {return itsView;}
    CBitmap* FinalMapViewImageBitmap();
    CBitmap* MemoryBitmap(void);
	unsigned int MapViewDescTopIndex(void) const {return itsMapViewDescTopIndex;}
    static std::string MakeUsedWinRegistryKeyStr(unsigned int theMapViewDescTopIndex);

// Dialog Data
	enum { IDD = IDD_DIALOG_MAP_VIEW_EXTRA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CRect CalcClientArea(void);
	void DoWhenClosing(void);
	void SetMapViewGridSize(const NFmiPoint &newSize);
    bool IsMapSelected(int theMapIndex);
    void MapAreaButtonSelect(unsigned int newMapIndex);

	CFmiExtraMapView *itsView;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	unsigned int itsMapViewDescTopIndex;

	CSize m_cx;
	CTableWnd m_SizeWnd;
	HACCEL m_hAccel;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnButtonSelectFinlandMap();
	afx_msg void OnUpdateButtonSelectFinlandMap(CCmdUI *pCmdUI);
	afx_msg void OnButtonSelectEuropeMap();
	afx_msg void OnUpdateButtonSelectEuropeMap(CCmdUI *pCmdUI);
	afx_msg void OnButtonSelectScandinaviaMap();
	afx_msg void OnUpdateButtonSelectScandinaviaMap(CCmdUI *pCmdUI);
	afx_msg void OnButtonGlobe();
	afx_msg void OnUpdateButtonGlobe(CCmdUI *pCmdUI);
	afx_msg void OnMenuitemViewGridSelectionDlg();
	afx_msg void OnButtonDataArea();
	afx_msg void OnButtonMapViewSettings();
protected:
	virtual void OnOK();
	virtual void OnCancel();
	CString GetFinalStatusString(UINT nID, const CString &strPromtpText);
public:
	afx_msg void OnClose();
protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnShowGridPoints();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnToggleGridPointColor();
	afx_msg void OnToggleGridPointSize();
	afx_msg void OnEditSpaceOut();
	afx_msg void OnChangeParamWindowPositionForward();
	afx_msg void OnChangeParamWindowPositionBackward();
	afx_msg void OnShowTimeString();
	afx_msg void OnButtonRefresh();
	afx_msg void OnChangeMapType();
	afx_msg void OnToggleShowNamesOnMap();
	afx_msg void OnShowMasksOnMap();
	afx_msg void OnShowProjectionLines();
	afx_msg void OnToggleLandBorderDrawColor();
	afx_msg void OnToggleLandBorderPenSize();
	afx_msg void OnAcceleratorBorrowParams1();
	afx_msg void OnAcceleratorBorrowParams2();
	afx_msg void OnAcceleratorBorrowParams3();
	afx_msg void OnAcceleratorBorrowParams4();
	afx_msg void OnAcceleratorBorrowParams5();
	afx_msg void OnAcceleratorBorrowParams6();
	afx_msg void OnAcceleratorBorrowParams7();
	afx_msg void OnAcceleratorBorrowParams8();
	afx_msg void OnAcceleratorBorrowParams9();
	afx_msg void OnAcceleratorBorrowParams10();
	afx_msg void OnAcceleratorMapRow1();
	afx_msg void OnAcceleratorMapRow2();
	afx_msg void OnAcceleratorMapRow3();
	afx_msg void OnAcceleratorMapRow4();
	afx_msg void OnAcceleratorMapRow5();
    afx_msg void OnAcceleratorMapRow6();
    afx_msg void OnAcceleratorMapRow7();
    afx_msg void OnAcceleratorMapRow8();
    afx_msg void OnAcceleratorMapRow9();
    afx_msg void OnAcceleratorMapRow10();
    afx_msg void OnAcceleratorToggleHelpCursorOnMap();
	afx_msg void OnAcceleratorToggleOvermapForeBackGround();
	afx_msg void OnAcceleratorToggleTooltip();
	afx_msg void OnAcceleratorToggleKeepMapRatio();
	afx_msg BOOL OnToolTipText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnButtonSelectParamDialogExtraMap();
    afx_msg void OnButtonZoomDialog();
	afx_msg void OnToggleMapViewDisplayMode();
	afx_msg void OnButtonExtraMapPrint();
	afx_msg void OnAcceleratorLockTimeToMainMap();
	afx_msg void OnAcceleratorMakeSwapBaseArea();
	afx_msg void OnAcceleratorSwapArea();
	afx_msg void OnAcceleratorSwapAreaSecondaryKey();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnAcceleratorCopyAllMapViewParams();
	afx_msg void OnAcceleratorPasteAllMapViewParams();
	afx_msg void OnAcceleratorToggleAnimationView();
	afx_msg void OnAcceleratorSetHomeTime();
	afx_msg void OnToggleShowPreviousNamesOnMap();
	afx_msg void OnChangePreviousMapType();
	afx_msg void OnButtonAnimation();
    afx_msg void OnAcceleratorExtraMapPanDown();
    afx_msg void OnAcceleratorExtraMapPanLeft();
    afx_msg void OnAcceleratorExtraMapPanRight();
    afx_msg void OnAcceleratorExtraMapPanUp();
    afx_msg void OnAcceleratorExtraMapZoomIn();
    afx_msg void OnAcceleratorExtraMapZoomOut();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnAcceleratorMoveManyMapRowsUp();
    afx_msg void OnAcceleratorMoveManyMapRowsDown();
    afx_msg void OnAcceleratorLockRowToMainMap();
    afx_msg void OnAcceleratorExtraMapLocationFinderTool();
	afx_msg void OnAcceleratorChangeTimeByStep1Forward();
	afx_msg void OnAcceleratorChangeTimeByStep1Backward();
	afx_msg void OnAcceleratorChangeTimeByStep2Forward();
	afx_msg void OnAcceleratorChangeTimeByStep2Backward();
	afx_msg void OnAcceleratorChangeTimeByStep3Forward();
	afx_msg void OnAcceleratorChangeTimeByStep3Backward();
	afx_msg void OnAcceleratorChangeTimeByStep4Forward();
	afx_msg void OnAcceleratorChangeTimeByStep4Backward();
	afx_msg void OnEditVisualizationsettingsExtraMap();
	afx_msg void OnAcceleratorEmChangeAllModelDataOnRowToPreviousModelRun();
	afx_msg void OnAcceleratorEmChangeAllModelDataOnRowToNextModelRun();
	afx_msg void OnAcceleratorExtraMapViewRangeMeterModeToggle();
	afx_msg void OnAcceleratorExtraMapViewRangeMeterColorToggle();
	afx_msg void OnAcceleratorLogViewerToolboxdeb();
	afx_msg void OnButtonOpenMainMapViewExtraMap();
	afx_msg void OnButtonOpenOtherSideMapViewExtraMap();
	afx_msg void OnButtonOpenTimeSerialViewExtraMap();
	afx_msg void OnButtonOpenSmarttoolsDialogExtraMap();
	afx_msg void OnButtonOpenSoundingViewExtraMap();
	afx_msg void OnButtonOpenCrosssectionViewExtraMap();
	afx_msg void OnButtonOpenViewMacroDialogExtraMap();
	afx_msg void OnAcceleratorExtraMapViewRangeMeterLockModeToggle();
	afx_msg void OnAcceleratorExtraMapMoveTimeBoxLocation();
	afx_msg void OnAcceleratorExtraMapObsComparisonMode();
	afx_msg void OnAcceleratorExtraMapToggleVirtualTimeMode();
};
