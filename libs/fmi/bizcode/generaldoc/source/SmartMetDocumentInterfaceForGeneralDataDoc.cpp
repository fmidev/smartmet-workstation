#include "SmartMetDocumentInterfaceForGeneralDataDoc.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiMapViewDescTop.h"
#include "ApplicationInterface.h"
#include "CombinedMapHandlerInterface.h"
#include "NFmiApplicationWinRegistry.h"
#include "SpecialDesctopIndex.h"
#include "NFmiCrossSectionSystem.h"

SmartMetDocumentInterfaceForGeneralDataDoc::SmartMetDocumentInterfaceForGeneralDataDoc(NFmiEditMapGeneralDataDoc *theDoc)
    :itsDoc(theDoc)
{
}

NFmiBetaProductionSystem& SmartMetDocumentInterfaceForGeneralDataDoc::BetaProductionSystem()
{
    return itsDoc->BetaProductionSystem();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::BetaProductGenerationRunning()
{
    return itsDoc->BetaProductGenerationRunning();
}


void SmartMetDocumentInterfaceForGeneralDataDoc::BetaProductGenerationRunning(bool newValue)
{
    itsDoc->BetaProductGenerationRunning(newValue);
}
void SmartMetDocumentInterfaceForGeneralDataDoc::SetAllViewIconsDynamically()
{
    itsDoc->SetAllViewIconsDynamically();
}

FmiLanguage SmartMetDocumentInterfaceForGeneralDataDoc::Language()
{
    return itsDoc->Language();
}

const std::string& SmartMetDocumentInterfaceForGeneralDataDoc::RootViewMacroPath()
{
    return itsDoc->RootViewMacroPath();
}

const NFmiMetTime& SmartMetDocumentInterfaceForGeneralDataDoc::CurrentTime(unsigned int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->currentTime(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::LoadViewMacroInfo(NFmiViewSettingMacro &theViewMacro, bool fTreatAsViewMacro)
{
    itsDoc->LoadViewMacroInfo(theViewMacro, fTreatAsViewMacro);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::FillViewMacroInfo(NFmiViewSettingMacro &theViewMacro, const std::string &theName, const std::string &theDescription)
{
    itsDoc->FillViewMacroInfo(theViewMacro, theName, theDescription);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption, bool flushLogger)
{
    itsDoc->LogAndWarnUser(theMessageStr, theDialogTitleStr, severity, category, justLog, addAbortOption, flushLogger);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty, bool fClearCache, int theWantedMapViewDescTop)
{
    itsDoc->RefreshApplicationViewsAndDialogs(reasonForUpdate, fMakeAreaViewDirty, fClearCache, theWantedMapViewDescTop);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::RefreshApplicationViewsAndDialogs(const std::string& reasonForUpdate, SmartMetViewId updatedViewsFlag)
{
    if(ApplicationInterface::GetApplicationInterfaceImplementation)
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs(reasonForUpdate, updatedViewsFlag);
}

NFmiMapViewDescTop* SmartMetDocumentInterfaceForGeneralDataDoc::MapViewDescTop(unsigned int theIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theIndex);
}

NFmiDrawParamList* SmartMetDocumentInterfaceForGeneralDataDoc::DrawParamListWithRealRowNumber(unsigned int theDescTopIndex, int theRealRowIndex)
{
    return itsDoc->GetCombinedMapHandler()->getDrawParamListWithRealRowNumber(theDescTopIndex, theRealRowIndex);
}

NFmiInfoOrganizer* SmartMetDocumentInterfaceForGeneralDataDoc::InfoOrganizer()
{
    return itsDoc->InfoOrganizer();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::LoadViewMacroFromBetaProduct(const std::string &theAbsoluteVieMacroPath, std::string &theErrorStringOut, bool justLogMessages)
{
    return itsDoc->LoadViewMacroFromBetaProduct(theAbsoluteVieMacroPath, theErrorStringOut, justLogMessages);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetCurrentGeneratedBetaProduct(const NFmiBetaProduct *theBetaProduct)
{
    itsDoc->SetCurrentGeneratedBetaProduct(theBetaProduct);
}

const NFmiBetaProduct* SmartMetDocumentInterfaceForGeneralDataDoc::GetCurrentGeneratedBetaProduct()
{
    return itsDoc->GetCurrentGeneratedBetaProduct();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::CurrentTime(unsigned int theDescTopIndex, const NFmiMetTime& newCurrentTime, bool fStayInsideAnimationTimes)
{
    return itsDoc->GetCombinedMapHandler()->currentTime(theDescTopIndex, newCurrentTime, fStayInsideAnimationTimes);
}

NFmiSatelliteImageCacheSystem& SmartMetDocumentInterfaceForGeneralDataDoc::SatelliteImageCacheSystem()
{
    return itsDoc->SatelliteImageCacheSystem();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::MakeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea)
{
    itsDoc->GetCombinedMapHandler()->makeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
}

NFmiApplicationWinRegistry& SmartMetDocumentInterfaceForGeneralDataDoc::ApplicationWinRegistry()
{
    return itsDoc->ApplicationWinRegistry();
}

const NFmiPoint& SmartMetDocumentInterfaceForGeneralDataDoc::BrushSize()
{
    return itsDoc->BrushSize();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::BrushSize(const NFmiPoint& theSize)
{
    itsDoc->BrushSize(theSize);
}

NFmiEditMapDataListHandler* SmartMetDocumentInterfaceForGeneralDataDoc::DataLists()
{
    return itsDoc->DataLists();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::UseMaskWithBrush()
{
    return itsDoc->UseMaskWithBrush();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UseMaskWithBrush(bool newState)
{
    itsDoc->UseMaskWithBrush(newState);
}

double SmartMetDocumentInterfaceForGeneralDataDoc::BrushValue()
{
    return itsDoc->BrushValue();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::BrushValue(double newValue)
{
    itsDoc->BrushValue(newValue);
}

float SmartMetDocumentInterfaceForGeneralDataDoc::BrushToolLimitSettingValue()
{
    return itsDoc->BrushToolLimitSettingValue();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::BrushToolLimitSettingValue(float newValue)
{
    itsDoc->BrushToolLimitSettingValue(newValue);
}

int SmartMetDocumentInterfaceForGeneralDataDoc::BrushToolLimitSetting()
{
    return itsDoc->BrushToolLimitSetting();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::BrushToolLimitSetting(int newValue)
{
    itsDoc->BrushToolLimitSetting(newValue);
}

CtrlViewUtils::FmiEditorModifyToolMode SmartMetDocumentInterfaceForGeneralDataDoc::ModifyToolMode()
{
    return itsDoc->ModifyToolMode();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ModifyToolMode(CtrlViewUtils::FmiEditorModifyToolMode newState)
{
    itsDoc->ModifyToolMode(newState);
}

boost::shared_ptr<NFmiDrawParam> SmartMetDocumentInterfaceForGeneralDataDoc::ActiveDrawParamFromActiveRow(unsigned int theDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->activeDrawParamFromActiveRow(theDescTopIndex);
}

int SmartMetDocumentInterfaceForGeneralDataDoc::AbsoluteActiveViewRow(unsigned int theDescTopIndex)
{
	return itsDoc->GetCombinedMapHandler()->absoluteActiveViewRow(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::AbsoluteActiveViewRow(unsigned int theDescTopIndex, int theAbsoluteActiveRowIndex)
{
    itsDoc->GetCombinedMapHandler()->absoluteActiveViewRow(theDescTopIndex, theAbsoluteActiveRowIndex);
}

double SmartMetDocumentInterfaceForGeneralDataDoc::BrushSpecialParamValue()
{
    return itsDoc->BrushSpecialParamValue();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::BrushSpecialParamValue(double newValue)
{
    itsDoc->BrushSpecialParamValue(newValue);
}

boost::shared_ptr<NFmiFastQueryInfo> SmartMetDocumentInterfaceForGeneralDataDoc::EditedSmartInfo()
{
    return itsDoc->EditedSmartInfo();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnButtonRedo()
{
    itsDoc->OnButtonRedo();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnButtonUndo()
{
    itsDoc->OnButtonUndo();
}

NFmiHelpDataInfoSystem* SmartMetDocumentInterfaceForGeneralDataDoc::HelpDataInfoSystem()
{
    return itsDoc->HelpDataInfoSystem();
}

NFmiProducerSystem& SmartMetDocumentInterfaceForGeneralDataDoc::ProducerSystem()
{
    return itsDoc->ProducerSystem();
}

NFmiProducerSystem& SmartMetDocumentInterfaceForGeneralDataDoc::ObsProducerSystem()
{
    return itsDoc->ObsProducerSystem();
}

NFmiProducerSystem& SmartMetDocumentInterfaceForGeneralDataDoc::SatelImageProducerSystem()
{
    return itsDoc->SatelImageProducerSystem();
}

NFmiCaseStudySystem& SmartMetDocumentInterfaceForGeneralDataDoc::CaseStudySystem()
{
    return itsDoc->CaseStudySystem();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::StoreCaseStudyMemory()
{
    return itsDoc->StoreCaseStudyMemory();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::CaseStudyToNormalMode()
{
    itsDoc->CaseStudyToNormalMode();
}

NFmiCaseStudySystem& SmartMetDocumentInterfaceForGeneralDataDoc::LoadedCaseStudySystem()
{
    return itsDoc->LoadedCaseStudySystem();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::CaseStudyModeOn()
{
    return itsDoc->CaseStudyModeOn();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::LoadCaseStudyData(const std::string &theCaseStudyMetaFile)
{
    return itsDoc->LoadCaseStudyData(theCaseStudyMetaFile);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UpdateEnableDataChangesToWinReg()
{
    itsDoc->UpdateEnableDataChangesToWinReg();
}

NFmiApplicationDataBase& SmartMetDocumentInterfaceForGeneralDataDoc::ApplicationDataBase()
{
    return itsDoc->ApplicationDataBase();
}

const std::string& SmartMetDocumentInterfaceForGeneralDataDoc::WorkingDirectory() const
{
    return itsDoc->WorkingDirectory();
}

const std::string& SmartMetDocumentInterfaceForGeneralDataDoc::ControlDirectory() const
{
    return itsDoc->ControlDirectory();
}

const std::string& SmartMetDocumentInterfaceForGeneralDataDoc::HelpDataPath() const
{
    return itsDoc->HelpDataPath();
}

NFmiIgnoreStationsData& SmartMetDocumentInterfaceForGeneralDataDoc::IgnoreStationsData()
{
    return itsDoc->IgnoreStationsData();
}

NFmiDataLoadingInfo& SmartMetDocumentInterfaceForGeneralDataDoc::GetUsedDataLoadingInfo()
{
    return itsDoc->GetUsedDataLoadingInfo();
}

NFmiModelDataBlender& SmartMetDocumentInterfaceForGeneralDataDoc::ModelDataBlender()
{
    return itsDoc->ModelDataBlender();
}

CtrlViewUtils::FmiSmartMetEditingMode SmartMetDocumentInterfaceForGeneralDataDoc::SmartMetEditingMode()
{
    return itsDoc->SmartMetEditingMode();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SmartMetEditingMode(CtrlViewUtils::FmiSmartMetEditingMode newValue, bool modifySettings)
{
    itsDoc->SmartMetEditingMode(newValue, modifySettings);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetDataLoadingProducerIndexVector(const std::vector<int>& theIndexVector)
{
    itsDoc->SetDataLoadingProducerIndexVector(theIndexVector);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::Printing()
{
    return itsDoc->Printing();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::Printing(bool newStatus)
{
    itsDoc->Printing(newStatus);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::Printing(unsigned int theMapViewDescTopIndex, bool newStatus)
{
    Printing(newStatus);
    auto mapViewDescTop = MapViewDescTop(theMapViewDescTopIndex);
    if(mapViewDescTop)
    {
        mapViewDescTop->PrintingModeOn(newStatus);
    }
}

boost::shared_ptr<NFmiDrawParam> SmartMetDocumentInterfaceForGeneralDataDoc::DefaultEditedDrawParam()
{
    return itsDoc->DefaultEditedDrawParam();
}

double SmartMetDocumentInterfaceForGeneralDataDoc::DrawObjectScaleFactor()
{
    return itsDoc->GetCombinedMapHandler()->drawObjectScaleFactor();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::DrawObjectScaleFactor(double newValue)
{
    itsDoc->GetCombinedMapHandler()->drawObjectScaleFactor(newValue);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::TransparencyContourDrawView(CWnd *theView)
{
    itsDoc->TransparencyContourDrawView(theView);
}

CWnd* SmartMetDocumentInterfaceForGeneralDataDoc::TransparencyContourDrawView()
{
    return itsDoc->TransparencyContourDrawView();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::IsToolMasterAvailable() const
{
    return itsDoc->IsToolMasterAvailable();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ToolMasterAvailable(bool newValue)
{
    itsDoc->ToolMasterAvailable(newValue);
}

NFmiCrossSectionSystem* SmartMetDocumentInterfaceForGeneralDataDoc::CrossSectionSystem()
{
    return itsDoc->CrossSectionSystem();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ActivateParamSelectionDlgAfterLeftDoubleClick()
{
    return itsDoc->ActivateParamSelectionDlgAfterLeftDoubleClick();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ActivateParamSelectionDlgAfterLeftDoubleClick(bool newValue)
{
    itsDoc->ActivateParamSelectionDlgAfterLeftDoubleClick(newValue);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ActivateParameterSelectionDlg(unsigned int theMapViewDescTopIndex)
{
    itsDoc->ActivateViewParamSelectorDlg(theMapViewDescTopIndex);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::OpenPopupMenu()
{
    return itsDoc->OpenPopupMenu();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OpenPopupMenu(bool newState)
{
    itsDoc->OpenPopupMenu(newState);
}

const NFmiPoint& SmartMetDocumentInterfaceForGeneralDataDoc::CrossSectionViewSizeInPixels()
{
    return itsDoc->CrossSectionViewSizeInPixels();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::CrossSectionViewSizeInPixels(const NFmiPoint& newSize)
{
    itsDoc->CrossSectionViewSizeInPixels(newSize);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ShowWaitCursorWhileDrawingView(void)
{
    return itsDoc->ShowWaitCursorWhileDrawingView();
}

NFmiMenuItemList* SmartMetDocumentInterfaceForGeneralDataDoc::PopupMenu()
{
    return itsDoc->PopupMenu();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
    itsDoc->GetCombinedMapHandler()->mapViewDirty(theDescTopIndex, makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, clearMacroParamDataCache, clearEditedDataDependentCaches, updateMapViewDrawingLayers);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ForceStationViewRowUpdate(unsigned int theDescTopIndex, unsigned int theRealRowIndex)
{
    itsDoc->GetCombinedMapHandler()->forceStationViewRowUpdate(theDescTopIndex, theRealRowIndex);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::MakePopUpCommandUsingRowIndex(unsigned short theCommandID)
{
    return itsDoc->MakePopUpCommandUsingRowIndex(theCommandID);
}

NFmiDataQualityChecker& SmartMetDocumentInterfaceForGeneralDataDoc::DataQualityChecker()
{
    return itsDoc->DataQualityChecker();
}

NFmiTrajectorySystem* SmartMetDocumentInterfaceForGeneralDataDoc::TrajectorySystem()
{
    return itsDoc->TrajectorySystem();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ViewBrushed()
{
    return itsDoc->ViewBrushed();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ViewBrushed(bool newState)
{
    itsDoc->ViewBrushed(newState);
}

NFmiPoint SmartMetDocumentInterfaceForGeneralDataDoc::GetPrintedMapAreaOnScreenSizeInPixels(unsigned int theDescTopIndex)
{
    return itsDoc->GetPrintedMapAreaOnScreenSizeInPixels(theDescTopIndex);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::EditedPointsSelectionChanged()
{
    return itsDoc->EditedPointsSelectionChanged();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::EditedPointsSelectionChanged(bool newValue)
{
    return itsDoc->EditedPointsSelectionChanged(newValue);
}

const NFmiPoint& SmartMetDocumentInterfaceForGeneralDataDoc::ToolTipLatLonPoint() const
{
    return itsDoc->ToolTipLatLonPoint();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ToolTipLatLonPoint(const NFmiPoint& theLatLon)
{
    itsDoc->ToolTipLatLonPoint(theLatLon);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::MiddleMouseButtonDown()
{
    return itsDoc->MiddleMouseButtonDown();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::MiddleMouseButtonDown(bool newState)
{
    itsDoc->MiddleMouseButtonDown(newState);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::MouseCaptured()
{
    return itsDoc->MouseCaptured();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::MouseCaptured(bool newState)
{
    itsDoc->MouseCaptured(newState);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::MustDrawTempView(bool newValue)
{
    itsDoc->MustDrawTempView(newValue);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::MustDrawTempView()
{
    return itsDoc->MustDrawTempView();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::MustDrawCrossSectionView(bool newValue)
{
    itsDoc->MustDrawCrossSectionView(newValue);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::MustDrawCrossSectionView()
{
    return itsDoc->MustDrawCrossSectionView();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UpdateTempView()
{
    itsDoc->UpdateTempView();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UpdateCrossSectionView()
{
    itsDoc->UpdateCrossSectionView();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ShowMouseHelpCursorsOnMap()
{
    return itsDoc->ShowMouseHelpCursorsOnMap();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ShowMouseHelpCursorsOnMap(bool newState)
{
    itsDoc->ShowMouseHelpCursorsOnMap(newState);
}

NFmiMetEditorOptionsData& SmartMetDocumentInterfaceForGeneralDataDoc::MetEditorOptionsData()
{
    return itsDoc->MetEditorOptionsData();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetSelectedMapHandler(unsigned int theDescTopIndex, unsigned int newMapIndex)
{
    itsDoc->GetCombinedMapHandler()->setSelectedMapHandler(theDescTopIndex, newMapIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnButtonDataArea(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->onButtonDataArea(theDescTopIndex);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::SetMapViewGrid(unsigned int theDescTopIndex, const NFmiPoint &newValue)
{
    return itsDoc->GetCombinedMapHandler()->setMapViewGrid(theDescTopIndex, newValue);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnShowTimeString(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->onShowTimeString(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnChangeParamWindowPosition(unsigned int theDescTopIndex, bool forward)
{
    if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
        itsDoc->CrossSectionSystem()->ParamWindowViewPositionChange(forward);
    else
        itsDoc->GetCombinedMapHandler()->onChangeParamWindowPosition(theDescTopIndex, forward);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnShowGridPoints(unsigned int theMapViewIndex)
{
    itsDoc->GetCombinedMapHandler()->onShowGridPoints(theMapViewIndex);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::SetDataToNextTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
    return itsDoc->GetCombinedMapHandler()->setDataToNextTime(theDescTopIndex, fStayInsideAnimationTimes);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::SetDataToPreviousTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
    return itsDoc->GetCombinedMapHandler()->setDataToPreviousTime(theDescTopIndex, fStayInsideAnimationTimes);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ScrollViewRow(unsigned int theDescTopIndex, int theCount)
{
    return itsDoc->GetCombinedMapHandler()->scrollViewRow(theDescTopIndex, theCount);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnToggleGridPointColor(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->onToggleGridPointColor(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnToggleGridPointSize(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->onToggleGridPointSize(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnEditSpaceOut(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->onEditSpaceOut(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnButtonRefresh(const std::string& message)
{
    itsDoc->OnButtonRefresh(message);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnShowProjectionLines()
{
    itsDoc->OnShowProjectionLines();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnShowMasksOnMap(unsigned int theDescTopIndex)
{
    itsDoc->OnShowMasksOnMap(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnToggleShowNamesOnMap(unsigned int theDescTopIndex, bool fForward)
{
    itsDoc->GetCombinedMapHandler()->onToggleShowNamesOnMap(theDescTopIndex, fForward);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnChangeMapType(unsigned int theDescTopIndex, bool fForward)
{
    itsDoc->OnChangeMapType(theDescTopIndex, fForward);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnToggleLandBorderPenSize(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->onToggleLandBorderPenSize(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnToggleLandBorderDrawColor(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->onToggleLandBorderDrawColor(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnAcceleratorBorrowParams(unsigned int theDescTopIndex, int row)
{
    itsDoc->GetCombinedMapHandler()->onAcceleratorBorrowParams(theDescTopIndex, row);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnAcceleratorMapRow(unsigned int theDescTopIndex, int theStartingRow)
{
    itsDoc->GetCombinedMapHandler()->onAcceleratorMapRow(theDescTopIndex, theStartingRow);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnAcceleratorToggleKeepMapRatio()
{
    itsDoc->GetCombinedMapHandler()->onAcceleratorToggleKeepMapRatio();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnToggleOverMapBackForeGround(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->onToggleOverMapBackForeGround(theDescTopIndex);
}

std::string SmartMetDocumentInterfaceForGeneralDataDoc::GetToolTipString(unsigned int commandID, std::string &theMagickWord)
{
    return itsDoc->GetToolTipString(commandID, theMagickWord);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ActivateZoomDialog(int theWantedDescTopIndex)
{
    itsDoc->ActivateZoomDialog(theWantedDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::MakeSwapBaseArea(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->makeSwapBaseArea(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SwapArea(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->swapArea(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::CopyMapViewDescTopParams(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->copyMapViewDescTopParams(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::PasteMapViewDescTopParams(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->pasteMapViewDescTopParams(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::CopyDrawParamsFromMapViewRow(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->copyDrawParamsFromMapViewRow(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::PasteDrawParamsToMapViewRow(unsigned int theDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->pasteDrawParamsToMapViewRow(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ToggleTimeControlAnimationView(unsigned int theDescTopIndex)
{
    itsDoc->ToggleTimeControlAnimationView(theDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OnAcceleratorSetHomeTime(int theMapViewDescTopIndex)
{
    itsDoc->OnAcceleratorSetHomeTime(theMapViewDescTopIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::PanMap(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, const NFmiPoint &theZoomDragUpPoint)
{
    itsDoc->PanMap(theMapViewDescTopIndex, theMapArea, theMousePoint, theZoomDragUpPoint);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ZoomMapInOrOut(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, double theZoomFactor)
{
    itsDoc->ZoomMapInOrOut(theMapViewDescTopIndex, theMapArea, theMousePoint, theZoomFactor);
}

const NFmiTimeBag& SmartMetDocumentInterfaceForGeneralDataDoc::EditedDataTimeBag()
{
    return itsDoc->EditedDataTimeBag();
}

int SmartMetDocumentInterfaceForGeneralDataDoc::FilterDialogUpdateStatus()
{
    return itsDoc->FilterDialogUpdateStatus();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::FilterDialogUpdateStatus(int newState)
{
    itsDoc->FilterDialogUpdateStatus(newState);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::DoAreaFiltering()
{
    return itsDoc->DoAreaFiltering();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::DoTimeFiltering()
{
    return itsDoc->DoTimeFiltering();
}

int SmartMetDocumentInterfaceForGeneralDataDoc::FilterFunction()
{
    return itsDoc->FilterFunction();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::FilterFunction(int newFilter)
{
    itsDoc->FilterFunction(newFilter);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::UseMasksWithFilterTool()
{
    return itsDoc->UseMasksWithFilterTool();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UseMasksWithFilterTool(bool newStatus)
{
    itsDoc->UseMasksWithFilterTool(newStatus);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetTestFilterMask(int newValue)
{
    itsDoc->SetTestFilterMask(newValue);
}

int SmartMetDocumentInterfaceForGeneralDataDoc::GetTestFilterMask()
{
    return itsDoc->GetTestFilterMask();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ResetFilters()
{
    itsDoc->ResetFilters();
}

int SmartMetDocumentInterfaceForGeneralDataDoc::FilteringParameterUsageState()
{
    return itsDoc->FilteringParameterUsageState();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::FilteringParameterUsageState(int newState)
{
    itsDoc->FilteringParameterUsageState(newState);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::UseTimeInterpolation()
{
    return itsDoc->UseTimeInterpolation();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UseTimeInterpolation(bool newState)
{
    itsDoc->UseTimeInterpolation(newState);
}

NFmiParamBag& SmartMetDocumentInterfaceForGeneralDataDoc::FilteringParamBag()
{
    return itsDoc->FilteringParamBag();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::FilteringParamBag(const NFmiParamBag& newBag)
{
    itsDoc->FilteringParamBag(newBag);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::DoCombineModelAndKlapse()
{
    return itsDoc->DoCombineModelAndKlapse();
}

NFmiMTATempSystem& SmartMetDocumentInterfaceForGeneralDataDoc::GetMTATempSystem()
{
    return itsDoc->GetMTATempSystem();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ShowToolTipTimeView()
{
    return itsDoc->ShowToolTipTimeView();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ShowToolTipTimeView(bool newValue)
{
    itsDoc->ShowToolTipTimeView(newValue);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ShowToolTipTempView()
{
    return itsDoc->ShowToolTipTempView();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ShowToolTipTempView(bool newValue)
{
    itsDoc->ShowToolTipTempView(newValue);
}

int SmartMetDocumentInterfaceForGeneralDataDoc::SoundingViewWindBarbSpaceOutFactor()
{
    return itsDoc->SoundingViewWindBarbSpaceOutFactor();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SoundingViewWindBarbSpaceOutFactor(int newValue)
{
    itsDoc->SoundingViewWindBarbSpaceOutFactor(newValue);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::DoTEMPDataUpdate(const std::string &theTEMPCodeTextStr, std::string &theTEMPCodeCheckReportStr, bool fJustCheckTEMPCode)
{
    itsDoc->DoTEMPDataUpdate(theTEMPCodeTextStr, theTEMPCodeCheckReportStr, fJustCheckTEMPCode);
}

const std::string& SmartMetDocumentInterfaceForGeneralDataDoc::LastTEMPDataStr()
{
    return itsDoc->LastTEMPDataStr();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ClearTEMPData()
{
    itsDoc->ClearTEMPData();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::TimeSerialViewDirty()
{
    return itsDoc->GetCombinedMapHandler()->timeSerialViewDirty();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::TimeSerialViewDirty(bool newValue)
{
    itsDoc->GetCombinedMapHandler()->timeSerialViewDirty(newValue);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::DrawOverBitmapThings(NFmiToolBox * theGTB)
{
    itsDoc->DrawOverBitmapThings(theGTB);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::MouseCapturedInTimeWindow()
{
    return itsDoc->MouseCapturedInTimeWindow();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::MouseCapturedInTimeWindow(bool newValue)
{
    itsDoc->MouseCapturedInTimeWindow(newValue);
}

CombinedMapHandlerInterface::MapViewDescTopVector& SmartMetDocumentInterfaceForGeneralDataDoc::MapViewDescTopList()
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTops();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::IsMasksUsedInTimeSerialViews()
{
    return itsDoc->IsMasksUsedInTimeSerialViews();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UseMasksInTimeSerialViews(bool newValue)
{
    return itsDoc->UseMasksInTimeSerialViews(newValue);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::UseCPGridCrop()
{
    return itsDoc->UseCPGridCrop();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UseCPGridCrop(bool newValue)
{
    return itsDoc->UseCPGridCrop(newValue);
}

const NFmiPoint& SmartMetDocumentInterfaceForGeneralDataDoc::TimeSerialViewSizeInPixels() const
{
    return itsDoc->TimeSerialViewSizeInPixels();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::TimeSerialViewSizeInPixels(const NFmiPoint &newValue)
{
    return itsDoc->TimeSerialViewSizeInPixels(newValue);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::UseTimeSerialAxisAutoAdjust()
{
    return itsDoc->UseTimeSerialAxisAutoAdjust();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UseTimeSerialAxisAutoAdjust(bool newValue)
{
    return itsDoc->UseTimeSerialAxisAutoAdjust(newValue);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::AllowRightClickDisplaySelection()
{
    return itsDoc->AllowRightClickDisplaySelection();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::AllowRightClickDisplaySelection(bool newValue)
{
    return itsDoc->AllowRightClickDisplaySelection(newValue);
}

NFmiAnalyzeToolData& SmartMetDocumentInterfaceForGeneralDataDoc::AnalyzeToolData()
{
    return itsDoc->AnalyzeToolData();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::UseMultiProcessCpCalc()
{
    return itsDoc->UseMultiProcessCpCalc();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UseMultiProcessCpCalc(bool newValue)
{
    return itsDoc->UseMultiProcessCpCalc(newValue);
}

NFmiCPManagerSet& SmartMetDocumentInterfaceForGeneralDataDoc::CPManagerSet()
{
    return itsDoc->CPManagerSet();
}

boost::shared_ptr<NFmiEditorControlPointManager> SmartMetDocumentInterfaceForGeneralDataDoc::CPManager(bool getOldSchoolCPManager)
{
    return itsDoc->CPManager(getOldSchoolCPManager);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::TimeSerialDataViewOn()
{
    return itsDoc->TimeSerialDataViewOn();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::TimeSerialDataViewOn(bool newValue)
{
    return itsDoc->TimeSerialDataViewOn(newValue);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ShowHelperData1InTimeSerialView()
{
    return itsDoc->ShowHelperData1InTimeSerialView();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ShowHelperData2InTimeSerialView()
{
    return itsDoc->ShowHelperData2InTimeSerialView();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ShowHelperData3InTimeSerialView()
{
    return itsDoc->ShowHelperData3InTimeSerialView();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ShowHelperData4InTimeSerialView()
{
    return itsDoc->ShowHelperData4InTimeSerialView();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ToggleShowHelperDatasInTimeView(int theCommand)
{
    return itsDoc->ToggleShowHelperDatasInTimeView(theCommand);
}

NFmiDrawParamList* SmartMetDocumentInterfaceForGeneralDataDoc::TimeSerialViewDrawParamList()
{
    return &itsDoc->GetCombinedMapHandler()->getTimeSerialViewDrawParamList();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetMapArea(unsigned int theDescTopIndex, const boost::shared_ptr<NFmiArea> &newArea)
{
    itsDoc->GetCombinedMapHandler()->setMapArea(theDescTopIndex, newArea);
}

const NFmiMetTime& SmartMetDocumentInterfaceForGeneralDataDoc::ActiveMapTime()
{
    return itsDoc->GetCombinedMapHandler()->activeMapTime();
}

const std::string& SmartMetDocumentInterfaceForGeneralDataDoc::FileDialogDirectoryMemory()
{
    return itsDoc->FileDialogDirectoryMemory();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::MakeAndStoreFileDialogDirectoryMemory(const std::string& thePathAndFileName)
{
    return itsDoc->MakeAndStoreFileDialogDirectoryMemory(thePathAndFileName);
}

NFmiFixedDrawParamSystem& SmartMetDocumentInterfaceForGeneralDataDoc::FixedDrawParamSystem()
{
    return itsDoc->FixedDrawParamSystem();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::TakeDrawParamInUseEveryWhere(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool useInMap, bool useInTimeSerial, bool useInCrossSection, bool useWithViewMacros)
{
    return itsDoc->GetCombinedMapHandler()->takeDrawParamInUseEveryWhere(theDrawParam, useInMap, useInTimeSerial, useInCrossSection, useWithViewMacros);
}

NFmiSmartToolInfo* SmartMetDocumentInterfaceForGeneralDataDoc::SmartToolInfo()
{
    return itsDoc->SmartToolInfo();
}

std::shared_ptr<NFmiMacroParamSystem> SmartMetDocumentInterfaceForGeneralDataDoc::MacroParamSystem()
{
    return itsDoc->MacroParamSystem();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::DoSmartToolEditing(const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly)
{
    return itsDoc->DoSmartToolEditing(theSmartToolText, theRelativePathMacroName, fSelectedLocationsOnly);
}

std::string& SmartMetDocumentInterfaceForGeneralDataDoc::SmartToolEditingErrorText()
{
    return itsDoc->SmartToolEditingErrorText();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::RemoveMacroParam(const std::string &theName)
{
    itsDoc->RemoveMacroParam(theName);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::AddMacroParamToView(unsigned int theDescTopIndex, int theViewRow, const std::string &theName)
{
    itsDoc->AddMacroParamToView(theDescTopIndex, theViewRow, theName);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::AddMacroParamToCrossSectionView(int theViewRow, const std::string &theName)
{
    itsDoc->AddMacroParamToCrossSectionView(theViewRow, theName);
}

const std::string& SmartMetDocumentInterfaceForGeneralDataDoc::GetLatestMacroParamErrorText()
{
    return itsDoc->GetLatestMacroParamErrorText();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::RemoveAllParamsFromMapViewRow(unsigned int theDescTopIndex, int theRowIndex)
{
    itsDoc->RemoveAllParamsFromMapViewRow(theDescTopIndex, theRowIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::RemoveAllParamsFromCrossSectionViewRow(int theRowIndex)
{
    itsDoc->RemoveAllParamsFromCrossSectionViewRow(theRowIndex);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetMacroParamDataGridSize(int xSize, int ySize)
{
    itsDoc->SetMacroParamDataGridSize(xSize, ySize);
}

const std::vector<std::string>& SmartMetDocumentInterfaceForGeneralDataDoc::SmartToolFileNames(bool updateList)
{
    return itsDoc->SmartToolFileNames(updateList);
}

NFmiAutoComplete& SmartMetDocumentInterfaceForGeneralDataDoc::AutoComplete()
{
    return itsDoc->AutoComplete();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::InvalidateMapView(bool bErase)
{
    itsDoc->InvalidateMapView(bErase);
}

CtrlViewUtils::GraphicalInfo& SmartMetDocumentInterfaceForGeneralDataDoc::GetGraphicalInfo(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getGraphicalInfo(theMapViewDescTopIndex);
}

AddParams::ParameterSelectionSystem& SmartMetDocumentInterfaceForGeneralDataDoc::ParameterSelectionSystem()
{
    return itsDoc->ParameterSelectionSystem();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UpdateParameterSelectionSystem()
{
    itsDoc->UpdateParameterSelectionSystem();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ExecuteCommand(const NFmiMenuItem &theMenuItem, int theViewIndex, int theViewTypeId)
{
    return itsDoc->ExecuteCommand(theMenuItem, theViewIndex, theViewTypeId);
}

int SmartMetDocumentInterfaceForGeneralDataDoc::DataToDBCheckMethod()
{
    return itsDoc->DataToDBCheckMethod();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::DataToDBCheckMethod(int newValue)
{
    return itsDoc->DataToDBCheckMethod(newValue);
}

NFmiEditDataUserList& SmartMetDocumentInterfaceForGeneralDataDoc::EditDataUserList()
{
    return itsDoc->EditDataUserList();
}

NFmiHelpEditorSystem& SmartMetDocumentInterfaceForGeneralDataDoc::HelpEditorSystem()
{
    return itsDoc->HelpEditorSystem();
}

int SmartMetDocumentInterfaceForGeneralDataDoc::SatelDataRefreshTimerInMinutes()
{
    return itsDoc->SatelDataRefreshTimerInMinutes();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SatelDataRefreshTimerInMinutes(int newValue)
{
    itsDoc->SatelDataRefreshTimerInMinutes(newValue);
}

const NFmiPoint& SmartMetDocumentInterfaceForGeneralDataDoc::StationDataGridSize()
{
    return itsDoc->StationDataGridSize();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::StationDataGridSize(const NFmiPoint &newValue)
{
    itsDoc->StationDataGridSize(newValue);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::DoAutoLoadDataAtStartUp() const
{
    return itsDoc->DoAutoLoadDataAtStartUp();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::DoAutoLoadDataAtStartUp(bool newValue)
{
    itsDoc->DoAutoLoadDataAtStartUp(newValue);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetMapViewCacheSize(double theNewSizeInMB)
{
    itsDoc->GetCombinedMapHandler()->setMapViewCacheSize(theNewSizeInMB);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ReloadFixedDrawParams()
{
    itsDoc->ReloadFixedDrawParams();
}

Q2ServerInfo& SmartMetDocumentInterfaceForGeneralDataDoc::GetQ2ServerInfo()
{
    return itsDoc->GetQ2ServerInfo();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::LastSelectedSynopWmoId(int theWmoId)
{
    itsDoc->LastSelectedSynopWmoId(theWmoId);
}

int SmartMetDocumentInterfaceForGeneralDataDoc::LastSelectedSynopWmoId()
{
    return itsDoc->LastSelectedSynopWmoId();
}

const NFmiPoint& SmartMetDocumentInterfaceForGeneralDataDoc::GetSynopHighlightLatlon()
{
    return itsDoc->GetSynopHighlightLatlon();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetHighlightedSynopStation(const NFmiPoint &theLatlon, int theWmoId, bool fShowHighlight)
{
    itsDoc->SetHighlightedSynopStation(theLatlon, theWmoId, fShowHighlight);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::SynopDataGridViewOn()
{
    return itsDoc->SynopDataGridViewOn();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SynopDataGridViewOn(bool newState)
{
    itsDoc->SynopDataGridViewOn(newState);
}

std::vector<boost::shared_ptr<NFmiFastQueryInfo>> SmartMetDocumentInterfaceForGeneralDataDoc::GetSortedSynopInfoVector(int theProducerId, int theProducerId2, int theProducerId3, int theProducerId4)
{
    return itsDoc->GetSortedSynopInfoVector(theProducerId, theProducerId2, theProducerId3, theProducerId4);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews)
{
    itsDoc->ForceDrawOverBitmapThings(originalCallerDescTopIndex, doOriginalView, doAllOtherMapViews);
}

NFmiLocationSelectionTool* SmartMetDocumentInterfaceForGeneralDataDoc::LocationSelectionTool2()
{
    return itsDoc->LocationSelectionTool2();
}

NFmiParamBag& SmartMetDocumentInterfaceForGeneralDataDoc::AllStaticParams()
{
    return itsDoc->AllStaticParams();
}

NFmiSynopPlotSettings* SmartMetDocumentInterfaceForGeneralDataDoc::SynopPlotSettings()
{
    return itsDoc->SynopPlotSettings();
}

NFmiSynopStationPrioritySystem* SmartMetDocumentInterfaceForGeneralDataDoc::SynopStationPrioritySystem()
{
    return itsDoc->SynopStationPrioritySystem();
}

std::vector<NFmiLightWeightViewSettingMacro>& SmartMetDocumentInterfaceForGeneralDataDoc::ViewMacroDescriptionList()
{
    return itsDoc->ViewMacroDescriptionList();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::RemoveViewMacro(const std::string& theMacroName)
{
    return itsDoc->RemoveViewMacro(theMacroName);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::RefreshViewMacroList()
{
    itsDoc->RefreshViewMacroList();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::LoadViewMacro(const std::string &theName)
{
    return itsDoc->LoadViewMacro(theName);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::StoreViewMacro(const std::string & theAbsoluteMacroFilePath, const std::string &theDescription)
{
    itsDoc->StoreViewMacro(theAbsoluteMacroFilePath, theDescription);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::CreateNewViewMacroDirectory(const std::string &thePath)
{
    return itsDoc->CreateNewViewMacroDirectory(thePath);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UndoViewMacro()
{
    itsDoc->UndoViewMacro();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::RedoViewMacro()
{
    itsDoc->RedoViewMacro();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::IsUndoableViewMacro()
{
    return itsDoc->IsUndoableViewMacro();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::IsRedoableViewMacro()
{
    return itsDoc->IsRedoableViewMacro();
}

const std::vector<std::string>& SmartMetDocumentInterfaceForGeneralDataDoc::CorruptedViewMacroFileList() const
{
    return itsDoc->CorruptedViewMacroFileList();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ChangeCurrentViewMacroDirectory(const std::string & theDirectoryName, bool fUseRootPathAsBase)
{
    itsDoc->ChangeCurrentViewMacroDirectory(theDirectoryName, fUseRootPathAsBase);
}

std::string SmartMetDocumentInterfaceForGeneralDataDoc::GetRelativeViewMacroPath()
{
    return itsDoc->GetRelativeViewMacroPath();
}

const std::vector<std::string>& SmartMetDocumentInterfaceForGeneralDataDoc::ViewMacroFileNames(bool updateList)
{
    return itsDoc->ViewMacroFileNames(updateList);
}

NFmiWindTableSystem& SmartMetDocumentInterfaceForGeneralDataDoc::WindTableSystem()
{
    return itsDoc->WindTableSystem();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SelectLocations(unsigned int theDescTopIndex, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const boost::shared_ptr<NFmiArea> &theMapArea,
    const NFmiPoint& theLatLon, const NFmiMetTime &theTime, int theSelectionCombineFunction, unsigned long theMask
    , bool fMakeMTAModeAdd, bool fDoOnlyMTAModeAdd)
{
    itsDoc->SelectLocations(theDescTopIndex, theInfo, theMapArea,
        theLatLon, theTime, theSelectionCombineFunction, theMask
        , fMakeMTAModeAdd, fDoOnlyMTAModeAdd);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UpdateRowInLockedDescTops(unsigned int theOrigDescTopIndex)
{
    itsDoc->GetCombinedMapHandler()->updateRowInLockedDescTops(theOrigDescTopIndex);
}

int SmartMetDocumentInterfaceForGeneralDataDoc::GetTimeRangeForWarningMessagesOnMapViewInMinutes()
{
    return itsDoc->GetTimeRangeForWarningMessagesOnMapViewInMinutes();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::MakeControlPointAcceleratorAction(ControlPointAcceleratorActions action, const std::string &updateMessage)
{
    return itsDoc->MakeControlPointAcceleratorAction(action, updateMessage);
}

NFmiMacroParamDataCache& SmartMetDocumentInterfaceForGeneralDataDoc::MacroParamDataCache()
{
    return itsDoc->MacroParamDataCache();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::DoMapViewOnSize(int mapViewDescTopIndex, const NFmiPoint &clientPixelSize, CDC* pDC)
{
    return itsDoc->DoMapViewOnSize(mapViewDescTopIndex, clientPixelSize, pDC);
}

NFmiGdiPlusImageMapHandler* SmartMetDocumentInterfaceForGeneralDataDoc::GetMapHandlerInterface(int mapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(mapViewDescTopIndex)->MapHandler();
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::ChangeTime(int theTypeOfChange, FmiDirection theDirection, unsigned long theMapViewIndex, double theAmountOfChange)
{
    return itsDoc->GetCombinedMapHandler()->changeTime(theTypeOfChange, theDirection, theMapViewIndex, theAmountOfChange);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::SetHatchingToolmasterEpsilonFactor(float newEpsilonFactor)
{
    ApplicationWinRegistry().HatchingToolmasterEpsilonFactor(newEpsilonFactor);
    if(ApplicationInterface::GetApplicationInterfaceImplementation)
        ApplicationInterface::GetApplicationInterfaceImplementation()->SetHatchingToolmasterEpsilonFactor(newEpsilonFactor);
}

CombinedMapHandlerInterface& SmartMetDocumentInterfaceForGeneralDataDoc::GetCombinedMapHandlerInterface()
{
    return *itsDoc->GetCombinedMapHandler();
}

NFmiParameterInterpolationFixer& SmartMetDocumentInterfaceForGeneralDataDoc::ParameterInterpolationFixer()
{
    return itsDoc->ParameterInterpolationFixer();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ApplyUpdatedViewsFlag(SmartMetViewId updatedViewsFlag)
{
    if(ApplicationInterface::GetApplicationInterfaceImplementation)
        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(updatedViewsFlag);
}

void SmartMetDocumentInterfaceForGeneralDataDoc::UpdateMacroParamDataGridSizeAfterVisualizationOptimizationsChanged()
{
    return itsDoc->UpdateMacroParamDataGridSizeAfterVisualizationOptimizationsChanged();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::OpenVisualizationsettingsDialog()
{
    if(ApplicationInterface::GetApplicationInterfaceImplementation)
        ApplicationInterface::GetApplicationInterfaceImplementation()->OpenVisualizationsettingsDialog();
}

std::shared_ptr<NFmiViewSettingMacro> SmartMetDocumentInterfaceForGeneralDataDoc::CurrentViewMacro()
{
    return itsDoc->CurrentViewMacro();
}

void SmartMetDocumentInterfaceForGeneralDataDoc::ToggleVirtualTimeMode(const std::string& logMessage)
{
    itsDoc->ToggleVirtualTimeMode(logMessage);
}

bool SmartMetDocumentInterfaceForGeneralDataDoc::VirtualTimeUsed() const
{
    return itsDoc->VirtualTimeUsed();
}

NFmiMacroParamDataGenerator& SmartMetDocumentInterfaceForGeneralDataDoc::GetMacroParamDataGenerator()
{
    return itsDoc->GetMacroParamDataGenerator();
}

#ifndef DISABLE_CPPRESTSDK
// =================================================

HakeMessage::Main& SmartMetDocumentInterfaceForGeneralDataDoc::WarningCenterSystem()
{
    return itsDoc->WarningCenterSystem();
}

#endif // DISABLE_CPPRESTSDK
