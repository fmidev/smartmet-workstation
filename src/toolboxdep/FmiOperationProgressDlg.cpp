// FmiOperationProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiOperationProgressDlg.h"


// CFmiOperationProgressDlg dialog

IMPLEMENT_DYNAMIC(CFmiOperationProgressDlg, CDialog)

CFmiOperationProgressDlg::CFmiOperationProgressDlg(NFmiStopFunctor &theStopper, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiOperationProgressDlg::IDD, pParent)
	, itsOperationStrU_(_T(""))
	, itsStopper(theStopper)
{

}

CFmiOperationProgressDlg::~CFmiOperationProgressDlg()
{
}

void CFmiOperationProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_OPERATION1, itsProgressCtrl);
    DDX_Text(pDX, IDC_STATIC_CANCEL_DLG_TEXT, itsOperationStrU_);
}


BEGIN_MESSAGE_MAP(CFmiOperationProgressDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CFmiOperationProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CFmiOperationProgressDlg message handlers

void CFmiOperationProgressDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

BOOL CFmiOperationProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	itsProgressCtrl.SetRange(0, 100);
	itsProgressCtrl.SetStep(1);
	itsProgressCtrl.SetShowPercent(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiOperationProgressDlg::OnOK()
{
	// Ei tehd� t��ll� mit��n.

//	CDialog::OnOK();
}

void CFmiOperationProgressDlg::OnCancel()
{
	// Cancel napin painallus laittaa threadille tiedon lopetuksesta
	itsStopper.Stop(true);

//	CDialog::OnCancel();
}

BOOL CFmiOperationProgressDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if(message == ID_MESSAGE_WORKING_THREAD_COMPLETED)
		CDialog::OnOK();
	if(message == ID_MESSAGE_WORKING_THREAD_CANCELED)
		CDialog::OnCancel();

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

void CFmiOperationProgressDlg::StepIt(void)
{
	itsProgressCtrl.StepIt();
}

void CFmiOperationProgressDlg::SetRange(int low, int high, int stepCount)
{
	itsProgressCtrl.SetRange(low, high);
	itsProgressCtrl.SetStep(stepCount);
}

void CFmiOperationProgressDlg::AddRange(int value)
{
	int low = 0;
	int high = 0;
	itsProgressCtrl.GetRange(low, high);
	high += value;
	itsProgressCtrl.SetRange(low, high);
}

bool CFmiOperationProgressDlg::DoPostMessage(unsigned int message, unsigned int wParam, long lParam)
{
	// Ty�-threadi l�ynnistet��n ensin, sitten tehd��n dialogille DoModal, jos ty� kest�� niin v�h�n aikaa
	// ett� ikkuna ei ole initialisoitu, kaataa PostMessage ohjelman.
	// siksi loopissa testataan onko GetSafeHwnd -metodin palauttama arvo ok. 
	// Jos ei, odota pikkuisen ja yrit� uudestaan...
	for(int i=0 ; GetSafeHwnd() == NULL; i++)
	{
		if(i > 200)
			return false; // t�ll� estet��n iki-looppi
		Sleep(10);
	}

	return PostMessage(static_cast<UINT>(message), static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam)) == TRUE;
}

bool CFmiOperationProgressDlg::WaitUntilInitialized(void)
{
	// ProgressCancel-dialogi avataan omassa threadissa, josta k�ytt�j� voi kesteytt��
	// toiminnon. Dialogin metodeja ei saa kutsua liian aikaisin, siksi tein
	// t�m�n tarkastus funktion, jotta tiedet��n voiko dialogia k�ytt�� jo.
	// Jos palauttaa false:n, ei pid� jatkaa.
	// siksi loopissa testataan onko GetSafeHwnd -metodin palauttama arvo ok. 
	// Jos ei, odota pikkuisen ja yrit� uudestaan...
	for(int i=0 ; GetSafeHwnd() == NULL; i++)
	{
		if(i > 200)
			return false;
		Sleep(10);
	}
    // Lis�ksi pit�� varmistaa ett� itsProgressCtrl on my�s alustettu!
    for(int i = 0; itsProgressCtrl.GetSafeHwnd() == NULL; i++)
    {
        if(i > 200)
            return false;
        Sleep(10);
    }
    return true;
}

