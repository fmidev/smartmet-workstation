// FmiParameterActivationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiParameterActivationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiParameterActivationDlg dialog


CFmiParameterActivationDlg::CFmiParameterActivationDlg(const NFmiParamBag& theParamBag, const NFmiParamBag* theHelpParamBag, CWnd* pParent /*=NULL*/)
:CDialog(CFmiParameterActivationDlg::IDD, pParent)
,itsParamBag(theParamBag)
,itsHelpParamBag(0)
{
	if(theHelpParamBag)
		itsHelpParamBag = new NFmiParamBag(*theHelpParamBag);
	//{{AFX_DATA_INIT(CFmiParameterActivationDlg)
	itsParamname1U_ = _T("");
    itsParamname2U_ = _T("");
    itsParamname10U_ = _T("");
    itsParamname3U_ = _T("");
    itsParamname4U_ = _T("");
    itsParamname5U_ = _T("");
    itsParamname6U_ = _T("");
    itsParamname7U_ = _T("");
    itsParamname8U_ = _T("");
    itsParamname9U_ = _T("");
	fParamActivation1 = FALSE;
	fParamActivation10 = FALSE;
	fParamActivation2 = FALSE;
	fParamActivation3 = FALSE;
	fParamActivation4 = FALSE;
	fParamActivation5 = FALSE;
	fParamActivation6 = FALSE;
	fParamActivation7 = FALSE;
	fParamActivation8 = FALSE;
	fParamActivation9 = FALSE;
	fParamActivation11 = FALSE;
    itsParamname11U_ = _T("");
	fToggleAllParamState = FALSE;
	//}}AFX_DATA_INIT
}

CFmiParameterActivationDlg::~CFmiParameterActivationDlg(void)
{
	delete itsHelpParamBag;
}

void CFmiParameterActivationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiParameterActivationDlg)
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_1, itsParamname1U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_2, itsParamname2U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_10, itsParamname10U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_3, itsParamname3U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_4, itsParamname4U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_5, itsParamname5U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_6, itsParamname6U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_7, itsParamname7U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_8, itsParamname8U_);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_9, itsParamname9U_);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_1, fParamActivation1);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_10, fParamActivation10);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_2, fParamActivation2);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_3, fParamActivation3);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_4, fParamActivation4);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_5, fParamActivation5);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_6, fParamActivation6);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_7, fParamActivation7);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_8, fParamActivation8);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_9, fParamActivation9);
	DDX_Check(pDX, IDC_CHECK_ACTIVATION_11, fParamActivation11);
    DDX_Text(pDX, IDC_STATIC_PARAM_NAME_11, itsParamname11U_);
	DDX_Check(pDX, IDC_CHECK_TOGGLE_ALL_PARAMS, fToggleAllParamState);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiParameterActivationDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiParameterActivationDlg)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_1, OnButtonSubparams1)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_10, OnButtonSubparams10)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_2, OnButtonSubparams2)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_3, OnButtonSubparams3)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_4, OnButtonSubparams4)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_5, OnButtonSubparams5)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_6, OnButtonSubparams6)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_7, OnButtonSubparams7)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_8, OnButtonSubparams8)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_9, OnButtonSubparams9)
	ON_BN_CLICKED(IDC_BUTTON_SUBPARAMS_11, OnButtonSubparams11)
	ON_BN_CLICKED(IDC_BUTTON_HELP_PARAMS, OnButtonHelpParams)
	ON_BN_CLICKED(IDC_CHECK_TOGGLE_ALL_PARAMS, OnCheckToggleAllParams)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiParameterActivationDlg message handlers

void CFmiParameterActivationDlg::OnOK() 
{
	UpdateData(TRUE);
	itsParamBag.Reset();
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation1 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation2 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation3 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation4 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation5 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation6 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation7 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation8 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation9 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation10 == TRUE);
	if(itsParamBag.Next())
		itsParamBag.Current()->SetActive(fParamActivation11 == TRUE);
	
	CDialog::OnOK();
}

BOOL CFmiParameterActivationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

// käydään parampagin 10 ensimmäistä parametria läpi (hardcode, mutta tämä olisi hankala tehdä dynaamisesti)	
	itsParamBag.Reset();
	if(itsParamBag.Next())
	{
		fParamActivation1 = itsParamBag.Current()->IsActive();
		itsParamname1U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_1);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_1);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_1);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_1);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_1);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation2 = itsParamBag.Current()->IsActive();
        itsParamname2U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_2);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_2);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_2);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_2);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_2);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation3 = itsParamBag.Current()->IsActive();
        itsParamname3U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_3);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_3);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_3);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_3);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_3);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation4 = itsParamBag.Current()->IsActive();
        itsParamname4U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_4);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_4);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_4);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_4);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_4);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation5 = itsParamBag.Current()->IsActive();
        itsParamname5U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_5);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_5);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_5);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_5);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_5);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation6 = itsParamBag.Current()->IsActive();
        itsParamname6U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_6);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_6);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_6);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_6);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_6);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation7 = itsParamBag.Current()->IsActive();
        itsParamname7U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_7);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_7);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_7);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_7);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_7);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation8 = itsParamBag.Current()->IsActive();
        itsParamname8U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_8);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_8);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_8);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_8);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_8);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation9 = itsParamBag.Current()->IsActive();
        itsParamname9U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_9);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_9);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_9);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_9);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_9);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation10 = itsParamBag.Current()->IsActive();
        itsParamname10U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_10);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_10);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_10);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_10);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_10);
		window->ShowWindow(SW_HIDE);
	}
	if(itsParamBag.Next())
	{
		fParamActivation11 = itsParamBag.Current()->IsActive();
        itsParamname11U_ = CA2T(itsParamBag.CurrentParamName());
		CWnd* window1 = GetDlgItem(IDC_CHECK_ACTIVATION_11);
		CWnd* window2 = GetDlgItem(IDC_BUTTON_SUBPARAMS_11);
		if(itsParamBag.Current()->HasDataParams())
		{
			window1->EnableWindow(FALSE);
			window2->EnableWindow(TRUE);
		}
		else
		{
			if(itsParamBag.Current()->Type() == kSymbolicParam)
				window1->EnableWindow(FALSE);
			else
				window1->EnableWindow(TRUE);
			window2->ShowWindow(SW_HIDE);
		}
	}
	else // jos ei ole parametria, piilotetaan nappulat
	{
		CWnd* window = GetDlgItem(IDC_CHECK_ACTIVATION_11);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_BUTTON_SUBPARAMS_11);
		window->ShowWindow(SW_HIDE);
		window = GetDlgItem(IDC_STATIC_PARAM_NAME_11);
		window->ShowWindow(SW_HIDE);
	}

	if(!itsHelpParamBag)
	{
		CWnd* window = GetDlgItem(IDC_BUTTON_HELP_PARAMS);
		window->ShowWindow(SW_HIDE);
	}

	int paramCount = itsParamBag.GetSize();
	if(paramCount)
	{
		CWnd* window = GetDlgItem(IDC_STATIC_NO_PARAMS_WARNING);
		window->ShowWindow(SW_HIDE);
	}
	else
		paramCount = 2;
	
	CRect rect;
	GetWindowRect(rect);
	int topButtonSpace = 90;
	int paramRowHeight = 30;
	int height = topButtonSpace + paramCount * paramRowHeight;
	CRect newRect(rect.TopLeft().x, rect.TopLeft().y, rect.BottomRight().x, rect.TopLeft().y+height);
	MoveWindow(newRect);
	
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiParameterActivationDlg::OnButtonSubparams1() 
{
	if(itsParamBag.SetCurrentIndex(1-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams10() 
{
	if(itsParamBag.SetCurrentIndex(10-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams2() 
{
	if(itsParamBag.SetCurrentIndex(2-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams3() 
{
	if(itsParamBag.SetCurrentIndex(3-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams4() 
{
	if(itsParamBag.SetCurrentIndex(4-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams5() 
{
	if(itsParamBag.SetCurrentIndex(5-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams6() 
{
	if(itsParamBag.SetCurrentIndex(6-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams7() 
{
	if(itsParamBag.SetCurrentIndex(7-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams8() 
{
	if(itsParamBag.SetCurrentIndex(8-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams9() 
{
	if(itsParamBag.SetCurrentIndex(9-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonSubparams11() 
{
	if(itsParamBag.SetCurrentIndex(11-1)) // indeksi alkoikin 0:sta eikä ykkösestä, että muttujan nimiin tuli yksi liian suuret numerot
	{
		NFmiDataIdent* param = itsParamBag.Current();
		if(param->HasDataParams())
		{
			CFmiParameterActivationDlg dlg(*(itsParamBag.Current()->GetDataParams()));
			if(dlg.DoModal() == IDOK)
			{
				NFmiParamBag newActivityParamBag = dlg.ParamBag();
				for(param->ResetDataParams(), newActivityParamBag.Reset(); param->NextDataParam() && newActivityParamBag.Next(); )
				{
					param->CurrentDataParam().SetActive(newActivityParamBag.Current()->IsActive()); 
				}
			}
		}
	}
}

void CFmiParameterActivationDlg::OnButtonHelpParams() 
{
	if(itsHelpParamBag)
	{
		CFmiParameterActivationDlg dlg(*itsHelpParamBag);
		if(dlg.DoModal() == IDOK)
		{
			*itsHelpParamBag = dlg.ParamBag();
		}
	}
}

void CFmiParameterActivationDlg::OnCheckToggleAllParams() 
{
	UpdateData(TRUE);
	itsParamBag.SetActivities(fToggleAllParamState == TRUE);
	fParamActivation1 = fToggleAllParamState;
	fParamActivation2 = fToggleAllParamState;
	fParamActivation3 = fToggleAllParamState;
	fParamActivation4 = fToggleAllParamState;
	fParamActivation5 = fToggleAllParamState;
	fParamActivation6 = fToggleAllParamState;
	fParamActivation7 = fToggleAllParamState;
	fParamActivation8 = fToggleAllParamState;
	fParamActivation9 = fToggleAllParamState;
	fParamActivation10 = fToggleAllParamState;
	fParamActivation11 = fToggleAllParamState;
	UpdateData(FALSE);
}
