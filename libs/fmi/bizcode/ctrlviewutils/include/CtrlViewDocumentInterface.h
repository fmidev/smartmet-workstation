#pragma once

#include "MapViewMode.h"
#include "NFmiPoint.h"
#include "NFmiDataMatrix.h"
#include "NFmiLevel.h"
#include "FmiSmartMetEditingMode.h"
#include "NFmiMetEditorTypes.h"
#include "FmiEditorModifyToolMode.h"
#include "NFmiProducer.h"
#include "NFmiInfoData.h"
#include "SmartMetViewId.h"
#include "CombinedMapHandlerInterface.h"

#include <functional>
#include <list>
#include <boost/shared_ptr.hpp>

class NFmiAreaMaskList;
class NFmiInfoOrganizer;
class NFmiArea;
class NFmiDrawParamList;
class NFmiColor;
class NFmiProducerSystem;
class NFmiMenuItem;
class NFmiDrawParam;
class NFmiMetTime;
class NFmiAnimationData;
class NFmiTrajectorySystem;
class NFmiFastQueryInfo;
class NFmiMTATempSystem;
class MapHandlerInterface;
class NFmiDataLoadingInfo;
class NFmiModelDataBlender;
class NFmiMilliSecondTimer;
class NFmiTimeDescriptor;
class NFmiTimeBag;
class NFmiCrossSectionSystem;
class NFmiHelpEditorSystem;
class NFmiMetEditorOptionsData;
class NFmiEditorControlPointManager;
class NFmiLocation;
class NFmiAnalyzeToolData;
class NFmiIgnoreStationsData;
class NFmiMacroParamSystem;
class TimeSerialModificationDataInterface;
class NFmiObsComparisonInfo;
class NFmiLocationSelectionTool;
class NFmiEditMapDataListHandler;
class NFmiSynopPlotSettings;
class NFmiSynopStationPrioritySystem;
class NFmiConceptualModelData;
class NFmiSmartToolInfo;
class NFmiSatelliteImageCacheSystem;
class CWnd;
class NFmiGridPointCache;
class NFmiMapViewCache;
class CDC;
class NFmiWindTableSystem;
class NFmiProjectionCurvatureInfo;
class NFmiPolyline;
class NFmiDataIdent;
class NFmiAutoComplete;
class NFmiMapViewTimeLabelInfo;
class NFmiBetaProduct;
class NFmiHelpDataInfoSystem;
class NFmiVPlaceDescriptor;
class NFmiBetaProductionSystem;
class NFmiApplicationWinRegistry;
class Q2ServerInfo;
class CBitmap;
class NFmiMacroParamDataCache;
class NFmiParam;
class TimeSerialParameters;
class NFmiColorContourLegendSettings;
class NFmiFixedDrawParamSystem;
class NFmiMacroPathSettings;
class WmsSupportInterface;
class CombinedMapHandlerInterface;
class TrueMapViewSizeInfo;
class NFmiSeaLevelPlumeData;
class NFmiMouseClickUrlActionData;

namespace Imagine
{
    class NFmiPath;
}
namespace HakeMessage
{
    class Main;
}
namespace Gdiplus
{
    class Bitmap;
}
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

namespace CatLog
{
    enum class Severity;
    enum class Category;
}

namespace Warnings
{
    class CapDataSystem;
}


// Interface that is meant to be used by non-MFC views in libraries under bizcode part of code base.
class CtrlViewDocumentInterface
{
public:
    using GetCtrlViewDocumentInterfaceImplementationCallBackType = std::function<CtrlViewDocumentInterface*(void)>;
    // Tämä pitää asettaa johonkin konkreettiseen funktioon, jotta käyttäjä koodi saa käyttöönsä halutun interface toteutuksen
    static GetCtrlViewDocumentInterfaceImplementationCallBackType GetCtrlViewDocumentInterfaceImplementation;

	virtual ~CtrlViewDocumentInterface() = default;

    virtual CtrlViewUtils::MapViewMode MapViewDisplayMode(int theMapViewDescTopIndex) = 0;
    virtual const NFmiPoint& ViewGridSize(int theMapViewDescTopIndex) = 0;
    virtual int MapRowStartingIndex(int theMapViewDescTopIndex) = 0;
    virtual CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(int theMapViewDescTopIndex) = 0;
    virtual bool CreateMaskSelectionPopup(int theRowIndex) = 0;
    virtual boost::shared_ptr<NFmiAreaMaskList> ParamMaskListMT() = 0;
    virtual bool CreateMaskParamsPopup(int theRowIndex, int theParamIndex) = 0;
    virtual bool CreateParamSelectionPopup(unsigned int theDescTopIndex, int theRowIndex) = 0;
    virtual void ActivateParamSelectionDlgAfterLeftDoubleClick(bool newValue) = 0;
    virtual bool ScrollViewRow(unsigned int theDescTopIndex, int theCount) = 0;
    virtual const std::string& HelpDataPath(void) const = 0;
	virtual const std::string& ControlPath(void) const = 0;
    virtual void LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption = false, bool flushLogger = false) = 0;
    virtual NFmiInfoOrganizer* InfoOrganizer(void) = 0;
    virtual bool IsToolMasterAvailable(void) = 0;
    virtual NFmiDrawParamList* DrawParamList(int theDescTopIndex, int theIndex) = 0;
    virtual NFmiDrawParamList* TimeSerialViewDrawParamList(void) = 0;
    virtual const NFmiColor& HelpColor(void) const = 0;
    virtual NFmiProducerSystem& ProducerSystem(void) = 0;
    virtual NFmiProducerSystem& ObsProducerSystem(void) = 0;
    virtual FmiLanguage Language() = 0;
    virtual const NFmiPoint& CrossSectionViewSizeInPixels(void) = 0;
    virtual const NFmiPoint& MapViewSizeInPixels(int theMapViewDescTopIndex) = 0;
    virtual bool Printing() = 0;
    virtual bool CreateMapViewTimeBoxPopup(unsigned int theDescTopIndex) = 0;
    virtual bool CreateViewParamsPopup(unsigned int theDescTopIndex, int theRowIndex, int layerIndex, double layerIndexRealValue) = 0;
    virtual void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty = false, bool fClearCache = false, int theWantedMapViewDescTop = -1) = 0;
    virtual void RefreshApplicationViewsAndDialogs(const std::string& reasonForUpdate, SmartMetViewId updatedViewsFlag, bool redrawMapView = false, bool clearMapViewBitmapCacheRows = false, int theWantedMapViewDescTop = -1) = 0;
    virtual bool ExecuteCommand(const NFmiMenuItem &theMenuItem, int theViewIndex, int theViewTypeId) = 0;
    virtual bool ChangeParamSettingsToNextFixedDrawParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext) = 0;
    virtual bool ChangeActiveMapViewParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams) = 0;
    virtual bool MoveActiveMapViewParamInDrawingOrderList(unsigned int theDescTopIndex, int theMapRow, bool fRaise, bool fUseCrossSectionParams) = 0;
    virtual void CheckAnimationLockedModeTimeBags(unsigned int theDescTopIndex, bool ignoreSatelImages) = 0;
    virtual bool MouseCaptured(void) = 0;
    virtual void MouseCaptured(bool newValue) = 0;
    virtual void SetModelRunOffset(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theMoveByValue, unsigned int theDescTopIndex, int theViewRowIndex) = 0;
    virtual bool SetNearestBeforeModelOrigTimeRunoff(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiMetTime &theTime, unsigned int theDescTopIndex, int theViewRowIndex) = 0;
    virtual const NFmiMetTime& CurrentTime(unsigned int theDescTopIndex) = 0;
    virtual bool CurrentTime(unsigned int theDescTopIndex, const NFmiMetTime& newCurrentTime, bool fStayInsideAnimationTimes = false) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> DefaultEditedDrawParam(void) = 0;
    virtual NFmiAnimationData& AnimationData(int theMapViewDescTopIndex) = 0;
    virtual NFmiTrajectorySystem* TrajectorySystem(void) = 0;
    virtual const NFmiColor& GeneralColor(int theIndex) = 0;
    virtual const NFmiMetTime& ActiveMapTime(void) = 0;
    virtual void MakeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea) = 0;
    virtual NFmiMetTime GetModelOrigTime(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex = 0) = 0;
    virtual void UpdateCrossSectionMacroParamDataSize(void) = 0;
    virtual NFmiMTATempSystem& GetMTATempSystem(void) = 0;
    virtual int SoundingViewWindBarbSpaceOutFactor() = 0;
    virtual std::unique_ptr<MapHandlerInterface> GetMapHandlerInterface(int theMapViewDescTopIndex) = 0;
    virtual bool KeepMapAspectRatio() = 0;
    virtual double ClientViewXperYRatio(int theMapViewDescTopIndex) = 0;
    virtual const NFmiRect& AreaFilterRangeLimits(void) = 0;
    virtual void AreaFilterRangeLimits(const NFmiRect& theRect) = 0;
    virtual const NFmiRect& AreaFilterRange(int index) = 0;
    virtual void AreaFilterRange(int index, const NFmiRect& theRect) = 0;
    virtual boost::shared_ptr<NFmiFastQueryInfo> EditedSmartInfo(void) = 0;
    virtual NFmiDataLoadingInfo& GetUsedDataLoadingInfo(void) = 0;
    virtual NFmiModelDataBlender& ModelDataBlender(void) = 0;
    virtual const NFmiPoint& TimeFilterLimits(void) = 0;
    virtual const NFmiPoint& TimeFilterRange(int index) = 0;
    virtual void TimeFilterRange(int index, const NFmiPoint& thePoint, bool fRoundToNearestHour) = 0;
    virtual CtrlViewUtils::FmiSmartMetEditingMode SmartMetEditingMode(void) = 0;
    virtual bool EditedDataNeedsToBeLoaded() = 0;
    virtual NFmiMilliSecondTimer& EditedDataNeedsToBeLoadedTimer(void) = 0;
    virtual const NFmiTimeDescriptor& TimeControlViewTimes(unsigned int theDescTopIndex) = 0;
    virtual void TimeControlViewTimes(unsigned int theDescTopIndex, const NFmiTimeDescriptor &newTimeDescriptor) = 0;
    virtual const NFmiMetTime& TimeFilterStartTime(void) = 0;
    virtual const NFmiMetTime& TimeFilterEndTime(void) = 0;
    virtual void SetTimeFilterStartTime(const NFmiMetTime& theTime) = 0;
    virtual void SetTimeFilterEndTime(const NFmiMetTime& theTime) = 0;
    virtual void TimeControlTimeStep(int theMapViewDescTopIndex, float timeStepInHours) = 0;
    virtual float TimeControlTimeStep(int theDescTopIndex) = 0;
    virtual long TimeControlTimeStepInMinutes(int theDescTopIndex) const = 0;
    virtual void MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers) = 0;
    virtual void ForceStationViewRowUpdate(unsigned int theDescTopIndex, unsigned int theRealRowIndex) = 0;
    virtual bool SetDataToNextTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes = false) = 0;
    virtual bool SetDataToPreviousTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes = false) = 0;
    virtual void TimeSerialViewTimeBag(const NFmiTimeBag &theTimeBag) = 0;
    virtual const NFmiTimeBag& TimeSerialViewTimeBag(void) const = 0;
    virtual bool MouseCapturedInTimeWindow(void) = 0;
    virtual void MouseCapturedInTimeWindow(bool newValue) = 0;
    virtual const NFmiPoint& TimeSerialViewSizeInPixels(void) const = 0;
    virtual void TimeSerialViewSizeInPixels(const NFmiPoint &newValue) = 0;
    virtual int FilterDialogUpdateStatus(void) = 0;
    virtual void FilterDialogUpdateStatus(int newState) = 0;
    virtual void ResetTimeFilterTimes() = 0;
    virtual NFmiCrossSectionSystem* CrossSectionSystem(void) = 0;
    virtual bool IsOperationalModeOn(void) = 0;
    virtual bool ShowHelperData1InTimeSerialView() = 0;
    virtual bool ShowHelperData2InTimeSerialView() = 0;
    virtual bool ShowHelperData3InTimeSerialView() = 0;
    virtual bool ShowHelperData4InTimeSerialView() = 0;
    virtual bool AllowRightClickDisplaySelection(void) = 0;
    virtual void AllowRightClickDisplaySelection(bool newValue) = 0;
    virtual NFmiHelpEditorSystem& HelpEditorSystem(void) = 0;
    virtual NFmiMetEditorOptionsData& MetEditorOptionsData(void) = 0;
    virtual boost::shared_ptr<NFmiEditorControlPointManager> CPManager(bool getOldSchoolCPManager = false) = 0;
    virtual boost::shared_ptr<NFmiFastQueryInfo> GetNearestSynopStationInfo(const NFmiLocation &theLocation, const NFmiMetTime &theTime, bool ignoreTime, std::vector<boost::shared_ptr<NFmiFastQueryInfo> > *thePossibleInfoVector, double maxDistanceInMeters = 1000. * kFloatMissing) = 0;
    virtual bool IsMasksUsedInTimeSerialViews(void) = 0;
    virtual NFmiAnalyzeToolData& AnalyzeToolData(void) = 0;
    virtual const NFmiPoint& OutOfEditedAreaTimeSerialPoint(void) const = 0;
    virtual const NFmiPoint& PreciseTimeSerialLatlonPoint() const = 0;
    virtual bool IsPreciseTimeSerialLatlonPointUsed() = 0;
    virtual bool TimeSerialViewDirty(void) = 0;
    virtual void TimeSerialViewDirty(bool newValue) = 0;
    virtual bool CreateTimeSerialDialogPopup(int index) = 0;
    virtual bool CreateTimeSerialDialogOnViewPopup(int index) = 0;
    virtual bool DoTimeSeriesValuesModifying(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, int theUsedMask, NFmiTimeDescriptor& theTimeDescriptor, std::vector<float> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue = -1) = 0;
    virtual void UpdateToModifiedDrawParam(unsigned int mapViewDescTopIndex, boost::shared_ptr<NFmiDrawParam>& drawParam, int viewRowIndex) = 0;
    virtual bool UseTimeSerialAxisAutoAdjust(void) = 0;
    virtual bool UseQ2Server(void) = 0;
    virtual bool Registry_ShowLastSendTimeOnMapView() = 0;
    virtual double Registry_MaximumFontSizeFactor() = 0;
    virtual bool Registry_ShowStationPlot(int theMapViewDescTopIndex) = 0;
    virtual int Registry_SpacingOutFactor(int theMapViewDescTopIndex) = 0;
    virtual bool Registry_ShowMasksOnMap(int theMapViewDescTopIndex) = 0;
    virtual NFmiIgnoreStationsData& IgnoreStationsData(void) = 0;
    virtual const NFmiColor& StationPointColor(int theMapViewDescTopIndex) const = 0;
    virtual const NFmiPoint& StationPointSize(int theMapViewDescTopIndex) const = 0;
    virtual std::shared_ptr<NFmiMacroParamSystem> MacroParamSystem(void) = 0;
    virtual CtrlViewUtils::FmiEditorModifyToolMode ModifyToolMode(void) = 0;
    virtual size_t SelectedGridPointLimit(void) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> GetDrawDifferenceDrawParam(void) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> GetSelectedGridPointDrawParam(void) = 0;
    virtual bool DrawSelectionOnThisView(void) = 0;
    virtual void DrawSelectionOnThisView(bool newValue) = 0;
    virtual const NFmiPoint& ToolTipLatLonPoint(void) const = 0;
    virtual void ToolTipLatLonPoint(const NFmiPoint& theLatLon) = 0;
    virtual void ToolTipTime(const NFmiMetTime& theTime) = 0;
    virtual const NFmiMetTime& ToolTipTime(void) = 0;
    virtual TimeSerialModificationDataInterface& GenDocDataAdapter(void) = 0;
    virtual bool UseMultithreaddingWithModifyingFunctions(void) = 0;
    virtual std::string GetModelOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex = 0) = 0;
    virtual NFmiObsComparisonInfo& ObsComparisonInfo(void) = 0;
    virtual void GetDataFromQ2Server(const std::string &theURLStr, const std::string &theParamsStr, bool fUseBinaryData, 
        int theUsedCompression, NFmiDataMatrix<float> &theDataMatrixOut, std::string &theExtraInfoStrOut) = 0;
    virtual void SetMacroErrorText(const std::string &theErrorStr) = 0;
    virtual const NFmiPoint& StationDataGridSize(void) = 0;
    virtual NFmiLocationSelectionTool* LocationSelectionTool2(void) = 0;
    virtual void SelectLocations(unsigned int theDescTopIndex, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const boost::shared_ptr<NFmiArea> &theMapArea,
        const NFmiPoint& theLatLon , const NFmiMetTime &theTime, int theSelectionCombineFunction, unsigned long theMask
        , bool fMakeMTAModeAdd, bool fDoOnlyMTAModeAdd) = 0;
    virtual NFmiEditMapDataListHandler* DataLists(void) = 0;
    virtual bool ShowObsComparisonOnMap(int theDescTopIndex) = 0;
    virtual std::vector<boost::shared_ptr<NFmiFastQueryInfo> > GetSortedSynopInfoVector(int theProducerId, int theProducerId2 = -1, int theProducerId3 = -1, int theProducerId4 = -1) = 0;
    virtual int AbsoluteActiveViewRow(unsigned int theDescTopIndex) = 0;
    virtual void AbsoluteActiveViewRow(unsigned int theDescTopIndex, int theAbsoluteActiveRowIndex) = 0;
    virtual NFmiSynopPlotSettings* SynopPlotSettings(void) = 0;
    virtual NFmiSynopStationPrioritySystem* SynopStationPrioritySystem(void) = 0;
    virtual NFmiPoint ActualMapBitmapSizeInPixels(unsigned int theDescTopIndex) = 0;
    virtual NFmiConceptualModelData& ConceptualModelData(void) = 0;
    virtual void MakeHTTPRequest(const std::string &theUrlStr, std::string &theResponseStrOut, bool fDoGet) = 0;
    virtual bool LeftMouseButtonDown(void) = 0;
    virtual void LeftMouseButtonDown(bool newState) = 0;
    virtual bool RightMouseButtonDown(void) = 0;
    virtual void RightMouseButtonDown(bool newState) = 0;
    virtual bool MiddleMouseButtonDown(void) = 0;
    virtual void MiddleMouseButtonDown(bool newState) = 0;
    virtual NFmiDrawParamList* CrossSectionViewDrawParamList(int theRowIndex) = 0;
    virtual NFmiSmartToolInfo* SmartToolInfo(void) = 0;
    virtual void SetLatestMacroParamErrorText(const std::string& theErrorText) = 0;
    virtual bool CreateCrossSectionViewPopup(int theRowIndex) = 0;
    virtual const NFmiRect& RelativeMapRect(int theDescTopIndex) = 0;
    virtual bool BetaProductGenerationRunning() = 0;
    virtual const NFmiMetTime& LastEditedDataSendTime() = 0;
    virtual bool IsLastEditedDataSendHasComeBack() = 0;
    virtual bool DataModificationInProgress(void) = 0;
    virtual void ActiveMapDescTopIndex(unsigned int newValue) = 0;
    virtual const NFmiRect& UpdateRect(void) = 0;
    virtual void UpdateRect(const NFmiRect& theRect) = 0;
    virtual int ToolTipRealRowIndex(void) = 0;
    virtual void ToolTipRealRowIndex(int newRealRowIndex) = 0;
    virtual void DoAutoZoom(unsigned int theDescTopIndex) = 0;
    virtual bool MapMouseDragPanMode(void) = 0;
    virtual void MapMouseDragPanMode(bool newState) = 0;
    virtual void MapViewBitmapDirty(int theDescTopIndex, bool dirtyFlag) = 0;
    virtual NFmiSatelliteImageCacheSystem& SatelliteImageCacheSystem() = 0;
    virtual CWnd* TransparencyContourDrawView(void) = 0;
    virtual NFmiGridPointCache& GridPointCache(int theDescTopIndex) = 0;
    virtual NFmiMapViewCache& MapViewCache(int theDescTopIndex) = 0;
    virtual CDC* CopyCDC(int theDescTopIndex) = 0;
    virtual bool IsCPGridCropInAction(void) = 0;
    virtual bool ShowSoundingMarkersOnMap(int theDescTopIndex) = 0;
    virtual bool ShowCrossSectionMarkersOnMap(int theDescTopIndex) = 0;
    virtual bool ShowTrajectorsOnMap(int theDescTopIndex) = 0;
    virtual std::vector<NFmiProducer>& ExtraSoundingProducerList(void) = 0;
    virtual bool ShowWarningMarkersOnMap(int theDescTopIndex) = 0;
    virtual CDC* MapBlitDC(int theDescTopIndex) = 0;
    virtual int ToolTipColumnIndex() const = 0;
    virtual void ToolTipColumnIndex(int newIndex) = 0;
    virtual int ToolTipMapViewDescTopIndex() const = 0;
    virtual void ToolTipMapViewDescTopIndex(int newIndex) = 0;
    virtual bool HasActiveViewChanged(void) = 0;
    virtual void LastBrushedViewTime(const NFmiMetTime& newTime) = 0;
    virtual void LastBrushedViewRealRowIndex(int newRealRowIndex) = 0;
    virtual const NFmiTimeBag& EditedDataTimeBag(void) = 0;
    virtual const boost::shared_ptr<NFmiArea> CPGridCropLatlonArea(void) = 0;
    virtual bool IsCPGridCropNotPlausible(void) = 0;
    virtual boost::shared_ptr<NFmiArea> CPGridCropInnerLatlonArea(void) = 0;
    virtual NFmiWindTableSystem& WindTableSystem(void) = 0;
    virtual NFmiProjectionCurvatureInfo* ProjectionCurvatureInfo(void) = 0;
    virtual bool DrawLandBorders(int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions) = 0;
    virtual const NFmiColor& LandBorderColor(int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions) = 0;
    virtual int LandBorderPenSize(int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions) = 0;
    virtual Gdiplus::Bitmap* LandBorderMapBitmap(unsigned int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions) const = 0;
    virtual void SetLandBorderMapBitmap(unsigned int theDescTopIndex, Gdiplus::Bitmap *newBitmap, NFmiDrawParam* separateBorderLayerDrawOptions) = 0;
    virtual boost::shared_ptr<Imagine::NFmiPath> LandBorderPath(int theDescTopIndex) = 0;
    virtual void DrawBorderPolyLineList(int theDescTopIndex, std::list<NFmiPolyline*> &polyLineList) = 0;
    virtual std::list<NFmiPolyline*>& DrawBorderPolyLineList(int theDescTopIndex) = 0;
    virtual const std::list<std::vector<NFmiPoint>>& DrawBorderPolyLineListGdiplus(int theDescTopIndex) = 0;
    virtual void DrawBorderPolyLineListGdiplus(int theDescTopIndex, const std::list<std::vector<NFmiPoint>> &newValue) = 0;
    virtual void DrawBorderPolyLineListGdiplus(int theDescTopIndex, std::list<std::vector<NFmiPoint>> &&newValue) = 0;
    virtual int DrawOverMapMode(int theDescTopIndex) = 0;
    virtual void SnapShotData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const std::string &theModificationText
        , const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime) = 0;
    virtual bool ShowWaitCursorWhileDrawingView(void) = 0;
    virtual NFmiAutoComplete& AutoComplete(void) = 0;
    virtual bool ShowMouseHelpCursorsOnMap(void) = 0;
    virtual bool SynopDataGridViewOn(void) = 0;
    virtual bool ShowSynopHighlight(void) = 0;
    virtual const NFmiPoint& GetSynopHighlightLatlon(void) = 0;
    virtual bool ShowTimeString(int theDescTopIndex) = 0;
    virtual NFmiMapViewTimeLabelInfo& MapViewTimeLabelInfo() = 0;
    virtual const NFmiBetaProduct* GetCurrentGeneratedBetaProduct() = 0;
    virtual const NFmiMetTime& ActiveViewTime(void) = 0;
    virtual void ActiveViewTime(const NFmiMetTime& theTime) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> ActiveDrawParamFromActiveRow(unsigned int theDescTopIndex) = 0;
    virtual bool ViewBrushed(void) = 0;
    virtual void ViewBrushed(bool newState) = 0;
    virtual bool CheckAndValidateAfterModifications(NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam = kFmiLastParameter) = 0;
    virtual void ZoomMapInOrOut(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, double theZoomFactor) = 0;
    virtual bool UseMaskWithBrush(void) = 0;
    virtual int BrushToolLimitSetting(void) = 0;
    virtual float BrushToolLimitSettingValue(void) = 0;
    virtual void MustDrawCrossSectionView(bool newValue) = 0;
    virtual void MustDrawTempView(bool newValue) = 0;
    virtual bool MustDrawTempView() = 0;
    virtual bool TimeSerialDataViewOn(void) = 0;
    virtual void MustDrawTimeSerialView(bool newValue) = 0;
    virtual bool ShowSelectedPointsOnMap(int theMapViewDescTopIndex) = 0;
    virtual bool ShowControlPointsOnMap(int theMapViewDescTopIndex) = 0;
    virtual void ZoomMapWithRelativeRect(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiRect &theZoomedRect) = 0;
    virtual FmiDirection ParamWindowViewPosition(int theMapViewDescTopIndex) = 0;
    virtual bool IsParamWindowViewVisible(int theMapViewDescTopIndex) = 0;
    virtual NFmiHelpDataInfoSystem* HelpDataInfoSystem(void) = 0;
    virtual bool CreateCPPopup() = 0;
    virtual void PanMap(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, const NFmiPoint &theZoomDragUpPoint) = 0;
    virtual void ActiveViewRect(const NFmiRect& theRect) = 0;
    virtual const NFmiPoint& BrushSize(void) = 0;
    virtual double BrushSpecialParamValue(void) = 0;
    virtual const NFmiVPlaceDescriptor& SoundingPlotLevels(void) = 0;
    virtual double BrushValue(void) = 0;
    virtual boost::shared_ptr<NFmiFastQueryInfo> GetModelClimatologyData(const NFmiLevel& theLevel) = 0;
    virtual boost::shared_ptr<NFmiFastQueryInfo> GetBestSuitableModelFractileData(boost::shared_ptr<NFmiFastQueryInfo>& usedOriginalInfo) = 0;
    virtual boost::shared_ptr<NFmiFastQueryInfo> GetMosTemperatureMinAndMaxData() = 0;
    virtual bool UseCombinedMapMode() const = 0;
    virtual void UseCombinedMapMode(bool newValue) = 0;
    virtual NFmiBetaProductionSystem& BetaProductionSystem() = 0;
    virtual void SetLastActiveDescTopAndViewRow(unsigned int theDescTopIndex, int theActiveRowIndex) = 0;
    virtual NFmiApplicationWinRegistry& ApplicationWinRegistry() = 0;
    virtual Q2ServerInfo& GetQ2ServerInfo() = 0;
    virtual Warnings::CapDataSystem& GetCapDataSystem() = 0;
    virtual int GetTimeRangeForWarningMessagesOnMapViewInMinutes() = 0;
    virtual NFmiMacroParamDataCache& MacroParamDataCache() = 0;
    virtual bool SetupObsBlenderData(const NFmiPoint &theLatlon, const NFmiParam &theParam, NFmiInfoData::Type theDataType, bool fGroundData, const NFmiProducer &theProducer, NFmiMetTime &firstEditedTimeOut, boost::shared_ptr<NFmiFastQueryInfo> &usedObsBlenderInfoOut, float &analyzeValueOut, std::vector<std::string> &messagesOut) = 0;
    virtual TimeSerialParameters& GetTimeSerialParameters() = 0;
    virtual void UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(int theMapViewDescTopIndex) = 0;
    virtual NFmiColorContourLegendSettings& ColorContourLegendSettings() = 0;
    virtual void SetPrintedDescTopIndex(int nowPrintedDescTopIndex) = 0;
    virtual int GetPrintedDescTopIndex() = 0;
    virtual void ResetPrintedDescTopIndex() = 0;
    virtual unsigned int SelectedMapIndex(int mapViewDescTopIndex) = 0;
    virtual void SetCPCropGridSettings(const boost::shared_ptr<NFmiArea>& newArea, unsigned int mapViewDescTopIndex) = 0;
    virtual NFmiFixedDrawParamSystem& FixedDrawParamSystem() = 0;
    virtual void ApplyFixeDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex, const std::shared_ptr<NFmiDrawParam>& theFixedDrawParam) = 0;
    virtual NFmiMacroPathSettings& MacroPathSettings() = 0;
    virtual int CurrentCrossSectionRowIndex() = 0;
    virtual CombinedMapHandlerInterface& GetCombinedMapHandlerInterface() = 0;
    virtual bool BorderDrawBitmapDirty(int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions) const = 0;
    virtual bool BorderDrawPolylinesDirty(int theDescTopIndex) const = 0;
    virtual bool BorderDrawPolylinesGdiplusDirty(int theDescTopIndex) const = 0;
    virtual void SetBorderDrawDirtyState(int theDescTopIndex, CountryBorderDrawDirtyState newState) = 0;
    virtual double SingleMapViewHeightInMilliMeters(int theDescTopIndex) const = 0;
    virtual bool IsTimeControlViewVisible(int theDescTopIndex) const = 0;
    virtual TrueMapViewSizeInfo&  GetTrueMapViewSizeInfo(int theDescTopIndex) const = 0;
    virtual NFmiSeaLevelPlumeData& SeaLevelPlumeData() const = 0;
    virtual NFmiMouseClickUrlActionData& MouseClickUrlActionData() const = 0;
    virtual void VirtualTime(const NFmiMetTime& virtualTime) = 0;
    virtual const NFmiMetTime& VirtualTime() const = 0;
    virtual bool VirtualTimeUsed() const = 0;
    virtual void ToggleVirtualTimeMode(const std::string& logMessage) = 0;
    virtual std::string GetVirtualTimeTooltipText() const = 0;
    virtual void UpdateMacroParamSystemContent(std::shared_ptr<NFmiMacroParamSystem> updatedMacroParamSystemPtr) = 0;

#ifndef DISABLE_CPPRESTSDK
    virtual HakeMessage::Main& WarningCenterSystem(void) = 0;
    virtual std::shared_ptr<WmsSupportInterface> GetWmsSupport() const = 0;
#endif // DISABLE_CPPRESTSDK
};
