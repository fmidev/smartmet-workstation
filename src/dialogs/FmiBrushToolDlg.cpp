// FmiBrushToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiBrushToolDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiStringList.h"
#include "NFmiEditMapDataListHandler.h"
#include "NFmiDrawParam.h"
#include "NFmiSmartInfo.h"
#include "NFmiValueString.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "CtrlViewFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiBrushToolDlg dialog


CFmiBrushToolDlg::CFmiBrushToolDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
:CDialog(CFmiBrushToolDlg::IDD, pParent)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsMapViewDescTopIndex(0) // sidotaan muokkausdialogit pääkarttanäyttöön
,itsFogIntensityValueList(0)
,itsPrecipitationTypeValueList(0)
,itsPrecipitationFormValueList(0)
,itsLastSpecialParameterIndex(-1)
,itsLastSpecialParameterName(kFmiLastParameter)
,itsLastParameterName(kFmiLastParameter)
{
	//{{AFX_DATA_INIT(CFmiBrushToolDlg)
	fUseMask = FALSE;
    itsBrushValueStrU_ = _T("");
    itsBrushSizeStrU_ = _T("");
    itsUnitStrU_ = _T("");
	itsLimitingOption = -1;
    itsBrushLimitingValueStrU_ = _T("");
	//}}AFX_DATA_INIT
}

CFmiBrushToolDlg::~CFmiBrushToolDlg(void)
{
	delete itsFogIntensityValueList;
	delete itsPrecipitationTypeValueList;
	delete itsPrecipitationFormValueList;
}

BOOL CFmiBrushToolDlg::Create(void) // modaalittomaa varten
{
	return CDialog::Create(CFmiBrushToolDlg::IDD);
}

void CFmiBrushToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiBrushToolDlg)
	DDX_Control(pDX, IDC_COMBO_DISCRETE_VALUE_SELECTIO, itsDiscreteBrushvalueCombo);
	DDX_Control(pDX, IDC_SLIDER_BRUSH_SIZE, itsBrushSizeSlider);
	DDX_Check(pDX, IDC_CHECK_USE_MASKS, fUseMask);
    DDX_Text(pDX, IDC_EDIT_BRUSH_VALUE, itsBrushValueStrU_);
    DDX_Text(pDX, IDC_STATIC_BRUSH_SIZE, itsBrushSizeStrU_);
    DDX_Text(pDX, IDC_STATIC_UNIT_STR, itsUnitStrU_);
	DDX_Radio(pDX, IDC_RADIO_LIMITING_OPTION1, itsLimitingOption);
    DDX_Text(pDX, IDC_EDIT_BRUSH_LIMITING_VALUE, itsBrushLimitingValueStrU_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiBrushToolDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiBrushToolDlg)
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_USE_MASKS, OnCheckUseMasks)
	ON_EN_CHANGE(IDC_EDIT_BRUSH_VALUE, OnChangeEditBrushValue)
	ON_CBN_SELCHANGE(IDC_COMBO_DISCRETE_VALUE_SELECTIO, OnSelchangeComboDiscreteValueSelectio)
	ON_BN_CLICKED(ID_BUTTON_REDO, OnButtonRedo)
	ON_BN_CLICKED(ID_BUTTON_UNDO, OnButtonUndo)
	ON_EN_CHANGE(IDC_EDIT_BRUSH_LIMITING_VALUE, OnChangeEditBrushLimitingValue)
	ON_BN_CLICKED(IDC_RADIO_LIMITING_OPTION1, OnRadioLimitingOption1)
	ON_BN_CLICKED(IDC_RADIO_LIMITING_OPTION2, OnRadioLimitingOption2)
	ON_BN_CLICKED(IDC_RADIO_LIMITING_OPTION3, OnRadioLimitingOption3)
	ON_BN_CLICKED(IDC_RADIO_LIMITING_OPTION4, OnRadioLimitingOption4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiBrushToolDlg message handlers

void CFmiBrushToolDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	UpdateData(TRUE);
	int newPos = itsBrushSizeSlider.GetPos();
	itsSmartMetDocumentInterface->BrushSize(NFmiPoint(newPos, newPos));
	UpdateBrushSizeString();
	Invalidate(FALSE);
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFmiBrushToolDlg::UpdateBrushSizeString(void)
{
	NFmiString tmp(NFmiValueString::GetStringWithMaxDecimalsSmartWay(itsBrushSizeSlider.GetPos(), 0));
	tmp += " %";
    itsBrushSizeStrU_ = CA2T(tmp);
	UpdateData(FALSE);
}

BOOL CFmiBrushToolDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	itsFogIntensityValueList = new NFmiStringList(*itsSmartMetDocumentInterface->DataLists()->ParamValueList(kFmiFogIntensity));  
#ifdef USE_POTENTIAL_VALUES_IN_EDITING
    itsPrecipitationTypeValueList = new NFmiStringList(*itsSmartMetDocumentInterface->DataLists()->ParamValueList(kFmiPotentialPrecipitationType));
    itsPrecipitationFormValueList = new NFmiStringList(*itsSmartMetDocumentInterface->DataLists()->ParamValueList(kFmiPotentialPrecipitationForm));
#else
    itsPrecipitationTypeValueList = new NFmiStringList(*itsSmartMetDocumentInterface->DataLists()->ParamValueList(kFmiPrecipitationType));
    itsPrecipitationFormValueList = new NFmiStringList(*itsSmartMetDocumentInterface->DataLists()->ParamValueList(kFmiPrecipitationForm));
#endif
	fUseMask = itsSmartMetDocumentInterface->UseMaskWithBrush();
	NFmiValueString valueStr(itsSmartMetDocumentInterface->BrushValue(), "%0.2f");
    itsBrushValueStrU_ = CA2T(valueStr);

	NFmiValueString valueStr2(itsSmartMetDocumentInterface->BrushToolLimitSettingValue(), "%0.2f");
    itsBrushLimitingValueStrU_ = CA2T(valueStr2);
	itsLimitingOption = itsSmartMetDocumentInterface->BrushToolLimitSetting();

	itsBrushSizeSlider.SetRange(0,100);
	itsBrushSizeSlider.SetPos(int(itsSmartMetDocumentInterface->BrushSize().X()));
	InitDialogTexts();
	UpdateBrushSizeString();
	FillSpecialValueCombo();
	UpdateParamUnitString();
	EnableButtons();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiBrushToolDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("BrushToolDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_UNDO, "ID_BUTTON_UNDO");
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_REDO, "ID_BUTTON_REDO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_MASKS, "IDC_CHECK_USE_MASKS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BRUSH_VALUE, "IDC_STATIC_BRUSH_VALUE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BRUSH_SIZE_STR, "IDC_STATIC_BRUSH_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_CONSTRAINT_HOLDER, "IDC_STATIC_CONSTRAINT_HOLDER");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LIMITING_OPTION1, "IDC_RADIO_LIMITING_OPTION1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LIMITING_OPTION2, "IDC_RADIO_LIMITING_OPTION2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LIMITING_OPTION3, "IDC_RADIO_LIMITING_OPTION3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LIMITING_OPTION4, "IDC_RADIO_LIMITING_OPTION4");
	
}

void CFmiBrushToolDlg::OnClose() 
{
	DoWhenClosing();
	CDialog::OnClose();
}

void CFmiBrushToolDlg::DoWhenClosing(void)
{
	itsSmartMetDocumentInterface->ModifyToolMode(CtrlViewUtils::kFmiEditorModifyToolModeNormal);
}

void CFmiBrushToolDlg::OnCheckUseMasks() 
{
	UpdateData(TRUE);
	itsSmartMetDocumentInterface->UseMaskWithBrush(fUseMask != 0);
}

void CFmiBrushToolDlg::OnChangeEditBrushValue() 
{
	UpdateData(TRUE);
    int num = itsBrushValueStrU_.Replace(',', '.'); // ihmiset kirjoittavat helposti ,:n .:een sijasta (piste pitää olla)
    NFmiValueString valueStr = CT2A(itsBrushValueStrU_);
	double value = 0;
	valueStr.ConvertToDouble(value);
	itsSmartMetDocumentInterface->BrushValue(value);
}

void CFmiBrushToolDlg::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        if(itsSmartMetDocumentInterface->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush)
        {
            FillSpecialValueCombo();
            UpdateParamUnitString();
        }
    }
}

void CFmiBrushToolDlg::FillSpecialValueCombo(void)
{
	boost::shared_ptr<NFmiDrawParam> activeDrawParam = GetActiveDrawParam();
	if(activeDrawParam)
	{
		FmiParameterName paramId = FmiParameterName(activeDrawParam->Param().GetParam()->GetIdent());
		if(paramId != itsLastParameterName)
		{
			itsDiscreteBrushvalueCombo.ResetContent();
			itsLastParameterName = paramId;
			if(activeDrawParam->Param().Type() != kContinuousParam)
			{
				itsDiscreteBrushvalueCombo.ResetContent();
				NFmiStringList* wantedList = GetDiscreteList(paramId);
				if(wantedList)
				{
					wantedList->Reset();
					do
					{
						NFmiString* temp = wantedList->Current();
						if(temp)
                            itsDiscreteBrushvalueCombo.InsertString(-1, CA2T(*temp));
					} while(wantedList->Next());

					if(paramId == itsLastSpecialParameterName)
					{
						itsDiscreteBrushvalueCombo.SetCurSel(itsLastSpecialParameterIndex);
					}
					else // aseta 1. listasta
					{
						itsLastSpecialParameterName = paramId;
						if(wantedList->Reset())
							itsDiscreteBrushvalueCombo.SetCurSel(wantedList->Index());
					}
				}
				itsDiscreteBrushvalueCombo.ShowWindow(SW_SHOW);
				CWnd* window = GetDlgItem(IDC_EDIT_BRUSH_VALUE);
				if(window)
					window->ShowWindow(SW_HIDE);

			}
			else
			{
				itsDiscreteBrushvalueCombo.ShowWindow(SW_HIDE);
				CWnd* window = GetDlgItem(IDC_EDIT_BRUSH_VALUE);
				if(window)
					window->ShowWindow(SW_SHOW);
			}
			SetSelectedDiscreteValueToDocument();
		}
	}
	UpdateData(FALSE);
}

NFmiStringList* CFmiBrushToolDlg::GetDiscreteList(FmiParameterName theParamId)
{
	switch(theParamId)
	{
	case kFmiFogIntensity:
		return itsFogIntensityValueList;
	case kFmiPrecipitationType:
    case kFmiPotentialPrecipitationType:
		return itsPrecipitationTypeValueList;
    case kFmiPrecipitationForm:
    case kFmiPotentialPrecipitationForm:
		return itsPrecipitationFormValueList;
	}
	return 0;
}

void CFmiBrushToolDlg::SetSelectedDiscreteValueToDocument(void)
{
	int index = itsDiscreteBrushvalueCombo.GetCurSel();
	if(index >= 0)
		itsLastSpecialParameterIndex = index;
	if(itsLastParameterName == kFmiPrecipitationType || itsLastParameterName == kFmiPotentialPrecipitationType)
		itsSmartMetDocumentInterface->BrushSpecialParamValue(index+1);
	else
		itsSmartMetDocumentInterface->BrushSpecialParamValue(index);
}


void CFmiBrushToolDlg::OnSelchangeComboDiscreteValueSelectio() 
{
	UpdateData(TRUE);
	SetSelectedDiscreteValueToDocument();
	UpdateData(FALSE);
}

boost::shared_ptr<NFmiDrawParam> CFmiBrushToolDlg::GetActiveDrawParam()
{
	return itsSmartMetDocumentInterface->ActiveDrawParamFromActiveRow(itsMapViewDescTopIndex);
}

void CFmiBrushToolDlg::UpdateParamUnitString(void)
{
	boost::shared_ptr<NFmiDrawParam> activeDrawParam = GetActiveDrawParam();
	if(activeDrawParam)
	{
		NFmiString str(activeDrawParam->ParameterAbbreviation());
		str += " [";
		str += activeDrawParam->Unit();
		str += "]";
        itsUnitStrU_ = CA2T(str);
	}
	else
        itsUnitStrU_ = _TEXT(" -");
	UpdateData(FALSE);
}

void CFmiBrushToolDlg::EnableButtons()
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->EditedSmartInfo();
        NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo*>(info.get());
        if(smartInfo)
        {
            CWnd* window;
            if(smartInfo->Undo())
            {
                window = GetDlgItem(ID_BUTTON_UNDO);
                window->EnableWindow(TRUE);
            }
            else
            {
                window = GetDlgItem(ID_BUTTON_UNDO);
                window->EnableWindow(FALSE);
            }
            if(smartInfo->Redo())
            {
                window = GetDlgItem(ID_BUTTON_REDO);
                window->EnableWindow(TRUE);
            }
            else
            {
                window = GetDlgItem(ID_BUTTON_REDO);
                window->EnableWindow(FALSE);
            }
            return;
        }
        CWnd* window = GetDlgItem(ID_BUTTON_UNDO);
        if(window)
            window->EnableWindow(FALSE);
        window = GetDlgItem(ID_BUTTON_REDO);
        if(window)
            window->EnableWindow(FALSE);
    }
}

void CFmiBrushToolDlg::OnButtonRedo() 
{
	itsSmartMetDocumentInterface->OnButtonRedo();
}

void CFmiBrushToolDlg::OnButtonUndo() 
{
	itsSmartMetDocumentInterface->OnButtonUndo();
}

void CFmiBrushToolDlg::OnChangeEditBrushLimitingValue() 
{
	UpdateData(TRUE);
    int num = itsBrushLimitingValueStrU_.Replace(',', '.'); // ihmiset kirjoittavat helposti ,:n .:een sijasta (piste pitää olla)
    NFmiValueString valueStr = CT2A(itsBrushLimitingValueStrU_);
	double value = 0;
	valueStr.ConvertToDouble(value);
	itsSmartMetDocumentInterface->BrushToolLimitSettingValue(static_cast<float>(value));
}

void CFmiBrushToolDlg::OnRadioLimitingOption1() 
{
	SetLimitingOptionToDoc();
	bool status = EnableDlgItem(IDC_EDIT_BRUSH_VALUE, true);
}

void CFmiBrushToolDlg::OnRadioLimitingOption2() 
{
	SetLimitingOptionToDoc();
	bool status = EnableDlgItem(IDC_EDIT_BRUSH_VALUE, true);
}

void CFmiBrushToolDlg::OnRadioLimitingOption3() 
{
	SetLimitingOptionToDoc();
	bool status = EnableDlgItem(IDC_EDIT_BRUSH_VALUE, true);
}

void CFmiBrushToolDlg::OnRadioLimitingOption4() 
{
	SetLimitingOptionToDoc();
	bool status = EnableDlgItem(IDC_EDIT_BRUSH_VALUE, false); // laitetaan edit-box pois päältä, että ei tule epäselvyyksiä, mistä asetus arvo otetaan
}

void CFmiBrushToolDlg::SetLimitingOptionToDoc(void)
{
	UpdateData(TRUE);
	itsSmartMetDocumentInterface->BrushToolLimitSetting(itsLimitingOption);
}

bool CFmiBrushToolDlg::EnableDlgItem(int theDlgId, bool fEnable)
{
	CWnd *window = GetDlgItem(theDlgId);
	if(window)
	{
		if(fEnable)
			window->EnableWindow(TRUE);
		else
			window->EnableWindow(FALSE);
		return true;
	}
	return false;
}

void CFmiBrushToolDlg::OnCancel()
{
	DoWhenClosing();
	CDialog::OnCancel();
}

void CFmiBrushToolDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}
