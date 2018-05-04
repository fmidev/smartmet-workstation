// FmiTempBalloonSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTempBalloonSettingsDlg.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"


// CFmiTempBalloonSettingsDlg dialog

IMPLEMENT_DYNAMIC(CFmiTempBalloonSettingsDlg, CDialog)
CFmiTempBalloonSettingsDlg::CFmiTempBalloonSettingsDlg(NFmiTempBalloonTrajectorSettings &theSettings, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiTempBalloonSettingsDlg::IDD, pParent)
	,itsSettings(theSettings)
	, itsRisingSpeed(0)
	, itsFallSpeed(0)
	, itsTopHeightInKM(0)
	, itsFloatingTimeInMinutes(0)
{
}

CFmiTempBalloonSettingsDlg::~CFmiTempBalloonSettingsDlg()
{
}

void CFmiTempBalloonSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_RISING_SPEED, itsRisingSpeed);
	DDX_Text(pDX, IDC_EDIT_FALLING_SPEED, itsFallSpeed);
	DDX_Text(pDX, IDC_EDIT_TOP_HEIGHT_IN_KM, itsTopHeightInKM);
	DDX_Text(pDX, IDC_EDIT_FLOATING_TIME_IN_MINUTES, itsFloatingTimeInMinutes);
}


BEGIN_MESSAGE_MAP(CFmiTempBalloonSettingsDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CFmiTempBalloonSettingsDlg message handlers

void CFmiTempBalloonSettingsDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	itsSettings.RisingSpeed(itsRisingSpeed);
	itsSettings.FallSpeed(itsFallSpeed);
	itsSettings.TopHeightInKM(itsTopHeightInKM);
	itsSettings.FloatingTimeInMinutes(itsFloatingTimeInMinutes);

	OnOK();
}

BOOL CFmiTempBalloonSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// tämä asettaa FMI tunnuksen (IL -pallukka joka on talletettu IDR_MAINFRAME2 -nimiseksi ikoniksi resursseihim)
//	HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME2));
//	this->SetIcon(hIcon, FALSE);

	itsRisingSpeed = itsSettings.RisingSpeed();
	itsFallSpeed = itsSettings.FallSpeed();
	itsTopHeightInKM = itsSettings.TopHeightInKM();
	itsFloatingTimeInMinutes = itsSettings.FloatingTimeInMinutes();
	InitDialogTexts();
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiTempBalloonSettingsDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("CFmiTempBalloonSettingsDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SOUNDING_BALLOON_RISE_SPEED_STR, "IDC_STATIC_SOUNDING_BALLOON_RISE_SPEED_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SOUNDING_BALLOON_FALL_SPEED_STR, "IDC_STATIC_SOUNDING_BALLOON_FALL_SPEED_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SOUNDING_BALLOON_HEIGHT_STR, "IDC_STATIC_SOUNDING_BALLOON_HEIGHT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SOUNDING_BALLOON_FLOAT_TIME_STR, "IDC_STATIC_SOUNDING_BALLOON_FLOAT_TIME_STR");
}
