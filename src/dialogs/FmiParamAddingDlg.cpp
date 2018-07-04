// FmiParamAddingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiParamAddingDlg.h"
#include "afxdialogex.h"
#include "PERSIST2.H"
#include "CloneBitmap.h"
#include "SmartMetToolboxDep_resource.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "ParamAddingSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMenuItem.h"
#include "SpecialDesctopIndex.h"
#include "boost\math\special_functions\round.hpp"

// *************************************************
// NFmiParamAddingGridCtrl

UINT_PTR g_TitleTextUpdater = 0;
const UINT_PTR g_TitleTextUpdaterTimer = 1;
const std::string g_TitleStr = ::GetDictionaryString("Param adding dialog");

IMPLEMENT_DYNCREATE(NFmiParamAddingGridCtrl, CGridCtrl)

// CFmiParamAddingDlg message handlers
BEGIN_MESSAGE_MAP(NFmiParamAddingGridCtrl, CGridCtrl)
    ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

void NFmiParamAddingGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CGridCtrl::OnLButtonDblClk(nFlags, point);

    if(itsLButtonDblClkCallback)
        itsLButtonDblClkCallback();
}


// *************************************************
// CFmiParamAddingDlg dialog

const NFmiViewPosRegistryInfo CFmiParamAddingDlg::s_ViewPosRegistryInfo(CRect(360, 240, 830, 760), "\\ParamAddingDlg");

IMPLEMENT_DYNAMIC(CFmiParamAddingDlg, CDialogEx)

CFmiParamAddingDlg::CFmiParamAddingDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
    : CDialogEx(IDD_DIALOG_PARAM_ADDING, pParent)
    , itsGridCtrl()
    , itsTreeColumn()
    , itsHeaders()
    , fDialogInitialized(false)
    , itsSmartMetDocumentInterface(smartMetDocumentInterface)
    , itsParamAddingSystem(&(smartMetDocumentInterface->ParamAddingSystem()))
{

}

CFmiParamAddingDlg::~CFmiParamAddingDlg()
{
}

void CFmiParamAddingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_GridControl(pDX, IDC_CUSTOM_GRID_PARAM_ADDING, itsGridCtrl);
}


BEGIN_MESSAGE_MAP(CFmiParamAddingDlg, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CFmiParamAddingDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiParamAddingDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

BOOL CFmiParamAddingDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    fDialogInitialized = true;

    HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
    this->SetIcon(hIcon, FALSE);

    // Call InitHeaders before CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry !!
    InitDialogTexts();
    InitHeaders();

    // Tee paikan asetus vasta tooltipin alustuksen j‰lkeen, niin se toimii ilman OnSize-kutsua.
    std::string errorBaseStr("Error in CFmiCaseStudyDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

    // Do not allow selection, it would look bad for one row being all blue
    itsGridCtrl.EnableSelection(FALSE);
    UpdateGridControlValues();
    auto LButtonDblClkCallback = [this]() {this->HandleGridCtrlsLButtonDblClk(); };
    itsGridCtrl.itsLButtonDblClkCallback = LButtonDblClkCallback;
    AdjustDialogControls();

    // Aletaan tarkastelemaan kerran sekunnissa ett‰ mik‰ on aktiivinen n‰yttˆ ja aktiivinen rivi ja p‰ivitet‰‰n tarvittaessa otsikon teksti‰ vastaavasti
    g_TitleTextUpdater = SetTimer(g_TitleTextUpdaterTimer, 1000, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiParamAddingDlg::OnCancel()
{
    DoWhenClosing();

    CDialogEx::OnCancel();
}

void CFmiParamAddingDlg::OnOK()
{
    DoWhenClosing();

    CDialogEx::OnOK();
}

void CFmiParamAddingDlg::OnClose()
{
    DoWhenClosing();

    CDialogEx::OnClose();
}

void CFmiParamAddingDlg::DoWhenClosing(void)
{
    AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
}

void CFmiParamAddingDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    lpMMI->ptMinTrackSize.x = 250;
    lpMMI->ptMinTrackSize.y = 250;

    CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CFmiParamAddingDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    AdjustDialogControls();
}

void CFmiParamAddingDlg::AdjustDialogControls(void)
{
    if(fDialogInitialized)
    {
        CWnd *win = GetDlgItem(IDC_CUSTOM_GRID_PARAM_ADDING);
        if(win)
        {
            CRect gridControlRect;
            GetClientRect(&gridControlRect);
            win->MoveWindow(gridControlRect);
            FitNameColumnOnVisibleArea(gridControlRect.Width());
        }
    }
}

#ifdef max
#undef max
#endif

void CFmiParamAddingDlg::FitNameColumnOnVisibleArea(int gridCtrlWidth)
{
    if(itsGridCtrl.GetColumnCount())
    {
        CRect cellRect;
        itsGridCtrl.GetCellRect(0, 0, cellRect);
        int otherColumnsCombinedWidth = cellRect.Width();
        itsGridCtrl.GetCellRect(0, 2, cellRect);
        otherColumnsCombinedWidth += cellRect.Width();

        // Calculate new width for name column so that it will fill the client area
        // Total width (gridCtrlWidth) - otherColumnsCombinedWidth - some value (32) that represents the width of the vertical scroll control
        int newNameColumnWidth = gridCtrlWidth - otherColumnsCombinedWidth - 32;
        // Let's make sure that last column isn't shrinken too much
        newNameColumnWidth = std::max(newNameColumnWidth, 120);
        itsGridCtrl.SetColumnWidth(1, newNameColumnWidth);
    }
}


void CFmiParamAddingDlg::InitHeaders(void)
{
    int basicColumnWidthUnit = 18;
    itsHeaders.clear();
    itsHeaders.push_back(ParamAddingHeaderParInfo("Row", ParamAddingHeaderParInfo::kRowNumber, boost::math::iround(basicColumnWidthUnit * 3.5)));
    itsHeaders.push_back(ParamAddingHeaderParInfo("Name", ParamAddingHeaderParInfo::kItemName, boost::math::iround(basicColumnWidthUnit * 24.)));
    itsHeaders.push_back(ParamAddingHeaderParInfo("Id", ParamAddingHeaderParInfo::kItemId, boost::math::iround(basicColumnWidthUnit * 3.5)));
}

static const COLORREF gFixedBkColor = RGB(239, 235, 222);

static void SetHeaders(CGridCtrl &theGridCtrl, const std::vector<ParamAddingHeaderParInfo> &theHeaders, int rowCount, int theFixedRowCount, int theFixedColumnCount, bool &fFirstTime)
{
    int columnCount = static_cast<int>(theHeaders.size());
    theGridCtrl.SetRowCount(rowCount);
    theGridCtrl.SetColumnCount(columnCount);
    theGridCtrl.SetGridLines(GVL_BOTH);
    theGridCtrl.SetFixedRowCount(theFixedRowCount);
    theGridCtrl.SetFixedColumnCount(theFixedColumnCount);
    theGridCtrl.SetListMode(TRUE);
    theGridCtrl.SetHeaderSort(FALSE);
    theGridCtrl.SetFixedBkColor(gFixedBkColor);

    int currentRow = 0;
    // 1. on otsikko rivi on parametrien nimi‰ varten
    for(int i = 0; i<columnCount; i++)
    {
        theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
        theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
        if(fFirstTime) // s‰‰det‰‰n sarakkeiden leveydet vain 1. kerran
            theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
    }
    fFirstTime = false;
}

static bool IsTreeNodeCollapsed(CTreeColumn &treeColumnControl, int rowIndex)
{
    BOOL isPlus = FALSE;
    BOOL isMinus = FALSE;
    BOOL isLastLeaf = FALSE;

    if(treeColumnControl.TreeCellHasPlusMinus(rowIndex, &isPlus, &isMinus, &isLastLeaf))
    {
        return isPlus == TRUE;
    }
    else
        return true;
}

void CFmiParamAddingDlg::SetTreeNodeInformationBackToDialogRowData()
{
    auto &rowData = itsParamAddingSystem->dialogRowData();
    int rowIndex = itsGridCtrl.GetFixedRowCount();
    for(auto &rowItem : rowData)
    {
        rowItem.dialogTreeNodeCollapsed(::IsTreeNodeCollapsed(itsTreeColumn, rowIndex++));
    }
}

void CFmiParamAddingDlg::UpdateGridControlValues(void)
{
    static bool fFirstTime = true;
    int fixedRowCount = 1;
    int fixedColumnCount = 1;

    if(fFirstTime || itsParamAddingSystem->dialogDataNeedsUpdate())
    {
        SetTreeNodeInformationBackToDialogRowData();
        itsParamAddingSystem->updateDialogData();
        int dataRowCount = static_cast<int>(itsParamAddingSystem->dialogRowData().size());
        int maxRowCount = fixedRowCount + dataRowCount;
        SetHeaders(itsGridCtrl, itsHeaders, maxRowCount, fixedRowCount, fixedColumnCount, fFirstTime);
        UpdateRows(fixedRowCount, fixedColumnCount, false);

        const auto &treePatternArray = itsParamAddingSystem->dialogTreePatternArray();
        if(treePatternArray.size()) // pit‰‰ testata 0 koko vastaan, muuten voi kaatua
        {
            itsTreeColumn.TreeSetup(&itsGridCtrl, 1, static_cast<int>(treePatternArray.size()), 1, &treePatternArray[0], TRUE, FALSE);
            MakeTreeNodeCollapseSettings();
        }
        fFirstTime = false;
    }
}

void CFmiParamAddingDlg::MakeTreeNodeCollapseSettings()
{
    const auto &rowItemData = itsParamAddingSystem->dialogRowData();
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    // First collapse all nodes
    itsTreeColumn.TreeDataCollapseAllSubLevels(currentRowCount);
    // Then open them one by one according to settings
    for(const auto &rowItem : rowItemData)
    {
        if(!rowItem.dialogTreeNodeCollapsed())
        {
            itsTreeColumn.TreeDataExpandOneLevel(currentRowCount);
        }
        currentRowCount++;
    }
    itsTreeColumn.TreeRefreshRows();
}

void CFmiParamAddingDlg::UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly)
{
    const auto &rowData = itsParamAddingSystem->dialogRowData();
    int currentRowCount = fixedRowCount;
    for(size_t i = 0; i < rowData.size(); i++)
    {
        SetGridRow(currentRowCount++, rowData[i], fixedColumnCount);
    }
}

static const COLORREF gCategoryColor = RGB(250, 220, 220);
static const COLORREF gProducerColor = RGB(220, 250, 220);
//static const COLORREF gDataColor = RGB(220, 220, 250);
static const COLORREF gDataColor = RGB(200, 200, 255);
static const COLORREF gParamColor = RGB(255, 255, 255);
static const COLORREF gSubParamColor = RGB(255, 255, 245);
static const COLORREF gLevelColor1 = RGB(200, 200, 255);
static const COLORREF gLevelColor2 = RGB(170, 170, 255);
static const COLORREF gLevelColor3 = RGB(140, 140, 255);
static const COLORREF gLevelColor4 = RGB(110, 110, 255);
static const COLORREF gLevelColor5 = RGB(90, 90, 255);
static const COLORREF gLevelColor6 = RGB(60, 60, 255);
static const COLORREF gLevelColor7 = RGB(40, 40, 255);
static const COLORREF gLevelColor8 = RGB(20, 20, 255);
static const COLORREF gLevelColor9 = RGB(10, 10, 255);
static const COLORREF gLevelColor10 = RGB(0, 0, 255);
static const COLORREF gErrorColor = RGB(190, 190, 190);

static COLORREF getUsedBackgroundColor(const AddParams::SingleRowItem &theRowItem)
{
    // Params always have white background color
    if(theRowItem.leafNode()) { return gParamColor; }

    if(!NFmiDrawParam::IsMacroParamCase(theRowItem.dataType()))
    {
        switch(theRowItem.rowType())
        {
        case AddParams::kCategoryType:
            return gCategoryColor;
        case AddParams::kProducerType:
            return gProducerColor;
        case AddParams::kDataType:
            return gDataColor;
        case AddParams::kParamType:
            return gParamColor;
        case AddParams::kSubParamType:
        case AddParams::kSubParamLevelType:
            return gSubParamColor;
        case AddParams::kLevelType:
            return gLevelColor1;
        default:
            return gErrorColor;
        }
    }

    switch(theRowItem.treeDepth())
    {
    case 1:
        return gCategoryColor;
    case 2:
        return gProducerColor;
    case 3:
        return gLevelColor1;
    case 4:
        return gLevelColor2;
    case 5:
        return gLevelColor3;
    case 6:
        return gLevelColor4;
    case 7:
        return gLevelColor5;
    case 8:
        return gLevelColor6;
    case 9:
        return gLevelColor7;
    case 10:
        return gLevelColor8;
    case 11:
        return gLevelColor9;
    case 12:
        return gLevelColor10;
    default:
        return gErrorColor;
    }
}

static std::string GetColumnText(int theRow, int theColumn, const AddParams::SingleRowItem &theRowItem)
{
    switch(theColumn)
    {
    case ParamAddingHeaderParInfo::kRowNumber:
        return std::to_string(theRow);
    case ParamAddingHeaderParInfo::kItemName:
        return theRowItem.itemName();
    case ParamAddingHeaderParInfo::kItemId:
    {
        if(theRowItem.itemId())
            return std::to_string(theRowItem.itemId());
        else
            return ""; // 0 is value for non interesting id (like for category id)
    }
    default:
        return "";
    }
}

void CFmiParamAddingDlg::SetGridRow(int row, const AddParams::SingleRowItem &theRowItem, int theFixedColumnCount)
{
    for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
    {
        itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theRowItem).c_str()));
        if(column >= theFixedColumnCount)
        {
            // Laita read-only -bitti p‰‰lle
            itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); 
            COLORREF usedBkColor = ::getUsedBackgroundColor(theRowItem);
            itsGridCtrl.SetItemBkColour(row, column, usedBkColor);
        }
    }
}

void CFmiParamAddingDlg::Update()
{
    if(IsWindowVisible())
        UpdateGridControlValues();
}

void CFmiParamAddingDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(g_TitleStr.c_str()));
//    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_PRINT, "IDC_BUTTON_PRINT");
}

void CFmiParamAddingDlg::HandleGridCtrlsLButtonDblClk()
{
    auto cell = itsGridCtrl.GetFocusCell();
    if(cell.IsValid())
    {
        int rowItemIndex = cell.row - itsGridCtrl.GetFixedRowCount();
        const auto &rowData = itsParamAddingSystem->dialogRowData();
        if(rowItemIndex >= 0 && rowItemIndex < rowData.size())
            HandleRowItemSelection(rowData[rowItemIndex]);
    }
}

void CFmiParamAddingDlg::HandleRowItemSelection(const AddParams::SingleRowItem &rowItem)
{    
    if(rowItem.dataType() != NFmiInfoData::kNoDataType && rowItem.leafNode())
    {
        NFmiMenuItem *addParamCommand;
        if(NFmiDrawParam::IsMacroParamCase(rowItem.dataType())) {
            addParamCommand = new NFmiMenuItem(
                static_cast<int>(itsParamAddingSystem->LastAcivatedDescTopIndex()),
                rowItem.itemName(),
                NFmiDataIdent(NFmiParam(rowItem.itemId(), rowItem.displayName()), NFmiProducer(rowItem.parentItemId(), rowItem.parentItemName())),
                kAddViewWithRealRowNumber,
                NFmiMetEditorTypes::kFmiParamsDefaultView,
                rowItem.level().get(),
                rowItem.dataType());
                                                                                                                                                                                                                                                          
            addParamCommand->MacroParamInitName(rowItem.uniqueDataId());
        } 
        else
        {
            addParamCommand = new NFmiMenuItem(
                static_cast<int>(itsParamAddingSystem->LastAcivatedDescTopIndex()),
                NFmiString("Add some param"),
                NFmiDataIdent(NFmiParam(rowItem.itemId(), rowItem.displayName()), NFmiProducer(rowItem.parentItemId(), rowItem.parentItemName())),
                kAddViewWithRealRowNumber,
                NFmiMetEditorTypes::kFmiParamsDefaultView,
                rowItem.level().get(), 
                rowItem.dataType());
        }

        itsSmartMetDocumentInterface->ExecuteCommand(*addParamCommand, itsParamAddingSystem->LastActivatedRowIndex(), 0);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("ParamAddingDlg: Adding param to map view");
    }
}

std::string CFmiParamAddingDlg::MakeActiveViewRowText()
{
    std::string str;
    itsLastAcivatedDescTopIndex = itsParamAddingSystem->LastAcivatedDescTopIndex();
    if(itsLastAcivatedDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
    {
        str += "Active Map view ";
        str += std::to_string(itsLastAcivatedDescTopIndex + 1);
    }
    else if(itsLastAcivatedDescTopIndex == CtrlViewUtils::kFmiTimeSerialView)
    {
        str += "Time view active";
    }
    else if(itsLastAcivatedDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
    {
        str += "Cross section view active";
    }

    str += ", active row ";
    itsLastActivatedRowIndex = itsParamAddingSystem->LastActivatedRowIndex();
    str += std::to_string(itsLastActivatedRowIndex);

    return str;
}

void CFmiParamAddingDlg::OnTimer(UINT_PTR nIDEvent)
{
    switch(nIDEvent)
    {
    case g_TitleTextUpdaterTimer:
    {
        if(NeedToUpdateTitleText())
        {
            SetWindowText(CA2T(MakeTitleText().c_str()));
        }
        return;
    }
    }

    CDialogEx::OnTimer(nIDEvent);
}

bool CFmiParamAddingDlg::NeedToUpdateTitleText()
{
    if(itsLastAcivatedDescTopIndex != itsParamAddingSystem->LastAcivatedDescTopIndex() || itsLastActivatedRowIndex != itsParamAddingSystem->LastActivatedRowIndex())
        return true;
    else
        return false;
}

std::string CFmiParamAddingDlg::MakeTitleText()
{
    std::string str = g_TitleStr;
    str += " (";
    str += MakeActiveViewRowText();
    str += ")";
    return str;
}


BOOL CFmiParamAddingDlg::OnEraseBkgnd(CDC* pDC)
{
    return FALSE;

    //return CDialogEx::OnEraseBkgnd(pDC);
}
