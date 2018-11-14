// FmiFilterDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiFilterDataDlg.h"
#include "FmiFilterDataView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiSmartInfo.h"
#include "FmiParameterActivationDlg.h"
#include "NFmiDrawParam.h"
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
// CFmiFilterDataDlg dialog


CFmiFilterDataDlg::CFmiFilterDataDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
:CDialog(CFmiFilterDataDlg::IDD, pParent)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsMapViewDescTopIndex(0) // sidotaan muokkausdialogit pääkarttanäyttöön
,itsAreaFilterView1(0)
,itsAreaFilterView2(0)
,itsTimeFilterView1(0)
,itsTimeFilterView2(0)
,itsTimeControlView(0)
,itsEditedDataTimeBag()
{
	//{{AFX_DATA_INIT(CFmiFilterDataDlg)
	fFilterSelectedLocations = FALSE;
	fUseMasks = FALSE;
	itsParameterSelection = 0;
	fUseTimeInterpolation = FALSE;
	//}}AFX_DATA_INIT
}


void CFmiFilterDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiFilterDataDlg)
	DDX_Control(pDX, IDC_STATIC_EDITED_PARAMS_STRING, itsSelectedParamsCtrl);
	DDX_Control(pDX, IDC_COMBO_FILTER_SELECTION, itsFilterSelection);
	DDX_Check(pDX, IDC_CHECK_FILTER_SELECTED_LOCATIONS, fFilterSelectedLocations);
	DDX_Check(pDX, IDC_CHECK_USE_MASKS, fUseMasks);
	DDX_Radio(pDX, IDC_RADIO_PARAMETER_SELECTION_ACTIVE, itsParameterSelection);
	DDX_Check(pDX, IDC_CHECK_USE_TIME_INTERPOLATION, fUseTimeInterpolation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiFilterDataDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiFilterDataDlg)
	ON_BN_CLICKED(ID_BUTTON_REDO, OnButtonRedo)
	ON_BN_CLICKED(ID_BUTTON_UNDO, OnButtonUndo)
	ON_BN_CLICKED(ID_BUTTON_AREA_FILTER_ACTION, OnButtonAreaFilterAction)
	ON_BN_CLICKED(ID_BUTTON_TIME_FILTER_ACTION, OnButtonTimeFilterAction)
	ON_BN_CLICKED(ID_BUTTON_RESET, OnButtonReset)
	ON_BN_CLICKED(IDC_RADIO_PARAMETER_SELECTION_ACTIVE, OnRadioParameterSelectionActive)
	ON_BN_CLICKED(IDC_RADIO_PARAMETER_SELECTION_ALL, OnRadioParameterSelectionAll)
	ON_BN_CLICKED(IDC_RADIO_PARAMETER_SELECTION_SELECTED, OnRadioParameterSelectionSelected)
	ON_BN_CLICKED(ID_BUTTON_PARAM_SELECTION, OnButtonParamSelection)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_USE_TIME_INTERPOLATION, OnCheckUseTimeInterpolation)
	ON_BN_CLICKED(ID_BUTTON_KLAPSE, OnButtonKlapse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiFilterDataDlg message handlers

void CFmiFilterDataDlg::RefreshApplicationViews(const std::string &reasonForUpdate, bool clearEditedDependentMacroParamCacheData)
{
	itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, false, false, false, clearEditedDependentMacroParamCacheData);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(reasonForUpdate);
}

void CFmiFilterDataDlg::OnButtonRedo()
{
    itsSmartMetDocumentInterface->OnButtonRedo();
}

void CFmiFilterDataDlg::OnButtonUndo()
{
    itsSmartMetDocumentInterface->OnButtonUndo();
}

BOOL CFmiFilterDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();

    itsFilterSelection.InsertString(-1, _TEXT("Avg"));
    itsFilterSelection.InsertString(-1, _TEXT("Max"));
    itsFilterSelection.InsertString(-1, _TEXT("Min"));
    itsFilterSelection.InsertString(-1, _TEXT("WAvg"));
    itsFilterSelection.InsertString(-1, _TEXT("Medi"));
	itsFilterSelection.SetCurSel(0);
	itsParameterSelection = 0;

// ************ 1. areafilter ikkunan luonti ***************************
	CRect rect;
	CWnd* staticWnd = GetDlgItem(IDC_CLIENT_VIEW_AREA);
	WINDOWPLACEMENT wplace;
	if(staticWnd)
		staticWnd->GetWindowPlacement(&wplace);
	rect = wplace.rcNormalPosition;

	rect.InflateRect(-1,-1,-1,-1); // 'hieno' upotus tulee näkyviin
	
	itsAreaFilterView1 = new CFmiFilterDataView(itsMapViewDescTopIndex, this, itsSmartMetDocumentInterface, 1, NFmiCtrlView::kAreaFilterView);
	itsAreaFilterView1->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsAreaFilterView1->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
// ************ 1. areafilter ikkunan luonti ***************************

// ************ 2. areafilter ikkunan luonti ***************************
	staticWnd = GetDlgItem(IDC_CLIENT_VIEW_AREA2);
	if(staticWnd)
		staticWnd->GetWindowPlacement(&wplace);
	rect = wplace.rcNormalPosition;

	rect.InflateRect(-1,-1,-1,-1); // 'hieno' upotus tulee näkyviin
	
	itsAreaFilterView2 = new CFmiFilterDataView(itsMapViewDescTopIndex, this, itsSmartMetDocumentInterface, 2, NFmiCtrlView::kAreaFilterView);
	itsAreaFilterView2->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsAreaFilterView2->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
// ************ 2. areafilter ikkunan luonti ***************************

// ************ 1. timefilter ikkunan luonti ***************************
	staticWnd = GetDlgItem(IDC_TIME_FILTER_VIEW1);
	if(staticWnd)
		staticWnd->GetWindowPlacement(&wplace);
	rect = wplace.rcNormalPosition;

	rect.InflateRect(-1,-1,-1,-1); // 'hieno' upotus tulee näkyviin
	
	itsTimeFilterView1 = new CFmiFilterDataView(itsMapViewDescTopIndex, this, itsSmartMetDocumentInterface, 1, NFmiCtrlView::kTimeFilterView);
	itsTimeFilterView1->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsTimeFilterView1->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
// ************ 1. timefilter ikkunan luonti ***************************

// ************ 2. timefilter ikkunan luonti ***************************
	staticWnd = GetDlgItem(IDC_TIME_FILTER_VIEW2);
	if(staticWnd)
		staticWnd->GetWindowPlacement(&wplace);
	rect = wplace.rcNormalPosition;

	rect.InflateRect(-1,-1,-1,-1); // 'hieno' upotus tulee näkyviin
	
	itsTimeFilterView2 = new CFmiFilterDataView(itsMapViewDescTopIndex, this, itsSmartMetDocumentInterface, 2, NFmiCtrlView::kTimeFilterView);
	itsTimeFilterView2->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsTimeFilterView2->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
// ************ 2. timefilter ikkunan luonti ***************************

	if(IsNewTimeControlNeeded())
		CreateTimeControlView();

	CWnd* window = GetDlgItem(ID_BUTTON_UNDO);
	if(window)
		window->EnableWindow(FALSE);
	window = GetDlgItem(ID_BUTTON_REDO);
	if(window)
		window->EnableWindow(FALSE);

	InitDialogFromDoc();
	UpdateEditedParamsString();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CFmiFilterDataDlg::IsNewTimeControlNeeded(void)
{
	if(itsSmartMetDocumentInterface->EditedSmartInfo())
	{
		if(itsEditedDataTimeBag == itsSmartMetDocumentInterface->EditedDataTimeBag())
			return FALSE;
		else
			return TRUE;
	}
	return FALSE;
}

void CFmiFilterDataDlg::CreateTimeControlView(void)
{
	if(itsTimeControlView)
	{
		delete itsTimeControlView;
		itsTimeControlView = 0;
	}
	if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->DefaultEditedDrawParam())
	{
		itsEditedDataTimeBag = itsSmartMetDocumentInterface->EditedDataTimeBag();
		CRect rect;
		WINDOWPLACEMENT wplace;
	// ************ timecontrol ikkunan luonti ***************************
		CWnd* staticWnd = GetDlgItem(IDC_STATIC_MODIFIED_TIMES_VIEW);
		if(staticWnd)
			staticWnd->GetWindowPlacement(&wplace);
		rect = wplace.rcNormalPosition;

		rect.InflateRect(-1,-1,-1,-1); // 'hieno' upotus tulee näkyviin
		
		itsTimeControlView = new CFmiFilterDataView(itsMapViewDescTopIndex, this, itsSmartMetDocumentInterface, 2, NFmiCtrlView::kTimeControlView);
		itsTimeControlView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
		itsTimeControlView->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
	// ************ timecontrol ikkunan luonti ***************************
	}
}

BOOL CFmiFilterDataDlg::Create(CWnd* pParentWnd)
{
	return CDialog::Create(CFmiFilterDataDlg::IDD, pParentWnd);
}

void CFmiFilterDataDlg::EnableButtons()
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        if(itsSmartMetDocumentInterface)
        {
            boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsSmartMetDocumentInterface->EditedSmartInfo();
            if(editedInfo)
            {
                CWnd* window;
                if(dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->Undo())
                {
                    window = GetDlgItem(ID_BUTTON_UNDO);
                    window->EnableWindow(TRUE);
                }
                else
                {
                    window = GetDlgItem(ID_BUTTON_UNDO);
                    window->EnableWindow(FALSE);
                }
                if(dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->Redo())
                {
                    window = GetDlgItem(ID_BUTTON_REDO);
                    window->EnableWindow(TRUE);
                }
                else
                {
                    window = GetDlgItem(ID_BUTTON_REDO);
                    window->EnableWindow(FALSE);
                }
            }
        }
    }
}

void CFmiFilterDataDlg::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        int dialogUpdateStatus = itsSmartMetDocumentInterface->FilterDialogUpdateStatus();
        if(dialogUpdateStatus == 2)
        {
            if(itsAreaFilterView1)
                ((CZoomView*)itsAreaFilterView1)->Update();
            if(itsAreaFilterView2)
                ((CZoomView*)itsAreaFilterView2)->Update();
            if(itsTimeFilterView1)
                ((CZoomView*)itsTimeFilterView1)->Update();
            if(itsTimeFilterView2)
                ((CZoomView*)itsTimeFilterView2)->Update();
            UpdateEditedParamsString();
        }

        if(dialogUpdateStatus == 2 || dialogUpdateStatus == 1)
        {
            if(IsNewTimeControlNeeded())
                CreateTimeControlView();
            if(itsTimeControlView)
                ((CZoomView*)itsTimeControlView)->Update();
        }

        UpdateEditedParamsString();
        itsSmartMetDocumentInterface->FilterDialogUpdateStatus(0); // 0 dialogia ei enää tarvitse päivittää

        {
            CWnd *win = GetDlgItem(ID_BUTTON_TIME_FILTER_ACTION);
            if(win)
                win->EnableWindow(TRUE);
            win = GetDlgItem(ID_BUTTON_AREA_FILTER_ACTION);
            if(win)
                win->EnableWindow(TRUE);
        }
    }
}

void CFmiFilterDataDlg::OnButtonAreaFilterAction()
{
	if(itsSmartMetDocumentInterface)
	{
		CWaitCursor tempCursor;
		SetDocumentValues();
        itsSmartMetDocumentInterface->DoAreaFiltering();
		RefreshApplicationViews("Data filter tool: Area filter action", true);
	}
}

void CFmiFilterDataDlg::OnButtonTimeFilterAction()
{
	if(itsSmartMetDocumentInterface)
	{
		CWaitCursor tempCursor;
		SetDocumentValues();
        itsSmartMetDocumentInterface->DoTimeFiltering();
		RefreshApplicationViews("Data filter tool: Time filter action", true);
	}
}

void CFmiFilterDataDlg::SetDocumentValues(void)
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->FilterFunction(itsFilterSelection.GetCurSel());
    itsSmartMetDocumentInterface->UseMasksWithFilterTool(fUseMasks != 0);
	if(fFilterSelectedLocations)
        itsSmartMetDocumentInterface->SetTestFilterMask(int(NFmiMetEditorTypes::kFmiSelectionMask));
	else
        itsSmartMetDocumentInterface->SetTestFilterMask(int(NFmiMetEditorTypes::kFmiNoMask));
    itsSmartMetDocumentInterface->FilteringParameterUsageState(itsParameterSelection);

    itsSmartMetDocumentInterface->UseTimeInterpolation(fUseTimeInterpolation != 0);
}

void CFmiFilterDataDlg::OnButtonReset()
{
	if(itsSmartMetDocumentInterface)
	{
        itsSmartMetDocumentInterface->ResetFilters();
		RefreshApplicationViews("Data filter tool: Reset filters", false);
	}
}

void CFmiFilterDataDlg::OnRadioParameterSelectionActive()
{
	SetParamSelectionButtonState();
	UpdateEditedParamsString();
}

void CFmiFilterDataDlg::OnRadioParameterSelectionAll()
{
	SetParamSelectionButtonState();
	UpdateEditedParamsString();
	UpdateData(FALSE);
}

void CFmiFilterDataDlg::OnRadioParameterSelectionSelected()
{
	SetParamSelectionButtonState();
	UpdateEditedParamsString();
	UpdateData(FALSE);
}

void CFmiFilterDataDlg::SetParamSelectionButtonState(void)
{
	UpdateData(TRUE);
	CWnd *win = GetDlgItem(ID_BUTTON_PARAM_SELECTION);
	if(win && itsParameterSelection == 2) // 2 = selected params
		win->EnableWindow(TRUE);
	else
		win->EnableWindow(FALSE);
}

void CFmiFilterDataDlg::OnButtonParamSelection()
{
	if(itsSmartMetDocumentInterface)
	{
		CFmiParameterActivationDlg dlg(itsSmartMetDocumentInterface->FilteringParamBag(), 0);
		if(dlg.DoModal() == IDOK)
		{
			if(itsParameterSelection == 2)
                itsSmartMetDocumentInterface->FilteringParamBag(dlg.ParamBag());
			UpdateEditedParamsString();
		}
	}
}

void CFmiFilterDataDlg::InitDialogFromDoc(void)
{
	UpdateData(TRUE);

	if(itsSmartMetDocumentInterface->GetTestFilterMask() == int(NFmiMetEditorTypes::kFmiNoMask))
		fFilterSelectedLocations = FALSE;
	else
		fFilterSelectedLocations = TRUE;
	fUseMasks = itsSmartMetDocumentInterface->UseMasksWithFilterTool();
	itsParameterSelection = itsSmartMetDocumentInterface->FilteringParameterUsageState();

	fUseTimeInterpolation = itsSmartMetDocumentInterface->UseTimeInterpolation();

	UpdateData(FALSE);
	SetParamSelectionButtonState();
}

void CFmiFilterDataDlg::UpdateEditedParamsString(void)
{
	itsSelectedParamsStrU_ = _TEXT("");
	boost::shared_ptr<NFmiDrawParam> activeDrawParam;
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo;

	switch(itsParameterSelection)
	{
	case 0: // aktiivinen parametri
		activeDrawParam = itsSmartMetDocumentInterface->ActiveDrawParam(itsMapViewDescTopIndex, itsSmartMetDocumentInterface->ActiveViewRow(itsMapViewDescTopIndex));
		if(activeDrawParam)
		{
			if(activeDrawParam->DataType() == NFmiInfoData::kEditable)
			{
				NFmiString str(activeDrawParam->ParameterAbbreviation());
				itsSelectedParamsStrU_ = CA2T(str);
			}
			else
                itsSelectedParamsStrU_ = CA2T(::GetDictionaryString("DataModificationDlgCantEditSelectedStr").c_str());
		}
		else
            itsSelectedParamsStrU_ = CA2T(::GetDictionaryString("DataModificationDlgNoParamSelectedStr").c_str());
		break;

	case 1: // kaikki parametrit
		editedInfo = itsSmartMetDocumentInterface->EditedSmartInfo();
		if(editedInfo)
            itsSelectedParamsStrU_ = CA2T(::GetDictionaryString("DataModificationDlgAllParamsSelectedStr").c_str());
		else
            itsSelectedParamsStrU_ = CA2T(::GetDictionaryString("DataModificationDlgNoDataStr").c_str());
		break;

	case 2: // valitut parametrit
		editedInfo = itsSmartMetDocumentInterface->EditedSmartInfo();
		if(editedInfo)
		{
			NFmiParamBag filteringParamBag = itsSmartMetDocumentInterface->FilteringParamBag();
			for(filteringParamBag.Reset(); filteringParamBag.Next(); )
			{
				if(filteringParamBag.Current()->HasDataParams())
				{
					NFmiParamBag subParamBag(*filteringParamBag.Current()->GetDataParams());
					for(subParamBag.Reset(); subParamBag.Next(); )
					{
						if(subParamBag.Current()->IsActive())
						{
							NFmiString str(subParamBag.Current()->GetParamName());
                            itsSelectedParamsStrU_ += CA2T(str);
                            itsSelectedParamsStrU_ += _TEXT(", ");
						}
					}
				}
				else
				{
					if(filteringParamBag.Current()->IsActive())
					{
						NFmiString str(filteringParamBag.Current()->GetParamName());
                        itsSelectedParamsStrU_ += CA2T(str);
                        itsSelectedParamsStrU_ += _TEXT(", ");
					}
				}
			}
            if(itsSelectedParamsStrU_ == _TEXT(""))
                itsSelectedParamsStrU_ = CA2T(::GetDictionaryString("DataModificationDlgNoParamSelectedStr").c_str());
		}
		else
            itsSelectedParamsStrU_ = CA2T(::GetDictionaryString("DataModificationDlgNoDataStr").c_str());
		break;
	}
	Invalidate(FALSE); // EN SAA TÄTÄ POIS, eli muuten teksti ei päivity
}

void CFmiFilterDataDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
 	itsSelectedParamsCtrl.GetClientRect(rect);
 	CPaintDC remarkDC(&itsSelectedParamsCtrl);    // piirretään teksti Static-ikkunaan
	CRect rect2(rect);
	remarkDC.SetBkColor(dc.GetBkColor()); // tekstin taustaväriksi asetetaan dialogin (piirtopinta dc) taustaväri
	remarkDC.SetTextColor(dc.GetBkColor());
	remarkDC.Rectangle(rect2);
	remarkDC.SetTextColor(RGB(255,0,0)); // punainen teksti
	rect.InflateRect(-1,-1,-1,-1);
    remarkDC.DrawText(itsSelectedParamsStrU_, rect, DT_LEFT);
}

void CFmiFilterDataDlg::OnCheckUseTimeInterpolation()
{
	UpdateData(TRUE);
	if(fUseTimeInterpolation)
	{
		itsTimeFilterView1->ShowWindow(SW_HIDE);
		itsTimeFilterView2->ShowWindow(SW_HIDE);
		CWnd *win = GetDlgItem(ID_BUTTON_TIME_FILTER_ACTION);
		if(win)
			win->EnableWindow(FALSE);
	}
	else
	{
		itsTimeFilterView1->ShowWindow(SW_SHOW);
		itsTimeFilterView2->ShowWindow(SW_SHOW);
//		if(!(itsSmartMetDocumentInterface->EditorMode() == kFmiMetEditorModeCompareModels))
		{
			CWnd *win = GetDlgItem(ID_BUTTON_TIME_FILTER_ACTION);
			if(win)
				win->EnableWindow(TRUE);
		}
	}
}

void CFmiFilterDataDlg::OnButtonKlapse()
{
	CWaitCursor tempCursor;
	SetDocumentValues();
    itsSmartMetDocumentInterface->DoCombineModelAndKlapse();
	RefreshApplicationViews("Data filter tool: Applying radar forecast data", true);
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiFilterDataDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("DataModificationDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_UNDO, "ID_BUTTON_UNDO");
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_REDO, "ID_BUTTON_REDO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_MASKS, "IDC_CHECK_USE_MASKS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_MODIFICATION_FILTER_STR, "IDC_STATIC_DATA_MODIFICATION_FILTER_STR");
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_RESET, "ID_BUTTON_RESET");
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_KLAPSE, "ID_BUTTON_KLAPSE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_MODIFICATION_PARAM_SELECTION_STR, "IDC_STATIC_DATA_MODIFICATION_PARAM_SELECTION_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_PARAMETER_SELECTION_ACTIVE, "IDC_RADIO_PARAMETER_SELECTION_ACTIVE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_PARAMETER_SELECTION_ALL, "IDC_RADIO_PARAMETER_SELECTION_ALL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_PARAMETER_SELECTION_SELECTED, "IDC_RADIO_PARAMETER_SELECTION_SELECTED");
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_PARAM_SELECTION, "ID_BUTTON_PARAM_SELECTION");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_FILTER_SELECTED_LOCATIONS, "IDC_CHECK_FILTER_SELECTED_LOCATIONS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_TIME_INTERPOLATION, "IDC_CHECK_USE_TIME_INTERPOLATION");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_MODIFICATION_AREA_STR, "IDC_STATIC_DATA_MODIFICATION_AREA_STR");
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_AREA_FILTER_ACTION, "ID_BUTTON_AREA_FILTER_ACTION");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_MODIFICATION_TIME_STR, "IDC_STATIC_DATA_MODIFICATION_TIME_STR");
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_TIME_FILTER_ACTION, "ID_BUTTON_TIME_FILTER_ACTION");
}
