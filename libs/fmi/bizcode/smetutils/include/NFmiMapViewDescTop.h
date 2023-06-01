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

		// HUOM!! T‰m‰ laittaa kommentteja mukaan!
		void Write(std::ostream& os) const;
		// HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
		// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
		// sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
		void Read(std::istream& is);

		// dipmaphandler osio pit‰‰ ottaa t‰h‰n erikois k‰sittelyyn
		int itsUsedMapIndex = 0;
		int itsUsedOverMapDibIndex = -1;
		int itsUsedCombinedModeMapIndex = 0;
		int itsUsedCombinedModeOverMapDibIndex = -1;
		NFmiString itsZoomedAreaStr;
		std::string itsBackgroundMacroReference;
		std::string itsOverlayMacroReference;
	};


	NFmiMapViewDescTop();
	NFmiMapViewDescTop(const std::string &theSettingsBaseName, NFmiMapConfigurationSystem *theMapConfigurationSystem, NFmiProjectionCurvatureInfo* theProjectionCurvatureInfo, const std::string &theControlPath, int theMapViewDescTopIndex);
	NFmiMapViewDescTop(const NFmiMapViewDescTop& other);
	NFmiMapViewDescTop& operator=(const NFmiMapViewDescTop& other);
	~NFmiMapViewDescTop();

	void Init(NFmiMapViewWinRegistry &theMapViewWinRegistry);
	void StoreMapViewDescTopToSettings(void);
    void InitForViewMacro(const NFmiMapViewDescTop &theOther, NFmiMapViewWinRegistry &theMapViewWinRegistry, bool getFromRegisty, bool disableWindowManipulations);
	std::vector<ViewMacroDipMapHelper> GetViewMacroDipMapHelperList(void);
	void SetViewMacroDipMapHelperList(const std::vector<ViewMacroDipMapHelper> &theData);
	void StoreToMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry);

	NFmiProjectionCurvatureInfo* ProjectionCurvatureInfo(void) {return itsProjectionCurvatureInfo;}
	void SelectedMapIndex(unsigned int newValue);
    unsigned int SelectedMapIndex(void) const {return itsSelectedMapIndexVM;}
	NFmiGdiPlusImageMapHandler* MapHandler(void) const;
	NFmiGdiPlusImageMapHandler* MapHandler(unsigned int mapAreaIndex) const;
	std::vector<NFmiGdiPlusImageMapHandler*>& GdiPlusImageMapHandlerList(void) {return itsGdiPlusImageMapHandlerList;}

	NFmiMapViewCache& MapViewCache(void) {return itsMapViewCache;}
    bool RedrawMapView(void) const;
    void SetRedrawMapView(bool newValue);
    void ClearRedrawMapView();
	NFmiPtrList<NFmiDrawParamList>* DrawParamListVector(void) {return itsDrawParamListVector;}
	bool ScrollViewRow(int theCount);
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
	long TimeControlTimeStepInMinutes() const;
    void MapViewDirty(bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool updateMapViewDrawingLayers);
    CtrlViewUtils::MapViewMode MapViewDisplayMode(void) const {return itsMapViewDisplayMode;}
	void MapViewDisplayMode(CtrlViewUtils::MapViewMode newValue);
	bool ShowTimeString(void){return fShowTimeString;}
	void ShowTimeString(bool newState){fShowTimeString = newState;}
	const NFmiRect& RelativeMapRect(void);
	void RelativeMapRect(const NFmiRect& theMapRect);
	void CalcClientViewXperYRatio(const NFmiPoint& theViewSize);
	double ClientViewXperYRatio(void){return itsClientViewXperYRatio;};
	void ClientViewXperYRatio(double theClientViewXperYRatio){itsClientViewXperYRatio = theClientViewXperYRatio;};
	const NFmiPoint& MapViewSizeInPixels(void){return itsMapViewSizeInPixels;};
	void MapViewSizeInPixels(const NFmiPoint& newSize, CDC* pDC, double theDrawObjectScaleFactor, bool fHideTimeControlView);
	void RecalculateMapViewSizeInPixels(double theDrawObjectScaleFactor);
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
	void ParamWindowViewPositionChange(bool forward);
	void TimeBoxPositionChange();
	FmiDirection ParamWindowViewPosition() const { return itsParamWindowViewPosition; }

    boost::shared_ptr<Imagine::NFmiPath> LandBorderPath(void);
	void ToggleLandBorderColor(void);

	// N‰m‰ border layer piirtoon liittyv‰t metodit jotka ottavat separateBorderLayerDrawOptions parametrin
	// toimivat seuraavalla periaatteella:
	// Jos k‰ytt‰j‰ haluaa tietoja yleisest‰ border-draw asetuksista, on em. parametri nullptr.
	// Jos se on nullptr:st‰ poikkeava, kyse on erillinen border-layer, jonka tiedot haetaan erikseen.
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
	int AbsoluteActiveViewRow(void) const {return itsAbsoluteActiveViewRow;}
	void AbsoluteActiveViewRow(int newValue) { itsAbsoluteActiveViewRow = newValue;}
    NFmiAnimationData& AnimationDataRef(void) {return itsAnimationData;}
	int CalcPrintingPageShiftInMinutes(void);
	void SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime);
	std::string GetCurrentGuiMapLayerText(bool backgroundMap);
	double SingleMapViewHeightInMilliMeters() const;
	bool PrintingModeOn() const { return fPrintingModeOn; }
	void PrintingModeOn(bool newState) { fPrintingModeOn = newState; }
	void UpdateOneMapViewSize();
	bool IsTimeControlViewVisibleTotal() const;
	FmiDirection TimeBoxLocation() const { return itsTimeBoxLocationVM; }
	void TimeBoxLocation(FmiDirection newLocation);
	float TimeBoxTextSizeFactor() const { return itsTimeBoxTextSizeFactorVM; }
	void TimeBoxTextSizeFactor(float newFactor);
	static float TimeBoxTextSizeFactorMinLimit();
	static float TimeBoxTextSizeFactorMaxLimit();
	const NFmiColor& TimeBoxFillColor() const { return itsTimeBoxFillColorVM; }
	void TimeBoxFillColor(const NFmiColor &newColor) { itsTimeBoxFillColorVM = newColor; }
	void SetTimeBoxFillColor(NFmiColor newColorNotAlpha);
	float GetTimeBoxFillColorAlpha() const { return itsTimeBoxFillColorVM.Alpha(); }
	void SetTimeBoxFillColorAlpha(float newAlpha);
	static const NFmiColor TimeBoxFillColorDefault;
	static const FmiDirection TimeBoxLocationDefault;
	static const float TimeBoxTextSizeFactorDefault;

	// HUOM!! T‰m‰ laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
	void Read(std::istream& is);
private:
	void Clear(void);
	void InitStationPointDrawingSystem(void);
	void InitMapViewDescTopFromSettings(void);
	void InitGdiPlusImageMapHandlerSystem(void);
	NFmiGdiPlusImageMapHandler* CreateGdiPlusImageMapHandler(std::shared_ptr<NFmiMapConfiguration> &theMapConfiguration);
	void SetGdiPlusImageMapHandlerSelectedMaps(NFmiGdiPlusImageMapHandler &theGdiPlusImageMapHandler, int mapHandlerIndex);
	void StoreHandlerSelectedMapsToSettings(NFmiGdiPlusImageMapHandler &theGdiPlusImageMapHandler, int mapHandlerIndex);
	void InitLandBorderDrawingSystem(void);
	void InitMapViewDrawParamListVector(void);
	void SetSelectedMapsFromSettings(void);
	void InitFromMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry);
    int CalcVisibleRowCount() const;
    int CalcMaxRowStartingIndex() const;
	void StoreHandlerSelectedMapsToSettings(void);
	const Gdiplus::Bitmap* GetSeparateBorderLayerCacheBitmap(const std::string& cacheKeyString);
	void InsertSeparateBorderLayerCacheBitmap(const std::string& cacheKeyString, std::unique_ptr<Gdiplus::Bitmap>&& cacheBitmap);
	void ClearBaseLandBorderMapBitmap();

	// t‰ll‰ aloitus pohjalla luetaan conffi fileist‰ tarvittavat alustukset (esim. "MapViewDescTop::map1")
	std::string itsSettingsBaseName; 
	// Joskus luokan pit‰‰ tiet‰‰ mik‰ descTop itse on
	int itsMapViewDescTopIndex; 
	// t‰m‰ on dokumentin omistuksessa
	NFmiMapConfigurationSystem *itsMapConfigurationSystem = nullptr; 
	// t‰m‰ on dokumentin omistuksessa
	NFmiProjectionCurvatureInfo* itsProjectionCurvatureInfo = nullptr;
	// SmartMetin kontrollipolku (annetaan -p optiolla), t‰m‰n avulla luetaan kartta kuvia ja area-tietoja.
	// OLI aiemmin editorin tyˆhakemisto, mutta se ei toiminut oikein, varsinkin kun kyseess‰ 
	// oli PV-projekti jossa dropbox-konffit ja sit‰ k‰ytettiin VC++ debuggerista.
	std::string itsControlPath; 
	// jokaisella desctopilla pit‰‰ olla oma 'map-serverins‰'
	std::vector<NFmiGdiPlusImageMapHandler*> itsGdiPlusImageMapHandlerList; 
	// luokka joka pitaa huolta karttanayton cachesta
	NFmiMapViewCache itsMapViewCache; 
	bool fRedrawMapView;
	// t‰h‰n talletetaan raja viivan piirron v‰ri vaihtoehdot
	std::vector<NFmiColor> itsLandBorderColors; 
	// valitun v‰rin indeksi on tallessa t‰ss‰, jos indeksi on negatiivinen, ei piirret‰ rajaviivoja
	int itsLandBorderColorIndex; 
	NFmiPoint itsLandBorderPenSize;
	NFmiTimeDescriptor itsTimeControlViewTimes;
	// t‰t‰ k‰ytet‰‰n mm. zoomausdialogin rajoittimena(k‰ytetyn karttan‰ytˆn x/y suhde, riippuu k‰ytetyst‰ ruudukosta ja ikkunan 'fyysisest‰' koosta)
	double itsClientViewXperYRatio;	
	// mink‰ suhteellisen osan ottaa 'kartasto' clientn‰ytˆlle varatusta osasta
	NFmiRect itsRelativeMapRect; 
	// p‰ivitet‰‰n CView:in OnSize()-metodissa, k‰ytet‰‰n datan harvennus laskuissa.
	NFmiPoint itsMapViewSizeInPixels; 
	// Parametrilaatikon paikkaa halutaan vaihdella. Se voi nyt saada seuraavat arvot:
	// 1. kTopLeft, 2. kTopRight, 3. kBottomRight, 4. kBottomLeft, 5. kNoDirection (piilossa)
	FmiDirection itsParamWindowViewPosition;
	// lista drawparam-listoja (n‰yttˆruudukossa eri rivit laitetaan
	// omaan drawparamlist:iin, jotka sijoitetaan t‰h‰n listojen listaan)
	NFmiPtrList<NFmiDrawParamList> *itsDrawParamListVector = nullptr;
	// t‰h‰n talletetaan CView:ssa tehty DC, johon on talletettu bitmap, mik‰ sitten 
	// blitataan jokaiseen n‰yttˆruudun kohtaan erikseen (toivottavasti vain v‰liaikainen viritys)
	CDC* itsMapBlitDC = nullptr;
	// jos 0, piirret‰‰n l‰pin‰kyv‰ kartta ns. background karttaan, jos 1, piirret‰‰n se ns. foreground karttaan 
	// eli datan p‰‰lle jos siis yleens‰ piirret‰‰n t‰t‰ osaa kartasta
	int itsDrawOverMapMode; 
	// n‰ytˆn rivej‰ voi skrollata ylˆs ja alas ja t‰ss‰ on sen rivin indeksi, mik‰ n‰ytet‰‰n editorin 1. rivill‰ (alkaa 1:st‰)
	int itsMapRowStartingIndex; 
	// t‰m‰ annetaan CView: OnDraw:ssa ja t‰t‰ k‰ytet‰‰n NFmiStationViewHandler:in Draw:ssa tekem‰‰n n‰ytˆst‰ bitmap kopioita
	CDC *itsCopyCDC = nullptr;
	// nelj‰ tilaa: 0 = n‰yt‰ aikakontrolliikkuna+teksti 1=vain aik.kont.ikkuna, 2=‰l‰ n‰yt‰ kumpaakaan ja 3= n‰yt‰ vain teksti
	int itsShowTimeOnMapMode; 
	bool fShowTimeString;
	// ensimm‰isen sarakkeen aika
	NFmiMetTime itsCurrentTime; 
	// karttan‰yttˆruudukon koko maksimissaan (pit‰‰ olla 5 x 10(?) eli ainakin 5 rivi‰ on must)
	NFmiPoint itsViewGridSizeMax; 
	// valitun v‰rin indeksi on tallessa t‰ss‰
	int itsStationPointColorIndex; 
	NFmiPoint itsStationPointSize;
	// kertoo, kuinka paljon aikaa siirret‰‰n kun klikataan hiirell‰ aikakontrolli ikkunaa
	// kokonaisosa tunteja varten ja desimaalit minuutteja (0.5 = 30 minuuttia jne.)
	float itsTimeControlTimeStep; 
    CtrlViewUtils::MapViewMode itsMapViewDisplayMode;
	// Kun n‰yttˆruudukkoa klikataan hiirell‰, yksi ruuduista muuttuu aktiiviseksi. 
	// Sen ruudun absoluuttinen rivinumero (1-50) talletetaan t‰h‰n (eli ei suhteellinen n‰kyviss‰ olevista riveist‰).
	int itsAbsoluteActiveViewRow = 1;
	// onko t‰m‰ desctop n‰yttˆ n‰kyviss‰ vai ei
	bool fDescTopOn; 
	// Piiret‰‰nkˆ karttan‰ytˆlle vain bitmap uudestaan (t‰ll‰ saadaan pyyhitty‰ piirretty tooltippi pois)
	bool fMapViewBitmapDirty; 
	// t‰ss‰ on karttan‰ytˆn pointteri
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

    // N‰it‰ asetuksia k‰ytet‰‰n en‰‰ vain viewMakroja teht‰ess‰ tai luettaessa, n‰iden todelliset 
    // asetukset tehd‰‰n nyky‰‰n Windows rekistereihin NFmiMapViewWinRegistry-luokan kautta.
    // VM nimen lopussa viittaa ViewMacroon.

	// n‰ytet‰‰nkˆ aktiivisen rivin kartoilla maski v‰rein‰?
	bool fShowMasksOnMapVM; 
	// 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemm‰n
	int itsSpacingOutFactorVM; 
	// 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
	// HUOM! t‰lle pit‰‰ olla erillinen asetus funktio, koska t‰t‰ k‰ytet‰‰n luokan sis‰ll‰, eli muutokset p‰ivitet‰‰n sek‰ t‰nne ett‰ Win-rekistereihin
	unsigned int itsSelectedMapIndexVM; 
	// n‰ytet‰‰nkˆ se typer‰ asema piste vai ei?
    bool fShowStationPlotVM; 

    // karttan‰yttˆruudukon koko (esim. 3 rivi‰, joissa 5 aikaa == NFmiPoint(5,3))
    // HUOM! t‰lle pit‰‰ olla asetus ja palautus funktiot, koska win-rekisteriss‰ talletetaan t‰m‰ pointti stringin‰
    // HUOM2 T‰t‰ p‰ivitet‰‰n ajon aikana vain t‰nne eik‰ Win-rekistereihin, 
    // joten t‰m‰ on laitettava Win-rekisteriin StoreToMapViewWinRegistry-kutsussa!!!
    NFmiPoint itsViewGridSizeVM;

    // T‰h‰n tehd‰‰n yksi maiden rajat kartalla bitmap, jota sitten 'l‰tkit‰‰n' oikeisiin kohtiin ruudukkon‰ytˆss‰.
    // Huom! pakko tehd‰ gdiplus bitmap, jotta saadaan mukaan l‰pin‰kyv‰ v‰ri
    Gdiplus::Bitmap *itsLandBorderMapBitmap = nullptr;
	// T‰ss‰ on erillisin‰ piirtolayereina olevien maiden rajaviivojen bitmap ja polyline datat.
	// Uudet erillism‰‰ritellyt border-layerit voivat olla eri piirtos‰‰dˆill‰ ja niit‰ voi olla jokaisella karttarivill‰ omansa.
	NFmiCountryBorderBitmapCache itsSeparateCountryBorderBitmapCache;
	TrueMapViewSizeInfo itsTrueMapViewSizeInfo;
	// Kun karttan‰yttˆ‰ printataan, laitetaan t‰m‰ p‰‰lle, jotta kartta-alue saadaan aina maksimiksi eli (0,0-1,1).
	// Kun tullaan pois printtauksesta, pit‰‰ t‰m‰ taas laittaa false:ksi!
	// T‰t‰ ei talleteta minnek‰‰n, eik‰ arvoa kopioida mitenk‰‰n.
	bool fPrintingModeOn = false;
	// Mihin kohtaan kartan aikalegenda piirret‰‰n (bottom-left/center/right tai top-left/center/right)
	FmiDirection itsTimeBoxLocationVM = TimeBoxLocationDefault;
	// Aikatekstill‰ on konffattu kaksi eri teksti‰ ja niille s‰‰det‰‰n koot erikseen konffeissa.
	// Tekstikokoja voidaan s‰‰t‰‰ t‰m‰n avulla pienemmiksi (arvo < 1) ja isommiksi (arvo > 1).
	// Rajat t‰lle s‰‰dˆlle on laitettu 0.5 - 2.5.
	float itsTimeBoxTextSizeFactorVM = TimeBoxTextSizeFactorDefault;
	static const float itsTimeBoxTextSizeFactorMinLimit;
	static const float itsTimeBoxTextSizeFactorMaxLimit;
	// Aikatekstin laatikon pohjav‰ri ja sen alpha on tallessa t‰ss‰
	NFmiColor itsTimeBoxFillColorVM = TimeBoxFillColorDefault;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Read(is); return is;}

