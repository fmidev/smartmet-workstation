// FmiWarningMessageOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiWarningMessageOptionsDlg.h"
#include "HakeMessage/Main.h"
#include "WarningSymbolInfo.h"
#include "HakeMessage/HakeSystemConfigurations.h"
#include "FmiWarningMessageSymbolOptionsDlg.h"
#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include <direct.h>
#include "CtrlViewGdiPlusFunctions.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "persist2.h"


// CFmiWarningMessageOptionsDlg dialog

const NFmiViewPosRegistryInfo CFmiWarningMessageOptionsDlg::s_ViewPosRegistryInfo(CRect(320, 130, 700, 670), "\\WarningMessageOptionsDlg");

IMPLEMENT_DYNAMIC(CFmiWarningMessageOptionsDlg, CDialog)
CFmiWarningMessageOptionsDlg::CFmiWarningMessageOptionsDlg(HakeMessage::Main *theWarningCenterSystem, NFmiApplicationWinRegistry &theApplicationWinRegistry, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiWarningMessageOptionsDlg::IDD, pParent)
	,itsWarningCenterSystem(theWarningCenterSystem)
    ,itsApplicationWinRegistry(theApplicationWinRegistry)
{
}

CFmiWarningMessageOptionsDlg::~CFmiWarningMessageOptionsDlg()
{
}

void CFmiWarningMessageOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_MESSAG_TYPE_LIST, itsMessageTypeList);
    DDX_Control(pDX, IDC_CHECK_WARNING_OPTIONS_CHECK_ALL, itsCheckAllControl);
}


BEGIN_MESSAGE_MAP(CFmiWarningMessageOptionsDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_ADD, &CFmiWarningMessageOptionsDlg::OnBnClickedButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CFmiWarningMessageOptionsDlg::OnBnClickedButtonRemove)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTON_EDIT, &CFmiWarningMessageOptionsDlg::OnBnClickedButtonEdit)
    ON_WM_GETMINMAXINFO()
    ON_BN_CLICKED(IDC_CHECK_WARNING_OPTIONS_CHECK_ALL, &CFmiWarningMessageOptionsDlg::OnBnClickedCheckWarningOptionsCheckAll)
END_MESSAGE_MAP()

void CFmiWarningMessageOptionsDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiWarningMessageOptionsDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsApplicationWinRegistry, MakeUsedWinRegistryKeyStr(0), this);
}

// CFmiWarningMessageOptionsDlg message handlers

BOOL CFmiWarningMessageOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    DoResizerHooking(); // Tätä pitää kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisteristä

    std::string errorBaseStr("Error in CFmiWarningMessageOptionsDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsApplicationWinRegistry, this, false, errorBaseStr, 0);

    itsCheckAllControl.SetCheck(FALSE);
    LoadMessageTypeInfoFromWarningCenterSystem();
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiWarningMessageOptionsDlg::InitDialogTexts()
{
    SetWindowText(CA2T(::GetDictionaryString("Warning message options").c_str()));
    CFmiWin32Helpers::SetDialogItemText(this, IDOK, "Ok");
    CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "Cancel");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_ADD, "Add");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_EDIT, "Edit");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REMOVE, "Remove");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_WARNING_OPTIONS_CHECK_ALL, "Check all");
}

void CFmiWarningMessageOptionsDlg::DoResizerHooking()
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDOK, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDCANCEL, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_WARNING_OPTIONS_CHECK_ALL, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_EDIT, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_REMOVE, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    // CheckListControl takes rest of the view
    bOk = m_resizer.SetAnchor(IDC_LIST_MESSAG_TYPE_LIST, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
}

void CFmiWarningMessageOptionsDlg::OnOK()
{
    UpdateData(TRUE);
    StoreMessageTypeInfoToWarningCenterSystem();
    DoWhenClosing();

    CDialog::OnOK();
}

void CFmiWarningMessageOptionsDlg::OnCancel()
{
    DoWhenClosing();

    CDialog::OnCancel();
}


void CFmiWarningMessageOptionsDlg::OnClose()
{
    DoWhenClosing();

    CDialog::OnClose();
}

void CFmiWarningMessageOptionsDlg::DoWhenClosing()
{
    // Suljettaessa laitetaan dialogin koko ja sijainti talteen
    Persist2::WriteWindowRectToWinRegistry(itsApplicationWinRegistry, this->MakeUsedWinRegistryKeyStr(0), this);
}

#ifdef min
#undef min
#endif

void CFmiWarningMessageOptionsDlg::StoreMessageTypeInfoToWarningCenterSystem()
{
#ifndef DISABLE_CPPRESTSDK
    auto &messageTypeInfovector = itsWarningCenterSystem->getLegacyData().MessageTypeInfos();
    int totalCount = std::min(static_cast<int>(messageTypeInfovector.size()), itsMessageTypeList.GetCount());
    for(int index = 0; index < totalCount; index++)
    {
        messageTypeInfovector[index].setShow(itsMessageTypeList.GetCheck(index) == TRUE);
    }

    itsWarningCenterSystem->getLegacyData().StoreSettings();
#endif // DISABLE_CPPRESTSDK
}

static CString MakeMessageTypeInfoString(const HakeLegacySupport::MessageTypeInfo &messageTypeInfo)
{
    std::string str = std::to_string(messageTypeInfo.messageCategory());
    str += "   (";
    str += messageTypeInfo.messageCategoryName();
    str += ")";
    str += "   [";
    if(messageTypeInfo.messageType() == HakeLegacySupport::kHakeMessage)
        str += "Hake";
    else
        str += "KaHa";
    str += "]";

    return CString(CA2T(str.c_str()));
}

void CFmiWarningMessageOptionsDlg::LoadMessageTypeInfoFromWarningCenterSystem()
{
#ifndef DISABLE_CPPRESTSDK
    itsMessageTypeList.ResetContent();
    int counter = 0;
    const auto &messageTypeInfos = itsWarningCenterSystem->getLegacyData().MessageTypeInfos();
    for(const auto &messageTypeInfo : messageTypeInfos)
    {
        itsMessageTypeList.AddString(::MakeMessageTypeInfoString(messageTypeInfo));
        itsMessageTypeList.SetCheck(counter++, messageTypeInfo.show());
    }

    UpdateData(FALSE);
#endif // DISABLE_CPPRESTSDK
}

void CFmiWarningMessageOptionsDlg::OnBnClickedButtonAdd()
{
#ifndef DISABLE_CPPRESTSDK
    HakeLegacySupport::MessageTypeInfo messageTypeInfo;
    CFmiWarningMessageSymbolOptionsDlg dlg(messageTypeInfo, this);
    if(dlg.DoModal() == IDOK)
    {
        itsWarningCenterSystem->getLegacyData().AddMessageTypeInfo(messageTypeInfo);
        LoadMessageTypeInfoFromWarningCenterSystem();
    }
#endif // DISABLE_CPPRESTSDK
}

void CFmiWarningMessageOptionsDlg::OnBnClickedButtonRemove()
{
#ifndef DISABLE_CPPRESTSDK
    // Jos indeksi on -1, listalta ei ole valittuna mitään, eikä tarvitse tehdä mitään.
    if(itsMessageTypeList.GetCurSel() >= 0)
    {
        itsWarningCenterSystem->getLegacyData().RemoveMessageTypeInfo(itsMessageTypeList.GetCurSel());
        LoadMessageTypeInfoFromWarningCenterSystem();
    }
#endif // DISABLE_CPPRESTSDK
}

void CFmiWarningMessageOptionsDlg::OnBnClickedButtonEdit()
{
#ifndef DISABLE_CPPRESTSDK
    // Jos indeksi on -1, listalta ei ole valittuna mitään, eikä tarvitse tehdä mitään.
    if(itsMessageTypeList.GetCurSel() >= 0)
    {
        auto messageTypeInfoPtr = itsWarningCenterSystem->getLegacyData().GetMessageTypeInfo(itsMessageTypeList.GetCurSel());
        if(messageTypeInfoPtr)
        {
            CFmiWarningMessageSymbolOptionsDlg dlg(*messageTypeInfoPtr, this);
            if(dlg.DoModal() == IDOK)
            {
                // Sortataan sanomat infot, jotta ne ovat aina oikeassa järjestyksessä
                itsWarningCenterSystem->getLegacyData().SortMessageTypeInfoVector();
                LoadMessageTypeInfoFromWarningCenterSystem();
            }
        }
    }
#endif // DISABLE_CPPRESTSDK
}


void CFmiWarningMessageOptionsDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    lpMMI->ptMinTrackSize.x = 410;
    lpMMI->ptMinTrackSize.y = 340;

    CDialog::OnGetMinMaxInfo(lpMMI);
}


void CFmiWarningMessageOptionsDlg::OnBnClickedCheckWarningOptionsCheckAll()
{
    UpdateData(TRUE);
    auto checkState = itsCheckAllControl.GetCheck();
    for(int i = 0; i < itsMessageTypeList.GetCount(); i++)
        itsMessageTypeList.SetCheck(i, checkState);
    if(checkState)
        CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_WARNING_OPTIONS_CHECK_ALL, "Uncheck all");
    else
        CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_WARNING_OPTIONS_CHECK_ALL, "Check all");
    UpdateData(FALSE);
}
