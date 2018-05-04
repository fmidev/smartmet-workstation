// FmiWarningMessageSymbolOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiWarningMessageSymbolOptionsDlg.h"
#include "WarningSymbolInfo.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewWin32Functions.h"
#include "HakeMessage/MessageTypeInfo.h"

// CFmiWarningMessageSymbolOptionsDlg dialog

IMPLEMENT_DYNAMIC(CFmiWarningMessageSymbolOptionsDlg, CDialog)
CFmiWarningMessageSymbolOptionsDlg::CFmiWarningMessageSymbolOptionsDlg(HakeLegacySupport::MessageTypeInfo &theMessageTypeInfo, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiWarningMessageSymbolOptionsDlg::IDD, pParent)
	, itsOriginalMessageTypeInfoReference(theMessageTypeInfo)
    , fMessageTypeInfoShow(FALSE)
    , itsMessageTypeInfoCategoryId(0)
    , itsMessageTypeInfoCategoryDescription(_T(""))
    , itsMessageTypeSelector()
{
}

void CFmiWarningMessageSymbolOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_WARNING_MESSAGE_INFO_SHOW, fMessageTypeInfoShow);
    DDX_Text(pDX, IDC_EDIT_WARNING_MESSAGE_INFO_CATEGORY, itsMessageTypeInfoCategoryId);
    DDX_Text(pDX, IDC_EDIT_WARNING_MESSAGE_INFO_CATEGORY_DESCRIPTION, itsMessageTypeInfoCategoryDescription);
    DDX_Control(pDX, IDC_COMBO_WARNING_MESSAGE_INFO_TYPE_SELECTOR, itsMessageTypeSelector);
}


BEGIN_MESSAGE_MAP(CFmiWarningMessageSymbolOptionsDlg, CDialog)
END_MESSAGE_MAP()

// CFmiWarningMessageSymbolOptionsDlg message handlers

void CFmiWarningMessageSymbolOptionsDlg::OnOK()
{
	UpdateData(TRUE);

    // Painettu Ok:ta, otetaan muutokset talteen originaaliin olioon muokatusta muuttujasta.
    StoreMessageTypeInfoFromControls(itsOriginalMessageTypeInfoReference);

	CDialog::OnOK();
}

BOOL CFmiWarningMessageSymbolOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    itsMessageTypeSelector.InsertString(0, _TEXT("Hake"));
    itsMessageTypeSelector.InsertString(1, _TEXT("KaHa"));

    InitializeFromMessageTypeInfo(itsOriginalMessageTypeInfoReference);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiWarningMessageSymbolOptionsDlg::InitializeFromMessageTypeInfo(HakeLegacySupport::MessageTypeInfo &theMessageTypeInfo)
{
    fMessageTypeInfoShow = theMessageTypeInfo.show();
    itsMessageTypeInfoCategoryId = theMessageTypeInfo.messageCategory();
    itsMessageTypeInfoCategoryDescription = CA2T(theMessageTypeInfo.messageCategoryName().c_str());
    if(theMessageTypeInfo.messageType() == HakeLegacySupport::kKahaMessage)
        itsMessageTypeSelector.SetCurSel(1);
    else
        itsMessageTypeSelector.SetCurSel(0);
    UpdateData(FALSE);
}

void CFmiWarningMessageSymbolOptionsDlg::StoreMessageTypeInfoFromControls(HakeLegacySupport::MessageTypeInfo &theMessageTypeInfo)
{
    UpdateData(TRUE);
    theMessageTypeInfo.setShow(fMessageTypeInfoShow == TRUE);
    theMessageTypeInfo.setMessageCategory(itsMessageTypeInfoCategoryId);
    theMessageTypeInfo.setMessageCategoryName(std::string(CT2A(itsMessageTypeInfoCategoryDescription)));
    if(itsMessageTypeSelector.GetCurSel() == 0)
        theMessageTypeInfo.setMessageType(HakeLegacySupport::kHakeMessage);
    else
        theMessageTypeInfo.setMessageType(HakeLegacySupport::kKahaMessage);
}
