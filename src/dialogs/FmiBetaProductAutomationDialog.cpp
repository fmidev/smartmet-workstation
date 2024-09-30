#include "stdafx.h"
#include "FmiBetaProductAutomationDialog.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "NFmiBetaProductSystem.h"
#include "GridCellCheck.h"
#include "NFmiFileSystem.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiPathUtils.h"
#include "boost\math\special_functions\round.hpp"

// ***************************************
// NFmiBetaAutomationGridCtrl grid-control
// ***************************************

IMPLEMENT_DYNCREATE(NFmiBetaAutomationGridCtrl, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiBetaAutomationGridCtrl, CGridCtrl)
    ON_WM_RBUTTONUP()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void NFmiBetaAutomationGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
    CGridCtrl::OnRButtonUp(nFlags, point);

    // Tarkoitus on deselectoida kaikki
    SetSelectedRange(-1, -1, -1, -1, TRUE, TRUE);

    if(itsRightClickUpCallback)
        itsRightClickUpCallback(GetCellFromPt(point));
}

void NFmiBetaAutomationGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    CGridCtrl::OnLButtonUp(nFlags, point);

    if(itsLeftClickUpCallback)
        itsLeftClickUpCallback(GetCellFromPt(point));
}

// ***************************************
// CFmiBetaProductAutomationDialog dialog
// ***************************************

IMPLEMENT_DYNAMIC(CFmiBetaProductAutomationDialog, CTabPageSSL)

CFmiBetaProductAutomationDialog::CFmiBetaProductAutomationDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
: CTabPageSSL(CFmiBetaProductAutomationDialog::IDD, pParent)
, itsGridCtrl()
,itsHeaders()
,fGridControlInitialized(false)
,fDialogInitialized(false)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsBetaProductionSystem(&smartMetDocumentInterface->BetaProductionSystem())
,fAutomationModeOn(FALSE)
, itsBetaProductPathErrorStringU_(_T(""))
, itsBetaProductPathU_(_T(""))
, itsBetaProductAutomation(std::make_shared<NFmiBetaProductAutomation>())
, itsBetaProductAutomationComparisonObject()
, itsTriggerModeIndex(0)
, itsBetaAutomationFixedTimesStringU_(_T(""))
, itsBetaAutomationTimeStepStringU_(_T(""))
, itsBetaAutomationFirstRunOfDayValueU_(_T(""))
, itsBetaAutomationTriggerModeInfoStringU_(_T(""))
, itsBetaAutomationStartTimeModeIndex(0)
, itsBetaAutomationEndTimeModeIndex(0)
, itsBetaAutomationStartTimeClockOffsetValueU_(_T(""))
, itsBetaAutomationEndTimeClockOffsetValueU_(_T(""))
, itsBetaAutomationStartTimeClockOffsetErrorU_(_T(""))
, itsBetaAutomationEndTimeClockOffsetErrorU_(_T(""))
, itsAutomationNameU_(_T(""))
, itsAutomationListNameU_(_T(""))
, itsBetaAutomationDataTriggersStringU_(_T(""))
{

}

CFmiBetaProductAutomationDialog::~CFmiBetaProductAutomationDialog()
{
}

void CFmiBetaProductAutomationDialog::DoDataExchange(CDataExchange* pDX)
{
    CTabPageSSL::DoDataExchange(pDX);
    DDX_GridControl(pDX, IDC_CUSTOM_GRID_AUTOMATION_LIST, itsGridCtrl);
    DDX_Check(pDX, IDC_CHECK_AUTOMATIION_MODE_ON, fAutomationModeOn);
    DDX_Text(pDX, IDC_STATIC_BETA_PRODUCT_ERROR_TEXT, itsBetaProductPathErrorStringU_);
    DDX_Text(pDX, IDC_EDIT_BETA_PRODUCT_PATH, itsBetaProductPathU_);
    DDX_Radio(pDX, IDC_RADIO_TRIGGER_MODE_FIXED_TIMES, itsTriggerModeIndex);
    DDX_Text(pDX, IDC_EDIT_BETA_AUTOMATION_FIXED_TIMES, itsBetaAutomationFixedTimesStringU_);
    DDX_Text(pDX, IDC_EDIT_BETA_AUTOMATION_TIME_STEP_VALUE, itsBetaAutomationTimeStepStringU_);
    DDX_Text(pDX, IDC_EDIT_BETA_AUTOMATION_FIRST_RUN_OF_DAY_VALUE, itsBetaAutomationFirstRunOfDayValueU_);
    DDX_Text(pDX, IDC_STATIC_BETA_AUTOMATION_TRIGGER_MODE_INFO_STRING, itsBetaAutomationTriggerModeInfoStringU_);
    DDX_Radio(pDX, IDC_RADIO_BETA_AUTOMATION_START_TIME_FROM_PRODUCT, itsBetaAutomationStartTimeModeIndex);
    DDX_Radio(pDX, IDC_RADIO_BETA_AUTOMATION_END_TIME_FROM_PRODUCT, itsBetaAutomationEndTimeModeIndex);
    DDX_Text(pDX, IDC_EDIT_BETA_AUTOMATION_START_TIME_WALL_CLOCK_OFFSET_VALUE, itsBetaAutomationStartTimeClockOffsetValueU_);
    DDX_Text(pDX, IDC_EDIT_BETA_AUTOMATION_END_TIME_WALL_CLOCK_OFFSET_VALUE, itsBetaAutomationEndTimeClockOffsetValueU_);
    DDX_Text(pDX, IDC_STATIC_BETA_AUTOMATION_START_TIME_OFFSET_ERROR_TEXT, itsBetaAutomationStartTimeClockOffsetErrorU_);
    DDX_Text(pDX, IDC_STATIC_BETA_AUTOMATION_END_TIME_OFFSET_ERROR_TEXT, itsBetaAutomationEndTimeClockOffsetErrorU_);
    DDX_Control(pDX, IDC_BUTTON_BETA_AUTOMATION_SAVE, itsBetaAutomationSaveButton);
    DDX_Text(pDX, IDC_STATIC_BETA_AUTOMATION_NAME_VALUE, itsAutomationNameU_);
    DDX_Control(pDX, IDC_BUTTON_BETA_AUTOMATION_SAVE_AS, itsBetaAutomationSaveAsButton);
    DDX_Text(pDX, IDC_STATIC_BETA_AUTOMATION_LIST_NAME_VALUE, itsAutomationListNameU_);
    DDX_Control(pDX, IDC_BUTTON_SAVE_AUTOMATION_LIST, itsAutomationListSaveButton);
    DDX_Control(pDX, IDC_BUTTON_SAVE_AS_AUTOMATION_LIST, itsAutomationListSaveAsButton);
    DDX_Control(pDX, IDC_BUTTON_ADD_EDITED_BETA_AUTOMATION_TO_LIST, itsAddEditedAutomationToListButton);
    DDX_Control(pDX, IDC_BUTTON_REMOVE_BETA_AUTOMATION_FROM_LIST, itsRemoveAutomationFromListButton);
    DDX_Text(pDX, IDC_EDIT_BETA_AUTOMATION_DATA_TRIGGERS, itsBetaAutomationDataTriggersStringU_);
}


BEGIN_MESSAGE_MAP(CFmiBetaProductAutomationDialog, CTabPageSSL)
    ON_BN_CLICKED(IDC_BUTTON_BETA_PRODUCT_PATH_BROWSE, &CFmiBetaProductAutomationDialog::OnBnClickedButtonBetaProductPathBrowse)
    ON_EN_CHANGE(IDC_EDIT_BETA_PRODUCT_PATH, &CFmiBetaProductAutomationDialog::OnEnChangeEditBetaProductPath)
    ON_WM_DESTROY()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_RADIO_TRIGGER_MODE_FIXED_TIMES, &CFmiBetaProductAutomationDialog::OnBnClickedRadioTriggerModeFixedTimes)
    ON_BN_CLICKED(IDC_RADIO_TRIGGER_MODE_TIMES_STEP, &CFmiBetaProductAutomationDialog::OnBnClickedRadioTriggerModeTimesStep)
    ON_BN_CLICKED(IDC_RADIO_TRIGGER_MODE_DATA_EVENT, &CFmiBetaProductAutomationDialog::OnBnClickedRadioTriggerModeDataEvent)
    ON_EN_CHANGE(IDC_EDIT_BETA_AUTOMATION_TIME_STEP_VALUE, &CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationTimeStepValue)
    ON_EN_CHANGE(IDC_EDIT_BETA_AUTOMATION_FIXED_TIMES, &CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationFixedTimes)
    ON_EN_CHANGE(IDC_EDIT_BETA_AUTOMATION_FIRST_RUN_OF_DAY_VALUE, &CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationFirstRunOfDayValue)
    ON_BN_CLICKED(IDC_RADIO_BETA_AUTOMATION_START_TIME_FROM_PRODUCT, &CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationStartTimeFromProduct)
    ON_BN_CLICKED(IDC_RADIO_BETA_AUTOMATION_START_TIME_WALL_CLOCK_OFFSET, &CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationStartTimeWallClockOffset)
    ON_BN_CLICKED(IDC_RADIO_BETA_AUTOMATION_START_TIME_FROM_MODEL, &CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationStartTimeFromModel)
    ON_BN_CLICKED(IDC_RADIO_BETA_AUTOMATION_END_TIME_FROM_PRODUCT, &CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationEndTimeFromProduct)
    ON_BN_CLICKED(IDC_RADIO_BETA_AUTOMATION_END_TIME_WALL_CLOCK_OFFSET, &CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationEndTimeWallClockOffset)
    ON_BN_CLICKED(IDC_RADIO_BETA_AUTOMATION_END_TIME_FROM_MODEL, &CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationEndTimeFromModel)
    ON_BN_CLICKED(IDC_BUTTON_BETA_AUTOMATION_SAVE, &CFmiBetaProductAutomationDialog::OnBnClickedButtonBetaAutomationSave)
    ON_BN_CLICKED(IDC_BUTTON_BETA_AUTOMATION_LOAD, &CFmiBetaProductAutomationDialog::OnBnClickedButtonBetaAutomationLoad)
    ON_EN_CHANGE(IDC_EDIT_BETA_AUTOMATION_START_TIME_WALL_CLOCK_OFFSET_VALUE, &CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationStartTimeWallClockOffsetValue)
    ON_EN_CHANGE(IDC_EDIT_BETA_AUTOMATION_END_TIME_WALL_CLOCK_OFFSET_VALUE, &CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationEndTimeWallClockOffsetValue)
    ON_BN_CLICKED(IDC_BUTTON_ADD_BETA_AUTOMATION_TO_LIST, &CFmiBetaProductAutomationDialog::OnBnClickedButtonAddBetaAutomationToList)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE_BETA_AUTOMATION_FROM_LIST, &CFmiBetaProductAutomationDialog::OnBnClickedButtonRemoveBetaAutomationFromList)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_AUTOMATION_LIST, &CFmiBetaProductAutomationDialog::OnBnClickedButtonSaveAutomationList)
    ON_BN_CLICKED(IDC_BUTTON_LOAD_AUTOMATION_LIST, &CFmiBetaProductAutomationDialog::OnBnClickedButtonLoadAutomationList)
    ON_BN_CLICKED(IDC_BUTTON_BETA_AUTOMATION_SAVE_AS, &CFmiBetaProductAutomationDialog::OnBnClickedButtonBetaAutomationSaveAs)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_AS_AUTOMATION_LIST, &CFmiBetaProductAutomationDialog::OnBnClickedButtonSaveAsAutomationList)
    ON_BN_CLICKED(IDC_BUTTON_ADD_EDITED_BETA_AUTOMATION_TO_LIST, &CFmiBetaProductAutomationDialog::OnBnClickedButtonAddEditedBetaAutomationToList)
    ON_BN_CLICKED(IDC_CHECK_AUTOMATIION_MODE_ON, &CFmiBetaProductAutomationDialog::OnBnClickedCheckAutomatiionModeOn)
    ON_BN_CLICKED(IDC_BUTTON_RUN_SELECTED_AUTOMATION, &CFmiBetaProductAutomationDialog::OnBnClickedButtonRunSelectedAutomation)
    ON_BN_CLICKED(IDC_BUTTON_RUN_ALL_AUTOMATIONS, &CFmiBetaProductAutomationDialog::OnBnClickedButtonRunAllAutomations)
    ON_BN_CLICKED(IDC_BUTTON_RUN_ENABLED_AUTOMATIONS, &CFmiBetaProductAutomationDialog::OnBnClickedButtonRunEnabledAutomations)
    ON_EN_CHANGE(IDC_EDIT_BETA_AUTOMATION_DATA_TRIGGERS, &CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationDataTriggers)
END_MESSAGE_MAP()



static void SetHeaders(CGridCtrl &theGridCtrl, const std::vector<BetaAutomationHeaderParInfo> &theHeaders)
{
    static const COLORREF gFixedBkColor = RGB(239, 235, 222);

    int columnCount = static_cast<int>(theHeaders.size());
    theGridCtrl.SetColumnCount(columnCount);
    theGridCtrl.SetFixedRowCount(1);
    theGridCtrl.SetFixedColumnCount(1);
    theGridCtrl.SetGridLines(GVL_BOTH);
    theGridCtrl.SetListMode(TRUE);
    theGridCtrl.SetHeaderSort(FALSE);
    theGridCtrl.SetFixedBkColor(gFixedBkColor);
    theGridCtrl.SetSingleColSelection(TRUE);
    theGridCtrl.SetSingleRowSelection(TRUE);

    int currentRow = 0;
    // 1. on otsikko rivi on parametrien nimiä varten
    for(int i = 0; i<columnCount; i++)
    {
        theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
        theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
        theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
    }
}


// CFmiBetaProductAutomationDialog message handlers


BOOL CFmiBetaProductAutomationDialog::OnInitDialog()
{
    CTabPageSSL::OnInitDialog();

    itsGridCtrl.SetLeftClickUpCallback(std::bind(&CFmiBetaProductAutomationDialog::SelectedGridCell, this, std::placeholders::_1));
    itsGridCtrl.SetRightClickUpCallback(std::bind(&CFmiBetaProductAutomationDialog::DeselectGridCell, this, std::placeholders::_1));

    InitHeaders();
    SetHeaders(itsGridCtrl, itsHeaders);
    InitDialogTexts();

    InitControlsFromDocument();
    UpdateSelectedAutomationListName();
    UpdateAutomationList();

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiBetaProductAutomationDialog::InitDialogTexts()
{
    SetWindowText(CA2T(::GetDictionaryString("Beta Product Automation").c_str())); // Tämä otsikko ei tule näkyviin koska tämä on oikeasti tabi-sivu
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_AUTOMATION_GROUP_TEXT, "Automated Beta product");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_AUTOMATION_LIST_GROUP_TEXT, "Beta product automation list");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_AUTOMATIION_MODE_ON, "Automation mode on");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_USED_BETA_PRODUCT_GROUP, "Beta product information");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BETA_PRODUCT_PATH_TEXT, "Beta product path");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BETA_PRODUCT_PATH_BROWSE, "Browse B-prod");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_USED_TRIGGER_MODE_GROUP, "Trigger mode information");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRIGGER_MODE_FIXED_TIMES, "Fixed time list");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_FIXED_TIMES_SAMPLE_TEXT, "HH:mm[,HH:mm,...]\n(in Utc)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRIGGER_MODE_TIMES_STEP, "Time step");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BETA_AUTOMATION_TIME_STEP_HEADER, NFmiBetaProductAutomation::RunTimeStepInHoursTitle().c_str());
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BETA_AUTOMATION_FIRST_RUN_OF_DAY_HEADER, NFmiBetaProductAutomation::FirstRunTimeOfDayTitle().c_str());
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRIGGER_MODE_DATA_EVENT, "Data event");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_TRIGGERS_SAMPLE_TEXT, "param_prod[_level] list:\nT_ec, par10_prod240_500");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BETA_AUTOMATION_START_TIME_MODE_GROUP, "Start time mode");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BETA_AUTOMATION_END_TIME_MODE_GROUP, "End time mode");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_BETA_AUTOMATION_START_TIME_FROM_PRODUCT, "Beta product time");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_BETA_AUTOMATION_START_TIME_WALL_CLOCK_OFFSET, "Wall clock offset [h]");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_BETA_AUTOMATION_START_TIME_FROM_MODEL, "Model data time");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_BETA_AUTOMATION_END_TIME_FROM_PRODUCT, "Beta product time");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_BETA_AUTOMATION_END_TIME_WALL_CLOCK_OFFSET, "Wall clock offset [h]");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_BETA_AUTOMATION_END_TIME_FROM_MODEL, "Model data time");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BETA_AUTOMATION_SAVE, "Save automation");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BETA_AUTOMATION_SAVE_AS, "Save as automation");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BETA_AUTOMATION_LOAD, "Load automation");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_ADD_EDITED_BETA_AUTOMATION_TO_LIST, "Add edited");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_ADD_BETA_AUTOMATION_TO_LIST, "Add to list");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REMOVE_BETA_AUTOMATION_FROM_LIST, "Remove");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SAVE_AUTOMATION_LIST, "Save list");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SAVE_AS_AUTOMATION_LIST, "Save as list");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD_AUTOMATION_LIST, "Load list");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BETA_AUTOMATION_NAME_TEXT, "Name:");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RUN_SELECTED_AUTOMATION, "Run selected");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RUN_ENABLED_AUTOMATIONS, "Run all enabled");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RUN_ALL_AUTOMATIONS, "Run all");
}

void CFmiBetaProductAutomationDialog::InitControlsFromDocument()
{
    fAutomationModeOn = itsBetaProductionSystem->AutomationModeOn();
    itsBetaProductPathU_ = CA2T(itsBetaProductionSystem->BetaProductPath().c_str());
    itsTriggerModeIndex = itsBetaProductionSystem->TriggerModeIndex();
    itsBetaAutomationFixedTimesStringU_ = CA2T(itsBetaProductionSystem->FixedTimesString().c_str());
    itsBetaAutomationTimeStepStringU_ = CA2T(itsBetaProductionSystem->AutomationTimeStepInHoursString().c_str());
    itsBetaAutomationFirstRunOfDayValueU_ = CA2T(itsBetaProductionSystem->FirstRunTimeOfDayString().c_str());
    itsBetaAutomationStartTimeModeIndex = itsBetaProductionSystem->StartTimeModeIndex();
    itsBetaAutomationEndTimeModeIndex = itsBetaProductionSystem->EndTimeModeIndex();
    itsBetaAutomationStartTimeClockOffsetValueU_ = CA2T(itsBetaProductionSystem->StartTimeClockOffsetInHoursString().c_str());
    itsBetaAutomationEndTimeClockOffsetValueU_ = CA2T(itsBetaProductionSystem->EndTimeClockOffsetInHoursString().c_str());
    itsAutomationNameU_ = CA2T(itsBetaProductionSystem->AutomationPath().c_str());
    if(!itsAutomationNameU_.IsEmpty())
    {
        itsSelectedAutomationFullFilePath = CT2A(itsAutomationNameU_);
        itsSelectedAutomationFullFilePath += "." + NFmiBetaProductionSystem::BetaAutomationFileExtension();
    }
    itsBetaAutomationDataTriggersStringU_ = CA2T(itsBetaProductionSystem->TriggerDataString().c_str());

    UpdateData(FALSE);

    UpdateBetaProductPathInfo();
    UpdateTriggerModeInfo();
    UpdateStartTimeModeInfo();
    UpdateEndTimeModeInfo();
    MakeAutomationComparisonObject();
}

void CFmiBetaProductAutomationDialog::InitControlsFromLoadedBetaAutomation()
{
    itsBetaProductPathU_ = CA2T(itsBetaProductAutomation->OriginalBetaProductPath().c_str());
    itsBetaProductPathErrorStringU_ = CA2T(itsBetaProductAutomation->BetaProductPathStatusString().c_str());
    itsTriggerModeIndex = itsBetaProductAutomation->TriggerModeInfo().itsTriggerMode;
    itsBetaAutomationFixedTimesStringU_ = CA2T(itsBetaProductAutomation->TriggerModeInfo().itsFixedRunTimesString.c_str());
    itsBetaAutomationTimeStepStringU_ = CA2T(itsBetaProductAutomation->TriggerModeInfo().itsRunTimeStepInHoursString.c_str());
    itsBetaAutomationFirstRunOfDayValueU_ = CA2T(itsBetaProductAutomation->TriggerModeInfo().itsFirstRunTimeOfDayString.c_str());
    itsBetaAutomationTriggerModeInfoStringU_ = CA2T(itsBetaProductAutomation->TriggerModeInfoStatusString().c_str());
    itsBetaAutomationStartTimeModeIndex = itsBetaProductAutomation->StartTimeModeInfo().itsTimeMode;
    itsBetaAutomationEndTimeModeIndex = itsBetaProductAutomation->EndTimeModeInfo().itsTimeMode;
    itsBetaAutomationStartTimeClockOffsetValueU_ = CA2T(itsBetaProductAutomation->StartTimeModeInfo().itsWallClockOffsetInHoursString.c_str());
    itsBetaAutomationEndTimeClockOffsetValueU_ = CA2T(itsBetaProductAutomation->EndTimeModeInfo().itsWallClockOffsetInHoursString.c_str());
    itsBetaAutomationStartTimeClockOffsetErrorU_ = CA2T(itsBetaProductAutomation->StartTimeModeInfoStatusString().c_str());
    itsBetaAutomationEndTimeClockOffsetErrorU_ = CA2T(itsBetaProductAutomation->EndTimeModeInfoStatusString().c_str());
    itsBetaAutomationDataTriggersStringU_ = CA2T(itsBetaProductAutomation->TriggerModeInfo().itsTriggerDataString.c_str());
    UpdateData(FALSE);

}

void CFmiBetaProductAutomationDialog::StoreControlValuesToDocument()
{
    UpdateData(TRUE);

    itsBetaProductionSystem->AutomationModeOn(fAutomationModeOn == TRUE);
    itsBetaProductionSystem->BetaProductPath(CFmiWin32Helpers::CT2std(itsBetaProductPathU_));
    itsBetaProductionSystem->TriggerModeIndex(itsTriggerModeIndex);
    itsBetaProductionSystem->FixedTimesString(CFmiWin32Helpers::CT2std(itsBetaAutomationFixedTimesStringU_));
    itsBetaProductionSystem->AutomationTimeStepInHoursString(CFmiWin32Helpers::CT2std(itsBetaAutomationTimeStepStringU_));
    itsBetaProductionSystem->FirstRunTimeOfDayString(CFmiWin32Helpers::CT2std(itsBetaAutomationFirstRunOfDayValueU_));
    itsBetaProductionSystem->StartTimeModeIndex(itsBetaAutomationStartTimeModeIndex);
    itsBetaProductionSystem->EndTimeModeIndex(itsBetaAutomationEndTimeModeIndex);
    itsBetaProductionSystem->StartTimeClockOffsetInHoursString(CFmiWin32Helpers::CT2std(itsBetaAutomationStartTimeClockOffsetValueU_));
    itsBetaProductionSystem->EndTimeClockOffsetInHoursString(CFmiWin32Helpers::CT2std(itsBetaAutomationEndTimeClockOffsetValueU_));
    itsBetaProductionSystem->AutomationPath(CFmiWin32Helpers::CT2std(itsAutomationNameU_));
    itsBetaProductionSystem->TriggerDataString(CFmiWin32Helpers::CT2std(itsBetaAutomationDataTriggersStringU_));
}

void CFmiBetaProductAutomationDialog::DoWhenClosing(void)
{
    StoreControlValuesToDocument();
}

void CFmiBetaProductAutomationDialog::InitHeaders(void)
{
    int basicColumnWidthUnit = 18;
    itsHeaders.clear();
    
    itsHeaders.push_back(BetaAutomationHeaderParInfo(::GetDictionaryString("Row"), BetaAutomationHeaderParInfo::kRowNumber, basicColumnWidthUnit * 2));
    itsHeaders.push_back(BetaAutomationHeaderParInfo(::GetDictionaryString("Name"), BetaAutomationHeaderParInfo::kAutomationName, basicColumnWidthUnit * 9));
    itsHeaders.push_back(BetaAutomationHeaderParInfo(::GetDictionaryString("Enable"), BetaAutomationHeaderParInfo::kEnable, basicColumnWidthUnit * 3));
    itsHeaders.push_back(BetaAutomationHeaderParInfo(::GetDictionaryString("Next"), BetaAutomationHeaderParInfo::kNextRuntime, boost::math::iround(basicColumnWidthUnit * 3.2)));
    itsHeaders.push_back(BetaAutomationHeaderParInfo(::GetDictionaryString("Last"), BetaAutomationHeaderParInfo::kLastRuntime, boost::math::iround(basicColumnWidthUnit * 3.2)));
    itsHeaders.push_back(BetaAutomationHeaderParInfo(::GetDictionaryString("Status"), BetaAutomationHeaderParInfo::kAutomationStatus, boost::math::iround(basicColumnWidthUnit * 8.)));
    itsHeaders.push_back(BetaAutomationHeaderParInfo(::GetDictionaryString("Path"), BetaAutomationHeaderParInfo::kAutomationPath, boost::math::iround(basicColumnWidthUnit * 30.)));
}

void CFmiBetaProductAutomationDialog::UpdateBetaProductPathInfo()
{
    UpdateData(TRUE);
    itsBetaProductAutomation->CheckBetaProductPath(CFmiWin32Helpers::CT2std(itsBetaProductPathU_));
    itsBetaProductPathErrorStringU_ = CA2T(itsBetaProductAutomation->BetaProductPathStatusString().c_str());
    CheckForSaveButtonEnablations();
    UpdateData(FALSE);
}

void CFmiBetaProductAutomationDialog::UpdateTriggerModeInfo()
{
    UpdateData(TRUE);
    itsBetaProductAutomation->CheckTriggerModeInfo(itsTriggerModeIndex, CFmiWin32Helpers::CT2std(itsBetaAutomationFixedTimesStringU_), CFmiWin32Helpers::CT2std(itsBetaAutomationTimeStepStringU_), CFmiWin32Helpers::CT2std(itsBetaAutomationFirstRunOfDayValueU_), CFmiWin32Helpers::CT2std(itsBetaAutomationDataTriggersStringU_));
    itsBetaAutomationTriggerModeInfoStringU_ = CA2T(itsBetaProductAutomation->TriggerModeInfoStatusString().c_str());
    CheckForSaveButtonEnablations();
    UpdateData(FALSE);
}

void CFmiBetaProductAutomationDialog::UpdateStartTimeModeInfo()
{
    UpdateData(TRUE);
    itsBetaProductAutomation->CheckStartTimeModeInfo(static_cast<NFmiBetaProductAutomation::TimeMode>(itsBetaAutomationStartTimeModeIndex), CFmiWin32Helpers::CT2std(itsBetaAutomationStartTimeClockOffsetValueU_));
    itsBetaAutomationStartTimeClockOffsetErrorU_ = CA2T(itsBetaProductAutomation->StartTimeModeInfoStatusString().c_str());
    CheckForSaveButtonEnablations();
    UpdateData(FALSE);
}

void CFmiBetaProductAutomationDialog::UpdateEndTimeModeInfo()
{
    UpdateData(TRUE);
    itsBetaProductAutomation->CheckEndTimeModeInfo(static_cast<NFmiBetaProductAutomation::TimeMode>(itsBetaAutomationEndTimeModeIndex), CFmiWin32Helpers::CT2std(itsBetaAutomationEndTimeClockOffsetValueU_));
    itsBetaAutomationEndTimeClockOffsetErrorU_ = CA2T(itsBetaProductAutomation->EndTimeModeInfoStatusString().c_str());
    CheckForSaveButtonEnablations();
    UpdateData(FALSE);
}

void CFmiBetaProductAutomationDialog::OnBnClickedButtonBetaProductPathBrowse()
{
    UpdateData(TRUE);

    std::string betaProductPath = itsBetaProductAutomation->BetaProductPath();
    std::string initialDirectory = PathUtils::getPathSectionFromTotalFilePath(betaProductPath);

    CFileDialog dlg(TRUE, NULL, CA2T(betaProductPath.c_str()), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CA2T(NFmiBetaProductionSystem::BetaProductFileFilter().c_str()));
    dlg.m_ofn.lpstrInitialDir = CA2T(initialDirectory.c_str());
    if(dlg.DoModal() == IDOK)
    {
        itsBetaProductPathU_ = dlg.GetPathName();
        std::string relativeFilePath = PathUtils::getRelativePathIfPossible(CFmiWin32Helpers::CT2std(itsBetaProductPathU_), itsBetaProductionSystem->GetBetaProductionBaseDirectory());
        itsBetaProductPathU_ = CA2T(relativeFilePath.c_str());
        UpdateData(FALSE);
        UpdateBetaProductPathInfo();
    }
}


void CFmiBetaProductAutomationDialog::OnEnChangeEditBetaProductPath()
{
    UpdateBetaProductPathInfo();
}


void CFmiBetaProductAutomationDialog::OnDestroy()
{
    CTabPageSSL::OnDestroy();

    StoreControlValuesToDocument();
}


HBRUSH CFmiBetaProductAutomationDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CTabPageSSL::OnCtlColor(pDC, pWnd, nCtlColor);

    if(pWnd->GetDlgCtrlID() == IDC_STATIC_BETA_PRODUCT_ERROR_TEXT)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProductAutomation->BetaProductPathStatus());
    else if(pWnd->GetDlgCtrlID() == IDC_STATIC_BETA_AUTOMATION_TRIGGER_MODE_INFO_STRING)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProductAutomation->TriggerModeInfoStatus());
    else if(pWnd->GetDlgCtrlID() == IDC_EDIT_BETA_AUTOMATION_START_TIME_WALL_CLOCK_OFFSET_VALUE)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProductAutomation->StartTimeModeInfoStatus());
    else if(pWnd->GetDlgCtrlID() == IDC_STATIC_BETA_AUTOMATION_START_TIME_OFFSET_ERROR_TEXT)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProductAutomation->StartTimeModeInfoStatus());
    else if(pWnd->GetDlgCtrlID() == IDC_STATIC_BETA_AUTOMATION_END_TIME_OFFSET_ERROR_TEXT)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProductAutomation->EndTimeModeInfoStatus());

    return hbr;
}

static void AbsolutePathOkActivation(CButton &theSaveButton, const std::string &thePath)
{
    if(NFmiFileSystem::IsAbsolutePath(thePath))
        theSaveButton.EnableWindow(TRUE); // Save -buttonia varten pitää olla käytössä absoluuttinen polku
    else
        theSaveButton.EnableWindow(FALSE); // jos ei ollut absoluuttista polkua, estetään napin käyttö
}

void CFmiBetaProductAutomationDialog::EnableButtonAddEditedAutomation()
{
    if(itsBetaProductAutomation->InputWasGood())
    { // Jos kaikki inputit olivat kunnossa
        if(!itsBetaProductionSystem->UsedAutomationList().HasAutomationAlready(itsSelectedAutomationFullFilePath))
            ::AbsolutePathOkActivation(itsAddEditedAutomationToListButton, itsSelectedAutomationFullFilePath);
        else
            itsAddEditedAutomationToListButton.EnableWindow(FALSE);
    }
}

void CFmiBetaProductAutomationDialog::EnableButtonRemoveAutomationFromList()
{
    // Onko grid-controllista valittu jokin automaatio
    if(itsGridCtrl.GetSelectedCellRange().IsValid())
        itsRemoveAutomationFromListButton.EnableWindow(TRUE);
    else
        itsRemoveAutomationFromListButton.EnableWindow(FALSE);
}

// Tarkistetaan kaikkien Save ja Save as -nappuloiden tila, onko sallittua painaa sitä vai ei.
void CFmiBetaProductAutomationDialog::CheckForSaveButtonEnablations()
{
    if(!itsSmartMetDocumentInterface->BetaProductGenerationRunning())
    {
        // Beta automaatioihin liittyvät napit
        if(itsBetaProductAutomation->InputWasGood())
        {
            // Jos kaikki inputit olivat kunnossa, sallitaan Save as -buttonin käyttö
            itsBetaAutomationSaveAsButton.EnableWindow(TRUE);
            if(HasAutomationChanged())
                ::AbsolutePathOkActivation(itsBetaAutomationSaveButton, itsSelectedAutomationFullFilePath);
            else
                itsBetaAutomationSaveButton.EnableWindow(FALSE);
        }
        else
        {
            // Jos inputissa vikaa, niin estetään Save -buttonien käyttö
            itsBetaAutomationSaveButton.EnableWindow(FALSE);
            itsBetaAutomationSaveAsButton.EnableWindow(FALSE);
        }

        // Beta automaatiolistoihin liittyvät napit
        const NFmiBetaProductAutomationList usedAutomationList = itsBetaProductionSystem->UsedAutomationList();
        if(usedAutomationList.IsOk())
        {
            // Jos kaikki inputit olivat kunnossa, sallitaan Save as -buttonin käyttö
            itsAutomationListSaveAsButton.EnableWindow(TRUE);
            ::AbsolutePathOkActivation(itsAutomationListSaveButton, itsBetaProductionSystem->UsedAutomationListPathString());
        }
        else
        {
            // Jos inputissa vikaa, niin estetään Save -buttonien käyttö
            itsAutomationListSaveButton.EnableWindow(FALSE);
            itsAutomationListSaveAsButton.EnableWindow(FALSE);
        }

        EnableButtonRemoveAutomationFromList();
        EnableButtonAddEditedAutomation();
    }
    else
    {
        // Jos kuvatuotanto käynnissä, niin estetään Save -buttonien käyttö
        itsBetaAutomationSaveButton.EnableWindow(FALSE);
        itsBetaAutomationSaveAsButton.EnableWindow(FALSE);
        itsAutomationListSaveButton.EnableWindow(FALSE);
        itsAutomationListSaveAsButton.EnableWindow(FALSE);
        itsAddEditedAutomationToListButton.EnableWindow(FALSE);
        itsRemoveAutomationFromListButton.EnableWindow(FALSE);
    }

    UpdateData(FALSE);
}

void CFmiBetaProductAutomationDialog::OnBnClickedRadioTriggerModeFixedTimes()
{
    UpdateTriggerModeInfo();
}


void CFmiBetaProductAutomationDialog::OnBnClickedRadioTriggerModeTimesStep()
{
    UpdateTriggerModeInfo();
}


void CFmiBetaProductAutomationDialog::OnBnClickedRadioTriggerModeDataEvent()
{
    UpdateTriggerModeInfo();
}


void CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationTimeStepValue()
{
    UpdateTriggerModeInfo();
}


void CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationFixedTimes()
{
    UpdateTriggerModeInfo();
}


void CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationFirstRunOfDayValue()
{
    UpdateTriggerModeInfo();
}

void CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationStartTimeFromProduct()
{
    UpdateStartTimeModeInfo();
}


void CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationStartTimeWallClockOffset()
{
    UpdateStartTimeModeInfo();
}


void CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationStartTimeFromModel()
{
    UpdateStartTimeModeInfo();
}


void CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationEndTimeFromProduct()
{
    UpdateEndTimeModeInfo();
}


void CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationEndTimeWallClockOffset()
{
    UpdateEndTimeModeInfo();
}


void CFmiBetaProductAutomationDialog::OnBnClickedRadioBetaAutomationEndTimeFromModel()
{
    UpdateEndTimeModeInfo();
}

// Talletus suoraan takaisin listalta valittun tai tiedostosta ladattun automaation osoittamaan tiedostoon
void CFmiBetaProductAutomationDialog::OnBnClickedButtonBetaAutomationSave()
{
    StoreControlValuesToDocument(); // Ennen tallennusta talletetaan varmuuden vuoksi säädöt myös dokumenttiin

    BetaProduct::SaveObjectToKnownFileInJsonFormat(*itsBetaProductAutomation, itsSelectedAutomationFullFilePath, "Beta-automation", false);
    MakeAutomationComparisonObject();
    DoNextRuntimeUpdates();
}

void CFmiBetaProductAutomationDialog::DoNextRuntimeUpdates()
{
    // Päivitetään automaatiolistan beta-automaatiot, jos niihin olisi tullut muutoksia
    itsBetaProductionSystem->UsedAutomationList().RefreshAutomationList();
    itsBetaProductionSystem->UsedAutomationList().DoFullChecks(itsBetaProductionSystem->AutomationModeOn());
    UpdateAutomationList(); // Jos tehdyt muutokset vaikuttavat listassa oleviin automaatioihin, pitää listaa päivittää
}

// Talletetaan automaatio käyttäjän valitsemaan tiedostoon
void CFmiBetaProductAutomationDialog::OnBnClickedButtonBetaAutomationSaveAs()
{
    StoreControlValuesToDocument(); // Ennen tallennusta talletetaan varmuuden vuoksi säädöt myös dokumenttiin

    auto initialSavePath = itsBetaProductionSystem->BetaAutomationSaveInitialPath();
    if(BetaProduct::SaveObjectInJsonFormat(*itsBetaProductAutomation, initialSavePath, NFmiBetaProductionSystem::BetaAutomationFileFilter(), NFmiBetaProductionSystem::BetaAutomationFileExtension(), itsBetaProductionSystem->GetBetaProductionBaseDirectory(), "Beta-automation", "Betaautomation1", false, &itsSelectedAutomationFullFilePath, this))
    {
        itsBetaProductionSystem->BetaAutomationSaveInitialPath(initialSavePath);
    }
    itsAutomationNameU_ = CA2T(PathUtils::getRelativeStrippedFileName(itsSelectedAutomationFullFilePath, itsBetaProductionSystem->GetBetaProductionBaseDirectory(), NFmiBetaProductionSystem::BetaAutomationFileExtension()).c_str());
    MakeAutomationComparisonObject();
    UpdateData(FALSE);
}

void CFmiBetaProductAutomationDialog::UpdateSelectedAutomationListName()
{
    itsAutomationListNameU_ = CA2T(PathUtils::getRelativeStrippedFileName(itsBetaProductionSystem->UsedAutomationListPathString(), itsBetaProductionSystem->GetBetaProductionBaseDirectory(), NFmiBetaProductionSystem::BetaAutomationListFileExtension()).c_str());
}

void CFmiBetaProductAutomationDialog::ApplySelectedBetaAutomation()
{
    itsAutomationNameU_ = CA2T(PathUtils::getRelativeStrippedFileName(itsSelectedAutomationFullFilePath, itsBetaProductionSystem->GetBetaProductionBaseDirectory(), NFmiBetaProductionSystem::BetaAutomationFileExtension()).c_str());
    InitControlsFromLoadedBetaAutomation();
    MakeAutomationComparisonObject();
    CheckForSaveButtonEnablations();
}

void CFmiBetaProductAutomationDialog::OnBnClickedButtonBetaAutomationLoad()
{
    auto initialSavePath = itsBetaProductionSystem->BetaAutomationSaveInitialPath();
    if(BetaProduct::LoadObjectInJsonFormat(*itsBetaProductAutomation, initialSavePath, NFmiBetaProductionSystem::BetaAutomationFileFilter(), NFmiBetaProductionSystem::BetaAutomationFileExtension(), itsBetaProductionSystem->GetBetaProductionBaseDirectory(), "Beta-automation", false, &itsSelectedAutomationFullFilePath, this))
    {
        itsBetaProductionSystem->BetaAutomationSaveInitialPath(initialSavePath);
        ApplySelectedBetaAutomation();
    }
}

void CFmiBetaProductAutomationDialog::SelectedGridCell(const CCellID &theSelectedCell)
{
    if(theSelectedCell.IsValid())
    {
        int zeroBasedRowIndex = theSelectedCell.row - itsGridCtrl.GetFixedRowCount();
        NFmiBetaProductAutomationListItem &selectedListItem = itsBetaProductionSystem->UsedAutomationList().Get(zeroBasedRowIndex);
        if(!selectedListItem.IsEmpty())
        {
            itsBetaProductAutomation = selectedListItem.itsBetaProductAutomation;
            itsSelectedAutomationFullFilePath = selectedListItem.itsBetaProductAutomationAbsolutePath;
            ApplySelectedBetaAutomation();
        }
    }
}

void CFmiBetaProductAutomationDialog::Update()
{
    UpdateAutomationList();
}

void CFmiBetaProductAutomationDialog::DeselectGridCell(const CCellID &theSelectedCell)
{
    // Grid-ctrl on deselectoinut itsensä, nyt on tarkoitus laittaa eri kontrollit oikeisiin tiloihin
    CheckForSaveButtonEnablations();
}

void CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationStartTimeWallClockOffsetValue()
{
    UpdateStartTimeModeInfo();
}


void CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationEndTimeWallClockOffsetValue()
{
    UpdateEndTimeModeInfo();
}

void CFmiBetaProductAutomationDialog::UpdateAutomationList()
{
    const NFmiBetaProductAutomationList::AutomationContainer &dataVector = itsBetaProductionSystem->UsedAutomationList().AutomationVector();
    itsGridCtrl.SetRowCount(static_cast<int>(dataVector.size() + itsGridCtrl.GetFixedRowCount()));
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    for(size_t i = 0; i < dataVector.size(); i++)
    {
        SetGridRow(currentRowCount++, *dataVector[i]);
    }
    itsGridCtrl.UpdateData(FALSE);
}

// Halutaan palauttaa HH:mm eli hours:minutes teksti annetulle ajalle.
// Jos aika oli puuttuvaa, palautetaan --:--.
// Viimeiselle Beta-automaation ajoaika halutaan kuitenkin merkitä S-kirjaimella (= started),
// jos tuotetta ei ole ohjelman ajon aikan tehty vielä.
static std::string GetTimeTextHHmm(const NFmiMetTime &theTime, bool fJustStarted = false)
{
    if(theTime == NFmiMetTime::gMissingTime)
        return "--:--";

    NFmiMetTime aTime(1);
    bool nextDay = aTime.GetDay() < theTime.GetDay();

    std::string str;
    if(fJustStarted)
        str = "S ";
    else if(nextDay)
        str = "N ";
    str += theTime.ToStr("HH:mm", kEnglish);
    return str;
}

static std::string GetColumnText(int theRow, int theColumn, const NFmiBetaProductAutomationListItem &theListItem)
{
    switch(theColumn)
    {
    case BetaAutomationHeaderParInfo::kRowNumber:
        return NFmiStringTools::Convert(theRow);
    case BetaAutomationHeaderParInfo::kAutomationName:
        return theListItem.AutomationName();
    case BetaAutomationHeaderParInfo::kNextRuntime:
        return ::GetTimeTextHHmm(theListItem.itsNextRunTime);
    case BetaAutomationHeaderParInfo::kLastRuntime:
        return ::GetTimeTextHHmm(theListItem.itsLastRunTime, !theListItem.fProductsHaveBeenGenerated);
    case BetaAutomationHeaderParInfo::kAutomationStatus:
        return theListItem.ShortStatusText();
    case BetaAutomationHeaderParInfo::kAutomationPath:
        return theListItem.FullAutomationPath();
    default:
        return "";
    }
}

void CFmiBetaProductAutomationDialog::SetGridRow(int row, const NFmiBetaProductAutomationListItem &theListItem)
{
    using namespace std::placeholders;

    static const COLORREF gEnabledBkColor = RGB(255, 255, 255);
    static const COLORREF gDisabledBkColor = RGB(222, 222, 222);
    static const COLORREF gErrorBkColor = RGB(239, 135, 122);

    NFmiBetaProductAutomationListItem::ErrorStatus listItemErrorStatus = theListItem.GetErrorStatus();
    bool itemEnabled = theListItem.fEnable;
    for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
    {
        itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theListItem).c_str()));
        if(column >= itsGridCtrl.GetFixedColumnCount())
        {
            if(column != BetaAutomationHeaderParInfo::kEnable) // kaikki muut ovat read-only paitsi enable -checkbox
                itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); // Laita read-only -bitti päälle

            if(itemEnabled)
                itsGridCtrl.SetItemBkColour(row, column, gEnabledBkColor);
            else
                itsGridCtrl.SetItemBkColour(row, column, gDisabledBkColor);

            if(listItemErrorStatus != NFmiBetaProductAutomationListItem::kFmiListItemOk)
                itsGridCtrl.SetItemBkColour(row, column, gErrorBkColor);

            if(column == BetaAutomationHeaderParInfo::kEnable)
            {
                itsGridCtrl.SetCellType(row, column, RUNTIME_CLASS(CGridCellCheck));
                CGridCellCheck *pTempCell = (CGridCellCheck*)itsGridCtrl.GetCell(row, column);
                pTempCell->SetCheck(itemEnabled);
                pTempCell->SetCheckBoxClickedCallback(std::bind(&CFmiBetaProductAutomationDialog::HandleEnableAutomationCheckBoxClick, this, _1, _2));
            }
        }
    }
}

void CFmiBetaProductAutomationDialog::HandleEnableAutomationCheckBoxClick(int col, int row)
{
    UpdateData(TRUE);
    CGridCellCheck *pCell = dynamic_cast<CGridCellCheck *>(itsGridCtrl.GetCell(row, col));
    if(pCell)
    {
        bool newState = pCell->GetCheck() == TRUE;
        int dataIndex = row - 1; // rivit alkavat 1:stä, mutta datat on vektorissa 0:sta alkaen
        NFmiBetaProductAutomationList::AutomationContainer &dataVector = itsBetaProductionSystem->UsedAutomationList().AutomationVector();
        if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
        {
            NFmiBetaProductAutomationListItem &listItem = *dataVector[dataIndex];
            listItem.fEnable = newState;
            SetGridRow(row, listItem);
            itsGridCtrl.UpdateData(FALSE);
        }
    }
}

void CFmiBetaProductAutomationDialog::AddAutomationToList(const std::string &theFullFilePath)
{
    if(itsBetaProductionSystem->UsedAutomationList().Add(theFullFilePath))
    {
        UpdateAutomationList();
        CheckForSaveButtonEnablations();
    }
}

void CFmiBetaProductAutomationDialog::OnBnClickedButtonAddBetaAutomationToList()
{
    auto initialSavePath = itsBetaProductionSystem->BetaAutomationListSaveInitialPath();
    std::string selectedAbsoluteFilePath;
    if(BetaProduct::GetFilePathFromUser(NFmiBetaProductionSystem::BetaAutomationFileFilter(), initialSavePath, selectedAbsoluteFilePath, true, "", this))
    {
        itsBetaProductionSystem->BetaAutomationListSaveInitialPath(initialSavePath);
        AddAutomationToList(selectedAbsoluteFilePath);
    }
}

void CFmiBetaProductAutomationDialog::OnBnClickedButtonRemoveBetaAutomationFromList()
{
    CCellRange selectedRange(itsGridCtrl.GetSelectedCellRange());
    for(int row = selectedRange.GetMinRow(); row <= selectedRange.GetMaxRow(); row++)
    {
        if(row >= itsGridCtrl.GetFixedRowCount() && row < itsGridCtrl.GetRowCount())
        {
            itsBetaProductionSystem->UsedAutomationList().Remove(row - itsGridCtrl.GetFixedRowCount());
            itsBetaProductionSystem->UsedAutomationList().DoFullChecks(itsBetaProductionSystem->AutomationModeOn());
            UpdateAutomationList();
            CheckForSaveButtonEnablations();
        }
    }
}

void CFmiBetaProductAutomationDialog::OnBnClickedButtonSaveAutomationList()
{
    StoreControlValuesToDocument(); // Ennen tallennusta talletetaan varmuuden vuoksi säädöt myös dokumenttiin

    BetaProduct::SaveObjectToKnownFileInJsonFormat(itsBetaProductionSystem->UsedAutomationList(), itsBetaProductionSystem->UsedAutomationListPathString(), "Beta-automation-list", false);
}


void CFmiBetaProductAutomationDialog::OnBnClickedButtonLoadAutomationList()
{
    auto initialSavePath = itsBetaProductionSystem->BetaAutomationListSaveInitialPath();
    std::string usedAbsoluteFilePath;
    if(BetaProduct::LoadObjectInJsonFormat(itsBetaProductionSystem->UsedAutomationList(), initialSavePath, NFmiBetaProductionSystem::BetaAutomationListFileFilter(), NFmiBetaProductionSystem::BetaAutomationListFileExtension(), itsBetaProductionSystem->GetBetaProductionBaseDirectory(), "Automation-list", false, &usedAbsoluteFilePath, this))
    {
        itsBetaProductionSystem->BetaAutomationListSaveInitialPath(initialSavePath);
        itsBetaProductionSystem->UsedAutomationList().DoFullChecks(itsBetaProductionSystem->AutomationModeOn()); // Tehdään täydet tarkastelut vielä kun tiedetään missä moodissa ollaan
        itsBetaProductionSystem->UsedAutomationListPathString(usedAbsoluteFilePath);
        UpdateSelectedAutomationListName();
        UpdateAutomationList();
        CheckForSaveButtonEnablations();
        UpdateData(FALSE);
    }
}


void CFmiBetaProductAutomationDialog::OnBnClickedButtonSaveAsAutomationList()
{
    StoreControlValuesToDocument(); // Ennen tallennusta talletetaan varmuuden vuoksi säädöt myös dokumenttiin

    auto initialSavePath = itsBetaProductionSystem->BetaAutomationListSaveInitialPath();
    std::string usedAbsoluteFilePath;
    if(BetaProduct::SaveObjectInJsonFormat(itsBetaProductionSystem->UsedAutomationList(), initialSavePath, NFmiBetaProductionSystem::BetaAutomationListFileFilter(), NFmiBetaProductionSystem::BetaAutomationListFileExtension(), itsBetaProductionSystem->GetBetaProductionBaseDirectory(), "Automation-list", "Automationlist1", false, &usedAbsoluteFilePath, this))
    {
        itsBetaProductionSystem->BetaAutomationListSaveInitialPath(initialSavePath);
        itsBetaProductionSystem->UsedAutomationListPathString(usedAbsoluteFilePath);
        UpdateSelectedAutomationListName();
        CheckForSaveButtonEnablations();
        UpdateData(FALSE);
    }
}


void CFmiBetaProductAutomationDialog::OnBnClickedButtonAddEditedBetaAutomationToList()
{
    AddAutomationToList(itsSelectedAutomationFullFilePath);
}

void CFmiBetaProductAutomationDialog::MakeAutomationComparisonObject()
{
    if(itsBetaProductAutomation)
        itsBetaProductAutomationComparisonObject = std::make_shared<NFmiBetaProductAutomation>(*itsBetaProductAutomation);
    else
        itsBetaProductAutomationComparisonObject.reset();
}

bool CFmiBetaProductAutomationDialog::HasAutomationChanged()
{
    if(itsBetaProductAutomationComparisonObject && itsBetaProductAutomation)
        return *itsBetaProductAutomationComparisonObject != *itsBetaProductAutomation;
    return false;
}


void CFmiBetaProductAutomationDialog::OnBnClickedCheckAutomatiionModeOn()
{
    UpdateData(TRUE);
    itsBetaProductionSystem->AutomationModeOn(fAutomationModeOn == TRUE);
    itsBetaProductionSystem->UsedAutomationList().DoFullChecks(itsBetaProductionSystem->AutomationModeOn());
    UpdateAutomationList();
    itsSmartMetDocumentInterface->SetAllViewIconsDynamically();
}


void CFmiBetaProductAutomationDialog::OnBnClickedButtonRunSelectedAutomation()
{
    UpdateData(TRUE);
    auto selectedCellRange = itsGridCtrl.GetSelectedCellRange();
    if(selectedCellRange.IsValid())
    {
        if(!itsBetaProductionSystem->DoOnDemandBetaAutomations(selectedCellRange.GetMinRow(), true))
        {
            CatLog::logMessage("No valid selection for on-demand Beta-automation run", CatLog::Severity::Warning, CatLog::Category::Operational, true);
        }
    }
    else
        CatLog::logMessage("Nothing was selected for on-demand Beta-automation run", CatLog::Severity::Warning, CatLog::Category::Operational, true);
    UpdateAutomationList();
}


void CFmiBetaProductAutomationDialog::OnBnClickedButtonRunAllAutomations()
{
    UpdateData(TRUE);
    if(!itsBetaProductionSystem->DoOnDemandBetaAutomations(-1, false))
    {
        CatLog::logMessage("Nothing was found for \"Run all\" on-demand Beta-automation run", CatLog::Severity::Warning, CatLog::Category::Operational, true);
    }
    UpdateAutomationList();
}


void CFmiBetaProductAutomationDialog::OnBnClickedButtonRunEnabledAutomations()
{
    UpdateData(TRUE);
    if(!itsBetaProductionSystem->DoOnDemandBetaAutomations(-1, true))
    {
        CatLog::logMessage("Nothing was found for \"Run enabled\" on-demand Beta-automation run", CatLog::Severity::Warning, CatLog::Category::Operational, true);
    }
    UpdateAutomationList();
}


void CFmiBetaProductAutomationDialog::OnEnChangeEditBetaAutomationDataTriggers()
{
    UpdateTriggerModeInfo();
}
