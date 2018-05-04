// FmiDataValidationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiDataValidationDlg.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiValueString.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiDataValidationDlg dialog


CFmiDataValidationDlg::CFmiDataValidationDlg(NFmiMetEditorOptionsData* theOptionsData, CWnd* pParent /*=NULL*/)
:CDialog(CFmiDataValidationDlg::IDD, pParent)
,itsOptionsData(theOptionsData)
{
	//{{AFX_DATA_INIT(CFmiDataValidationDlg)
	fDataValidation_PrForm_T = FALSE;
	fDataValidation_T_DP = FALSE;
	fMakeDataValidationAtSendingDB = FALSE;
    itsSnowLimitStrU_ = _T("");
    itsRainLimitStrU_ = _T("");
	fMakeValidationsAutomatically = FALSE;
	fUseLargeScalePrecip = FALSE;
	//}}AFX_DATA_INIT
}


void CFmiDataValidationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiDataValidationDlg)
	DDX_Check(pDX, IDC_CHECK_DATA_VALIDATION_PRFORM_T, fDataValidation_PrForm_T);
	DDX_Check(pDX, IDC_CHECK_DATA_VALIDATION_T_DP, fDataValidation_T_DP);
	DDX_Check(pDX, IDC_CHECK_MAKE_VALIDATIONS_AT_SENDING_TO_DB, fMakeDataValidationAtSendingDB);
    DDX_Text(pDX, IDC_EDIT_SNOW_LIMIT, itsSnowLimitStrU_);
    DDV_MaxChars(pDX, itsSnowLimitStrU_, 5);
    DDX_Text(pDX, IDC_EDIT_RAIN_LIMIT, itsRainLimitStrU_);
	DDX_Check(pDX, IDC_CHECK_MAKE_VALIDATIONS_AUTOMATICALLY, fMakeValidationsAutomatically);
	DDX_Check(pDX, IDC_CHECK_PRECIPITATION_TYPE_LARGE_SCALE, fUseLargeScalePrecip);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiDataValidationDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiDataValidationDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiDataValidationDlg message handlers

void CFmiDataValidationDlg::OnOK() 
{
	SetDocValues();
	CDialog::OnOK();
}

void CFmiDataValidationDlg::SetDocValues(void)
{
	UpdateData(TRUE);
	itsOptionsData->UseDataValiditation_PrForm_T(fDataValidation_PrForm_T == TRUE);
	itsOptionsData->UseDataValiditation_T_DP(fDataValidation_T_DP == TRUE);
	itsOptionsData->MakeDataValiditationAtSendingToDB(fMakeDataValidationAtSendingDB == TRUE);
	itsOptionsData->MakeDataValiditationAutomatic(fMakeValidationsAutomatically == TRUE);
	itsOptionsData->DefaultPrecipitationTypeIsLargeScale(fUseLargeScalePrecip == TRUE);

    NFmiValueString str = CT2A(itsSnowLimitStrU_);
	if(str.IsNumeric())
		itsOptionsData->SnowTemperatureLimit(float(str));
	else
		itsOptionsData->SnowTemperatureLimit(kFloatMissing);

    NFmiValueString str2 = CT2A(itsRainLimitStrU_);
	if(str2.IsNumeric())
		itsOptionsData->RainTemperatureLimit(float(str2));
	else
		itsOptionsData->RainTemperatureLimit(kFloatMissing);
}

BOOL CFmiDataValidationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	InitDialogTexts();
	fDataValidation_PrForm_T = itsOptionsData->UseDataValiditation_PrForm_T();
	fDataValidation_T_DP = itsOptionsData->UseDataValiditation_T_DP();
	fMakeDataValidationAtSendingDB = itsOptionsData->MakeDataValiditationAtSendingToDB();
	fMakeValidationsAutomatically = itsOptionsData->MakeDataValiditationAutomatic();
	fUseLargeScalePrecip = itsOptionsData->DefaultPrecipitationTypeIsLargeScale();

	NFmiValueString str(itsOptionsData->SnowTemperatureLimit(),"%5.1f");
    itsSnowLimitStrU_ = CA2T(str);

	NFmiValueString str2(itsOptionsData->RainTemperatureLimit(),"%5.1f");
    itsRainLimitStrU_ = CA2T(str2);
	
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiDataValidationDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("Data validation setup").c_str()));

	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "Accept");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "Cancel");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_MAKE_VALIDATIONS_AT_SENDING_TO_DB, "Make validations when sending data to DB");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_MAKE_VALIDATIONS_AUTOMATICALLY, "Make validations automatically");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_PRECIPITATION_TYPE_LARGE_SCALE, "Default added precipitation type large scale");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GROUP, "Used validations");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SNOW_LIMIT, "Snow limit [°C]");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_WATER_LIMIT, "Water limit [°C]");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DATA_VALIDATION_PRFORM_T, "Precip. form - temperature");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DATA_VALIDATION_T_DP, "Temperature greater than dew point");
}
