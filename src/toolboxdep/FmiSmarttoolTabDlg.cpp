// FmiSmarttoolTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiSmarttoolTabDlg.h"

#include "SmartMetDocumentInterface.h"
#include "NFmiSmartToolInfo.h"
#include "NFmiDictionaryFunction.h"
#include "SmartMetMfcUtils_resource.h"

#include <boost/filesystem.hpp>


// *********************************************
// ** NFmiSmarttoolsMacroGridCtrl grid-control *
// *********************************************

IMPLEMENT_DYNCREATE(NFmiSmarttoolsMacroGridCtrl, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiSmarttoolsMacroGridCtrl, CGridCtrl)
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

void NFmiSmarttoolsMacroGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    CGridCtrl::OnLButtonUp(nFlags, point);

    if(itsLeftClickUpCallback)
        itsLeftClickUpCallback(GetCellFromPt(point));
}

void NFmiSmarttoolsMacroGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CGridCtrl::OnLButtonDblClk(nFlags, point);

    if(itsLeftDoubleClickUpCallback)
        itsLeftDoubleClickUpCallback(GetCellFromPt(point));
}


void NFmiSmarttoolsMacroGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
    CGridCtrl::OnRButtonUp(nFlags, point);

    if(itsRightClickUpCallback)
        itsRightClickUpCallback(GetCellFromPt(point));
}

void NFmiSmarttoolsMacroGridCtrl::SetHeaders(const std::vector<SmarttoolsMacroHeaderParInfo> &theHeaders)
{
    static const COLORREF gFixedBkColor = RGB(239, 235, 222);

    int columnCount = static_cast<int>(theHeaders.size());
    SetColumnCount(columnCount);
    SetFixedRowCount(1);
    SetFixedColumnCount(1);
    SetGridLines(GVL_BOTH);
    SetListMode(TRUE);
    SetHeaderSort(FALSE);
    SetFixedBkColor(gFixedBkColor);
    SetSingleColSelection(TRUE);
    SetSingleRowSelection(TRUE);

    int currentRow = 0;
    // 1. on otsikko rivi on parametrien nimi‰ varten
    for(int i = 0; i<columnCount; i++)
    {
        SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
        SetItemState(currentRow, i, GetItemState(currentRow, i) | GVIS_READONLY);
        SetColumnWidth(i, theHeaders[i].itsColumnWidth);
    }
}


// CFmiSmarttoolTabDlg dialog

IMPLEMENT_DYNAMIC(CFmiSmarttoolTabDlg, CTabPageSSL)

CFmiSmarttoolTabDlg::CFmiSmarttoolTabDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
: CTabPageSSL(CFmiSmarttoolTabDlg::IDD, pParent)
    , itsSmartMetDocumentInterface(smartMetDocumentInterface)
    , itsSmartToolInfo(smartMetDocumentInterface ? smartMetDocumentInterface->SmartToolInfo() : 0)
    , itsSplitterHelper(this, IDC_BUTTON_SPLITTER_SMARTTOOL_HORIZONTAL, IDC_BUTTON_SPLITTER_SMARTTOOL_VERTICAL, 0.6, IDC_CHECK_HORIZONTAL_SPLIT_SCREEN, IDC_RICHEDIT_MACRO_TEXT, IDC_RICHEDIT_VIEW_ONLY_MACRO_TEXT, IDC_STATIC_SPLITTER_CONTROL)
    , fSearchOptionCaseSensitive(FALSE)
    , fSearchOptionMatchAnywhere(TRUE)
    , itsViewOnlyMacroText_DoubleClickBackupU_()
    , fIgnoreLeftClickAfterDoubleClick(false)
    , itsIgnoreLeftClickTimer()

    , itsEditedMacroPathU_(_T(""))
    , itsViewedMacroPathU_(_T(""))
{
    
}

CFmiSmarttoolTabDlg::~CFmiSmarttoolTabDlg()
{
}

void CFmiSmarttoolTabDlg::DoDataExchange(CDataExchange* pDX)
{
    CTabPageSSL::DoDataExchange(pDX);
    DDX_GridControl(pDX, IDC_CUSTOM_SMARTTOOL_GRID_CONTROL, itsGridCtrl);
    itsSplitterHelper.SetUpDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, itsSpeedSearchMacroControl);
    DDX_Control(pDX, IDC_RICHEDIT_MACRO_TEXT, itsMacroTextControl);
    DDX_Control(pDX, IDC_RICHEDIT_VIEW_ONLY_MACRO_TEXT, itsViewOnlyMacroTextControl);
    DDX_Text(pDX, IDC_STATIC_EDITED_SMARTTOOL_PATH, itsEditedMacroPathU_);
    DDX_Text(pDX, IDC_STATIC_EDITED_SMARTTOOL_VIEWED_MACRO_PATH, itsViewedMacroPathU_);
}


BEGIN_MESSAGE_MAP(CFmiSmarttoolTabDlg, CTabPageSSL)
    ON_WM_SIZE()
    ON_MESSAGE(UWM_SPLIT_MOVED, &CFmiSmarttoolTabDlg::OnSplitterControlMove)
    ON_WM_GETMINMAXINFO()
    ON_BN_CLICKED(IDC_CHECK_HORIZONTAL_SPLIT_SCREEN, &CFmiSmarttoolTabDlg::OnBnClickedCheckHorizontalSplitScreen)
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_LEFT_ICON_CLICKED, &CFmiSmarttoolTabDlg::OnGUSIconEditLeftIconClicked)
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_RIGHT_ICON_CLICKED, &CFmiSmarttoolTabDlg::OnGUSIconEditRightIconClicked)
    ON_MESSAGE(WM_USER_GUS_ICONEDIT_CLICK_SEARCH_LIST, &CFmiSmarttoolTabDlg::OnGUSIconEditSearchListClicked)
    ON_COMMAND(ID_SEARCHOPTION_CASESENSITIVE, &CFmiSmarttoolTabDlg::OnSearchOptionCaseSesensitive)
    ON_COMMAND(ID_SEARCHOPTION_MATCHANYWHERE, &CFmiSmarttoolTabDlg::OnSearchOptionMatchAnywhere)
    ON_EN_CHANGE(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, &CFmiSmarttoolTabDlg::OnEnChangeEditSpeedSearchViewMacro)
    ON_BN_CLICKED(IDC_BUTTON_SMARTTOOL_EDIT, &CFmiSmarttoolTabDlg::OnBnClickedButtonSmarttoolEdit)
END_MESSAGE_MAP()


// CFmiSmarttoolTabDlg message handlers

BOOL CFmiSmarttoolTabDlg::OnInitDialog()
{
    CTabPageSSL::OnInitDialog();
    itsSplitterHelper.PreResizerHookingSetup(); // T‰t‰ pit‰‰ kutsua ennen DoResizerHooking -metodia.
    DoResizerHooking(); // T‰t‰ pit‰‰ kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisterist‰
    itsSplitterHelper.PostResizerHookingSetup();

    itsGridCtrl.SetLeftClickUpCallback(std::bind(&CFmiSmarttoolTabDlg::LeftClickedGridCell, this, std::placeholders::_1));
    itsGridCtrl.SetLeftDoubleClickUpCallback(std::bind(&CFmiSmarttoolTabDlg::DoubleLeftClickedGridCell, this, std::placeholders::_1));
    itsGridCtrl.SetRightClickUpCallback(std::bind(&CFmiSmarttoolTabDlg::RightClickedGridCell, this, std::placeholders::_1));
    InitHeaders();
    itsGridCtrl.SetHeaders(itsHeaders);

    InitSpeedSearchControl();
    ResetSearchResource();

    InitMacroListFromDoc();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

static std::string MakeWinRegistryKey()
{
    return NFmiApplicationWinRegistry::MakeBaseRegistryPath() + NFmiApplicationWinRegistry::MakeGeneralSectionName();
}

void CFmiSmarttoolTabDlg::InitHeaders(void)
{
    itsHeaders.clear();

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;
    std::string registryKey = ::MakeWinRegistryKey();
    std::string sectionName = "\\SmarttoolTabDlg";
    mRowColumnWidth = ::CreateRegValue<CachedRegInt>(registryKey, sectionName, "\\RowColumnWidth", usedKey, 32);
    mNameColumnWidth = ::CreateRegValue<CachedRegInt>(registryKey, sectionName, "\\NameColumnWidth", usedKey, 240);
    mModifiedColumnWidth = ::CreateRegValue<CachedRegInt>(registryKey, sectionName, "\\ModifiedColumnWidth", usedKey, 120);


    itsHeaders.push_back(SmarttoolsMacroHeaderParInfo(::GetDictionaryString("Row"), SmarttoolsMacroHeaderParInfo::kRowNumber, *mRowColumnWidth));
    itsHeaders.push_back(SmarttoolsMacroHeaderParInfo(::GetDictionaryString("Name"), SmarttoolsMacroHeaderParInfo::kMacroName, *mNameColumnWidth));
    itsHeaders.push_back(SmarttoolsMacroHeaderParInfo(::GetDictionaryString("Modified"), SmarttoolsMacroHeaderParInfo::kModificationDate, *mModifiedColumnWidth));
}

void CFmiSmarttoolTabDlg::StoreWinRegistrySettings()
{
    *mRowColumnWidth = itsGridCtrl.GetColumnWidth(SmarttoolsMacroHeaderParInfo::kRowNumber);
    *mNameColumnWidth = itsGridCtrl.GetColumnWidth(SmarttoolsMacroHeaderParInfo::kMacroName);
    *mModifiedColumnWidth = itsGridCtrl.GetColumnWidth(SmarttoolsMacroHeaderParInfo::kModificationDate);
}

void CFmiSmarttoolTabDlg::FillGridRowsWithDirectoryMacroData(SmartToolMacroData::ContainerType &theMacroDirectory, int &theCurrentRowNumberInOut, FmiLanguage theLanguage)
{
    for(auto &macroData : theMacroDirectory)
    {
        macroData.itsGridControlRowNumber = theCurrentRowNumberInOut;
        SetGridRow(theCurrentRowNumberInOut++, macroData, theLanguage);
        if(macroData.fIsDirectory)
            FillGridRowsWithDirectoryMacroData(macroData.itsDirectoryContent, theCurrentRowNumberInOut, theLanguage);
    }
}

void CFmiSmarttoolTabDlg::InitMacroListFromDoc(void)
{
    CCellRange selectedCellRange = itsGridCtrl.GetSelectedCellRange();
    itsGridCtrl.SetRowCount(static_cast<int>(itsSmartToolMacroDataTree.itsTotalSize + itsGridCtrl.GetFixedRowCount()));
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    FillGridRowsWithDirectoryMacroData(itsSmartToolMacroDataTree.itsDirectoryContent, currentRowCount, itsSmartMetDocumentInterface->Language());

    if(itsSmartToolMacroDataTree.itsTreePatternArray)
    {
        std::vector<unsigned char> &treePatternArray = *itsSmartToolMacroDataTree.itsTreePatternArray;
        if(treePatternArray.size()) // pit‰‰ testata 0 koko vastaan, muuten voi kaatua
            itsTreeColumn.TreeSetup(&itsGridCtrl, 1, static_cast<int>(treePatternArray.size()), 1, &treePatternArray[0], TRUE, FALSE);
    }
}

static std::string GetColumnText(int theRow, int theColumn, const SmartToolMacroData &theMacroData, FmiLanguage theLanguage)
{
    switch(theColumn)
    {
    case SmarttoolsMacroHeaderParInfo::kRowNumber:
        return NFmiStringTools::Convert(theRow);
    case SmarttoolsMacroHeaderParInfo::kMacroName:
    {
        return theMacroData.itsDisplaydedName;
    }
    case SmarttoolsMacroHeaderParInfo::kModificationDate:
    {
        return std::string(theMacroData.itsModifiedTime.ToStr("YYYY.MM.DD HH:mm:SS", theLanguage));
    }
    default:
        return "";
    }
}

void CFmiSmarttoolTabDlg::SetGridRow(int row, const SmartToolMacroData &theMacroData, FmiLanguage theLanguage)
{
    static const COLORREF gMacroBkColor = RGB(255, 255, 255);
    static const COLORREF gDirectoryBkColor = RGB(242, 245, 255);

    for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
    {
        itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theMacroData, theLanguage).c_str()));
        if(column >= itsGridCtrl.GetFixedColumnCount())
        {
            itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); // Laita read-only -bitti p‰‰lle kaikille ruuduille
            if(theMacroData.fIsDirectory)
                itsGridCtrl.SetItemBkColour(row, column, gDirectoryBkColor);
            else
                itsGridCtrl.SetItemBkColour(row, column, gMacroBkColor);
        }
    }
}


BOOL CFmiSmarttoolTabDlg::Create(CWnd* pParentWnd)
{
    return CTabPageSSL::Create(CFmiSmarttoolTabDlg::IDD, pParentWnd);
}

// Tab dialogia varten
BOOL CFmiSmarttoolTabDlg::Create(UINT nIDTemplate, CWnd* pParentWnd)
{
    return CTabPageSSL::Create(nIDTemplate, pParentWnd);
}

void CFmiSmarttoolTabDlg::Update()
{
}

void CFmiSmarttoolTabDlg::DoWhenClosing(void)
{
}

void CFmiSmarttoolTabDlg::DoResizerHooking(void)
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_ACTION, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_MODIFY_ONLY_SELECTED_LOCATIONS, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_STR, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_EDITED_SMARTTOOL_PATH, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_SPLITTER_CONTROL, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_ERROR_STR, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_EDITED_SMARTTOOL_VIEWED_MACRO_PATH, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RICHEDIT_MACRO_ERROR_TEXT, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_DBCHECKER_SETTINGS, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMARTTOOL_SAVE, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMARTTOOL_SAVE_AS, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMARTTOOL_EDIT, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMARTTOOL_REMOVE, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SMARTTOOL_REFRESH, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CUSTOM_SMARTTOOL_GRID_CONTROL, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SPLITTER_SMARTTOOL_HORIZONTAL, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SPLITTER_SMARTTOOL_VERTICAL, ANCHOR_VERTICALLY | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_HORIZONTAL_SPLIT_SCREEN, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    
    // HUOM! IDC_RICHEDIT_MACRO_TEXT ja IDC_RICHEDIT_VIEW_ONLY_MACRO_TEXT kontrolleja ei saa laittaa 
    // resizer-systeemiin, koska ne joudutaan laskemaan erikseen AdjustSplitterControl -matodissa.

}

void CFmiSmarttoolTabDlg::OnSize(UINT nType, int cx, int cy)
{
    CTabPageSSL::OnSize(nType, cx, cy);

    itsSplitterHelper.AdjustSplitterControl();
}

LRESULT CFmiSmarttoolTabDlg::OnSplitterControlMove(WPARAM wParam, LPARAM lParam)
{
    //int xPos = LOWORD(lParam); // horizontal position of cursor
    //int yPos = HIWORD(lParam); // vertical position of cursor
    itsSplitterHelper.ReCalculateNewSplitFactor();
    return 1;
}


void CFmiSmarttoolTabDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // set the minimum tracking width and height of the window
    lpMMI->ptMinTrackSize.x = 470;
    lpMMI->ptMinTrackSize.y = 450;
}


void CFmiSmarttoolTabDlg::OnBnClickedCheckHorizontalSplitScreen()
{
    itsSplitterHelper.OnSplitterOrientationCheckboxClicked();
}

// Onko hiiren klikkaus osunut taulukon sis‰‰n JA se ei ole otsikko rivill‰.
bool CFmiSmarttoolTabDlg::IsClickedCellOk(const CCellID &theSelectedCell)
{
    if(theSelectedCell.IsValid() && theSelectedCell.row >= itsGridCtrl.GetFixedRowCount())
        return true;
    else
        return false;
}

static TCHAR* oldStr = _TEXT("\r\r\n");
static TCHAR* newStr = _TEXT("\r\n");
static int StripExtraCarrigeReturns(CString &theString)
{
    return theString.Replace(oldStr, newStr);
}

static void SetRichEditText(CRichEditCtrl &theRichEditCtrl, const CString &theNewText)
{
    // Jos tiedostossa olleessa tekstiss‰ on mukana "\r\r\n" -rivinvaihto yhdistelmi‰, ne aiheuttavat ongelmia.
    // Tekstit n‰kyv‰t sellaisenaan ihan ok kun se laitetaan richEditContolliin, mutta kun se copy-paste:taan
    // Joko samaan tai toiseen richEditControlliin, rivin vaihdot eiv‰t en‰‰ n‰y, vaan kaikki teksti on yhdell‰ rivill‰.
    CString strippedStr = theNewText;
    ::StripExtraCarrigeReturns(strippedStr);

    // Valitaan koko controllin teksti, ett‰ se voidaan korvata uudella tekstill‰ (n‰in s‰ilyy myˆs k‰ytetty fontti koko)
    theRichEditCtrl.SetSel(0, theRichEditCtrl.GetTextLength());
    theRichEditCtrl.ReplaceSel(strippedStr, TRUE);
}

static void SetRichEditText(CRichEditCtrl &theRichEditCtrl, const std::string &theNewText)
{
    ::SetRichEditText(theRichEditCtrl, CString(CA2T(theNewText.c_str())));
}

int globalCounter = 0;


// Otetaann left-clickin yhteydess‰ talteen sek‰ viewable macro text ett‰ sen polku teksti,
// jotta ne voidaan palauttaa tupla-klikin yhteydess‰ takaisin.
void CFmiSmarttoolTabDlg::MakeLeftClickBackups()
{
    itsViewOnlyMacroTextControl.GetWindowText(itsViewOnlyMacroText_DoubleClickBackupU_);
    itsViewOnlyMacroPathText_DoubleClickBackupU_ = itsViewedMacroPathU_;
}

void CFmiSmarttoolTabDlg::RestoreLeftClickBackups()
{
    // Pit‰‰ palauttaa viewOnly-kontrolliin originaali left-click:iss‰ ollut teksti
    ::SetRichEditText(itsViewOnlyMacroTextControl, itsViewOnlyMacroText_DoubleClickBackupU_);
    itsViewedMacroPathU_ = itsViewOnlyMacroPathText_DoubleClickBackupU_;
    MarkLeftClickIgnoreActions();

    UpdateData(FALSE);
}

void CFmiSmarttoolTabDlg::LeftClickedGridCell(const CCellID &theSelectedCell)
{
    if(!IgnoreLeftClickAfterDoubleClick())
    {
        if(IsClickedCellOk(theSelectedCell))
        {
            SmartToolMacroData *macroData = itsSmartToolMacroDataTree.FindMacro(theSelectedCell.row, itsGridCtrl.GetFixedRowCount());
            if(macroData && macroData->fIsDirectory == false && itsSmartToolInfo->SpeedLoadScript(macroData->itsOriginalName))
            {
                //TRACE("%d CFmiSmarttoolTabDlg::LeftClickedGridCell", globalCounter++);
                MakeLeftClickBackups();

                ::SetRichEditText(itsViewOnlyMacroTextControl, itsSmartToolInfo->CurrentScript());
                UpdatePathInfo(itsViewedMacroPathU_, macroData->itsOriginalName, false);
            }
        }
    }
}

bool CFmiSmarttoolTabDlg::IgnoreLeftClickAfterDoubleClick()
{
    if(fIgnoreLeftClickAfterDoubleClick)
    {
        fIgnoreLeftClickAfterDoubleClick = false;
        itsIgnoreLeftClickTimer.StopTimer();
        if(itsIgnoreLeftClickTimer.TimeDiffInMSeconds() < 150)
            return true; // ignoroidaan double clikin j‰lkeen tullut  left-click vain jos sanoma on tullut tarpeeksi nopeasti
    }
    return false;
}

void CFmiSmarttoolTabDlg::MarkLeftClickIgnoreActions()
{
    fIgnoreLeftClickAfterDoubleClick = true;
    itsIgnoreLeftClickTimer.StartTimer();
}

void CFmiSmarttoolTabDlg::DoubleLeftClickedGridCell(const CCellID &theSelectedCell)
{
    PutSelectedMacroToEditingControl(theSelectedCell, true);
}

void CFmiSmarttoolTabDlg::UpdatePathInfo(CString &thePathVariable, const std::string &theRelativeMacroPath, bool editedMacro)
{
    std::string totalPathStr = editedMacro ? ::GetDictionaryString("Edited") : ::GetDictionaryString("Viewed");
    totalPathStr += ": ";
    totalPathStr += theRelativeMacroPath;

    thePathVariable = CA2T(totalPathStr.c_str());
    UpdateData(FALSE);
}

void CFmiSmarttoolTabDlg::PutSelectedMacroToEditingControl(const CCellID &theSelectedCell, bool doDoubleClickActions)
{
    if(IsClickedCellOk(theSelectedCell))
    {
        SmartToolMacroData *macroData = itsSmartToolMacroDataTree.FindMacro(theSelectedCell.row, itsGridCtrl.GetFixedRowCount());
        if(macroData && macroData->fIsDirectory == false && itsSmartToolInfo->SpeedLoadScript(macroData->itsOriginalName))
        {
            //TRACE("%d CFmiSmarttoolTabDlg::DoubleLeftClickedGridCell", globalCounter++);
            ::SetRichEditText(itsMacroTextControl, itsSmartToolInfo->CurrentScript());

            if(doDoubleClickActions)
                RestoreLeftClickBackups();
            UpdatePathInfo(itsEditedMacroPathU_, macroData->itsOriginalName, true);
        }
    }
}

void CFmiSmarttoolTabDlg::RightClickedGridCell(const CCellID &theSelectedCell)
{
    CCellRange range(theSelectedCell.row, 0, theSelectedCell.row, itsGridCtrl.GetColumnCount() - 1);
    itsGridCtrl.SetSelectedRange(range, TRUE);
}

void CFmiSmarttoolTabDlg::OnBnClickedButtonSmarttoolEdit()
{
    PutSelectedMacroToEditingControl(itsGridCtrl.GetSelectedCellRange().GetTopLeft(), false);
}

void CFmiSmarttoolTabDlg::ResetSearchResource()
{
    // Add data source from ViewMacroSystem
    itsSpeedSearchMacroControl.RemoveAll();
    const std::vector<std::string> &smarttoolNames = itsSmartMetDocumentInterface->SmartToolFileNames(true);

    for(const auto &name : smarttoolNames)
        itsSpeedSearchMacroControl.AddSearchString(CA2T(name.c_str()));

    MakeSmartToolMacroDataContent(smarttoolNames, itsSmartMetDocumentInterface->SmartToolInfo()->RootLoadDirectory());
}

void CFmiSmarttoolTabDlg::InitSpeedSearchControl()
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

LRESULT CFmiSmarttoolTabDlg::OnGUSIconEditLeftIconClicked(WPARAM wParam, LPARAM lParam)
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

LRESULT CFmiSmarttoolTabDlg::OnGUSIconEditRightIconClicked(WPARAM wParam, LPARAM lParam)
{
    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL == nCtrlId)
    {
        if(itsSpeedSearchMacroControl.GetEditTextLength() > 0)
            itsSpeedSearchMacroControl.ClearEditText();
    }

    return 1;
}

LRESULT CFmiSmarttoolTabDlg::OnGUSIconEditSearchListClicked(WPARAM wParam, LPARAM lParam)
{
    int nCtrlId = (int)wParam;
    if(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL == nCtrlId)
    {
        std::string selectedMacroName = CT2A(itsSpeedSearchMacroControl.GetSelectedString());
//        DoSmartToolLoad(selectedMacroName, true);
    }

    return 1;
}

void CFmiSmarttoolTabDlg::OnSearchOptionCaseSesensitive()
{
    fSearchOptionCaseSensitive = !fSearchOptionCaseSensitive;
    itsSpeedSearchMacroControl.SetSearchCaseSense(fSearchOptionCaseSensitive);
}

void CFmiSmarttoolTabDlg::OnSearchOptionMatchAnywhere()
{
    fSearchOptionMatchAnywhere = !fSearchOptionMatchAnywhere;
    itsSpeedSearchMacroControl.SetSearchAnywhere(fSearchOptionMatchAnywhere);
}


void CFmiSmarttoolTabDlg::OnEnChangeEditSpeedSearchViewMacro()
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

void CFmiSmarttoolTabDlg::MakeSmartToolMacroDataContent(const std::vector<std::string> &theSmarttoolMacroNames, const std::string &theAbsoluteRootPath)
{
    itsSmartToolMacroDataTree.Reset(); 
    itsSmartToolMacroDataTree.fIsDirectory = true; // Huom! rootilla ei ole nime‰ eik‰ oikeastaan muita tietoja kuin ett‰ se on hakemisto ja siin‰ hakemistossa olevat macrot ja alihakemistot

    for(const auto &macroPath : theSmarttoolMacroNames)
    {
        try
        {
            itsSmartToolMacroDataTree.InsertMacro(macroPath, macroPath, theAbsoluteRootPath);
        }
        catch(...)
        {
        }
    }
    itsSmartToolMacroDataTree.SortMacros();
    itsSmartToolMacroDataTree.MakeTreePatternArray();
}


