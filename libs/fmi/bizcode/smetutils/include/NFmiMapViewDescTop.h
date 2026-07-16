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

		// NOTE!! This includes comments!
		void Write(std::ostream& os) const;
		// NOTE!! before this class is read in from a file, remove the comments
		// with the NFmiCommentStripper class, because comments are inserted
		// when writing. So do not use this method directly, but the Init(filename) method!!!!
		void Read(std::istream& is);

		// The dipmaphandler section must be given special handling here
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

	// These border layer drawing methods that take the separateBorderLayerDrawOptions parameter
	// work on the following principle:
	// If the user wants information about the general border-draw settings, that parameter is nullptr.
	// If it differs from nullptr, it concerns a separate border-layer whose information is fetched separately.
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
	static const std::vector<float>& TimeBoxTextSizeAllowedFactors() { return itsTimeBoxTextSizeAllowedFactors; }
	const NFmiColor& TimeBoxFillColor() const { return itsTimeBoxFillColorVM; }
	void TimeBoxFillColor(const NFmiColor &newColor) { itsTimeBoxFillColorVM = newColor; }
	void SetTimeBoxFillColor(NFmiColor newColorNotAlpha);
	float GetTimeBoxFillColorAlpha() const { return itsTimeBoxFillColorVM.Alpha(); }
	void SetTimeBoxFillColorAlpha(float newAlpha);
	static const NFmiColor TimeBoxFillColorDefault;
	static const FmiDirection TimeBoxLocationDefault;
	static const float TimeBoxTextSizeFactorDefault;

	// NOTE!! This includes comments!
	void Write(std::ostream& os) const;
	// NOTE!! before this class is read in from a file, remove the comments
	// with the NFmiCommentStripper class, because comments are inserted
	// when writing. So do not use this method directly, but the Init(filename) method!!!!
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

	// With this base name the required initializations are read from the config files (e.g. "MapViewDescTop::map1")
	std::string itsSettingsBaseName;
	// Sometimes the class needs to know which descTop it itself is
	int itsMapViewDescTopIndex;
	// This is owned by the document
	NFmiMapConfigurationSystem *itsMapConfigurationSystem = nullptr;
	// This is owned by the document
	NFmiProjectionCurvatureInfo* itsProjectionCurvatureInfo = nullptr;
	// SmartMet's control path (given with the -p option), used to read map images and area information.
	// It USED to be the editor's working directory, but that did not work correctly, especially when
	// dealing with a PV project that had dropbox configs and was run from the VC++ debugger.
	std::string itsControlPath;
	// Each desctop must have its own 'map-server'
	std::vector<NFmiGdiPlusImageMapHandler*> itsGdiPlusImageMapHandlerList;
	// Class that takes care of the map view cache
	NFmiMapViewCache itsMapViewCache;
	bool fRedrawMapView;
	// The color options for drawing the border line are stored here
	std::vector<NFmiColor> itsLandBorderColors;
	// The index of the selected color is stored here; if the index is negative, border lines are not drawn
	int itsLandBorderColorIndex;
	NFmiPoint itsLandBorderPenSize;
	NFmiTimeDescriptor itsTimeControlViewTimes;
	// This is used e.g. as a limiter for the zoom dialog (the x/y ratio of the used map view, depends on the used grid and the 'physical' size of the window)
	double itsClientViewXperYRatio;
	// What relative portion the 'atlas' takes of the portion reserved for the client view
	NFmiRect itsRelativeMapRect;
	// Updated in CView's OnSize() method, used in the data thinning calculations.
	NFmiPoint itsMapViewSizeInPixels;
	// The position of the parameter box can be varied. It can now take the following values:
	// 1. kTopLeft, 2. kTopRight, 3. kBottomRight, 4. kBottomLeft, 5. kNoDirection (hidden)
	FmiDirection itsParamWindowViewPosition;
	// List of drawparam lists (in the view grid the different rows are put
	// into their own drawparamlist, which are placed in this list of lists)
	NFmiPtrList<NFmiDrawParamList> *itsDrawParamListVector = nullptr;
	// Here is stored the DC made in CView, into which a bitmap has been stored, which is then
	// blitted separately to each position of the view grid (hopefully only a temporary hack)
	CDC* itsMapBlitDC = nullptr;
	// If 0, the transparent map is drawn into the so-called background map; if 1, it is drawn into the so-called foreground map
	// i.e. on top of the data if this part of the map is generally drawn
	int itsDrawOverMapMode;
	// The view rows can be scrolled up and down, and here is the index of the row that is shown on the editor's 1st row (starts from 1)
	int itsMapRowStartingIndex;
	// This is given in CView OnDraw and is used in NFmiStationViewHandler's Draw to make bitmap copies of the view
	CDC *itsCopyCDC = nullptr;
	// Four states: 0 = show time control window+text, 1 = only time control window, 2 = show neither, and 3 = show only text
	int itsShowTimeOnMapMode;
	bool fShowTimeString;
	// The time of the first column
	NFmiMetTime itsCurrentTime;
	// The maximum size of the map view grid (must be 5 x 10(?), i.e. at least 5 rows is a must)
	NFmiPoint itsViewGridSizeMax;
	// The index of the selected color is stored here
	int itsStationPointColorIndex;
	NFmiPoint itsStationPointSize;
	// Tells how much time is shifted when the time control window is clicked with the mouse
	// the integer part for hours and the decimals for minutes (0.5 = 30 minutes etc.)
	float itsTimeControlTimeStep;
    CtrlViewUtils::MapViewMode itsMapViewDisplayMode;
	// When the view grid is clicked with the mouse, one of the cells becomes active.
	// That cell's absolute row number (1-50) is stored here (i.e. not relative to the visible rows).
	int itsAbsoluteActiveViewRow = 1;
	// Whether this desctop view is visible or not
	bool fDescTopOn;
	// Whether only the bitmap is redrawn to the map view (this is used to wipe away a drawn tooltip)
	bool fMapViewBitmapDirty;
	// Here is the pointer to the map view
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

    // These settings are only used anymore when creating or reading viewMacros; their actual
    // settings are nowadays made into the Windows registry via the NFmiMapViewWinRegistry class.
    // The VM at the end of the name refers to ViewMacro.

	// Whether masks are shown as colors on the active row's maps?
	bool fShowMasksOnMapVM;
	// 0=no thinning, 1=thin somewhat, 2=thin more
	int itsSpacingOutFactorVM;
	// 0 = map1 (e.g. finland), 1 = map2 (e.g. scandinavia), 2 = map3 (e.g. europe), 3 = map4 (e.g. world)
	// NOTE! this needs a separate setter function, because it is used inside the class, i.e. changes are updated both here and to the Win registry
	unsigned int itsSelectedMapIndexVM;
	// Whether that silly station point is shown or not?
    bool fShowStationPlotVM;

    // The size of the map view grid (e.g. 3 rows with 5 times == NFmiPoint(5,3))
    // NOTE! this needs setter and getter functions, because in the win registry this point is stored as a string
    // NOTE2 During runtime this is updated only here and not to the Win registry,
    // so it must be put into the Win registry in the StoreToMapViewWinRegistry call!!!
    NFmiPoint itsViewGridSizeVM;

    // Here one country-borders-on-map bitmap is made, which is then 'stamped' onto the correct positions in the grid view.
    // Note! a gdiplus bitmap must be used, in order to include a transparent color
    Gdiplus::Bitmap *itsLandBorderMapBitmap = nullptr;
	// Here are the bitmap and polyline data of the country border lines that are separate draw layers.
	// New separately defined border-layers can have different draw settings, and each map row can have its own.
	NFmiCountryBorderBitmapCache itsSeparateCountryBorderBitmapCache;
	TrueMapViewSizeInfo itsTrueMapViewSizeInfo;
	// When the map view is printed, this is turned on so that the map area is always maximized, i.e. (0,0-1,1).
	// When coming out of printing, this must be set back to false!
	// This is not stored anywhere, and the value is not copied in any way.
	bool fPrintingModeOn = false;
	// Where the map's time legend is drawn (bottom-left/center/right or top-left/center/right)
	FmiDirection itsTimeBoxLocationVM = TimeBoxLocationDefault;
	// The time text has two different texts configured, and their sizes are adjusted separately in the configs.
	// The text sizes can be adjusted with this to be smaller (value < 1) and larger (value > 1).
	// The limits for this adjustment have been set to 0.5 - 2.5.
	float itsTimeBoxTextSizeFactorVM = TimeBoxTextSizeFactorDefault;
	static const std::vector<float> itsTimeBoxTextSizeAllowedFactors;
	// The background color of the time text box and its alpha are stored here
	NFmiColor itsTimeBoxFillColorVM = TimeBoxFillColorDefault;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMapViewDescTop::ViewMacroDipMapHelper& item){item.Read(is); return is;}

