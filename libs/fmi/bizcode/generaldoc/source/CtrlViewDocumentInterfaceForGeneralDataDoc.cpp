#include "CtrlViewDocumentInterfaceForGeneralDataDoc.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiHelpEditorSystem.h"
#include "GraphicalInfo.h"
#include "GdiPlusMapHandlerInterface.h"
#include "NFmiApplicationWinRegistry.h"
#include "TimeSerialModification.h"
#include "ApplicationInterface.h"
#include "CombinedMapHandlerInterface.h"
#include "SpecialDesctopIndex.h"
#include "NFmiCrossSectionSystem.h"

CtrlViewDocumentInterfaceForGeneralDataDoc::CtrlViewDocumentInterfaceForGeneralDataDoc(NFmiEditMapGeneralDataDoc *theDoc)
    :itsDoc(theDoc)
{
}

CtrlViewUtils::MapViewMode CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewDisplayMode(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->MapViewDisplayMode();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::ViewGridSize(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->ViewGridSize();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::MapRowStartingIndex(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->MapRowStartingIndex();
}

CtrlViewUtils::GraphicalInfo& CtrlViewDocumentInterfaceForGeneralDataDoc::GetGraphicalInfo(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getGraphicalInfo(theMapViewDescTopIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateMaskSelectionPopup(int theRowIndex)
{
    return itsDoc->CreateMaskSelectionPopup(theRowIndex);
}

boost::shared_ptr<NFmiAreaMaskList> CtrlViewDocumentInterfaceForGeneralDataDoc::ParamMaskListMT()
{
    return itsDoc->ParamMaskListMT();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateMaskParamsPopup(int theRowIndex, int theParamIndex)
{
    return itsDoc->CreateMaskParamsPopup(theRowIndex, theParamIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateParamSelectionPopup(unsigned int theDescTopIndex, int theRowIndex)
{
    return itsDoc->CreateParamSelectionPopup(theDescTopIndex, theRowIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ActivateParamSelectionDlgAfterLeftDoubleClick(bool newValue)
{
    itsDoc->ActivateParamSelectionDlgAfterLeftDoubleClick(newValue);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ScrollViewRow(unsigned int theDescTopIndex, int theCount)
{
    return itsDoc->GetCombinedMapHandler()->scrollViewRow(theDescTopIndex, theCount);
}

const std::string& CtrlViewDocumentInterfaceForGeneralDataDoc::HelpDataPath(void) const
{
    return itsDoc->HelpDataPath();
}

const std::string& CtrlViewDocumentInterfaceForGeneralDataDoc::ControlPath(void) const
{
	return itsDoc->ControlDirectory();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption, bool flushLogger)
{
    itsDoc->LogAndWarnUser(theMessageStr, theDialogTitleStr, severity, category, justLog, addAbortOption, flushLogger);
}

NFmiInfoOrganizer* CtrlViewDocumentInterfaceForGeneralDataDoc::InfoOrganizer(void)
{
    return itsDoc->InfoOrganizer();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsToolMasterAvailable(void)
{
    return itsDoc->IsToolMasterAvailable();
}

NFmiDrawParamList* CtrlViewDocumentInterfaceForGeneralDataDoc::DrawParamList(int theDescTopIndex, int theIndex)
{
    return itsDoc->GetCombinedMapHandler()->getDrawParamList(theDescTopIndex, theIndex);
}

NFmiDrawParamList* CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewDrawParamList(void)
{
    return &itsDoc->GetCombinedMapHandler()->getTimeSerialViewDrawParamList();
}

const NFmiColor& CtrlViewDocumentInterfaceForGeneralDataDoc::HelpColor(void) const
{ 
    return itsDoc->HelpEditorSystem().HelpColor(); 
}

NFmiProducerSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::ProducerSystem(void)
{
    return itsDoc->ProducerSystem();
}

NFmiProducerSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::ObsProducerSystem(void)
{
    return itsDoc->ObsProducerSystem();
}

FmiLanguage CtrlViewDocumentInterfaceForGeneralDataDoc::Language()
{
    return itsDoc->Language();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::CrossSectionViewSizeInPixels(void)
{
    return itsDoc->CrossSectionViewSizeInPixels();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewSizeInPixels(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->MapViewSizeInPixels();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::Printing()
{
    return itsDoc->Printing();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateMapViewTimeBoxPopup(unsigned int theDescTopIndex)
{
    return itsDoc->CreateMapViewTimeBoxPopup(theDescTopIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateViewParamsPopup(unsigned int theDescTopIndex, int theRowIndex, int layerIndex, double layerIndexRealValue)
{
    return itsDoc->CreateViewParamsPopup(theDescTopIndex, theRowIndex, layerIndex, layerIndexRealValue);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty, bool fClearCache, int theWantedMapViewDescTop)
{
    itsDoc->RefreshApplicationViewsAndDialogs(reasonForUpdate, fMakeAreaViewDirty, fClearCache, theWantedMapViewDescTop);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::RefreshApplicationViewsAndDialogs(const std::string& reasonForUpdate, SmartMetViewId updatedViewsFlag, bool redrawMapView, bool clearMapViewBitmapCacheRows, int theWantedMapViewDescTop)
{
    ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs(reasonForUpdate, updatedViewsFlag, redrawMapView, clearMapViewBitmapCacheRows, theWantedMapViewDescTop);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ExecuteCommand(const NFmiMenuItem &theMenuItem, int theViewIndex, int theViewTypeId)
{
    return itsDoc->ExecuteCommand(theMenuItem, theViewIndex, theViewTypeId);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ChangeParamSettingsToNextFixedDrawParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext)
{
    return itsDoc->GetCombinedMapHandler()->changeParamSettingsToNextFixedDrawParam(theDescTopIndex, theMapRow, theParamIndex, fNext);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ChangeActiveMapViewParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams)
{
    return itsDoc->GetCombinedMapHandler()->changeActiveMapViewParam(theDescTopIndex, theMapRow, theParamIndex, fNext, fUseCrossSectionParams);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::MoveActiveMapViewParamInDrawingOrderList(unsigned int theDescTopIndex, int theMapRow, bool fRaise, bool fUseCrossSectionParams)
{
    return itsDoc->GetCombinedMapHandler()->moveActiveMapViewParamInDrawingOrderList(theDescTopIndex, theMapRow, fRaise, fUseCrossSectionParams);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::CheckAnimationLockedModeTimeBags(unsigned int theDescTopIndex, bool ignoreSatelImages)
{
    itsDoc->GetCombinedMapHandler()->checkAnimationLockedModeTimeBags(theDescTopIndex, ignoreSatelImages);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::MouseCaptured(void)
{
    return itsDoc->MouseCaptured();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MouseCaptured(bool newValue)
{
    itsDoc->MouseCaptured(newValue);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetModelRunOffset(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theMoveByValue, unsigned int theDescTopIndex, int theViewRowIndex)
{
    itsDoc->SetModelRunOffset(theDrawParam, theMoveByValue, theDescTopIndex, theViewRowIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::SetNearestBeforeModelOrigTimeRunoff(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiMetTime &theTime, unsigned int theDescTopIndex, int theViewRowIndex)
{
    return itsDoc->SetNearestBeforeModelOrigTimeRunoff(theDrawParam, theTime, theDescTopIndex, theViewRowIndex);
}

const NFmiMetTime& CtrlViewDocumentInterfaceForGeneralDataDoc::CurrentTime(unsigned int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->currentTime(theDescTopIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CurrentTime(unsigned int theDescTopIndex, const NFmiMetTime& newCurrentTime, bool fStayInsideAnimationTimes)
{
    return itsDoc->GetCombinedMapHandler()->currentTime(theDescTopIndex, newCurrentTime, fStayInsideAnimationTimes);
}

boost::shared_ptr<NFmiDrawParam> CtrlViewDocumentInterfaceForGeneralDataDoc::DefaultEditedDrawParam(void)
{
    return itsDoc->DefaultEditedDrawParam();
}

NFmiAnimationData& CtrlViewDocumentInterfaceForGeneralDataDoc::AnimationData(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->AnimationDataRef();
}

NFmiTrajectorySystem* CtrlViewDocumentInterfaceForGeneralDataDoc::TrajectorySystem(void)
{
    return itsDoc->TrajectorySystem();
}

const NFmiColor& CtrlViewDocumentInterfaceForGeneralDataDoc::GeneralColor(int theIndex)
{
    return itsDoc->GeneralColor(theIndex);
}

const NFmiMetTime& CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveMapTime(void)
{
    return itsDoc->GetCombinedMapHandler()->activeMapTime();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MakeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea)
{
    itsDoc->GetCombinedMapHandler()->makeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
}

NFmiMetTime CtrlViewDocumentInterfaceForGeneralDataDoc::GetModelOrigTime(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex)
{
    return itsDoc->GetModelOrigTime(theDrawParam, theIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::UpdateCrossSectionMacroParamDataSize(void)
{
    itsDoc->UpdateCrossSectionMacroParamDataSize();
}

NFmiMTATempSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::GetMTATempSystem(void)
{
    return itsDoc->GetMTATempSystem();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::SoundingViewWindBarbSpaceOutFactor()
{
    return itsDoc->SoundingViewWindBarbSpaceOutFactor();
}

std::unique_ptr<MapHandlerInterface> CtrlViewDocumentInterfaceForGeneralDataDoc::GetMapHandlerInterface(int theMapViewDescTopIndex)
{
    return std::make_unique<GdiPlusMapHandlerInterface>(itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->MapHandler());
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::KeepMapAspectRatio()
{
    return itsDoc->ApplicationWinRegistry().KeepMapAspectRatio();
}

double CtrlViewDocumentInterfaceForGeneralDataDoc::ClientViewXperYRatio(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->ClientViewXperYRatio();
}

const NFmiRect& CtrlViewDocumentInterfaceForGeneralDataDoc::AreaFilterRangeLimits(void)
{
    return itsDoc->AreaFilterRangeLimits();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::AreaFilterRangeLimits(const NFmiRect& theRect)
{
    itsDoc->AreaFilterRangeLimits(theRect);
}

const NFmiRect& CtrlViewDocumentInterfaceForGeneralDataDoc::AreaFilterRange(int index)
{
    return itsDoc->AreaFilterRange(index);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::AreaFilterRange(int index, const NFmiRect& theRect)
{
    itsDoc->AreaFilterRange(index, theRect);
}

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::EditedSmartInfo(void)
{
    return itsDoc->EditedSmartInfo();
}

NFmiDataLoadingInfo& CtrlViewDocumentInterfaceForGeneralDataDoc::GetUsedDataLoadingInfo(void)
{
    return itsDoc->GetUsedDataLoadingInfo();
}

NFmiModelDataBlender& CtrlViewDocumentInterfaceForGeneralDataDoc::ModelDataBlender(void)
{
    return itsDoc->ModelDataBlender();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::TimeFilterLimits(void)
{
    return itsDoc->TimeFilterLimits();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::TimeFilterRange(int index)
{
    return itsDoc->TimeFilterRange(index);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::TimeFilterRange(int index, const NFmiPoint& thePoint, bool fRoundToNearestHour)
{
    itsDoc->TimeFilterRange(index, thePoint, fRoundToNearestHour);
}

CtrlViewUtils::FmiSmartMetEditingMode CtrlViewDocumentInterfaceForGeneralDataDoc::SmartMetEditingMode(void)
{
    return itsDoc->SmartMetEditingMode();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::EditedDataNeedsToBeLoaded()
{
    return itsDoc->EditedDataNeedsToBeLoaded();
}

NFmiMilliSecondTimer& CtrlViewDocumentInterfaceForGeneralDataDoc::EditedDataNeedsToBeLoadedTimer(void)
{
    return itsDoc->EditedDataNeedsToBeLoadedTimer();
}

const NFmiTimeDescriptor& CtrlViewDocumentInterfaceForGeneralDataDoc::TimeControlViewTimes(unsigned int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->timeControlViewTimes(theDescTopIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::TimeControlViewTimes(unsigned int theDescTopIndex, const NFmiTimeDescriptor &newTimeDescriptor)
{
    itsDoc->GetCombinedMapHandler()->timeControlViewTimes(theDescTopIndex, newTimeDescriptor);
}

const NFmiMetTime& CtrlViewDocumentInterfaceForGeneralDataDoc::TimeFilterStartTime(void)
{
    return itsDoc->TimeFilterStartTime();
}

const NFmiMetTime& CtrlViewDocumentInterfaceForGeneralDataDoc::TimeFilterEndTime(void)
{
    return itsDoc->TimeFilterEndTime();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetTimeFilterStartTime(const NFmiMetTime& theTime)
{
    itsDoc->SetTimeFilterStartTime(theTime);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetTimeFilterEndTime(const NFmiMetTime& theTime)
{
    itsDoc->SetTimeFilterEndTime(theTime);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::TimeControlTimeStep(int theMapViewDescTopIndex, float timeStepInHours)
{
    itsDoc->GetCombinedMapHandler()->timeControlTimeStep(theMapViewDescTopIndex, timeStepInHours);
}

float CtrlViewDocumentInterfaceForGeneralDataDoc::TimeControlTimeStep(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->TimeControlTimeStep();
}

long CtrlViewDocumentInterfaceForGeneralDataDoc::TimeControlTimeStepInMinutes(int theDescTopIndex) const
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->TimeControlTimeStepInMinutes();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
    itsDoc->GetCombinedMapHandler()->mapViewDirty(theDescTopIndex, makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, clearMacroParamDataCache, clearEditedDataDependentCaches, updateMapViewDrawingLayers);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ForceStationViewRowUpdate(unsigned int theDescTopIndex, unsigned int theRealRowIndex)
{
    itsDoc->GetCombinedMapHandler()->forceStationViewRowUpdate(theDescTopIndex, theRealRowIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::SetDataToNextTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
    return itsDoc->GetCombinedMapHandler()->setDataToNextTime(theDescTopIndex, fStayInsideAnimationTimes);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::SetDataToPreviousTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
    return itsDoc->GetCombinedMapHandler()->setDataToPreviousTime(theDescTopIndex, fStayInsideAnimationTimes);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewTimeBag(const NFmiTimeBag &theTimeBag)
{
    itsDoc->TimeSerialViewTimeBag(theTimeBag);
}

const NFmiTimeBag& CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewTimeBag(void) const
{
    return itsDoc->TimeSerialViewTimeBag();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::MouseCapturedInTimeWindow(void)
{
    return itsDoc->MouseCapturedInTimeWindow();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MouseCapturedInTimeWindow(bool newValue)
{
    itsDoc->MouseCapturedInTimeWindow(newValue);
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewSizeInPixels(void) const
{
    return itsDoc->TimeSerialViewSizeInPixels();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewSizeInPixels(const NFmiPoint &newValue)
{
    itsDoc->TimeSerialViewSizeInPixels(newValue);
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::FilterDialogUpdateStatus(void)
{
    return itsDoc->FilterDialogUpdateStatus();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::FilterDialogUpdateStatus(int newState)
{
    itsDoc->FilterDialogUpdateStatus(newState);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ResetTimeFilterTimes()
{
    itsDoc->ResetTimeFilterTimes();
}

NFmiCrossSectionSystem* CtrlViewDocumentInterfaceForGeneralDataDoc::CrossSectionSystem(void)
{
    return itsDoc->CrossSectionSystem();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsOperationalModeOn(void)
{
    return itsDoc->IsOperationalModeOn();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowHelperData1InTimeSerialView()
{
    return itsDoc->ShowHelperData1InTimeSerialView();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowHelperData2InTimeSerialView()
{
    return itsDoc->ShowHelperData2InTimeSerialView();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowHelperData3InTimeSerialView()
{
    return itsDoc->ShowHelperData3InTimeSerialView();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowHelperData4InTimeSerialView()
{
    return itsDoc->ShowHelperData4InTimeSerialView();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::AllowRightClickDisplaySelection(void)
{
    return itsDoc->AllowRightClickDisplaySelection();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::AllowRightClickDisplaySelection(bool newValue)
{
    itsDoc->AllowRightClickDisplaySelection(newValue);
}

NFmiHelpEditorSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::HelpEditorSystem(void)
{
    return itsDoc->HelpEditorSystem();
}

NFmiMetEditorOptionsData& CtrlViewDocumentInterfaceForGeneralDataDoc::MetEditorOptionsData(void)
{
    return itsDoc->MetEditorOptionsData();
}

boost::shared_ptr<NFmiEditorControlPointManager> CtrlViewDocumentInterfaceForGeneralDataDoc::CPManager(bool getOldSchoolCPManager)
{
    return itsDoc->CPManager(getOldSchoolCPManager);
}

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::GetNearestSynopStationInfo(const NFmiLocation &theLocation, const NFmiMetTime &theTime, bool ignoreTime, std::vector<boost::shared_ptr<NFmiFastQueryInfo> > *thePossibleInfoVector, double maxDistanceInMeters)
{
    return itsDoc->GetNearestSynopStationInfo(theLocation, theTime, ignoreTime, thePossibleInfoVector, maxDistanceInMeters);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsMasksUsedInTimeSerialViews(void)
{
    return itsDoc->IsMasksUsedInTimeSerialViews();
}

NFmiAnalyzeToolData& CtrlViewDocumentInterfaceForGeneralDataDoc::AnalyzeToolData(void)
{
    return itsDoc->AnalyzeToolData();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::OutOfEditedAreaTimeSerialPoint(void) const
{
    return itsDoc->OutOfEditedAreaTimeSerialPoint();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::PreciseTimeSerialLatlonPoint() const
{
    return itsDoc->PreciseTimeSerialLatlonPoint();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsPreciseTimeSerialLatlonPointUsed()
{
    return itsDoc->IsPreciseTimeSerialLatlonPointUsed();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewDirty(void)
{
    return itsDoc->GetCombinedMapHandler()->timeSerialViewDirty();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewDirty(bool newValue)
{
    itsDoc->GetCombinedMapHandler()->timeSerialViewDirty(newValue);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateTimeSerialDialogPopup(int index)
{
    return itsDoc->CreateTimeSerialDialogPopup(index);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateTimeSerialDialogOnViewPopup(int index)
{
    return itsDoc->CreateTimeSerialDialogOnViewPopup(index);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::DoTimeSeriesValuesModifying(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, int theUsedMask, NFmiTimeDescriptor& theTimeDescriptor, std::vector<float> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue)
{
    return itsDoc->DoTimeSeriesValuesModifying(theModifiedDrawParam, theUsedMask, theTimeDescriptor, theModificationFactorCurvePoints, theEditorTool, fUseSetForDiscreteData, theUnchangedValue);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::UpdateToModifiedDrawParam(unsigned int mapViewDescTopIndex, boost::shared_ptr<NFmiDrawParam>& drawParam, int viewRowIndex)
{
    itsDoc->GetCombinedMapHandler()->updateToModifiedDrawParam(mapViewDescTopIndex, drawParam, viewRowIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::UseTimeSerialAxisAutoAdjust(void)
{
    return itsDoc->UseTimeSerialAxisAutoAdjust();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::UseQ2Server(void)
{
    return itsDoc->UseQ2Server();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::Registry_ShowLastSendTimeOnMapView()
{
    return itsDoc->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().ShowLastSendTimeOnMapView();
}

double CtrlViewDocumentInterfaceForGeneralDataDoc::Registry_MaximumFontSizeFactor()
{
    return itsDoc->ApplicationWinRegistry().MaximumFontSizeFactor();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::Registry_ShowStationPlot(int theMapViewDescTopIndex)
{
    return itsDoc->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theMapViewDescTopIndex)->ShowStationPlot();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::Registry_SpacingOutFactor(int theMapViewDescTopIndex)
{
    return itsDoc->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theMapViewDescTopIndex)->SpacingOutFactor();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::Registry_ShowMasksOnMap(int theMapViewDescTopIndex)
{
    return itsDoc->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theMapViewDescTopIndex)->ShowMasksOnMap();
}

NFmiIgnoreStationsData& CtrlViewDocumentInterfaceForGeneralDataDoc::IgnoreStationsData(void)
{
    return itsDoc->IgnoreStationsData();
}

const NFmiColor& CtrlViewDocumentInterfaceForGeneralDataDoc::StationPointColor(int theMapViewDescTopIndex) const
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->StationPointColor();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::StationPointSize(int theMapViewDescTopIndex) const
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->StationPointSize();
}

NFmiMacroParamSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::MacroParamSystem(void)
{
    return itsDoc->MacroParamSystem();
}

CtrlViewUtils::FmiEditorModifyToolMode CtrlViewDocumentInterfaceForGeneralDataDoc::ModifyToolMode(void)
{
    return itsDoc->ModifyToolMode();
}

size_t CtrlViewDocumentInterfaceForGeneralDataDoc::SelectedGridPointLimit(void)
{
    return itsDoc->SelectedGridPointLimit();
}

boost::shared_ptr<NFmiDrawParam> CtrlViewDocumentInterfaceForGeneralDataDoc::GetDrawDifferenceDrawParam(void)
{
    return itsDoc->GetDrawDifferenceDrawParam();
}

boost::shared_ptr<NFmiDrawParam> CtrlViewDocumentInterfaceForGeneralDataDoc::GetSelectedGridPointDrawParam(void)
{
    return itsDoc->GetSelectedGridPointDrawParam();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::DrawSelectionOnThisView(void)
{
    return itsDoc->DrawSelectionOnThisView();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DrawSelectionOnThisView(bool newValue)
{
    itsDoc->DrawSelectionOnThisView(newValue);
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipLatLonPoint(void) const
{
    return itsDoc->ToolTipLatLonPoint();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipLatLonPoint(const NFmiPoint& theLatLon)
{
    itsDoc->ToolTipLatLonPoint(theLatLon);
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipMapViewDescTopIndex() const
{
    return itsDoc->ToolTipMapViewDescTopIndex();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipMapViewDescTopIndex(int newIndex)
{
    itsDoc->ToolTipMapViewDescTopIndex(newIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipTime(const NFmiMetTime& theTime)
{
    itsDoc->ToolTipTime(theTime);
}

const NFmiMetTime& CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipTime(void)
{
    return itsDoc->ToolTipTime();
}

TimeSerialModificationDataInterface& CtrlViewDocumentInterfaceForGeneralDataDoc::GenDocDataAdapter(void)
{
    return itsDoc->GenDocDataAdapter();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::UseMultithreaddingWithModifyingFunctions(void)
{
    return itsDoc->UseMultithreaddingWithModifyingFunctions();
}

std::string CtrlViewDocumentInterfaceForGeneralDataDoc::GetModelOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex)
{
    return itsDoc->GetModelOrigTimeString(theDrawParam, theIndex);
}

NFmiObsComparisonInfo& CtrlViewDocumentInterfaceForGeneralDataDoc::ObsComparisonInfo(void)
{
    return itsDoc->ObsComparisonInfo();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::GetDataFromQ2Server(const std::string &theURLStr, const std::string &theParamsStr, bool fUseBinaryData,
    int theUsedCompression, NFmiDataMatrix<float> &theDataMatrixOut, std::string &theExtraInfoStrOut)
{
    itsDoc->GetDataFromQ2Server(theURLStr, theParamsStr, fUseBinaryData, theUsedCompression, theDataMatrixOut, theExtraInfoStrOut);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetMacroErrorText(const std::string &theErrorStr)
{
    itsDoc->SetMacroErrorText(theErrorStr);
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::StationDataGridSize(void)
{
    return itsDoc->StationDataGridSize();
}

NFmiLocationSelectionTool* CtrlViewDocumentInterfaceForGeneralDataDoc::LocationSelectionTool2(void)
{
    return itsDoc->LocationSelectionTool2();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SelectLocations(unsigned int theDescTopIndex, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const boost::shared_ptr<NFmiArea> &theMapArea, 
    const NFmiPoint& theLatLon, const NFmiMetTime &theTime, int theSelectionCombineFunction, unsigned long theMask
    , bool fMakeMTAModeAdd, bool fDoOnlyMTAModeAdd)
{
    itsDoc->SelectLocations(theDescTopIndex, theInfo, theMapArea,theLatLon, theTime, theSelectionCombineFunction, theMask
        , fMakeMTAModeAdd, fDoOnlyMTAModeAdd);
}

NFmiEditMapDataListHandler* CtrlViewDocumentInterfaceForGeneralDataDoc::DataLists(void)
{
    return itsDoc->DataLists();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowObsComparisonOnMap(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->ShowObsComparisonOnMap();
}

std::vector<boost::shared_ptr<NFmiFastQueryInfo>> CtrlViewDocumentInterfaceForGeneralDataDoc::GetSortedSynopInfoVector(int theProducerId, int theProducerId2, int theProducerId3, int theProducerId4)
{
    return itsDoc->GetSortedSynopInfoVector(theProducerId, theProducerId2, theProducerId3, theProducerId4);
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::AbsoluteActiveViewRow(unsigned int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->absoluteActiveViewRow(theDescTopIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::AbsoluteActiveViewRow(unsigned int theDescTopIndex, int theAbsoluteActiveRowIndex)
{
    itsDoc->GetCombinedMapHandler()->absoluteActiveViewRow(theDescTopIndex, theAbsoluteActiveRowIndex);
}

NFmiSynopPlotSettings* CtrlViewDocumentInterfaceForGeneralDataDoc::SynopPlotSettings(void)
{
    return itsDoc->SynopPlotSettings();
}

NFmiSynopStationPrioritySystem* CtrlViewDocumentInterfaceForGeneralDataDoc::SynopStationPrioritySystem(void)
{
    return itsDoc->SynopStationPrioritySystem();
}

NFmiPoint CtrlViewDocumentInterfaceForGeneralDataDoc::ActualMapBitmapSizeInPixels(unsigned int theDescTopIndex)
{
    return itsDoc->ActualMapBitmapSizeInPixels(theDescTopIndex);
}

NFmiConceptualModelData& CtrlViewDocumentInterfaceForGeneralDataDoc::ConceptualModelData(void)
{
    return itsDoc->ConceptualModelData();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MakeHTTPRequest(const std::string &theUrlStr, std::string &theResponseStrOut, bool fDoGet)
{
    itsDoc->MakeHTTPRequest(theUrlStr, theResponseStrOut, fDoGet);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::LeftMouseButtonDown(void)
{
    return itsDoc->LeftMouseButtonDown();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::LeftMouseButtonDown(bool newState)
{
    itsDoc->LeftMouseButtonDown(newState);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::RightMouseButtonDown(void)
{
    return itsDoc->RightMouseButtonDown();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::RightMouseButtonDown(bool newState)
{
    itsDoc->RightMouseButtonDown(newState);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::MiddleMouseButtonDown(void)
{
    return itsDoc->MiddleMouseButtonDown();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MiddleMouseButtonDown(bool newState)
{
    itsDoc->MiddleMouseButtonDown(newState);
}

NFmiDrawParamList* CtrlViewDocumentInterfaceForGeneralDataDoc::CrossSectionViewDrawParamList(int theRowIndex)
{
    return itsDoc->GetCombinedMapHandler()->getCrossSectionViewDrawParamList(theRowIndex);
}

NFmiSmartToolInfo* CtrlViewDocumentInterfaceForGeneralDataDoc::SmartToolInfo(void)
{
    return itsDoc->SmartToolInfo();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetLatestMacroParamErrorText(const std::string& theErrorText)
{
    itsDoc->SetLatestMacroParamErrorText(theErrorText);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateCrossSectionViewPopup(int theRowIndex)
{
    return itsDoc->CreateCrossSectionViewPopup(theRowIndex);
}

const NFmiRect& CtrlViewDocumentInterfaceForGeneralDataDoc::RelativeMapRect(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->RelativeMapRect();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::BetaProductGenerationRunning()
{
    return itsDoc->BetaProductGenerationRunning();
}

const NFmiMetTime& CtrlViewDocumentInterfaceForGeneralDataDoc::LastEditedDataSendTime()
{
    return itsDoc->LastEditedDataSendTime();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsLastEditedDataSendHasComeBack()
{
    return itsDoc->IsLastEditedDataSendHasComeBack();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::DataModificationInProgress(void)
{
    return itsDoc->DataModificationInProgress();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveMapDescTopIndex(unsigned int newValue)
{
    itsDoc->GetCombinedMapHandler()->activeMapDescTopIndex(newValue);
}

const NFmiRect& CtrlViewDocumentInterfaceForGeneralDataDoc::UpdateRect(void)
{
    return itsDoc->UpdateRect();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::UpdateRect(const NFmiRect& theRect)
{
    itsDoc->UpdateRect(theRect);
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipRealRowIndex(void)
{
    return itsDoc->ToolTipRealRowIndex();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipRealRowIndex(int newRealRowIndex)
{
    itsDoc->ToolTipRealRowIndex(newRealRowIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DoAutoZoom(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->doAutoZoom(theDescTopIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::MapMouseDragPanMode(void)
{
    return itsDoc->MapMouseDragPanMode();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MapMouseDragPanMode(bool newState)
{
    itsDoc->MapMouseDragPanMode(newState);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewBitmapDirty(int theDescTopIndex, bool dirtyFlag)
{
    itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->MapViewBitmapDirty(dirtyFlag);
}

NFmiSatelliteImageCacheSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::SatelliteImageCacheSystem()
{
    return itsDoc->SatelliteImageCacheSystem();
}

CWnd* CtrlViewDocumentInterfaceForGeneralDataDoc::TransparencyContourDrawView(void)
{
    return itsDoc->TransparencyContourDrawView();
}

NFmiGridPointCache& CtrlViewDocumentInterfaceForGeneralDataDoc::GridPointCache(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->GridPointCache();
}

NFmiMapViewCache& CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewCache(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->MapViewCache();
}

CDC* CtrlViewDocumentInterfaceForGeneralDataDoc::CopyCDC(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->CopyCDC();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsCPGridCropInAction(void)
{
    return itsDoc->IsCPGridCropInAction();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowSoundingMarkersOnMap(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->ShowSoundingMarkersOnMap();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowCrossSectionMarkersOnMap(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->ShowCrossSectionMarkersOnMap();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowTrajectorsOnMap(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->ShowTrajectorsOnMap();
}

std::vector<NFmiProducer>& CtrlViewDocumentInterfaceForGeneralDataDoc::ExtraSoundingProducerList(void)
{
    return itsDoc->ExtraSoundingProducerList();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowWarningMarkersOnMap(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->ShowWarningMarkersOnMap();
}

CDC* CtrlViewDocumentInterfaceForGeneralDataDoc::MapBlitDC(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->MapBlitDC();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipColumnIndex() const
{
    return itsDoc->ToolTipColumnIndex();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipColumnIndex(int newIndex)
{
    itsDoc->ToolTipColumnIndex(newIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::HasActiveViewChanged(void)
{
    return itsDoc->HasActiveViewChanged();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::LastBrushedViewTime(const NFmiMetTime& newTime)
{
    itsDoc->LastBrushedViewTime(newTime);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::LastBrushedViewRealRowIndex(int newRealRowIndex)
{
    itsDoc->LastBrushedViewRealRowIndex(newRealRowIndex);
}

const NFmiTimeBag& CtrlViewDocumentInterfaceForGeneralDataDoc::EditedDataTimeBag(void)
{
    return itsDoc->EditedDataTimeBag();
}

const boost::shared_ptr<NFmiArea> CtrlViewDocumentInterfaceForGeneralDataDoc::CPGridCropLatlonArea(void)
{
    return itsDoc->CPGridCropLatlonArea();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsCPGridCropNotPlausible(void)
{
    return itsDoc->IsCPGridCropNotPlausible();
}

boost::shared_ptr<NFmiArea> CtrlViewDocumentInterfaceForGeneralDataDoc::CPGridCropInnerLatlonArea(void)
{
    return itsDoc->CPGridCropInnerLatlonArea();
}

NFmiWindTableSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::WindTableSystem(void)
{
    return itsDoc->WindTableSystem();
}

NFmiProjectionCurvatureInfo* CtrlViewDocumentInterfaceForGeneralDataDoc::ProjectionCurvatureInfo(void)
{
    return itsDoc->GetCombinedMapHandler()->projectionCurvatureInfo();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::DrawLandBorders(int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->DrawLandBorders(separateBorderLayerDrawOptions);
}

const NFmiColor& CtrlViewDocumentInterfaceForGeneralDataDoc::LandBorderColor(int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->LandBorderColor(separateBorderLayerDrawOptions);
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::LandBorderPenSize(int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->LandBorderPenSize(separateBorderLayerDrawOptions);
}

Gdiplus::Bitmap* CtrlViewDocumentInterfaceForGeneralDataDoc::LandBorderMapBitmap(unsigned int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions) const
{ 
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->LandBorderMapBitmap(separateBorderLayerDrawOptions);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetLandBorderMapBitmap(unsigned int theDescTopIndex, Gdiplus::Bitmap *newBitmap, NFmiDrawParam* separateBorderLayerDrawOptions)
{
    itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->SetLandBorderMapBitmap(newBitmap, separateBorderLayerDrawOptions);
}

boost::shared_ptr<Imagine::NFmiPath> CtrlViewDocumentInterfaceForGeneralDataDoc::LandBorderPath(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->LandBorderPath();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineList(int theDescTopIndex, std::list<NFmiPolyline*> &polyLineList)
{
    itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->DrawBorderPolyLineList(polyLineList);
}

const std::list<std::vector<NFmiPoint>>& CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineListGdiplus(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->DrawBorderPolyLineListGdiplus();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineListGdiplus(int theDescTopIndex, const std::list<std::vector<NFmiPoint>> &newValue)
{
    itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->DrawBorderPolyLineListGdiplus(newValue);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineListGdiplus(int theDescTopIndex, std::list<std::vector<NFmiPoint>> &&newValue)
{
    itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->DrawBorderPolyLineListGdiplus(std::move(newValue));
}

std::list<NFmiPolyline*>& CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineList(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->DrawBorderPolyLineList();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::DrawOverMapMode(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->DrawOverMapMode();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SnapShotData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const std::string &theModificationText
    , const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime)
{
    itsDoc->SnapShotData(theInfo, theDataIdent, theModificationText, theStartTime, theEndTime);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowWaitCursorWhileDrawingView(void)
{
    return itsDoc->ShowWaitCursorWhileDrawingView();
}

NFmiAutoComplete& CtrlViewDocumentInterfaceForGeneralDataDoc::AutoComplete(void)
{
    return itsDoc->AutoComplete();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowMouseHelpCursorsOnMap(void)
{
    return itsDoc->ShowMouseHelpCursorsOnMap();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::SynopDataGridViewOn(void)
{
    return itsDoc->SynopDataGridViewOn();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowSynopHighlight(void)
{
    return itsDoc->ShowSynopHighlight();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::GetSynopHighlightLatlon(void)
{
    return itsDoc->GetSynopHighlightLatlon();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowTimeString(int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->ShowTimeString();
}

NFmiMapViewTimeLabelInfo& CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewTimeLabelInfo()
{
    return itsDoc->MapViewTimeLabelInfo();
}

const NFmiBetaProduct* CtrlViewDocumentInterfaceForGeneralDataDoc::GetCurrentGeneratedBetaProduct()
{
    return itsDoc->GetCurrentGeneratedBetaProduct();
}

const NFmiMetTime& CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveViewTime(void)
{
    return itsDoc->ActiveViewTime();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveViewTime(const NFmiMetTime& theTime)
{
    itsDoc->ActiveViewTime(theTime);
}

boost::shared_ptr<NFmiDrawParam> CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveDrawParamFromActiveRow(unsigned int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->activeDrawParamFromActiveRow(theDescTopIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ViewBrushed(void)
{
    return itsDoc->ViewBrushed();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ViewBrushed(bool newState)
{
    itsDoc->ViewBrushed(newState);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CheckAndValidateAfterModifications(NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam)
{
    return itsDoc->CheckAndValidateAfterModifications(theModifyingTool, fMakeDataSnapshotAction, theLocationMask, theParam);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ZoomMapInOrOut(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, double theZoomFactor)
{
    itsDoc->ZoomMapInOrOut(theMapViewDescTopIndex, theMapArea, theMousePoint, theZoomFactor);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::UseMaskWithBrush(void)
{
    return itsDoc->UseMaskWithBrush();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::BrushToolLimitSetting(void)
{
    return itsDoc->BrushToolLimitSetting();
}

float CtrlViewDocumentInterfaceForGeneralDataDoc::BrushToolLimitSettingValue(void)
{
    return itsDoc->BrushToolLimitSettingValue();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MustDrawCrossSectionView(bool newValue)
{
    itsDoc->MustDrawCrossSectionView(newValue);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MustDrawTempView(bool newValue)
{
    itsDoc->MustDrawTempView(newValue);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::MustDrawTempView()
{
    return itsDoc->MustDrawTempView();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialDataViewOn(void)
{
    return itsDoc->TimeSerialDataViewOn();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MustDrawTimeSerialView(bool newValue)
{
    itsDoc->MustDrawTimeSerialView(newValue);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowSelectedPointsOnMap(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->ShowSelectedPointsOnMap();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowControlPointsOnMap(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->ShowControlPointsOnMap();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ZoomMapWithRelativeRect(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiRect &theZoomedRect)
{
    itsDoc->ZoomMapWithRelativeRect(theMapViewDescTopIndex, theMapArea, theZoomedRect);
}

FmiDirection CtrlViewDocumentInterfaceForGeneralDataDoc::ParamWindowViewPosition(int theMapViewDescTopIndex)
{
    if(theMapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
        return itsDoc->CrossSectionSystem()->ParamWindowViewPosition();
    else
        return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->ParamWindowViewPosition();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsParamWindowViewVisible(int theMapViewDescTopIndex)
{
    auto paramWindowViewPosition = kNoDirection;
    if(theMapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
        paramWindowViewPosition = itsDoc->CrossSectionSystem()->ParamWindowViewPosition();
    else
        paramWindowViewPosition = itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->ParamWindowViewPosition();

    return paramWindowViewPosition != kNoDirection;
}

NFmiHelpDataInfoSystem* CtrlViewDocumentInterfaceForGeneralDataDoc::HelpDataInfoSystem(void)
{
    return itsDoc->HelpDataInfoSystem();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateCPPopup()
{
    return itsDoc->CreateCPPopup();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::PanMap(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, const NFmiPoint &theZoomDragUpPoint)
{
    itsDoc->PanMap(theMapViewDescTopIndex, theMapArea, theMousePoint, theZoomDragUpPoint);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveViewRect(const NFmiRect& theRect)
{
    itsDoc->ActiveViewRect(theRect);
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::BrushSize(void)
{
    return itsDoc->BrushSize();
}

double CtrlViewDocumentInterfaceForGeneralDataDoc::BrushSpecialParamValue(void)
{
    return itsDoc->BrushSpecialParamValue();
}

const NFmiVPlaceDescriptor& CtrlViewDocumentInterfaceForGeneralDataDoc::SoundingPlotLevels(void)
{
    return itsDoc->SoundingPlotLevels();
}

double CtrlViewDocumentInterfaceForGeneralDataDoc::BrushValue(void)
{
    return itsDoc->BrushValue();
}

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::GetModelClimatologyData(const NFmiLevel& theLevel)
{
    return itsDoc->GetModelClimatologyData(theLevel);
}

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::GetBestSuitableModelFractileData(boost::shared_ptr<NFmiFastQueryInfo>& usedOriginalInfo)
{
    return itsDoc->GetBestSuitableModelFractileData(usedOriginalInfo);
}

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::GetMosTemperatureMinAndMaxData()
{
    return itsDoc->GetMosTemperatureMinAndMaxData();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::UseCombinedMapMode() const
{
    return itsDoc->GetCombinedMapHandler()->useCombinedMapMode();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::UseCombinedMapMode(bool newValue)
{
    itsDoc->GetCombinedMapHandler()->useCombinedMapMode(newValue);
}

NFmiBetaProductionSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::BetaProductionSystem()
{
    return itsDoc->BetaProductionSystem();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetLastActiveDescTopAndViewRow(unsigned int theDescTopIndex, int theActiveRowIndex)
{
    itsDoc->SetLastActiveDescTopAndViewRow(theDescTopIndex, theActiveRowIndex);
}

NFmiApplicationWinRegistry& CtrlViewDocumentInterfaceForGeneralDataDoc::ApplicationWinRegistry()
{
    return itsDoc->ApplicationWinRegistry();
}

Q2ServerInfo& CtrlViewDocumentInterfaceForGeneralDataDoc::GetQ2ServerInfo()
{
    return itsDoc->GetQ2ServerInfo();
}

Warnings::CapDataSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::GetCapDataSystem()
{
    return itsDoc->GetCapDataSystem();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::GetTimeRangeForWarningMessagesOnMapViewInMinutes()
{
    return itsDoc->GetTimeRangeForWarningMessagesOnMapViewInMinutes();
}

NFmiMacroParamDataCache& CtrlViewDocumentInterfaceForGeneralDataDoc::MacroParamDataCache()
{
    return itsDoc->MacroParamDataCache();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::SetupObsBlenderData(const NFmiPoint &theLatlon, const NFmiParam &theParam, NFmiInfoData::Type theDataType, bool fGroundData, const NFmiProducer &theProducer, NFmiMetTime &firstEditedTimeOut, boost::shared_ptr<NFmiFastQueryInfo> &usedObsBlenderInfoOut, float &analyzeValueOut, std::vector<std::string> &messagesOut)
{
    return FmiModifyEditdData::SetupObsBlenderData(itsDoc->GenDocDataAdapter(), theLatlon, theParam, theDataType, fGroundData, theProducer, firstEditedTimeOut, usedObsBlenderInfoOut, analyzeValueOut, messagesOut);
}

TimeSerialParameters& CtrlViewDocumentInterfaceForGeneralDataDoc::GetTimeSerialParameters()
{
    return itsDoc->GetTimeSerialParameters();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(int theMapViewDescTopIndex)
{
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(::GetWantedMapViewIdFlag(theMapViewDescTopIndex));
}

NFmiColorContourLegendSettings& CtrlViewDocumentInterfaceForGeneralDataDoc::ColorContourLegendSettings()
{
    return itsDoc->ColorContourLegendSettings();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetPrintedDescTopIndex(int nowPrintedDescTopIndex)
{
    itsDoc->SetPrintedDescTopIndex(nowPrintedDescTopIndex);
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::GetPrintedDescTopIndex()
{
    return itsDoc->GetPrintedDescTopIndex();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ResetPrintedDescTopIndex()
{
    itsDoc->ResetPrintedDescTopIndex();
}

unsigned int CtrlViewDocumentInterfaceForGeneralDataDoc::SelectedMapIndex(int mapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(mapViewDescTopIndex)->SelectedMapIndex();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetCPCropGridSettings(const boost::shared_ptr<NFmiArea>& newArea, unsigned int mapViewDescTopIndex)
{
    itsDoc->SetCPCropGridSettings(newArea, mapViewDescTopIndex);
}

NFmiFixedDrawParamSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::FixedDrawParamSystem()
{
    return itsDoc->FixedDrawParamSystem();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ApplyFixeDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex, const std::shared_ptr<NFmiDrawParam>& theFixedDrawParam)
{
    itsDoc->GetCombinedMapHandler()->applyFixeDrawParam(theMenuItem, theRowIndex, theFixedDrawParam);
}

NFmiMacroPathSettings& CtrlViewDocumentInterfaceForGeneralDataDoc::MacroPathSettings()
{
    return itsDoc->MacroPathSettings();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::CurrentCrossSectionRowIndex()
{
    return itsDoc->CurrentCrossSectionRowIndex();
}

CombinedMapHandlerInterface& CtrlViewDocumentInterfaceForGeneralDataDoc::GetCombinedMapHandlerInterface()
{
    return *itsDoc->GetCombinedMapHandler();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::BorderDrawBitmapDirty(int theDescTopIndex, NFmiDrawParam* separateBorderLayerDrawOptions) const
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->BorderDrawBitmapDirty(separateBorderLayerDrawOptions);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::BorderDrawPolylinesDirty(int theDescTopIndex) const
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->BorderDrawPolylinesDirty();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::BorderDrawPolylinesGdiplusDirty(int theDescTopIndex) const
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->BorderDrawPolylinesGdiplusDirty();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetBorderDrawDirtyState(int theDescTopIndex, CountryBorderDrawDirtyState newState)
{
    itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->SetBorderDrawDirtyState(newState);
}

double CtrlViewDocumentInterfaceForGeneralDataDoc::SingleMapViewHeightInMilliMeters(int theDescTopIndex) const
{
    if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
        return itsDoc->CrossSectionSystem()->GetTrueMapViewSizeInfo().singleMapSizeInMM().Y();
    else
        return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->SingleMapViewHeightInMilliMeters();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsTimeControlViewVisible(int theDescTopIndex) const
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->IsTimeControlViewVisible();
}

TrueMapViewSizeInfo& CtrlViewDocumentInterfaceForGeneralDataDoc::GetTrueMapViewSizeInfo(int theDescTopIndex) const
{
    if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
        return itsDoc->CrossSectionSystem()->GetTrueMapViewSizeInfo();
    else
        return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->GetTrueMapViewSizeInfo();
}

NFmiSeaLevelPlumeData& CtrlViewDocumentInterfaceForGeneralDataDoc::SeaLevelPlumeData() const
{
    return itsDoc->SeaLevelPlumeData();
}

NFmiMouseClickUrlActionData& CtrlViewDocumentInterfaceForGeneralDataDoc::MouseClickUrlActionData() const
{
    return itsDoc->MouseClickUrlActionData();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::VirtualTime(const NFmiMetTime& virtualTime)
{
    itsDoc->VirtualTime(virtualTime);
}

const NFmiMetTime& CtrlViewDocumentInterfaceForGeneralDataDoc::VirtualTime() const
{
    return itsDoc->VirtualTime();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::VirtualTimeUsed() const
{
    return itsDoc->VirtualTimeUsed();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ToggleVirtualTimeMode()
{
    itsDoc->ToggleVirtualTimeMode();
}

std::string CtrlViewDocumentInterfaceForGeneralDataDoc::GetVirtualTimeTooltipText() const
{
    return itsDoc->GetVirtualTimeTooltipText();
}

#ifndef DISABLE_CPPRESTSDK
// ===============================================

HakeMessage::Main& CtrlViewDocumentInterfaceForGeneralDataDoc::WarningCenterSystem(void)
{
    return itsDoc->WarningCenterSystem();
}

std::shared_ptr<WmsSupportInterface> CtrlViewDocumentInterfaceForGeneralDataDoc::GetWmsSupport() const
{
    return itsDoc->GetCombinedMapHandler()->getWmsSupport();
}

#endif // DISABLE_CPPRESTSDK
