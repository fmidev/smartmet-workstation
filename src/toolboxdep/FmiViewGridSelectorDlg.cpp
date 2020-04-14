#include "stdafx.h"
#include "FmiViewGridSelectorDlg.h"
#include "FmiFilterDataView.h"
#include "NFmiGridViewSelectorView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMapViewDescTop.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiViewGridSelectorDlg dialog


CFmiViewGridSelectorDlg::CFmiViewGridSelectorDlg(int theMapViewDescTopIndex, SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
:CDialog(CFmiViewGridSelectorDlg::IDD, pParent)
,itsViewGridSelectorView(0)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsSelectedGridSize()
{
	//{{AFX_DATA_INIT(CFmiViewGridSelectorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFmiViewGridSelectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiViewGridSelectorDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiViewGridSelectorDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiViewGridSelectorDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiViewGridSelectorDlg message handlers

BOOL CFmiViewGridSelectorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
// ************ apu-ikkunan luonti ***************************
	CRect rect;
	CWnd* staticWnd = GetDlgItem(IDC_STATIC_VIEW_BASE);
	WINDOWPLACEMENT wplace;
	if(staticWnd)
		staticWnd->GetWindowPlacement(&wplace);
	rect = wplace.rcNormalPosition;

	rect.InflateRect(-1,-1,-1,-1); // 'hieno' upotus tulee näkyviin
	
	itsViewGridSelectorView = new CFmiFilterDataView(itsMapViewDescTopIndex, this, itsSmartMetDocumentInterface, 1, NFmiCtrlView::kGridViewSelectorView);
	itsViewGridSelectorView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsViewGridSelectorView->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
// ************ apu-ikkunan luonti ***************************

	InitDialogTexts();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiViewGridSelectorDlg::OnOK() 
{
	// helvetin rumat cast:it muuten!!! (Marko)
	NFmiGridViewSelectorView* view = (NFmiGridViewSelectorView*) ((CFmiFilterDataView*)itsViewGridSelectorView)->ZoomView();
	if(view)
		itsSelectedGridSize = view->SelectedGridViewSize();
	
	CDialog::OnOK();
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiViewGridSelectorDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("ViewGridSelectorDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
}
