#pragma once

// FmiTempView.h : header file
//

#include "ZoomView.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "PPToolTip.h"

class NFmiSoundingView;
class NFmiTempView;
class NFmiRect;
class TrueMapViewSizeInfo;
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

/////////////////////////////////////////////////////////////////////////////
// CFmiTempView view

class CFmiTempView :  public CZoomView // public CView
{
public:
	CFmiTempView(CWnd* theParent, SmartMetDocumentInterface *smartMetDocumentInterface);

	void DoPrint(void);
	void Update(bool fMakeDirty);
	void ResetScales(void);
	bool ViewDirty(void){return fViewDirty;}
	void ViewDirty(bool newState){fViewDirty = newState;}
	void DrawOverBitmapThings(NFmiToolBox *theToolBox);
    int MapViewDescTopIndex(void);
    void ResetSoundingData(void);
	const std::string& SoundingIndexStr(void) const;
	void DoDraw(void);

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
	CtrlViewUtils::GraphicalInfo& GetGraphicalInfo();
	TrueMapViewSizeInfo& GetTrueMapViewSizeInfo();
	void CurrentPrintTime(const NFmiMetTime &theTime);
	const NFmiRect* RelativePrintRect(void); 
	void RelativePrintRect(const NFmiRect &theRect); 
	CBitmap* MemoryBitmap(void) {return itsMemoryBitmap;}
	CSize GetPrintedAreaOnScreenSizeInPixels(void);
	NFmiPoint PrintViewSizeInPixels(void);
	void PrintViewSizeInPixels(const NFmiPoint &theSize);
	void SetPrintCopyCDC(CDC* pDC);
	void MakePrintViewDirty(bool fViewDirty, bool fCacheDirty);
	void OldWayPrintUpdate(void); // tätä kutsutaan yleisessä printtaus funktiossa 
	bool DoAllwaysLowMemoryPrint(void) const {return false;} // ks. CFmiWin32TemplateHelpers::OnPrintMapView -metodia.
	bool IsMapView() const { return false; } // ks. esim. CFmiWin32TemplateHelpers::PrintMapViewWithFullResolution -funktiota.
	// ********************************************************************************************
    void SetToolMastersDC(CDC * /* theDC */ ) {};
	void SetToolsDCs(CDC* theDC);
	// Couple non used parameters were added, because time-serial-view needs these and this 
	// method is used used by beta-product system through template interface.
	void AutoAdjustValueScales(bool justActiveRow, bool redrawOnSuccess);

protected:
	CFmiTempView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFmiTempView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiTempView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFmiTempView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFmiTempView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	NFmiTempView*	itsView;

	CBitmap* itsMemoryBitmap;
	CPoint isCurrentMousePoint; // mouseMove:ssa laitetaan talteen currentti hiiren sijainti
	bool fViewDirty;
	NFmiMetTime itsPrintingStartTime; // tämä on moni sivu printtauksessa oleva alkuaika
	int itsPrintingPageShiftInMinutes; // näin monta minuuttia on eri printattavien sivujen alku aikojen erotus

	CPPToolTip m_tooltip; // tooltip-otus
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
protected:
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

