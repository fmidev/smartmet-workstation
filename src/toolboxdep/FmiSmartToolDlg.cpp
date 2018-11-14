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
,itsMacroParamNameU_(_T(""))
, itsMacroNameU_(_T(""))
,itsMacroParamDataGridSizeX(0)
,itsMacroParamDataGridSizeY(0)
,fCrossSectionMode(FALSE)
,fQ3Macro(FALSE)
,fSearchOptionCaseSensitive(FALSE)
,fSearchOptionMatchAnywhere(TRUE)
,itsUsedMacroPathU_(_T(""))
#ifndef DISABLE_EXTREME_TOOLKITPRO
,itsSyntaxEditControl()
,itsSyntaxEditControlAcceleratorTable(NULL)
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
    DDX_Text(pDX, IDC_EDIT_MACRO_PARAM_NAME, itsMacroParamNameU_);
    DDX_Control(pDX, IDC_LIST_PARAM_MACROS, itsMacroParamList);
    DDX_Text(pDX, IDC_EDIT_MACRO_NAME, itsMacroNameU_);
    DDX_Text(pDX, IDC_EDIT_MACRO_PARAM_DATA_GRID_SIZE_X, itsMacroParamDataGridSizeX);
    DDX_Text(pDX, IDC_EDIT_MACRO_PARAM_DATA_GRID_SIZE_Y, itsMacroParamDataGridSizeY);
    DDX_Check(pDX, IDC_CHECK_MACRO_PARAM_CROSSSECTION_MODE, fCrossSectionMode);
    DDX_Check(pDX, IDC_CHECK_Q3_MACRO, fQ3Macro);
    DDX_Radio(pDX, IDC_RADIO_VIEWMACRO_SELECTED_MAP1, itsSelectedMapViewDescTopIndex);
    DDX_Control(pDX, IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, itsSpeedSearchMacroControl);
    DDX_Text(pDX, IDC_STATIC_MACRO_PATH_TEXT, itsUsedMacroPathU_);
}


BEGIN_MESSAGE_MAP(CFmiSmartToolDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiSmartToolDlg)
	ON_BN_CLICKED(IDC_BUTTON_ACTION, OnButtonAction)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_LOAD, OnButtonSmartToolLoad)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_LOAD_DB_CHECKER, OnButtonSmartToolLoadDbChecker)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_SAVE, OnButtonSmartToolSave)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOL_SAVE_DB_CHECKER, OnButtonSmartToolSaveDbChecker)
	ON_BN_CLICKED(IDC_CHECK_MAKE_DB_CHECK_AT_SEND, OnCheckMakeDbCheckAtSend)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_MODIFY_ONLY_SELECTED_LOCATIONS, OnBnClickedCheckModifyOnlySelectedLocations)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_SAVE, OnBnClickedButtonMacroParamSave)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_REMOVE, OnBnClickedButtonMacroParamRemove)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_PROPERTIES, OnBnClickedButtonMacroParamProperties)
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
    ON_EN_CHANGE(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, &CFmiSmartToolDlg::OnEnChangeEditSpeedSearchViewMacro)
    ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiSmartToolDlg message handlers

BOOL CFmiSmartToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    DoResizerHooking(); // T‰t‰ pit‰‰ kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisterist‰

	std::string errorBaseStr("Error in CFmiSmartToolDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

    HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

#ifndef DISABLE_EXTREME_TOOLKITPRO
    InitializeSyntaxEditControl();
#else
    WarnUserAboutNoEditingSmarttools();
#endif // DISABLE_EXTREME_TOOLKITPRO

	fMakeDBCheckAtSend = itsSmartToolInfo->MakeDBCheckAtSend();
	UpdateMacroName();
	UpdateMacroParamDisplayList(false);

	InitDialogTexts();
	NFmiPoint gridSize(itsSmartMetDocumentInterface->InfoOrganizer()->GetMacroParamDataGridSize());
	itsMacroParamDataGridSizeX = static_cast<int>(gridSize.X());
	itsMacroParamDataGridSizeY = static_cast<int>(gridSize.Y());
    UpdateLoadDirectoryString();
    InitSpeedSearchControl();
    ResetSearchResource();
    DisableActionButtomIfInViewMode();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
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

// Huom! T‰h‰n metodiin tulee theFilePath arvoja, joita ei lˆydy ja ne heitt‰v‰t poikkeuksen:
// 1. Kun vaihdetaan kansiota
// 2. K‰ynnistyess‰ "Last smarttool" -muistissa on tiedosto, joka sijaitsee jossain alihakemistossa eik‰ root-hakemistossa.
bool CFmiSmartToolDlg::LoadSmarttoolToSyntaxEditControl(const std::string &theFilePath)
{
    try
    {
        CXTPSyntaxEditBufferManager* pDataMan = itsSyntaxEditControl->GetEditBuffer();
        if(pDataMan)
        {
            pDataMan->SetFileExt(_TEXT(".st"));

            CFile fileText(CA2T(theFilePath.c_str()), CFile::modeRead);
            CArchive arText(&fileText, CArchive::load);

            pDataMan->Serialize(arText);

            itsSyntaxEditControl->RefreshColors();
            itsSyntaxEditControl->RecalcScrollBars();

            itsSyntaxEditControl->SendMessage(WM_NCPAINT);

            UpdateSyntaxEditControl();
            return true;
        }
    }
    catch(CFileException *pE)
    {
//        pE->ReportError();
        pE->Delete(); // Poikkeukset pit‰‰ napata, ja kutsua niiden Delete -metodia, muuten ne vuotavat!!
    }
    catch(CArchiveException *pE)
    {
        //pE->ReportError();
        pE->Delete();
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
        CFile fileText(CA2T(theFilePath.c_str()), CFile::modeWrite | CFile::modeCreate);
        CArchive arText(&fileText, CArchive::store);

        CXTPSyntaxEditBufferManager* pDataMan = itsSyntaxEditControl->GetEditBuffer();
        if(pDataMan)
        {
            pDataMan->Serialize(arText);

            arText.Close();
            fileText.Close();

            itsSyntaxEditControl->RefreshColors();
            UpdateSyntaxEditControl();
            return true;
        }
    }
    catch(CFileException *pE)
    {
        //pE->ReportError();
        pE->Delete();
    }
    catch(CArchiveException *pE)
    {
        //pE->ReportError();
        pE->Delete();
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
    bOk = m_resizer.SetAnchor(IDC_EDIT_MACRO_NAME, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_SYNTAX_EDIT_CONTROL_PLACER, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_ERROR_STR, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RICHEDIT_MACRO_ERROR_TEXT, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMART_TOOL_SAVE, ANCHOR_TOP | ANCHOR_RIGHT);
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
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_REMOVE, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_PROPERTIES, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_REFRESH_LIST, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACRO_PARAM_LATEST_ERROR_TEXT, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_Q3_MACRO, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_PARAM_NAME_STR, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_MACRO_PARAM_NAME, ANCHOR_BOTTOM | ANCHOR_LEFT);
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
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_PATH_TEXT, ANCHOR_TOP | ANCHOR_RIGHT);
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
        if(itsSmartMetDocumentInterface->MacroParamSystem().UpdateMacroParamListView())
        {
            itsSmartMetDocumentInterface->MacroParamSystem().UpdateMacroParamListView(false);
            UpdateMacroParamDisplayList(false);
        }
        DisableActionButtomIfInViewMode();
#ifndef DISABLE_EXTREME_TOOLKITPRO
        UpdateSyntaxEditControl();
#endif // DISABLE_EXTREME_TOOLKITPRO
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

bool CFmiSmartToolDlg::LoadSmarttoolFormula(const std::string &theFilePath)
{
#ifndef DISABLE_EXTREME_TOOLKITPRO
    return LoadSmarttoolToSyntaxEditControl(theFilePath);
#else
    return false;
#endif // DISABLE_EXTREME_TOOLKITPRO
}

void CFmiSmartToolDlg::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool editedDataModified, const std::string &possibleEditedDrawParamPath)
{
    if(!possibleEditedDrawParamPath.empty())
    {
        std::vector<std::string> modifiedMacroParamPathList{ possibleEditedDrawParamPath };
        itsSmartMetDocumentInterface->MacroParamDataCache().clearMacroParamCache(modifiedMacroParamPathList);
    }
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, false, false, editedDataModified);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(reasonForUpdate);
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
    UpdateLoadDirectoryString();
    if(status)
    {
        CatLog::logMessage(string("Loaded smartTool: ") + theSmartToolName, CatLog::Severity::Info, CatLog::Category::Macro);
        LoadSmarttoolFormula(GetSmarttoolFilePath());
        UpdateMacroName();
        itsMacroParamNameU_ = _TEXT(""); // tyhjennet‰‰n varmuuden vuoksi makroParam name, koska kaksi save-nappia voi aiheuttaa sekaannuksia
        itsMacroParamList.SetCurSel(LB_ERR); // laitetaan macroParamlista osoittamaan 'ei mit‰‰n'
        UpdateData(FALSE);
    }
}

void CFmiSmartToolDlg::OnButtonSmartToolLoadDbChecker()
{ // ladataan virallinen DBCheck-macro ruudulle esim. testi k‰yttˆˆn
    LoadSmarttoolFormula(itsSmartToolInfo->DBCheckerFileName());
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

void CFmiSmartToolDlg::OnButtonSmartToolSave()
{
	UpdateData(TRUE);
    string tmpName = CT2A(itsMacroNameU_);
	NFmiStringTools::Trim(tmpName, ' '); // pakko trimmata pois mahdolliset vasemman/oikeanpuoleiset v‰lilyˆnnit, muuten makroa ei voi valita en‰‰ myˆhemmin
	if(tmpName.empty())
		return ;
    bool scriptExist = itsSmartToolInfo->ScriptExist(tmpName);
    if(scriptExist)
	{
		std::string messageStr(::GetDictionaryString("SmartToolDlgOverWrite"));
		messageStr += "\n\"";
		messageStr += tmpName;
		messageStr += "\"";
        if (::MessageBox(this->GetSafeHwnd(), CA2T(messageStr.c_str()), CA2T(::GetDictionaryString("SmartToolDlgOverWrite2").c_str()), MB_ICONINFORMATION | MB_OKCANCEL) != IDOK)
			return ;
	}
	// pit‰‰ ensin tallettaa currenttiksi skriptiksi ja sitten tallettaa tiedostoon
	itsSmartToolInfo->CurrentScript(GetSmarttoolFormulaText());
    if(itsSmartToolInfo->SaveScript(tmpName))
    {
        CatLog::logMessage(string("Saved smartTool: ") + string(tmpName), CatLog::Severity::Info, CatLog::Category::Macro);
        if(!scriptExist)
            ResetSearchResource();
    }
	else
	{
		std::string errMsgTitle("Cannot save smarttool skript");
		std::string errMsg("Cannot save smarttool skript:\n");
		std::string fullFileName(itsSmartToolInfo->GetFullScriptFileName(tmpName));
		errMsg += fullFileName;
		errMsg += "\nCheck that the folder exists and you have write permissions there.";

        ::MessageBox(this->GetSafeHwnd(), CA2T(errMsg.c_str()), CA2T(errMsgTitle.c_str()), MB_ICONINFORMATION | MB_OK);
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

void CFmiSmartToolDlg::UpdateMacroName(void)
{
    itsMacroNameU_ = CA2T(itsSmartToolInfo->CurrentScriptName().c_str());
}

void CFmiSmartToolDlg::OnBnClickedCheckModifyOnlySelectedLocations()
{
	UpdateData(TRUE);
}

bool CFmiSmartToolDlg::CreateNewMacroParamDirectory(const std::string &thePath)
{
	std::string usedDirectoryName(thePath);
	NFmiStringTools::TrimL(usedDirectoryName, '<');
	NFmiStringTools::TrimR(usedDirectoryName, '>');
	if(NFmiFileSystem::CreateDirectory(itsSmartMetDocumentInterface->MacroParamSystem().CurrentPath() + usedDirectoryName))
	{
		UpdateMacroParamDisplayList(true);
		return true;
	}
	return false;
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

static std::string MakeMacroParamInitFileName(NFmiMacroParamSystem& macroParamSystem, const std::string &macroParamName)
{
    std::string initFileName(macroParamSystem.CurrentPath());
    initFileName += macroParamName;
    initFileName += ".dpa";
    return initFileName;
}

static std::string MakeMacroParamOverWriteQuestionText(boost::shared_ptr<NFmiMacroParam> &currentMacroParam)
{
    string questionStr(::GetDictionaryString("SmartToolDlgMacroParamOverWrite1"));
    questionStr += "\n";
    questionStr += currentMacroParam->Name();
    questionStr += " ";
    questionStr += ::GetDictionaryString("SmartToolDlgMacroParamOverWrite2");
    return questionStr;
}

void CFmiSmartToolDlg::DoFinalMacroParamWrite(NFmiMacroParamSystem& macroParamSystem, boost::shared_ptr<NFmiMacroParam> &macroParamPointer)
{
    try
    {
        boost::shared_ptr<NFmiMacroParamFolder> currentFolder = macroParamSystem.GetCurrent();
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

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamSave()
{
	UpdateData(TRUE);
	std::string macroParamName = CT2A(itsMacroParamNameU_);
	NFmiStringTools::Trim(macroParamName, ' '); // pakko trimmata pois mahdolliset vasemman/oikeanpuoleiset v‰lilyˆnnit, muuten makroa ei voi valita en‰‰ myˆhemmin
	if(macroParamName.empty())
		return ;
	if(macroParamName[0] == '<')
	{
		CreateNewMacroParamDirectory(macroParamName);
		return ;
	}
	boost::shared_ptr<NFmiMacroParam> macroParamPointer = ::CreateMacroParamPointer(macroParamName, GetSmarttoolFormulaText(), GetUsedMacroParamType());
	NFmiMacroParamSystem& mpSystem = itsSmartMetDocumentInterface->MacroParamSystem();
	std::string initFileName = ::MakeMacroParamInitFileName(mpSystem, macroParamPointer->Name());

	bool updateViews = false; // kun talletetaan vanhan p‰‰lle, pit‰‰ p‰ivitt‰‰ ruutuja, koska macro-teksti on saattanut muuttua
	if(mpSystem.FindTotal(initFileName))
	{
        boost::shared_ptr<NFmiMacroParam> currentMacroParam = mpSystem.CurrentMacroParam(); // Otetaan talteen erilliseen muuttujaan, koska ilmeisesti CurrentMacroParam -pointteri voi 'korruptoitua', en tied‰ miksi, mutta SmartMet voi kaatua kun ollaan "pit‰‰ ottaa talteen vanhat piirto-ominaisuudet" -rivill‰ jompaan kumpaan DrawParam() -kutsuun ja oletan ett‰ sen t‰ytyy olla CurrentMacroParam() -kohdasta.
        string questionStr = ::MakeMacroParamOverWriteQuestionText(currentMacroParam);
        if (::MessageBox(this->GetSafeHwnd(), CA2T(questionStr.c_str()), CA2T(::GetDictionaryString("SmartToolDlgMacroParamOverWrite3").c_str()), MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL)
			return ;
        macroParamPointer->DrawParam(currentMacroParam->DrawParam()); // pit‰‰ ottaa talteen vanhat piirto-ominaisuudet!
		updateViews = true; // nyt tiedet‰‰n, ett‰ pit‰‰ p‰ivitt‰‰ n‰yttˆj‰
	}

    macroParamPointer->DrawParam()->InitFileName(initFileName);
    macroParamPointer->DrawParam()->MacroParamRelativePath(mpSystem.RelativePath());
    DoFinalMacroParamWrite(mpSystem, macroParamPointer);

	UpdateMacroParamDisplayList(true);
	itsMacroParamList.SetCurSel(itsMacroParamList.FindString(-1, itsMacroParamNameU_)); // asettaa talletetun macroParamin aktiiviseksi
	if(updateViews)
	{
        std::vector<std::string> modifiedMacroParamPaths{ initFileName };
        itsSmartMetDocumentInterface->MacroParamDataCache().clearMacroParamCache(modifiedMacroParamPaths);
		RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param save", false, initFileName); // p‰ivitet‰‰n varmuuden vuoksi ruutuja, jos karttan‰ytˆll‰ olleen macroparametrin macroa on muutettu
	}
}

NFmiInfoData::Type CFmiSmartToolDlg::GetUsedMacroParamType()
{
    return (this->fQ3Macro == TRUE) ? NFmiInfoData::kQ3MacroParam : NFmiInfoData::kMacroParam;
}

void CFmiSmartToolDlg::UpdateMacroParamDisplayList(bool fForceThreadUpdate)
{
	int currentSelection = itsMacroParamList.GetCurSel();
	itsMacroParamList.ResetContent();
	boost::shared_ptr<NFmiMacroParamFolder> currentFolder = itsSmartMetDocumentInterface->MacroParamSystem().GetCurrent();
	if(currentFolder)
	{
		currentFolder->RefreshMacroParams(); // t‰ss‰ p‰ivitet‰‰n vain smartTool-dialogin k‰ytt‰m‰ current-hakemisto, mutta ei esim. pop-up valikoiden k‰ytt‰m‰‰ puu rakennetta
		std::vector<std::string> stringVector(currentFolder->GetDialogListStrings(this->fQ3Macro == TRUE));
		std::vector<std::string>::iterator it = stringVector.begin();
		for( ; it != stringVector.end(); ++it)
            itsMacroParamList.AddString(CA2T((*it).c_str()));
	}
	itsMacroParamList.SetCurSel(currentSelection);
    if(fForceThreadUpdate)
    {
		CFmiMacroParamUpdateThread::ForceUpdate(); // t‰m‰ pakottaa ett‰ myˆs pop-up valikoiden k‰ytt‰m‰‰ puu rakennetta p‰ivitet‰‰n
        itsSmartMetDocumentInterface->UpdateParamAddingSystem();
    }
}

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamRemove()
{
	int index = itsMacroParamList.GetCurSel();
	if(index != LB_ERR)
	{
        CString nameU_;
		itsMacroParamList.GetText(index, nameU_);

		std::string str;
		std::string titleStr;
		if(nameU_[0] == '<')
		{
			str += ::GetDictionaryString("SmartToolDlgMacroParamRemoveDir");
			str += ":\n";
			str += CT2A(nameU_);
			titleStr = ::GetDictionaryString("SmartToolDlgMacroParamRemoveDir2");
		}
		else
		{
			str += ::GetDictionaryString("SmartToolDlgMacroParamRemove");
			str += ":\n";
			str += CT2A(nameU_);
			titleStr = ::GetDictionaryString("SmartToolDlgMacroParamRemove2");
		}

        if (::MessageBox(this->GetSafeHwnd(), CA2T(str.c_str()), CA2T(titleStr.c_str()), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		{
			std::string tmp = CT2A(nameU_);
            itsSmartMetDocumentInterface->RemoveMacroParam(tmp);
            CatLog::logMessage(string("Removed macroParam: ") + tmp, CatLog::Severity::Info, CatLog::Category::Macro);

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
			RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param remove", false, ""); // p‰ivitet‰‰n varmuuden vuoksi ruutuja, jos poitettiin karttan‰ytˆlt‰ macroparametri
		}
	}
}

// katsoo jos on joku listassa valittu macroparametri ja palauttaa sen
// theSelectedParametrissa.
// palauttaa true jos lˆytyi, muuten false.
NFmiMacroParam* CFmiSmartToolDlg::FindSelectedMacroParam(void)
{
	int index = itsMacroParamList.GetCurSel();
	if(index != LB_ERR)
	{
        CString nameU_;
		itsMacroParamList.GetText(index, nameU_);
		std::string tmp = CT2A(nameU_);
		NFmiMacroParamSystem& mpSystem = itsSmartMetDocumentInterface->MacroParamSystem();
		boost::shared_ptr<NFmiMacroParamFolder> currentFolder = mpSystem.GetCurrent();
		if(currentFolder && currentFolder->Find(tmp))
			return currentFolder->Current().get();
	}
	return 0;
}

void CFmiSmartToolDlg::OnBnClickedButtonMacroParamProperties()
{
    NFmiMacroParam *selectedMacroParam = FindSelectedMacroParam();
    if(selectedMacroParam)
    {
        if(selectedMacroParam->IsMacroParamDirectory())
            ::MessageBox(this->GetSafeHwnd(), CA2T(::GetDictionaryString("A directory was selected and you can't modify it's draw properties").c_str()), CA2T(::GetDictionaryString("No selection").c_str()), MB_ICONINFORMATION | MB_OK);
        else
        {
            CFmiModifyDrawParamDlg dlg(itsSmartMetDocumentInterface, selectedMacroParam->DrawParam(), itsSmartMetDocumentInterface->InfoOrganizer()->GetDrawParamPath(), true, false, itsSelectedMapViewDescTopIndex, this); // smarttool-dialogista ei voi toistaiseksi lis‰t‰ kuin p‰‰karttan‰ytˆlle macroParameja
            if(dlg.DoModal() == IDOK)
            {
                RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param draw options changed", false, selectedMacroParam->DrawParam()->InitFileName());
            }
            else
            {
                if(dlg.RefreshPressed())
                    RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param draw options changed back", false, selectedMacroParam->DrawParam()->InitFileName()); // jos painettu refres-nappia ja sitten cancelia, pit‰‰ p‰ivitt‰‰ ruutu
            }
        }
    }
    else
        ::MessageBox(this->GetSafeHwnd(), CA2T(::GetDictionaryString("No macroParam was selected").c_str()), CA2T(::GetDictionaryString("No selection").c_str()), MB_ICONINFORMATION | MB_OK);
}

void CFmiSmartToolDlg::AddSelectedMacroParamToRow(int theRow)
{
	UpdateData(TRUE);
	int index = itsMacroParamList.GetCurSel();
	if(index != LB_ERR)
	{
        CString nameU_;
		itsMacroParamList.GetText(index, nameU_);
		std::string tmp = CT2A(nameU_);
		if(fCrossSectionMode)
            itsSmartMetDocumentInterface->AddMacroParamToCrossSectionView(theRow, tmp);
		else
            itsSmartMetDocumentInterface->AddMacroParamToView(itsSelectedMapViewDescTopIndex, theRow, tmp);
		RefreshApplicationViewsAndDialogs("SmartToolDlg: macro-param added to some view", false, "");
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
    auto macroParamFolder = theMacroParamSystem.GetCurrent();
    auto macroParam = theMacroParamSystem.CurrentMacroParam();
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
	bool fMakeEmpty = false;
	int index = itsMacroParamList.GetCurSel();
	if(index != LB_ERR)
	{
        CString nameU_;
		itsMacroParamList.GetText(index, nameU_);

		NFmiMacroParamSystem& mpSystem = itsSmartMetDocumentInterface->MacroParamSystem();
		std::string tmp = CT2A(nameU_);
		if(mpSystem.FindMacro(tmp))
		{
			itsMacroParamNameU_ = nameU_;
            LoadSmarttoolFormula(GetMacroParamFilePath(mpSystem));
			itsMacroNameU_ = _TEXT(""); // Tyhjennet‰‰n varmuuden vuoksi smartToolin nimi, ett‰ kahden save-napin takia ei tule talletettua vahingossa v‰‰r‰ll‰ napilla
		}
		else
			fMakeEmpty = true;
	}
	else
		fMakeEmpty = true;
	if(fMakeEmpty)
	{
		itsMacroParamNameU_ = _TEXT("");
	}
	itsSmartToolInfo->CurrentScript(GetSmarttoolFormulaText()); // p‰ivitet‰‰n myˆs currentiksi macro-tekstiksi

	UpdateData(FALSE);
}

void CFmiSmartToolDlg::OnLbnDblclkListParamMacros()
{ // t‰ss‰ on tarkoitus menn‰ alihakemistoihin, jos klikattu oli hakemisto
	// ja ei tehd‰ mit‰‰n jos oli tavallinen macroParam

	int index = itsMacroParamList.GetCurSel();
	if(index != LB_ERR)
	{
        CString nameU_;
		itsMacroParamList.GetText(index, nameU_);
		if(nameU_[0] == '<')
		{ // Jos hakemiston nimi, siirryt‰‰n sinne
			std::string tmpName = CT2A(nameU_);
            itsSmartMetDocumentInterface->MacroParamSystem().CurrentPath(tmpName);
			UpdateMacroParamDisplayList(false);
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
        ResetSearchResource();
        UpdateLoadDirectoryString();
        bool status = itsSmartToolInfo->LoadScript(dlg.SelectedScriptName());
		if(status)
		{
            LoadSmarttoolFormula(GetSmarttoolFilePath());
			UpdateMacroName();
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
            itsSmartMetDocumentInterface->RemoveAllParamsFromCrossSectionViewRow(theRowIndex);
		else
            itsSmartMetDocumentInterface->RemoveAllParamsFromMapViewRow(itsSelectedMapViewDescTopIndex, theRowIndex);
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
    ResetSearchResource();
    boost::shared_ptr<NFmiMacroParamFolder> currentFolder = itsSmartMetDocumentInterface->MacroParamSystem().GetCurrent();
	if(currentFolder)
	{
		currentFolder->RefreshMacroParams();
		UpdateMacroParamDisplayList(true);
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
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_SAVE, "IDC_BUTTON_SMART_TOOL_SAVE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_LOAD, "IDC_BUTTON_SMART_TOOL_LOAD");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_REMOVE, "IDC_BUTTON_SMART_TOOL_REMOVE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_SAVE_DB_CHECKER, "IDC_BUTTON_SMART_TOOL_SAVE_DB_CHECKER");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOL_LOAD_DB_CHECKER, "IDC_BUTTON_SMART_TOOL_LOAD_DB_CHECKER");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_MAKE_DB_CHECK_AT_SEND, "IDC_CHECK_MAKE_DB_CHECK_AT_SEND");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PARAM_HOLDER, "IDC_STATIC_MACRO_PARAM_HOLDER");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_SAVE, "IDC_BUTTON_MACRO_PARAM_SAVE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_REMOVE, "IDC_BUTTON_MACRO_PARAM_REMOVE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_PROPERTIES, "IDC_BUTTON_MACRO_PARAM_PROPERTIES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_REFRESH_LIST, "IDC_BUTTON_MACRO_PARAM_REFRESH_LIST");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_LATEST_ERROR_TEXT, "IDC_BUTTON_MACRO_PARAM_LATEST_ERROR_TEXT");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PARAM_NAME_STR, "IDC_STATIC_MACRO_PARAM_NAME_STR");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PARAM_DATA_GRID_SIZE_STR, "IDC_STATIC_MACRO_PARAM_DATA_GRID_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_DATA_GRID_SIZE_USE, "NormalWordCapitalUse");
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
    lpMMI->ptMinTrackSize.x = 493;
    lpMMI->ptMinTrackSize.y = 539;
}

void CFmiSmartToolDlg::UpdateLoadDirectoryString()
{
    std::string usedLoadDirectory = "\\" + itsSmartToolInfo->GetRelativeLoadPath();
    itsUsedMacroPathU_ = CA2T(usedLoadDirectory.c_str());
    UpdateData(FALSE);
}

LRESULT CFmiSmartToolDlg::OnGUSIconEditLeftIconClicked(WPARAM wParam, LPARAM lParam)
{
    CRect rcEdit;

    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL == nCtrlId)
    {
        itsSpeedSearchMacroControl.GetWindowRect(&rcEdit);

        CMenu* pMenu = itsSpeedSearchMacroControlMenu.GetSubMenu(0);
        pMenu->CheckMenuItem(ID_SEARCHOPTION_CASESENSITIVE, fSearchOptionCaseSensitive ? MF_CHECKED : MF_UNCHECKED);
        pMenu->CheckMenuItem(ID_SEARCHOPTION_MATCHANYWHERE, fSearchOptionMatchAnywhere ? MF_CHECKED : MF_UNCHECKED);
        pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rcEdit.left, rcEdit.bottom, this);
    }

    return 1;
}

LRESULT CFmiSmartToolDlg::OnGUSIconEditRightIconClicked(WPARAM wParam, LPARAM lParam)
{
    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL == nCtrlId)
    {
        if(itsSpeedSearchMacroControl.GetEditTextLength() > 0)
            itsSpeedSearchMacroControl.ClearEditText();
    }

    return 1;
}

LRESULT CFmiSmartToolDlg::OnGUSIconEditSearchListClicked(WPARAM wParam, LPARAM lParam)
{
    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL == nCtrlId)
    {
        std::string selectedMacroName = CT2A(itsSpeedSearchMacroControl.GetSelectedString());
        DoSmartToolLoad(selectedMacroName, true);
    }

    return 1;
}

void CFmiSmartToolDlg::OnSearchOptionCaseSesensitive()
{
    fSearchOptionCaseSensitive = !fSearchOptionCaseSensitive;
    itsSpeedSearchMacroControl.SetSearchCaseSense(fSearchOptionCaseSensitive);
}

void CFmiSmartToolDlg::OnSearchOptionMatchAnywhere()
{
    fSearchOptionMatchAnywhere = !fSearchOptionMatchAnywhere;
    itsSpeedSearchMacroControl.SetSearchAnywhere(fSearchOptionMatchAnywhere);
}


void CFmiSmartToolDlg::OnEnChangeEditSpeedSearchViewMacro()
{
    if(itsSpeedSearchMacroControl.GetResultsCount() > 0)
    {
        itsSpeedSearchMacroControl.SetTextColor(RGB(0, 0, 0));
    }
    else
    {
        itsSpeedSearchMacroControl.SetTextColor(RGB(255, 0, 0));
    }
}

void CFmiSmartToolDlg::ResetSearchResource()
{
    // Add data source from ViewMacroSystem
    itsSpeedSearchMacroControl.RemoveAll();
    const std::vector<std::string> smarttoolNames = itsSmartMetDocumentInterface->SmartToolFileNames(true);

    for(const auto &name : smarttoolNames)
        itsSpeedSearchMacroControl.AddSearchString(CA2T(name.c_str()));
}

void CFmiSmartToolDlg::InitSpeedSearchControl()
{
    // Init the first edit.
    // We just use default parameters.
    // You can find the skin rect from "GUSIconEditSkin.bmp" and "GUSIconEditSkin.offset.txt".
    itsSpeedSearchMacroControl.InitializeResource(
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

    itsSpeedSearchMacroControl.SetEmptyTips(CA2T((::GetDictionaryString("Search smarttool macros...").c_str())));
    itsSpeedSearchMacroControl.SetSearchCaseSense(fSearchOptionCaseSensitive);
    itsSpeedSearchMacroControl.SetSearchAnywhere(fSearchOptionMatchAnywhere);
    itsSpeedSearchMacroControlMenu.LoadMenu(IDR_MENU_GUS_ICON_EDIT);

    // CGUSIconEdit -kontrollin muita mahdollisia optioita kommenteissa
    //itsSpeedSearchViewMacroControl.SetMaxDisplayItem(20);
    //itsSpeedSearchViewMacroControl.SetNormalItemsBKColor(RGB(201, 241, 238));
    //itsSpeedSearchViewMacroControl.SetEvenItemsBKColor(RGB(201, 187, 234));
    //itsSpeedSearchViewMacroControl.SetNonSelItemsKeywordsBKColor(RGB(248, 134, 151));
    //itsSpeedSearchViewMacroControl.SetNormalItemsTextColor(RGB(104, 36, 94));
    //itsSpeedSearchViewMacroControl.SetSelItemBKColor(RGB(221, 215, 89), TRUE);
    //itsSpeedSearchViewMacroControl.SetSelItemTextColor(RGB(203, 7, 75));
    //itsSpeedSearchViewMacroControl.SetRememberWindowNewSize(TRUE);
    //itsSpeedSearchViewMacroControl.SetSortDisplayedSearchResults(TRUE, TRUE);
}

// Tehd‰‰n ainakin 15 minuutin v‰lein pikahaku listan p‰ivityst‰, jos tulee uusia smarttool-macroja synkronoinnin kautta
// HUOM! Tehd‰‰n t‰m‰ p‰ivitys yritys vain tietyist‰ toiminnoista, esim. smarttool-macron lataus ja pika-etsint‰ valinnan yhteydess‰.
// En halua ett‰ t‰t‰ kutsutaan kesken mahdollista pikahaun tekoa jolloin seurauksena aletaan resetoimaan jo tehty‰ hakua.
void CFmiSmartToolDlg::DoTimedResetSearchResource()
{
    static NFmiMilliSecondTimer timer;

    if(timer.CurrentTimeDiffInMSeconds() > 15 * 60 * 1000)
    {
        ResetSearchResource();
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
