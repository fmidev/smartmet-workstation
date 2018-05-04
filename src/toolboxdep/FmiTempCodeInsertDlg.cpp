// FmiTempCodeInsertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTempCodeInsertDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"

// CFmiTempCodeInsertDlg dialog

IMPLEMENT_DYNAMIC(CFmiTempCodeInsertDlg, CDialog)

CFmiTempCodeInsertDlg::CFmiTempCodeInsertDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiTempCodeInsertDlg::IDD, pParent)
	, itsSmartMetDocumentInterface(smartMetDocumentInterface)
	, itsTempCodeInputStrU_(_T(""))
	, itsTempCheckRaportStrU_(_T(""))
{

}

CFmiTempCodeInsertDlg::~CFmiTempCodeInsertDlg()
{
}

void CFmiTempCodeInsertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RICHEDIT_INPUT_TEMP_CODES, itsTempCodeInputStrU_);
	DDX_Text(pDX, IDC_STATIC_CHECK_MESSAGE_STR, itsTempCheckRaportStrU_);
}


BEGIN_MESSAGE_MAP(CFmiTempCodeInsertDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFmiTempCodeInsertDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_CLEAR_TEMPS, &CFmiTempCodeInsertDlg::OnBnClickedButtonCheckClearTemps)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_TEMP_TEXTS, &CFmiTempCodeInsertDlg::OnBnClickedButtonCheckTempTexts)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CFmiTempCodeInsertDlg message handlers

void CFmiTempCodeInsertDlg::OnBnClickedOk()
{
	// 1. pyyd� viimeisin TEMP-teksti richedit-kontrollilta
	UpdateData(TRUE);
	std::string tempCodeTextStr = CT2A(itsTempCodeInputStrU_);
	// 2. Anna se dokumentille ja k�ynnist� tarkistus ja datojen k�ytt��n otto
	std::string tempCodeCheckReportStr; // t�t� ei k�ytet� koska dialogi suljetaan
	CWaitCursor cursor;
    itsSmartMetDocumentInterface->DoTEMPDataUpdate(tempCodeTextStr, tempCodeCheckReportStr);
	OnOK();
}

void CFmiTempCodeInsertDlg::OnBnClickedButtonCheckClearTemps()
{
	// tyhjenn� dokumentista TEMP-data ja ruudulta teksti
	itsTempCodeInputStrU_ = _TEXT("");
	std::string tempCodeCheckReportStr; // t�t� ei k�ytet� koska dialogi suljetaan
	CWaitCursor cursor;
    itsSmartMetDocumentInterface->DoTEMPDataUpdate(std::string(CT2A(itsTempCodeInputStrU_)), tempCodeCheckReportStr);
    itsSmartMetDocumentInterface->ClearTEMPData();
	itsTempCheckRaportStrU_ = _TEXT("");
	UpdateData(FALSE);
}

void CFmiTempCodeInsertDlg::OnBnClickedButtonCheckTempTexts()
{
	// 1. pyyd� viimeisin TEMP-teksti richedit-kontrollilta
	UpdateData(TRUE);
	std::string tempCodeTextStr = CT2A(itsTempCodeInputStrU_);
	// 2. Anna se dokumentille ja k�ynnist� tarkistus 
	std::string tempCodeCheckReportStr;
	CWaitCursor cursor;
    itsSmartMetDocumentInterface->DoTEMPDataUpdate(tempCodeTextStr, tempCodeCheckReportStr, true);
	// 3. tulosta raportti raportti-ikkunaan
	itsTempCheckRaportStrU_ = _TEXT("bs"); // pit�� updeitata ensin jotain muuta v�lill�, koska muuten MFC yritt�� optimoida eik� p�ivit� virheteksti�
								// jos se oli sama kuin aiemmin. Paha vain, mutta ruutu on saattanut muuttaa kokoaan jolloin virhe tekstin aluekin on muuttunut,
								// joten homma pit�� pakottaa piirt�m��n uudestaan.
	UpdateData(FALSE);
    itsTempCheckRaportStrU_ = CA2T(tempCodeCheckReportStr.c_str());
	UpdateData(FALSE);
}

BOOL CFmiTempCodeInsertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

    itsTempCodeInputStrU_ = CA2T(itsSmartMetDocumentInterface->LastTEMPDataStr().c_str());
	InitDialogTexts();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiTempCodeInsertDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRect rect(CalcRichEditArea());
	CWnd* win = GetDescendantWindow(IDC_RICHEDIT_INPUT_TEMP_CODES);
	if(win)
		win->MoveWindow(rect, TRUE);

	CRect rect2(CalcCheckReportArea());
	CWnd* win2 = GetDescendantWindow(IDC_STATIC_CHECK_MESSAGE_STR);
	if(win2)
		win2->MoveWindow(rect2, TRUE);
}

CRect CFmiTempCodeInsertDlg::CalcCheckReportArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CWnd* win = GetDescendantWindow(IDC_STATIC_CHECK_MESSAGE_STR);
	if(win)
	{
		WINDOWPLACEMENT wplace;
		win->GetWindowPlacement(&wplace);
		CRect usedRect = wplace.rcNormalPosition;
		usedRect.right = rect.right;
		return usedRect;
	}
	return rect;
}

CRect CFmiTempCodeInsertDlg::CalcRichEditArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CWnd* win = GetDescendantWindow(IDOK);
	if(win)
	{
		WINDOWPLACEMENT wplace;
		win->GetWindowPlacement(&wplace);
		CRect lowRect = wplace.rcNormalPosition;
		rect.top = lowRect.bottom + 3;
	}
	return rect;
}

// T�m� funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell�.
// T�m� on ik�v� kyll� teht�v� erikseen dialogin muokkaus ty�kalusta, eli
// tekij�n pit�� lis�t� erikseen t�nne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiTempCodeInsertDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("TempCodeInsertDlg").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CHECK_CLEAR_TEMPS, "IDC_BUTTON_CHECK_CLEAR_TEMPS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CHECK_TEMP_TEXTS, "IDC_BUTTON_CHECK_TEMP_TEXTS");
}
