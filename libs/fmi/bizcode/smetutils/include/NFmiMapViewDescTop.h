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

		// HUOM!! Tämä laittaa kommentteja mukaan!
		void Write(std::ostream& os) const;
		// HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
		// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
		// sekaan. Eli älä käytä suoraan tätä metodia, vaan Init(filename)-metodia!!!!
		void Read(std::istream& is);

		// dipmaphandler osio pitää ottaa tähän erikois käsittelyyn
		int itsUsedMapIndex = 0;
		int itsUsedOverMapDibIndex = -1;
		int itsUsedCombinedModeMapIndex = 0;
		int itsUsedCombinedModeOverMapDibIndex = -1;
		// Tähän luetaan vanhoista näyttömakroista pre-wgs84 newbase:n NFmiArea::AreaStr metodin arvo (esim. stereographic,20,90,60:6,51.3,49,70.2).
		// Uudessa newbasessa tähän käytetään uutta NFmiArea::AreaFactoryStr metodia.
		NFmiString itsAreaFactoryStr;
		// Tähän talletetaan uusi Proj-kirjaston pohjainen projektio+area string, joka tehdään NFmiArea::AreaFactoryProjStr metodilla.
		// Tämä arvo talletetaan NFmiExtraDataStorage:n listoihin. Vanhat SmartMetit ignooraavat siis tämän, mutta jos tähän löytyy arvo,
		// sitä käytetään uudessa uuden NFmiArea olion luontiin, mutta jos tämä tyhjä, käytetään itsAreaFactoryStr:ia sitten.
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

	// Nämä border layer piirtoon liittyvät metodit jotka ottavat separateBorderLayerDrawOptions parametrin
	// toimivat seuraavalla periaatteella:
	// Jos käyttäjä haluaa tietoja yleisestä border-draw asetuksista, on em. parametri nullptr.
	// Jos se on nullptr:stä poikkeava, kyse on erillinen border-layer, jonka tiedot haetaan erikseen.
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

	// HUOM!! Tämä laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli älä käytä suoraan tätä metodia, vaan Init(filename)-metodia!!!!
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

	// tällä aloitus pohjalla luetaan conffi fileistä tarvittavat alustukset (esim. "MapViewDescTop::map1")
	std::string itsSettingsBaseName; 
	// Joskus luokan pitää tietää mikä descTop itse on
	int itsMapViewDescTopIndex; 
	// tämä on dokumentin omistuksessa
	NFmiMapConfigurationSystem *itsMapConfigurationSystem = nullptr; 
	// tämä on dokumentin omistuksessa
	NFmiProjectionCurvatureInfo* itsProjectionCurvatureInfo = nullptr;
	// SmartMetin kontrollipolku (annetaan -p optiolla), tämän avulla luetaan kartta kuvia ja area-tietoja.
	// OLI aiemmin editorin työhakemisto, mutta se ei toiminut oikein, varsinkin kun kyseessä 
	// oli PV-projekti jossa dropbox-konffit ja sitä käytettiin VC++ debuggerista.
	std::string itsControlPath; 
	// jokaisella desctopilla pitää olla oma 'map-serverinsä'
	std::vector<NFmiGdiPlusImageMapHandler*> itsGdiPlusImageMapHandlerList; 
	// luokka joka pitaa huolta karttanayton cachesta
	NFmiMapViewCache itsMapViewCache; 
	bool fRedrawMapView;
	// tähän talletetaan raja viivan piirron väri vaihtoehdot
	std::vector<NFmiColor> itsLandBorderColors; 
	// valitun värin indeksi on tallessa tässä, jos indeksi on negatiivinen, ei piirretä rajaviivoja
	int itsLandBorderColorIndex; 
	NFmiPoint itsLandBorderPenSize;
	NFmiTimeDescriptor itsTimeControlViewTimes;
	// tätä käytetään mm. zoomausdialogin rajoittimena(käytetyn karttanäytön x/y suhde, riippuu käytetystä ruudukosta ja ikkunan 'fyysisestä' koosta)
	double itsClientViewXperYRatio;	
	// minkä suhteellisen osan ottaa 'kartasto' clientnäytölle varatusta osasta
	NFmiRect itsRelativeMapRect; 
	// päivitetään CView:in OnSize()-metodissa, käytetään datan harvennus laskuissa.
	NFmiPoint itsMapViewSizeInPixels; 
	bool fShowParamWindowView;
	// lista drawparam-listoja (näyttöruudukossa eri rivit laitetaan
	// omaan drawparamlist:iin, jotka sijoitetaan tähän listojen listaan)
	NFmiPtrList<NFmiDrawParamList> *itsDrawParamListVector = nullptr;
	// tähän talletetaan CView:ssa tehty DC, johon on talletettu bitmap, mikä sitten 
	// blitataan jokaiseen näyttöruudun kohtaan erikseen (toivottavasti vain väliaikainen viritys)
	CDC* itsMapBlitDC = nullptr;
	// jos 0, piirretään läpinäkyvä kartta ns. background karttaan, jos 1, piirretään se ns. foreground karttaan 
	// eli datan päälle jos siis yleensä piirretään tätä osaa kartasta
	int itsDrawOverMapMode; 
	// näytön rivejä voi skrollata ylös ja alas ja tässä on sen rivin indeksi, mikä näytetään editorin 1. rivillä (alkaa 1:stä)
	int itsMapRowStartingIndex; 
	// tämä annetaan CView: OnDraw:ssa ja tätä käytetään NFmiStationViewHandler:in Draw:ssa tekemään näytöstä bitmap kopioita
	CDC *itsCopyCDC = nullptr;
	// neljä tilaa: 0 = näytä aikakontrolliikkuna+teksti 1=vain aik.kont.ikkuna, 2=älä näytä kumpaakaan ja 3= näytä vain teksti
	int itsShowTimeOnMapMode; 
	bool fShowTimeString;
	// ensimmäisen sarakkeen aika
	NFmiMetTime itsCurrentTime; 
	// karttanäyttöruudukon koko maksimissaan (pitää olla 5 x 10(?) eli ainakin 5 riviä on must)
	NFmiPoint itsViewGridSizeMax; 
	// valitun värin indeksi on tallessa tässä
	int itsStationPointColorIndex; 
	NFmiPoint itsStationPointSize;
	// kertoo, kuinka paljon aikaa siirretään kun klikataan hiirellä aikakontrolli ikkunaa
	// kokonaisosa tunteja varten ja desimaalit minuutteja (0.5 = 30 minuuttia jne.)
	float itsTimeControlTimeStep; 
    CtrlViewUtils::MapViewMode itsMapViewDisplayMode;
	// kun näyttöruudukkoa klikataan hiirellä, yksi ruuduista muuttuu aktiiviseksi, sen ruudun rivinumero talletetaan tähän
	// HUOM! tämä on suhteellinen rivinumero eli pitää ottaa huomioon monesko rivi on ensimmäisenä näkyvissä karttanäyttö ruudukossa
	int itsActiveViewRow;
	// onko tämä desctop näyttö näkyvissä vai ei
	bool fDescTopOn; 
	// Piiretäänkö karttanäytölle vain bitmap uudestaan (tällä saadaan pyyhittyä piirretty tooltippi pois)
	bool fMapViewBitmapDirty; 
	// tässä on karttanäytön pointteri
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

    // Näitä asetuksia käytetään enää vain viewMakroja tehtäessä tai luettaessa, näiden todelliset 
    // asetukset tehdään nykyään Windows rekistereihin NFmiMapViewWinRegistry-luokan kautta.
    // VM nimen lopussa viittaa ViewMacroon.

	// näytetäänkö aktiivisen rivin kartoilla maski väreinä?
	bool fShowMasksOnMapVM; 
	// 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemmän
	int itsSpacingOutFactorVM; 
	// 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
	// HUOM! tälle pitää olla erillinen asetus funktio, koska tätä käytetään luokan sisällä, eli muutokset päivitetään sekä tänne että Win-rekistereihin
	unsigned int itsSelectedMapIndexVM; 
	// näytetäänkö se typerä asema piste vai ei?
    bool fShowStationPlotVM; 

    // karttanäyttöruudukon koko (esim. 3 riviä, joissa 5 aikaa == NFmiPoint(5,3))
    // HUOM! tälle pitää olla asetus ja palautus funktiot, koska win-rekisterissä talletetaan tämä pointti stringinä
    // HUOM2 Tätä päivitetään ajon aikana vain tänne eikä Win-rekistereihin, 
    // joten tämä on laitettava Win-rekisteriin StoreToMapViewWinRegistry-kutsussa!!!
    NFmiPoint itsViewGridSizeVM;

    // Tähän tehdään yksi maiden rajat kartalla bitmap, jota sitten 'lätkitään' oikeisiin kohtiin ruudukkonäytössä.
    // Huom! pakko tehdä gdiplus bitmap, jotta saadaan mukaan läpinäkyvä väri
    Gdiplus::Bitmap *itsLandBorderMapBitmap = nullptr;
	// Tässä on erillisinä piirtolayereina olevien maiden rajaviivojen bitmap ja polyline datat.
	// Uudet erillismääritellyt border-layerit voivat olla eri piirtosäädöillä ja niitä voi olla jokaisella karttarivillä omansa.
	NFmiCountryBorderBitmapCache itsSeparateCountryBorderBitmapCache;
	TrueMapViewSizeInfo itsTrueMapViewSizeInfo;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Read(is); return is;}

