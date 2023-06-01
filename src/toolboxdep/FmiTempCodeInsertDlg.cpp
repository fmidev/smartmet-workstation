// FmiTempCodeInsertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTempCodeInsertDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiPathUtils.h"
#include "NFmiFileSystem.h"

const long gKiloByte = 1024;
const long gMegaByte = gKiloByte * gKiloByte;
const long gMaxTempDataTextLength = 200 * gMegaByte;

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
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_TEMP_TEXTS_FILE, &CFmiTempCodeInsertDlg::OnBnClickedButtonBrowseTempTextsFile)
END_MESSAGE_MAP()


// CFmiTempCodeInsertDlg message handlers

void CFmiTempCodeInsertDlg::OnBnClickedOk()
{
	// 1. pyydä viimeisin TEMP-teksti richedit-kontrollilta
	UpdateData(TRUE);
	std::string tempCodeTextStr = CT2A(itsTempCodeInputStrU_);
	// 2. Anna se dokumentille ja käynnistä tarkistus ja datojen käyttöön otto
	std::string tempCodeCheckReportStr; // tätä ei käytetä koska dialogi suljetaan
	CWaitCursor cursor;
    try
    {
        itsSmartMetDocumentInterface->DoTEMPDataUpdate(tempCodeTextStr, tempCodeCheckReportStr);
    }
    catch(std::exception &e)
    {
        // Pakko napata mahdolliset poikkeukset kiinni, koska jos modaalittoman dialogi heittää poikkeuksen tässä kohtaa, 
        // jää pääohjelma jumiin, kun poikkeus otetaan kiinni CSmartMetApp::Run metodissa
        std::string errorMessage = "Error while trying to construct TEMP based sounding data: ";
        errorMessage += e.what();
        itsSmartMetDocumentInterface->LogAndWarnUser(errorMessage, "", CatLog::Severity::Error, CatLog::Category::Data, true, false, true);
    }

	OnOK();
}

void CFmiTempCodeInsertDlg::OnBnClickedButtonCheckClearTemps()
{
	// tyhjennä dokumentista TEMP-data ja ruudulta teksti
	itsTempCodeInputStrU_.Empty();
    itsSmartMetDocumentInterface->ClearTEMPData();
	itsTempCheckRaportStrU_.Empty();
	UpdateData(FALSE);
}

void CFmiTempCodeInsertDlg::OnBnClickedButtonCheckTempTexts()
{
	// 1. pyydä viimeisin TEMP-teksti richedit-kontrollilta
	UpdateData(TRUE);
	std::string tempCodeTextStr = CT2A(itsTempCodeInputStrU_);
	// 2. Anna se dokumentille ja käynnistä tarkistus 
	std::string tempCodeCheckReportStr;
	CWaitCursor cursor;
    itsSmartMetDocumentInterface->DoTEMPDataUpdate(tempCodeTextStr, tempCodeCheckReportStr, true);
	// 3. tulosta raportti raportti-ikkunaan
	itsTempCheckRaportStrU_ = _TEXT("bs"); // pitää updeitata ensin jotain muuta välillä, koska muuten MFC yrittää optimoida eikä päivitä virhetekstiä
								// jos se oli sama kuin aiemmin. Paha vain, mutta ruutu on saattanut muuttaa kokoaan jolloin virhe tekstin aluekin on muuttunut,
								// joten homma pitää pakottaa piirtämään uudestaan.
	UpdateData(FALSE);
    itsTempCheckRaportStrU_ = CA2T(tempCodeCheckReportStr.c_str());
	UpdateData(FALSE);
}

BOOL CFmiTempCodeInsertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	itsTempCodeInputStrU_ = CA2T(itsSmartMetDocumentInterface->LastTEMPDataStr().c_str());
	InitDialogTexts();
	// CRichEditCtrl luokan tekstinsyöttökentän oletuskoko on jotain 64k, pitää kasvattaa
	// koska uudet Wyoming bufr CSV datat ovat niin mielettömän isoja (yhdessä luotauksessa 3000+ leveliä)
	auto richEditCtrl = static_cast<CRichEditCtrl*>(GetDlgItem(IDC_RICHEDIT_INPUT_TEMP_CODES));
	if(richEditCtrl)
	{
		richEditCtrl->LimitText(gMaxTempDataTextLength + 5);
	}
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

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiTempCodeInsertDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("TempCodeInsertDlg").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CHECK_CLEAR_TEMPS, "IDC_BUTTON_CHECK_CLEAR_TEMPS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CHECK_TEMP_TEXTS, "IDC_BUTTON_CHECK_TEMP_TEXTS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BROWSE_TEMP_TEXTS_FILE, "Browse...");
}


void CFmiTempCodeInsertDlg::OnBnClickedButtonBrowseTempTextsFile()
{
	static TCHAR BASED_CODE szFilter[] = _TEXT("TEMP CSV data files (*.csv)|*.csv|TEMP txt data files (*.txt)|*.txt|All Files (*.*)|*.*||");
	static std::string lastLoadedFilePath;

	UpdateData(TRUE);
	std::string initialDirectory = PathUtils::getPathSectionFromTotalFilePath(lastLoadedFilePath);
	CFileDialog dlg(TRUE, NULL, CA2T(lastLoadedFilePath.c_str()), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = CA2T(initialDirectory.c_str());
	if(dlg.DoModal() == IDOK)
	{
		lastLoadedFilePath = CT2A(dlg.GetPathName());
		std::string fileContent;
		if(NFmiFileSystem::ReadFile2String(lastLoadedFilePath, fileContent, gMaxTempDataTextLength))
		{
			itsTempCodeInputStrU_ = CA2T(fileContent.c_str());
		}
		else
		{
			itsTempCheckRaportStrU_ = _TEXT("Unable to read data from wanted file:\n");
			itsTempCheckRaportStrU_ += CA2T(lastLoadedFilePath.c_str());
		}
		UpdateData(FALSE);
	}
}
