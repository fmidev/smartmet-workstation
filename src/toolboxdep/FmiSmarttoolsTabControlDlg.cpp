// FmiSmarttoolsTabControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiSmarttoolsTabControlDlg.h"
#include "CloneBitmap.h"
#include "NFmiDictionaryFunction.h"


// CFmiSmarttoolsTabControlDlg dialog

IMPLEMENT_DYNAMIC(CFmiSmarttoolsTabControlDlg, CDialog)

CFmiSmarttoolsTabControlDlg::CFmiSmarttoolsTabControlDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
: CDialog(CFmiSmarttoolsTabControlDlg::IDD, pParent)
, itsSmarttoolsTab(smartMetDocumentInterface, pParent)
, itsMacroParamTab(smartMetDocumentInterface, pParent)
, itsIcon()
, itsSmartMetDocumentInterface(smartMetDocumentInterface)
{
    itsIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
}

CFmiSmarttoolsTabControlDlg::~CFmiSmarttoolsTabControlDlg()
{
    DoWhenClosing(true);
}

void CFmiSmarttoolsTabControlDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_SMARTTOOLS_MACRO_CONTROL, itsTabControl);
}


BEGIN_MESSAGE_MAP(CFmiSmarttoolsTabControlDlg, CDialog)
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CFmiSmarttoolsTabControlDlg message handlers


BOOL CFmiSmarttoolsTabControlDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    DoResizerHooking(); // Tätä pitää kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisteristä

    SetIcon(itsIcon, FALSE);

    // Setup the tab control
    int nPageID = 0;
    itsSmarttoolsTab.Create(CFmiSmarttoolTabDlg::IDD, this);
    itsTabControl.AddSSLPage(CA2T(::GetDictionaryString("Smarttools").c_str()), nPageID++, &itsSmarttoolsTab);
    itsMacroParamTab.Create(CFmiMacroParamTabDlg::IDD, this);
    itsTabControl.AddSSLPage(CA2T(::GetDictionaryString("MacroParams").c_str()), nPageID++, &itsMacroParamTab);
    itsTabControl.ActivateSSLPage(0);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

// modaalitonta dialogi luontia varten
BOOL CFmiSmarttoolsTabControlDlg::Create(CWnd* pParentWnd)
{
    return CDialog::Create(CFmiSmarttoolsTabControlDlg::IDD, pParentWnd);
}

void CFmiSmarttoolsTabControlDlg::OnPaint()
{
    if(IsIconic())	{
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, itsIcon);
    }
    else {
        CDialog::OnPaint();
    }
}


HCURSOR CFmiSmarttoolsTabControlDlg::OnQueryDragIcon()
{
    return (HCURSOR)itsIcon;
}

void CFmiSmarttoolsTabControlDlg::Update()
{
    itsSmarttoolsTab.Update();
    itsMacroParamTab.Update();
}


void CFmiSmarttoolsTabControlDlg::OnCancel()
{
    DoWhenClosing(false);

    CDialog::OnCancel();
}


void CFmiSmarttoolsTabControlDlg::OnOK()
{
    DoWhenClosing(false);

    CDialog::OnOK();
}


void CFmiSmarttoolsTabControlDlg::OnClose()
{
    DoWhenClosing(false);

    CDialog::OnClose();
}

void CFmiSmarttoolsTabControlDlg::DoWhenClosing(bool calledFromDestructor)
{
//    itsDoc->BetaProductionSystem().BetaProductTabControlIndex(itsBetaProductControl.GetSSLActivePage());
//    itsDoc->BetaProductionSystem().BetaProductSaveInitialPath(BetaProduct::InitialSavePath()); // Talletetaan tämä kerran kaikkiene BetaProduct tabi-dialogien puolesta

    if(!calledFromDestructor)
    {
        itsSmarttoolsTab.DoWhenClosing();
        itsMacroParamTab.DoWhenClosing();

        AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
    }
}

void CFmiSmarttoolsTabControlDlg::DoResizerHooking(void)
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_TAB_SMARTTOOLS_MACRO_CONTROL, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
}

void CFmiSmarttoolsTabControlDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    itsTabControl.ResizeActiveSSLPage();
}

void CFmiSmarttoolsTabControlDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // set the minimum tracking width and height of the window
    lpMMI->ptMinTrackSize.x = 505;
    lpMMI->ptMinTrackSize.y = 538;
}
