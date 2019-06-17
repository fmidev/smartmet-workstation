#include "CtrlViewDocumentInterfaceForGeneralDataDoc.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiHelpEditorSystem.h"
#include "GraphicalInfo.h"
#include "GdiPlusMapHandlerInterface.h"
#include "NFmiApplicationWinRegistry.h"
#include "TimeSerialModification.h"

CtrlViewDocumentInterfaceForGeneralDataDoc::CtrlViewDocumentInterfaceForGeneralDataDoc(NFmiEditMapGeneralDataDoc *theDoc)
    :itsDoc(theDoc)
{
}

CtrlViewUtils::MapViewMode CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewDisplayMode(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->MapViewDisplayMode();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::ViewGridSize(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->ViewGridSize();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::MapRowStartingIndex(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->MapRowStartingIndex();
}

CtrlViewUtils::GraphicalInfo& CtrlViewDocumentInterfaceForGeneralDataDoc::GetGraphicalInfo(int theMapViewDescTopIndex)
{
    return itsDoc->GetGraphicalInfo(theMapViewDescTopIndex);
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
    return itsDoc->ScrollViewRow(theDescTopIndex, theCount);
}

const std::string& CtrlViewDocumentInterfaceForGeneralDataDoc::HelpDataPath(void) const
{
    return itsDoc->HelpDataPath();
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
    return itsDoc->DrawParamList(theDescTopIndex, theIndex);
}

NFmiDrawParamList* CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewDrawParamList(void)
{
    return itsDoc->TimeSerialViewDrawParamList();
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
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->MapViewSizeInPixels();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::Printing()
{
    return itsDoc->Printing();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateViewParamsPopup(unsigned int theDescTopIndex, int theRowIndex, int index)
{
    return itsDoc->CreateViewParamsPopup(theDescTopIndex, theRowIndex, index);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty, bool fClearCache, int theWantedMapViewDescTop)
{
    itsDoc->RefreshApplicationViewsAndDialogs(reasonForUpdate, fMakeAreaViewDirty, fClearCache, theWantedMapViewDescTop);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ExecuteCommand(const NFmiMenuItem &theMenuItem, int theViewIndex, int theViewTypeId)
{
    return itsDoc->ExecuteCommand(theMenuItem, theViewIndex, theViewTypeId);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ChangeParamSettingsToNextFixedDrawParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams)
{
    return itsDoc->ChangeParamSettingsToNextFixedDrawParam(theDescTopIndex, theMapRow, theParamIndex, fNext, fUseCrossSectionParams);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ChangeActiveMapViewParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams)
{
    return itsDoc->ChangeActiveMapViewParam(theDescTopIndex, theMapRow, theParamIndex, fNext, fUseCrossSectionParams);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::MoveActiveMapViewParamInDrawingOrderList(unsigned int theDescTopIndex, int theMapRow, bool fRaise, bool fUseCrossSectionParams)
{
    return itsDoc->MoveActiveMapViewParamInDrawingOrderList(theDescTopIndex, theMapRow, fRaise, fUseCrossSectionParams);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::CheckAnimationLockedModeTimeBags(unsigned int theDescTopIndex, bool ignoreSatelImages)
{
    itsDoc->CheckAnimationLockedModeTimeBags(theDescTopIndex, ignoreSatelImages);
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
    return itsDoc->CurrentTime(theDescTopIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CurrentTime(unsigned int theDescTopIndex, const NFmiMetTime& newCurrentTime, bool fStayInsideAnimationTimes)
{
    return itsDoc->CurrentTime(theDescTopIndex, newCurrentTime, fStayInsideAnimationTimes);
}

boost::shared_ptr<NFmiDrawParam> CtrlViewDocumentInterfaceForGeneralDataDoc::DefaultEditedDrawParam(void)
{
    return itsDoc->DefaultEditedDrawParam();
}

NFmiAnimationData& CtrlViewDocumentInterfaceForGeneralDataDoc::AnimationData(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->AnimationDataRef();
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
    return itsDoc->ActiveMapTime();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MakeDrawedInfoVectorForMapView(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea)
{
    itsDoc->MakeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
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
    return std::make_unique<GdiPlusMapHandlerInterface>(itsDoc->MapViewDescTop(theMapViewDescTopIndex)->MapHandler());
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::KeepMapAspectRatio()
{
    return itsDoc->ApplicationWinRegistry().KeepMapAspectRatio();
}

double CtrlViewDocumentInterfaceForGeneralDataDoc::ClientViewXperYRatio(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->ClientViewXperYRatio();
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
    return itsDoc->TimeControlViewTimes(theDescTopIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::TimeControlViewTimes(unsigned int theDescTopIndex, const NFmiTimeDescriptor &newTimeDescriptor)
{
    itsDoc->TimeControlViewTimes(theDescTopIndex, newTimeDescriptor);
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
    itsDoc->TimeControlTimeStep(theMapViewDescTopIndex, timeStepInHours);
}

float CtrlViewDocumentInterfaceForGeneralDataDoc::TimeControlTimeStep(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->TimeControlTimeStep();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
    itsDoc->MapViewDirty(theDescTopIndex, makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, clearMacroParamDataCache, clearEditedDataDependentCaches, updateMapViewDrawingLayers);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ForceStationViewRowUpdate(unsigned int theDescTopIndex, unsigned int theRealRowIndex)
{
    itsDoc->ForceStationViewRowUpdate(theDescTopIndex, theRealRowIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::SetDataToNextTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
    return itsDoc->SetDataToNextTime(theDescTopIndex, fStayInsideAnimationTimes);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::SetDataToPreviousTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
    return itsDoc->SetDataToPreviousTime(theDescTopIndex, fStayInsideAnimationTimes);
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

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::GetNearestSynopStationInfo(const NFmiLocation &theLocation, const NFmiMetTime &theTime, bool ignoreTime, checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > *thePossibleInfoVector, double maxDistanceInMeters)
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
    return itsDoc->TimeSerialViewDirty();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::TimeSerialViewDirty(bool newValue)
{
    itsDoc->TimeSerialViewDirty(newValue);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateTimeSerialDialogPopup(int index)
{
    return itsDoc->CreateTimeSerialDialogPopup(index);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CreateTimeSerialDialogOnViewPopup(int index)
{
    return itsDoc->CreateTimeSerialDialogOnViewPopup(index);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::DoTimeSeriesValuesModifying(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, int theUsedMask, NFmiTimeDescriptor& theTimeDescriptor, checkedVector<double> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue)
{
    return itsDoc->DoTimeSeriesValuesModifying(theModifiedDrawParam, theUsedMask, theTimeDescriptor, theModificationFactorCurvePoints, theEditorTool, fUseSetForDiscreteData, theUnchangedValue);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::UpdateModifiedDrawParamMarko(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    itsDoc->UpdateModifiedDrawParamMarko(theDrawParam);
}

FmiTimeEditMode CtrlViewDocumentInterfaceForGeneralDataDoc::TimeEditMode(void) const
{
    return itsDoc->TimeEditMode();
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
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->StationPointColor();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::StationPointSize(int theMapViewDescTopIndex) const
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->StationPointSize();
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
    return itsDoc->MapViewDescTop(theDescTopIndex)->ShowObsComparisonOnMap();
}

checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> CtrlViewDocumentInterfaceForGeneralDataDoc::GetSortedSynopInfoVector(int theProducerId, int theProducerId2, int theProducerId3, int theProducerId4)
{
    return itsDoc->GetSortedSynopInfoVector(theProducerId, theProducerId2, theProducerId3, theProducerId4);
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveViewRow(unsigned int theDescTopIndex)
{
    return itsDoc->ActiveViewRow(theDescTopIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveViewRow(unsigned int theDescTopIndex, int theActiveRowIndex)
{
    itsDoc->ActiveViewRow(theDescTopIndex, theActiveRowIndex);
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
    return itsDoc->CrossSectionViewDrawParamList(theRowIndex);
}

NFmiPoint CtrlViewDocumentInterfaceForGeneralDataDoc::ActualCrossSectionBitmapSizeInPixels(void)
{
    return itsDoc->ActualCrossSectionBitmapSizeInPixels();
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

void CtrlViewDocumentInterfaceForGeneralDataDoc::CrossSectionDataViewFrame(const NFmiRect &theRect)
{
    itsDoc->CrossSectionDataViewFrame(theRect);
}

const NFmiRect& CtrlViewDocumentInterfaceForGeneralDataDoc::RelativeMapRect(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->RelativeMapRect();
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
    itsDoc->ActiveMapDescTopIndex(newValue);
}

const NFmiRect& CtrlViewDocumentInterfaceForGeneralDataDoc::UpdateRect(void)
{
    return itsDoc->UpdateRect();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::UpdateRect(const NFmiRect& theRect)
{
    itsDoc->UpdateRect(theRect);
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipRowIndex(void)
{
    return itsDoc->ToolTipRowIndex();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ToolTipRowIndex(int newIndex)
{
    itsDoc->ToolTipRowIndex(newIndex);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DoAutoZoom(unsigned int theDescTopIndex)
{
    itsDoc->DoAutoZoom(theDescTopIndex);
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
    itsDoc->MapViewDescTop(theDescTopIndex)->MapViewBitmapDirty(dirtyFlag);
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
    return itsDoc->MapViewDescTop(theDescTopIndex)->GridPointCache();
}

NFmiMapViewCache& CtrlViewDocumentInterfaceForGeneralDataDoc::MapViewCache(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->MapViewCache();
}

CDC* CtrlViewDocumentInterfaceForGeneralDataDoc::CopyCDC(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->CopyCDC();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::IsCPGridCropInAction(void)
{
    return itsDoc->IsCPGridCropInAction();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowSoundingMarkersOnMap(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->ShowSoundingMarkersOnMap();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowCrossSectionMarkersOnMap(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->ShowCrossSectionMarkersOnMap();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowTrajectorsOnMap(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->ShowTrajectorsOnMap();
}

std::vector<NFmiProducer>& CtrlViewDocumentInterfaceForGeneralDataDoc::ExtraSoundingProducerList(void)
{
    return itsDoc->ExtraSoundingProducerList();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowWarningMarkersOnMap(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->ShowWarningMarkersOnMap();
}

CDC* CtrlViewDocumentInterfaceForGeneralDataDoc::MapBlitDC(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->MapBlitDC();
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

void CtrlViewDocumentInterfaceForGeneralDataDoc::LastBrushedViewRow(int newRow)
{
    itsDoc->LastBrushedViewRow(newRow);
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

NFmiSeaIcingWarningSystem& CtrlViewDocumentInterfaceForGeneralDataDoc::SeaIcingWarningSystem(void)
{
    return itsDoc->SeaIcingWarningSystem();
}

NFmiProjectionCurvatureInfo* CtrlViewDocumentInterfaceForGeneralDataDoc::ProjectionCurvatureInfo(void)
{
    return itsDoc->ProjectionCurvatureInfo();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::DrawLandBorders(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->DrawLandBorders();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::BorderDrawDirty(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->BorderDrawDirty();
}

const NFmiColor& CtrlViewDocumentInterfaceForGeneralDataDoc::LandBorderColor(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->LandBorderColor();
}

const NFmiPoint& CtrlViewDocumentInterfaceForGeneralDataDoc::LandBorderPenSize(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->LandBorderPenSize();
}

boost::shared_ptr<Imagine::NFmiPath> CtrlViewDocumentInterfaceForGeneralDataDoc::LandBorderPath(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->LandBorderPath();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineList(int theDescTopIndex, std::list<NFmiPolyline*> &polyLineList)
{
    itsDoc->MapViewDescTop(theDescTopIndex)->DrawBorderPolyLineList(polyLineList);
}

const std::list<std::vector<NFmiPoint>>& CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineListGdiplus(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->DrawBorderPolyLineListGdiplus();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineListGdiplus(int theDescTopIndex, const std::list<std::vector<NFmiPoint>> &newValue)
{
    itsDoc->MapViewDescTop(theDescTopIndex)->DrawBorderPolyLineListGdiplus(newValue);
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineListGdiplus(int theDescTopIndex, std::list<std::vector<NFmiPoint>> &&newValue)
{
    itsDoc->MapViewDescTop(theDescTopIndex)->DrawBorderPolyLineListGdiplus(newValue);
}


void CtrlViewDocumentInterfaceForGeneralDataDoc::BorderDrawDirty(int theDescTopIndex, bool dirtyFlag)
{
    itsDoc->MapViewDescTop(theDescTopIndex)->BorderDrawDirty(dirtyFlag);
}

std::list<NFmiPolyline*>& CtrlViewDocumentInterfaceForGeneralDataDoc::DrawBorderPolyLineList(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->DrawBorderPolyLineList();
}

int CtrlViewDocumentInterfaceForGeneralDataDoc::DrawOverMapMode(int theDescTopIndex)
{
    return itsDoc->MapViewDescTop(theDescTopIndex)->DrawOverMapMode();
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
    return itsDoc->MapViewDescTop(theDescTopIndex)->ShowTimeString();
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

boost::shared_ptr<NFmiDrawParam> CtrlViewDocumentInterfaceForGeneralDataDoc::ActiveDrawParam(unsigned int theDescTopIndex, int theRowIndex)
{
    return itsDoc->ActiveDrawParam(theDescTopIndex, theRowIndex);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ViewBrushed(void)
{
    return itsDoc->ViewBrushed();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ViewBrushed(bool newState)
{
    itsDoc->ViewBrushed(newState);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::CheckAndValidateAfterModifications(NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam, bool fPasteAction)
{
    return itsDoc->CheckAndValidateAfterModifications(theModifyingTool, fMakeDataSnapshotAction, theLocationMask, theParam, fPasteAction);
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
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->ShowSelectedPointsOnMap();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowControlPointsOnMap(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->ShowControlPointsOnMap();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::ZoomMapWithRelativeRect(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiRect &theZoomedRect)
{
    itsDoc->ZoomMapWithRelativeRect(theMapViewDescTopIndex, theMapArea, theZoomedRect);
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::ShowParamWindowView(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->ShowParamWindowView();
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

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::GetModelClimatologyData()
{
    return itsDoc->GetModelClimatologyData();
}

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::GetFavoriteSurfaceModelFractileData()
{
    return itsDoc->GetFavoriteSurfaceModelFractileData();
}

boost::shared_ptr<NFmiFastQueryInfo> CtrlViewDocumentInterfaceForGeneralDataDoc::GetMosTemperatureMinAndMaxData()
{
    return itsDoc->GetMosTemperatureMinAndMaxData();
}

bool CtrlViewDocumentInterfaceForGeneralDataDoc::UseWmsMaps()
{
    return itsDoc->UseWmsMaps();
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::UseWmsMaps(bool newValue)
{
    itsDoc->UseWmsMaps(newValue);
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

Gdiplus::Bitmap* CtrlViewDocumentInterfaceForGeneralDataDoc::LandBorderMapBitmap(unsigned int theDescTopIndex)
{ 
    return itsDoc->MapViewDescTop(theDescTopIndex)->LandBorderMapBitmap(); 
}

void CtrlViewDocumentInterfaceForGeneralDataDoc::SetLandBorderMapBitmap(unsigned int theDescTopIndex, Gdiplus::Bitmap *newBitmap)
{
    itsDoc->MapViewDescTop(theDescTopIndex)->SetLandBorderMapBitmap(newBitmap);
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

#ifndef DISABLE_CPPRESTSDK
// ===============================================

HakeMessage::Main& CtrlViewDocumentInterfaceForGeneralDataDoc::WarningCenterSystem(void)
{
    return itsDoc->WarningCenterSystem();
}

Wms::WmsSupport& CtrlViewDocumentInterfaceForGeneralDataDoc::WmsSupport()
{
    return itsDoc->WmsSupport();
}

#endif // DISABLE_CPPRESTSDK
