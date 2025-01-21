#pragma once

#include "ZoomView.h"
#include "PPToolTip.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"

class NFmiRect;
class SmartMetDocumentInterface;
class NFmiCrossSectionManagerView;
class TrueMapViewSizeInfo;
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}
/////////////////////////////////////////////////////////////////////////////
// CFmiCrossSectionView view
// Tämä hallitseen näytön klikkaukset ja päivitykset.
// Pitää sisällään luokat, jotka huolehtivat poikkileikkauksien piirrosta.

class CFmiCrossSectionView :  public CZoomView // public CView
{
public:
	CFmiCrossSectionView(CWnd* theParent
				   , SmartMetDocumentInterface *smartMetDocumentInterface);

	void DoPrint(void);
	void Update(bool fMakeDirty);
	bool ViewDirty(void){return fViewDirty;}
	void ViewDirty(bool newState){fViewDirty = newState;}

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
	bool DoAllwaysLowMemoryPrint(void) const { return false; } // ks. CFmiWin32TemplateHelpers::OnPrintMapView -metodia.
	bool IsMapView() const {return false;} // ks. esim. CFmiWin32TemplateHelpers::PrintMapViewWithFullResolution -funktiota.
	// ********************************************************************************************
	void DoDraw(void);
	void DrawOverBitmapThings(NFmiToolBox *theToolBox);
    int MapViewDescTopIndex(void);
    void SetToolMastersDC(CDC* theDC);
    virtual void OnInitialUpdate();
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	void SetToolsDCs(CDC* theDC);
	// Couple non used parameters were added, because time-serial-view needs these and this 
	// method is used used by beta-product system through template interface.
	void AutoAdjustValueScales(bool justActiveRow, bool redrawOnSuccess);

protected:
	CFmiCrossSectionView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFmiCrossSectionView)

    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual ~CFmiCrossSectionView();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:

	NFmiCrossSectionManagerView* itsView;
	CBitmap* itsMemoryBitmap;
	bool fViewDirty;
	CPoint isCurrentMousePoint; // mouseMove:ssa laitetaan talteen currentti hiiren sijainti
	CPPToolTip m_tooltip;
	NFmiMetTime itsPrintingStartTime; // tämä on moni sivu printtauksessa oleva alkuaika
	int itsPrintingPageShiftInMinutes; // näin monta minuuttia on eri printattavien sivujen alku aikojen erotus
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

