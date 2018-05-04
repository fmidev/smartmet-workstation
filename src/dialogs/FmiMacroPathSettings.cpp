#include "stdafx.h"
#include "FmiMacroPathSettings.h"
#include <direct.h>
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "SmartMetDocumentInterface.h"
#include "CloneBitmap.h"
#include "XFolderDialog.h"

// CFmiMacroPathSettings dialog

IMPLEMENT_DYNAMIC(CFmiMacroPathSettings, CDialog)
CFmiMacroPathSettings::CFmiMacroPathSettings(SmartMetDocumentInterface *smartMetDocumentInterface,
						const std::string &theSmartToolPathStr,
						const std::string &theViewMacroPathStr,
						const std::string &theMacroParamPathStr,
						const std::string &theDrawParamPathStr,
						const std::string &theLoggerPathStr,
						CWnd* pParent)
	: CDialog(CFmiMacroPathSettings::IDD, pParent)
	, itsSmartToolPathStrU_(CA2T(theSmartToolPathStr.c_str()))
    , itsViewMacroPathStrU_(CA2T(theViewMacroPathStr.c_str()))
    , itsMacroParamPathStrU_(CA2T(theMacroParamPathStr.c_str()))
    , itsDrawParamPathStrU_(CA2T(theDrawParamPathStr.c_str()))
    , itsLoggerPathStrU_(CA2T(theLoggerPathStr.c_str()))
	, itsSmartMetDocumentInterface(smartMetDocumentInterface)
{
}

CFmiMacroPathSettings::~CFmiMacroPathSettings()
{
}

void CFmiMacroPathSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SMART_TOOLS_PATH, itsSmartToolPathStrU_);
    DDX_Text(pDX, IDC_EDIT_VIEW_MACRO_PATH, itsViewMacroPathStrU_);
    DDX_Text(pDX, IDC_EDIT_MACRO_PARAM_PATH, itsMacroParamPathStrU_);
    DDX_Text(pDX, IDC_EDIT_DRAW_PARAM_PATH, itsDrawParamPathStrU_);
    DDX_Text(pDX, IDC_EDIT_LOGGER_PATH, itsLoggerPathStrU_);
	DDX_Control(pDX, IDC_COMBO_CHANGE_DRIVE_LETTER, itsDriveLetterCombo);
}


BEGIN_MESSAGE_MAP(CFmiMacroPathSettings, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_SMART_TOOLS_BROWSE, OnBnClickedButtonSmartToolsBrowse)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_MACRO_BROWSE, OnBnClickedButtonViewMacroBrowse)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_PARAM_BROWSE, OnBnClickedButtonMacroParamBrowse)
	ON_BN_CLICKED(IDC_BUTTON_DRAW_PARAM_BROWSE, OnBnClickedButtonDrawParamBrowse)
	ON_BN_CLICKED(IDC_BUTTON_LOGGER_BROWSE, OnBnClickedButtonLoggerBrowse)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_DRIVE_LETTER_NOW, &CFmiMacroPathSettings::OnBnClickedButtonChangeDriveLetterNow)
END_MESSAGE_MAP()


// CFmiMacroPathSettings message handlers

void CFmiMacroPathSettings::OnBnClickedOk()
{
	UpdateData(TRUE);
	
	OnOK();
}

void CFmiMacroPathSettings::OnBnClickedButtonSmartToolsBrowse()
{
    Browse(itsSmartToolPathStrU_);
	UpdateData(FALSE);
}

void CFmiMacroPathSettings::OnBnClickedButtonViewMacroBrowse()
{
    Browse(itsViewMacroPathStrU_);
	UpdateData(FALSE);
}

void CFmiMacroPathSettings::OnBnClickedButtonMacroParamBrowse()
{
    Browse(itsMacroParamPathStrU_);
	UpdateData(FALSE);
}

void CFmiMacroPathSettings::OnBnClickedButtonDrawParamBrowse()
{
    Browse(itsDrawParamPathStrU_);
	UpdateData(FALSE);
}

void CFmiMacroPathSettings::OnBnClickedButtonLoggerBrowse()
{
    Browse(itsLoggerPathStrU_);
	UpdateData(FALSE);
}

void CFmiMacroPathSettings::InitDriveLetterCombo(void)
{
	itsDriveLetterCombo.Clear();
    CString driveLetterU_ = _TEXT("A:");
	for(int i = 0; i <= 25; i++)
	{
        itsDriveLetterCombo.AddString(driveLetterU_);
        TCHAR ch = driveLetterU_.GetAt(0);
        driveLetterU_.SetAt(0, ++ch);
	}
	itsDriveLetterCombo.SetCurSel(2); // laitetaan C-asema default arvoksi
}

BOOL CFmiMacroPathSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();
	InitDriveLetterCombo();

	CWnd *buttonWnd = GetDlgItem(IDC_BUTTON_CHANGE_DRIVE_LETTER_NOW);
	m_tooltip.Create(this);
	m_tooltip.SetDelayTime(PPTOOLTIP_TIME_AUTOPOP, 30000); // kuinka kauan tooltippi viipyy, jos kursoria ei liikuteta [ms]
	m_tooltip.SetDelayTime(PPTOOLTIP_TIME_INITIAL, 500); // kuinka nopeasti tooltip ilmestyy näkyviin, jos kursoria ei liikuteta [ms]
	if(buttonWnd)
		m_tooltip.AddTool(buttonWnd, _TEXT("This will change drive letter of couple of SmartMet settings paths.\nThis includes logger's log-file path and working files path.\nChanges will be made when you press the button and confirm the change.\nDon't press this if you don't know what you are doing!"));
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiMacroPathSettings::Browse(CString & thePathU_)
{
    CXFolderDialog dlg(thePathU_);
    dlg.SetTitle(CA2T(::GetDictionaryString("Select folder").c_str()));
    if(dlg.DoModal() == IDOK)
        thePathU_ = dlg.GetPath();
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiMacroPathSettings::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("MacroPathSettingDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PATH_SMARTTOOL_STR, "IDC_STATIC_MACRO_PATH_SMARTTOOL_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PATH_VIEWMACRO_STR, "IDC_STATIC_MACRO_PATH_VIEWMACRO_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PATH_MACROPARAM_STR, "IDC_STATIC_MACRO_PATH_MACROPARAM_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PATH_DRAWPARAM_STR, "IDC_STATIC_MACRO_PATH_DRAWPARAM_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PATH_LOGGER_STR, "IDC_STATIC_MACRO_PATH_LOGGER_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SMART_TOOLS_BROWSE, "IDC_BUTTON_SMART_TOOLS_BROWSE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_VIEW_MACRO_BROWSE, "IDC_BUTTON_VIEW_MACRO_BROWSE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_MACRO_PARAM_BROWSE, "IDC_BUTTON_MACRO_PARAM_BROWSE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_DRAW_PARAM_BROWSE, "IDC_BUTTON_DRAW_PARAM_BROWSE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOGGER_BROWSE, "IDC_BUTTON_LOGGER_BROWSE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PATH_SETTING_WARNING_STR, "IDC_STATIC_MACRO_PATH_SETTING_WARNING_STR");
}

void CFmiMacroPathSettings::OnBnClickedButtonChangeDriveLetterNow()
{
    CString tmpStrU_;
    itsDriveLetterCombo.GetLBText(itsDriveLetterCombo.GetCurSel(), tmpStrU_);
    std::string tmp = CT2A(tmpStrU_);
	itsSmartMetDocumentInterface->ChangeDriveLetterInSettings(tmp);

	UpdateData(FALSE);
}

BOOL CFmiMacroPathSettings::PreTranslateMessage(MSG* pMsg)
{
	m_tooltip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}
