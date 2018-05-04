// FmiMacroParamTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiMacroParamTabDlg.h"
#include "afxdialogex.h"


// CFmiMacroParamTabDlg dialog

IMPLEMENT_DYNAMIC(CFmiMacroParamTabDlg, CTabPageSSL)

CFmiMacroParamTabDlg::CFmiMacroParamTabDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
: CTabPageSSL(CFmiMacroParamTabDlg::IDD, pParent)
    ,itsSmartMetDocumentInterface(smartMetDocumentInterface)
    , itsSplitterHelper(this, IDC_BUTTON_SPLITTER_MACROPARAM_HORIZONTAL, IDC_BUTTON_SPLITTER_MACROPARAM_VERTICAL, 0.6, IDC_CHECK_HORIZONTAL_SPLIT_SCREEN, IDC_RICHEDIT_MACROPARAM_TEXT, IDC_RICHEDIT_VIEW_ONLY_MACROPARAM_TEXT, IDC_STATIC_SPLITTER_CONTROL)
{

}

CFmiMacroParamTabDlg::~CFmiMacroParamTabDlg()
{
}

BOOL CFmiMacroParamTabDlg::Create(CWnd* pParentWnd)
{
    return CTabPageSSL::Create(CFmiMacroParamTabDlg::IDD, pParentWnd);
}

// Tab dialogia varten
BOOL CFmiMacroParamTabDlg::Create(UINT nIDTemplate, CWnd* pParentWnd)
{
    return CTabPageSSL::Create(nIDTemplate, pParentWnd);
}

void CFmiMacroParamTabDlg::Update()
{
}

void CFmiMacroParamTabDlg::DoWhenClosing(void)
{
}

void CFmiMacroParamTabDlg::DoDataExchange(CDataExchange* pDX)
{
    CTabPageSSL::DoDataExchange(pDX);
    DDX_GridControl(pDX, IDC_CUSTOM_MACROPARAM_GRID_CONTROL, itsGridCtrl);
    itsSplitterHelper.SetUpDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFmiMacroParamTabDlg, CTabPageSSL)
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()
    ON_MESSAGE(UWM_SPLIT_MOVED, &CFmiMacroParamTabDlg::OnSplitterControlMove)
    ON_BN_CLICKED(IDC_CHECK_HORIZONTAL_SPLIT_SCREEN, &CFmiMacroParamTabDlg::OnBnClickedCheckHorizontalSplitScreen)
END_MESSAGE_MAP()


// CFmiMacroParamTabDlg message handlers


BOOL CFmiMacroParamTabDlg::OnInitDialog()
{
    CTabPageSSL::OnInitDialog();
    itsSplitterHelper.PreResizerHookingSetup();
    DoResizerHooking(); // Tätä pitää kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisteristä
    itsSplitterHelper.PostResizerHookingSetup();

    // TODO:  Add extra initialization here

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiMacroParamTabDlg::DoResizerHooking(void)
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_STR, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_HORIZONTAL_SPLIT_SCREEN, ANCHOR_LEFT | ANCHOR_TOP);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_EDITED_MACROPARAM_PATH, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_VIEWED_MACROPARAM_PATH, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_SPLITTER_CONTROL, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_ERROR_STR, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RICHEDIT_MACRO_ERROR_TEXT, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_SPEED_SEARCH_MACRO_CONTROL, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACROPARAM_SAVE, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACROPARAM_SAVE_AS, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACROPARAM_EDIT, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACROPARAM_REMOVE, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_MACROPARAM_REFRESH, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CUSTOM_MACROPARAM_GRID_CONTROL, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SPLITTER_MACROPARAM_HORIZONTAL, ANCHOR_BOTTOM | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_SPLITTER_MACROPARAM_VERTICAL, ANCHOR_VERTICALLY | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);

    bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_PARAM_DATA_GRID_SIZE_STR2, ANCHOR_BOTTOM | ANCHOR_LEFT);
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
    bOk = m_resizer.SetAnchor(IDC_RADIO_MACROPARAM_SELECTED_MAP1, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RADIO_MACROPARAM_SELECTED_MAP2, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_RADIO_MACROPARAM_SELECTED_MAP3, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_MACRO_PARAM_CROSSSECTION_MODE, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_USED_ROW_STR, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MAP_VIEW_NUMBER_STR, ANCHOR_BOTTOM | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    // HUOM! IDC_RICHEDIT_MACRO_TEXT ja IDC_RICHEDIT_VIEW_ONLY_MACRO_TEXT kontrolleja ei saa laittaa 
    // resizer-systeemiin, koska ne joudutaan laskemaan erikseen AdjustSplitterControl -matodissa.

}



void CFmiMacroParamTabDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // set the minimum tracking width and height of the window
    lpMMI->ptMinTrackSize.x = 470;
    lpMMI->ptMinTrackSize.y = 450;
}


void CFmiMacroParamTabDlg::OnSize(UINT nType, int cx, int cy)
{
    CTabPageSSL::OnSize(nType, cx, cy);

    itsSplitterHelper.AdjustSplitterControl();
}

LRESULT CFmiMacroParamTabDlg::OnSplitterControlMove(WPARAM wParam, LPARAM lParam)
{
    //int xPos = LOWORD(lParam); // horizontal position of cursor
    //int yPos = HIWORD(lParam); // vertical position of cursor
    itsSplitterHelper.ReCalculateNewSplitFactor();
    return 1;
}


void CFmiMacroParamTabDlg::OnBnClickedCheckHorizontalSplitScreen()
{
    itsSplitterHelper.OnSplitterOrientationCheckboxClicked();
}
