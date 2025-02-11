#pragma once

// TimeEditValuesView.h : header file
//
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "ZoomView.h"
#include "PPToolTip.h"

/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesView view
class NFmiDrawParam;
class SmartMetDocumentInterface;
class NFmiTimeValueEditManagerView;
class NFmiRect;
class TrueMapViewSizeInfo;
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

class CTimeEditValuesView : public CZoomView//CView
{
protected:
	CTimeEditValuesView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTimeEditValuesView)

// Attributes
public:
	CTimeEditValuesView(CWnd* theParent
					   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					   , SmartMetDocumentInterface *smartMetDocumentInterface);
	bool ChangeTimeSeriesValues(void);
	void ResetAllModifyFactorValues(void);
// Operations
	void Update(void); // en saa nyt windows messageja menemään esim. Invalidate:lla, joten tein erillisen Update funktion
	void DoPrint(void);
	void AutoAdjustValueScales(bool justActiveRow, bool redrawOnSuccess);

	// printtaus template funktioiden rajapinta funktioita CView-luokasta perityille näyttöluokille
	// ********************************************************************************************
    SmartMetDocumentInterface* GetSmartMetDocumentInterface(void) {return itsSmartMetDocumentInterface;}
	NFmiMetTime CalcPrintingStartTime(void);
	const NFmiMetTime& PrintingStartTime(void) {return itsPrintingStartTime;}
	void PrintingStartTime(const NFmiMetTime &newValue) {itsPrintingStartTime = newValue;}
	int PrintingPageShiftInMinutes(void) {return itsPrintingPageShiftInMinutes;}
	void PrintingPageShiftInMinutes(int newValue) {itsPrintingPageShiftInMinutes = newValue;}
	int CalcPrintingPageShiftInMinutes(void);
	NFmiToolBox* ToolBox(void) {return itsToolBox;}
    CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(); // HUOM! tämä on dummy, ei ole vielä toteutettu hyvälaatuista printtaus koodia, missä näitä tietoja tarvitaan.
	TrueMapViewSizeInfo& GetTrueMapViewSizeInfo();
	void CurrentPrintTime(const NFmiMetTime &theTime);
	const NFmiRect* RelativePrintRect(void); 
	void RelativePrintRect(const NFmiRect &theRect); 
	CBitmap* MemoryBitmap(void){return itsMemoryBitmap;}
	CSize GetPrintedAreaOnScreenSizeInPixels(void);
	NFmiPoint PrintViewSizeInPixels(void);
	void PrintViewSizeInPixels(const NFmiPoint &theSize);
	void SetPrintCopyCDC(CDC* pDC);
	void MakePrintViewDirty(bool fViewDirty, bool fCacheDirty);
	void OldWayPrintUpdate(void); // tätä kutsutaan yleisessä printtaus funktiossa
	bool DoAllwaysLowMemoryPrint(void) const {return true;} // ks. CFmiWin32TemplateHelpers::OnPrintMapView -metodia. 
															// Aikasarjan printttaus kunnolla vaatii paljon koodia, joka osaisi laskea mm. erilaisia symboli kokoja jms.
	bool IsMapView() const { return false; } // ks. esim. CFmiWin32TemplateHelpers::PrintMapViewWithFullResolution -funktiota.
	// ************************************
    void SetToolMastersDC(CDC * /* theDC */) {};
    void DoDraw(void);
	void DrawOverBitmapThings(NFmiToolBox *theToolBox);
    int MapViewDescTopIndex(void);
	void SetToolsDCs(CDC* theDC);
	std::string MakeCsvDataString();

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeEditValuesView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTimeEditValuesView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	afx_msg UINT OnGetDlgCode();
	//{{AFX_MSG(CTimeEditValuesView)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void MakeAllMapViewsUpdated(bool newValue);

	NFmiTimeValueEditManagerView* itsManagerView;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;

	CBitmap* itsMemoryBitmap;
	CPPToolTip m_tooltip;
	NFmiMetTime itsPrintingStartTime; // tämä on moni sivu printtauksessa oleva alkuaika
	int itsPrintingPageShiftInMinutes; // näin monta minuuttia on eri printattavien sivujen alku aikojen erotus
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void HideToolTip(void);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

