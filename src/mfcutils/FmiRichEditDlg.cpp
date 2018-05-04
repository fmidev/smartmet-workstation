// FmiRichEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiRichEditDlg.h"


// CFmiRichEditDlg dialog

IMPLEMENT_DYNAMIC(CFmiRichEditDlg, CDialog)
CFmiRichEditDlg::CFmiRichEditDlg(const std::string theCaption, const std::string &theRichEditStr, CWnd* pParent /*=NULL*/)
:CDialog(CFmiRichEditDlg::IDD, pParent)
,itsCaption(theCaption)
,itsRichEditStr(theRichEditStr)
{
}

CFmiRichEditDlg::~CFmiRichEditDlg()
{
}

void CFmiRichEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_TEXT_SECTION, itsRichEditCtrl);
}


BEGIN_MESSAGE_MAP(CFmiRichEditDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CFmiRichEditDlg message handlers

BOOL CFmiRichEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetRichEditFont();
	SetWindowText(CA2T(itsCaption.c_str()));
    itsRichEditCtrl.SetWindowText(CA2T(itsRichEditStr.c_str()));
	itsRichEditCtrl.SetSel(0, 0); // ei toimi, valitsee aina kaiken tekstin ja juoksuttaa kursoerin loppuun
	tagPOINT bs;
	bs.x = 0;
	bs.y = 0;
	itsRichEditCtrl.SetCaretPos(bs);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiRichEditDlg::SetRichEditFont(void)
{
	CFont myFont;
	myFont.CreateFont(16
					,0 /* width */
					,0 // escapement (angle in 0.1 degrees)
					,1 // orientation (angle in 0.1 degrees)
					,FW_NORMAL // weight (FW_NORMAL, FW_BOLD jne.)
					,0 // italic
					,0 // underline
					,0 // strikeout
					,1 /* char set */
					,OUT_TT_PRECIS//040397 //291096/LW oli 1
					,1
					,PROOF_QUALITY
					,DEFAULT_PITCH /* FF_DONTCARE*/ /* 1 == FIXED_PITCH */ // 29.9.1998/Marko&Persa
					,_TEXT("Courier New")); //040397/LW oli NULL

	itsRichEditCtrl.SetFont(&myFont);
}

void CFmiRichEditDlg::SetRichEditToClientArea(void)
{
	if(itsRichEditCtrl.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(rect);
		itsRichEditCtrl.MoveWindow(rect, FALSE);
	}
}

void CFmiRichEditDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	SetRichEditToClientArea();
}

BOOL CFmiRichEditDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

//	return CDialog::OnEraseBkgnd(pDC);
}
