
#include "NFmiGenDocDataAdapter.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiWindTableSystem.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "NFmiGriddingHelperInterface.h"
#include "NFmiEditorControlPointManager.h"
#include "FmiSmartMetEditingMode.h"
#include "NFmiApplicationWinRegistry.h"
#include "CombinedMapHandlerInterface.h"

NFmiGenDocDataAdapter::NFmiGenDocDataAdapter(NFmiEditMapGeneralDataDoc *theDoc)
:itsDoc(theDoc)
{
}

NFmiGenDocDataAdapter::~NFmiGenDocDataAdapter(void)
{
}

bool NFmiGenDocDataAdapter::CheckValidationFromSettings(void)
{
	return true; // genDocin kanssa tehd‰‰n aina asetuksien mukaiset validaatiot, joten t‰m‰ palauttaa aina true:n!!
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiGenDocDataAdapter::EditedInfo(void)
{
	return itsDoc->EditedSmartInfo();
}

boost::shared_ptr<NFmiAreaMaskList> NFmiGenDocDataAdapter::ParamMaskList(void)
{
	return itsDoc->ParamMaskListMT();
}

bool NFmiGenDocDataAdapter::UseMasksInTimeSerialViews(void) 
{
	return itsDoc->IsMasksUsedInTimeSerialViews();
}

NFmiAnalyzeToolData& NFmiGenDocDataAdapter::AnalyzeToolData(void)
{
	return itsDoc->AnalyzeToolData();
}

NFmiInfoOrganizer* NFmiGenDocDataAdapter::InfoOrganizer(void)
{
	return itsDoc->InfoOrganizer();
}

bool NFmiGenDocDataAdapter::IsEditedDataInReadOnlyMode()
{
	return itsDoc->IsEditedDataInReadOnlyMode();
}

int NFmiGenDocDataAdapter::DoMessageBox(const std::string &theMessage, const std::string &theTitle, unsigned int theMessageBoxType) 
{
	return itsDoc->DoMessageBox(theMessage, theTitle, theMessageBoxType);
}

void NFmiGenDocDataAdapter::MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
    itsDoc->GetCombinedMapHandler()->mapViewDirty(theDescTopIndex, makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, clearMacroParamDataCache, clearEditedDataDependentCaches, updateMapViewDrawingLayers);
}

void NFmiGenDocDataAdapter::WindTableSystemMustaUpdateTable(bool newState)
{
	itsDoc->WindTableSystem().MustaUpdateTable(newState);
}

boost::shared_ptr<NFmiDrawParam> NFmiGenDocDataAdapter::GetUsedDrawParamForEditedData(const NFmiDataIdent &theDataIdent)
{
	return itsDoc->GetCombinedMapHandler()->getUsedDrawParamForEditedData(theDataIdent);
}

NFmiMetEditorOptionsData& NFmiGenDocDataAdapter::MetEditorOptionsData(void)
{
	return itsDoc->MetEditorOptionsData();
}

boost::shared_ptr<NFmiEditorControlPointManager> NFmiGenDocDataAdapter::CPManager(bool getOldSchoolCPManager)
{
	return itsDoc->CPManager(getOldSchoolCPManager);
}

CtrlViewUtils::FmiSmartMetEditingMode NFmiGenDocDataAdapter::SmartMetEditingMode(void)
{
	return static_cast<CtrlViewUtils::FmiSmartMetEditingMode>(itsDoc->SmartMetEditingMode());
}

void NFmiGenDocDataAdapter::SmartMetEditingMode(CtrlViewUtils::FmiSmartMetEditingMode newValue, bool modifySettings)
{
	itsDoc->SmartMetEditingMode(static_cast<CtrlViewUtils::FmiSmartMetEditingMode>(newValue), modifySettings);
}

int NFmiGenDocDataAdapter::FilteringParameterUsageState(void)
{
	return itsDoc->FilteringParameterUsageState();
}

void NFmiGenDocDataAdapter::FilteringParameterUsageState(int newValue)
{
	itsDoc->FilteringParameterUsageState(newValue);
}

boost::shared_ptr<NFmiDrawParam> NFmiGenDocDataAdapter::ActiveDrawParamFromActiveRow(unsigned int theDescTopIndex)
{
	return itsDoc->GetCombinedMapHandler()->activeDrawParamFromActiveRow(theDescTopIndex);
}

int NFmiGenDocDataAdapter::AbsoluteActiveViewRow(unsigned int theDescTopIndex)
{
	return itsDoc->GetCombinedMapHandler()->absoluteActiveViewRow(theDescTopIndex);
}

NFmiParamBag& NFmiGenDocDataAdapter::FilteringParamBag(void)
{
	return itsDoc->FilteringParamBag();
}

const NFmiTimeDescriptor& NFmiGenDocDataAdapter::EditedDataTimeDescriptor(void)
{
	return itsDoc->EditedDataTimeDescriptor();
}

const NFmiMetTime& NFmiGenDocDataAdapter::ActiveViewTime(void)
{
	return itsDoc->ActiveViewTime();
}

/*
void NFmiGenDocDataAdapter::RefreshMasks(void)
{
	itsDoc->RefreshMasks();
}
*/

boost::shared_ptr<NFmiTimeDescriptor> NFmiGenDocDataAdapter::CreateDataFilteringTimeDescriptor(boost::shared_ptr<NFmiFastQueryInfo>& theEditedData)
{
	NFmiTimeDescriptor tmpTimeDesc(theEditedData->TimeDescriptor());
	return boost::shared_ptr<NFmiTimeDescriptor>(new NFmiTimeDescriptor(tmpTimeDesc.GetIntersection(TimeFilterStartTime(), TimeFilterEndTime())));
}

NFmiSmartToolInfo* NFmiGenDocDataAdapter::SmartToolInfo(void)
{
	return itsDoc->SmartToolInfo();
}

std::string& NFmiGenDocDataAdapter::SmartToolEditingErrorText(void)
{
	return itsDoc->SmartToolEditingErrorText();
}

class GeneralDataDocGridding : public NFmiGriddingHelperInterface
{
    NFmiEditMapGeneralDataDoc *itsDoc;
public:
    GeneralDataDocGridding(NFmiEditMapGeneralDataDoc *theDoc)
        :itsDoc(theDoc)
    {}

    void MakeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea) override
    {
        itsDoc->GetCombinedMapHandler()->makeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
    }
    NFmiIgnoreStationsData& IgnoreStationsData() override
    {
        return itsDoc->IgnoreStationsData();
    }

    const NFmiGriddingProperties& GriddingProperties(bool getEditingRelatedProperties) override
    {
        return itsDoc->ApplicationWinRegistry().GriddingProperties(getEditingRelatedProperties);
    }

};

NFmiGriddingHelperInterface* NFmiGenDocDataAdapter::GetGriddingHelper(void)
{
    static GeneralDataDocGridding generalDataDocGridding(itsDoc);
    return &generalDataDocGridding;
}

void NFmiGenDocDataAdapter::LastBrushedViewRealRowIndex(int newRealRowIndex)
{
	itsDoc->LastBrushedViewRealRowIndex(newRealRowIndex);
}

bool NFmiGenDocDataAdapter::UseMasksWithFilterTool(void)
{
	return itsDoc->UseMasksWithFilterTool();
}

int NFmiGenDocDataAdapter::TestFilterUsedMask(void)
{
	return itsDoc->GetTestFilterMask();
}

NFmiGrid* NFmiGenDocDataAdapter::ClipBoardData(void)
{
	return itsDoc->ClipBoardData();
}

int NFmiGenDocDataAdapter::FilterFunction(void)
{
	return itsDoc->FilterFunction();
}

const NFmiMetTime& NFmiGenDocDataAdapter::TimeFilterStartTime(void)
{
	return itsDoc->TimeFilterStartTime();
}

const NFmiMetTime& NFmiGenDocDataAdapter::TimeFilterEndTime(void)
{
	return itsDoc->TimeFilterEndTime();
}

const NFmiRect& NFmiGenDocDataAdapter::AreaFilterRange(int index)
{
	return itsDoc->AreaFilterRange(index);
}

bool NFmiGenDocDataAdapter::UseTimeInterpolation(void)
{
	return itsDoc->UseTimeInterpolation();
}

const NFmiPoint& NFmiGenDocDataAdapter::TimeFilterRangeStart(void)
{
	return itsDoc->TimeFilterRangeStart();
}

const NFmiPoint& NFmiGenDocDataAdapter::TimeFilterRangeEnd(void)
{
	return itsDoc->TimeFilterRangeEnd();
}

bool NFmiGenDocDataAdapter::IsSmoothTimeShiftPossible(void)
{
	return itsDoc->IsSmoothTimeShiftPossible();
}

int NFmiGenDocDataAdapter::DataToDBCheckMethod(void)
{
	return itsDoc->DataToDBCheckMethod();
}

NFmiDataLoadingInfo& NFmiGenDocDataAdapter::GetUsedDataLoadingInfo(void)
{
	return itsDoc->GetUsedDataLoadingInfo();
}

NFmiHelpEditorSystem& NFmiGenDocDataAdapter::HelpEditorSystem(void)
{
	return itsDoc->HelpEditorSystem();
}

bool NFmiGenDocDataAdapter::StoreData(const std::string& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave)
{
	return itsDoc->StoreData(theFileName, theSmartInfo, askForSave);
}

bool NFmiGenDocDataAdapter::StoreData(bool fDoSaveTmpRename, const std::string& theFileName, NFmiQueryData *theData)
{
	return itsDoc->StoreData(fDoSaveTmpRename, theFileName, theData, false);
}

NFmiHelpDataInfoSystem* NFmiGenDocDataAdapter::HelpDataInfoSystem(void)
{
	return itsDoc->HelpDataInfoSystem();
}

bool NFmiGenDocDataAdapter::DataLoadingOK(bool noError)
{
	return itsDoc->DataLoadingOK(noError);
}

void NFmiGenDocDataAdapter::AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern, NFmiInfoData::Type theType, const std::string& theNotificationStr, bool loadFromFileState, bool& dataWasDeleted)
{
	itsDoc->AddQueryData(theData, theDataFileName, theDataFilePattern, theType, theNotificationStr, loadFromFileState, dataWasDeleted);
}

bool NFmiGenDocDataAdapter::WarnIfCantSaveWorkingFile(void)
{
	return itsDoc->WarnIfCantSaveWorkingFile();
}

NFmiHPlaceDescriptor* NFmiGenDocDataAdapter::PossibleUsedDataLoadingGrid(void)
{
	return itsDoc->PossibleUsedDataLoadingGrid();
}

bool NFmiGenDocDataAdapter::UseEditedDataParamDescriptor(void) 
{
	return itsDoc->UseEditedDataParamDescriptor();
}

NFmiParamDescriptor& NFmiGenDocDataAdapter::EditedDataParamDescriptor(void)
{
	return itsDoc->EditedDataParamDescriptor();
}

std::vector<int>& NFmiGenDocDataAdapter::DataLoadingProducerIndexVector(void) 
{
	return itsDoc->DataLoadingProducerIndexVector();
}

void NFmiGenDocDataAdapter::EditedDataNeedsToBeLoaded(bool newValue)
{
	itsDoc->EditedDataNeedsToBeLoaded(newValue);
}

void NFmiGenDocDataAdapter::PutWarningFlagTimerOn(void)
{
	itsDoc->PutWarningFlagTimerOn();
}

NFmiModelDataBlender& NFmiGenDocDataAdapter::ModelDataBlender(void)
{
	return itsDoc->ModelDataBlender();
}

NFmiProducerIdLister& NFmiGenDocDataAdapter::ProducerIdLister(void)
{
	return itsDoc->ProducerIdLister();
}

void NFmiGenDocDataAdapter::DoDataLoadingProblemsDlg(const std::string &theMessage)
{
	itsDoc->DoDataLoadingProblemsDlg(theMessage);
}

void NFmiGenDocDataAdapter::DataModificationInProgress(bool newValue)
{
	itsDoc->DataModificationInProgress(newValue);
}

bool NFmiGenDocDataAdapter::DataModificationInProgress(void)
{
	return itsDoc->DataModificationInProgress();
}

const std::string& NFmiGenDocDataAdapter::GetCurrentSmartToolMacro(void)
{
	return itsDoc->GetCurrentSmartToolMacro();
}

std::shared_ptr<NFmiMacroParamSystem> NFmiGenDocDataAdapter::MacroParamSystem(void)
{
	return itsDoc->MacroParamSystem();
}

void NFmiGenDocDataAdapter::SetLatestMacroParamErrorText(const std::string& theErrorText)
{
	itsDoc->SetLatestMacroParamErrorText(theErrorText);
}

void NFmiGenDocDataAdapter::SetMacroErrorText(const std::string &theErrorStr)
{
	itsDoc->SetMacroErrorText(theErrorStr);
}

boost::shared_ptr<NFmiArea> NFmiGenDocDataAdapter::MapHandlerArea(bool fGetZoomedArea)
{
	auto* mapHandler = itsDoc->GetCombinedMapHandler()->getMapViewDescTop(0)->MapHandler();
	if(fGetZoomedArea)
		return mapHandler->Area();
	else
		return mapHandler->TotalArea();
}

FmiLanguage NFmiGenDocDataAdapter::Language(void)
{
	return itsDoc->Language();
}

const NFmiRect& NFmiGenDocDataAdapter::CPGridCropRect(void)
{
	return itsDoc->CPGridCropRect();
}

bool NFmiGenDocDataAdapter::UseCPGridCrop(void)
{
	return itsDoc->UseCPGridCrop();
}

const NFmiPoint& NFmiGenDocDataAdapter::CPGridCropMargin(void)
{
	return itsDoc->CPGridCropMargin();
}

MultiProcessClientData& NFmiGenDocDataAdapter::GetMultiProcessClientData(void)
{
	return itsDoc->GetMultiProcessClientData();
}

const std::string& NFmiGenDocDataAdapter::GetSmartMetGuid(void)
{
	return itsDoc->GetSmartMetGuid();
}

bool NFmiGenDocDataAdapter::UseMultiProcessCpCalc(void) 
{
    return itsDoc->UseMultiProcessCpCalc();
}

bool NFmiGenDocDataAdapter::MakeSureToolMasterPoolIsRunning(void)
{
    return itsDoc->MakeSureToolMasterPoolIsRunning();
}

bool NFmiGenDocDataAdapter::IsWorkingDataSaved()
{
    return itsDoc->IsWorkingDataSaved();
}

void NFmiGenDocDataAdapter::LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption)
{
    itsDoc->LogAndWarnUser(theMessageStr, theDialogTitleStr, severity, category, justLog, addAbortOption);
}

boost::shared_ptr<NFmiArea> NFmiGenDocDataAdapter::GetUsedMapViewArea(int theMapViewDescTopIndex)
{
	return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->MapHandler()->Area();
}
