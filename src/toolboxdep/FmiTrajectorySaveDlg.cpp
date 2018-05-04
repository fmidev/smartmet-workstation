// FmiTrajectorySaveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTrajectorySaveDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiFileString.h"

// rich_edit ja edit kontrollien kanssa on ihme ongelmia
// rivinvaihtojen kanssa. T‰ss‰ poistetaan yksi ylim‰‰r‰inen
// cr (carriage return) joka n‰ytt‰‰ tulevan aina teksteihin
// joka rivin vaihdon yhteyteen. t‰m‰ tekee muutoksen
// <cr><cr><nl> -> <cr><nl>
static int StripExtraCarrigeReturns(CString &theStringU_)
{
	const TCHAR* oldStr = _TEXT("\r\r\n");
    const TCHAR* oldStr2 = _TEXT("\r\n");
    const TCHAR* newStr = _TEXT("\n");

    theStringU_.Replace(oldStr, newStr);
    return theStringU_.Replace(oldStr2, newStr);
}
// ja toisin p‰in, kun teksti‰ laitetaan takaisin edit-boxiin, ppit‰‰ siihen lis‰t‰
// se carriage return aina kun on new line.
static int AddExtraCarrigeReturns(CString &theStringU_)
{
    const TCHAR* oldStr = _TEXT("\n");
    const TCHAR* newStr = _TEXT("\r\n");

    return theStringU_.Replace(oldStr, newStr);
}


// CFmiTrajectorySaveDlg dialog

IMPLEMENT_DYNAMIC(CFmiTrajectorySaveDlg, CDialog)

CFmiTrajectorySaveDlg::CFmiTrajectorySaveDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiTrajectorySaveDlg::IDD, pParent)
    , itsLegendTextU_(_T(""))
	, itsSmartMetDocumentInterface(smartMetDocumentInterface)
{
}

CFmiTrajectorySaveDlg::~CFmiTrajectorySaveDlg()
{
}

void CFmiTrajectorySaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_TRAJECTORY_SAVE_LEGEND_TEXT, itsLegendTextU_);
}


BEGIN_MESSAGE_MAP(CFmiTrajectorySaveDlg, CDialog)
	ON_BN_CLICKED(IDC_TRAJECTORY_SAVE_AS, OnBnClickedTrajectorySaveAs)
END_MESSAGE_MAP()


// CFmiTrajectorySaveDlg message handlers

BOOL CFmiTrajectorySaveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    itsLegendTextU_ = CA2T(itsSmartMetDocumentInterface->TrajectorySystem()->LastTrajectoryLegendStr().c_str());
    ::AddExtraCarrigeReturns(itsLegendTextU_);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiTrajectorySaveDlg::OnOK()
{
	UpdateData(TRUE);

    ::StripExtraCarrigeReturns(itsLegendTextU_);
    std::string legendText = CT2A(itsLegendTextU_);
    auto trajectorySystem = itsSmartMetDocumentInterface->TrajectorySystem();
    trajectorySystem->LastTrajectoryLegendStr(legendText);
	try
	{
        trajectorySystem->SaveXML(trajectorySystem->MakeCurrentTrajectorySaveFileName());
	}
	catch(std::exception & e)
	{
		std::string dialogTitle("Trajectory save failed");
        itsSmartMetDocumentInterface->LogAndWarnUser(std::string(e.what()) + "\nTry generate trajectories again.", dialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, false);
	}
	catch(...)
	{
		std::string dialogTitle("Trajectory save failed");
        itsSmartMetDocumentInterface->LogAndWarnUser("Unknown error with trajectory save, \ntry generate trajectories again.", dialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, false);
	}

	CDialog::OnOK();
}

static bool GetSaveAsFileName(std::string &theFileName)
{
    static TCHAR BASED_CODE szFilter[] = _TEXT("Paths (*.*)|*.*|");

	static bool firstTime = true;
	static std::string lastSaveDirectory;

	if(firstTime)
	{
		firstTime = false;
		lastSaveDirectory = "c:\\";
	}

	// t‰ss‰ on mukana myˆs file pattern, joten se pit‰‰ ottaa pois ja sijoittaa wantedPath-muuttujaan pelkk‰ aloituspolku
	NFmiFileString fileString(lastSaveDirectory);
	NFmiString wantedPath(fileString.Device());
	wantedPath += fileString.Path();

	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	dlg.m_ofn.lpstrInitialDir = CA2T(wantedPath);
	if(dlg.DoModal() == IDOK)
	{
		lastSaveDirectory = CT2A(dlg.GetPathName());
		theFileName = lastSaveDirectory;
		return true;
	}
	lastSaveDirectory = CT2A(dlg.GetPathName());
	return false;
}


void CFmiTrajectorySaveDlg::OnBnClickedTrajectorySaveAs()
{
	UpdateData(TRUE);

    ::StripExtraCarrigeReturns(itsLegendTextU_);
    std::string legendText = CT2A(itsLegendTextU_);
    auto trajectorySystem = itsSmartMetDocumentInterface->TrajectorySystem();
    trajectorySystem->LastTrajectoryLegendStr(legendText);
	try
	{
		std::string filename;
		if(::GetSaveAsFileName(filename))
            trajectorySystem->SaveXML(filename);
	}
	catch(std::exception & e)
	{
		std::string dialogTitle("Trajectory save failed");
        itsSmartMetDocumentInterface->LogAndWarnUser(std::string(e.what()) + "\nTry generate trajectories again.", dialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, false);
	}
	catch(...)
	{
		std::string dialogTitle("Trajectory save failed");
        itsSmartMetDocumentInterface->LogAndWarnUser("Unknown error with trajectory save, \ntry generate trajectories again.", dialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, false);
	}

	CDialog::OnOK();
}



