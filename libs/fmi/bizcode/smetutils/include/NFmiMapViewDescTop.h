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
		ViewMacroDipMapHelper(void){}
		~ViewMacroDipMapHelper(void){}

		// HUOM!! Tämä laittaa kommentteja mukaan!
		void Write(std::ostream& os) const;
		// HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
		// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
		// sekaan. Eli älä käytä suoraan tätä metodia, vaan Init(filename)-metodia!!!!
		void Read(std::istream& is);

		// dipmaphandler osio pitää ottaa tähän erikois käsittelyyn
		int itsUsedMapIndex;
		int itsUsedOverMapDibIndex;
		NFmiString itsZoomedAreaStr;
	};


	NFmiMapViewDescTop(void);
	NFmiMapViewDescTop(const std::string &theSettingsBaseName, NFmiMapConfigurationSystem *theMapConfigurationSystem, NFmiProjectionCurvatureInfo* theProjectionCurvatureInfo, const std::string &theControlPath);
	~NFmiMapViewDescTop(void);

	void Init(NFmiMapViewWinRegistry &theMapViewWinRegistry);
	void StoreMapViewDescTopToSettings(void);
    void InitForViewMacro(const NFmiMapViewDescTop &theOther, NFmiMapViewWinRegistry &theMapViewWinRegistry, bool getFromRegisty);
	std::vector<ViewMacroDipMapHelper> GetViewMacroDipMapHelperList(void);
	void SetViewMacroDipMapHelperList(const std::vector<ViewMacroDipMapHelper> &theData);
	void StoreToMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry);

	NFmiProjectionCurvatureInfo* ProjectionCurvatureInfo(void) {return itsProjectionCurvatureInfo;}
	void SelectedMapIndex(unsigned int newValue);
    unsigned int SelectedMapIndex(void) const {return itsSelectedMapIndexVM;}
	NFmiGdiPlusImageMapHandler* MapHandler(void);
    std::vector<NFmiGdiPlusImageMapHandler*>& GdiPlusImageMapHandlerList(void) {return itsGdiPlusImageMapHandlerList;}

	NFmiMapViewCache& MapViewCache(void) {return itsMapViewCache;}
	bool AreaViewDirty(void) {return fAreaViewDirty;}
	void AreaViewDirty(bool newValue) {fAreaViewDirty = newValue;}
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
	void AreaViewDirty(bool areaViewDirty, bool clearCache);
	void MapDirty(bool mapDirty, bool clearCache);
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
	void MapViewSizeInPixels(const NFmiPoint& newSize, bool fHideTimeControlView = false);
	int ToggleShowTimeOnMapMode(void);
	int ShowTimeOnMapMode(void) const {return itsShowTimeOnMapMode;}
	void ShowTimeOnMapMode(int newValue);
	CDC* CopyCDC(void){return itsCopyCDC;};
	void CopyCDC(CDC *theCDC){itsCopyCDC = theCDC;};
	int DrawOverMapMode(void) {return itsDrawOverMapMode;}
	void DrawOverMapMode(int newValue) {itsDrawOverMapMode = newValue;}
	CDC* MapBlitDC(void){return itsMapBlitDC;};
	void MapBlitDC(CDC* theDC){itsMapBlitDC=theDC;};
	bool ShowParamWindowView(void){return fShowParamWindowView;};
	void ShowParamWindowView(bool newState){fShowParamWindowView = newState;};
    boost::shared_ptr<Imagine::NFmiPath> LandBorderPath(void);
	const NFmiColor& LandBorderColor(void);
	void ToggleLandBorderColor(void);
	bool DrawLandBorders(void);
	const NFmiPoint& LandBorderPenSize(void){return itsLandBorderPenSize;}
	void LandBorderPenSize(const NFmiPoint &newValue){itsLandBorderPenSize = newValue;}
	void ToggleLandBorderPenSize(void);
	std::list<NFmiPolyline*>& DrawBorderPolyLineList(void);
	void DrawBorderPolyLineList(std::list<NFmiPolyline*> &newValue);
    const std::list<std::vector<NFmiPoint>>& DrawBorderPolyLineListGdiplus();
    void DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>> &newValue);
    void DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>> &&newValue);
    bool BorderDrawDirty(void){return fBorderDrawDirty;}
	void BorderDrawDirty(bool newValue){fBorderDrawDirty = newValue;}
	int LandBorderColorIndex(void) const {return itsLandBorderColorIndex;}
	void LandBorderColorIndex(int newValue) {itsLandBorderColorIndex = newValue;}
	bool DescTopOn(void) {return fDescTopOn;}
	void DescTopOn(bool newValue) {fDescTopOn = newValue;}
	bool MapViewBitmapDirty(void) const {return fMapViewBitmapDirty;}
	void MapViewBitmapDirty(bool newValue) {fMapViewBitmapDirty = newValue;}
	bool MapViewUpdated(void) const {return fMapViewUpdated;}
	void MapViewUpdated(bool newValue) {fMapViewUpdated = newValue;}
	const NFmiColor& StationPointColor(void) const;
	const NFmiPoint& StationPointSize(void) const;
	void ToggleStationPointColor(void);
	void ToggleStationPointSize(void);
	bool SetMapViewGrid(const NFmiPoint &newValue, NFmiMapViewWinRegistry *theMapViewWinRegistry);
	CWnd* MapView(void) {return itsMapView;}
	void MapView(CWnd *theMapView) {itsMapView = theMapView;}
	CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(void){return itsGraphicalInfo;}
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
    Gdiplus::Bitmap* LandBorderMapBitmap() { return itsLandBorderMapBitmap; }
    void SetLandBorderMapBitmap(Gdiplus::Bitmap *newBitmap);

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

	std::string itsSettingsBaseName; // tällä aloitus pohjalla luetaan conffi fileistä tarvittavat alustukset (esim. "MapViewDescTop::map1")
	NFmiMapConfigurationSystem *itsMapConfigurationSystem; // tämä on dokumentin omistuksessa
	NFmiProjectionCurvatureInfo* itsProjectionCurvatureInfo; // tämä on dokumentin omistuksessa

	std::string itsControlPath; // SmartMetin kontrollipolku (annetaan -p optiolla), tämän avulla luetaan kartta kuvia ja area-tietoja.
                                // OLI aiemmin editorin työhakemisto, mutta se ei toiminut oikein, varsinkin kun kyseessä 
                                // oli PV-projekti jossa dropbox-konffit ja sitä käytettiin VC++ debuggerista.
	std::vector<NFmiGdiPlusImageMapHandler*> itsGdiPlusImageMapHandlerList; // jokaisella desctopilla pitää olla oma 'map-serverinsä'
	NFmiMapViewCache itsMapViewCache; // luokka joka pitaa huolta karttanayton cachesta
	bool fAreaViewDirty;

	checkedVector<NFmiColor> itsLandBorderColors; // tähän talletetaan raja viivan piirron väri vaihtoehdot
	int itsLandBorderColorIndex; // valitunb värin indeksi on tallessa tässä, jos indeksi on negatiivinen, ei piirretä rajaviivoja
	NFmiPoint itsLandBorderPenSize;
	bool fBorderDrawDirty; // tarviiko tehdä piirtolistaa, vai voiko currentin piirtää sellaisenaan? Menee likaisksi, kun:
							// zoomataan, vaihdetaan karttapohjaa, muutetaan polyline väri/paksuus
	NFmiTimeDescriptor itsTimeControlViewTimes;

	double itsClientViewXperYRatio;	// tätä käytetään mm. zoomausdialogin rajoittimena(käytetyn karttanäytön x/y suhde, riippuu käytetystä ruudukosta ja ikkunan 'fyysisestä' koosta)
	NFmiRect itsRelativeMapRect; // minkä suhteellisen osan ottaa 'kartasto' clientnäytölle varatusta osasta
	NFmiPoint itsMapViewSizeInPixels; // päivitetään CView:in OnSize()-metodissa, käytetään datan harvennus laskuissa.
	bool fShowParamWindowView;
	NFmiPtrList<NFmiDrawParamList> *itsDrawParamListVector; // lista drawparam-listoja (näyttöruudukossa eri rivit laitetaan
															// omaan drawparamlist:iin, jotka sijoitetaan tähän listojen listaan)
	CDC* itsMapBlitDC;	// tähän talletetaan CView:ssa tehty DC, johon on talletettu
						// bitmap, mikä sitten blitataan jokaiseen näyttöruudun kohtaan
						// erikseen (toivottavasti vain väliaikainen viritys)
	int itsDrawOverMapMode; // jos 0, piirretään läpinäkyvä kartta ns. background karttaan, jos 1, piirretään se ns. foreground karttaan eli datan päälle
							// jos siis yleensä piirretään tätä osaa kartasta

	int itsMapRowStartingIndex; // näytön rivejä voi skrollata ylös ja alas ja tässä on sen rivin indeksi, mikä näytetään editorin 1. rivillä (alkaa 1:stä)
	CDC *itsCopyCDC; // tämä annetaan CView: OnDraw:ssa ja tätä käytetään NFmiStationViewHandler:in Draw:ssa tekemään näytöstä bitmap kopioita

	int itsShowTimeOnMapMode; // neljä tilaa: 0 = näytä aikakontrolliikkuna+teksti 1=vain aik.kont.ikkuna, 2=älä näytä kumpaakaan ja 3= näytä vain teksti
	bool fShowTimeString;
	NFmiMetTime itsCurrentTime; // ensimmäisen sarakkeen aika

	NFmiPoint itsViewGridSizeMax; // karttanäyttöruudukon koko maksimissaan (pitää olla 5 x 10(?) eli ainakin 5 riviä on must)
	int itsStationPointColorIndex; // valitun värin indeksi on tallessa tässä
	NFmiPoint itsStationPointSize;
	float itsTimeControlTimeStep; // kertoo, kuinka paljon aikaa siirretään kun klikataan hiirellä aikakontrolli ikkunaa
								  // kokonaisosa tunteja varten ja desimaalit minuutteja (0.5 = 30 minuuttia jne.)
    CtrlViewUtils::MapViewMode itsMapViewDisplayMode;
	int itsActiveViewRow;// kun näyttöruudukkoa klikataan hiirellä, yksi ruuduista muuttuu aktiiviseksi, sen ruudun rivinumero talletetaan tähän
						// HUOM! tämä on suhteellinen rivinumero eli pitää ottaa huomioon monesko rivi on ensimmäisenä näkyvissä karttanäyttö ruudukossa
	bool fDescTopOn; // onko tämä desctop näkyvissä vai ei
	bool fMapViewBitmapDirty; // Piiretäänkö karttanäytölle vain bitmap uudestaan (tällä saadaan pyyhittyä piirretty tooltippi pois)
	bool fMapViewUpdated;	  // tämä on vastapainoksi mapviewdirtylle. Jos hiirellä klikataan
								  // ruutua, ei piirretä karttaa, koska asema valinnat piirretään
								  // vastavärien avulla. Tämän avulla voin laittaa kartan piirron
								  // hoitumaan zeditmap2doc-luokan updateallviewsanddialogs:ista
								  // ja sen piiron esto tarvitsee laittaa vain kahteen paikkaa
	CWnd *itsMapView; // tässä on karttanäytön pointteri
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
	bool fShowMasksOnMapVM; // näytetäänkö aktiivisen rivin kartoilla maski väreinä?
	int itsSpacingOutFactorVM; // 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemmän
	unsigned int itsSelectedMapIndexVM; // 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
                                        // HUOM! tälle pitää olla erillinen asetus funktio, koska tätä käytetään luokan sisällä, eli muutokset päivitetään sekä tänne että Win-rekistereihin
    bool fShowStationPlotVM; // näytetäänkö se typerä asema piste vai ei?

    // karttanäyttöruudukon koko (esim. 3 riviä, joissa 5 aikaa == NFmiPoint(5,3))
    // HUOM! tälle pitää olla asetus ja palautus funktiot, koska win-rekisterissä talletetaan tämä pointti stringinä
    // HUOM2 Tätä päivitetään ajon aikana vain tänne eikä Win-rekistereihin, 
    // joten tämä on laitettava Win-rekisteriin StoreToMapViewWinRegistry-kutsussa!!!
    NFmiPoint itsViewGridSizeVM;

    // Tähän tehdään yksi maiden rajat kartalla bitmap, jota sitten 'lätkitään' oikeisiin kohtiin ruudukkonäytössä.
    // Huom! pakko tehdä gdiplus bitmap, jotta saadaan mukaan läpinäkyvä väri
    Gdiplus::Bitmap *itsLandBorderMapBitmap;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Read(is); return is;}

