#include "stdafx.h"
#include "FmiSmartToolDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiSmartToolInfo.h"
#include "NFmiFileSystem.h"
#include "NFmiStringTools.h"
#include "NFmiDrawParam.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParamFolder.h"
#include "NFmiMacroParam.h"
#include "NFmiMacroParamfunctions.h"
#include "NFmiPathUtils.h"

#include <direct.h>
#include <cassert>
#include "FmiWin32Helpers.h"
#include "FmiModifyDrawParamDlg.h"
#include "FmiSmartToolLoadingDlg.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiInfoOrganizer.h"
#include "CloneBitmap.h"
#include "FmiMacroParamUpdateThread.h"
#include "FmiWin32TemplateHelpers.h"
#include "CtrlViewFunctions.h"
#include "SmartMetMfcUtils_resource.h"
#include "persist2.h"
#include "NFmiMacroParamDataCache.h"
#include "ApplicationInterface.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiFileString.h"
#include <boost/math/special_functions.hpp>
#include <fstream>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

#ifndef DISABLE_EXTREME_TOOLKITPRO
#include <SyntaxEdit\XTPSyntaxEditBufferManager.h>
#endif // DISABLE_EXTREME_TOOLKITPRO

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CFmiSmartToolDlg dialog

const NFmiViewPosRegistryInfo CFmiSmartToolDlg::s_ViewPosRegistryInfo(CRect(300, 200, 793, 739), "\\SmartToolDlg");

IMPLEMENT_DYNAMIC(CFmiSmartToolDlg, CDialog)

CFmiSmartToolDlg::CFmiSmartToolDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
:CDialog(CFmiSmartToolDlg::IDD, pParent)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsSmartToolInfo(smartMetDocumentInterface ? smartMetDocumentInterface->SmartToolInfo() : 0)
,itsSelectedMapViewDescTopIndex(0)
,itsMacroParamDataGridSizeX(0)
,itsMacroParamDataGridSizeY(0)
,fCrossSectionMode(FALSE)
,fQ3Macro(FALSE)
,fSearchOptionCaseSensitive(FALSE)
,fSearchOptionMatchAnywhere(TRUE)
,itsLoadedSmarttoolMacroPathU_()
,itsLoadedMacroParamPathTextU_(_T(""))
#ifndef DISABLE_EXTREME_TOOLKITPRO
,itsSyntaxEditControl()
,itsSyntaxEditControlAcceleratorTable(NULL)
,fShowTooltipsOnSmarttoolDialog(FALSE)
#endif // DISABLE_EXTREME_TOOLKITPRO
{
	assert(itsSmartToolInfo); // t‰ss‰ pit‰‰ olla jotain, koska myˆhemmin ei tarkistuksia!
	//{{AFX_DATA_INIT(CFmiSmartToolDlg)
    itsMacroErrorTextU_ = _T("");
	fModifyOnlySelectedLocations = FALSE;
	fMakeDBCheckAtSend = FALSE;
	//}}AFX_DATA_INIT
}

CFmiSmartToolDlg::~CFmiSmartToolDlg(void)
{
}

void CFmiSmartToolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFmiSmartToolDlg)
    DDX_Text(pDX, IDC_RICHEDIT_MACRO_ERROR_TEXT, itsMacroErrorTextU_);
    DDX_Check(pDX, IDC_CHECK_MODIFY_ONLY_SELECTED_LOCATIONS, fModifyOnlySelectedLocations);
    DDX_Check(pDX, IDC_CHECK_MAKE_DB_CHECK_AT_SEND, fMakeDBCheckAtSend);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_LIST_PARAM_MACROS, itsMacroParamList);
    DDX_Text(pDX, IDC_EDIT_MACRO_PARAM_DATA_GRID_SIZE_X, itsMacroParamDataGridSizeX);
    DDX_Text(pDX, IDC_EDIT_MACRO_PARAM_DATA_GRID_SIZE_Y, itsMacroParamDataGridSizeY);
    DDX_Check(pDX, IDC_CHECK_MACRO_PARAM_CROSSSECTION_MODE, fCrossSectionMode);
    DDX_Check(pDX, IDC_CHECK_Q3_MACRO, fQ3Macro);
    DDX_Radio(pDX, IDC_RADIO_VIEWMACRO_SELECTED_MAP1, itsSelectedMapViewDescTopIndex);
    DDX_Control(pDX, IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, itsSpeedSearchMacroControl);
    DDX_Control(pDX, IDC_EDIT_SPEED_SEARCH_MACRO_PARAM_CONTROL, itsSpeedSearchMacroParamControl);
    DDX_Text(pDX, IDC_STATIC_LOADED_SMARTTOOL_PATH_TEXT, itsLoadedSmarttoolMacroPathU_);
    DDX_Text(pDX, IDC_STATIC_LOADED_MACRO_PARAM_TEXT, itsLoadedMacroParamPathTextU_);
    DDX_Check(pDX, IDC_CHECK_SHOW_TOOLTIP_ON_SMARTTOOL_DIALOG, fShowTooltipsOnSmarttoolDialog);
}


BEGIN_MESSAGE_MAP(CFmiSmartToolDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiSmartToolDlg)
	ON_BN_CLICKED(IDC_BUTTON_ACTION, OnButtonAction)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_LOAD, OnButtonSmartToolLoad)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_LOAD_DB_CHECKER, OnButtonSmartToolLoadDbChecker)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_SAVE_AS, OnButtonSmartToolSaveAs)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_SAVE_DB_CHECKER, OnButtonSmartToolSaveDbChecker)
	ON_BN_CLICKED(IDC_CHECK_MAKE_DB_CHECK_AT_SEND, OnCheckMakeDbCheckAtSend)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_MODIFY_ONLY_SELECTED_LOCATIONS, OnBnClickedCheckModifyOnlySelectedLocations)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_SAVE_AS, OnBnClickedButtonMacroParamSaveAs)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_REMOVE, OnBnClickedButtonMacroParamRemove)
	ON_BN_CLICKED(IDC_BUTTON_ADD_TO_ROW_1, OnBnClickedButtonAddToRow1)
	ON_BN_CLICKED(IDC_BUTTON_ADD_TO_ROW_2, OnBnClickedButtonAddToRow2)
	ON_BN_CLICKED(IDC_BUTTON_ADD_TO_ROW_3, OnBnClickedButtonAddToRow3)
	ON_BN_CLICKED(IDC_BUTTON_ADD_TO_ROW_4, OnBnClickedButtonAddToRow4)
	ON_BN_CLICKED(IDC_BUTTON_ADD_TO_ROW_5, OnBnClickedButtonAddToRow5)
	ON_LBN_SELCHANGE(IDC_LIST_PARAM_MACROS, OnLbnSelchangeListParamMacros)
	ON_LBN_DBLCLK(IDC_LIST_PARAM_MACROS, OnLbnDblclkListParamMacros)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_LATEST_ERROR_TEXT, OnBnClickedButtonMacroParamLatestErrorText)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_REMOVE, OnBnClickedButtonSmartToolRemove)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_ALL_FROM_ROW1, OnBnClickedButtonRemoveAllFromRow1)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_ALL_FROM_ROW2, OnBnClickedButtonRemoveAllFromRow2)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_ALL_FROM_ROW3, OnBnClickedButtonRemoveAllFromRow3)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_ALL_FROM_ROW4, OnBnClickedButtonRemoveAllFromRow4)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_ALL_FROM_ROW5, OnBnClickedButtonRemoveAllFromRow5)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_REFRESH_LIST, OnBnClickedButtonMacroParamRefreshList)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_DATA_GRID_SIZE_USE, OnBnClickedButtonMacroParamDataGridSizeUse)
	ON_BN_CLICKED(IDC_CHECK_MACRO_PARAM_CROSSSECTION_MODE, OnBnClickedCheckMacroParamCrosssectionMode)
	ON_BN_CLICKED(IDC_CHECK_Q3_MACRO, &CFmiSmartToolDlg::OnBnClickedCheckQ3Macro)
	ON_BN_CLICKED(IDC_RADIO_VIEWMACRO_SELECTED_MAP1, &CFmiSmartToolDlg::OnBnClickedRadioViewmacroSelectedMap1)
	ON_BN_CLICKED(IDC_RADIO_VIEWMACRO_SELECTED_MAP2, &CFmiSmartToolDlg::OnBnClickedRadioViewmacroSelectedMap2)
	ON_BN_CLICKED(IDC_RADIO_VIEWMACRO_SELECTED_MAP3, &CFmiSmartToolDlg::OnBnClickedRadioViewmacroSelectedMap3)
    ON_WM_GETMINMAXINFO()
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_LEFT_ICON_CLICKED, &CFmiSmartToolDlg::OnGUSIconEditLeftIconClicked)
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_RIGHT_ICON_CLICKED, &CFmiSmartToolDlg::OnGUSIconEditRightIconClicked)
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_CLICK_SEARCH_LIST, &CFmiSmartToolDlg::OnGUSIconEditSearchListClicked)
    ON_COMMAND(ID_SEARCHOPTION_CASESENSITIVE, &CFmiSmartToolDlg::OnSearchOptionCaseSesensitive)
    ON_COMMAND(ID_SEARCHOPTION_MATCHANYWHERE, &CFmiSmartToolDlg::OnSearchOptionMatchAnywhere)
    ON_EN_CHANGE(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, &CFmiSmartToolDlg::OnEnChangeEditSpeedSearchMacro)
    ON_EN_CHANGE(IDC_EDIT_SPEED_SEARCH_MACRO_PARAM_CONTROL, &CFmiSmartToolDlg::OnEnChangeEditSpeedSearchMacroParam)
    ON_WM_SIZE()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_SAVE, &CFmiSmartToolDlg::OnBnClickedButtonMacroParamSave)
    ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_SAVE, &CFmiSmartToolDlg::OnBnClickedButtonSmartToolSave)
    ON_BN_CLICKED(IDC_CHECK_SHOW_TOOLTIP_ON_SMARTTOOL_DIALOG, &CFmiSmartToolDlg::OnBnClickedCheckShowTooltipOnSmarttoolDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiSmartToolDlg message handlers

BOOL CFmiSmartToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    DoResizerHooking(); // T‰t‰ pit‰‰ kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisterist‰

	std::string errorBaseStr("Error in CFmiSmartToolDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
    CFmiWin32Helpers::SetUsedWindowIconDynamically(this);

#ifndef DISABLE_EXTREME_TOOLKITPRO
    InitializeSyntaxEditControl();
#else
    WarnUserAboutNoEditingSmarttools();
#endif // DISABLE_EXTREME_TOOLKITPRO

	fMakeDBCheckAtSend = itsSmartToolInfo->MakeDBCheckAtSend();
	UpdateMacroParamDisplayList(false);

	InitDialogTexts();
    EnableColorCodedControls();
	NFmiPoint gridSize(itsSmartMetDocumentInterface->InfoOrganizer()->GetMacroParamDataGridSize());
	itsMacroParamDataGridSizeX = static_cast<int>(gridSize.X());
	itsMacroParamDataGridSizeY = static_cast<int>(gridSize.Y());
    UpdateLoadedSmarttoolMacroPathString();
    InitSpeedSearchControls();
    ResetSpeedSearchResources(true, false);
    DisableActionButtomIfInViewMode();
    EnableSaveButtons();
    InitTooltipControl();
    fShowTooltipsOnSmarttoolDialog = itsSmartMetDocumentInterface->ApplicationWinRegistry().ShowTooltipOnSmarttoolDialog();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiSmartToolDlg::SetDialogControlTooltip(int controlId, const std::string& tooltipRawText)
{
    CWnd* controlWnd = GetDlgItem(controlId);
    if(controlWnd)
    {
        auto finalText = ::GetDictionaryString(tooltipRawText.c_str());
        m_tooltip.AddTool(controlWnd, CA2T(finalText.c_str()));
    }
}

static std::string MakeAddMacroParamTooltipString(int rowIndex)
{
    std::string str = "Add selected macro-param on row ";
    str += std::to_string(rowIndex);
    str += " on selected view";
    return str;
}

static std::string MakeRemoveAllParametersFromRowString(int rowIndex)
{
    std::string str = "Remove <b>ALL</b> parameters from row ";
    str += std::to_string(rowIndex);
    str += " on selected view";
    return str;
}

static std::string MakeSelectedViewString(int viewIndex)
{
    std::string str = "Set ";
    if(viewIndex == 1)
        str += "main-map-view";
    else if(viewIndex == 4)
        str += "cross-section-view";
    else
    {
        str += "map-view ";
        str += std::to_string(viewIndex);
    }

    str += " as 'activated' view.";
    str += "\nAll add macro-param and clear view row actions are then done to this view.";
    return str;
}

void CFmiSmartToolDlg::InitTooltipControl()
{
    m_tooltip.Create(this);
    m_tooltip.SetDelayTime(PPTOOLTIP_TIME_AUTOPOP, 30000); // kuinka kauan tooltippi viipyy, jos kursoria ei liikuteta [ms]
    m_tooltip.SetDelayTime(PPTOOLTIP_TIME_INITIAL, 1200); // kuinka nopeasti tooltip ilmestyy n‰kyviin, jos kursoria ei liikuteta [ms]

    // T‰ss‰ erikseen jokainen kontrolli, jolle halutaan joku tooltip teksti
    SetDialogControlTooltip(IDC_BUTTON_SMART_TOOL_SAVE, "Save current Macro text to selected smarttool file\n(if any is selected)");
    SetDialogControlTooltip(IDC_BUTTON_SMART_TOOL_SAVE_AS, "Save current Macro text to wanted smarttool file\n(opens file browser)");
    SetDialogControlTooltip(IDC_BUTTON_SMART_TOOL_LOAD, "Opens dialog to select smarttool to be loaded to Macro text");
    SetDialogControlTooltip(IDC_BUTTON_SMART_TOOL_REMOVE, "Opens dialog to select smarttool to be removed");
    SetDialogControlTooltip(IDC_BUTTON_ACTION, "Will execute current Macro text to edited data");
    SetDialogControlTooltip(IDC_CHECK_MODIFY_ONLY_SELECTED_LOCATIONS, "Modify only selected locations setting.\nIf checked, and current Macro text is executed on edited data,\nwill do modifications only on selected grid points.\nOtherwise all edited data grid points are modified.");
    SetDialogControlTooltip(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, "Speed search smarttools");
    SetDialogControlTooltip(IDC_EDIT_SPEED_SEARCH_MACRO_PARAM_CONTROL, "Speed search macroParams");
    SetDialogControlTooltip(IDC_STATIC_LOADED_SMARTTOOL_PATH_TEXT, "Last loaded or saved smarttool's path.\nThis will be cleared if any macro-params are loaded or saved in between.\n(Clearing is meant to help users with this dual-purpose dialog with mixed smarttool and macro-param handling)");

    std::string dbCheckerString = "\nDBChecker means \"Send edited data to Database checker\"\nThis is smarttool script that can be set to be automatically do wanted checks before data is actually sent.";
    SetDialogControlTooltip(IDC_BUTTON_SMART_TOOL_SAVE_DB_CHECKER, std::string("Saves current Macro text as DBChecker.") + dbCheckerString);
    SetDialogControlTooltip(IDC_BUTTON_SMART_TOOL_LOAD_DB_CHECKER, std::string("Loads current DBChecker to Macro text.") + dbCheckerString);
    SetDialogControlTooltip(IDC_CHECK_MAKE_DB_CHECK_AT_SEND, std::string("Use DBChecker to edited data before it's sent.") + dbCheckerString);

    SetDialogControlTooltip(IDC_BUTTON_MACRO_PARAM_SAVE, "Save current Macro text to selected macro-param file\n(if any is selected)");
    SetDialogControlTooltip(IDC_BUTTON_MACRO_PARAM_SAVE_AS, "Save current Macro text to wanted macro-param file\n(opens file browser)");
    SetDialogControlTooltip(IDC_BUTTON_MACRO_PARAM_REMOVE, "Removes selected macro-param\n(if any is selected)");
    SetDialogControlTooltip(IDC_BUTTON_MACRO_PARAM_REFRESH_LIST, "Updates shown macro-param list control\n(All macro-params, the whole directory tree, are reloaded from files)");
    SetDialogControlTooltip(IDC_BUTTON_MACRO_PARAM_LATEST_ERROR_TEXT, "Show the latest known error caused by any smarttool or macro-param");
    SetDialogControlTooltip(IDC_STATIC_LOADED_MACRO_PARAM_TEXT, "Last loaded or saved macro-param's path.\nThis will be cleared if any smarttools are loaded or saved in between.\n(Clearing is meant to help users with this dual-purpose dialog with mixed smarttool and macro-param handling)");

    SetDialogControlTooltip(IDC_CHECK_Q3_MACRO, "If checked, show only q3 type macro-params on list control\notherwise show only 'normal' macro-params.\n(q3 is FMI only smartmet-server feature?)");
    
    SetDialogControlTooltip(IDC_EDIT_MACRO_PARAM_DATA_GRID_SIZE_X, "Used x-size of calculated macro-param grid");
    SetDialogControlTooltip(IDC_EDIT_MACRO_PARAM_DATA_GRID_SIZE_Y, "Used y-size of calculated macro-param grid");
    SetDialogControlTooltip(IDC_BUTTON_MACRO_PARAM_DATA_GRID_SIZE_USE, "Change the size of calculated macro-param data grid.\nMacro-params are calculated on pre-defined grids\nthat cover the used map-area on map-views.\n(You can by pass this size by using RESOLUTION=xxx in Macro text formula)");
 
    SetDialogControlTooltip(IDC_BUTTON_ADD_TO_ROW_1, ::MakeAddMacroParamTooltipString(1));
    SetDialogControlTooltip(IDC_BUTTON_ADD_TO_ROW_2, ::MakeAddMacroParamTooltipString(2));
    SetDialogControlTooltip(IDC_BUTTON_ADD_TO_ROW_3, ::MakeAddMacroParamTooltipString(3));
    SetDialogControlTooltip(IDC_BUTTON_ADD_TO_ROW_4, ::MakeAddMacroParamTooltipString(4));
    SetDialogControlTooltip(IDC_BUTTON_ADD_TO_ROW_5, ::MakeAddMacroParamTooltipString(5));
    SetDialogControlTooltip(IDC_BUTTON_REMOVE_ALL_FROM_ROW1, ::MakeRemoveAllParametersFromRowString(1));
    SetDialogControlTooltip(IDC_BUTTON_REMOVE_ALL_FROM_ROW2, ::MakeRemoveAllParametersFromRowString(2));
    SetDialogControlTooltip(IDC_BUTTON_REMOVE_ALL_FROM_ROW3, ::MakeRemoveAllParametersFromRowString(3));
    SetDialogControlTooltip(IDC_BUTTON_REMOVE_ALL_FROM_ROW4, ::MakeRemoveAllParametersFromRowString(4));
    SetDialogControlTooltip(IDC_BUTTON_REMOVE_ALL_FROM_ROW5, ::MakeRemoveAllParametersFromRowString(5));
    SetDialogControlTooltip(IDC_RADIO_VIEWMACRO_SELECTED_MAP1, ::MakeSelectedViewString(1));
    SetDialogControlTooltip(IDC_RADIO_VIEWMACRO_SELECTED_MAP2, ::MakeSelectedViewString(2));
    SetDialogControlTooltip(IDC_RADIO_VIEWMACRO_SELECTED_MAP3, ::MakeSelectedViewString(3));
    SetDialogControlTooltip(IDC_CHECK_MACRO_PARAM_CROSSSECTION_MODE, ::MakeSelectedViewString(4));

}

BOOL CFmiSmartToolDlg::PreTranslateMessage(MSG* pMsg)
{
    if(fShowTooltipsOnSmarttoolDialog)
        m_tooltip.RelayEvent(pMsg);

    return CDialog::PreTranslateMessage(pMsg);
}

void CFmiSmartToolDlg::EnableColorCodedControls()
{
    CFmiWin32Helpers::EnableColorCodedControl(this, IDC_CHECK_Q3_MACRO);
}

void CFmiSmartToolDlg::WarnUserAboutNoEditingSmarttools()
{
    CWnd *controlPlaceHolderWin = GetDlgItem(IDC_STATIC_SYNTAX_EDIT_CONTROL_PLACER);
    if(controlPlaceHolderWin)
    {
        // Let's put this place-holder control visible so that warning message is seen by user.
        controlPlaceHolderWin->ModifyStyle(0, WS_VISIBLE);
        controlPlaceHolderWin->SetWindowText(_TEXT("No smarttool formula editing without Extreme ToolkitPro support"));
    }
}

#ifndef DISABLE_EXTREME_TOOLKITPRO
// ========================================================================================================

static void MakeSmarttoolFileErrorLogging(std::string errorPart1, std::string errorPart2, const std::string &filePath, std::exception *possibleException)
{
    std::string errorMessage = errorPart1;
    errorMessage += filePath;
    errorMessage += errorPart2;
    if(possibleException)
        errorMessage += possibleException->what();
    CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Macro);
}


bool CFmiSmartToolDlg::LoadSmarttoolToSyntaxEditControl(const std::string &theFilePath)
{
    try
    {
        CXTPSyntaxEditBufferManager* pDataMan = itsSyntaxEditControl->GetEditBuffer();
        if(pDataMan)
        {
            pDataMan->SetFileExt(_TEXT(".st"));
            std::string fileContent;
            if(NFmiFileSystem::ReadFile2String(theFilePath, fileContent))
            {
                CString fileContentUnicode = CA2T(fileContent.c_str());
                itsSyntaxEditControl->SetText(fileContentUnicode);
                itsSyntaxEditControl->RefreshColors();
                itsSyntaxEditControl->SetTopRow(1);
                itsSyntaxEditControl->RecalcScrollBars();

                itsSyntaxEditControl->SendMessage(WM_NCPAINT);

                UpdateSyntaxEditControl();
                return true;
            }
        }
    }
    catch(std::exception &e)
    {
        ::MakeSmarttoolFileErrorLogging("Loading smarttool from file '", "' failed, with error: ", theFilePath, &e);
    }
    catch(...)
    {
        ::MakeSmarttoolFileErrorLogging("Loading smarttool from file '", "' failed, with unknown error", theFilePath, nullptr);
    }

    return false;
}

void CFmiSmartToolDlg::UpdateSyntaxEditControl()
{
    itsSyntaxEditControl->Invalidate(FALSE);
}

bool CFmiSmartToolDlg::StoreSmarttoolFromSyntaxEditControl(const std::string &theFilePath)
{
    try
    {
        CXTPSyntaxEditBufferManager* pDataMan = itsSyntaxEditControl->GetEditBuffer();
        if(pDataMan)
        {
            auto formulaContentUnicode = itsSyntaxEditControl->GetText();
            std::string formulaContent = CT2A(formulaContentUnicode);
            std::ofstream out(theFilePath, std::ios::binary | std::ios::out);
            if(out)
                out.write(formulaContent.c_str(), formulaContent.size());
            else
                ::MakeSmarttoolFileErrorLogging("Saving smarttool to file '", "' failed, with unknown error", theFilePath, nullptr);
            itsSyntaxEditControl->RefreshColors();
            UpdateSyntaxEditControl();
            return true;
        }
    }
    catch(std::exception &e)
    {
        ::MakeSmarttoolFileErrorLogging("Saving smarttool to file '", "' failed, with error: ", theFilePath, &e);
    }
    catch(...)
    {
        ::MakeSmarttoolFileErrorLogging("Saving smarttool to file '", "' failed, with unknown error", theFilePath, nullptr);
    }
    return false;
}

void CFmiSmartToolDlg::InitializeSyntaxEditControl()
{
    itsSyntaxEditControl = std::make_unique<CXTPSyntaxEditCtrl>();
    BOOL bCreate = itsSyntaxEditControl->Create(this, TRUE, TRUE);
    itsSyntaxEditControl->ModifyStyleEx(0, WS_EX_CLIENTEDGE);
    //itsSyntaxEditControl.ModifyStyle(0, WS_BORDER);	

    AdjustSyntaxEditControlWindows();

    itsSyntaxEditControl->SetFocus();

    std::string configFilePath = MakeSyntaxEditConfigFilePath();
    itsSyntaxEditControl->SetConfigFile(CA2T(configFilePath.c_str()));
    itsSyntaxEditControl->EnableEditAccelerators(TRUE);
    itsSyntaxEditControl->SetAutoIndent(TRUE);

    LoadSmarttoolToSyntaxEditControl(GetSmarttoolFilePath());

    itsSyntaxEditControlAcceleratorTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_SYNTAX_EDIT_CONTROL));

    auto autoCompleteWindow = itsSyntaxEditControl->GetAutoCompleteWnd();
    if(autoCompleteWindow)
    {
        CString maxAutoCompliteText = _T("occurrence_betweeneq(par, radius_km, time_offset1, time_offset2, limit1, limit2)");
        auto textSizeInPixels = autoCompleteWindow->GetDC()->GetTextExtent(maxAutoCompliteText);
        // Jostain syyst‰ laskettu maksimi tekstin pituu on aivan liian pitk‰, lyhennet‰‰n sit‰ hieman
        int usedWidthInPixels = boost::math::iround(textSizeInPixels.cx * 0.85);
        autoCompleteWindow->SetWndWidth(usedWidthInPixels);
    }
}

std::string CFmiSmartToolDlg::MakeSyntaxEditConfigFilePath()
{
    std::string configDirectoryPath = itsSmartMetDocumentInterface->ControlDirectory() + "\\edit_conf\\";
    configDirectoryPath = NFmiSettings::Optional<std::string>("SmartMet::SmarttoolDlg::SyntaxEditDirectoryPath", configDirectoryPath);
    configDirectoryPath = PathUtils::getAbsoluteFilePath(configDirectoryPath, itsSmartMetDocumentInterface->ControlDirectory());
    return configDirectoryPath + "\\SyntaxEdit.ini";
}

void CFmiSmartToolDlg::AdjustSyntaxEditControlWindows()
{
    if(itsSyntaxEditControl) // Kun tullaan 1. kerran OnSize -metodiin, ei t‰t‰ kontrollia ole viel‰ luotu
    {
        CWnd *controlPlaceHolderWin = GetDlgItem(IDC_STATIC_SYNTAX_EDIT_CONTROL_PLACER);
        if(controlPlaceHolderWin)
        {
            CRect rcEdit;
            controlPlaceHolderWin->GetWindowRect(&rcEdit);
            ScreenToClient(rcEdit);
            //rcEdit.DeflateRect(3, 3);
            itsSyntaxEditControl->MoveWindow(&rcEdit);
            UpdateSyntaxEditControl();
        }
    }
}

static int GetLastColumn(CXTPSyntaxEditCtrl &syntaxEditControl)
{
    int nLastRow = syntaxEditControl.GetRowCount();
    LPCTSTR szLineText = syntaxEditControl.GetLineText(nLastRow);
    return (int)_tcsclen(szLineText) + 1;
}

// Tekstin saaminen on hankalaa SyntaxEdit-kontrollista (siell‰ on taas CXTPSyntaxEditBufferManager).
// Kontrolli itse k‰sittelee teksti‰ Unicodena. Lis‰ksi teksti pit‰‰ hakea CMemeFile -otuksesta.
// En voi tehd‰ luokan normaalia Serialize -talletusta, koska silloin tekstit menisiv‰t Unicode tiedostoihin ja 
// niit‰ ei voisi luka vanhemmilla SmartMet versioilla (teksteihin tulisi bin‰‰ri mˆssˆ‰ sekaan).
std::string CFmiSmartToolDlg::GetMacroTextFromSyntaxEditor()
{
    CXTPSyntaxEditBufferManager* pDataMan = itsSyntaxEditControl->GetEditBuffer();
    if(pDataMan)
    {
        // 1. Luetaan koko kontrollin tekstisis‰ltˆ CMemFile olioon.
        CMemFile memFile;
        pDataMan->GetBuffer(1, 1, pDataMan->GetRowCount(), ::GetLastColumn(*itsSyntaxEditControl), memFile, FALSE, TRUE);
        memFile.SeekToBegin(); // Aseta file-olio alkuun!

                               // 2. Luetaan file olion sis‰lt‰ CString:iin
        CString macroTextU_;
        UINT nBytes = (UINT)memFile.GetLength();
        int nChars = nBytes / sizeof(TCHAR);
        UINT nBytesRead = memFile.Read(macroTextU_.GetBuffer(nChars + 1), nBytes);
        macroTextU_.ReleaseBuffer(nChars); // T‰m‰ lis‰‰ stringin loppuun \0 -merkin ja ilmeisesti vapauttaa jotain dynaamisesti varattua juttua.

                                           // 3. Konvertoidaan Unicode teksti ascii tekstiksi
        std::string macroText = CT2A(macroTextU_);

        return macroText;
    }

    return "";
}

// ========================================================================================================
#endif // DISABLE_EXTREME_TOOLKITPRO

std::string CFmiSmartToolDlg::GetSmarttoolFilePath()
{
    return itsSmartToolInfo->GetFullScriptFileName(itsSmartToolInfo->CurrentScriptName());
}

void CFmiSmartToolDlg::DoResizerHooking(void)
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_ACTION, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_STR, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_MODIFY_ONLY_SELECTED_LOCATIONS, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_SYNTAX_EDIT_CONTROL_PLACER, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_ERROR_STR, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RICHEDIT_MACRO_ERROR_TEXT, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_SMARTTOOL_HOLDER, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMART_TOOL_SAVE, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMART_TOOL_SAVE_AS, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMART_TOOL_LOAD, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMART_TOOL_REMOVE, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMART_TOOL_SAVE_DB_CHECKER, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMART_TOOL_LOAD_DB_CHECKER, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_MAKE_DB_CHECK_AT_SEND, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_PARAM_HOLDER, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_SAVE, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_SAVE_AS, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_REMOVE, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_REFRESH_LIST, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_LATEST_ERROR_TEXT, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_Q3_MACRO, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_PARAM_DATA_GRID_SIZE_STR, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_X_STR, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_Y_STR, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_MACRO_PARAM_DATA_GRID_SIZE_X, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_MACRO_PARAM_DATA_GRID_SIZE_Y, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_DATA_GRID_SIZE_USE, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_LIST_PARAM_MACROS, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD_TO_ROW_1, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_REMOVE_ALL_FROM_ROW1, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD_TO_ROW_2, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_REMOVE_ALL_FROM_ROW2, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD_TO_ROW_3, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_REMOVE_ALL_FROM_ROW3, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD_TO_ROW_4, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_REMOVE_ALL_FROM_ROW4, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD_TO_ROW_5, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_REMOVE_ALL_FROM_ROW5, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RADIO_VIEWMACRO_SELECTED_MAP1, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RADIO_VIEWMACRO_SELECTED_MAP2, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RADIO_VIEWMACRO_SELECTED_MAP3, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_MACRO_PARAM_CROSSSECTION_MODE, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_SPEED_SEARCH_MACRO_PARAM_CONTROL, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_LOADED_SMARTTOOL_PATH_TEXT, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_LOADED_MACRO_PARAM_TEXT, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
}

void CFmiSmartToolDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiSmartToolDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiSmartToolDlg::DisableActionButtomIfInViewMode(void)
{
    CWnd *actionButtom = GetDlgItem(IDC_BUTTON_ACTION);
    if(actionButtom)
    {
        if(itsSmartMetDocumentInterface->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan Muokkaa-nappula
            actionButtom->EnableWindow(FALSE);
        else
            actionButtom->EnableWindow(TRUE);
    }
}

void CFmiSmartToolDlg::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa
    {
        auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
        if(macroParamSystemPtr->UpdateMacroParamListView())
        {
            macroParamSystemPtr->UpdateMacroParamListView(false);
            UpdateMacroParamDisplayList(false);
        }
        DisableActionButtomIfInViewMode();
        EnableSaveButtons();
    }
}

std::string CFmiSmartToolDlg::GetSmarttoolFormulaText()
{
#ifndef DISABLE_EXTREME_TOOLKITPRO
    return GetMacroTextFromSyntaxEditor();
#else
    return "";
#endif // DISABLE_EXTREME_TOOLKITPRO
}

bool CFmiSmartToolDlg::LoadSmarttoolFormula(const std::string &theFilePath, bool smarttoolCase)
{
    if(smarttoolCase)
    {
        UpdateLoadedSmarttoolMacroPathString();
        itsLoadedMacroParamPathTextU_.Empty();
    }
    else
    {
        UpdateLoadedMacroParamPathString();
        itsLoadedSmarttoolMacroPathU_.Empty();
    }
    EnableSaveButtons();
#ifndef DISABLE_EXTREME_TOOLKITPRO
    return LoadSmarttoolToSyntaxEditControl(theFilePath);
#else
    return false;
#endif // DISABLE_EXTREME_TOOLKITPRO
}

void CFmiSmartToolDlg::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool editedDataModified, const std::string &possibleEditedDrawParamPath, bool makeMapDirtyActions)
{
    if(!possibleEditedDrawParamPath.empty())
    {
        std::vector<std::string> modifiedMacroParamPathList{ possibleEditedDrawParamPath };
        itsSmartMetDocumentInterface->MacroParamDataCache().clearMacroParamCache(modifiedMacroParamPathList);
    }

    if(makeMapDirtyActions)
        itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, editedDataModified, false);

    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(reasonForUpdate, SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView | SmartMetViewId::TimeSerialView);
}

bool CFmiSmartToolDlg::EnableDlgItem(int theDlgId, bool fEnable)
{
	CWnd *window = GetDlgItem(theDlgId);
	if(window)
	{
		if(fEnable)
			window->EnableWindow(TRUE);
		else
			window->EnableWindow(FALSE);
		return true;
	}
	return false;
}

static std::string GetLastLoadedSmartToolRelativePathName(SmartMetDocumentInterface *smartMetDocumentInterface)
{
    auto smartToolInfo = smartMetDocumentInterface->SmartToolInfo();
    return PathUtils::getRelativeStrippedFileName(smartToolInfo->GetFullScriptFileName(smartToolInfo->CurrentScriptName()), smartToolInfo->RootLoadDirectory(), "st");
}

void CFmiSmartToolDlg::OnButtonAction()
{
	if(itsSmartMetDocumentInterface)
	{
		static int counter = 1;
		CWaitCursor cursor; // laittaa tiimalasin kursoriksi
		string counterStr = NFmiStringTools::Convert<int>(counter);

		UpdateData(TRUE);
		std::string macroText = GetSmarttoolFormulaText();
        itsMacroErrorTextU_ = CA2T(::GetDictionaryString("SmartToolDlgExecutionStarts").c_str());
		UpdateData(FALSE);

		// Invalidate window so entire client area
		// is redrawn when UpdateWindow is called.
		Invalidate();

		// Update Window to cause View to redraw.
		UpdateWindow();
        // koska smarttool muokkaus menee nyky‰‰n omaan threadiin ja p‰‰karttan‰yttˆ lukitaan Proegress-cancelointi -dialogilla, 
        // pit‰‰ t‰ss‰ est‰‰ ett‰ ei voi painaa uudestaan Muokkaa-nappulaa. Napin normaali tila palautetaan kun l‰hetet‰‰n viesti ID_MESSAGE_SMARTTOOL_MODIFICATION_ENDED
		EnableDlgItem(IDC_BUTTON_ACTION, false); 

        auto relativePathMacroName = ::GetLastLoadedSmartToolRelativePathName(itsSmartMetDocumentInterface);
		if(itsSmartMetDocumentInterface->DoSmartToolEditing(macroText, relativePathMacroName, fModifyOnlySelectedLocations == TRUE))
            itsMacroErrorTextU_ = CString(CA2T(string(counterStr + ". " + ::GetDictionaryString("SmartToolDlgExecutionOk")).c_str()));
		else
            itsMacroErrorTextU_ = CString(CA2T(counterStr.c_str())) + CString(" ") + CA2T(itsSmartMetDocumentInterface->SmartToolEditingErrorText().c_str());

		counter++;
		UpdateData(FALSE);

		RefreshApplicationViewsAndDialogs("SmartToolDlg: smarttool data modification", true, "");
	}
}

void CFmiSmartToolDlg::SetMacroErrorText(const std::string &theErrorStr)
{
    itsMacroErrorTextU_ = CA2T(theErrorStr.c_str());
	UpdateData(FALSE);
}

void CFmiSmartToolDlg::OnButtonSmartToolLoad()
{
	CFmiSmartToolLoadingDlg dlg(itsSmartToolInfo, this);

	if(dlg.DoModal() == IDOK)
	{
        DoSmartToolLoad(dlg.SelectedScriptName(), false);
	}
}

void CFmiSmartToolDlg::DoSmartToolLoad(const std::string &theSmartToolName, bool fDoSpeedLoad)
{
    bool status = false;
    if(fDoSpeedLoad)
        status = itsSmartToolInfo->SpeedLoadScript(theSmartToolName);
    else
        status = itsSmartToolInfo->LoadScript(theSmartToolName);

    DoTimedResetSearchResource();
    if(status)
    {
        CatLog::logMessage(string("Loaded smartTool: ") + theSmartToolName, CatLog::Severity::Info, CatLog::Category::Macro);
        LoadFormulaFromSmarttool();
        itsMacroParamList.SetCurSel(LB_ERR); // laitetaan macroParamlista osoittamaan 'ei mit‰‰n'
        UpdateData(FALSE);
    }
}

void CFmiSmartToolDlg::LoadFormulaFromSmarttool()
{
    LoadSmarttoolFormula(GetSmarttoolFilePath(), true);
}

void CFmiSmartToolDlg::LoadFormulaFromMacroParam()
{
    LoadSmarttoolFormula(GetMacroParamFilePath(*(itsSmartMetDocumentInterface->MacroParamSystem())), false);
}

void CFmiSmartToolDlg::OnButtonSmartToolLoadDbChecker()
{ // ladataan virallinen DBCheck-macro ruudulle esim. testi k‰yttˆˆn
    LoadSmarttoolFormula(itsSmartToolInfo->DBCheckerFileName(), true);
	itsSmartToolInfo->CurrentScript(itsSmartToolInfo->DBCheckerText()); // pit‰‰ p‰ivitt‰‰ myˆs currenttia skripti‰

	itsMacroParamList.SetCurSel(LB_ERR); // laitetaan macroParamlista osoittamaan 'ei mit‰‰n'
	UpdateData(FALSE);
}

static TCHAR* oldStr = _TEXT("\r\r\n");
static TCHAR* newStr = _TEXT("\r\n");
static int StripExtraCarrigeReturns(CString &theString)
{
	return theString.Replace(oldStr, newStr);
}

// rich_edit ja edit kontrollien kanssa on ihme ongelmia
// rivinvaihtojen kanssa. T‰ss‰ poistetaan yksi ylim‰‰r‰inen
// cr (carriage return) joka n‰ytt‰‰ tulevan aina teksteihin
// joka rivin vaihdon yhteyteen. t‰m‰ tekee muutoksen
// <cr><cr><nl> -> <cr><nl>
//std::string CFmiSmartToolDlg::GetWantedMacroString(void)
//{
//	int count = StripExtraCarrigeReturns(itsMacroTextU_);
//	string str = CT2A(itsMacroTextU_);
//	return str;
//}

const std::string g_SmarttoolFileExtension = "st";
const std::string g_SmarttoolFileTotalExtension = "." + g_SmarttoolFileExtension;
const std::string g_SmarttoolFileFilter = "Smarttool Files (*." + g_SmarttoolFileExtension + ")|*." + g_SmarttoolFileExtension + "|All Files (*.*)|*.*||";
const std::string g_SmarttoolDefaultFileName = "smarttool1";

void CFmiSmartToolDlg::OnButtonSmartToolSaveAs()
{
	UpdateData(TRUE);

    std::string initialPath = itsSmartToolInfo->RootLoadDirectory() + itsSmartToolInfo->GetRelativeLoadPath();
    std::string initialFilename = itsSmartToolInfo->CurrentScriptName();
    if(initialFilename.empty())
    {
        initialFilename = g_SmarttoolDefaultFileName;
    }
    initialFilename += g_SmarttoolFileTotalExtension;
    std::string filePath;
    if(BetaProduct::GetFilePathFromUserTotal(g_SmarttoolFileFilter, initialPath, filePath, false, initialFilename, g_SmarttoolFileExtension, itsSmartToolInfo->RootLoadDirectory(), this))
    {

        // pit‰‰ ensin tallettaa currenttiksi skriptiksi ja sitten tallettaa tiedostoon
        itsSmartToolInfo->CurrentScript(GetSmarttoolFormulaText());
        if(itsSmartToolInfo->SaveScript(filePath))
        {
            CatLog::logMessage(string("Saved smartTool: ") + string(filePath), CatLog::Severity::Info, CatLog::Category::Macro);
            ResetSpeedSearchResources(true, false);
            UpdateLoadedSmarttoolMacroPathString();
        }
        else
        {
            MakeSmarttoolSaveError(filePath);
        }
    }
}

void CFmiSmartToolDlg::MakeSmarttoolSaveError(const std::string & fullFilePath)
{
    std::string errMsgTitle("Cannot save smarttool skript");
    std::string errMsg("Cannot save smarttool skript:\n");
    errMsg += fullFilePath;
    errMsg += "\nCheck that the folder exists and you have write permissions there.";

    ::MessageBox(this->GetSafeHwnd(), CA2T(errMsg.c_str()), CA2T(errMsgTitle.c_str()), MB_ICONERROR | MB_OK);
}


void CFmiSmartToolDlg::OnBnClickedButtonSmartToolSave()
{
    UpdateData(TRUE);

    if(SmarttoolCanBeSaved())
    {
        // pit‰‰ ensin tallettaa currenttiksi skriptiksi ja sitten tallettaa tiedostoon
        itsSmartToolInfo->CurrentScript(GetSmarttoolFormulaText());
        auto totalSmarttoolFilePath = itsSmartToolInfo->GetFullScriptFileName(itsSmartToolInfo->CurrentScriptName());
        if(itsSmartToolInfo->SaveScript(totalSmarttoolFilePath))
        {
            CatLog::logMessage(string("Saved smartTool: ") + string(totalSmarttoolFilePath), CatLog::Severity::Info, CatLog::Category::Macro);
        }
        else
        {
            MakeSmarttoolSaveError(totalSmarttoolFilePath);
        }
    }
}

void CFmiSmartToolDlg::OnButtonSmartToolSaveDbChecker()
{ // talletetaan virallinen DBCheck-macro talteen omaan tiedostoon
	string str(::GetDictionaryString("SmartToolDlgSaveDBChecker1"));
	str += "\n";
	str += ::GetDictionaryString("SmartToolDlgSaveDBChecker2");
	str += "\n";
	str += ::GetDictionaryString("SmartToolDlgSaveDBChecker3");
    int action = ::MessageBox(this->GetSafeHwnd(), CA2T(str.c_str()), CA2T(::GetDictionaryString("NormalWordCapitalWarning").c_str()), MB_OKCANCEL);
	if(action == IDOK)
	{
		UpdateData(TRUE);
		itsSmartToolInfo->DBCheckerText(GetSmarttoolFormulaText());
		itsSmartToolInfo->SaveDBChecker();
	}
}

void CFmiSmartToolDlg::OnCheckMakeDbCheckAtSend()
{
	UpdateData(TRUE);
	itsSmartToolInfo->MakeDBCheckAtSend(fMakeDBCheckAtSend == TRUE);
}

void CFmiSmartToolDlg::OnBnClickedCheckModifyOnlySelectedLocations()
{
	UpdateData(TRUE);
}

static boost::shared_ptr<NFmiDrawParam> CreateDefaultDrawParamPointer()
{
    return boost::shared_ptr<NFmiDrawParam>(new NFmiDrawParam());
}

boost::shared_ptr<NFmiMacroParam> CreateMacroParamPointer(const std::string &macroParamName, const std::string &macroParamFormula, NFmiInfoData::Type macroType)
{
    boost::shared_ptr<NFmiMacroParam> macroParamPointer(new NFmiMacroParam);
    macroParamPointer->Name(macroParamName);
    macroParamPointer->MacroText(macroParamFormula);
    macroParamPointer->DrawParam(::CreateDefaultDrawParamPointer());
    macroParamPointer->DrawParam()->ParameterAbbreviation(macroParamPointer->Name());
    macroParamPointer->DrawParam()->DataType(macroType);
    return macroParamPointer;
}

void CFmiSmartToolDlg::DoFinalMacroParamWrite(NFmiMacroParamSystem& macroParamSystem, boost::shared_ptr<NFmiMacroParam> &macroParamPointer)
{
    try
    {
        boost::shared_ptr<NFmiMacroParamFolder> currentFolder = macroParamSystem.GetCurrentFolder();
        if(currentFolder)
        {
            currentFolder->Save(macroParamPointer);
            CatLog::logMessage(string("Saved macroParam: ") + macroParamPointer->Name(), CatLog::Severity::Info, CatLog::Category::Macro);
        }
    }
    catch(std::exception &e)
    {
        CatLog::logMessage(e.what(), CatLog::Severity::Error, CatLog::Category::Macro);

        std::string errMsgTitle("Cannot save macroParam");
        std::string errMsg(e.what());
        errMsg += "\nCheck that the folder exists and you have write permissions there.";

        ::MessageBox(this->GetSafeHwnd(), CA2T(errMsg.c_str()), CA2T(errMsgTitle.c_str()), MB_ICONINFORMATION | MB_OK);
    }
}

static bool IsInCurrentMacroParamDirectory(NFmiMacroParamSystem& mpSystem, const std::string filePath)
{
    std::experimental::filesystem::path currentMacroParamDirectory = mpSystem.CurrentPath();
    currentMacroParamDirectory.make_preferred();
    auto currentDirectoryString = currentMacroParamDirectory.string();
    // Otetaan mahdollinen loppu kenoviiva pois polusta
    if(currentDirectoryString.back() == '\\')
        currentMacroParamDirectory = currentMacroParamDirectory.parent_path();
    std::experimental::filesystem::path filePathDirectory = filePath;
    filePathDirectory.remove_filename();
    return currentMacroParamDirectory == filePathDirectory;
}


const std::string g_MacroParamFileExtension = "st";
const std::string g_MacroParamFileTotalExtension = "." + g_MacroParamFileExtension;
const std::string g_MacroParamFileFilter = "MacroParam Files (*." + g_MacroParamFileExtension + ")|*." + g_MacroParamFileExtension + "|All Files (*.*)|*.*||";
const std::string g_MacroParamDefaultFileName = "macroparam1";

static std::string GetRealMacroParamDrawParamFileName(const std::string& macroParamFormulaFileName)
{
    std::experimental::filesystem::path drawParamFilePath = macroParamFormulaFileName;
    drawParamFilePath.replace_extension("dpa");
    return drawParamFilePath.string();
}

bool CFmiSmartToolDlg::SmarttoolCanBeSaved() const
{
    if(itsLoadedSmarttoolMacroPathU_.IsEmpty())
        return false;
    else if(!itsSmartToolInfo || itsSmartToolInfo->CurrentScript().empty())
        return false;
    else
        return true;
}

void CFmiSmartToolDlg::EnableSaveButtons()
{
    // macroParam tapaus ensin
    auto currentMacroParam = itsSmartMetDocumentInterface->MacroParamSystem()->GetCurrentMacroParam();
    bool enableSaveMacroParamButton = true;
    if(itsLoadedMacroParamPathTextU_.IsEmpty())
        enableSaveMacroParamButton = false;
    else if(!currentMacroParam || currentMacroParam->IsMacroParamDirectory())
        enableSaveMacroParamButton = false;
    EnableDlgItem(IDC_BUTTON_MACRO_PARAM_SAVE, enableSaveMacroParamButton);

    // smarttool tapaus sitten
    bool enableSaveSmarttoolButton = SmarttoolCanBeSaved();
    EnableDlgItem(IDC_BUTTON_SMART_TOOL_SAVE, enableSaveSmarttoolButton);
}

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamSave()
{
    UpdateData(TRUE);

    try
    {
        if(!itsLoadedMacroParamPathTextU_.IsEmpty())
        {
            auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
            auto currentMacroParam = macroParamSystemPtr->GetCurrentMacroParam();
            if(currentMacroParam && currentMacroParam->IsMacroParamDirectory() == false)
            {
                std::string absoluteMacroParamDirectory = macroParamSystemPtr->RootPath() + macroParamSystemPtr->RelativePath();
                PathUtils::addDirectorySeparatorAtEnd(absoluteMacroParamDirectory);

                std::string finalMacroParamPath = absoluteMacroParamDirectory + currentMacroParam->Name() + g_MacroParamFileTotalExtension;
                std::string messageBoxText = ::GetDictionaryString("Do you want to overwrite macroParam file:\n");
                messageBoxText += finalMacroParamPath;
                std::string messageBoxTitle = ::GetDictionaryString("File overwrite");
                if(::MessageBox(this->GetSafeHwnd(), CA2T(messageBoxText.c_str()), CA2T(messageBoxTitle.c_str()), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
                    return;
                currentMacroParam->MacroText(GetSmarttoolFormulaText());
                if(currentMacroParam->Store(absoluteMacroParamDirectory, currentMacroParam->Name()))
                {
                    auto realMacroParamDrawParamFileName = ::GetRealMacroParamDrawParamFileName(finalMacroParamPath);
                    std::vector<std::string> modifiedMacroParamPaths{ realMacroParamDrawParamFileName };
                    itsSmartMetDocumentInterface->MacroParamDataCache().clearMacroParamCache(modifiedMacroParamPaths);
                    RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param save", false, finalMacroParamPath); // p‰ivitet‰‰n varmuuden vuoksi ruutuja, jos karttan‰ytˆll‰ olleen macroparametrin macroa on muutettu
                    return;
                }
                else
                    itsSmartMetDocumentInterface->LogAndWarnUser("Failed to save macroParam, unknown reason", "MacroParam save failed", CatLog::Severity::Error, CatLog::Category::Macro, false, false, true);
            }
            else
                itsSmartMetDocumentInterface->LogAndWarnUser("Failed to save macroParam, no macroParam was selected?", "MacroParam save failed", CatLog::Severity::Error, CatLog::Category::Macro, false, false, true);
        }
        else
            itsSmartMetDocumentInterface->LogAndWarnUser("Failed to save macroParam, smarttool was loaded last?", "MacroParam save failed", CatLog::Severity::Error, CatLog::Category::Macro, false, false, true);
    }
    catch(std::exception & e)
    {
        std::string messageBoxText = ::GetDictionaryString("Unable to store macroParam file:\n");
        messageBoxText += e.what();
        std::string messageBoxTitle = ::GetDictionaryString("MacroParam save failed");
        itsSmartMetDocumentInterface->LogAndWarnUser(messageBoxText, messageBoxTitle, CatLog::Severity::Error, CatLog::Category::Macro, false, false, true);
    }
}

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamSaveAs()
{
    UpdateData(TRUE);

    auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
    std::string initialPath = macroParamSystemPtr->RootPath() + macroParamSystemPtr->RelativePath();
    PathUtils::addDirectorySeparatorAtEnd(initialPath);
    std::string initialFilename;
    if(macroParamSystemPtr->GetCurrentMacroParam())
    {
        initialFilename = macroParamSystemPtr->GetCurrentMacroParam()->Name();
    }
    if(initialFilename.empty())
    {
        initialFilename += g_MacroParamDefaultFileName;
    }
    initialFilename += g_MacroParamFileTotalExtension;

    std::string filePath;
    if(BetaProduct::GetFilePathFromUserTotal(g_MacroParamFileFilter, initialPath, filePath, false, initialFilename, g_MacroParamFileExtension, macroParamSystemPtr->RootPath(), this))
    {
        NFmiFileString fileString(filePath);

        boost::shared_ptr<NFmiMacroParam> macroParamPointer = ::CreateMacroParamPointer(std::string(fileString.Header()), GetSmarttoolFormulaText(), GetUsedMacroParamType());

        bool updateViews = false; // kun talletetaan vanhan p‰‰lle, pit‰‰ p‰ivitt‰‰ ruutuja, koska macro-teksti on saattanut muuttua
        if(macroParamSystemPtr->SetCurrentToWantedMacroPath(filePath))
        {
            boost::shared_ptr<NFmiMacroParam> currentMacroParam = macroParamSystemPtr->GetCurrentMacroParam(); // Otetaan talteen erilliseen muuttujaan, koska ilmeisesti CurrentMacroParam -pointteri voi 'korruptoitua', en tied‰ miksi, mutta SmartMet voi kaatua kun ollaan "pit‰‰ ottaa talteen vanhat piirto-ominaisuudet" -rivill‰ jompaan kumpaan DrawParam() -kutsuun ja oletan ett‰ sen t‰ytyy olla CurrentMacroParam() -kohdasta.
            if(currentMacroParam)
            {
                macroParamPointer->DrawParam(currentMacroParam->DrawParam()); // pit‰‰ ottaa talteen vanhat piirto-ominaisuudet!
                updateViews = true; // nyt tiedet‰‰n, ett‰ pit‰‰ p‰ivitt‰‰ n‰yttˆj‰
            }
        }
        else if(!::IsInCurrentMacroParamDirectory(*macroParamSystemPtr, filePath))
        {
            // Jos macroa ei lˆydy nyky hakemistosta, pit‰‰ hakemisto hakea globaalisti root-hakemiston alta
            macroParamSystemPtr->SetCurrentPathByAbsolutePath(filePath);
        }
        auto realMacroParamDrawParamFileName = ::GetRealMacroParamDrawParamFileName(filePath);
        macroParamPointer->DrawParam()->InitFileName(realMacroParamDrawParamFileName);
        macroParamPointer->DrawParam()->MacroParamRelativePath(macroParamSystemPtr->RelativePath());
        DoFinalMacroParamWrite(*macroParamSystemPtr, macroParamPointer);

        UpdateMacroParamDisplayList(true);
        CString macroParamNameU = CA2T(fileString.Header());
        macroParamSystemPtr->FindMacroFromCurrentFolder(std::string(fileString.Header()));
        itsMacroParamList.SetCurSel(itsMacroParamList.FindString(-1, macroParamNameU)); // asettaa talletetun macroParamin aktiiviseksi
        UpdateLoadedMacroParamPathString();
        if(updateViews)
        {
            std::vector<std::string> modifiedMacroParamPaths{ realMacroParamDrawParamFileName };
            itsSmartMetDocumentInterface->MacroParamDataCache().clearMacroParamCache(modifiedMacroParamPaths);
            RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param save", false, filePath); // p‰ivitet‰‰n varmuuden vuoksi ruutuja, jos karttan‰ytˆll‰ olleen macroparametrin macroa on muutettu
        }
    }
}

NFmiInfoData::Type CFmiSmartToolDlg::GetUsedMacroParamType()
{
    return (this->fQ3Macro == TRUE) ? NFmiInfoData::kQ3MacroParam : NFmiInfoData::kMacroParam;
}

#ifdef min
#undef min
#undef max
#endif

void CFmiSmartToolDlg::UpdateMacroParamDisplayListAfterSpeedLoad()
{
    itsMacroParamList.ResetContent();
    auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
    auto foundMacroParamName = macroParamSystemPtr->GetCurrentMacroParam()->Name();
    boost::shared_ptr<NFmiMacroParamFolder> currentFolder = macroParamSystemPtr->GetCurrentFolder();
    if(currentFolder)
    {
        currentFolder->RefreshMacroParams(); // t‰ss‰ p‰ivitet‰‰n vain smartTool-dialogin k‰ytt‰m‰ current-hakemisto, mutta ei esim. pop-up valikoiden k‰ytt‰m‰‰ puu rakennetta
        std::vector<std::string> stringVector(currentFolder->GetDialogListStrings(this->fQ3Macro == TRUE));
        std::vector<std::string>::iterator it = stringVector.begin();
        for(; it != stringVector.end(); ++it)
        {
            itsMacroParamList.AddString(CA2T((*it).c_str()));
        }
        auto selectedMacroName = currentFolder->Current()->Name();
        auto selectedNameIndex = itsMacroParamList.FindStringExact(0, CA2T(selectedMacroName.c_str()));
        if(selectedNameIndex == LB_ERR)
            itsMacroParamList.SetCurSel(currentFolder->CurrentIndex());
        else
            itsMacroParamList.SetCurSel(selectedNameIndex);

        // currentFolder->RefreshMacroParams operaation j‰lkeen palautetaan valittu macroParam taas oikeasti valituksi
        macroParamSystemPtr->FindMacroFromCurrentFolder(foundMacroParamName);
    }
}

void CFmiSmartToolDlg::UpdateMacroParamDisplayList(bool fForceThreadUpdate)
{
	int currentSelection = itsMacroParamList.GetCurSel();
	itsMacroParamList.ResetContent();
    auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
	boost::shared_ptr<NFmiMacroParamFolder> currentFolder = macroParamSystemPtr->GetCurrentFolder();
	if(currentFolder)
	{
		currentFolder->RefreshMacroParams(); // t‰ss‰ p‰ivitet‰‰n vain smartTool-dialogin k‰ytt‰m‰ current-hakemisto, mutta ei esim. pop-up valikoiden k‰ytt‰m‰‰ puu rakennetta
		std::vector<std::string> stringVector(currentFolder->GetDialogListStrings(this->fQ3Macro == TRUE));
		std::vector<std::string>::iterator it = stringVector.begin();
		for( ; it != stringVector.end(); ++it)
            itsMacroParamList.AddString(CA2T((*it).c_str()));
	}
    int usedSelectionIndex = std::min(currentSelection, itsMacroParamList.GetCount() - 1);
	itsMacroParamList.SetCurSel(usedSelectionIndex);
    if(macroParamSystemPtr->FindMacroFromCurrentFolder(GetSelectedMacroParamName()))
    {
        // Ei p‰ivitet‰ valitun macroParamin sis‰ltˆ‰, koska se 'resetoi' mahdolliset k‰ytt‰j‰n tekem‰t editoinnit
////        LoadFormulaFromMacroParam();
    }

    if(fForceThreadUpdate)
    {
		CFmiMacroParamUpdateThread::ForceUpdate(); // t‰m‰ pakottaa ett‰ myˆs pop-up valikoiden k‰ytt‰m‰‰ puu rakennetta p‰ivitet‰‰n
        itsSmartMetDocumentInterface->UpdateParameterSelectionSystem();
    }
    ResetSpeedSearchResources(false, true);
}

std::string CFmiSmartToolDlg::GetSelectedMacroParamName() const
{
    int index = itsMacroParamList.GetCurSel();
    if(index != LB_ERR)
    {
        CString nameU_;
        itsMacroParamList.GetText(index, nameU_);
        return std::string(CT2A(nameU_));
    }
    return "";
}

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamRemove()
{
    auto index = itsMacroParamList.GetCurSel();
    auto macroParamName = GetSelectedMacroParamName();
    if(!macroParamName.empty())
	{
		std::string str;
		std::string titleStr;
		if(macroParamName[0] == '<')
		{
			str += ::GetDictionaryString("SmartToolDlgMacroParamRemoveDir");
			str += ":\n";
			str += macroParamName;
			titleStr = ::GetDictionaryString("SmartToolDlgMacroParamRemoveDir2");
		}
		else
		{
			str += ::GetDictionaryString("SmartToolDlgMacroParamRemove");
			str += ":\n";
			str += macroParamName;
			titleStr = ::GetDictionaryString("SmartToolDlgMacroParamRemove2");
		}

        if (::MessageBox(this->GetSafeHwnd(), CA2T(str.c_str()), CA2T(titleStr.c_str()), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		{
            itsSmartMetDocumentInterface->RemoveMacroParam(macroParamName);
            CatLog::logMessage(string("Removed macroParam: ") + macroParamName, CatLog::Severity::Info, CatLog::Category::Macro);

			UpdateMacroParamDisplayList(true);
			int ssize = itsMacroParamList.GetCount();
			if(ssize > 0)
			{
				if(index == 0)
					itsMacroParamList.SetCurSel(index);
				else
					itsMacroParamList.SetCurSel(index-1);
			}
			else
				itsMacroParamList.SetCurSel(LB_ERR);
			RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param remove", false, "", false); // p‰ivitet‰‰n varmuuden vuoksi ruutuja, jos poitettiin karttan‰ytˆlt‰ macroparametri
		}
	}
}

void CFmiSmartToolDlg::AddSelectedMacroParamToRow(int theRow)
{
	UpdateData(TRUE);
    auto macroParamName = GetSelectedMacroParamName();
    if(!macroParamName.empty())
    {
		if(fCrossSectionMode)
            itsSmartMetDocumentInterface->AddMacroParamToCrossSectionView(theRow, macroParamName);
		else
            itsSmartMetDocumentInterface->AddMacroParamToView(itsSelectedMapViewDescTopIndex, theRow, macroParamName);
		RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param added to some view", false, "", false);
	}
}

void CFmiSmartToolDlg::OnBnClickedButtonAddToRow1()
{
	AddSelectedMacroParamToRow(1);
}

void CFmiSmartToolDlg::OnBnClickedButtonAddToRow2()
{
	AddSelectedMacroParamToRow(2);
}

void CFmiSmartToolDlg::OnBnClickedButtonAddToRow3()
{
	AddSelectedMacroParamToRow(3);
}

void CFmiSmartToolDlg::OnBnClickedButtonAddToRow4()
{
	AddSelectedMacroParamToRow(4);
}

void CFmiSmartToolDlg::OnBnClickedButtonAddToRow5()
{
	AddSelectedMacroParamToRow(5);
}

std::string CFmiSmartToolDlg::GetMacroParamFilePath(NFmiMacroParamSystem &theMacroParamSystem)
{
    auto macroParamFolder = theMacroParamSystem.GetCurrentFolder();
    auto macroParam = theMacroParamSystem.GetCurrentMacroParam();
    if(macroParamFolder && macroParam)
    {
        std::string drawParamFileName, macroParamFileName;
        MacroParam::GetFileNames(macroParamFolder->Path(), macroParam->Name(), drawParamFileName, macroParamFileName, GetUsedMacroParamType());
        return macroParamFileName;
    }
    return "";
}

void CFmiSmartToolDlg::OnLbnSelchangeListParamMacros()
{
    auto macroParamName = GetSelectedMacroParamName();
    DoMacroParamLoad(macroParamName, false);
}

void CFmiSmartToolDlg::DoMacroParamLoad(const std::string& theMacroParamName, bool fDoSpeedLoad)
{
    if(!theMacroParamName.empty())
    {
        auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
        bool macroParamFound = false;
        if(fDoSpeedLoad)
        {
            macroParamFound = macroParamSystemPtr->FindMacroParamPath(theMacroParamName);
            if(macroParamFound)
            {
                UpdateMacroParamDisplayListAfterSpeedLoad();
            }
        }
        else
            macroParamFound = macroParamSystemPtr->FindMacroFromCurrentFolder(theMacroParamName);

        if(macroParamFound)
        {
            LoadFormulaFromMacroParam();
            itsSmartToolInfo->CurrentScript(GetSmarttoolFormulaText()); // p‰ivitet‰‰n myˆs currentiksi macro-tekstiksi
        }
    }

    UpdateData(FALSE);
}

void CFmiSmartToolDlg::OnLbnDblclkListParamMacros()
{ // t‰ss‰ on tarkoitus menn‰ alihakemistoihin, jos klikattu oli hakemisto
	// ja ei tehd‰ mit‰‰n jos oli tavallinen macroParam

    auto macroParamName = GetSelectedMacroParamName();
    if(!macroParamName.empty())
    {
		if(macroParamName[0] == '<')
		{ // Jos hakemiston nimi, siirryt‰‰n sinne
            itsSmartMetDocumentInterface->MacroParamSystem()->CurrentPath(macroParamName);
			UpdateMacroParamDisplayList(false);
            UpdateLoadedMacroParamPathString();
        }
	}
}

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamLatestErrorText()
{
	if(itsSmartMetDocumentInterface->GetLatestMacroParamErrorText().empty())
        ::MessageBox(this->GetSafeHwnd(), CA2T(::GetDictionaryString("SmartToolDlgMacroParamNoLatestError").c_str()), CA2T(::GetDictionaryString("SmartToolDlgMacroParamLatestErrorTitle").c_str()), MB_ICONINFORMATION | MB_OK);
	else
        ::MessageBox(this->GetSafeHwnd(), CA2T(itsSmartMetDocumentInterface->GetLatestMacroParamErrorText().c_str()), CA2T(::GetDictionaryString("SmartToolDlgMacroParamLatestErrorTitle").c_str()), MB_ICONINFORMATION | MB_OK);
}

void CFmiSmartToolDlg::OnBnClickedButtonSmartToolRemove()
{
	CFmiSmartToolLoadingDlg dlg(itsSmartToolInfo, this);
    CatLog::logMessage(string("Opening smartTool removal dialog."), CatLog::Severity::Info, CatLog::Category::Macro);

	if(dlg.DoModal() == IDOK)
	{
        ResetSpeedSearchResources(true, false);
        bool status = itsSmartToolInfo->LoadScript(dlg.SelectedScriptName());
		if(status)
		{
            LoadFormulaFromSmarttool();
            itsMacroParamList.SetCurSel(LB_ERR); // laitetaan macroParamlista osoittamaan 'ei mit‰‰n'
            UpdateData(FALSE);
		}
	}
}

void CFmiSmartToolDlg::ClearMapViewRow(int theRowIndex)
{
	UpdateData(TRUE);
	if(itsSmartMetDocumentInterface)
	{
        if(fCrossSectionMode)
        {
            itsSmartMetDocumentInterface->RemoveAllParamsFromCrossSectionViewRow(theRowIndex);
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::CrossSectionView);
        }
        else
        {
            itsSmartMetDocumentInterface->RemoveAllParamsFromMapViewRow(itsSelectedMapViewDescTopIndex, theRowIndex);
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(::GetWantedMapViewIdFlag(itsSelectedMapViewDescTopIndex));
        }
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("SmartToolDlg: clear view row's all params");
	}
}

void CFmiSmartToolDlg::OnBnClickedButtonRemoveAllFromRow1()
{
	ClearMapViewRow(1);
}

void CFmiSmartToolDlg::OnBnClickedButtonRemoveAllFromRow2()
{
	ClearMapViewRow(2);
}

void CFmiSmartToolDlg::OnBnClickedButtonRemoveAllFromRow3()
{
	ClearMapViewRow(3);
}

void CFmiSmartToolDlg::OnBnClickedButtonRemoveAllFromRow4()
{
	ClearMapViewRow(4);
}

void CFmiSmartToolDlg::OnBnClickedButtonRemoveAllFromRow5()
{
	ClearMapViewRow(5);
}

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamRefreshList()
{
    auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
    // Varmistetaan ett‰ root kansio on alustettu tai ett‰ edes se on 'valittuna', jos tausta-threadissa 
    // tehty MacroParamSystem olion alustus on jotenkin estynyt tai mennyt pieleen.
    macroParamSystemPtr->EnsureRootFolderInitialized();
    boost::shared_ptr<NFmiMacroParamFolder> currentFolder = macroParamSystemPtr->GetCurrentFolder();
	if(currentFolder)
	{
		currentFolder->RefreshMacroParams();
		UpdateMacroParamDisplayList(true);
        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::SmartToolDlg);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("SmartToolDlg: Refresh macroParam list");
		UpdateData(FALSE);
	}
}

// T‰m‰ funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell‰.
// T‰m‰ on ik‰v‰ kyll‰ teht‰v‰ erikseen dialogin muokkaus tyˆkalusta, eli
// tekij‰n pit‰‰ lis‰t‰ erikseen t‰nne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiSmartToolDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("SmartToolDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_ACTION, "IDC_BUTTON_ACTION");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_STR, "IDC_STATIC_MACRO_STR");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_MODIFY_ONLY_SELECTED_LOCATIONS, "IDC_CHECK_MODIFY_ONLY_SELECTED_LOCATIONS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_ERROR_STR, "IDC_STATIC_MACRO_ERROR_STR");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SMARTTOOL_HOLDER, "Smarttool section");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_SAVE, "Save");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_SAVE_AS, "Save as");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_LOAD, "Load");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_REMOVE, "Remove");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_SAVE_DB_CHECKER, "IDC_BUTTON_SMART_TOOL_SAVE_DB_CHECKER");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_LOAD_DB_CHECKER, "IDC_BUTTON_SMART_TOOL_LOAD_DB_CHECKER");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_MAKE_DB_CHECK_AT_SEND, "IDC_CHECK_MAKE_DB_CHECK_AT_SEND");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PARAM_HOLDER, "IDC_STATIC_MACRO_PARAM_HOLDER");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_SAVE, "Save MacroPar");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_SAVE_AS, "Save as MacroPar");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_REMOVE, "IDC_BUTTON_MACRO_PARAM_REMOVE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_REFRESH_LIST, "IDC_BUTTON_MACRO_PARAM_REFRESH_LIST");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_LATEST_ERROR_TEXT, "IDC_BUTTON_MACRO_PARAM_LATEST_ERROR_TEXT");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PARAM_DATA_GRID_SIZE_STR, "IDC_STATIC_MACRO_PARAM_DATA_GRID_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_DATA_GRID_SIZE_USE, "NormalWordCapitalUse");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_Q3_MACRO, "Q3 scripts");
}

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamDataGridSizeUse()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->SetMacroParamDataGridSize(itsMacroParamDataGridSizeX, itsMacroParamDataGridSizeY);
	NFmiPoint trueGridSize(itsSmartMetDocumentInterface->InfoOrganizer()->GetMacroParamDataGridSize()); // t‰ss‰ haetaan tarkistetut koot takaisin
	itsMacroParamDataGridSizeX = static_cast<int>(trueGridSize.X());
	itsMacroParamDataGridSizeY = static_cast<int>(trueGridSize.Y());
	UpdateData(FALSE);
}

void CFmiSmartToolDlg::OnBnClickedCheckMacroParamCrosssectionMode()
{
	UpdateData(TRUE);
}

void CFmiSmartToolDlg::OnBnClickedCheckQ3Macro()
{
	UpdateData(TRUE);
	UpdateMacroParamDisplayList(false);
	UpdateData(FALSE);
}

void CFmiSmartToolDlg::OnBnClickedRadioViewmacroSelectedMap1()
{
	UpdateData(TRUE);
}

void CFmiSmartToolDlg::OnBnClickedRadioViewmacroSelectedMap2()
{
	UpdateData(TRUE);
}

void CFmiSmartToolDlg::OnBnClickedRadioViewmacroSelectedMap3()
{
	UpdateData(TRUE);
}

BOOL CFmiSmartToolDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if(message == ID_MESSAGE_SMARTTOOL_MODIFICATION_ENDED)
		EnableDlgItem(IDC_BUTTON_ACTION, true); // enabloidaan smarttool muokkausten j‰lkeen taas muokkaa-nappi

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

void CFmiSmartToolDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // set the minimum tracking width and height of the window
    lpMMI->ptMinTrackSize.x = 553;
    lpMMI->ptMinTrackSize.y = 599;
}

void CFmiSmartToolDlg::UpdateLoadedSmarttoolMacroPathString()
{
    std::string usedLoadedMacroPath = itsSmartToolInfo->GetRelativeLoadPath();
    usedLoadedMacroPath += itsSmartToolInfo->CurrentScriptName();
    itsLoadedSmarttoolMacroPathU_ = CA2T(usedLoadedMacroPath.c_str());
    UpdateData(FALSE);
}

void CFmiSmartToolDlg::UpdateLoadedMacroParamPathString()
{
    auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
    if(macroParamSystemPtr->GetCurrentMacroParam())
    {
        std::string usedLoadedMacroPath = macroParamSystemPtr->RelativePath();
        usedLoadedMacroPath += "\\" + macroParamSystemPtr->GetCurrentMacroParam()->Name();
        itsLoadedMacroParamPathTextU_ = CA2T(usedLoadedMacroPath.c_str());
        UpdateData(FALSE);
    }
}

static void DoSpeedSearchPopup(CWnd *view, CGUSIconEdit &speedSearchControl, CMenu &speedSearchControlMenu, BOOL searchOptionCaseSensitive, BOOL searchOptionMatchAnywhere)
{
    CRect rcEdit;
    speedSearchControl.GetWindowRect(&rcEdit);

    CMenu* pMenu = speedSearchControlMenu.GetSubMenu(0);
    pMenu->CheckMenuItem(ID_SEARCHOPTION_CASESENSITIVE, searchOptionCaseSensitive ? MF_CHECKED : MF_UNCHECKED);
    pMenu->CheckMenuItem(ID_SEARCHOPTION_MATCHANYWHERE, searchOptionMatchAnywhere ? MF_CHECKED : MF_UNCHECKED);
    pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rcEdit.left, rcEdit.bottom, view);
}

LRESULT CFmiSmartToolDlg::OnGUSIconEditLeftIconClicked(WPARAM wParam, LPARAM lParam)
{
    CRect rcEdit;

    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL == nCtrlId)
        ::DoSpeedSearchPopup(this, itsSpeedSearchMacroControl, itsSpeedSearchMacroControlMenu, fSearchOptionCaseSensitive, fSearchOptionMatchAnywhere);
    else if(IDC_EDIT_SPEED_SEARCH_MACRO_PARAM_CONTROL == nCtrlId)
        ::DoSpeedSearchPopup(this, itsSpeedSearchMacroParamControl, itsSpeedSearchMacroControlMenu, fSearchOptionCaseSensitive, fSearchOptionMatchAnywhere);

    return 1;
}

static void ClearSpeedSearchControl(CGUSIconEdit& speedSearchControl)
{
    if(speedSearchControl.GetEditTextLength() > 0)
        speedSearchControl.ClearEditText();
}


LRESULT CFmiSmartToolDlg::OnGUSIconEditRightIconClicked(WPARAM wParam, LPARAM lParam)
{
    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL == nCtrlId)
        ::ClearSpeedSearchControl(itsSpeedSearchMacroControl);
    else if(IDC_EDIT_SPEED_SEARCH_MACRO_PARAM_CONTROL == nCtrlId)
        ::ClearSpeedSearchControl(itsSpeedSearchMacroParamControl);

    return 1;
}

LRESULT CFmiSmartToolDlg::OnGUSIconEditSearchListClicked(WPARAM wParam, LPARAM lParam)
{
    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL == nCtrlId)
    {
        std::string selectedMacroName = CT2A(itsSpeedSearchMacroControl.GetSelectedString());
        DoSmartToolLoad(selectedMacroName, true);
        itsSpeedSearchMacroParamControl.ClearEditText();
    }
    else if(IDC_EDIT_SPEED_SEARCH_MACRO_PARAM_CONTROL == nCtrlId)
    {
        std::string selectedMacroParamName = CT2A(itsSpeedSearchMacroParamControl.GetSelectedString());
        DoMacroParamLoad(selectedMacroParamName, true);
        itsSpeedSearchMacroControl.ClearEditText();
    }

    return 1;
}

void CFmiSmartToolDlg::OnSearchOptionCaseSesensitive()
{
    fSearchOptionCaseSensitive = !fSearchOptionCaseSensitive;
    itsSpeedSearchMacroControl.SetSearchCaseSense(fSearchOptionCaseSensitive);
    itsSpeedSearchMacroParamControl.SetSearchCaseSense(fSearchOptionCaseSensitive);
}

void CFmiSmartToolDlg::OnSearchOptionMatchAnywhere()
{
    fSearchOptionMatchAnywhere = !fSearchOptionMatchAnywhere;
    itsSpeedSearchMacroControl.SetSearchAnywhere(fSearchOptionMatchAnywhere);
    itsSpeedSearchMacroParamControl.SetSearchAnywhere(fSearchOptionMatchAnywhere);
}

static void DoEnChangeForSpeedSearchControl(CGUSIconEdit& speedSearchControl)
{
    if(speedSearchControl.GetResultsCount() > 0)
    {
        speedSearchControl.SetTextColor(RGB(0, 0, 0));
    }
    else
    {
        speedSearchControl.SetTextColor(RGB(255, 0, 0));
    }
}

void CFmiSmartToolDlg::OnEnChangeEditSpeedSearchMacro()
{
    ::DoEnChangeForSpeedSearchControl(itsSpeedSearchMacroControl);
}

void CFmiSmartToolDlg::OnEnChangeEditSpeedSearchMacroParam()
{
    ::DoEnChangeForSpeedSearchControl(itsSpeedSearchMacroParamControl);
}

static void ResetSpeedSearchResourcesForControl(CGUSIconEdit& speedSearchControl, const std::vector<std::string> &macroNames)
{
    speedSearchControl.RemoveAll();
    for(const auto& name : macroNames)
        speedSearchControl.AddSearchString(CA2T(name.c_str()));
}

void CFmiSmartToolDlg::ResetSpeedSearchResources(bool doSmartTools, bool doMacroParams)
{
    if(doSmartTools)
    {
        ::ResetSpeedSearchResourcesForControl(itsSpeedSearchMacroControl, itsSmartMetDocumentInterface->SmartToolFileNames(true));
    }
    if(doMacroParams)
    {
        ::ResetSpeedSearchResourcesForControl(itsSpeedSearchMacroParamControl, itsSmartMetDocumentInterface->MacroParamSystem()->MacroParamSpeedSearchPathNames(true));
    }
}

static void InitSpeedSearchControl(CGUSIconEdit& speedSearchControl, const std::string& startingText, BOOL searchOptionCaseSensitive, BOOL searchOptionMatchAnywhere)
{
    // Init the first edit.
    // We just use default parameters.
    // You can find the skin rect from "GUSIconEditSkin.bmp" and "GUSIconEditSkin.offset.txt".
    auto initStatus = speedSearchControl.InitializeResource(
        IDB_BITMAP_GUS_ICON_EDIT_SKIN,
        CRect(3, 69, 27, 90),
        CRect(3, 69, 27, 90),
        CRect(26, 47, 27, 68),
        CRect(26, 47, 27, 68),
        CRect(48, 47, 67, 68),
        CRect(28, 69, 47, 90),
        CRect(5, 5, 22, 22),
        CRect(1, 4, 14, 17)
        );

    speedSearchControl.SetEmptyTips(CA2T((::GetDictionaryString(startingText.c_str()).c_str())));
    speedSearchControl.SetSearchCaseSense(searchOptionCaseSensitive);
    speedSearchControl.SetSearchAnywhere(searchOptionMatchAnywhere);

    // CGUSIconEdit -kontrollin muita mahdollisia optioita kommenteissa
    //speedSearchControl.SetMaxDisplayItem(20);
    //speedSearchControl.SetNormalItemsBKColor(RGB(201, 241, 238));
    //speedSearchControl.SetEvenItemsBKColor(RGB(201, 187, 234));
    //speedSearchControl.SetNonSelItemsKeywordsBKColor(RGB(248, 134, 151));
    //speedSearchControl.SetNormalItemsTextColor(RGB(104, 36, 94));
    //speedSearchControl.SetSelItemBKColor(RGB(221, 215, 89), TRUE);
    //speedSearchControl.SetSelItemTextColor(RGB(203, 7, 75));
    //speedSearchControl.SetRememberWindowNewSize(TRUE);
    //speedSearchControl.SetSortDisplayedSearchResults(TRUE, TRUE);
}

void CFmiSmartToolDlg::InitSpeedSearchControls()
{
    itsSpeedSearchMacroControlMenu.LoadMenu(IDR_MENU_GUS_ICON_EDIT);
    ::InitSpeedSearchControl(itsSpeedSearchMacroControl, "Search smarttool macros...", fSearchOptionCaseSensitive, fSearchOptionMatchAnywhere);
    ::InitSpeedSearchControl(itsSpeedSearchMacroParamControl, "Search macro-params...", fSearchOptionCaseSensitive, fSearchOptionMatchAnywhere);
}

// Tehd‰‰n ainakin 15 minuutin v‰lein pikahaku listan p‰ivityst‰, jos tulee uusia smarttool-macroja synkronoinnin kautta
// HUOM! Tehd‰‰n t‰m‰ p‰ivitys yritys vain tietyist‰ toiminnoista, esim. smarttool-macron lataus ja pika-etsint‰ valinnan yhteydess‰.
// En halua ett‰ t‰t‰ kutsutaan kesken mahdollista pikahaun tekoa jolloin seurauksena aletaan resetoimaan jo tehty‰ hakua.
void CFmiSmartToolDlg::DoTimedResetSearchResource()
{
    static NFmiMilliSecondTimer timer;

    if(timer.CurrentTimeDiffInMSeconds() > 15 * 60 * 1000)
    {
        ResetSpeedSearchResources(true, true);
        timer.StartTimer(); // k‰ynnistet‰‰n uusi laskuri
    }
}

void CFmiSmartToolDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

#ifndef DISABLE_EXTREME_TOOLKITPRO
    AdjustSyntaxEditControlWindows();
#endif // DISABLE_EXTREME_TOOLKITPRO
}

HBRUSH CFmiSmartToolDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if(pWnd->GetDlgCtrlID() == IDC_CHECK_Q3_MACRO)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, !fQ3Macro);

    return hbr;
}

void CFmiSmartToolDlg::OnBnClickedCheckShowTooltipOnSmarttoolDialog()
{
    UpdateData(TRUE);
    itsSmartMetDocumentInterface->ApplicationWinRegistry().ShowTooltipOnSmarttoolDialog(fShowTooltipsOnSmarttoolDialog == TRUE);
}

void CFmiSmartToolDlg::OpenMacroParamForEditing(const std::string& absoluteMacroParamPath)
{
    auto macroParamSystemPtr = itsSmartMetDocumentInterface->MacroParamSystem();
    if(!macroParamSystemPtr)
        return;

    auto usedMacroParamPath = PathUtils::getRelativePathIfPossible(absoluteMacroParamPath, macroParamSystemPtr->RootPath());
    DoMacroParamLoad(usedMacroParamPath, true);
}
