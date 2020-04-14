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
	itsToolSelection = -1;
    itsSearchRadiusStringU_ = _T("");
    itsSelectedToolExplanatoryStringU_ = _T("");
	//}}AFX_DATA_INIT
}


void CFmiLocationSelectionToolDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiLocationSelectionToolDlg2)
	DDX_Control(pDX, IDC_SLIDER_RELATIVE_RADIUS, itsSearchRadiusSlider);
	DDX_Radio(pDX, IDC_RADIO_SINGLE_SELECTION, itsToolSelection);
    DDX_Text(pDX, IDC_STATIC_RADIUS_TEXT, itsSearchRadiusStringU_);
    DDX_Text(pDX, IDC_STATIC_RAJOITUKSET, itsSelectedToolExplanatoryStringU_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiLocationSelectionToolDlg2, CDialog)
	//{{AFX_MSG_MAP(CFmiLocationSelectionToolDlg2)
	ON_BN_CLICKED(IDC_RADIO_CIRCLE_SELECTION, OnRadioCircleSelection)
	ON_BN_CLICKED(IDC_RADIO_SINGLE_SELECTION, OnRadioSingleSelection)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_SELECTION_REDO, OnButtonSelectionRedo)
	ON_BN_CLICKED(IDC_BUTTON_SELECTION_UNDO, OnButtonSelectionUndo)
	ON_BN_CLICKED(IDC_RADIO_SELECTION_TOOL_SELECT_ALL, OnRadio16)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiLocationSelectionToolDlg2 message handlers

BOOL CFmiLocationSelectionToolDlg2::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	if(itsSelectionTool)
	{
		itsToolSelection = static_cast<int>(itsSelectionTool->SelectedTool());
		if(itsToolSelection < 0 || itsToolSelection > static_cast<int>(FmiLocationSearchTool::SelectAll))
			itsToolSelection = 0;

		itsSearchRadiusSlider.SetRange(0, 100);
		itsSearchRadiusSlider.SetTicFreq(10);
		itsSearchRadiusSlider.SetPos(int(itsSelectionTool->SearchRange()*100));
		UpdateSearchRadiusString();
	}

	InitDialogTexts();
 	UpdateData(FALSE);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiLocationSelectionToolDlg2::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        UpdateRemarkText();
    }
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

void CFmiLocationSelectionToolDlg2::OnRadio16() 
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
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
	}
}

void CFmiLocationSelectionToolDlg2::OnButtonSelectionUndo() 
{
	boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->EditedSmartInfo();
	NFmiSmartInfo *editedInfo = dynamic_cast<NFmiSmartInfo*>(info.get());
	if(editedInfo && editedInfo->LocationSelectionUndo())
	{
		editedInfo->LocationSelectionUndoData();
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
	}
}

void CFmiLocationSelectionToolDlg2::RadioButtonPushed(void) 
{
	UpdateRemarkText();
}

void CFmiLocationSelectionToolDlg2::UpdateRemarkText(BOOL theFirstNumber)
{
// Päivittää CStatic-ikkunan itsRemarkText tekstin
	UpdateData(TRUE);
	UpdateSelectionTool(); // laitoin tämän tähän, koska UpdateRemarkText-metodia kutsutaan aina kun tehdään jotain muutoksia
	UpdateData(FALSE);
}

void CFmiLocationSelectionToolDlg2::OnOK() 
{
	UpdateSelectionTool();

	CDialog::OnOK();
}

void CFmiLocationSelectionToolDlg2::UpdateSelectionTool(void)
{
	UpdateData(TRUE);
	itsSelectionTool->SelectedTool(static_cast<FmiLocationSearchTool>(itsToolSelection));
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
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SELECTION_TOOL_SELECT_ALL, "IDC_RADIO_SELECTION_TOOL_SELECT_ALL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SELECTION_UNDO, "IDC_BUTTON_SELECTION_UNDO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SELECTION_REDO, "IDC_BUTTON_SELECTION_REDO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SELECTION_TOOL_RANGE_STR, "IDC_STATIC_SELECTION_TOOL_RANGE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SELECTION_GROUP_STR, "IDC_STATIC_SELECTION_GROUP_STR");
}

