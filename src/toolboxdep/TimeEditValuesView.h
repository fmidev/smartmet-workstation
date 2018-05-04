#pragma once

// TimeEditValuesView.h : header file
//
#include "NFmiMetTime.h"
#include "ZoomView.h"
#include "PPToolTip.h"

/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesView view
class NFmiDrawParam;
class SmartMetDocumentInterface;
class NFmiTimeValueEditManagerView;
class NFmiRect;
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
	void ManualModifierLength(double newValue);
// Operations
	void Update(void); // en saa nyt windows messageja menem��n esim. Invalidate:lla, joten tein erillisen Update funktion
	void DoPrint(void);
	void AutoAdjustValueScales(bool fJustActive);

	// printtaus template funktioiden rajapinta funktioita CView-luokasta perityille n�ytt�luokille
	// ********************************************************************************************
    SmartMetDocumentInterface* GetSmartMetDocumentInterface(void) {return itsSmartMetDocumentInterface;}
	NFmiMetTime CalcPrintingStartTime(void);
	const NFmiMetTime& PrintingStartTime(void) {return itsPrintingStartTime;}
	void PrintingStartTime(const NFmiMetTime &newValue) {itsPrintingStartTime = newValue;}
	int PrintingPageShiftInMinutes(void) {return itsPrintingPageShiftInMinutes;}
	void PrintingPageShiftInMinutes(int newValue) {itsPrintingPageShiftInMinutes = newValue;}
	int CalcPrintingPageShiftInMinutes(void);
	NFmiToolBox* ToolBox(void) {return itsToolBox;}
    CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(void); // HUOM! t�m� on dummy, ei ole viel� toteutettu hyv�laatuista printtaus koodia, miss� n�it� tietoja tarvitaan.
	void CurrentPrintTime(const NFmiMetTime &theTime);
	const NFmiRect* RelativePrintRect(void); 
	void RelativePrintRect(const NFmiRect &theRect); 
	CBitmap* MemoryBitmap(void){return itsMemoryBitmap;}
	CSize GetPrintedAreaOnScreenSizeInPixels(void);
	NFmiPoint PrintViewSizeInPixels(void);
	void PrintViewSizeInPixels(const NFmiPoint &theSize);
	void SetPrintCopyCDC(CDC* pDC);
	void MakePrintViewDirty(bool fViewDirty, bool fCacheDirty);
	void OldWayPrintUpdate(void); // t�t� kutsutaan yleisess� printtaus funktiossa
	bool DoAllwaysLowMemoryPrint(void) const {return true;} // ks. CFmiWin32TemplateHelpers::OnPrintMapView -metodia. 
															// Aikasarjan printttaus kunnolla vaatii paljon koodia, joka osaisi laskea mm. erilaisia symboli kokoja jms.
	// ************************************
    void SetToolMastersDC(CDC * /* theDC */) {};
    void DoDraw(void);
	void DrawOverBitmapThings(NFmiToolBox *theToolBox);

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
	NFmiMetTime itsPrintingStartTime; // t�m� on moni sivu printtauksessa oleva alkuaika
	int itsPrintingPageShiftInMinutes; // n�in monta minuuttia on eri printattavien sivujen alku aikojen erotus
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void HideToolTip(void);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

