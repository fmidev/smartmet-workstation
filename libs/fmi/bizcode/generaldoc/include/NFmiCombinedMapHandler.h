#pragma once

#include "CombinedMapHandlerInterface.h"
#include "catlog/catlog.h"
#include "NFmiDataMatrix.h"
#include "NFmiPtrList.h"
#include "NFmiCombinedMapModeState.h"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <memory>
#include <map>
#include <list>

class NFmiMapViewDescTop;
class WmsSupportInterface;
class NFmiMapConfigurationSystem;
class NFmiProjectionCurvatureInfo;
class NFmiFastQueryInfo;
class NFmiTimeBag;
class NFmiDrawParam;
class NFmiArea;
struct MacroParamDataInfo;
class NFmiDrawParamList;
class NFmiFastDrawParamList;
class NFmiMenuItem;
class OGRGeometryCollection;
class OGRGeometry;

// All kinds of background map and overlay map stuff handling is here.
// This has local bitmaps and interface to wms-server clients as well.
class NFmiCombinedMapHandler : public CombinedMapHandlerInterface
{
    // T�h�n talletetaan yhden karttan�yt�n kartta-alueiden 1-4 combined-mode tilat
    using MapViewCombinedMapModeState = std::map<unsigned int, NFmiCombinedMapModeState>;
    // T�h�n talletetaan kaikkien karttan�ytt�jen (1-3) kaikkien kartta-alueiden combined-mode tilat
    using TotalMapViewCombinedMapModeState = std::map<unsigned int, MapViewCombinedMapModeState>;

    std::string absoluteControlPath_;
    // T�h�n tulee kaikkien eri mapview (1-3) desctoppien tiedot
    MapViewDescTopVector mapViewDescTops_;
    std::unique_ptr<NFmiMapConfigurationSystem> mapConfigurationSystem_;
    // T�m� tiet�� miten piirret��n lat-lon viivat eri projektioihin
    std::unique_ptr<NFmiProjectionCurvatureInfo> projectionCurvatureInfo_; 
    // T�t� lippua k�ytet��n aikasarja ikkunan kaksoispuskuroinnisssa
    bool timeSerialViewDirty_ = true;
    // T�h�n talletetaan sen karttan�yt�n indeksi, mink� oletetaan olevan aktiivinen, oletus on 0
    unsigned int activeMapDescTopIndex_ = 0; 
    std::unique_ptr<NFmiFastDrawParamList> modifiedPropertiesDrawParamList_;
    // Poikkileikkausn�ytt�-ruudukon drawparamit
    std::unique_ptr<NFmiPtrList<NFmiDrawParamList>> crossSectionDrawParamListVector_; 
    // Knows all the parameters that has to be drawn on timeserial-view
    std::unique_ptr<NFmiDrawParamList> timeSerialViewDrawParamList_;
    // All the side-parameters on each timeserial-view row.
    // Each row's main-parameter can have 0-n number of side-parameters, that will be shown on view with different colors.
    SideParametersContainer timeSerialViewSideParameters_;
    // Hiiren oikean klikkauksen rivinumero talletetaan t�h�n
    unsigned long timeSerialViewIndex_ = 0; 

    // **************** Uudet country border datat, joita ty�tet��n Gdal:in avulla **********************
    std::string countryBorderShapeFile_;
    // T�ss� on 'raaka' wgs84 datumissa oleva latlon koordinaatistossa oleva rajaviiva shape data
    std::unique_ptr<OGRGeometryCollection> globalBaseCountryBorderGeometry_;
    // T�h�n lasketaan jokaiseen eri karttapohjaan (kartat 1-4) leikatut rajaviivat. N�ill� voidaan optimoida zoomaukseen k�ytettyj� rajaviiva laskuja.
    std::vector<std::shared_ptr<OGRGeometry>> clippedCountryBorderGeometrys_;

    // ******** Piirto-ominaisuuksien copy-paste toimintoihin liittyv�t datat ****************
    // T�t� k�ytet��n yhden parametrin piirto-ominaisuuksien copy/paste komennon talletuspaikkana
    std::unique_ptr<NFmiDrawParam> copyPasteDrawParam_;
    // Onko yht��n piirto-ominaisuus oliota kopioitu viel� t�h�n?
    bool copyPasteDrawParamAvailableYet_ = false; 
    // T�t� k�ytet��n kopioimaan ja pasteamaan yhden karttan�ytt�rivin parametrit asetuksineen
    std::unique_ptr<NFmiDrawParamList> copyPasteDrawParamList_;
    // Onko yht��n copy komentoa tehty viel�, vaikuttaa siihen ilmestyyk� pop-up valikkoon paste
    bool copyPasteDrawParamListUsedYet_ = false;
    // T�t� k�ytet��n copy-paste toimintoon poikkileikkausn�ytt�rivien drawParameita
    std::unique_ptr<NFmiDrawParamList> copyPasteCrossSectionDrawParamList_;
    // Onko yht��n copy komentoa tehty viel�, vaikuttaa siihen ilmestyyk� pop-up valikkoon paste
    bool copyPasteCrossSectionDrawParamListUsedYet_ = false;
    // T�t� k�ytet��n kopioimaan ja pasteamaan koko karttan�yt�n kaikkien rivien kaikki parametrit
    std::unique_ptr<NFmiPtrList<NFmiDrawParamList>> copyPasteDrawParamListVector_;
    // Onko yht��n copy komentoa tehty viel�, vaikuttaa siihen ilmestyyk� pop-up valikkoon paste
    bool copyPasteDrawParamListVectorUsedYet_ = false;

    // ******* Itse combined map moodiin liittyvi� muuttujia *****************
    TotalMapViewCombinedMapModeState combinedBackgroundMapModeStates_;
    TotalMapViewCombinedMapModeState combinedOverlayMapModeStates_;

    // Gui:hin liittyv�� map-layer tietoa (n�iden avulla tehd��n ainakin select-map-layer popup valikot).
    // staticxxx viittaa siihen ett� kyse on staattisista bitmapeista tiedostoissa, niit�
    // voi olla erilaiset setit jokaiselle erilaiselle kartta-alueelle.
    std::vector<MapAreaMapLayerRelatedInfo> staticBackgroundMapLayerRelatedInfos_;
    std::vector<MapAreaMapLayerRelatedInfo> staticOverlayMapLayerRelatedInfos_;
    // wmsxxx viittaa dynaamisiin Wms server layereihin, niit� on sama setti kaikille 
    // kartta-alueille, siksi niille yksinkertainen vector rakenne.
    MapAreaMapLayerRelatedInfo wmsBackgroundMapLayerRelatedInfos_;
    MapAreaMapLayerRelatedInfo wmsOverlayMapLayerRelatedInfos_;

public:
    ~NFmiCombinedMapHandler();

    void initialize(const std::string& absoluteControlPath);

    // Interface's override methods
    void storeMapViewDescTopToSettings() override;
    bool currentTime(unsigned int mapViewDescTopIndex, const NFmiMetTime& newCurrentTime, bool stayInsideAnimationTimes) override;
    const NFmiMetTime& currentTime(unsigned int mapViewDescTopIndex) override;
    void mapViewDirty(unsigned int mapViewDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool doClearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers) override;
    void makeNeededDirtyOperationsWhenDataAdded(NFmiQueryData* queryData, NFmiInfoData::Type dataType, const NFmiTimeDescriptor& removedDataTimes, const std::string& fileName) override;
    const NFmiTimeDescriptor& timeControlViewTimes(unsigned int mapViewDescTopIndex) override;
    void timeControlViewTimes(unsigned int mapViewDescTopIndex, const NFmiTimeDescriptor& newTimeDescriptor) override;
    void maskChangedDirtyActions() override;
    NFmiDrawParamList* getDrawParamList(unsigned int mapViewDescTopIndex, int rowIndex) override;
    NFmiDrawParamList* getDrawParamListWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex) override;
    void makeMapViewRowDirty(int mapViewDescTopIndex, int viewRowIndex) override;
    bool timeSerialViewDirty() override;
    void timeSerialViewDirty(bool newValue) override;
    MapViewDescTopVector& getMapViewDescTops() override;
    SmartMetViewId getUpdatedViewIdMaskForChangingTime() override;
    NFmiMapViewDescTop* getMapViewDescTop(unsigned int mapViewDescTopIndex, bool allowNullptrReturn = false) const override;
    void setMapArea(unsigned int mapViewDescTopIndex, const boost::shared_ptr<NFmiArea>& newArea) override;
    void storeMapViewSettingsToWinRegistry() override;
    void centerTimeControlView(unsigned int mapviewDescTopIndex, const NFmiMetTime& wantedTime, bool updateSelectedTime) override;
    const std::unique_ptr<NFmiFastDrawParamList>& getModifiedPropertiesDrawParamList() const override;
    unsigned int getRealRowNumber(unsigned int mapViewDescTopIndex, int rowIndex) override;
    unsigned int getRelativeRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex) override;
    void hideShowAllMapViewParams(unsigned int mapViewDescTopIndex, bool hideAllObservations, bool showAllObservations, bool hideAllForecasts, bool showAllForecasts) override;
    void setModelRunOffset(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void activateView(const NFmiMenuItem& menuItem, int rowIndex) override;
    void addViewWithRealRowNumber(bool normalParameterAdd, const NFmiMenuItem& menuItem, int realRowIndex, bool isViewMacroDrawParam, const std::string* macroParamInitFileName) override;
    void addCrossSectionView(const NFmiMenuItem& menuItem, int viewRowIndex, bool treatAsViewMacro) override;
    void addView(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void addAsOnlyView(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void changeParamLevel(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void removeView(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void removeAllViews(unsigned int mapViewDescTopIndex, int viewRowIndex) override;
    void addAsOnlyCrossSectionView(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void removeAllCrossSectionViews(int viewRowIndex) override;
    void toggleShowDifferenceToOriginalData(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void changeAllProducersInMapRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) override;
    void changeAllDataTypesInMapRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) override;
    void copyDrawParamOptions(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) override;
    void pasteDrawParamOptions(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) override;
    boost::shared_ptr<NFmiDrawParam> getUsedMapViewDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void copyMapViewDescTopParams(unsigned int mapViewDescTopIndex) override;
    void pasteMapViewDescTopParams(unsigned int mapViewDescTopIndex) override;
    bool copyPasteDrawParamAvailableYet() const override { return copyPasteDrawParamAvailableYet_; }
    bool copyPasteDrawParamListUsedYet() const override { return  copyPasteDrawParamListUsedYet_; }
    bool copyPasteCrossSectionDrawParamListUsedYet() const { return  copyPasteCrossSectionDrawParamListUsedYet_; }
    bool copyPasteDrawParamListVectorUsedYet() const  override { return  copyPasteDrawParamListVectorUsedYet_; }
    NFmiDrawParamList* getCrossSectionViewDrawParamList(int viewRowIndex) override;
    void updateMacroDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, bool crossSectionCase, boost::shared_ptr<NFmiDrawParam>& drawParam) override;
    boost::shared_ptr<NFmiDrawParam> getUsedMacroDrawParam(const NFmiMenuItem& menuItem) override;
    bool modifyMacroDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, bool crossSectionCase) override;
    void modifyCrossSectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void activateCrossSectionParam(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void modifyView(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void toggleShowLegendState(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void swapViewRows(const NFmiMenuItem& menuItem) override;
    void saveDrawParamSettings(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void forceStationViewRowUpdate(unsigned int mapViewDescTopIndex, unsigned int theRealRowIndex) override;
    void reloadDrawParamSettings(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void applyFixeDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void applyFixeDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, const std::shared_ptr<NFmiDrawParam>& fixedDrawParam) override;
    boost::shared_ptr<NFmiDrawParam> getCrosssectionDrawParamFromViewLists(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void removeCrosssectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void hideView(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void showView(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    NFmiPtrList<NFmiDrawParamList>& getCrossSectionDrawParamListVector() override;
    NFmiDrawParamList& getTimeSerialViewDrawParamList() override;
    NFmiDrawParamList* getTimeSerialViewSideParameters(int viewRowIndex) override;
    SideParametersContainer& getTimeSerialViewSideParameterList() override;
    void addTimeSerialViewSideParameter(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam) override;
    void removeTimeSerialViewSideParameter(const NFmiMenuItem& menuItem) override;
    void removeAllTimeSerialViewSideParameters(int viewRowIndex) override;
    void removeAllTimeSerialViews() override;
    void showCrossSectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, bool showParam) override;
    bool modifyDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void addTimeSerialView(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam) override;
    void removeTimeSerialView(const NFmiMenuItem& menuItem) override;
    void timeSerialViewModelRunCountSet(const NFmiMenuItem& menuItem) override;
    unsigned long& getTimeSerialViewIndexReference() override;
    boost::shared_ptr<NFmiDrawParam> activeDrawParamFromActiveRow(unsigned int theDescTopIndex) override;
    boost::shared_ptr<NFmiDrawParam> activeDrawParamWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex) override;
    void changeMapType(unsigned int mapViewDescTopIndex, bool goForward) override;
    bool scrollViewRow(unsigned int mapViewDescTopIndex, int scrollCount) override;
    void timeControlTimeStep(unsigned int mapViewDescTopIndex, float timeStepInMinutes) override;
    float timeControlTimeStep(unsigned int mapViewDescTopIndex) override;
    void copyDrawParamsFromViewRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) override;
    void pasteDrawParamsToViewRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) override;
    void copyDrawParamsFromMapViewRow(unsigned int mapViewDescTopIndex) override;
    void pasteDrawParamsToMapViewRow(unsigned int mapViewDescTopIndex) override;
    int absoluteActiveViewRow(unsigned int mapViewDescTopIndex) override;
    void absoluteActiveViewRow(unsigned int mapViewDescTopIndex, int theAbsoluteActiveRowIndex) override;
    void removeAllViewsWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex) override;
    NFmiProjectionCurvatureInfo* projectionCurvatureInfo() override;
    void projectionCurvatureInfo(const NFmiProjectionCurvatureInfo& newValue) override;
    NFmiPtrList<NFmiDrawParamList>* getDrawParamListVector(unsigned int mapViewDescTopIndex) override;
    void clearDesctopsAllParams(unsigned int mapViewDescTopIndex) override;
    void makeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> >& infoVectorOut, boost::shared_ptr<NFmiDrawParam>& drawParam, const boost::shared_ptr<NFmiArea>& area) override;
    void makeApplyViewMacroDirtyActions(double drawObjectScaleFactor) override;
    void makeSwapBaseArea(unsigned int mapViewDescTopIndex) override;
    void swapArea(unsigned int mapViewDescTopIndex) override;
    void removeMacroParamFromDrawParamLists(const std::string& macroParamName) override;
    bool moveActiveMapViewParamInDrawingOrderList(unsigned int mapViewDescTopIndex, int viewRowIndex, bool raiseParam, bool useCrossSectionParams) override;
    bool changeActiveMapViewParam(unsigned int mapViewDescTopIndex, int realRowIndex, int paramIndex, bool nextParam, bool useCrossSectionParams) override;
    void makeMacroParamCacheUpdatesForWantedRow(int mapViewDescTopIndex, int viewRowIndex) override;
    void takeDrawParamInUseEveryWhere(boost::shared_ptr<NFmiDrawParam>& drawParam, bool useInMap, bool useInTimeSerial, bool useInCrossSection, bool useWithViewMacros) override;
    void borrowParams(unsigned int mapViewDescTopIndex, int realViewRowIndex) override;
    bool setMapViewGrid(unsigned int mapViewDescTopIndex, const NFmiPoint& newValue) override;
    CtrlViewUtils::GraphicalInfo& getGraphicalInfo(unsigned int mapViewDescTopIndex) override;
    void onShowGridPoints(unsigned int mapViewDescTopIndex) override;
    void onToggleGridPointColor(unsigned int mapViewDescTopIndex) override;
    void onToggleGridPointSize(unsigned int mapViewDescTopIndex) override;
    void onEditSpaceOut(unsigned int mapViewDescTopIndex) override;
    void onChangeParamWindowPosition(unsigned int mapViewDescTopIndex, bool forward) override;
    void onShowTimeString(unsigned int mapViewDescTopIndex) override;
    WmsSupportInterface& getWmsSupport() override;
    void onToggleShowNamesOnMap(unsigned int mapViewDescTopIndex, bool goForward) override;
    void onToggleLandBorderDrawColor(unsigned int mapViewDescTopIndex) override;
    void onToggleLandBorderPenSize(unsigned int mapViewDescTopIndex) override;
    void updateMapView(unsigned int mapViewDescTopIndex) override;
    void onAcceleratorBorrowParams(unsigned int mapViewDescTopIndex, int viewRowIndex) override;
    void onAcceleratorMapRow(unsigned int mapViewDescTopIndex, int startingViewRow) override;
    void onToggleOverMapBackForeGround(unsigned int mapViewDescTopIndex) override;
    void onAcceleratorToggleKeepMapRatio() override;
    void onButtonDataArea(unsigned int mapViewDescTopIndex) override;
    double drawObjectScaleFactor() override;
    void drawObjectScaleFactor(double newValue) override;
    boost::shared_ptr<NFmiDrawParam> getUsedDrawParamForEditedData(const NFmiDataIdent& dataIdent) override;
    std::string getCurrentMapLayerGuiName(int mapViewDescTopIndex, bool backgroundMap) override;
    std::string getCurrentMapLayerGuiText(int mapViewDescTopIndex, bool backgroundMap) override;
    bool useCombinedMapMode() const override;
    void useCombinedMapMode(bool newValue) override;
    const NFmiMetTime& activeMapTime() override;
    bool changeParamSettingsToNextFixedDrawParam(unsigned int mapViewDescTopIndex, int realRowIndex, int paramIndex, bool gotoNext) override;
    void checkAnimationLockedModeTimeBags(unsigned int mapViewDescTopIndex, bool ignoreSatelImages) override;
    bool setDataToNextTime(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes) override;
    bool setDataToPreviousTime(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes) override;
    void updateFromModifiedDrawParam(boost::shared_ptr<NFmiDrawParam>& drawParam, bool groundData) override;
    void updateToModifiedDrawParam(unsigned int mapViewDescTopIndex, boost::shared_ptr<NFmiDrawParam>& drawParam, int viewRowIndex) override;
    unsigned int activeMapDescTopIndex() override;
    void activeMapDescTopIndex(unsigned int mapViewDescTopIndex) override;
    void doAutoZoom(unsigned int mapViewDescTopIndex) override;
    void setSelectedMapHandler(unsigned int mapViewDescTopIndex, unsigned int newMapIndex) override;
    bool changeTime(int typeOfChange, FmiDirection direction, unsigned long mapViewDescTopIndex, double amountOfChange) override;
    void setMapViewCacheSize(double theNewSizeInMB) override;
    void updateRowInLockedDescTops(unsigned int originalMapViewDescTopIndex) override;
    void setSelectedMap(unsigned int mapViewDescTopIndex, int newMapIndex) override;
    void checkForNewConceptualModelData() override;
    int toggleShowTimeOnMapMode(unsigned int mapViewDescTopIndex) override;
    void makeWholeDesctopDirtyActions(unsigned int mapViewDescTopIndex, NFmiPtrList<NFmiDrawParamList>* drawParamListVector) override;
    boost::shared_ptr<NFmiDrawParam> getDrawParamFromViewLists(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    bool useWmsMapDrawForThisDescTop(unsigned int mapViewDescTopIndex) override;
    bool useWmsOverlayMapDrawForThisDescTop(unsigned int mapViewDescTopIndex) override;
    bool wmsSupportAvailable() const override;
    bool localOnlyMapModeUsed() const override;
    NFmiCombinedMapModeState& getCombinedMapModeState(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex) override;
    NFmiCombinedMapModeState& getCombinedOverlayMapModeState(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex) override;
    bool isOverlayMapDrawnForThisDescTop(unsigned int mapViewDescTopIndex, int wantedDrawOverMapMode) override;
    void addBorderLineLayer(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void moveBorderLineLayer(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void insertParamLayer(const NFmiMenuItem& menuItem, int viewRowIndex) override;
    void setBorderDrawDirtyState(unsigned int mapViewDescTopIndex, CountryBorderDrawDirtyState newState, NFmiDrawParam* separateBorderLayerDrawOptions = nullptr) override;
    void activeEditedParameterMayHaveChangedViewUpdateFlagSetting(int mapViewDescTopIndex) override;
    const MapAreaMapLayerRelatedInfo& getCurrentMapLayerRelatedInfos(int mapViewDescTopIndex, bool backgroundMapCase, bool wmsCase) override;
    void selectMapLayer(unsigned int mapViewDescTopIndex, const std::string& mapLayerName, bool backgroundMapCase, bool wmsCase) override;
    std::pair<std::string, std::string> getMacroReferenceNamesForViewMacro(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex) override;
    void selectMapLayersByMacroReferenceNamesFromViewMacro(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, const std::string& backgroundMacroReferenceName, const std::string& overlayMacroReferenceName) override;
    void selectCombinedMapModeIndices(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, int usedCombinedModeMapIndex, int usedCombinedModeOverlayMapIndex) override;

private:
    unsigned int getMapViewCount() const;
    unsigned int getMapAreaCount() const;
    unsigned int getCurrentMapAreaIndex(unsigned int mapViewDescTopIndex) const;
    std::unique_ptr<NFmiMapViewDescTop> createMapViewDescTop(const std::string& baseSettingStr, int mapViewIndex);
    std::string getMapViewDescTopSettingString(const std::string& baseStr, int mapViewDescTopIndex);
    void initWmsSupport();
    void initMapConfigurationSystemMain();
    void initMapConfigurationSystem();
    void initProjectionCurvatureInfo();
    void initLandBorderDrawingSystemWithGdal();
    std::pair<unsigned int, MapViewCombinedMapModeState> makeTotalMapViewCombinedMapModeState(unsigned int mapViewIndex, unsigned int usedWmsMapLayerCount, bool doBackgroundCase);
    void initCombinedMapStates();
    void storeCombinedMapStates();
    void initCombinedMapSelectionIndices();
    void initWmsSupportSelectionIndices();
    std::vector<int> getCombinedModeSelectedMapIndicesFromWinRegistry(unsigned int mapViewDescTopIndex, bool doBackgroundMaps);
    void doCutBorderDrawInitializationWithGdal();
    void generateClippedCountryBorderGeometrysWithGdal();
    void makeNeededDirtyOperationsWhenDataAdded(unsigned int mapViewDescTopIndex, NFmiFastQueryInfo& fastInfo, NFmiInfoData::Type dataType, const NFmiTimeBag& dirtyViewTimes, const std::string& fileName);
    void logMessage(const std::string &logMessage, CatLog::Severity severity, CatLog::Category category);
    void logAndWarnUser(const std::string& logMessage, const std::string& titleString, CatLog::Severity severity, CatLog::Category category, bool addAbortOption);
    void mapViewDirtyForAllDescTops(bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool doClearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers);
    void clearMacroParamDataCache(unsigned int mapViewDescTopIndex, bool doClearMacroParamDataCache, bool clearEditedDataDependentCaches);
    bool makeMacroParamDrawingLayerCacheChecks(boost::shared_ptr<NFmiDrawParam>& drawParam, NFmiFastQueryInfo& fastInfo, NFmiInfoData::Type dataType, NFmiMapViewDescTop& descTop, unsigned int mapViewDescTopIndex, int cacheRowNumber, const std::string& fileName);
    std::string doGetMacroParamFormula(boost::shared_ptr<NFmiDrawParam>& drawParam, CatLog::Category category);
    bool isAnimationTimebagCheckNeeded(unsigned int mapviewDescTopIndex);
    bool findLastObservation(unsigned long mapViewDescTopIndex, int timeStepInMinutes, NFmiMetTime& newLastTime, bool ignoreSatelImages);
    bool findEarliestLastObservation(unsigned long mapViewDescTopIndex, int timeStepInMinutes, NFmiMetTime& newLastTime, bool ignoreSatelImages);
    void updateTimeInLockedDescTops(const NFmiMetTime& wantedTime, unsigned int originalMapViewDescTopIndex);
    std::vector<std::string> makeListOfUsedMacroParamsDependedOnEditedData();
    void clearAllMacroParamDataCacheDependentOfEditedDataAfterEditedDataChanges();
    bool isMacroParamDependentOfEditedData(boost::shared_ptr<NFmiDrawParam>& drawParam);
    void clearAllViewRowsWithEditedData();
    bool doMacroParamVerticalDataChecks(NFmiFastQueryInfo& info, NFmiInfoData::Type dataType, const MacroParamDataInfo& macroParamDataInfo);
    NFmiMetTime adjustTimeToDescTopTimeStep(unsigned int mapViewDescTopIndex, const NFmiMetTime& wantedTime);
    bool currentTimeForAllDescTops(const NFmiMetTime& newCurrentTime);
    bool doAnimationRestriction(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes);
    NFmiMetTime calcAnimationRestrictedTime(unsigned int mapViewDescTopIndex, const NFmiMetTime& wantedTime, bool stayInsideAnimationTimes);
    bool checkForNewConceptualModelDataBruteForce(unsigned int mapViewDescTopIndex);
    bool isAnimationTimeBoxVisibleOverTimeControlView(unsigned int mapViewDescTopIndex);
    NFmiMetTime calcCenterAnimationTimeBoxTime(unsigned int mapViewDescTopIndex);
    void setAnimationBoxToVisibleIfNecessary(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes);
    bool setDataToNextTimeForAllDescTops(bool stayInsideAnimationTimes);
    bool setDataToPreviousTimeForAllDescTops(bool stayInsideAnimationTimes);
    void makeViewRowDirtyActions(unsigned int mapViewDescTopIndex, int realRowIndex, NFmiDrawParamList* drawParamList);
    void drawParamSettingsChangedDirtyActions(unsigned int mapViewDescTopIndex, int realRowIndex, boost::shared_ptr<NFmiDrawParam>& drawParam);
    NFmiDrawParamList* getWantedDrawParamList(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams);
    void initCrossSectionDrawParamListVector();
    void setWantedLayerIndex(const NFmiCombinedMapModeState& combinedMapModeState, unsigned int mapViewDescTopIndex, bool backgroundCase);
    void setWantedLayerIndex(const NFmiCombinedMapModeState& combinedMapModeState, unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, bool backgroundCase);
    void mapLayerChangedRefreshActions(unsigned int mapViewDescTopIndex, const std::string& refreshMessage);
    void initializeMapLayerInfos();
    void initializeStaticMapLayerInfos(std::vector<MapAreaMapLayerRelatedInfo> &mapLayerRelatedInfos, bool backgroundMapCase);
    void initializeWmsMapLayerInfos();
    int getSelectedCombinedModeMapIndex(int mapViewDescTopIndex, const std::string& mapLayerName, bool backgroundMapCase, bool wmsCase);
    void selectMapLayerByMacroReferenceNameFromViewMacro(bool backgroundMapCase, unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, NFmiCombinedMapModeState& combinedMapModeState, const std::string& macroReferenceName, const MapAreaMapLayerRelatedInfo& staticMapLayerRelatedInfos, const MapAreaMapLayerRelatedInfo& wmsMapLayerRelatedInfos);
    void addEmptySideParamList(int viewRowIndex);
    void removeSideParamList(int viewRowIndex);
    SideParametersIterator getTimeSerialViewSideParameterIterator(int viewRowIndex);
    boost::shared_ptr<NFmiDrawParam> createTimeSerialViewDrawParam(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam);
};
