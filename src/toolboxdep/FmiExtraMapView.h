#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiMetTime.h"
#include "PPToolTip.h"
#include "boost/shared_ptr.hpp"

class NFmiEditMapView;
class NFmiToolBox;
class NFmiDrawParam;
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}
class NFmiRect;
class SmartMetDocumentInterface;
class NFmiStationViewHandler;

// CFmiExtraMapView view

class CFmiExtraMapView : public CView
{
	DECLARE_DYNCREATE(CFmiExtraMapView)

protected:
	CFmiExtraMapView();           // protected constructor used by dynamic creation
	virtual ~CFmiExtraMapView();

public:
	CFmiExtraMapView(SmartMetDocumentInterface *smartMetDocumentInterface, unsigned int theMapViewDescTopIndex);

	void DoPrint(void);
	void UpdateMap(void);
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	void DoDraw(void);
	void DrawOverBitmapThings(NFmiToolBox * theGTB);
	static std::string::size_type FindPlotImagePosition(const std::string &theTootipStr, std::string &theFoundPlotStr, bool &drawSoundingPlot, bool &drawMinMaxPlot, bool &drawMetarPlot);
    NFmiStationViewHandler* GetMapViewHandler(int theRowIndex, int theIndex);
    void PutTextInStatusBar(const std::string &theText);

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
    CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(void);
	void CurrentPrintTime(const NFmiMetTime &theTime);
	const NFmiRect* RelativePrintRect(void); 
	void RelativePrintRect(const NFmiRect &theRect); 
	CBitmap* MemoryBitmap(void) {return itsMemoryBitmap;}
	CSize GetPrintedAreaOnScreenSizeInPixels(void);
	NFmiPoint PrintViewSizeInPixels(void);
	void PrintViewSizeInPixels(const NFmiPoint &theSize);
	void SetPrintCopyCDC(CDC* pDC);
	void MakePrintViewDirty(bool fViewDirty, bool fCacheDirty);
	void OldWayPrintUpdate(void); // t�t� kutsutaan yleisess� printtaus funktiossa
	bool DoAllwaysLowMemoryPrint(void) const {return false;} // Kokeilen toimiiko nyt hi-res printtaus my�s apukarttan�yt�iss�, ep�ilen ett� ongelmat ovat liittyneet datan latausten yhteydess� teht�viin n�ytt�jen p�ivityksiin, mik� on nyt estetty, kun on printtaus k�ynniss�.
															// Apukarttan�yt�t ovat ainoa poikkeus, eli ne printataan aina k�ytt�en cache kuvaa. 
															// T�m� siksi ett� kunnon prittaus ei jostain syyst� toimi oikein ainoastaan t�lle n�yt�lle.
															// ks. CFmiWin32TemplateHelpers::OnPrintMapView -metodia.
    int MapViewDescTopIndex(void) { return itsMapViewDescTopIndex; }
    bool GenerateMapBitmap(CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC, CBitmap *theOldBitmap);
    void SetToolMastersDC(CDC* theDC);
    // ********************************************************************************************
    void SetToolsDCs(CDC* theDC);
    NFmiEditMapView* EditMapView() { return itsEditMapView; }
    CBitmap* SynopPlotBitmap() { return itsSynopPlotBitmap; }
    void SynopPlotBitmap(CBitmap *bitmap) { itsSynopPlotBitmap = bitmap; }
    HBITMAP SynopPlotBitmapHandle() { return itsSynopPlotBitmapHandle; }
    CPPToolTip* ToolTipControl() { return &m_tooltip; }
    void SetToolBoxsDC(CDC* theDC);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

private:
	void CreateEditMapView(void);
	void MakeCombatibleBitmap(CBitmap **theMemoryBitmap, int cx = 0, int cy = 0);

    SmartMetDocumentInterface* itsSmartMetDocumentInterface;

	CBitmap* itsMemoryBitmap;
	CBitmap* itsMapBitmap; // t�h�n tehd��n yksi kartta bitmap, jota sitten 'l�tkit��n' oikeisiin kohtiin ruudukkon�yt�ss�
	// Bitmapin, johon mahdollinen synop-plot image talletetaan.
	CBitmap* itsSynopPlotBitmap;
	HBITMAP itsSynopPlotBitmapHandle;
	NFmiEditMapView* itsEditMapView;
	NFmiToolBox * itsToolBox;
	NFmiDrawingEnvironment itsDrawingEnvironment;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	int itsMapViewDescTopIndex;
	bool fMouseCursorOnWnd;
	CPPToolTip m_tooltip;
	NFmiMetTime itsPrintingStartTime; // t�m� on moni sivu printtauksessa oleva alkuaika
	int itsPrintingPageShiftInMinutes; // n�in monta minuuttia on eri printattavien sivujen alku aikojen erotus
	bool fPrintingOnDontSetDcs; // Kun apukarttan�yt�ll� printataan, pit�� t�m� lippu asettaa true:ksi ja sen j�lkeen pit�� est�� kaikki
								// CDC asetukset, koska jostain syyst� t�m� apukarttan�ytt� toimii erilailla printtaus-dialogin kanssa ja
								// se ei esim. blokkaa mit��n hiirien k�sittelyj�, kuten muut n�yt�t (mm. p��kartta-, luotaus-, aikasarja- ja poikkileikkausn�yt�t)
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	void ForceDrawOverBitmapThings(void);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
protected:
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
    afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
};


