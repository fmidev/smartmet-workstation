#pragma once

// SmartMetView.h : interface of the CSmartMetView class
//


#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "SmartMetOpenUrlAction.h"

class NFmiToolBox;
class NFmiEditMapView;
class NFmiLocationBag;
class NFmiDrawParam;
class CSmartMetDoc;
class NFmiFastQueryInfo;
class NFmiEditMapGeneralDataDoc;
class SmartMetDocumentInterface;
class TrueMapViewSizeInfo;
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}
class NFmiRect;
class NFmiViewSettingMacro;
class CTableWnd;
class CPPToolTip;


class CSmartMetView : public CView
{
protected: // create from serialization only
	CSmartMetView();
	DECLARE_DYNCREATE(CSmartMetView)

// Attributes
public:
	CSmartMetDoc* GetDocument() const;
	void CreateEditMapView(void);
    void Update(void);
    void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool redrawMapView = false, bool clearMapViewBitmapCacheRows = false, int theWantedMapViewDescTop = -1);
    void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, SmartMetViewId updatedViewsFlag, bool redrawMapView = false, bool clearMapViewBitmapCacheRows = FALSE, int theWantedMapViewDescTop = -1);
	void DrawOverBitmapThings(NFmiToolBox * theGTB); // tällä piirretään tavara, joka tulee myös bitmapin päälle
	void ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews);
	void ActivateFilterDlg(void);
	virtual void DoDraw() ;  // You should subclass from this to get more interesting pictures
	void StoreViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro);
	void LoadViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro);
	void UpdateMapView(unsigned int theDescTopIndex);
	void ActivateZoomDialog(int theWantedDescTopIndex);
	void OldWayPrintUpdate(void); // tätä kutsutaan yleisessä printtaus funktiossa
	void SetNotificationMessage(const std::string &theNotificationMsgStr, const std::string &theNotificationTitle, int theStyle, int theTimeout, bool fNoSound);
	void PutWarningFlagTimerOn(void);
    void PutTextInStatusBar(const std::string &theText);

	// printtaus template funktioiden rajapinta funktioita CView-luokasta perityille näyttöluokille
	// ********************************************************************************************
    SmartMetDocumentInterface* GetSmartMetDocumentInterface();
    NFmiEditMapGeneralDataDoc* GetGeneralDoc(void);
	const NFmiMetTime& PrintingStartTime(void) {return itsPrintingStartTime;}
	void PrintingStartTime(const NFmiMetTime &newValue) {itsPrintingStartTime = newValue;}
	int PrintingPageShiftInMinutes(void) {return itsPrintingPageShiftInMinutes;}
	void PrintingPageShiftInMinutes(int newValue) {itsPrintingPageShiftInMinutes = newValue;}
	NFmiToolBox* ToolBox(void) {return itsToolBox;}
    CtrlViewUtils::GraphicalInfo& GetGraphicalInfo();
	TrueMapViewSizeInfo& GetTrueMapViewSizeInfo();
	void CurrentPrintTime(const NFmiMetTime &theTime);
	const NFmiRect* RelativePrintRect(void);
	void RelativePrintRect(const NFmiRect &theRect);
    CBitmap* FinalMapViewImageBitmap() { return itsFinalMapViewImageBitmap; }
    CBitmap* MemoryBitmap(void) {return itsMemoryBitmap;}
    CBitmap* MapBitmap() { return itsMapBitmap; }
    CSize GetPrintedAreaOnScreenSizeInPixels(void);
	NFmiPoint PrintViewSizeInPixels(void);
	void PrintViewSizeInPixels(const NFmiPoint &theSize);
	void SetPrintCopyCDC(CDC* pDC);
	void MakePrintViewDirty(bool fViewDirty, bool fCacheDirty);
	int CalcPrintingPageShiftInMinutes(void);
	NFmiMetTime CalcPrintingStartTime(void);
	void SetMacroErrorText(const std::string &theErrorStr);
	bool DoAllwaysLowMemoryPrint(void) const {return false;} // ks. CFmiWin32TemplateHelpers::OnPrintMapView -metodia.
	bool IsMapView() const { return true; } // ks. esim. CFmiWin32TemplateHelpers::PrintMapViewWithFullResolution -funktiota.
	int MapViewDescTopIndex(void) { return itsMapViewDescTopIndex; }
    void SetToolMastersDC(CDC* theDC); // toolmasterin DC:n
    bool GenerateMapBitmap(CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC);
    // ************************************
    void SetToolsDCs(CDC* theDC); // asettaa toolmasterin ja toolboxin DC:t
    NFmiEditMapView* EditMapView() { return itsEditMapView; }
    CBitmap* SynopPlotBitmap() { return itsSynopPlotBitmap; }
    CPPToolTip* ToolTipControl() { return m_tooltip; }
    HBITMAP SynopPlotBitmapHandle() { return itsSynopPlotBitmapHandle; }
    void SetToolBoxsDC(CDC* theDC); // toolboxin DC:n

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CSmartMetView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif // FMI_DISABLE_MFC_FEATURE_PACK
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFilePrint();
	DECLARE_MESSAGE_MAP()

private:

// HUOM!! printtauksen yhteydessä kutsu ensin DC:n asetus ja sitten printinfon!!!
	boost::shared_ptr<NFmiFastQueryInfo> GetWantedInfo(int theProducerId);
	void MouseEnter(void);
	void MouseLeave(void);
	void SetMapViewGridSize(const NFmiPoint &newSize);
	void DrawSynopPlotImage(bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot);
	std::string MakeActiveDataLocationIndexString(const NFmiPoint &theLatlon);
    void ForceOtherMapViewsDrawOverBitmapThings(unsigned int theOriginalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews);
    void DoGraphReportOnDraw(const CtrlViewUtils::GraphicalInfo &graphicalInfo, double scaleFactor);
	void OnMouseMove_Implementation(UINT nFlags, CPoint point);
	void OnLButtonDown_Implementation(UINT nFlags, CPoint point);
	void OnLButtonUp_Implementation(UINT nFlags, CPoint point);
	void OnLButtonDblClk_Implementation(UINT nFlags, CPoint point);

    CBitmap* itsFinalMapViewImageBitmap;
	CBitmap* itsMemoryBitmap;
	CBitmap* itsMapBitmap; // tähän tehdään yksi kartta bitmap, jota sitten 'lätkitään' oikeisiin kohtiin ruudukkonäytössä
	CBitmap* itsOverMapBitmap; // tähän tehdään yksi kartta bitmap, jota sitten 'lätkitään' oikeisiin kohtiin ruudukkonäytössä
	NFmiEditMapView* itsEditMapView;
	NFmiToolBox * itsToolBox;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	int itsMapViewDescTopIndex;
	bool fMouseCursorOnWnd;
	bool fDoSpecialPrinting;
	NFmiMetTime itsPrintingStartTime; // tämä on moni sivu printtauksessa oleva alkuaika
	int itsPrintingPageShiftInMinutes; // näin monta minuuttia on eri printattavien sivujen alku aikojen erotus

	CSize m_cx;
	CTableWnd *m_SizeWnd;
	CPPToolTip *m_tooltip;
	// Bitmapin, johon mahdollinen synop-plot image talletetaan.
	CBitmap* itsSynopPlotBitmap;
	HBITMAP itsSynopPlotBitmapHandle;
	SmartMetOpenUrlAction itsCurrentOpenUrlAction = SmartMetOpenUrlAction::None;

public:
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMenuitemViewGridSelectionDlg();
	afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnAcceleratorSwapArea();
	afx_msg void OnAcceleratorSwapAreaSecondaryKey();
	afx_msg void OnAcceleratorMakeSwapBaseArea();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnAcceleratorCopyAllMapViewParams();
	afx_msg void OnAcceleratorPasteAllMapViewParams();
	afx_msg void OnAcceleratorToggleAnimationView();
	afx_msg void OnAcceleratorSetHomeTime();
	afx_msg void OnAcceleratorChangeTimeByStep1Forward();
	afx_msg void OnAcceleratorChangeTimeByStep1Backward();
	afx_msg void OnAcceleratorChangeTimeByStep2Forward();
	afx_msg void OnAcceleratorChangeTimeByStep2Backward();
	afx_msg void OnAcceleratorChangeTimeByStep3Forward();
	afx_msg void OnAcceleratorChangeTimeByStep3Backward();
	afx_msg void OnAcceleratorChangeTimeByStep4Forward();
	afx_msg void OnAcceleratorChangeTimeByStep4Backward();
	afx_msg void OnUpdateButtonDataToDatabase(CCmdUI *pCmdUI);
    afx_msg void OnAcceleratorMapZoomIn();
    afx_msg void OnAcceleratorMapZoomOut();
    afx_msg void OnAcceleratorMapPanDown();
    afx_msg void OnAcceleratorMapPanLeft();
    afx_msg void OnAcceleratorMapPanRight();
    afx_msg void OnAcceleratorMapPanUp();
	afx_msg void OnDisplayChange(UINT, int, int);
	afx_msg void OnAcceleratorChangeAllModelDataOnRowToPreviousModelRun();
	afx_msg void OnAcceleratorChangeAllModelDataOnRowToNextModelRun();
	afx_msg void OnAcceleratorMapViewRangeMeterModeToggle();
	afx_msg void OnAcceleratorMapViewRangeMeterColorToggle();
	afx_msg void OnAcceleratorMapViewRangeMeterLockModeToggle();
};

#ifndef _DEBUG  // debug version in SmartMetView.cpp
inline CSmartMetDoc* CSmartMetView::GetDocument() const
   { return reinterpret_cast<CSmartMetDoc*>(m_pDocument); }
#endif

