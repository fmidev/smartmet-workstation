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

		// HUOM!! T�m� laittaa kommentteja mukaan!
		void Write(std::ostream& os) const;
		// HUOM!! ennen kuin t�m� luokka luetaan sis��n tiedostosta, poista kommentit
		// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
		// sekaan. Eli �l� k�yt� suoraan t�t� metodia, vaan Init(filename)-metodia!!!!
		void Read(std::istream& is);

		// dipmaphandler osio pit�� ottaa t�h�n erikois k�sittelyyn
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

	std::string itsSettingsBaseName; // t�ll� aloitus pohjalla luetaan conffi fileist� tarvittavat alustukset (esim. "MapViewDescTop::map1")
	NFmiMapConfigurationSystem *itsMapConfigurationSystem; // t�m� on dokumentin omistuksessa
	NFmiProjectionCurvatureInfo* itsProjectionCurvatureInfo; // t�m� on dokumentin omistuksessa

	std::string itsControlPath; // SmartMetin kontrollipolku (annetaan -p optiolla), t�m�n avulla luetaan kartta kuvia ja area-tietoja.
                                // OLI aiemmin editorin ty�hakemisto, mutta se ei toiminut oikein, varsinkin kun kyseess� 
                                // oli PV-projekti jossa dropbox-konffit ja sit� k�ytettiin VC++ debuggerista.
	std::vector<NFmiGdiPlusImageMapHandler*> itsGdiPlusImageMapHandlerList; // jokaisella desctopilla pit�� olla oma 'map-serverins�'
	NFmiMapViewCache itsMapViewCache; // luokka joka pitaa huolta karttanayton cachesta
	bool fAreaViewDirty;

	checkedVector<NFmiColor> itsLandBorderColors; // t�h�n talletetaan raja viivan piirron v�ri vaihtoehdot
	int itsLandBorderColorIndex; // valitunb v�rin indeksi on tallessa t�ss�, jos indeksi on negatiivinen, ei piirret� rajaviivoja
	NFmiPoint itsLandBorderPenSize;
	bool fBorderDrawDirty; // tarviiko tehd� piirtolistaa, vai voiko currentin piirt�� sellaisenaan? Menee likaisksi, kun:
							// zoomataan, vaihdetaan karttapohjaa, muutetaan polyline v�ri/paksuus
	NFmiTimeDescriptor itsTimeControlViewTimes;

	double itsClientViewXperYRatio;	// t�t� k�ytet��n mm. zoomausdialogin rajoittimena(k�ytetyn karttan�yt�n x/y suhde, riippuu k�ytetyst� ruudukosta ja ikkunan 'fyysisest�' koosta)
	NFmiRect itsRelativeMapRect; // mink� suhteellisen osan ottaa 'kartasto' clientn�yt�lle varatusta osasta
	NFmiPoint itsMapViewSizeInPixels; // p�ivitet��n CView:in OnSize()-metodissa, k�ytet��n datan harvennus laskuissa.
	bool fShowParamWindowView;
	NFmiPtrList<NFmiDrawParamList> *itsDrawParamListVector; // lista drawparam-listoja (n�ytt�ruudukossa eri rivit laitetaan
															// omaan drawparamlist:iin, jotka sijoitetaan t�h�n listojen listaan)
	CDC* itsMapBlitDC;	// t�h�n talletetaan CView:ssa tehty DC, johon on talletettu
						// bitmap, mik� sitten blitataan jokaiseen n�ytt�ruudun kohtaan
						// erikseen (toivottavasti vain v�liaikainen viritys)
	int itsDrawOverMapMode; // jos 0, piirret��n l�pin�kyv� kartta ns. background karttaan, jos 1, piirret��n se ns. foreground karttaan eli datan p��lle
							// jos siis yleens� piirret��n t�t� osaa kartasta

	int itsMapRowStartingIndex; // n�yt�n rivej� voi skrollata yl�s ja alas ja t�ss� on sen rivin indeksi, mik� n�ytet��n editorin 1. rivill� (alkaa 1:st�)
	CDC *itsCopyCDC; // t�m� annetaan CView: OnDraw:ssa ja t�t� k�ytet��n NFmiStationViewHandler:in Draw:ssa tekem��n n�yt�st� bitmap kopioita

	int itsShowTimeOnMapMode; // nelj� tilaa: 0 = n�yt� aikakontrolliikkuna+teksti 1=vain aik.kont.ikkuna, 2=�l� n�yt� kumpaakaan ja 3= n�yt� vain teksti
	bool fShowTimeString;
	NFmiMetTime itsCurrentTime; // ensimm�isen sarakkeen aika

	NFmiPoint itsViewGridSizeMax; // karttan�ytt�ruudukon koko maksimissaan (pit�� olla 5 x 10(?) eli ainakin 5 rivi� on must)
	int itsStationPointColorIndex; // valitun v�rin indeksi on tallessa t�ss�
	NFmiPoint itsStationPointSize;
	float itsTimeControlTimeStep; // kertoo, kuinka paljon aikaa siirret��n kun klikataan hiirell� aikakontrolli ikkunaa
								  // kokonaisosa tunteja varten ja desimaalit minuutteja (0.5 = 30 minuuttia jne.)
    CtrlViewUtils::MapViewMode itsMapViewDisplayMode;
	int itsActiveViewRow;// kun n�ytt�ruudukkoa klikataan hiirell�, yksi ruuduista muuttuu aktiiviseksi, sen ruudun rivinumero talletetaan t�h�n
						// HUOM! t�m� on suhteellinen rivinumero eli pit�� ottaa huomioon monesko rivi on ensimm�isen� n�kyviss� karttan�ytt� ruudukossa
	bool fDescTopOn; // onko t�m� desctop n�kyviss� vai ei
	bool fMapViewBitmapDirty; // Piiret��nk� karttan�yt�lle vain bitmap uudestaan (t�ll� saadaan pyyhitty� piirretty tooltippi pois)
	bool fMapViewUpdated;	  // t�m� on vastapainoksi mapviewdirtylle. Jos hiirell� klikataan
								  // ruutua, ei piirret� karttaa, koska asema valinnat piirret��n
								  // vastav�rien avulla. T�m�n avulla voin laittaa kartan piirron
								  // hoitumaan zeditmap2doc-luokan updateallviewsanddialogs:ista
								  // ja sen piiron esto tarvitsee laittaa vain kahteen paikkaa
	CWnd *itsMapView; // t�ss� on karttan�yt�n pointteri
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
	bool fShowMasksOnMapVM; // n�ytet��nk� aktiivisen rivin kartoilla maski v�rein�?
	int itsSpacingOutFactorVM; // 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemm�n
	unsigned int itsSelectedMapIndexVM; // 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
                                        // HUOM! t�lle pit�� olla erillinen asetus funktio, koska t�t� k�ytet��n luokan sis�ll�, eli muutokset p�ivitet��n sek� t�nne ett� Win-rekistereihin
    bool fShowStationPlotVM; // n�ytet��nk� se typer� asema piste vai ei?

    // karttan�ytt�ruudukon koko (esim. 3 rivi�, joissa 5 aikaa == NFmiPoint(5,3))
    // HUOM! t�lle pit�� olla asetus ja palautus funktiot, koska win-rekisteriss� talletetaan t�m� pointti stringin�
    // HUOM2 T�t� p�ivitet��n ajon aikana vain t�nne eik� Win-rekistereihin, 
    // joten t�m� on laitettava Win-rekisteriin StoreToMapViewWinRegistry-kutsussa!!!
    NFmiPoint itsViewGridSizeVM;

    // T�h�n tehd��n yksi maiden rajat kartalla bitmap, jota sitten 'l�tkit��n' oikeisiin kohtiin ruudukkon�yt�ss�.
    // Huom! pakko tehd� gdiplus bitmap, jotta saadaan mukaan l�pin�kyv� v�ri
    Gdiplus::Bitmap *itsLandBorderMapBitmap;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Read(is); return is;}

