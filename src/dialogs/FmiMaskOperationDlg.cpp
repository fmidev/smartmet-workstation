#include "stdafx.h"
#include "FmiMaskOperationDlg.h"

#include "NFmiAreaMask.h"
#include "NFmiParam.h"
#include "NFmiStringList.h"
#include "NFmiValueString.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
/////////////////////////////////////////////////////////////////////////////
// CFmiMaskOperationDlg dialog


CFmiMaskOperationDlg::CFmiMaskOperationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFmiMaskOperationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFmiMaskOperationDlg)
	itsRadioButtons = -1;
    itsFirstLimitU_ = _TEXT("");
    itsSecondLimitU_ = _TEXT("");
    itsParameterNameU_ = _TEXT("");
    itsLimitsHeaderU_ = _TEXT("");
	//}}AFX_DATA_INIT
	itsMask=0;
    itsAbbreviationU_ = _TEXT("");
}

CFmiMaskOperationDlg::~CFmiMaskOperationDlg()
{
}

void CFmiMaskOperationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiMaskOperationDlg)
	DDX_Control(pDX, IDC_STATIC_MASK_REMARK, itsRemarkCtrl);
	DDX_Control(pDX, IDC_EDIT_RAJA_2, itsEditCtrlOfSecondLimit);
	DDX_Control(pDX, IDC_EDIT_RAJA_1, itsEditCtrlOfFirstLimit);
    DDX_Text(pDX, IDC_EDIT_RAJA_1, itsFirstLimitU_);
    DDX_Text(pDX, IDC_EDIT_RAJA_2, itsSecondLimitU_);
    DDX_Text(pDX, IDC_STATIC_PARA_NAME, itsParameterNameU_);
    DDX_Text(pDX, IDC_STATIC_MASK_LIMITATIONS_STR, itsLimitsHeaderU_);
	DDX_Radio(pDX, IDC_RADIO_MASK_CONDITIONAL1, itsRadioButtons);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiMaskOperationDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiMaskOperationDlg)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL8, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL9, OnRadio9)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL10, OnRadio10)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL11, OnRadio11)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL12, OnRadio12)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL13, OnRadio13)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL14, OnRadio14)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL15, OnRadio15)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL16, OnRadio17)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL17, OnRadio18)
	ON_BN_CLICKED(IDC_RADIO_MASK_CONDITIONAL18, OnRadio19)
	ON_EN_CHANGE(IDC_EDIT_RAJA_1, OnChangeEditRaja1)
	ON_EN_CHANGE(IDC_EDIT_RAJA_2, OnChangeEditRaja2)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

BOOL CFmiMaskOperationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();
	itsRadioButtons = 0;
	if(itsMask)
	{
 		Limit(itsMask->LowerLimit(), itsMask->UpperLimit());
		
		itsRadioButtons = int(itsMask->MaskOperation());
        itsAbbreviationU_ = itsParameterNameU_;
	}
	UpdateStateOfEditCtrl(FALSE);
	SetRemarkText();
	SetHeaderOfLimits();
	UpdateRampBitmaps();
 	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiMaskOperationDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
 	itsRemarkCtrl.GetClientRect(rect);
 	CPaintDC remarkDC(&itsRemarkCtrl);    // piirret‰‰n teksti Static-ikkunaan
	remarkDC.SetTextColor(RGB(250,10,5)); // punainen teksti
	remarkDC.SetBkColor(dc.GetBkColor()); // tekstin taustav‰riksi asetetaan dialogin (piirtopinta dc) taustav‰ri	
    remarkDC.DrawText(itsRemarkTextU_, rect, DT_CENTER | DT_WORDBREAK);
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CFmiMaskOperationDlg::OnOK()
{
	UpdateData(TRUE);
	if(FirstLimit() != kFloatMissing)
		itsMask->LowerLimit(FirstLimit());
	if(SecondLimit() != kFloatMissing)
		itsMask->UpperLimit(SecondLimit());
	itsMask->MaskOperation(MaskOperation());
	CDialog::OnOK();
}

//////// CFmiMaskOperationDlg message handlers ///////////

void CFmiMaskOperationDlg::OnChangeEditRaja1() 
{
	UpdateRemarkText(TRUE);
	UpdateHeaderOfLimits();
}

void CFmiMaskOperationDlg::OnChangeEditRaja2() 
{
	UpdateRemarkText(FALSE);
	UpdateHeaderOfLimits();
}

void CFmiMaskOperationDlg::OnRadio1() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio2() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio3() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio4() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio5() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio6() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio7() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio8() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio9() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio10() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio11() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio12() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio13() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio14() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio15() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio17() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio18() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::OnRadio19() 
{
	UpdateDialogAfterMaskChanged();
}

void CFmiMaskOperationDlg::UpdateDialogAfterMaskChanged(void)
{
	UpdateStateOfEditCtrl();	
	UpdateHeaderOfLimits();
	UpdateRemarkText();
	UpdateRampBitmaps();
}

////////////////////// public methods //////////////////////

void CFmiMaskOperationDlg::ParamMask(NFmiAreaMask* theMask)
{
// Set-metodi NFmiParamMask-attribuutin asetukseen. T‰m‰ t‰ytyy tehd‰ 
// erikseen dialogin luonnin j‰lkeen ennen kuin dialogi n‰ytet‰‰n k‰ytt‰j‰lle.
	itsMask = theMask;
}

FmiMaskOperation CFmiMaskOperationDlg::MaskOperation()
{
// Palauttaa dialogin FmiMaskOperation-parametrinimen.
// Koska radiobuttonit ovat enum-m‰‰rittelyn FmiMaskOperation
// mukaisessa j‰rjestyksess‰, voidaan itsRadioButtons-arvo
// castata suoraan palautettavaksi FmiMaskOperation-arvoksi
	return (FmiMaskOperation)itsRadioButtons;
}

double CFmiMaskOperationDlg::FirstLimit()
{
// Konvertoi CEdit-ruudun CString-tekstin itsFirstLimit 
// double-luvuksi ja palauttaa luvun k‰ytt‰j‰lle.
	double value = kFloatMissing;

    CString tmpU_(itsFirstLimitU_);
    int num = tmpU_.Replace(',', '.'); // ihmiset kirjoittavat helposti ,:n .:een sijasta (piste pit‰‰ olla)
    NFmiValueString aValueStr = CT2A(tmpU_);
	aValueStr.ConvertToDouble(value);

	return value;
}

double CFmiMaskOperationDlg::SecondLimit()
{
// Konvertoi CEdit-ruudun CString-tekstin itsSecondtLimit 
// double-luvuksi ja palauttaa luvun k‰ytt‰j‰lle.
	double value = kFloatMissing;

    CString tmpU_(itsSecondLimitU_);
    int num = tmpU_.Replace(',', '.'); // ihmiset kirjoittavat helposti ,:n .:een sijasta (piste pit‰‰ olla)
    NFmiValueString aValueStr = CT2A(tmpU_);
	aValueStr.ConvertToDouble(value);

	return double(value);
}

CString CFmiMaskOperationDlg::FirstLimit(int /* dummy */ )
{
    return itsFirstLimitU_;
}

CString CFmiMaskOperationDlg::SecondLimit(int /* dummy */ )
{
    return itsSecondLimitU_;
}


////////////////////// protected methods //////////////////////

void CFmiMaskOperationDlg::UpdateStateOfEditCtrl(BOOL updateFromDlg)
{
// P‰ivitt‰‰ dialogin CEdit-ruutujen tilan Enable/Disable. Ruutujen
// enablointi/disablointi m‰‰r‰ytyy siit‰, mik‰ radiobuttoni on valittu.
// Jos tunnistin BOOL updateFromDlg on TRUE (oletusarvo), p‰ivitet‰‰n  
// ohjelman k‰ytt‰j‰n valinta dialogista attribuuttiin itsRadioButtons.
// Mik‰li updateFromDlg == FALSE, p‰ivityst‰ ei tehd‰ - t‰t‰ toimintaa
// tarvitaan dialogin alustuksen yhteydess‰ metodissa OnInitDialog().

	if(updateFromDlg)
		UpdateData(TRUE);

	if(itsRadioButtons <= 0)
	{
		itsEditCtrlOfFirstLimit.EnableWindow(FALSE);
		itsEditCtrlOfSecondLimit.EnableWindow(FALSE);
	}
	if(itsRadioButtons > 0 && itsRadioButtons < 6)
	{
		itsEditCtrlOfFirstLimit.EnableWindow(TRUE);
		itsEditCtrlOfSecondLimit.EnableWindow(FALSE);
	}
	else if(itsRadioButtons >= 6 )
	{
		itsEditCtrlOfFirstLimit.EnableWindow(TRUE);
		itsEditCtrlOfSecondLimit.EnableWindow(TRUE);	
	}
}

void CFmiMaskOperationDlg::Limit(double firstLimit, double secondLimit)
{
// Asetetaan dialogin CEdit-ikkunoiden itsFirstLimit ja itsSecondLimit tekstit.
// T‰t‰ metodia k‰ytet‰‰n dialogin alustuksessa OnInitDialog():ssa.

    itsFirstLimitU_ = CA2T(NFmiValueString::GetStringWithMaxDecimalsSmartWay(firstLimit, 4));
    itsSecondLimitU_ = CA2T(NFmiValueString::GetStringWithMaxDecimalsSmartWay(secondLimit, 4));
}

void CFmiMaskOperationDlg::SetHeaderOfLimits()
{
// Asetetaan otsaketeksti dialogin yl‰reunassa olevaan CStatic-ikkunaan itsLimitsHeader. 
// Siin‰ n‰kyy sanallisesti millainen rajoitus parametrille on valittu radiobuttoneista.

    itsAbbreviationU_ = itsMask->Param() ? CA2T(itsMask->Param()->GetName()) : _TEXT("?");
	switch( itsRadioButtons )
	{
		case kFmiNoMaskOperation: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(": ");
            itsLimitsHeaderU_ += CA2T(::GetDictionaryString("CFmiMaskOperationDlgNoLimit").c_str());
			return;
		case kFmiMaskEqual: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" = ") + FirstLimit(1);
			return;
		case kFmiMaskGreaterThan: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" > ") + FirstLimit(1);
			return;
		case kFmiMaskLessThan: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" < ") + FirstLimit(1);
			return;
		case kFmiMaskGreaterOrEqualThan: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" >= ") + FirstLimit(1);
			return;
  		case kFmiMaskLessOrEqualThan: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" <= ") + FirstLimit(1);
			return;
  		case kFmiMaskGreaterThanAndLessThan: 
            itsLimitsHeaderU_ = FirstLimit(1) + _TEXT(" < ") + itsAbbreviationU_ + _TEXT(" < ") + SecondLimit(1);
			return;
  		case kFmiMaskGreaterOrEqualAndLessOrEqual: 
            itsLimitsHeaderU_ = FirstLimit(1) + _TEXT(" <= ") + itsAbbreviationU_ + _TEXT(" <= ") + SecondLimit(1);
			return;
  		case kFmiMaskGreaterOrEqualAndLessThan: 
            itsLimitsHeaderU_ = FirstLimit(1) + _TEXT(" <= ") + itsAbbreviationU_ + _TEXT(" < ") + SecondLimit(1);
			return;
  		case kFmiMaskGreaterThanAndLessOrEqual: 
            itsLimitsHeaderU_ = FirstLimit(1) + _TEXT(" < ") + itsAbbreviationU_ + _TEXT(" <= ") + SecondLimit(1);
			return;
  		case kFmiMaskLessThanOrGreaterThan: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" < ") + FirstLimit(1) + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" > ") + SecondLimit(1);
			return;
  		case kFmiMaskLessOrEqualOrGreaterOrEqual: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" <= ") + FirstLimit(1) + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" >= ") + SecondLimit(1);
			return;
  		case kFmiMaskLessOrEqualOrGreaterThan: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" <= ") + FirstLimit(1) + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" > ") + SecondLimit(1);
			return;
  		case kFmiMaskLessThanOrGreaterOrEqual: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" < ") + FirstLimit(1) + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" >= ") + SecondLimit(1);
			return;
		case kFmiMaskEqualOrEqual: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" = ") + FirstLimit(1) + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" = ") + SecondLimit(1);
			return;

		case kFmiMaskRisingRamp: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" ") + CA2T(::GetDictionaryString("CFmiMaskOperationDlgRampType1").c_str());
			return;
		case kFmiMaskLoweringRamp: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" ") + CA2T(::GetDictionaryString("CFmiMaskOperationDlgRampType2").c_str());
			return;
		case kFmiMaskDoubleRamp: 
            itsLimitsHeaderU_ = itsAbbreviationU_ + _TEXT(" ") + CA2T(::GetDictionaryString("CFmiMaskOperationDlgRampType3").c_str());
			return;
	}

}

void CFmiMaskOperationDlg::UpdateHeaderOfLimits()
{
// Dialogin yl‰renaan tulevan otsaketekstin p‰ivitys.
	UpdateData(TRUE);
	SetHeaderOfLimits();
	UpdateData(FALSE);
}

void CFmiMaskOperationDlg::SetRemarkText(BOOL theFirstNumber)
{
// Asettaa CStatic-ikkunan itsRemarkText tekstin
	if(FirstLimit()==kFloatMissing || SecondLimit()==kFloatMissing)
		itsRemarkTextU_ = CA2T(::GetDictionaryString("CFmiMaskOperationDlgError1").c_str());
	else if(FirstLimit() < SecondLimit() || !itsEditCtrlOfSecondLimit.IsWindowEnabled() || !itsEditCtrlOfFirstLimit.IsWindowEnabled() ) // rajat annettu oikein
        itsRemarkTextU_ = _TEXT("");
	else if(theFirstNumber)
        itsRemarkTextU_ = CA2T(::GetDictionaryString("CFmiMaskOperationDlgError2").c_str());
	else
        itsRemarkTextU_ = CA2T(::GetDictionaryString("CFmiMaskOperationDlgError3").c_str());
 	Invalidate();
}


void CFmiMaskOperationDlg::UpdateRemarkText(BOOL theFirstNumber)
{
// P‰ivitt‰‰ CStatic-ikkunan itsRemarkText tekstin
	UpdateData(TRUE);
	SetRemarkText(theFirstNumber);
	UpdateData(FALSE);
}

void CFmiMaskOperationDlg::UpdateRampBitmaps(void)
{
	CWnd *win1 = GetDlgItem(IDC_STATIC_RISING_RAMP);
	CWnd *win2 = GetDlgItem(IDC_STATIC_LOWERING_RAMP);
	CWnd *win3 = GetDlgItem(IDC_STATIC_DOUBLE_RAMP);
	switch( itsRadioButtons )
	{
		case kFmiMaskRisingRamp: 
			if(win1)
				win1->ShowWindow(SW_SHOW);
			if(win2)
				win2->ShowWindow(SW_HIDE);
			if(win1)
				win3->ShowWindow(SW_HIDE);
			return;
		case kFmiMaskLoweringRamp: 
			if(win1)
				win1->ShowWindow(SW_HIDE);
			if(win2)
				win2->ShowWindow(SW_SHOW);
			if(win1)
				win3->ShowWindow(SW_HIDE);
			return;
		case kFmiMaskDoubleRamp: 
			if(win1)
				win1->ShowWindow(SW_HIDE);
			if(win2)
				win2->ShowWindow(SW_HIDE);
			if(win1)
				win3->ShowWindow(SW_SHOW);
			return;
		default:
			if(win1)
				win1->ShowWindow(SW_HIDE);
			if(win2)
				win2->ShowWindow(SW_HIDE);
			if(win1)
				win3->ShowWindow(SW_HIDE);
			return;
	}
}

// T‰m‰ funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell‰.
// T‰m‰ on ik‰v‰ kyll‰ teht‰v‰ erikseen dialogin muokkaus tyˆkalusta, eli
// tekij‰n pit‰‰ lis‰t‰ erikseen t‰nne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiMaskOperationDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("CFmiMaskOperationDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MASK_GROUP_LIMITS, "IDC_STATIC_MASK_GROUP_LIMITS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MASK_LIMIT1_STR, "IDC_STATIC_MASK_LIMIT1_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MASK_LIMIT2_STR, "IDC_STATIC_MASK_LIMIT2_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL1, "IDC_RADIO_MASK_CONDITIONAL1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL2, "IDC_RADIO_MASK_CONDITIONAL2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL3, "IDC_RADIO_MASK_CONDITIONAL3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL4, "IDC_RADIO_MASK_CONDITIONAL4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL5, "IDC_RADIO_MASK_CONDITIONAL5");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL6, "IDC_RADIO_MASK_CONDITIONAL6");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL7, "IDC_RADIO_MASK_CONDITIONAL7");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL8, "IDC_RADIO_MASK_CONDITIONAL8");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL9, "IDC_RADIO_MASK_CONDITIONAL9");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL10, "IDC_RADIO_MASK_CONDITIONAL10");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL11, "IDC_RADIO_MASK_CONDITIONAL11");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL12, "IDC_RADIO_MASK_CONDITIONAL12");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL13, "IDC_RADIO_MASK_CONDITIONAL13");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL14, "IDC_RADIO_MASK_CONDITIONAL14");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL15, "IDC_RADIO_MASK_CONDITIONAL15");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL16, "IDC_RADIO_MASK_CONDITIONAL16");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL17, "IDC_RADIO_MASK_CONDITIONAL17");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MASK_CONDITIONAL18, "IDC_RADIO_MASK_CONDITIONAL18");
}

