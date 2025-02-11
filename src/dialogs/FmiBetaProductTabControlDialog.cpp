// FmiBetaProductTabControlDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FmiBetaProductTabControlDialog.h"
#include "afxdialogex.h"
#include "CloneBitmap.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiBetaProductSystem.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiDictionaryFunction.h"
#include "persist2.h"
#include "FmiWin32Helpers.h"


// CFmiBetaProductTabControlDialog dialog

const NFmiViewPosRegistryInfo CFmiBetaProductTabControlDialog::s_ViewPosRegistryInfo(CRect(250, 120, 370, 210), "\\BetaProductDialog");

IMPLEMENT_DYNAMIC(CFmiBetaProductTabControlDialog, CDialogEx)

CFmiBetaProductTabControlDialog::CFmiBetaProductTabControlDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
: CDialogEx(CFmiBetaProductTabControlDialog::IDD, pParent)
, itsBetaProductDialogTab(smartMetDocumentInterface)
, itsBetaProductAutomationDialogTab(smartMetDocumentInterface)
, itsBetaProductControl()
, itsSmartMetDocumentInterface(smartMetDocumentInterface)
{
}

CFmiBetaProductTabControlDialog::~CFmiBetaProductTabControlDialog()
{
    DoWhenClosing(true);
}

void CFmiBetaProductTabControlDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_BETA_PRODUCT_CONTROL, itsBetaProductControl);
}


BEGIN_MESSAGE_MAP(CFmiBetaProductTabControlDialog, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CFmiBetaProductTabControlDialog message handlers


BOOL CFmiBetaProductTabControlDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CFmiWin32Helpers::SetUsedWindowIconDynamically(this);

    // Setup the tab control
    int nPageID = 0;
    itsBetaProductDialogTab.Create(CFmiBetaProductDialog::IDD, this);
    itsBetaProductControl.AddSSLPage(CA2T(::GetDictionaryString("Beta product").c_str()), nPageID++, &itsBetaProductDialogTab);
    itsBetaProductAutomationDialogTab.Create(CFmiBetaProductAutomationDialog::IDD, this);
    itsBetaProductControl.AddSSLPage(CA2T(::GetDictionaryString("Automation").c_str()), nPageID++, &itsBetaProductAutomationDialogTab);
    itsBetaProductControl.ActivateSSLPage(itsSmartMetDocumentInterface->BetaProductionSystem().BetaProductTabControlIndex());

    std::string errorBaseStr("Error in CFmiBetaProductTabControlDialog::OnInitDialog while reading dialog size (and position) values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, true, errorBaseStr, 0);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiBetaProductTabControlDialog::SetDefaultValues(void)
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    // gets current window position
    BOOL bRet = GetWindowPlacement(&wndpl);

    CRect oldRect(wndpl.rcNormalPosition);
    const CRect &defaultRect = CFmiBetaProductTabControlDialog::ViewPosRegistryInfo().DefaultWindowRect();
    // HUOM! dialogin kokoa ei saa muuttaa!!
    MoveWindow(defaultRect.left, defaultRect.top, oldRect.Width(), oldRect.Height()); 
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

// modaalitonta dialogi luontia varten
BOOL CFmiBetaProductTabControlDialog::Create(CWnd* pParentWnd)
{
    return CDialogEx::Create(CFmiBetaProductTabControlDialog::IDD, pParentWnd);
}

void CFmiBetaProductTabControlDialog::OnPaint()
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
        dc.DrawIcon(x, y, GetIcon(FALSE));
    }
    else {
        CDialog::OnPaint();
    }
}


HCURSOR CFmiBetaProductTabControlDialog::OnQueryDragIcon()
{
    return (HCURSOR)GetIcon(FALSE);
}

void CFmiBetaProductTabControlDialog::Update()
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        auto tabIndex = itsBetaProductControl.GetSSLActivePage();
        if(tabIndex == 0)
            itsBetaProductDialogTab.Update();
        else if(tabIndex == 1)
            itsBetaProductAutomationDialogTab.Update();
    }
}


void CFmiBetaProductTabControlDialog::OnCancel()
{
    DoWhenClosing(false);

    CDialogEx::OnCancel();
}


void CFmiBetaProductTabControlDialog::OnOK()
{
    DoWhenClosing(false);

    CDialogEx::OnOK();
}


void CFmiBetaProductTabControlDialog::OnClose()
{
    DoWhenClosing(false);

    CDialogEx::OnClose();
}

void CFmiBetaProductTabControlDialog::DoWhenClosing(bool calledFromDestructor)
{
    itsSmartMetDocumentInterface->BetaProductionSystem().BetaProductTabControlIndex(itsBetaProductControl.GetSSLActivePage());

    if(!calledFromDestructor)
    {
        itsBetaProductDialogTab.DoWhenClosing();
        itsBetaProductAutomationDialogTab.DoWhenClosing();

        AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
    }
}

void CFmiBetaProductTabControlDialog::StoreControlValuesToDocument()
{
    itsSmartMetDocumentInterface->BetaProductionSystem().BetaProductTabControlIndex(itsBetaProductControl.GetSSLActivePage());
    itsBetaProductDialogTab.DoWhenClosing();
    itsBetaProductAutomationDialogTab.DoWhenClosing();
}

