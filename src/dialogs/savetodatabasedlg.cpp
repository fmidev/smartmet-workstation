// SaveToDataBaseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SaveToDataBaseDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiValueString.h"
#include "NFmiDataLoadingInfo.h"
#include "NFmiStringTools.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiHelpEditorSystem.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiMetEditorOptionsData.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiEditDataUserList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveToDataBaseDlg dialog

int CSaveToDataBaseDlg::itsSelectedUserIndex = -1; // Ohjelman aluksi laitetaan indeksi osoittamaan ohi listan, jotta käyttäjä joutuu tekemään valinnan (siis jos SmartMet käyttää username listaa)

CSaveToDataBaseDlg::CSaveToDataBaseDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
:CDialog(CSaveToDataBaseDlg::IDD, pParent)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsDataLoadingInfo(0)
, itsForecasterIDStrU_(_T("XXX"))
, itsDataCheckMethod(2)
, itsStartTimeWarningStringU_(_T(""))
, fStartTimeWarningOn(false)
, itsUserNameNotSelecterErrorStrU_(_T(""))
, fAddHelperId(FALSE)
{
	//{{AFX_DATA_INIT(CSaveToDataBaseDlg)
    itsTimeStringU_ = _T("");
    itsNameStringU_ = _T("");
	//}}AFX_DATA_INIT

	itsDataLoadingInfo = &(itsSmartMetDocumentInterface->GetUsedDataLoadingInfo());
}



void CSaveToDataBaseDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSaveToDataBaseDlg)
    DDX_Text(pDX, IDC_STATIC_time, itsTimeStringU_);
    DDX_Text(pDX, IDC_STATIC_file, itsNameStringU_);
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_EDIT_FORECASTER_ID, itsForecasterIDStrU_);
    DDX_Radio(pDX, IDC_RADIO_DATA_TO_DB_CHECK_METHOD1, itsDataCheckMethod);
    DDX_Text(pDX, IDC_STATIC_SAVE_DB_START_TIME_WARNING_STR, itsStartTimeWarningStringU_);
    DDX_Control(pDX, IDC_COMBO_USER_NAMES, itsUserNameComboBox);
    DDX_Text(pDX, IDC_STATIC_USER_NAME_NOT_SELECTED_ERROR_TEXT, itsUserNameNotSelecterErrorStrU_);
    DDX_Control(pDX, IDOK, itsAcceptButton);
    DDX_Check(pDX, IDC_CHECK_ADD_HELPER_ID, fAddHelperId);
}


BEGIN_MESSAGE_MAP(CSaveToDataBaseDlg, CDialog)
	//{{AFX_MSG_MAP(CSaveToDataBaseDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_CTLCOLOR()
    ON_CBN_SELCHANGE(IDC_COMBO_USER_NAMES, &CSaveToDataBaseDlg::OnCbnSelchangeComboUserNames)
    ON_BN_CLICKED(IDC_CHECK_ADD_HELPER_ID, &CSaveToDataBaseDlg::OnBnClickedCheckAddHelperId)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveToDataBaseDlg message handlers

NFmiString CSaveToDataBaseDlg::TimeString()
{
	boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->EditedSmartInfo();
	const NFmiTimeDescriptor &timeDesc = info->TimeDescriptor();

    auto language = itsSmartMetDocumentInterface->Language();
	NFmiString timeStr;
	timeStr += timeDesc.FirstTime().ToStr(::GetDictionaryString("SaveDataToDataBaseDlgTimeFormatStr"), language);
	timeStr += " -\n";
	timeStr += timeDesc.LastTime().ToStr(::GetDictionaryString("SaveDataToDataBaseDlgTimeFormatStr"), language);
	return timeStr;
}

NFmiString CSaveToDataBaseDlg::NameString()
{
	NFmiString name("");
	name += itsDataLoadingInfo->AreaIdentifier();
	name += " ";
	name += NFmiStringTools::Convert<int>(itsDataLoadingInfo->ForecastingLength());
	name += ::GetDictionaryString("SaveDataToDataBaseDlgHourStr");
	name += "  ";
	name += ::GetDictionaryString("SaveDataToDataBaseDlgResolutionStr");
	name += " ";
	name += NFmiStringTools::Convert<int>((itsDataLoadingInfo->MaximalCoverageTimeBag().Resolution()/60));
	name += " ";
	name += ::GetDictionaryString("SaveDataToDataBaseDlgHourStr");
	return name;
}

BOOL CSaveToDataBaseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();
    itsNameStringU_ = CA2T(NameString());
    itsTimeStringU_ = CA2T(TimeString());
    itsStartTimeWarningStringU_ = StartTimeWarningString();

	itsDataCheckMethod = itsSmartMetDocumentInterface->DataToDBCheckMethod();
    fAddHelperId = itsSmartMetDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().AddHelpDataIdAtSendindDataToDatabase();

    InitUserNameControls();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSaveToDataBaseDlg::InitUserNameControls()
{
    if(IsUserNameListUsed())
    {
        CFmiWin32Helpers::SetDialogItemVisibility(this, IDC_EDIT_FORECASTER_ID, false);
        CFmiWin32Helpers::SetDialogItemVisibility(this, IDC_COMBO_USER_NAMES, true);
        CFmiWin32Helpers::SetDialogItemVisibility(this, IDC_STATIC_USER_NAME_NOT_SELECTED_ERROR_TEXT, true);

        const std::vector<std::string> &userNameList = itsSmartMetDocumentInterface->EditDataUserList().UserNameList();
        for(const auto &userName : userNameList)
        {
            itsUserNameComboBox.AddString(CA2T(userName.c_str()));
        }
        itsUserNameComboBox.SetCurSel(CSaveToDataBaseDlg::itsSelectedUserIndex);
        CheckUserNameSelection();
    }
    else
    {
        CFmiWin32Helpers::SetDialogItemVisibility(this, IDC_EDIT_FORECASTER_ID, true);
        CFmiWin32Helpers::SetDialogItemVisibility(this, IDC_COMBO_USER_NAMES, false);
        CFmiWin32Helpers::SetDialogItemVisibility(this, IDC_STATIC_USER_NAME_NOT_SELECTED_ERROR_TEXT, false);
    }
}

void CSaveToDataBaseDlg::CheckUserNameSelection()
{
    if(!IsProperUserNameSelectedFromList())
    {
        itsUserNameNotSelecterErrorStrU_ = CA2T(::GetDictionaryString("Select user name!").c_str());
        itsAcceptButton.EnableWindow(FALSE);
    }
    else
    {
        itsUserNameNotSelecterErrorStrU_.Empty();
        itsAcceptButton.EnableWindow(TRUE);
    }
    UpdateData(FALSE);
}

CString CSaveToDataBaseDlg::StartTimeWarningString(void)
{
	NFmiMetTime currentTime(60);
	boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->EditedSmartInfo();
	int hourLimitWarning = 4;
	NFmiMetTime startTime(info->TimeDescriptor().FirstTime());
	std::string tmpStr;
	long diffInHours = currentTime.DifferenceInHours(startTime);
	if(diffInHours > hourLimitWarning)
	{
		fStartTimeWarningOn = true;
		tmpStr += ::GetDictionaryString("SaveDataToDataBaseStartTimeWarning1");
		tmpStr += " ";
		tmpStr += NFmiStringTools::Convert<long>(diffInHours);
		tmpStr += " ";
		tmpStr += ::GetDictionaryString("SaveDataToDataBaseStartTimeWarning2");
	}
	else
	{
		fStartTimeWarningOn = false;
	}
	return CString(CA2T(tmpStr.c_str()));
}

void CSaveToDataBaseDlg::OnOK()
{
	UpdateData(TRUE); // päivittää itsForecasterIDStr:in
	CDialog::OnOK();
    StoreSelectedUserName();
}

void CSaveToDataBaseDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CSaveToDataBaseDlg::InitDialogTexts(void)
{
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SAVE_DB_FORECASTER_ID_STR, "IDC_STATIC_SAVE_DB_FORECASTER_ID_STR");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_GROUP_DATA_TO_DB_CHECK_METHODS, "IDC_GROUP_DATA_TO_DB_CHECK_METHODS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_DATA_TO_DB_CHECK_METHOD1, "IDC_RADIO_DATA_TO_DB_CHECK_METHOD1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_DATA_TO_DB_CHECK_METHOD2, "IDC_RADIO_DATA_TO_DB_CHECK_METHOD2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_DATA_TO_DB_CHECK_METHOD3, "IDC_RADIO_DATA_TO_DB_CHECK_METHOD3");

	// IDC_RADIO_DATA_TO_DB_CHECK_METHOD2 on datan validaatio radio button. Jos systeemi on disabloitu, disabloidaan myös
	// radio buttoni
	CWnd *win = GetDlgItem(IDC_RADIO_DATA_TO_DB_CHECK_METHOD2);
	if(win)
		win->EnableWindow(itsSmartMetDocumentInterface->MetEditorOptionsData().DataValidationEnabled());

	if(itsSmartMetDocumentInterface->HelpEditorSystem().UsedHelpEditorStatus())
	{
        SetWindowText(CA2T(::GetDictionaryString("SaveDataToDataBaseDlgTitleHelpMode").c_str()));
		CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SAVE_DB_STR, "IDC_STATIC_SAVE_DB_STR_HelpMode");
	}
	else
	{
        SetWindowText(CA2T(::GetDictionaryString("SaveDataToDataBaseDlgTitle").c_str()));
		CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SAVE_DB_STR, "IDC_STATIC_SAVE_DB_STR");
	}
}

HBRUSH CSaveToDataBaseDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    auto &helpEditorSystem = itsSmartMetDocumentInterface->HelpEditorSystem();
	if(helpEditorSystem.UsedHelpEditorStatus())
	{
		int id = pWnd->GetDlgCtrlID();
		if(id == IDC_STATIC_SAVE_DB_STR || id == IDC_STATIC_time || id == IDC_STATIC_file)
		{
			NFmiColor color(helpEditorSystem.HelpColor());
			pDC->SetBkMode(OPAQUE);
			pDC->SetBkColor(RGB(color.Red()*255.f, color.Green()*255.f, color.Blue()*255.f));
		}
	}

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_SAVE_DB_START_TIME_WARNING_STR)
	{
		if(fStartTimeWarningOn)
		{
			pDC->SetTextColor(RGB(198, 0, 0));
		}
	}

    if(pWnd->GetDlgCtrlID() == IDC_STATIC_USER_NAME_NOT_SELECTED_ERROR_TEXT)
    {
        pDC->SetTextColor(RGB(198, 0, 0));
    }

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

bool CSaveToDataBaseDlg::IsProperUserNameSelectedFromList() const
{
    return CSaveToDataBaseDlg::itsSelectedUserIndex >= 0;
}

void CSaveToDataBaseDlg::OnCbnSelchangeComboUserNames()
{
    CSaveToDataBaseDlg::itsSelectedUserIndex = itsUserNameComboBox.GetCurSel();
    CheckUserNameSelection();
}

bool CSaveToDataBaseDlg::IsUserNameListUsed() const
{
    return itsSmartMetDocumentInterface->EditDataUserList().UserNameList().size() > 0;
}

void CSaveToDataBaseDlg::StoreSelectedUserName()
{
    if(IsUserNameListUsed())
    {
        if(IsProperUserNameSelectedFromList())
        {
            itsUserNameComboBox.GetLBText(CSaveToDataBaseDlg::itsSelectedUserIndex, itsForecasterIDStrU_);
        }
        else
        {
            itsForecasterIDStrU_ = CA2T(::GetDictionaryString("Invalid-user-name").c_str());
        }
    }
}


void CSaveToDataBaseDlg::OnBnClickedCheckAddHelperId()
{
    UpdateData(TRUE);
    itsSmartMetDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().AddHelpDataIdAtSendindDataToDatabase(fAddHelperId == TRUE);
}
