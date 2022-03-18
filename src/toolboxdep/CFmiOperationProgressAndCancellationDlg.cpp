// CFmiOperationProgressAndCancellationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CFmiOperationProgressAndCancellationDlg.h"


// CFmiOperationProgressAndCancellationDlg dialog

IMPLEMENT_DYNAMIC(CFmiOperationProgressAndCancellationDlg, CDialogEx)

CFmiOperationProgressAndCancellationDlg::CFmiOperationProgressAndCancellationDlg(const std::string& operationText, bool operationTextIsWarning, NFmiStopFunctor& theStopper, CWnd* pParent)
	: CDialogEx(IDD_DIALOG_OPERATION_PROGRESS_AND_CANCEL, pParent)
	, itsOperationRelatedStrU_(_T(""))
	, itsStopper(theStopper)
	, fOperationTextIsWarning(operationTextIsWarning)
{
	if(!operationText.empty())
	{
		itsOperationRelatedStrU_ = CA2T(operationText.c_str());
	}
}

CFmiOperationProgressAndCancellationDlg::~CFmiOperationProgressAndCancellationDlg()
{
}

void CFmiOperationProgressAndCancellationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_OPERATION_PROGRESS_STR, itsOperationRelatedStrU_);
	DDX_Control(pDX, IDC_PROGRESS_OF_OPERATION_BAR, itsOperationProgressBar);
}


BEGIN_MESSAGE_MAP(CFmiOperationProgressAndCancellationDlg, CDialogEx)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CFmiOperationProgressAndCancellationDlg message handlers


void CFmiOperationProgressAndCancellationDlg::OnOK()
{
	// Ei tehd‰ t‰‰ll‰ mit‰‰n.

//	CDialogEx::OnOK();
}


void CFmiOperationProgressAndCancellationDlg::OnCancel()
{
	// Cancel napin painallus laittaa threadille tiedon lopetuksesta
	itsStopper.Stop(true);

	//	CDialog::OnCancel();
}


BOOL CFmiOperationProgressAndCancellationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	itsOperationProgressBar.SetRange(0, 100);
	itsOperationProgressBar.SetStep(1);
	itsOperationProgressBar.SetShowPercent(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiOperationProgressAndCancellationDlg::StepIt(void)
{
	itsOperationProgressBar.StepIt();
}

void CFmiOperationProgressAndCancellationDlg::SetRange(int low, int high, int stepCount)
{
	itsOperationProgressBar.SetRange(low, high);
	itsOperationProgressBar.SetStep(stepCount);
}

void CFmiOperationProgressAndCancellationDlg::AddRange(int value)
{
	int low = 0;
	int high = 0;
	itsOperationProgressBar.GetRange(low, high);
	high += value;
	itsOperationProgressBar.SetRange(low, high);
}

static bool WaitForWindowToInitialize(CWnd* checkedWindow)
{
	// Progress dialogi k‰ynnistet‰‰n p‰‰-threadista, mutta sen kanssa operoidaan
	// tyˆ-threadista k‰sin. T‰m‰n funktion tarkoitus on varmistaa ett‰ tietyt
	// ikkunat ja kontrollit on alustettu, ennen kuin niit‰ oikeasti k‰ytet‰‰n.
	// T‰m‰ varmistus tehd‰‰n loopissa jossa testataan onko GetSafeHwnd -metodin palauttama arvo ok. 
	// Jos ei, odota pikkuisen ja yrit‰ uudestaan...
	// Jos ikkuna on lopuksi alustettu palauttaa true, mutta jos tarpeeksi monen yrityksen j‰lkeen homma 
	// ei ole valmis, palautetaan false, jotta ei j‰‰d‰ iki-looppiin.
	if(checkedWindow)
	{
		for(int i = 0; checkedWindow->GetSafeHwnd() == NULL; i++)
		{
			if(i > 200)
				return false; // t‰ll‰ estet‰‰n iki-looppi
			Sleep(10);
		}
		return true;
	}
	return false;
}

bool CFmiOperationProgressAndCancellationDlg::DoPostMessage(unsigned int message, unsigned int wParam, long lParam)
{
	if(::WaitForWindowToInitialize(this))
	{
		return PostMessage(static_cast<UINT>(message), static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam)) == TRUE;
	}

	return false;
}

bool CFmiOperationProgressAndCancellationDlg::WaitUntilInitialized(void)
{
	// Pit‰‰ varmistaa ett‰ dialogi on alustettu
	if(::WaitForWindowToInitialize(this))
	{
		// Lis‰ksi pit‰‰ varmistaa ett‰ itsProgressCtrl on myˆs alustettu!
		if(::WaitForWindowToInitialize(&itsOperationProgressBar))
		{
			return true;
		}
	}

	return false;
}


BOOL CFmiOperationProgressAndCancellationDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if(message == ID_MESSAGE_WORKING_THREAD_COMPLETED)
		CDialog::OnOK();
	if(message == ID_MESSAGE_WORKING_THREAD_CANCELED)
		CDialog::OnCancel();

	return CDialogEx::OnWndMsg(message, wParam, lParam, pResult);
}

HBRUSH CFmiOperationProgressAndCancellationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// V‰rj‰t‰‰n operaatioon liittyv‰ teksti punaisella, jos se on varoitus 
	// ja mustalla, jos ei.
	if(pWnd->GetDlgCtrlID() == IDC_STATIC_OPERATION_PROGRESS_STR)
	{
		if(fOperationTextIsWarning)
			pDC->SetTextColor(RGB(255, 0, 0));
		else
			pDC->SetTextColor(RGB(0, 0, 0));
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
