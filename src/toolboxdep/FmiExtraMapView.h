#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "PPToolTip.h"
#include "CtrlViewWin32Functions.h"
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
class TrueMapViewSizeInfo;

struct BitmapDeleter 
{
    bool deleteBitmap_ = true;
    BitmapDeleter(bool deleteBitmap)
        :deleteBitmap_(deleteBitmap)
    {}

    void operator()(CBitmap *bitmap) 
    { 
        CtrlView::DestroyBitmap(&bitmap, true);
    }
};

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

	// printtaus template funktioiden rajapinta funktioita CView-luokasta perityille n‰yttˆluokille
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
    CBitmap* FinalMapViewImageBitmap() { return itsFinalMapViewImageBitmap.get(); }
    CBitmap* MemoryBitmap(void) { return itsMemoryBitmap.get(); }
    CBitmap* MapBitmap(void) { return itsMapBitmap.get(); }
	CSize GetPrintedAreaOnScreenSizeInPixels(void);
	NFmiPoint PrintViewSizeInPixels(void);
	void PrintViewSizeInPixels(const NFmiPoint &theSize);
	void SetPrintCopyCDC(CDC* pDC);
	void MakePrintViewDirty(bool fViewDirty, bool fCacheDirty);
	void OldWayPrintUpdate(void); // t‰t‰ kutsutaan yleisess‰ printtaus funktiossa
	bool DoAllwaysLowMemoryPrint(void) const {return false;} // Kokeilen toimiiko nyt hi-res printtaus myˆs apukarttan‰ytˆiss‰, ep‰ilen ett‰ ongelmat ovat liittyneet datan latausten yhteydess‰ teht‰viin n‰yttˆjen p‰ivityksiin, mik‰ on nyt estetty, kun on printtaus k‰ynniss‰.
															// Apukarttan‰ytˆt ovat ainoa poikkeus, eli ne printataan aina k‰ytt‰en cache kuvaa. 
															// T‰m‰ siksi ett‰ kunnon prittaus ei jostain syyst‰ toimi oikein ainoastaan t‰lle n‰ytˆlle.
															// ks. CFmiWin32TemplateHelpers::OnPrintMapView -metodia.
	bool IsMapView() const { return true; } // ks. esim. CFmiWin32TemplateHelpers::PrintMapViewWithFullResolution -funktiota.
	int MapViewDescTopIndex(void) { return itsMapViewDescTopIndex; }
    bool GenerateMapBitmap(CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC);
    void SetToolMastersDC(CDC* theDC);
    // ********************************************************************************************
    void SetToolsDCs(CDC* theDC);
    NFmiEditMapView* EditMapView() { return itsEditMapView; }
    CBitmap* SynopPlotBitmap() { return itsSynopPlotBitmap.get(); }
    HBITMAP SynopPlotBitmapHandle() { return itsSynopPlotBitmapHandle; }
    CPPToolTip* ToolTipControl() { return &m_tooltip; }
    void SetToolBoxsDC(CDC* theDC);
	void ForceDrawOverBitmapThingsThisExtraMapView(void);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

private:
	void CreateEditMapView(void);
	void OnMouseMove_Implementation(UINT nFlags, CPoint point);
	void OnLButtonDown_Implementation(UINT nFlags, CPoint point);
	void OnLButtonUp_Implementation(UINT nFlags, CPoint point);
	void OnLButtonDblClk_Implementation(UINT nFlags, CPoint point);

    SmartMetDocumentInterface* itsSmartMetDocumentInterface;

    // CBitmap membereille k‰ytet‰‰n unique_ptr ratkaisua, koska tˆrm‰sin tosi outoon SmartMet kaato bugiin,
    // kun avattiin apukarttan‰yttˆj‰. Kun karttan‰yttˆ luotiin, oli sen itsMapBitmap luotu normaalisti.
    // Mutta kun karttan‰yttˆ‰ yritettiin piirt‰‰ CFmiWin32TemplateHelpers::MapViewOnDraw funktiossa, palautti
    // mapView->MapBitmap() -kutsu nullptr olion. Oliota ei oltu miss‰‰n vaiheessa tuhottu/nollattu. 
    // Mielest‰ni kyse oli jostain h‰m‰r‰st‰ VC++ 2015 k‰‰nt‰j‰n bugista. Kun muutin koodin k‰ytt‰m‰‰n
    // unique_ptr -oliota, ei ongelmaa en‰‰ esiintynyt. Siksi kaikki CBitmap oliot t‰‰ll‰ laitetaan nyt unique_ptr:in sis‰‰n.
    std::unique_ptr<CBitmap, BitmapDeleter> itsFinalMapViewImageBitmap;
    std::unique_ptr<CBitmap, BitmapDeleter> itsMemoryBitmap;
    // T‰h‰n tehd‰‰n yksi kartta bitmap, jota sitten 'l‰tkit‰‰n' oikeisiin kohtiin ruudukkon‰ytˆss‰
	std::unique_ptr<CBitmap, BitmapDeleter> itsMapBitmap;
	// Bitmapin, johon mahdollinen synop-plot image talletetaan. T‰h‰n ei saa k‰ytt‰‰ BitmapDeleter unique_ptr deleteria!
    std::unique_ptr<CBitmap> itsSynopPlotBitmap;
	HBITMAP itsSynopPlotBitmapHandle;
	NFmiEditMapView* itsEditMapView;
	NFmiToolBox * itsToolBox;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	int itsMapViewDescTopIndex;
	bool fMouseCursorOnWnd;
	CPPToolTip m_tooltip;
    // T‰m‰ on moni sivu printtauksessa oleva alkuaika
	NFmiMetTime itsPrintingStartTime; 
    // N‰in monta minuuttia on eri printattavien sivujen alku aikojen erotus
	int itsPrintingPageShiftInMinutes; 
    // Kun apukarttan‰ytˆll‰ printataan, pit‰‰ t‰m‰ lippu asettaa true:ksi ja sen j‰lkeen pit‰‰ est‰‰ kaikki
    // CDC asetukset, koska jostain syyst‰ t‰m‰ apukarttan‰yttˆ toimii erilailla printtaus-dialogin kanssa ja
    // se ei esim. blokkaa mit‰‰n hiirien k‰sittelyj‰, kuten muut n‰ytˆt (mm. p‰‰kartta-, luotaus-, aikasarja- ja poikkileikkausn‰ytˆt)
	bool fPrintingOnDontSetDcs; 
	SmartMetOpenUrlAction itsCurrentOpenUrlAction = SmartMetOpenUrlAction::None;
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
    afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
};


