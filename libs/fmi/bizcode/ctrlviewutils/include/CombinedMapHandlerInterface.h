#pragma once

#include "NFmiInfoData.h"
#include "NFmiPtrList.h"
#include "NFmiPoint.h"
#include "SmartMetViewId.h"
#include "CtrlViewFunctions.h"

#include "boost/shared_ptr.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <list>

class NFmiMetTime;
class NFmiQueryData;
class NFmiTimeDescriptor;
class NFmiDrawParam;
class NFmiDrawParamList;
class NFmiMapViewDescTop;
class NFmiArea;
class NFmiFastDrawParamList;
class NFmiMenuItem;
class NFmiFastQueryInfo;
class NFmiDataIdent;
class NFmiLevel;
class NFmiProjectionCurvatureInfo;
class WmsSupportInterface;
class NFmiCombinedMapModeState;
class NFmiColor;

namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

enum class CountryBorderDrawDirtyState
{
    None = 0, // Mit‰‰n ei liata
    Cosmetic = 1, // Vain viivan v‰ri tai paksuus on muuttunut, voidaan piirt‰‰ samoilla path-olioilla
    Geometry = 2 // Karttaa on zoomattu tai kartta ruudun kokoa on muutettu, kaikki piirto ja laskenta uusiksi
};

// N‰it‰ NFmiMapLayerGuiRelatedInfo tietoja k‰ytet‰‰n mm. kun rakennetaan kartta layerien vaihtoon liittyvi‰ popup valikoita.
// Samoin viewMacrojen reference nimien kanssa.
class NFmiMapLayerRelatedInfo
{
public:
    NFmiMapLayerRelatedInfo() = default;
    NFmiMapLayerRelatedInfo(const std::string& guiName, const std::string& macroReference, bool isWmsLayer);

    static const std::string& getWmsMacroReferencePrefix();
    static std::string addWmsMacroReferencePrefix(std::string macroReference);
    static bool hasWmsMacroReferencePrefix(const std::string& macroReference);
    static std::string stripWmsMacroReferencePrefix(std::string macroReference);

    std::string guiName_;
    std::string macroReference_;
    std::string prefixedMacroReference_;
    bool isWmsLayer_ = false;
};
// MapAreaMapLayerRelatedInfo on yhden kartta-alueen (esim. skandinavia) kaikki layerit.
using MapAreaMapLayerRelatedInfo = std::vector<NFmiMapLayerRelatedInfo>;

// Interface that is meant to be used to handle all kinds of background map and overlay map stuff handling.
class CombinedMapHandlerInterface
{
public:
    using MapViewDescTopVector = std::vector<std::unique_ptr<NFmiMapViewDescTop>>;
    using GetCombinedMapHandlerInterfaceImplementationCallBackType = std::function<CombinedMapHandlerInterface*(void)>;
    // Aikasarjaikkunan sivu-parametri container
    using SideParametersContainer = std::list<std::unique_ptr<NFmiDrawParamList>>;
    // Aikasarjaikkunan sivu-parametri containerin iteraattori
    using SideParametersIterator = std::list<std::unique_ptr<NFmiDrawParamList>>::iterator;

    // T‰m‰ pit‰‰ asettaa johonkin konkreettiseen funktioon, jotta k‰ytt‰j‰ koodi saa k‰yttˆˆns‰ halutun interface toteutuksen
    static GetCombinedMapHandlerInterfaceImplementationCallBackType GetCombinedMapHandlerInterfaceImplementation;

    virtual ~CombinedMapHandlerInterface() = default;

    virtual void storeMapViewDescTopToSettings() = 0;
    virtual bool currentTime(unsigned int mapViewDescTopIndex, const NFmiMetTime& newCurrentTime, bool stayInsideAnimationTimes) = 0;
    virtual const NFmiMetTime& currentTime(unsigned int mapViewDescTopIndex) = 0;
    virtual void mapViewDirty(unsigned int mapViewDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool doClearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers) = 0;
    virtual void makeNeededDirtyOperationsWhenDataAdded(NFmiQueryData* queryData, NFmiInfoData::Type dataType, const NFmiTimeDescriptor& removedDataTimes, const std::string& fileName) = 0;
    virtual const NFmiTimeDescriptor& timeControlViewTimes(unsigned int mapViewDescTopIndex) = 0;
    virtual void timeControlViewTimes(unsigned int mapViewDescTopIndex, const NFmiTimeDescriptor& newTimeDescriptor) = 0;
    virtual void maskChangedDirtyActions() = 0;
    virtual NFmiDrawParamList* getDrawParamList(unsigned int mapViewDescTopIndex, int rowIndex) = 0;
    virtual NFmiDrawParamList* getDrawParamListWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex) = 0;
    virtual void makeMapViewRowDirty(int mapViewDescTopIndex, int viewRowIndex) = 0;
    virtual bool timeSerialViewDirty() = 0;
    virtual void timeSerialViewDirty(bool newValue) = 0;
    virtual MapViewDescTopVector& getMapViewDescTops() = 0;
    virtual SmartMetViewId getUpdatedViewIdMaskForChangingTime() = 0;
    virtual NFmiMapViewDescTop* getMapViewDescTop(unsigned int mapViewDescTopIndex, bool allowNullptrReturn = false) const = 0;
    virtual void setMapArea(unsigned int mapViewDescTopIndex, const boost::shared_ptr<NFmiArea>& newArea) = 0;
    virtual void storeMapViewSettingsToWinRegistry() = 0;
    virtual void centerTimeControlView(unsigned int mapviewDescTopIndex, const NFmiMetTime& wantedTime, bool updateSelectedTime) = 0;
    virtual const std::unique_ptr<NFmiFastDrawParamList>& getModifiedPropertiesDrawParamList() const = 0;
    virtual unsigned int getRealRowNumber(unsigned int mapViewDescTopIndex, int rowIndex) = 0;
    virtual unsigned int getRelativeRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex) = 0;
    virtual void hideShowAllMapViewParams(unsigned int mapViewDescTopIndex, bool hideAllObservations, bool showAllObservations, bool hideAllForecasts, bool showAllForecasts) = 0;
    virtual void setModelRunOffset(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void setModelRunOffsetForAllModelDataOnActiveRow(unsigned int mapViewDescTopIndex, FmiDirection direction) = 0;
    virtual void activateView(const NFmiMenuItem& menuItem, int rowIndex) = 0;
    virtual void addViewWithRealRowNumber(bool normalParameterAdd, const NFmiMenuItem& menuItem, int realRowIndex, bool isViewMacroDrawParam) = 0;
    virtual void addCrossSectionView(const NFmiMenuItem& menuItem, int viewRowIndex, bool treatAsViewMacro) = 0;
    virtual void addView(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void changeParamLevel(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void removeView(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void removeAllViews(unsigned int mapViewDescTopIndex, int viewRowIndex) = 0;
    virtual void removeAllCrossSectionViews(int viewRowIndex) = 0;
    virtual void toggleShowDifferenceToOriginalData(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void changeAllProducersInMapRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) = 0;
    virtual void changeAllDataTypesInMapRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) = 0;
    virtual void copyDrawParamOptions(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) = 0;
    virtual void pasteDrawParamOptions(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> getUsedMapViewDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void copyMapViewDescTopParams(unsigned int mapViewDescTopIndex) = 0;
    virtual void pasteMapViewDescTopParams(unsigned int mapViewDescTopIndex) = 0;
    virtual bool copyPasteDrawParamAvailableYet() const = 0;
    virtual bool copyPasteDrawParamListUsedYet() const = 0;
    virtual bool copyPasteCrossSectionDrawParamListUsedYet() const = 0;
    virtual bool copyPasteDrawParamListVectorUsedYet() const = 0;
    virtual NFmiDrawParamList* getCrossSectionViewDrawParamList(int viewRowIndex) = 0;
    virtual void modifyCrossSectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void activateCrossSectionParam(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void modifyView(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void toggleShowLegendState(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void swapViewRows(const NFmiMenuItem& menuItem) = 0;
    virtual void saveDrawParamSettings(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void saveDrawParamSettings(boost::shared_ptr<NFmiDrawParam>& drawParam) = 0;
    virtual void forceStationViewRowUpdate(unsigned int theDescTopIndex, unsigned int theRealRowIndex) = 0;
    virtual void reloadDrawParamSettings(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void applyFixeDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void applyFixeDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, const std::shared_ptr<NFmiDrawParam>& fixedDrawParam) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> getCrosssectionDrawParamFromViewLists(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void removeCrosssectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void hideView(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void showView(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual NFmiPtrList<NFmiDrawParamList>& getCrossSectionDrawParamListVector() = 0;
    virtual NFmiDrawParamList& getTimeSerialViewDrawParamList() = 0;
    virtual NFmiDrawParamList* getTimeSerialViewSideParameters(int viewRowIndex) = 0;
    virtual SideParametersContainer& getTimeSerialViewSideParameterList() = 0;
    virtual boost::shared_ptr<NFmiDrawParam> addTimeSerialViewSideParameter(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam) = 0;
    virtual void removeTimeSerialViewSideParameter(const NFmiMenuItem& menuItem) = 0;
    virtual void removeAllTimeSerialViewSideParameters(int viewRowIndex) = 0;
    virtual void removeAllTimeSerialViews() = 0;
    virtual void showCrossSectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, bool showParam) = 0;
    virtual bool modifyDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> addTimeSerialView(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam) = 0;
    virtual void removeTimeSerialView(const NFmiMenuItem& menuItem) = 0;
    virtual void timeSerialViewModelRunCountSet(const NFmiMenuItem& menuItem) = 0;
    virtual unsigned long& getTimeSerialViewIndexReference() = 0;
    virtual boost::shared_ptr<NFmiDrawParam> activeDrawParamFromActiveRow(unsigned int theDescTopIndex) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> activeDrawParamWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex) = 0;
    virtual void changeMapType(unsigned int mapViewDescTopIndex, bool goForward) = 0;
    virtual bool scrollViewRow(unsigned int mapViewDescTopIndex, int scrollCount) = 0;
    virtual void timeControlTimeStep(unsigned int mapViewDescTopIndex, float timeStepInMinutes) = 0;
    virtual float timeControlTimeStep(unsigned int mapViewDescTopIndex) = 0;
    virtual void copyDrawParamsFromViewRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) = 0;
    virtual void pasteDrawParamsToViewRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams) = 0;
    virtual void copyDrawParamsFromMapViewRow(unsigned int mapViewDescTopIndex) = 0;
    virtual void pasteDrawParamsToMapViewRow(unsigned int mapViewDescTopIndex) = 0;
    virtual int absoluteActiveViewRow(unsigned int mapViewDescTopIndex) = 0;
    virtual void absoluteActiveViewRow(unsigned int mapViewDescTopIndex, int theAbsoluteActiveRowIndex) = 0;
    virtual void removeAllViewsWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex) = 0;
    virtual NFmiProjectionCurvatureInfo* projectionCurvatureInfo() = 0;
    virtual void projectionCurvatureInfo(const NFmiProjectionCurvatureInfo& newValue) = 0;
    virtual NFmiPtrList<NFmiDrawParamList>* getDrawParamListVector(unsigned int mapViewDescTopIndex) = 0;
    virtual void clearDesctopsAllParams(unsigned int mapViewDescTopIndex) = 0;
    virtual void makeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> >& infoVectorOut, boost::shared_ptr<NFmiDrawParam>& drawParam, const boost::shared_ptr<NFmiArea>& area) = 0;
    virtual void makeApplyViewMacroDirtyActions(double drawObjectScaleFactor) = 0;
    virtual void makeSwapBaseArea(unsigned int mapViewDescTopIndex) = 0;
    virtual void swapArea(unsigned int mapViewDescTopIndex) = 0;
    virtual void removeMacroParamFromDrawParamLists(const std::string& macroParamName) = 0;
    virtual bool moveActiveMapViewParamInDrawingOrderList(unsigned int mapViewDescTopIndex, int viewRowIndex, bool raiseParam, bool useCrossSectionParams) = 0;
    virtual bool changeActiveMapViewParam(unsigned int mapViewDescTopIndex, int realRowIndex, int paramIndex, bool nextParam, bool useCrossSectionParams) = 0;
    virtual void makeMacroParamCacheUpdatesForWantedRow(int mapViewDescTopIndex, int viewRowIndex) = 0;
    virtual void takeDrawParamInUseEveryWhere(boost::shared_ptr<NFmiDrawParam> & drawParam, bool useInMap, bool useInTimeSerial, bool useInCrossSection, bool useWithViewMacros) = 0;
    virtual void borrowParams(unsigned int mapViewDescTopIndex, int realViewRowIndex) = 0;
    virtual bool setMapViewGrid(unsigned int mapViewDescTopIndex, const NFmiPoint & newValue) = 0;
    virtual CtrlViewUtils::GraphicalInfo& getGraphicalInfo(unsigned int mapViewDescTopIndex) = 0;
    virtual void onShowGridPoints(unsigned int mapViewDescTopIndex) = 0;
    virtual void onToggleGridPointColor(unsigned int mapViewDescTopIndex) = 0;
    virtual void onToggleGridPointSize(unsigned int mapViewDescTopIndex) = 0;
    virtual void onEditSpaceOut(unsigned int mapViewDescTopIndex) = 0;
    virtual void onChangeParamWindowPosition(unsigned int mapViewDescTopIndex, bool forward) = 0;
    virtual void onMoveTimeBoxLocation(unsigned int mapViewDescTopIndex) = 0;
    virtual bool onSetTimeBoxLocation(unsigned int mapViewDescTopIndex, FmiDirection newPosition) = 0;
    virtual bool onSetTimeBoxTextSizeFactor(unsigned int mapViewDescTopIndex, float newSizeFactor) = 0;
    virtual void onSetTimeBoxFillColor(unsigned int mapViewDescTopIndex, NFmiColor newColorNoAlpha) = 0;
    virtual bool onSetTimeBoxFillColorAlpha(unsigned int mapViewDescTopIndex, float newColorAlpha) = 0;
    virtual void onShowTimeString(unsigned int mapViewDescTopIndex) = 0;
    virtual std::shared_ptr<WmsSupportInterface> getWmsSupport() const = 0;
    virtual void onToggleShowNamesOnMap(unsigned int mapViewDescTopIndex, bool goForward) = 0;
    virtual void onToggleLandBorderDrawColor(unsigned int mapViewDescTopIndex) = 0;
    virtual void onToggleLandBorderPenSize(unsigned int mapViewDescTopIndex) = 0;
    virtual void updateMapView(unsigned int mapViewDescTopIndex) = 0;
    virtual void onAcceleratorBorrowParams(unsigned int mapViewDescTopIndex, int viewRowIndex) = 0;
    virtual void onAcceleratorMapRow(unsigned int mapViewDescTopIndex, int startingViewRow) = 0;
    virtual void onToggleOverMapBackForeGround(unsigned int mapViewDescTopIndex) = 0;
    virtual void onAcceleratorToggleKeepMapRatio() = 0;
    virtual void onButtonDataArea(unsigned int mapViewDescTopIndex) = 0;
    virtual double drawObjectScaleFactor() = 0;
    virtual void drawObjectScaleFactor(double newValue) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> getUsedDrawParamForEditedData(const NFmiDataIdent& dataIdent) = 0;
    virtual std::string getCurrentMapLayerGuiName(int mapViewDescTopIndex, bool backgroundMap) = 0;
    virtual std::string getCurrentMapLayerGuiText(int mapViewDescTopIndex, bool backgroundMap) = 0;
    virtual bool useCombinedMapMode() const = 0;
    virtual void useCombinedMapMode(bool newValue) = 0;
    virtual const NFmiMetTime& activeMapTime() = 0;
    virtual bool changeParamSettingsToNextFixedDrawParam(unsigned int mapViewDescTopIndex, int realRowIndex, int paramIndex, bool gotoNext) = 0;
    virtual void checkAnimationLockedModeTimeBags(unsigned int mapViewDescTopIndex, bool ignoreSatelImages) = 0;
    virtual bool setDataToNextTime(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes) = 0;
    virtual bool setDataToPreviousTime(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes) = 0;
    virtual void updateFromModifiedDrawParam(boost::shared_ptr<NFmiDrawParam>& drawParam, bool groundData) = 0;
    virtual void updateToModifiedDrawParam(unsigned int mapViewDescTopIndex, boost::shared_ptr<NFmiDrawParam>& drawParam, int viewRowIndex) = 0;
    virtual unsigned int activeMapDescTopIndex() = 0;
    virtual void activeMapDescTopIndex(unsigned int mapViewDescTopIndex) = 0;
    virtual void doAutoZoom(unsigned int mapViewDescTopIndex) = 0;
    virtual void setSelectedMapHandler(unsigned int mapViewDescTopIndex, unsigned int newMapIndex) = 0;
    virtual bool changeTime(int typeOfChange, FmiDirection direction, unsigned long mapViewDescTopIndex, double amountOfChange) = 0;
    virtual void setMapViewCacheSize(double theNewSizeInMB) = 0;
    virtual void updateRowInLockedDescTops(unsigned int originalMapViewDescTopIndex) = 0;
    virtual void setSelectedMap(unsigned int mapViewDescTopIndex, int newMapIndex) = 0;
    virtual void checkForNewConceptualModelData() = 0;
    virtual int toggleShowTimeOnMapMode(unsigned int mapViewDescTopIndex) = 0;
    virtual void makeWholeDesctopDirtyActions(unsigned int mapViewDescTopIndex, NFmiPtrList<NFmiDrawParamList>* drawParamListVector) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> getDrawParamFromViewLists(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual bool useWmsMapDrawForThisDescTop(unsigned int mapViewDescTopIndex) = 0;
    virtual bool useWmsOverlayMapDrawForThisDescTop(unsigned int mapViewDescTopIndex) = 0;
    virtual bool wmsSupportAvailable() const = 0;
    virtual bool localOnlyMapModeUsed() const = 0;
    virtual NFmiCombinedMapModeState& getCombinedMapModeState(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex) = 0;
    virtual NFmiCombinedMapModeState& getCombinedOverlayMapModeState(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex) = 0;
    // wantedDrawOverMapMode parametri tarkoittaa ett‰ miss‰ tilassa piirtokoodia kysely tehd‰‰n (SmartMetissa moodi vaihtuu SHIFT + B:ll‰):
    // Jos arvo on 0, oltaisiin piirt‰m‰ss‰ overlay layeria heti pohjakartan p‰‰lle.
    // Jos arvo on 1, oltaisiin piirt‰m‰ss‰ overlay layeria data visualisointien p‰‰lle.
    virtual bool isOverlayMapDrawnForThisDescTop(unsigned int mapViewDescTopIndex, int wantedDrawOverMapMode) = 0;
    virtual void addBorderLineLayer(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void moveBorderLineLayer(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void insertParamLayer(const NFmiMenuItem& menuItem, int viewRowIndex) = 0;
    virtual void setBorderDrawDirtyState(unsigned int mapViewDescTopIndex, CountryBorderDrawDirtyState newState, NFmiDrawParam* separateBorderLayerDrawOptions = nullptr) = 0;
    virtual void activeEditedParameterMayHaveChangedViewUpdateFlagSetting(int mapViewDescTopIndex) = 0;
    virtual const MapAreaMapLayerRelatedInfo& getCurrentMapLayerRelatedInfos(int mapViewDescTopIndex, bool backgroundMapCase, bool wmsCase) = 0;
    virtual void selectMapLayer(unsigned int mapViewDescTopIndex, const std::string& mapLayerName, bool backgroundMapCase, bool wmsCase) = 0;
    virtual std::pair<std::string, std::string> getMacroReferenceNamesForViewMacro(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex) = 0;
    virtual void selectMapLayersByMacroReferenceNamesFromViewMacro(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, const std::string& backgroundMacroReferenceName, const std::string& overlayMacroReferenceName) = 0;
    virtual void selectCombinedMapModeIndices(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, int usedCombinedModeMapIndex, int usedCombinedModeOverlayMapIndex) = 0;
    virtual void clearMacroParamCache(unsigned long mapViewDescTopIndex, unsigned long realRowIndex, boost::shared_ptr<NFmiDrawParam>& drawParam) = 0;
    virtual void clearAllMacroParamDataCacheDependentOfEditedDataAfterEditedDataChanges() = 0;


    // Staattiset perushelper-funktiot
    static bool isDrawParamForecast(boost::shared_ptr<NFmiDrawParam>& drawParam);
    static bool isDrawParamObservation(boost::shared_ptr<NFmiDrawParam>& drawParam);
    static std::string getSelectedParamInfoString(const NFmiDataIdent* dataIdent, const NFmiLevel* level);
    static std::string getSelectedParamInfoString(boost::shared_ptr<NFmiFastQueryInfo>& fastInfo, bool ignoreLevel);
    static void verboseLogging(bool newValue);
    static bool verboseLogging();
    static void doVerboseFunctionStartingLogReporting(const std::string& functionName);
    static int getBorderLayerIndex(NFmiDrawParamList* drawParamList);
    static bool hasSeparateBorderLayer(NFmiDrawParamList* drawParamList);
    static void copyDrawParamsList(NFmiPtrList<NFmiDrawParamList>* copyFromList, NFmiPtrList<NFmiDrawParamList>* copyToList);
    static bool IsBorderLayerDrawn(const NFmiDrawParam* separateBorderLayerDrawOptions);
    static std::string getNoneOverlayName();
};
