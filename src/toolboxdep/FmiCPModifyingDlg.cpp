// FmiCPModifyingDlg.cpp : implementation file
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "stdafx.h"
#include "FmiCPModifyingDlg.h"
#include "NFmiPoint.h"
#include "NFmiGrid.h"
#include "NFmiEditorControlPointManager.h"
#include "FmiCPLocationView.h"
#include "NFmiMovingCPLocationView.h"
#include "CloneBitmap.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiCPModifyingDlg dialog


CFmiCPModifyingDlg::CFmiCPModifyingDlg(NFmiEditorControlPointManager* theCPManager, const NFmiPoint& theLaLonPoint, const NFmiGrid* theDataGrid, CWnd* pParent /*=NULL*/)
:CDialog(CFmiCPModifyingDlg::IDD, pParent)
,itsCPManager(theCPManager)
,itsCurrentLaLonPoint(new NFmiPoint(theLaLonPoint))
,itsMovingCPLocationView(0)
,itsDataGrid(theDataGrid)
{
	itsCPManager->FindNearestCP(*itsCurrentLaLonPoint,true);
	//{{AFX_DATA_INIT(CFmiCPModifyingDlg)
	//}}AFX_DATA_INIT
}

CFmiCPModifyingDlg::~CFmiCPModifyingDlg(void)
{
	delete itsCurrentLaLonPoint;
}


void CFmiCPModifyingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	int cpMovingInTime = FALSE;
	if(!pDX->m_bSaveAndValidate)
	{
		cpMovingInTime = itsCPManager->IsCPMovingInTime();
	}
	//{{AFX_DATA_MAP(CFmiCPModifyingDlg)
	DDX_Check(pDX, IDC_CHECK_CHANGE_IN_TIME, cpMovingInTime);
	//}}AFX_DATA_MAP
	if(pDX->m_bSaveAndValidate)
	{
		itsCPManager->CPMovingInTime(cpMovingInTime==TRUE);
	}
	EnableLocationEditView(cpMovingInTime == TRUE);
}


BEGIN_MESSAGE_MAP(CFmiCPModifyingDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiCPModifyingDlg)
	ON_BN_CLICKED(IDC_CHECK_CHANGE_IN_TIME, OnCheckChangeInTime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiCPModifyingDlg message handlers

BOOL CFmiCPModifyingDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	CreateMovingCPLocationView();
	UpdateData(TRUE);	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiCPModifyingDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("Control point dialog").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "Close");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "Cancel");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_CHANGE_IN_TIME, "CP that changes in time");
}

void CFmiCPModifyingDlg::EnableLocationEditView(bool newState)
{
	if(itsMovingCPLocationView)
	{
		if(newState)
			itsMovingCPLocationView->ShowWindow(SW_SHOW);
		else
			itsMovingCPLocationView->ShowWindow(SW_HIDE);
	}
}

void CFmiCPModifyingDlg::CreateMovingCPLocationView(void)
{
	CRect rect;
	CWnd* staticWnd = GetDlgItem(IDC_STATIC_VIEW_FRAME);
	WINDOWPLACEMENT wplace;
	if(staticWnd)
		staticWnd->GetWindowPlacement(&wplace);
	rect = wplace.rcNormalPosition;

	rect.InflateRect(-1,-1,-1,-1); // 'hieno' upotus tulee näkyviin
	
	itsMovingCPLocationView = new CFmiCPLocationView(this, 0, 1, NFmiCtrlView::kMovingCPLocationView, itsCPManager);
	itsMovingCPLocationView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsMovingCPLocationView->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
}

void CFmiCPModifyingDlg::OnCheckChangeInTime() 
{
	UpdateData(TRUE);	
}

void CFmiCPModifyingDlg::OnOK() 
{
	NFmiMovingCPLocationView* cpView = ((CFmiCPLocationView*)itsMovingCPLocationView)->GetCPView();
	int xCount = 20;
	int yCount = 20;
	if(itsDataGrid)
	{
		xCount = itsDataGrid->XNumber();
		yCount = itsDataGrid->YNumber();
	}
	itsCPManager->SetInTimeMovingCPRelativeLocations(cpView->GetRelativeLocationVector(xCount, yCount));

	NFmiEditorControlPointManager::ThreePoints points;
	points.itsStartPoint = cpView->SplineStart();
	points.itsMiddlePoint = cpView->SplineMiddle();
	points.itsEndPoint = cpView->SplineEnd();
	itsCPManager->CPMovingInTimeHelpPoints(points);

	CDialog::OnOK();
}
