// ======================================================================
/*!
 * \file NFmiMapViewDescTop.h
 * \brief This class handles all the thing required to show individual map-views.
 * There will be several different mapviews in editor and GeneralDocument contains
 * vector of these objects.
 */
// ======================================================================

#pragma once

#include <vector>

#include "NFmiMapViewCache.h"
#include "NFmiTimeDescriptor.h"
#include "NFmiColor.h"
#include "NFmiDrawParamList.h"
#include "NFmiRect.h"
#include "NFmiGridPointCache.h"
#include "NFmiPtrList.h"
#include "MapViewMode.h"
#include "GraphicalInfo.h"
#include "NFmiAnimationData.h"
#include "NFmiCountryBorderBitmapCache.h"
#include "CombinedMapHandlerInterface.h"
#include "TrueMapViewSizeInfo.h"

class CDC;
class NFmiPolyline;
class NFmiProjectionCurvatureInfo;
class NFmiMapConfigurationSystem;
class NFmiMapConfiguration;
class NFmiGdiPlusImageMapHandler;
class CWnd;
class NFmiMapViewWinRegistry;
class OGRGeometry;

namespace Imagine
{
    class NFmiPath;
}
namespace Gdiplus
{
    class Bitmap;
}

class NFmiMapViewDescTop
{
public:


	class ViewMacroDipMapHelper
	{
	public:
		ViewMacroDipMapHelper();

		// HUOM!! T�m� laittaa kommentteja mukaan!
		void Write(std::ostream& os) const;
		// HUOM!! ennen kuin t�m� luokka luetaan sis��n tiedostosta, poista kommentit
		// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
		// sekaan. Eli �l� k�yt� suoraan t�t� metodia, vaan Init(filename)-metodia!!!!
		void Read(std::istream& is);

		// dipmaphandler osio pit�� ottaa t�h�n erikois k�sittelyyn
		int itsUsedMapIndex = 0;
		int itsUsedOverMapDibIndex = -1;
		int itsUsedCombinedModeMapIndex = 0;
		int itsUsedCombinedModeOverMapDibIndex = -1;
		// T�h�n luetaan vanhoista n�ytt�makroista pre-wgs84 newbase:n NFmiArea::AreaStr metodin arvo (esim. stereographic,20,90,60:6,51.3,49,70.2).
		// Uudessa newbasessa t�h�n k�ytet��n uutta NFmiArea::AreaFactoryStr metodia.
		NFmiString itsAreaFactoryStr;
		// T�h�n talletetaan uusi Proj-kirjaston pohjainen projektio+area string, joka tehd��n NFmiArea::AreaFactoryProjStr metodilla.
		// T�m� arvo talletetaan NFmiExtraDataStorage:n listoihin. Vanhat SmartMetit ignooraavat siis t�m�n, mutta jos t�h�n l�ytyy arvo,
		// sit� k�ytet��n uudessa uuden NFmiArea olion luontiin, mutta jos t�m� tyhj�, k�ytet��n itsAreaFactoryStr:ia sitten.
		std::string itsAreaFactoryProjStr;
	};


	NFmiMapViewDescTop();
	NFmiMapViewDescTop(const std::string &theSettingsBaseName, NFmiMapConfigurationSystem *theMapConfigurationSystem, NFmiProjectionCurvatureInfo* theProjectionCurvatureInfo, const std::string &theControlPath, int theMapViewDescTopIndex);
	NFmiMapViewDescTop(const NFmiMapViewDescTop& other);
	NFmiMapViewDescTop& operator=(const NFmiMapViewDescTop& other);
	~NFmiMapViewDescTop();

	void Init(NFmiMapViewWinRegistry &theMapViewWinRegistry);
	void StoreMapViewDescTopToSettings(void);
    void InitForViewMacro(const NFmiMapViewDescTop &theOther, NFmiMapViewWinRegistry &theMapViewWinRegistry, bool getFromRegisty);
	std::vector<ViewMacroDipMapHelper> GetViewMacroDipMapHelperList(void);
	void SetViewMacroDipMapHelperList(const std::vector<ViewMacroDipMapHelper> &theData);
	void StoreToMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry);

	NFmiProjectionCurvatureInfo* ProjectionCurvatureInfo(void) {return itsProjectionCurvatureInfo;}
	void SelectedMapIndex(unsigned int newValue);
    unsigned int SelectedMapIndex(void) const {return itsSelectedMapIndexVM;}
	NFmiGdiPlusImageMapHandler* MapHandler(void) const;
    std::vector<NFmiGdiPlusImageMapHandler*>& GdiPlusImageMapHandlerList(void) {return itsGdiPlusImageMapHandlerList;}

	NFmiMapViewCache& MapViewCache(void) {return itsMapViewCache;}
    bool RedrawMapView(void) const;
    void SetRedrawMapView(bool newValue);
    void ClearRedrawMapView();
	NFmiPtrList<NFmiDrawParamList>* DrawParamListVector(void) {return itsDrawParamListVector;}
	bool ScrollViewRow(int theCount, int &theActiveViewRow);
    const NFmiPoint& ViewGridSize(void) const {return itsViewGridSizeVM;}
	void ViewGridSize(const NFmiPoint& newSize, NFmiMapViewWinRegistry *theMapViewWinRegistry);
	void ViewGridSizeMax(const NFmiPoint& newSize){itsViewGridSizeMax = newSize;};
    void ShowStationPlotVM(bool newValue) {fShowStationPlotVM = newValue;}
	const NFmiPoint& ViewGridSizeMax(void){return itsViewGridSizeMax;};
	int MapRowStartingIndex(void) const {return itsMapRowStartingIndex;}
	void MapRowStartingIndex(int newValue);
	bool IsVisibleRow(int theRowIndex);
	const NFmiMetTime& CurrentTime(void) const {return itsCurrentTime;}
	void CurrentTime(const NFmiMetTime &newValue) {itsCurrentTime = newValue;}
	float TimeControlTimeStep(void) const {return itsTimeControlTimeStep;}
	void TimeControlTimeStep(float newValue);
    void MapViewDirty(bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool updateMapViewDrawingLayers);
    CtrlViewUtils::MapViewMode MapViewDisplayMode(void) const {return itsMapViewDisplayMode;}
	void MapViewDisplayMode(CtrlViewUtils::MapViewMode newValue);
	bool ShowTimeString(void){return fShowTimeString;}
	void ShowTimeString(bool newState){fShowTimeString = newState;}
	const NFmiRect& RelativeMapRect(void);
	void RelativeMapRect(const NFmiRect& theMapRect){itsRelativeMapRect = theMapRect;};
	void CalcClientViewXperYRatio(const NFmiPoint& theViewSize);
	double ClientViewXperYRatio(void){return itsClientViewXperYRatio;};
	void ClientViewXperYRatio(double theClientViewXperYRatio){itsClientViewXperYRatio = theClientViewXperYRatio;};
	const NFmiPoint& MapViewSizeInPixels(void){return itsMapViewSizeInPixels;};
	void MapViewSizeInPixels(const NFmiPoint& newSize, CDC* pDC, double theDrawObjectScaleFactor, bool fHideTimeControlView);
	int ToggleShowTimeOnMapMode(void);
	int ShowTimeOnMapMode(void) const {return itsShowTimeOnMapMode;}
	bool IsTimeControlViewVisible() const;
	void ShowTimeOnMapMode(int newValue);
	CDC* CopyCDC(void){return itsCopyCDC;};
	void CopyCDC(CDC *theCDC){itsCopyCDC = theCDC;};
	int DrawOverMapMode(void) {return itsDrawOverMapMode;}
	void DrawOverMapMode(int newValue) {itsDrawOverMapMode = newValue;}
	CDC* MapBlitDC(void){return itsMapBlitDC;};
	void MapBlitDC(CDC* theDC){itsMapBlitDC=theDC;};
	bool ShowParamWindowView(void){return fShowParamWindowView;};
	void ShowParamWindowView(bool newState){fShowParamWindowView = newState;};
	std::shared_ptr<OGRGeometry> CountryBorderGeometry();
	void ToggleLandBorderColor(void);

	// N�m� border layer piirtoon liittyv�t metodit jotka ottavat separateBorderLayerDrawOptions parametrin
	// toimivat seuraavalla periaatteella:
	// Jos k�ytt�j� haluaa tietoja yleisest� border-draw asetuksista, on em. parametri nullptr.
	// Jos se on nullptr:st� poikkeava, kyse on erillinen border-layer, jonka tiedot haetaan erikseen.
	const NFmiColor& LandBorderColor(NFmiDrawParam* separateBorderLayerDrawOptions);
	bool DrawLandBorders(NFmiDrawParam* separateBorderLayerDrawOptions);
	int LandBorderPenSize(NFmiDrawParam* separateBorderLayerDrawOptions);
	bool BorderDrawBitmapDirty(NFmiDrawParam* separateBorderLayerDrawOptions) const;
	bool BorderDrawPolylinesDirty() const;
	bool BorderDrawPolylinesGdiplusDirty() const;
	Gdiplus::Bitmap* LandBorderMapBitmap(NFmiDrawParam* separateBorderLayerDrawOptions) const;
    void SetLandBorderMapBitmap(Gdiplus::Bitmap *newBitmap, NFmiDrawParam* separateBorderLayerDrawOptions);

	static std::string MakeSeparateBorderLayerCacheKey(const NFmiDrawParam& borderLayerDrawOptions);
	static int GetSeparateBorderLayerLineWidthInPixels(const NFmiDrawParam& borderLayerDrawOptions);
	static std::string MakeSeparateBorderLayerCacheKey(int lineWidthInPixels, const NFmiColor& color);

	void LandBorderPenSize(const NFmiPoint &newValue){itsLandBorderPenSize = newValue;}
	void ToggleLandBorderPenSize(void);
	std::list<NFmiPolyline*>& DrawBorderPolyLineList();
	void DrawBorderPolyLineList(std::list<NFmiPolyline*> &newValue);
    const std::list<std::vector<NFmiPoint>>& DrawBorderPolyLineListGdiplus();
    void DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>> &newValue);
    void DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>> &&newValue);
	void SetBorderDrawDirtyState(CountryBorderDrawDirtyState newState, NFmiDrawParam* separateBorderLayerDrawOptions = nullptr);
	void SetBorderDrawDirtyState(CountryBorderDrawDirtyState newState, const std::string &cacheKey);
	int LandBorderColorIndex(void) const {return itsLandBorderColorIndex;}
	void LandBorderColorIndex(int newValue) {itsLandBorderColorIndex = newValue;}
	bool DescTopOn(void) {return fDescTopOn;}
	void DescTopOn(bool newValue) {fDescTopOn = newValue;}
	bool MapViewBitmapDirty(void) const {return fMapViewBitmapDirty;}
	void MapViewBitmapDirty(bool newValue) {fMapViewBitmapDirty = newValue;}
	const NFmiColor& StationPointColor(void) const;
	const NFmiPoint& StationPointSize(void) const;
	void ToggleStationPointColor(void);
	void ToggleStationPointSize(void);
	bool SetMapViewGrid(const NFmiPoint &newValue, NFmiMapViewWinRegistry *theMapViewWinRegistry);
	CWnd* MapView(void) {return itsMapView;}
	void MapView(CWnd *theMapView) {itsMapView = theMapView;}
	CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(){return itsGraphicalInfo;}
	TrueMapViewSizeInfo& GetTrueMapViewSizeInfo() { return itsTrueMapViewSizeInfo; }
	NFmiGridPointCache& GridPointCache(void){return itsGridPointCache;}
	bool LockToMainMapViewTime(void) const {return fLockToMainMapViewTime;}
	void LockToMainMapViewTime(bool newValue) {fLockToMainMapViewTime = newValue;}
    bool LockToMainMapViewRow(void) const { return fLockToMainMapViewRow; }
    void LockToMainMapViewRow(bool newValue) { fLockToMainMapViewRow = newValue; }
    bool ShowTrajectorsOnMap(void) const {return fShowTrajectorsOnMap;}
	void ShowTrajectorsOnMap(bool newValue) {fShowTrajectorsOnMap = newValue;}
	bool ShowSoundingMarkersOnMap(void) const {return fShowSoundingMarkersOnMap;}
	void ShowSoundingMarkersOnMap(bool newValue) {fShowSoundingMarkersOnMap = newValue;}
	bool ShowCrossSectionMarkersOnMap(void) const {return fShowCrossSectionMarkersOnMap;}
	void ShowCrossSectionMarkersOnMap(bool newValue) {fShowCrossSectionMarkersOnMap = newValue;}
	bool ShowSelectedPointsOnMap(void) const {return fShowSelectedPointsOnMap;}
	void ShowSelectedPointsOnMap(bool newValue) {fShowSelectedPointsOnMap = newValue;}
	bool ShowControlPointsOnMap(void) const {return fShowControlPointsOnMap;}
	void ShowControlPointsOnMap(bool newValue) {fShowControlPointsOnMap = newValue;}
	bool ShowObsComparisonOnMap(void) const {return fShowObsComparisonOnMap;}
	void ShowObsComparisonOnMap(bool newValue) {fShowObsComparisonOnMap = newValue;}
	bool ShowWarningMarkersOnMap(void) const {return fShowWarningMarkersOnMap;}
	void ShowWarningMarkersOnMap(bool newValue) {fShowWarningMarkersOnMap = newValue;}
	void ToggleMapViewDisplayMode(void);
	NFmiPoint ActualMapBitmapSizeInPixels(void);
	const NFmiTimeDescriptor& TimeControlViewTimes(void) const {return itsTimeControlViewTimes;}
	void TimeControlViewTimes(const NFmiTimeDescriptor &newTimeDescriptor);
	int ActiveViewRow(void) const {return itsActiveViewRow;}
	void ActiveViewRow(int newValue) {itsActiveViewRow = newValue;}
    NFmiAnimationData& AnimationDataRef(void) {return itsAnimationData;}
	int CalcPrintingPageShiftInMinutes(void);
	void SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime);
	std::string GetCurrentMapLayerText(bool backgroundMap);
	double SingleMapViewHeightInMilliMeters() const;

	// HUOM!! T�m� laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin t�m� luokka luetaan sis��n tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli �l� k�yt� suoraan t�t� metodia, vaan Init(filename)-metodia!!!!
	void Read(std::istream& is);
private:
	void Clear(void);
	void InitStationPointDrawingSystem(void);
	void InitMapViewDescTopFromSettings(void);
	void InitGdiPlusImageMapHandlerSystem(void);
	NFmiGdiPlusImageMapHandler* CreateGdiPlusImageMapHandler(const NFmiMapConfiguration &theMapConfiguration);
	void SetGdiPlusImageMapHandlerSelectedMaps(NFmiGdiPlusImageMapHandler &theGdiPlusImageMapHandler, int mapHandlerIndex);
	void StoreHandlerSelectedMapsToSettings(NFmiGdiPlusImageMapHandler &theGdiPlusImageMapHandler, int mapHandlerIndex);
	void InitLandBorderDrawingSystem(void);
	void InitMapViewDrawParamListVector(void);
	void SetSelectedMapsFromSettings(void);
	void InitFromMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry);
    int CalcVisibleRowCount() const;
    int CalcMaxRowStartingIndex() const;
	void StoreHandlerSelectedMapsToSettings(void);
    void UpdateOneMapViewSize();
	const Gdiplus::Bitmap* GetSeparateBorderLayerCacheBitmap(const std::string& cacheKeyString);
	void InsertSeparateBorderLayerCacheBitmap(const std::string& cacheKeyString, std::unique_ptr<Gdiplus::Bitmap>&& cacheBitmap);
	void ClearBaseLandBorderMapBitmap();

	// t�ll� aloitus pohjalla luetaan conffi fileist� tarvittavat alustukset (esim. "MapViewDescTop::map1")
	std::string itsSettingsBaseName; 
	// Joskus luokan pit�� tiet�� mik� descTop itse on
	int itsMapViewDescTopIndex; 
	// t�m� on dokumentin omistuksessa
	NFmiMapConfigurationSystem *itsMapConfigurationSystem = nullptr; 
	// t�m� on dokumentin omistuksessa
	NFmiProjectionCurvatureInfo* itsProjectionCurvatureInfo = nullptr;
	// SmartMetin kontrollipolku (annetaan -p optiolla), t�m�n avulla luetaan kartta kuvia ja area-tietoja.
	// OLI aiemmin editorin ty�hakemisto, mutta se ei toiminut oikein, varsinkin kun kyseess� 
	// oli PV-projekti jossa dropbox-konffit ja sit� k�ytettiin VC++ debuggerista.
	std::string itsControlPath; 
	// jokaisella desctopilla pit�� olla oma 'map-serverins�'
	std::vector<NFmiGdiPlusImageMapHandler*> itsGdiPlusImageMapHandlerList; 
	// luokka joka pitaa huolta karttanayton cachesta
	NFmiMapViewCache itsMapViewCache; 
	bool fRedrawMapView;
	// t�h�n talletetaan raja viivan piirron v�ri vaihtoehdot
	std::vector<NFmiColor> itsLandBorderColors; 
	// valitun v�rin indeksi on tallessa t�ss�, jos indeksi on negatiivinen, ei piirret� rajaviivoja
	int itsLandBorderColorIndex; 
	NFmiPoint itsLandBorderPenSize;
	NFmiTimeDescriptor itsTimeControlViewTimes;
	// t�t� k�ytet��n mm. zoomausdialogin rajoittimena(k�ytetyn karttan�yt�n x/y suhde, riippuu k�ytetyst� ruudukosta ja ikkunan 'fyysisest�' koosta)
	double itsClientViewXperYRatio;	
	// mink� suhteellisen osan ottaa 'kartasto' clientn�yt�lle varatusta osasta
	NFmiRect itsRelativeMapRect; 
	// p�ivitet��n CView:in OnSize()-metodissa, k�ytet��n datan harvennus laskuissa.
	NFmiPoint itsMapViewSizeInPixels; 
	bool fShowParamWindowView;
	// lista drawparam-listoja (n�ytt�ruudukossa eri rivit laitetaan
	// omaan drawparamlist:iin, jotka sijoitetaan t�h�n listojen listaan)
	NFmiPtrList<NFmiDrawParamList> *itsDrawParamListVector = nullptr;
	// t�h�n talletetaan CView:ssa tehty DC, johon on talletettu bitmap, mik� sitten 
	// blitataan jokaiseen n�ytt�ruudun kohtaan erikseen (toivottavasti vain v�liaikainen viritys)
	CDC* itsMapBlitDC = nullptr;
	// jos 0, piirret��n l�pin�kyv� kartta ns. background karttaan, jos 1, piirret��n se ns. foreground karttaan 
	// eli datan p��lle jos siis yleens� piirret��n t�t� osaa kartasta
	int itsDrawOverMapMode; 
	// n�yt�n rivej� voi skrollata yl�s ja alas ja t�ss� on sen rivin indeksi, mik� n�ytet��n editorin 1. rivill� (alkaa 1:st�)
	int itsMapRowStartingIndex; 
	// t�m� annetaan CView: OnDraw:ssa ja t�t� k�ytet��n NFmiStationViewHandler:in Draw:ssa tekem��n n�yt�st� bitmap kopioita
	CDC *itsCopyCDC = nullptr;
	// nelj� tilaa: 0 = n�yt� aikakontrolliikkuna+teksti 1=vain aik.kont.ikkuna, 2=�l� n�yt� kumpaakaan ja 3= n�yt� vain teksti
	int itsShowTimeOnMapMode; 
	bool fShowTimeString;
	// ensimm�isen sarakkeen aika
	NFmiMetTime itsCurrentTime; 
	// karttan�ytt�ruudukon koko maksimissaan (pit�� olla 5 x 10(?) eli ainakin 5 rivi� on must)
	NFmiPoint itsViewGridSizeMax; 
	// valitun v�rin indeksi on tallessa t�ss�
	int itsStationPointColorIndex; 
	NFmiPoint itsStationPointSize;
	// kertoo, kuinka paljon aikaa siirret��n kun klikataan hiirell� aikakontrolli ikkunaa
	// kokonaisosa tunteja varten ja desimaalit minuutteja (0.5 = 30 minuuttia jne.)
	float itsTimeControlTimeStep; 
    CtrlViewUtils::MapViewMode itsMapViewDisplayMode;
	// kun n�ytt�ruudukkoa klikataan hiirell�, yksi ruuduista muuttuu aktiiviseksi, sen ruudun rivinumero talletetaan t�h�n
	// HUOM! t�m� on suhteellinen rivinumero eli pit�� ottaa huomioon monesko rivi on ensimm�isen� n�kyviss� karttan�ytt� ruudukossa
	int itsActiveViewRow;
	// onko t�m� desctop n�ytt� n�kyviss� vai ei
	bool fDescTopOn; 
	// Piiret��nk� karttan�yt�lle vain bitmap uudestaan (t�ll� saadaan pyyhitty� piirretty tooltippi pois)
	bool fMapViewBitmapDirty; 
	// t�ss� on karttan�yt�n pointteri
	CWnd *itsMapView = nullptr;
    CtrlViewUtils::GraphicalInfo itsGraphicalInfo;
	NFmiGridPointCache itsGridPointCache;
    NFmiAnimationData itsAnimationData;

    bool fLockToMainMapViewTime;
    bool fLockToMainMapViewRow;
	bool fShowTrajectorsOnMap;
	bool fShowSoundingMarkersOnMap;
	bool fShowCrossSectionMarkersOnMap;
	bool fShowSelectedPointsOnMap;
	bool fShowControlPointsOnMap;
	bool fShowObsComparisonOnMap;
	bool fShowWarningMarkersOnMap;

    // N�it� asetuksia k�ytet��n en�� vain viewMakroja teht�ess� tai luettaessa, n�iden todelliset 
    // asetukset tehd��n nyky��n Windows rekistereihin NFmiMapViewWinRegistry-luokan kautta.
    // VM nimen lopussa viittaa ViewMacroon.

	// n�ytet��nk� aktiivisen rivin kartoilla maski v�rein�?
	bool fShowMasksOnMapVM; 
	// 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemm�n
	int itsSpacingOutFactorVM; 
	// 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
	// HUOM! t�lle pit�� olla erillinen asetus funktio, koska t�t� k�ytet��n luokan sis�ll�, eli muutokset p�ivitet��n sek� t�nne ett� Win-rekistereihin
	unsigned int itsSelectedMapIndexVM; 
	// n�ytet��nk� se typer� asema piste vai ei?
    bool fShowStationPlotVM; 

    // karttan�ytt�ruudukon koko (esim. 3 rivi�, joissa 5 aikaa == NFmiPoint(5,3))
    // HUOM! t�lle pit�� olla asetus ja palautus funktiot, koska win-rekisteriss� talletetaan t�m� pointti stringin�
    // HUOM2 T�t� p�ivitet��n ajon aikana vain t�nne eik� Win-rekistereihin, 
    // joten t�m� on laitettava Win-rekisteriin StoreToMapViewWinRegistry-kutsussa!!!
    NFmiPoint itsViewGridSizeVM;

    // T�h�n tehd��n yksi maiden rajat kartalla bitmap, jota sitten 'l�tkit��n' oikeisiin kohtiin ruudukkon�yt�ss�.
    // Huom! pakko tehd� gdiplus bitmap, jotta saadaan mukaan l�pin�kyv� v�ri
    Gdiplus::Bitmap *itsLandBorderMapBitmap = nullptr;
	// T�ss� on erillisin� piirtolayereina olevien maiden rajaviivojen bitmap ja polyline datat.
	// Uudet erillism��ritellyt border-layerit voivat olla eri piirtos��d�ill� ja niit� voi olla jokaisella karttarivill� omansa.
	NFmiCountryBorderBitmapCache itsSeparateCountryBorderBitmapCache;
	TrueMapViewSizeInfo itsTrueMapViewSizeInfo;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Read(is); return is;}

