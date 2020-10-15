#ifdef _MSC_VER
#pragma warning(disable : 4996) // poistaa varoituksen "warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead"
#endif

#include "stdafx.h"
#include "FmiViewMacroDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiViewSettingMacro.h"
#include "NFmiStringTools.h"
#include "NFmiFileSystem.h"
#include "FmiWin32TemplateHelpers.h"

#include <algorithm>
#include "NFmiMetEditorOptionsData.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiApplicationWinRegistry.h"
#include "SmartMetMfcUtils_resource.h"
#include "persist2.h"
#include "NFmiBetaProductHelperFunctions.h"

#include "boost/filesystem.hpp"

// ***************************************
// ** NFmiViewMacroGridCtrl grid-control *
// ***************************************

IMPLEMENT_DYNCREATE(NFmiViewMacroGridCtrl, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiViewMacroGridCtrl, CGridCtrl)
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

void NFmiViewMacroGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    CGridCtrl::OnLButtonUp(nFlags, point);

    if(itsLeftClickUpCallback)
        itsLeftClickUpCallback(GetCellFromPt(point));
}

void NFmiViewMacroGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CGridCtrl::OnLButtonDblClk(nFlags, point);

    if(itsLeftDoubleClickUpCallback)
        itsLeftDoubleClickUpCallback(GetCellFromPt(point));
}


// CFmiViewMacroDlg dialog

const NFmiViewPosRegistryInfo CFmiViewMacroDlg::s_ViewPosRegistryInfo(CRect(500, 100, 1000, 600), "\\ViewMacroDlg");

IMPLEMENT_DYNAMIC(CFmiViewMacroDlg, CDialog)
CFmiViewMacroDlg::CFmiViewMacroDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
:CDialog(CFmiViewMacroDlg::IDD, pParent)
, itsGridCtrl()
, itsHeaders()
, fGridControlInitialized(false)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsMacroDescriptionU_(_T(""))
,fDisableWindowManipulations(FALSE)
,fSearchOptionCaseSensitive(FALSE)
,fSearchOptionMatchAnywhere(TRUE)
,itsCurrentPathStringU_(_T("\\"))
,mRowColumnWidth()
,mNameColumnWidth()
,mModifiedColumnWidth()
,mDescriptionColumnWidth()
{
}

CFmiViewMacroDlg::~CFmiViewMacroDlg()
{
//	WriteDialogRectToRegistery(); // t‰m‰ ei josatin syyst‰ toimi t‰‰ll‰, koska hwnd on jo 0, kun tanne p‰‰st‰‰n
    StoreWinRegistrySettings();
}

void CFmiViewMacroDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_GridControl(pDX, IDC_CUSTOM_VIEW_MACRO_GRID_CONTROL, itsGridCtrl);
    DDX_Text(pDX, IDC_EDIT_MACRO_DESCRIPTION, itsMacroDescriptionU_);
    DDX_Check(pDX, IDC_CHECK_DISABLE_WINDOW_MANIPULATION, fDisableWindowManipulations);
    DDX_Control(pDX, IDC_EDIT_SPEED_SEARCH_VIEW_MACRO, itsSpeedSearchViewMacroControl);
    DDX_Text(pDX, IDC_STATIC_CURRENT_PATH_TEXT, itsCurrentPathStringU_);
}


BEGIN_MESSAGE_MAP(CFmiViewMacroDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_MACRO_SAVE_AS, &CFmiViewMacroDlg::OnBnClickedButtonViewMacroSaveAs)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_REFRESH_LIST, OnBnClickedButtonRefreshList)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_VIEW_MACRO_UNDO, &CFmiViewMacroDlg::OnBnClickedButtonViewMacroUndo)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_MACRO_REDO, &CFmiViewMacroDlg::OnBnClickedButtonViewMacroRedo)
	ON_BN_CLICKED(IDC_CHECK_DISABLE_WINDOW_MANIPULATION, &CFmiViewMacroDlg::OnBnClickedCheckDisableWindowManipulation)
    ON_WM_GETMINMAXINFO()
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_LEFT_ICON_CLICKED, &CFmiViewMacroDlg::OnGUSIconEditLeftIconClicked)
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_RIGHT_ICON_CLICKED, &CFmiViewMacroDlg::OnGUSIconEditRightIconClicked)
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_CLICK_SEARCH_LIST, &CFmiViewMacroDlg::OnGUSIconEditSearchListClicked)
    ON_COMMAND(ID_SEARCHOPTION_CASESENSITIVE, &CFmiViewMacroDlg::OnSearchOptionCaseSesensitive)
    ON_COMMAND(ID_SEARCHOPTION_MATCHANYWHERE, &CFmiViewMacroDlg::OnSearchOptionMatchAnywhere)
    ON_EN_CHANGE(IDC_EDIT_SPEED_SEARCH_VIEW_MACRO, &CFmiViewMacroDlg::OnEnChangeEditSpeedSearchViewMacro)
    ON_WM_DESTROY()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_VIEW_MACRO_SAVE, &CFmiViewMacroDlg::OnBnClickedButtonViewMacroSave)
END_MESSAGE_MAP()

static void SetHeaders(CGridCtrl &theGridCtrl, const std::vector<ViewMacroHeaderParInfo> &theHeaders)
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
    // 1. on otsikko rivi on parametrien nimi‰ varten
    for(int i = 0; i<columnCount; i++)
    {
        theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
        theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
        theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
    }
}

static int FindWantedHeaderColumn(std::vector<ViewMacroHeaderParInfo> &theHeaders, ViewMacroHeaderParInfo::ColumnFunction theColumn)
{
    for(int i = 0; i< static_cast<int>(theHeaders.size()); i++)
        if(theHeaders[i].itsColumnFunction == theColumn)
            return i;
    return -1;
}

// rich_edit ja edit kontrollien kanssa on ihme ongelmia
// rivinvaihtojen kanssa. T‰ss‰ lis‰t‰‰n yksi puuttuva
// cr (carriage return) joka pit‰‰ lis‰t‰ aina teksteihin
// joka rivin vaihdon yhteyteen. t‰m‰ tekee muutoksen
// <nl> -> <cr><nl>
static std::string ConvertDescriptionStringToWantedFormat(const std::string &theStr)
{
    std::string returnStr;
    std::string::size_type ssize = theStr.size();
    std::string::size_type counter = 0;
    for(; counter < ssize; counter++)
    {
        if(theStr[counter] == '\n' && (counter == 0 || theStr[counter - 1] != '\r'))
            returnStr.push_back('\r');
        returnStr.push_back(theStr[counter]);
    }
    return returnStr;
}

// Onko hiiren klikkaus osunut taulukon sis‰‰n JA se ei ole otsikko rivill‰.
bool CFmiViewMacroDlg::IsClickedCellOk(const CCellID &theSelectedCell)
{
    if(theSelectedCell.IsValid() && theSelectedCell.row >= itsGridCtrl.GetFixedRowCount())
        return true;
    else
        return false;
}

void CFmiViewMacroDlg::LeftClickedGridCell(const CCellID &theSelectedCell)
{
    if(IsClickedCellOk(theSelectedCell))
    {
        itsSelectedMacroName = GetSelectedMacroName(); // T‰t‰ ei voi pyyt‰‰ suoraan theSelectedCell:in row numerolla, koska pit‰‰ ottaa huomioon otsikko rivi
        if(!itsSelectedMacroName.empty())
        {
            try
            {
                std::vector<NFmiLightWeightViewSettingMacro> &macroList = itsSmartMetDocumentInterface->ViewMacroDescriptionList();
                auto foundIter = std::find_if(macroList.begin(), macroList.end(), [&](const NFmiLightWeightViewSettingMacro &macro){return itsSelectedMacroName == macro.itsName; });
                if(foundIter != macroList.end())
                {
                    std::string descriptionStr(::ConvertDescriptionStringToWantedFormat(foundIter->itsDescription));
                    itsMacroDescriptionU_ = CA2T(descriptionStr.c_str());
                }
            }
            catch(std::exception &e)
            {
                std::string errStr("Unable to change view-macro: ");
                errStr += itsSelectedMacroName;
                errStr += "\nReason: ";
                errStr += e.what();
                // pit‰isikˆ raportoida message boxin avulla?
                itsSmartMetDocumentInterface->LogAndWarnUser(errStr, "ViewMacro loading problem", CatLog::Severity::Error, CatLog::Category::Operational, false);
            }
            UpdateData(FALSE);
        }
    }
}

void CFmiViewMacroDlg::DoubleLeftClickedGridCell(const CCellID &theSelectedCell)
{
    if(IsClickedCellOk(theSelectedCell))
    {
        int index = GetSelectedViewMacroRow(); // T‰t‰ ei voi pyyt‰‰ suoraan theSelectedCell:in row numerolla, koska pit‰‰ ottaa huomioon otsikko rivi
        LoadMacro(index);
    }
}

static std::string MakeWinRegistryKey()
{
    return NFmiApplicationWinRegistry::MakeBaseRegistryPath() + NFmiApplicationWinRegistry::MakeGeneralSectionName();
}

void CFmiViewMacroDlg::InitHeaders(void)
{
    itsHeaders.clear();

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;
    std::string registryKey = ::MakeWinRegistryKey();
    std::string sectionName = "\\ViewMacroDialog";
    mRowColumnWidth = ::CreateRegValue<CachedRegInt>(registryKey, sectionName, "\\RowColumnWidth", usedKey, 32);
    mNameColumnWidth = ::CreateRegValue<CachedRegInt>(registryKey, sectionName, "\\NameColumnWidth", usedKey, 160);
    mModifiedColumnWidth = ::CreateRegValue<CachedRegInt>(registryKey, sectionName, "\\ModifiedColumnWidth", usedKey, 120);
    mDescriptionColumnWidth = ::CreateRegValue<CachedRegInt>(registryKey, sectionName, "\\DescriptionColumnWidth", usedKey, 320);


    itsHeaders.push_back(ViewMacroHeaderParInfo(::GetDictionaryString("Row"), ViewMacroHeaderParInfo::kRowNumber, *mRowColumnWidth));
    itsHeaders.push_back(ViewMacroHeaderParInfo(::GetDictionaryString("Name"), ViewMacroHeaderParInfo::kMacroName, *mNameColumnWidth));
    itsHeaders.push_back(ViewMacroHeaderParInfo(::GetDictionaryString("Modified"), ViewMacroHeaderParInfo::kModificationDate, *mModifiedColumnWidth));
    itsHeaders.push_back(ViewMacroHeaderParInfo(::GetDictionaryString("Description"), ViewMacroHeaderParInfo::kDescription, *mDescriptionColumnWidth));
}

void CFmiViewMacroDlg::StoreWinRegistrySettings()
{
    *mRowColumnWidth = itsGridCtrl.GetColumnWidth(ViewMacroHeaderParInfo::kRowNumber);
    *mNameColumnWidth = itsGridCtrl.GetColumnWidth(ViewMacroHeaderParInfo::kMacroName);
    *mModifiedColumnWidth = itsGridCtrl.GetColumnWidth(ViewMacroHeaderParInfo::kModificationDate);
    *mDescriptionColumnWidth = itsGridCtrl.GetColumnWidth(ViewMacroHeaderParInfo::kDescription);
}

// CFmiViewMacroDlg message handlers

void CFmiViewMacroDlg::OnBnClickedOk()
{
	OnOK();
}

int CFmiViewMacroDlg::GetSelectedViewMacroRow()
{
    CCellRange cellRange = itsGridCtrl.GetSelectedCellRange();
    if(cellRange.IsValid())
    {
        if(cellRange.GetMinRow() == cellRange.GetMaxRow()) // Tarkistus ett‰ ei ole valittuna jotain pystysaraketta
            return cellRange.GetMinRow() - itsGridCtrl.GetFixedRowCount();
    }

    return -1;
}

void CFmiViewMacroDlg::OnBnClickedButtonLoad()
{
    int index = GetSelectedViewMacroRow();
	LoadMacro(index);
}

bool CFmiViewMacroDlg::IsNewMacro(const std::string &theName, int &theIndex)
{
    std::vector<NFmiLightWeightViewSettingMacro> &macroList = itsSmartMetDocumentInterface->ViewMacroDescriptionList();

    int ssize = static_cast<int>(macroList.size());
	for(int i=0 ; i<ssize; i++)
	{
		if(theName == macroList[i].itsName)
		{
			theIndex = i;
			return false;
		}
	}
	return true;
}

static bool IsCr(char value)
{
   return value == '\r';
}

const std::string g_ViewMacroFileExtension = "vmr";
const std::string g_ViewMacroFileTotalExtension = "." + g_ViewMacroFileExtension;
const std::string g_ViewMacroFileFilter = "View-macro Files (*." + g_ViewMacroFileExtension + ")|*." + g_ViewMacroFileExtension + "|All Files (*.*)|*.*||";
const std::string g_ViewMacroDefaultFileName = "viewmacro1";

static std::string GetViewMacroInitialPath(SmartMetDocumentInterface* smartMetDocumentInterface)
{
    return smartMetDocumentInterface->RootViewMacroPath() + smartMetDocumentInterface->GetRelativeViewMacroPath();
}

static bool IsSelectedNameActualViewMacro(const std::string& selectedMacroName)
{
    if(selectedMacroName.empty())
        return false;

    if(selectedMacroName[0] == '<')
        return false;

    return true;
}

static std::string GetInitialViewMacroFileName(const std::string & selectedMacroName)
{
    std::string initialFilename = selectedMacroName;
    if(!IsSelectedNameActualViewMacro(initialFilename))
    {
        initialFilename = g_ViewMacroDefaultFileName;
    }
    initialFilename += "." + g_ViewMacroFileExtension;
    return initialFilename;
}

bool CFmiViewMacroDlg::IsRealViewMacroFileSelected()
{
    int rowIndex = GetSelectedViewMacroRow(); // T‰t‰ ei voi pyyt‰‰ suoraan theSelectedCell:in row numerolla, koska pit‰‰ ottaa huomioon otsikko rivi
    if(rowIndex == LB_ERR)
        return false;

    if(::IsSelectedNameActualViewMacro(itsSelectedMacroName))
        return true;
    else
        return false;
}

static std::string MakeUsedViewMacroDescription(const CString& descriptionU_)
{
    std::string description = CT2A(descriptionU_);
    description.erase(std::remove(description.begin(), description.end(), '\r'), description.end());
    return description;
}

void CFmiViewMacroDlg::OnBnClickedButtonViewMacroSaveAs()
{
	UpdateData(TRUE);

    std::string initialPath = ::GetViewMacroInitialPath(itsSmartMetDocumentInterface);
    std::string initialFilename = ::GetInitialViewMacroFileName(itsSelectedMacroName);

    std::string filePath;
    if(BetaProduct::GetFilePathFromUserTotal(g_ViewMacroFileFilter, initialPath, filePath, false, initialFilename, g_ViewMacroFileExtension, itsSmartMetDocumentInterface->RootViewMacroPath(), this))
    {
        std::string description = ::MakeUsedViewMacroDescription(itsMacroDescriptionU_);
        itsSmartMetDocumentInterface->StoreViewMacro(filePath, description);
		InitMacroListFromDoc();
		SelectMacro(filePath);
        ResetSearchResource();
        UpdateRelativeMacroPath();
    }
}

void CFmiViewMacroDlg::OnBnClickedButtonViewMacroSave()
{
    UpdateData(TRUE);

    if(IsRealViewMacroFileSelected())
    {
        std::string totalViewMacroPath = ::GetViewMacroInitialPath(itsSmartMetDocumentInterface);
        totalViewMacroPath += ::GetInitialViewMacroFileName(itsSelectedMacroName);
        std::string description = ::MakeUsedViewMacroDescription(itsMacroDescriptionU_);

        std::string overWriteConformation = ::GetDictionaryString("Over write view-macro: '");
        overWriteConformation += itsSelectedMacroName;
        overWriteConformation += ::GetDictionaryString("' ?");
        if(::MessageBox(this->GetSafeHwnd(), CA2T(overWriteConformation.c_str()), CA2T(::GetDictionaryString("Save over macro").c_str()), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
        {
            itsSmartMetDocumentInterface->StoreViewMacro(totalViewMacroPath, description);
            // Let's initialize visible list so that saved viewMacro's new time-stamp will be updated to the grid-view
            InitMacroListFromDoc();
        }
    }
    else
    {
        ::MessageBox(this->GetSafeHwnd(), CA2T(::GetDictionaryString("Invalid view-macro selection for Save operation").c_str()), CA2T(::GetDictionaryString("Can't Save selected macro").c_str()), MB_ICONINFORMATION | MB_OK);
    }
}

void CFmiViewMacroDlg::SelectMacro(const std::string & theFilePath)
{
//    itsGridCtrl.SelectMacro(theName, ::FindWantedHeaderColumn(itsHeaders, ViewMacroHeaderParInfo::kMacroName));
    // Haetaan macro listasta nimen perusteell‰ macron iteraattori
    std::vector<NFmiLightWeightViewSettingMacro> &macroList = itsSmartMetDocumentInterface->ViewMacroDescriptionList();
    auto iter = std::find_if(macroList.begin(), macroList.end(), [&](const NFmiLightWeightViewSettingMacro &macro){return macro.itsInitFilePath == theFilePath; });
    if(iter != macroList.end())
    {
        // Lasketaan rivin indeksi ja lis‰t‰‰n siihen otsikkorivi
        int row = static_cast<int>(iter - macroList.begin() + itsGridCtrl.GetFixedRowCount());
        // Asetetaan kyseinen rivi valituksi, ja automaatio huolehtii lopusta
        itsGridCtrl.SetSelectedRange(row, 0, row, static_cast<int>(itsHeaders.size() - 1), TRUE);
        itsGridCtrl.EnsureVisible(row, 0);
    }
}

void CFmiViewMacroDlg::CreateNewDirectory(const std::string &thePath)
{
	if(itsSmartMetDocumentInterface->CreateNewViewMacroDirectory(thePath))
		InitMacroListFromDoc();
	else
        ::MessageBox(this->GetSafeHwnd(), CA2T(::GetDictionaryString("ViewMacroDlgDirectoryCreationFailed").c_str()), _TEXT("Virhe"), MB_OK | MB_ICONERROR);
}

std::string CFmiViewMacroDlg::GetSelectedMacroName()
{
    return GetMacroName(GetSelectedViewMacroRow());
}

std::string CFmiViewMacroDlg::GetMacroName(int index)
{
    std::vector<NFmiLightWeightViewSettingMacro> &macroList = itsSmartMetDocumentInterface->ViewMacroDescriptionList();
    if(index >= 0 && index < macroList.size())
    {
        return macroList[index].itsName;
    }
    return "";
}

void CFmiViewMacroDlg::OnBnClickedButtonRemove()
{
	UpdateData(TRUE);
    std::string name = GetSelectedMacroName();
    if(!name.empty())
	{
		std::string str;
		std::string titleStr;
		if(name[0] == '<')
		{
			str += ::GetDictionaryString("ViewMacroDlgMacroParamRemoveDir");
			str += ":\n";
			str += name;
			titleStr = ::GetDictionaryString("ViewMacroDlgMacroParamRemoveDir2");
		}
		else
		{
			str += ::GetDictionaryString("ViewMacroDlgMacroParamRemove");
			str += ":\n";
			str += name;
			titleStr = ::GetDictionaryString("ViewMacroDlgMacroParamRemove2");
		}
        if (::MessageBox(this->GetSafeHwnd(), CA2T(str.c_str()), CA2T(titleStr.c_str()), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		{
            if(itsSmartMetDocumentInterface->RemoveViewMacro(name))
            {
                InitMacroListFromDoc();
                itsGridCtrl.SetSelectedRange(-1, -1, -1, -1);
                itsMacroDescriptionU_ = _TEXT("");
                ResetSearchResource();
                UpdateData(FALSE);
            }
		}
	}
}

void CFmiViewMacroDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
}

void CFmiViewMacroDlg::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa
    {
        UpdateDialogButtons();
    }
}

BOOL CFmiViewMacroDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    DoResizerHooking(); // T‰t‰ pit‰‰ kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisterist‰
    CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
    std::string errorBaseStr("Error in CFmiViewMacroDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

    itsGridCtrl.SetLeftClickUpCallback(std::bind(&CFmiViewMacroDlg::LeftClickedGridCell, this, std::placeholders::_1));
    itsGridCtrl.SetLeftDoubleClickUpCallback(std::bind(&CFmiViewMacroDlg::DoubleLeftClickedGridCell, this, std::placeholders::_1));

    InitHeaders();
    SetHeaders(itsGridCtrl, itsHeaders);

	fDisableWindowManipulations = itsSmartMetDocumentInterface->MetEditorOptionsData().DisableWindowManipulations();

	InitDialogTexts();
    EnableColorCodedControls();

	InitMacroListFromDoc();
    InitSpeedSearchControl();
    ResetSearchResource();

	UpdateDialogButtons();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiViewMacroDlg::EnableColorCodedControls()
{
    CFmiWin32Helpers::EnableColorCodedControl(this, IDC_CHECK_DISABLE_WINDOW_MANIPULATION);
}

void CFmiViewMacroDlg::DoResizerHooking(void)
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_DISABLE_WINDOW_MANIPULATION, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDOK, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_LOAD, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_VIEW_MACRO_SAVE, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_VIEW_MACRO_SAVE_AS, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_REMOVE, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_REFRESH_LIST, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_VIEW_MACRO_UNDO, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_VIEW_MACRO_REDO, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_VIEW_MACRO_DESCRIPTION, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_MACRO_DESCRIPTION, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CUSTOM_VIEW_MACRO_GRID_CONTROL, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_SPEED_SEARCH_VIEW_MACRO, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_CURRENT_PATH_TEXT, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE); 
}

void CFmiViewMacroDlg::UpdateDialogButtons(void)
{
	CWnd *undoButton = GetDlgItem(IDC_BUTTON_VIEW_MACRO_UNDO);
	if(undoButton)
		undoButton->EnableWindow(itsSmartMetDocumentInterface->IsUndoableViewMacro());

	CWnd *redoButton = GetDlgItem(IDC_BUTTON_VIEW_MACRO_REDO);
	if(redoButton)
		redoButton->EnableWindow(itsSmartMetDocumentInterface->IsRedoableViewMacro());
}

static bool IsViewMacroCorrupted(const NFmiLightWeightViewSettingMacro &theMacro, const std::vector<std::string> &theCorruptedFiles)
{
    auto iter = std::find(theCorruptedFiles.begin(), theCorruptedFiles.end(), theMacro.itsInitFilePath);
    return iter != theCorruptedFiles.end();
}

void MakeCorruptionChecks(NFmiLightWeightViewSettingMacro &theMacro, const std::vector<std::string> &theCorruptedFiles)
{
    if(::IsViewMacroCorrupted(theMacro, theCorruptedFiles))
        theMacro.fViewMacroOk = false;
}

void CFmiViewMacroDlg::InitMacroListFromDoc(void)
{
    CCellRange selectedCellRange = itsGridCtrl.GetSelectedCellRange();
    std::vector<NFmiLightWeightViewSettingMacro> &macroList = itsSmartMetDocumentInterface->ViewMacroDescriptionList();
    itsGridCtrl.SetRowCount(static_cast<int>(macroList.size() + itsGridCtrl.GetFixedRowCount()));
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    const std::vector<std::string> &corruptedFiles = itsSmartMetDocumentInterface->CorruptedViewMacroFileList();

    for(size_t i = 0; i < macroList.size(); i++)
    {
        NFmiLightWeightViewSettingMacro &macro = macroList[i];
        ::MakeCorruptionChecks(macro, corruptedFiles);
        SetGridRow(currentRowCount++, macro);
    }
    itsGridCtrl.SetSelectedRange(selectedCellRange);
}

static std::string GetColumnText(int theRow, int theColumn, const NFmiLightWeightViewSettingMacro &theListItem, FmiLanguage theLanguage)
{
    switch(theColumn)
    {
    case ViewMacroHeaderParInfo::kRowNumber:
        return NFmiStringTools::Convert(theRow);
    case ViewMacroHeaderParInfo::kMacroName:
    {
        return theListItem.itsName;
    }
    case ViewMacroHeaderParInfo::kModificationDate:
    {
        return std::string(theListItem.itsFileModificationTime.ToStr("YYYY.MM.DD HH:mm:SS", theLanguage));
    }
    case ViewMacroHeaderParInfo::kDescription:
        if(theListItem.ViewMacroOk())
            return theListItem.itsDescription;
        else
            return "VIEWMACRO-FILE WAS CORRUPTED!!";
    default:
        return "";
    }
}

void CFmiViewMacroDlg::SetGridRow(int row, const NFmiLightWeightViewSettingMacro &theListItem)
{
    static const COLORREF gMacroBkColor = RGB(255, 255, 255);
    static const COLORREF gDirectoryBkColor = RGB(242, 245, 255);
    static const COLORREF gCorruptedBkColor = RGB(255, 242, 242);

    for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
    {
        itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theListItem, itsSmartMetDocumentInterface->Language()).c_str()));
        if(column >= itsGridCtrl.GetFixedColumnCount())
        {
            itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); // Laita read-only -bitti p‰‰lle kaikille ruuduille
            if(theListItem.ViewMacroOk())
            {
                if(theListItem.fIsViewMacroDirectory)
                    itsGridCtrl.SetItemBkColour(row, column, gDirectoryBkColor);
                else
                    itsGridCtrl.SetItemBkColour(row, column, gMacroBkColor);
            }
            else
                itsGridCtrl.SetItemBkColour(row, column, gCorruptedBkColor);
        }
    }
}


void CFmiViewMacroDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiViewMacroDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiViewMacroDlg::LoadMacro(int theIndex)
{
	if(theIndex != LB_ERR)
	{
        std::string name = GetMacroName(theIndex);

        if(!name.empty())
        {
            if(name[0] == '<')
            {
                ChangeDirectory(name, false);
                return;
            }

            if(!itsSmartMetDocumentInterface->LoadViewMacro(name))
            {
                // Jos macron lataus ep‰onnistui, p‰ivitet‰‰n lista uudestaan, jolloin korruptoitunut macro merkit‰‰n erikseen
                InitMacroListFromDoc();
            }
        }
	}
	UpdateDialogButtons();
    DoTimedResetSearchResource();
}

// Palauttaa viimeisen osan polusta
// c:\data\src\inc\ -> src
// \dir1 -> dir1
static std::string GetLastPartOfDirectory(const std::string &thePath)
{
    std::string path = thePath;
    // Muutetaan kaikki kenomerkit samanlaisiksi, jotta loppu funktio yksinkertaistuu.
    NFmiStringTools::ReplaceChars(path, '\\', '/');
    // Poistetaan lopusta mahdollinen kenomerkki
    NFmiStringTools::TrimR(path, '/');
    // Etsit‰‰n viimeinen kenomerkki, jos sellainen lˆytyy.
    std::string::size_type pos = path.find_last_of('/');
    if(pos != std::string::npos)
        return std::string(path.begin() + pos + 1, path.end());
    else
        return path;
}

int CFmiViewMacroDlg::FindMacroNameRow(const std::string &theMacroName)
{
    CString macroNameU_ = CA2T(theMacroName.c_str());
    int nameColumn = 1;
    for(int i = itsGridCtrl.GetFixedRowCount(); i < itsGridCtrl.GetRowCount(); i++)
    {
        if(macroNameU_ ==  itsGridCtrl.GetCell(i, nameColumn)->GetText())
            return i;
    }
    return -1;
}


static const std::string g_ParentDirectoryStr = "<..>";

// Kun siirryt‰‰n johonkin alikansioon, halutaan ett‰ makrolistaus n‰kyy 1. rivist‰ alkaen.
// Kun siirryt‰‰n emokansioon <..>, halutaan ett‰ listassa on valittuna ja n‰kyy se alikansio, mist‰ tultiin ylˆsp‰in.
// Parametri theDirectoryName: Kansion nimi, mihin halutaan menn‰, mukana on 'kulmasulut' esim. <dir1>. jos arvo on <..>, menn‰‰n emokansioon.
// Parametri theOldPath: Mist‰ polusta siirryt‰‰n haluttuun uuteen kansioon. T‰st‰ otetaan tulokansio, jos siirryt‰‰n emokansioon.
void CFmiViewMacroDlg::EnsureWantedRowVisibilityAfterDirectoryChange(const std::string &theDirectoryName, const std::string &theOldPath)
{
    int wantedVisibleRow = 1; // Oletuksena 1. rivi laitetaan n‰kyviin, n‰in tehd‰‰n kun menn‰‰n johonkin alikansioon.
    if(theDirectoryName == g_ParentDirectoryStr)
    {
        std::string subDirectory = ::GetLastPartOfDirectory(theOldPath);
        std::string searchStr = "<" + subDirectory + ">"; // Etsint‰ stringiin pit‰‰ lis‰t‰ kulmasulut, koska alihakemistot ovat siin‰ muodossa grid-kontrollissa
        wantedVisibleRow = FindMacroNameRow(searchStr);
        if(wantedVisibleRow == -1)
            wantedVisibleRow = 1;
        itsGridCtrl.SetSelectedRange(wantedVisibleRow, 0, wantedVisibleRow, itsGridCtrl.GetColumnCount()-1); // Valitaan viel‰ se alikansio mist‰ tultiin merkiksi k‰ytt‰j‰lle
    }
    else
        itsGridCtrl.SetSelectedRange(-1, -1, -1, -1); // Kun menn‰‰n alikansioon, poistetaan mahdollinen valinta
    itsGridCtrl.EnsureVisible(wantedVisibleRow, 0);
}

void CFmiViewMacroDlg::UpdateRelativeMacroPath()
{
    std::string relativeViewMacroPath = "\\"; // polku alkaa aina \ -viivalla
    relativeViewMacroPath += itsSmartMetDocumentInterface->GetRelativeViewMacroPath();
    itsCurrentPathStringU_ = CA2T(relativeViewMacroPath.c_str());
    UpdateData(FALSE);
}

void CFmiViewMacroDlg::ChangeDirectory(const std::string &theDirectoryName, bool fDoSpeedSearchLoad)
{
    bool useRootPathAsBase = fDoSpeedSearchLoad;
    itsSmartMetDocumentInterface->ChangeCurrentViewMacroDirectory(theDirectoryName, useRootPathAsBase);
	InitMacroListFromDoc();
    DoTimedResetSearchResource();
    EnsureWantedRowVisibilityAfterDirectoryChange(theDirectoryName, std::string(CT2A(itsCurrentPathStringU_)));
    UpdateRelativeMacroPath();
    // I assume that there is some kind of optimization update bug in CGridCtrl -class
    // because it doesn't seem to update grid rows when you move from directory to
    // another where there is in both the same number of items. The texts are updated
    // in the code, but each row has to be selected separately to be really updated visually.
    itsGridCtrl.Invalidate(FALSE);
}

void CFmiViewMacroDlg::OnBnClickedButtonRefreshList()
{
    itsSmartMetDocumentInterface->RefreshViewMacroList();
    ResetSearchResource();
	InitMacroListFromDoc();
}

// T‰m‰ funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell‰.
// T‰m‰ on ik‰v‰ kyll‰ teht‰v‰ erikseen dialogin muokkaus tyˆkalusta, eli
// tekij‰n pit‰‰ lis‰t‰ erikseen t‰nne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiViewMacroDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("ViewMacroDlgTitle").c_str()));

	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK_VIEW_MACRO_DLG"); // oikeasti buttonin id on IDOK, mutta k‰ytet‰‰n teksti‰ taikasanalla IDOK_VIEW_MACRO_DLG
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD, "IDC_BUTTON_LOAD");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_VIEW_MACRO_SAVE, "Save");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_VIEW_MACRO_SAVE_AS, "Save as");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REMOVE, "IDC_BUTTON_REMOVE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REFRESH_LIST, "IDC_BUTTON_REFRESH_LIST");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VIEW_MACRO_DESCRIPTION, "IDC_STATIC_VIEW_MACRO_DESCRIPTION");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DISABLE_WINDOW_MANIPULATION, "No SmartMet view manipulations");
}

void CFmiViewMacroDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiViewMacroDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiViewMacroDlg::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

void CFmiViewMacroDlg::DoWhenClosing(void)
{
    StoreWinRegistrySettings();
    AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
}

void CFmiViewMacroDlg::OnBnClickedButtonViewMacroUndo()
{
    itsSmartMetDocumentInterface->UndoViewMacro();
	UpdateDialogButtons();
}

void CFmiViewMacroDlg::OnBnClickedButtonViewMacroRedo()
{
    itsSmartMetDocumentInterface->RedoViewMacro();
	UpdateDialogButtons();
}

void CFmiViewMacroDlg::OnBnClickedCheckDisableWindowManipulation()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->MetEditorOptionsData().DisableWindowManipulations(fDisableWindowManipulations == TRUE);
}


void CFmiViewMacroDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // set the minimum tracking width and height of the window
    lpMMI->ptMinTrackSize.x = 493;
    lpMMI->ptMinTrackSize.y = 469;
}

void CFmiViewMacroDlg::InitSpeedSearchControl()
{
    // Init the first edit.
    // We just use default parameters.
    // You can find the skin rect from "GUSIconEditSkin.bmp" and "GUSIconEditSkin.offset.txt".
    itsSpeedSearchViewMacroControl.InitializeResource(
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

    itsSpeedSearchViewMacroControl.SetEmptyTips(CA2T((::GetDictionaryString("Search view macros...").c_str())));
    itsSpeedSearchViewMacroControl.SetSearchCaseSense(fSearchOptionCaseSensitive);
    itsSpeedSearchViewMacroControl.SetSearchAnywhere(fSearchOptionMatchAnywhere);
    itsSpeedSearchViewMacroControlMenu.LoadMenu(IDR_MENU_GUS_ICON_EDIT);

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

LRESULT CFmiViewMacroDlg::OnGUSIconEditLeftIconClicked(WPARAM wParam, LPARAM lParam)
{
    CRect rcEdit;

    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_VIEW_MACRO == nCtrlId)
    {
        itsSpeedSearchViewMacroControl.GetWindowRect(&rcEdit);

        CMenu* pMenu = itsSpeedSearchViewMacroControlMenu.GetSubMenu(0);
        pMenu->CheckMenuItem(ID_SEARCHOPTION_CASESENSITIVE, fSearchOptionCaseSensitive ? MF_CHECKED : MF_UNCHECKED);
        pMenu->CheckMenuItem(ID_SEARCHOPTION_MATCHANYWHERE, fSearchOptionMatchAnywhere ? MF_CHECKED : MF_UNCHECKED);
        pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rcEdit.left, rcEdit.bottom, this);
    }

    return 1;
}

LRESULT CFmiViewMacroDlg::OnGUSIconEditRightIconClicked(WPARAM wParam, LPARAM lParam)
{
    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_VIEW_MACRO == nCtrlId)
    {
        if(itsSpeedSearchViewMacroControl.GetEditTextLength() > 0)
            itsSpeedSearchViewMacroControl.ClearEditText();
    }

    return 1;
}

// Tehd‰‰n ainakin 15 minuutin v‰lein pikahaku listan p‰ivityst‰, jos tulee uusia viewMacroja synkronoinnin kautta
// HUOM! Tehd‰‰n t‰m‰ p‰ivitys yritys vain tietyist‰ toiminnoista, esim. viewMacron lataus ja haku kent‰n tyhjennys.
// En halua ett‰ t‰t‰ kutsutaan kesken mahdollista pikahaun tekoa jolloin seurauksena aletaan resetoimaan jo tehty‰ hakua.
void CFmiViewMacroDlg::DoTimedResetSearchResource()
{
    static NFmiMilliSecondTimer timer;

    if(timer.CurrentTimeDiffInMSeconds() > 15 * 60 * 1000)
    {
        ResetSearchResource();
        timer.StartTimer(); // k‰ynnistet‰‰n uusi laskuri
    }
}

void CFmiViewMacroDlg::DoSpeedSearchMacroSelection(const std::string &theViewMacroName)
{
    if(!theViewMacroName.empty())
    {
        boost::filesystem::path viewMacroPath(theViewMacroName);
        // boost::filesystem::path - parent_path -metodin paluu arvo esim.: xxx\yyy\viewmacro.vmr  =>  xxx\yyy
        ChangeDirectory(viewMacroPath.parent_path().string(), true); // T‰m‰ tekee myˆs DoTimedResetSearchResource -kutsun
        // boost::filesystem::path - stem -metodin paluu arvo esim.: xxx\yyy\viewmacro.vmr  =>  viewmacro
        std::string usedName = viewMacroPath.stem().string();
        if(itsSmartMetDocumentInterface->LoadViewMacro(usedName))
        {
            SelectMacro(usedName);
            UpdateDialogButtons();
        }
    }
}

LRESULT CFmiViewMacroDlg::OnGUSIconEditSearchListClicked(WPARAM wParam, LPARAM lParam)
{
    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_VIEW_MACRO == nCtrlId)
    {
        std::string selectedMacroName = CT2A(itsSpeedSearchViewMacroControl.GetSelectedString());
        DoSpeedSearchMacroSelection(selectedMacroName);
    }

    return 1;
}

void CFmiViewMacroDlg::OnSearchOptionCaseSesensitive()
{
    fSearchOptionCaseSensitive = !fSearchOptionCaseSensitive;
    itsSpeedSearchViewMacroControl.SetSearchCaseSense(fSearchOptionCaseSensitive);
}

void CFmiViewMacroDlg::OnSearchOptionMatchAnywhere()
{
    fSearchOptionMatchAnywhere = !fSearchOptionMatchAnywhere;
    itsSpeedSearchViewMacroControl.SetSearchAnywhere(fSearchOptionMatchAnywhere);
}


void CFmiViewMacroDlg::OnEnChangeEditSpeedSearchViewMacro()
{
    if(itsSpeedSearchViewMacroControl.GetResultsCount() > 0)
    {
        itsSpeedSearchViewMacroControl.SetTextColor(RGB(0, 0, 0));
    }
    else
    {
        itsSpeedSearchViewMacroControl.SetTextColor(RGB(255, 0, 0));
    }
}

void CFmiViewMacroDlg::ResetSearchResource()
{
    // Add data source from ViewMacroSystem
    itsSpeedSearchViewMacroControl.RemoveAll();
    const std::vector<std::string> viewMacroNames = itsSmartMetDocumentInterface->ViewMacroFileNames(true);

    for(const auto &name : viewMacroNames)
        itsSpeedSearchViewMacroControl.AddSearchString(CA2T(name.c_str()));
}


void CFmiViewMacroDlg::OnDestroy()
{
    CDialog::OnDestroy();
}


HBRUSH CFmiViewMacroDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if(pWnd->GetDlgCtrlID() == IDC_CHECK_DISABLE_WINDOW_MANIPULATION)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, !fDisableWindowManipulations);

    return hbr;
}

