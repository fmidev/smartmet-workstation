// FmiLocationSelectionToolDlg2.cpp : implementation file
//

#include "stdafx.h"
#include "FmiLocationSelectionToolDlg2.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiDrawParam.h"
#include "NFmiSmartInfo.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "NFmiValueString.h"
#include "NFmiLocationSelectionTool.h"
#include "CloneBitmap.h"
#include "catlog/catlog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiLocationSelectionToolDlg2 dialog


CFmiLocationSelectionToolDlg2::CFmiLocationSelectionToolDlg2(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
: CDialog(CFmiLocationSelectionToolDlg2::IDD, pParent)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsSelectionTool(smartMetDocumentInterface->LocationSelectionTool2())
, itsAbbreviationU_()
{
	//{{AFX_DATA_INIT(CFmiLocationSelectionToolDlg2)
	fUsePointedValue = FALSE;
	fSearchNeibhor = FALSE;
    itsLimit1U_ = _T("");
    itsLimit2U_ = _T("");
	itsToolSelection = -1;
	fUseSearchRadius = FALSE;
    itsLimit1UnitStringU_ = _T("");
    itsLimit2UnitStringU_ = _T("");
    itsSearchRadiusStringU_ = _T("");
    itsSelectedToolExplanatoryStringU_ = _T("");
    itsErrorStringU_ = _T("");
	//}}AFX_DATA_INIT
}


void CFmiLocationSelectionToolDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiLocationSelectionToolDlg2)
	DDX_Control(pDX, IDC_STATIC_REMARK, itsErrorTextControl);
	DDX_Control(pDX, IDC_SLIDER_RELATIVE_RADIUS, itsSearchRadiusSlider);
	DDX_Control(pDX, IDC_COMBO_PARAM_SELECTION, itsParamList);
	DDX_Check(pDX, IDC_CHECK_USE_CLICK_POINT_VALUE_AS_LIMIT1, fUsePointedValue);
	DDX_Check(pDX, IDC_CHECK_USE_NEIBHOR_SEARCH, fSearchNeibhor);
    DDX_Text(pDX, IDC_EDIT_RAJA_1, itsLimit1U_);
    DDX_Text(pDX, IDC_EDIT_RAJA_2, itsLimit2U_);
	DDX_Radio(pDX, IDC_RADIO_SINGLE_SELECTION, itsToolSelection);
	DDX_Check(pDX, IDC_CHECK_ENCLOSED_RADIUS, fUseSearchRadius);
    DDX_Text(pDX, IDC_STATIC_LIMIT1, itsLimit1UnitStringU_);
    DDX_Text(pDX, IDC_STATIC_LIMIT2, itsLimit2UnitStringU_);
    DDX_Text(pDX, IDC_STATIC_RADIUS_TEXT, itsSearchRadiusStringU_);
    DDX_Text(pDX, IDC_STATIC_RAJOITUKSET, itsSelectedToolExplanatoryStringU_);
    DDX_Text(pDX, IDC_STATIC_REMARK, itsErrorStringU_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiLocationSelectionToolDlg2, CDialog)
	//{{AFX_MSG_MAP(CFmiLocationSelectionToolDlg2)
	ON_BN_CLICKED(IDC_CHECK_ENCLOSED_RADIUS, OnCheckEnclosedRadius)
	ON_BN_CLICKED(IDC_CHECK_USE_CLICK_POINT_VALUE_AS_LIMIT1, OnCheckUseClickPointValueAsLimit1)
	ON_BN_CLICKED(IDC_CHECK_USE_NEIBHOR_SEARCH, OnCheckUseNeibhorSearch)
	ON_CBN_SELCHANGE(IDC_COMBO_PARAM_SELECTION, OnSelchangeComboParamSelection)
	ON_EN_CHANGE(IDC_EDIT_RAJA_1, OnChangeEditRaja1)
	ON_EN_CHANGE(IDC_EDIT_RAJA_2, OnChangeEditRaja2)
	ON_BN_CLICKED(IDC_RADIO_CIRCLE_SELECTION, OnRadioCircleSelection)
	ON_BN_CLICKED(IDC_RADIO_SINGLE_SELECTION, OnRadioSingleSelection)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_LIMIT_EQUALS, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_LIMIT_GREATER_THAN, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_LIMIT_LESS_THAN, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_LIMIT_GREATER_OR_EQUAL, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_LIMIT_LESS_OR_EQUAL, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS2, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS3, OnRadio9)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS4, OnRadio10)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER, OnRadio11)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER2, OnRadio12)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER3, OnRadio13)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER4, OnRadio14)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_SELECTION_REDO, OnButtonSelectionRedo)
	ON_BN_CLICKED(IDC_BUTTON_SELECTION_UNDO, OnButtonSelectionUndo)
	ON_CBN_DROPDOWN(IDC_COMBO_PARAM_SELECTION, OnDropdownComboParamSelection)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_SELECT_ALL, OnRadio16)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiLocationSelectionToolDlg2 message handlers

BOOL CFmiLocationSelectionToolDlg2::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	if(itsSelectionTool)
	{
		FillParamListCombo();
		itsParamList.SetCurSel(0);
		OnSelchangeComboParamSelection();
//		itsAbbreviation = (CString)(char*)itsSelectionTool->DrawParam()->ParameterAbbreviation();
 		Limit( NFmiValueString((float)itsSelectionTool->LowerLimit())
 			 , NFmiValueString((float)itsSelectionTool->UpperLimit()) );
 //		itsLimit1UnitString  = (CString)(char*)itsSelectionTool->DrawParam()->Unit();
//		itsLimit2UnitString = itsLimit1UnitString;
		UpdateParamInfo();
		itsToolSelection = int(itsSelectionTool->SelectedTool());
		if(itsToolSelection > kFmiCircleLocationSelection)
			itsToolSelection = int(itsSelectionTool->MaskOperation() + 2);

		itsSearchRadiusSlider.SetRange(0, 100);
		itsSearchRadiusSlider.SetTicFreq(10);
		itsSearchRadiusSlider.SetPos(int(itsSelectionTool->SearchRange()*100));
		UpdateSearchRadiusString();
	}

	InitDialogTexts();
	UpdateStateOfEditCtrl(FALSE);
	SetRemarkText();
	SetHeaderOfLimits();
 	UpdateData(FALSE);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiLocationSelectionToolDlg2::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        if(itsSelectionTool && itsSelectionTool->UseValueFromLocation())
        {
            Limit(NFmiValueString((float)itsSelectionTool->LowerLimit()), NFmiValueString((float)itsSelectionTool->UpperLimit()));
            UpdateData(FALSE);
        }
        UpdateStateOfEditCtrl();
        UpdateHeaderOfLimits();
        UpdateRemarkText();
    }
}

void CFmiLocationSelectionToolDlg2::UpdateParamInfo(void)
{
	if(itsSelectionTool->Info())
	{
        itsAbbreviationU_ = CA2T(itsSelectionTool->Info()->Param().GetParamName());
        itsLimit1UnitStringU_ = _TEXT("unit");
        itsLimit2UnitStringU_ = itsLimit1UnitStringU_;
 		UpdateData(FALSE);
	}
}

void CFmiLocationSelectionToolDlg2::SetHeaderOfLimits(void)
{
// Asetetaan otsaketeksti dialogin yläreunassa olevaan CStatic-ikkunaan itsLimitsHeader. 
// Siinä näkyy sanallisesti millainen rajoitus parametrille on valittu radiobuttoneista.

	switch( itsToolSelection )
	{
		case 0: 
			itsSelectedToolExplanatoryStringU_ = CA2T(::GetDictionaryString("IDC_RADIO_SINGLE_SELECTION").c_str());
			return;
		case 1: 
            itsSelectedToolExplanatoryStringU_ = CA2T(::GetDictionaryString("IDC_RADIO_CIRCLE_SELECTION").c_str());
			return;
		case 2: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" = ") + itsLimit1U_;
			return;
		case 3: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" > ") + itsLimit1U_;
			return;
		case 4: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" < ") + itsLimit1U_;
			return;
		case 5: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" >= ") + itsLimit1U_;
			return;
  		case 6: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" <= ") + itsLimit1U_;
			return;
  		case 7: 
            itsSelectedToolExplanatoryStringU_ = itsLimit1U_ + _TEXT(" < ") + itsAbbreviationU_ + _TEXT(" < ") + itsLimit2U_;
			return;
  		case 8: 
            itsSelectedToolExplanatoryStringU_ = itsLimit1U_ + _TEXT(" <= ") + itsAbbreviationU_ + _TEXT(" <= ") + itsLimit2U_;
			return;
  		case 9: 
            itsSelectedToolExplanatoryStringU_ = itsLimit1U_ + _TEXT(" <= ") + itsAbbreviationU_ + _TEXT(" < ") + itsLimit2U_;
			return;
  		case 10: 
            itsSelectedToolExplanatoryStringU_ = itsLimit1U_ + _TEXT(" < ") + itsAbbreviationU_ + _TEXT(" <= ") + itsLimit2U_;
			return;
  		case 11: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" < ") + itsLimit1U_ + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" > ") + itsLimit2U_;
			return;
  		case 12: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" <= ") + itsLimit1U_ + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" >= ") + itsLimit2U_;
			return;
  		case 13: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" <= ") + itsLimit1U_ + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" > ") + itsLimit2U_;
			return;
  		case 14: 
            itsSelectedToolExplanatoryStringU_ = itsAbbreviationU_ + _TEXT(" < ") + itsLimit1U_ + _TEXT(" ") + CA2T(::GetDictionaryString("NormalWordOr").c_str()) + _TEXT(" ") + itsAbbreviationU_ + _TEXT(" >= ") + itsLimit2U_;
			return;
	}

}

// outoa parametrien käyttöä, pitäisikö muuttaa parametreja tai toimintaa???
void CFmiLocationSelectionToolDlg2::Limit(const NFmiValueString &firstLimit, const NFmiValueString &secondLimit)
{
// Asetetaan dialogin CEdit-ikkunoiden itsFirstLimit ja itsSecondLimit tekstit.
// Tätä metodia käytetään dialogin alustuksessa OnInitDialog():ssa.

	NFmiValueString limit1((float)firstLimit,  "%.1f"); // otetaan mukaan vain yksi desimaali
	NFmiValueString limit2((float)secondLimit, "%.1f");
	if(firstLimit!=NFmiString(""))
        itsLimit1U_ = CA2T(NFmiValueString(limit1));
	if(secondLimit !=NFmiString(""))
        itsLimit2U_ = CA2T(NFmiValueString(limit2));
}

void CFmiLocationSelectionToolDlg2::UpdateStateOfEditCtrl(BOOL updateFromDlg)
{
// Päivittää dialogin CEdit-ruutujen tilan Enable/Disable. Ruutujen
// enablointi/disablointi määräytyy siitä, mikä radiobuttoni on valittu.
// Jos tunnistin BOOL updateFromDlg on TRUE (oletusarvo), päivitetään  
// ohjelman käyttäjän valinta dialogista attribuuttiin itsRadioButtons.
// Mikäli updateFromDlg == FALSE, päivitystä ei tehdä - tätä toimintaa
// tarvitaan dialogin alustuksen yhteydessä metodissa OnInitDialog().

	if(updateFromDlg)
		UpdateData(TRUE);
	if(itsToolSelection <= 1)
	{
		CWnd* editbox1 = GetDlgItem(IDC_EDIT_RAJA_1);
		editbox1->EnableWindow(FALSE);
		CWnd* editbox2 = GetDlgItem(IDC_EDIT_RAJA_2);
		editbox2->EnableWindow(FALSE);
	}
	if(itsToolSelection > 1 && itsToolSelection < 7)
	{
		CWnd* editbox1 = GetDlgItem(IDC_EDIT_RAJA_1);
		editbox1->EnableWindow(TRUE);
		CWnd* editbox2 = GetDlgItem(IDC_EDIT_RAJA_2);
		editbox2->EnableWindow(FALSE);
	}
	else if(itsToolSelection >= 7 )
	{
		CWnd* editbox1 = GetDlgItem(IDC_EDIT_RAJA_1);
		editbox1->EnableWindow(TRUE);
		CWnd* editbox2 = GetDlgItem(IDC_EDIT_RAJA_2);
		editbox2->EnableWindow(TRUE);
	}
}

void CFmiLocationSelectionToolDlg2::SetRemarkText(BOOL theFirstNumber)
{
// Asettaa CStatic-ikkunan itsRemarkText tekstin
	CWnd* editbox1 = GetDlgItem(IDC_EDIT_RAJA_1);
	CWnd* editbox2 = GetDlgItem(IDC_EDIT_RAJA_2);
	if(FirstLimit()==kFloatMissing || SecondLimit()==kFloatMissing)
		itsErrorStringU_ = CA2T(::GetDictionaryString("SelectionToolErrorInvalidNumber").c_str());
	else if(FirstLimit() < SecondLimit() || !editbox2->IsWindowEnabled() || !editbox1->IsWindowEnabled() ) // rajat annettu oikein
        itsErrorStringU_ = _TEXT("");
	else if(theFirstNumber)
        itsErrorStringU_ = CA2T(::GetDictionaryString("SelectionToolErrorLimits1").c_str());
	else
        itsErrorStringU_ = CA2T(::GetDictionaryString("SelectionToolErrorLimits2").c_str());
	UpdateData(FALSE);
// 	Invalidate();
}

double CFmiLocationSelectionToolDlg2::FirstLimit(void)
{
// Konvertoi CEdit-ruudun CString-tekstin itsFirstLimit 
// double-luvuksi ja palauttaa luvun käyttäjälle.
	float value = kFloatMissing;
    NFmiValueString aValueStr = CT2A(itsLimit1U_);
 	aValueStr.ConvertToFloat(value);
	return double(value);
}

double CFmiLocationSelectionToolDlg2::SecondLimit(void)
{
// Konvertoi CEdit-ruudun CString-tekstin itsSecondtLimit 
// double-luvuksi ja palauttaa luvun käyttäjälle.
 	float value = kFloatMissing;
    NFmiValueString aValueStr = CT2A(itsLimit2U_);
  	aValueStr.ConvertToFloat(value);
	return double(value);
}

void CFmiLocationSelectionToolDlg2::OnCheckEnclosedRadius() 
{
	UpdateData(TRUE);
	itsSelectionTool->LimitSearchWithRange(fUseSearchRadius != 0);	
}

void CFmiLocationSelectionToolDlg2::OnCheckUseClickPointValueAsLimit1() 
{
	UpdateData(TRUE);
	itsSelectionTool->UseValueFromLocation(fUsePointedValue != 0);	
}

void CFmiLocationSelectionToolDlg2::OnCheckUseNeibhorSearch() 
{
	// TODO: Add your control notification handler code here
	
}

void CFmiLocationSelectionToolDlg2::OnChangeEditRaja1() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	UpdateRemarkText(TRUE);
	UpdateHeaderOfLimits();
}

void CFmiLocationSelectionToolDlg2::OnChangeEditRaja2() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	UpdateRemarkText(FALSE);
	UpdateHeaderOfLimits();
}

void CFmiLocationSelectionToolDlg2::OnRadioCircleSelection() 
{
	CatLog::logMessage("Ympyrävalintaa painettu...(Valintatyökalu)", CatLog::Severity::Info, CatLog::Category::Operational);
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadioSingleSelection() 
{
    CatLog::logMessage("Yksittäinen valinta painettu...(Valintatyökalu)", CatLog::Severity::Info, CatLog::Category::Operational);
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio10() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio11() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio12() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio13() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio14() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio16() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio2() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio3() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio4() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio5() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio6() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio7() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio8() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnRadio9() 
{
	RadioButtonPushed();
}

void CFmiLocationSelectionToolDlg2::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	UpdateData(TRUE);
	int newPos = itsSearchRadiusSlider.GetPos();
	int maxPos = itsSearchRadiusSlider.GetRangeMax();
	UpdateSearchRadiusString();
	Invalidate(FALSE);
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFmiLocationSelectionToolDlg2::UpdateSearchRadiusString(void)
{
	NFmiString tmp(NFmiValueString::GetStringWithMaxDecimalsSmartWay(itsSearchRadiusSlider.GetPos(), 0));
	tmp += " %";
    itsSearchRadiusStringU_ = CA2T(tmp);
	itsSelectionTool->SearchRange(itsSearchRadiusSlider.GetPos()/100.f);
	UpdateData(FALSE);
}

void CFmiLocationSelectionToolDlg2::OnButtonSelectionRedo() 
{
	boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->EditedSmartInfo();
	NFmiSmartInfo *editedInfo = dynamic_cast<NFmiSmartInfo*>(info.get());
	if(editedInfo && editedInfo->LocationSelectionRedo())
	{
		editedInfo->LocationSelectionRedoData();
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__);
	}
}

void CFmiLocationSelectionToolDlg2::OnButtonSelectionUndo() 
{
	boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->EditedSmartInfo();
	NFmiSmartInfo *editedInfo = dynamic_cast<NFmiSmartInfo*>(info.get());
	if(editedInfo && editedInfo->LocationSelectionUndo())
	{
		editedInfo->LocationSelectionUndoData();
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__);
	}
}

void CFmiLocationSelectionToolDlg2::RadioButtonPushed(void) 
{
	UpdateStateOfEditCtrl();	
	UpdateHeaderOfLimits();
	UpdateRemarkText();
}

void CFmiLocationSelectionToolDlg2::UpdateRemarkText(BOOL theFirstNumber)
{
// Päivittää CStatic-ikkunan itsRemarkText tekstin
	UpdateData(TRUE);
	SetRemarkText(theFirstNumber);
	UpdateSelectionTool(); // laitoin tämän tähän, koska UpdateRemarkText-metodia kutsutaan aina kun tehdään jotain muutoksia
	UpdateData(FALSE);
}

void CFmiLocationSelectionToolDlg2::UpdateHeaderOfLimits(void)
{
// Dialogin ylärenaan tulevan otsaketekstin päivitys.
	UpdateData(TRUE);
	SetHeaderOfLimits();
	UpdateData(FALSE);
}

void CFmiLocationSelectionToolDlg2::OnOK() 
{
	UpdateSelectionTool();

	CDialog::OnOK();
}

FmiMaskOperation CFmiLocationSelectionToolDlg2::MaskOperation(void)
{
// tähän alkuun yksi poikkeus radio-button sääntöihin, mitkä seuraavat perässä
// maskall -radiobutton poikkeaa muista, koska ennen enum:ia kFmiMaskAll = 15
// on arvo, jota ei käytetä tässä dialogissa.
	if(FmiMaskOperation(itsToolSelection) == kFmiMaskAll)
		return (FmiMaskOperation)(itsToolSelection); // ei -1 kuten perässä tässä tapauksessa!

// Palauttaa dialogin FmiMaskOperation-parametrinimen.
// Koska radiobuttonit ovat enum-määrittelyn FmiMaskOperation
// mukaisessa järjestyksessä, voidaan itsRadioButtons-arvo
// castata suoraan palautettavaksi FmiMaskOperation-arvoksi
	return (FmiMaskOperation)(itsToolSelection-1); // -1 johtuu siitä, että kaksi ensimmäistä radiobuttonia eivät kuvaa maskeja
}

// Koska dialogi 'leijuu', tulevat valinnat heti voimaan ja tämä metodi muuttaa
// selection toolin asetukset vastaamaan dialogin tilaa.
void CFmiLocationSelectionToolDlg2::UpdateSelectionTool(void)
{
	UpdateData(TRUE);
	if(FirstLimit()!=kFloatMissing)
		itsSelectionTool->LowerLimit(FirstLimit());
	if(SecondLimit()!=kFloatMissing)
		itsSelectionTool->UpperLimit(SecondLimit());
	if(itsToolSelection <= kFmiCircleLocationSelection)
		itsSelectionTool->SelectedTool(FmiLocationSearchTool(itsToolSelection));
	else
	{
		itsSelectionTool->SelectedTool(kFmiParamMaskLocationSelection);
		itsSelectionTool->MaskOperation(MaskOperation());
	}
}
/* // en saanut toimimaan niinkuin maskaus dialogissa, siksi teksti jäi nyt mustaksi
void CFmiLocationSelectionToolDlg2::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
 	itsErrorTextControl.GetClientRect(rect);
 	CPaintDC remarkDC(&itsErrorTextControl);    // piirretään teksti Static-ikkunaan
//	CWnd* errortextwindow = GetDlgItem(IDC_STATIC_REMARK);
// 	errortextwindow->GetClientRect(rect);
// 	CPaintDC remarkDC(errortextwindow);    // piirretään teksti Static-ikkunaan
	remarkDC.SetTextColor(RGB(250,10,5)); // punainen teksti
	remarkDC.SetBkColor(dc.GetBkColor()); // tekstin taustaväriksi asetetaan dialogin (piirtopinta dc) taustaväri	
	remarkDC.DrawText(itsErrorString,rect,DT_CENTER | DT_WORDBREAK);
	
	// Do not call CDialog::OnPaint() for painting messages
}*/

void CFmiLocationSelectionToolDlg2::OnDropdownComboParamSelection() 
{
	ClearParamListCombo();
	FillParamListCombo();
}

void CFmiLocationSelectionToolDlg2::OnSelchangeComboParamSelection() 
{
	int index = itsParamList.GetCurSel();
	if(itsSmartMetDocumentInterface)
	{
        itsSmartMetDocumentInterface->SelectNewParamForSelectionTool(itsParameterNameTable[index]);
		UpdateParamInfo();
		UpdateHeaderOfLimits();
		UpdateData(FALSE);
	}
}

void CFmiLocationSelectionToolDlg2::ClearParamListCombo(void)
{
	int count = itsParamList.GetCount();
	for(int index = 0; index < count; index++)
		itsParamList.DeleteString(0);
	int count2 = itsParamList.GetCount();
}

void CFmiLocationSelectionToolDlg2::FillParamListCombo(void)
{
	if(itsSmartMetDocumentInterface)
	{
		int index = 0;
		NFmiParamBag params = itsSmartMetDocumentInterface->AllStaticParams();
		for(params.Reset(); params.Next(); )
		{
			if(params.Current()->HasDataParams())
			{
				NFmiParamBag *subParams = params.Current()->GetDataParams();
				for(subParams->Reset(); subParams->Next(); )
				{
					itsParameterNameTable[index] = subParams->Current()->GetParam()->GetIdent(); 
                    CString subParamNameU_ = CA2T(subParams->Current()->GetParamName());
                    int status2 = itsParamList.AddString(subParamNameU_);
					index++;
				}
			}
			else
			{
				itsParameterNameTable[index] = params.Current()->GetParam()->GetIdent(); 
                CString paramNameU_ = CA2T(params.Current()->GetParamName());
                int status = itsParamList.AddString(paramNameU_);
				index++;
			}
		}
	}
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiLocationSelectionToolDlg2::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("CFmiLocationSelectionToolDlg2Title").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SINGLE_SELECTION, "IDC_RADIO_SINGLE_SELECTION");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_CIRCLE_SELECTION, "IDC_RADIO_CIRCLE_SELECTION");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_LIMIT_EQUALS, "IDC_RADIO_SELECTION_TOOL_LIMIT_EQUALS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_LIMIT_GREATER_THAN, "IDC_RADIO_SELECTION_TOOL_LIMIT_GREATER_THAN");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_LIMIT_LESS_THAN, "IDC_RADIO_SELECTION_TOOL_LIMIT_LESS_THAN");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_LIMIT_GREATER_OR_EQUAL, "IDC_RADIO_SELECTION_TOOL_LIMIT_GREATER_OR_EQUAL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_LIMIT_LESS_OR_EQUAL, "IDC_RADIO_SELECTION_TOOL_LIMIT_LESS_OR_EQUAL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS, "IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS2, "IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS3, "IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS4, "IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER, "IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER2, "IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER3, "IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER4, "IDC_RADIO_SELECTION_TOOL_TWO_LIMIT_LESS_AND_GREATER4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_SELECT_ALL, "IDC_RADIO_SELECTION_TOOL_SELECT_ALL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SELECTION_TOOL_PARAM_STR, "IDC_STATIC_SELECTION_TOOL_PARAM_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SELECTION_UNDO, "IDC_BUTTON_SELECTION_UNDO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SELECTION_REDO, "IDC_BUTTON_SELECTION_REDO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SELECTION_RAJA1, "IDC_STATIC_SELECTION_RAJA1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SELECTION_RAJA2, "IDC_STATIC_SELECTION_RAJA2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SELECTION_TOOL_RANGE_STR, "IDC_STATIC_SELECTION_TOOL_RANGE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_ENCLOSED_RADIUS, "IDC_CHECK_ENCLOSED_RADIUS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_NEIBHOR_SEARCH, "IDC_CHECK_USE_NEIBHOR_SEARCH");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_CLICK_POINT_VALUE_AS_LIMIT1, "IDC_CHECK_USE_CLICK_POINT_VALUE_AS_LIMIT1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SELECTION_GROUP_STR, "IDC_STATIC_SELECTION_GROUP_STR");
}

