// FmiSmartToolLoadingDlg.cpp : implementation file
//
#ifdef _MSC_VER
#pragma warning(disable : 4996) // poistaa varoituksen "warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead"
#endif

#include "stdafx.h"
#include "FmiSmartToolLoadingDlg.h"
#include "NFmiSmartToolInfo.h"
#include "NFmiStringTools.h"
#include "NFmiFileSystem.h"
#include <vector>
#include "fmiGetDirectoryNameDlg.h"
#include <direct.h>
#include "CloneBitmap.h"
#include "FmiWin32Helpers.h"

using namespace std;
// CFmiSmartToolLoadingDlg dialog

CRect CFmiSmartToolLoadingDlg::itsStartingPlacement;
int CFmiSmartToolLoadingDlg::itsLastLoadedMacroIndex = LB_ERR;

IMPLEMENT_DYNAMIC(CFmiSmartToolLoadingDlg, CDialog)
CFmiSmartToolLoadingDlg::CFmiSmartToolLoadingDlg(NFmiSmartToolInfo *theSmartToolInfo, CWnd* pParent /*=NULL*/)
:CDialog(CFmiSmartToolLoadingDlg::IDD, pParent)
,itsSmartToolInfo(theSmartToolInfo)
{
}

CFmiSmartToolLoadingDlg::~CFmiSmartToolLoadingDlg()
{
}

void CFmiSmartToolLoadingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PARAM_MACROS, itsScriptNameList);
}


BEGIN_MESSAGE_MAP(CFmiSmartToolLoadingDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
	ON_LBN_DBLCLK(IDC_LIST_PARAM_MACROS, OnLbnDblclkListParamMacros)
	ON_BN_CLICKED(IDC_BUTTON_ADD_DIRECTORY, OnBnClickedButtonAddDirectory)
	ON_WM_TIMER()
    ON_WM_SIZE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CFmiSmartToolLoadingDlg message handlers

void CFmiSmartToolLoadingDlg::OnBnClickedOk()
{
	itsSelectedScriptName = "";
	UpdateData(TRUE);
	int index = itsScriptNameList.GetCurSel();
	if(index != LB_ERR)
	{
        CString nameU_;
		itsScriptNameList.GetText(index, nameU_);
		itsSelectedScriptName = CT2A(nameU_);
        CFmiSmartToolLoadingDlg::itsLastLoadedMacroIndex = index;
	}
	OnOK();
}

void CFmiSmartToolLoadingDlg::OnBnClickedButtonRemove()
{
	UpdateData(TRUE);
	int index = itsScriptNameList.GetCurSel();
	if(index != LB_ERR)
	{
        CString nameU_;
		itsScriptNameList.GetText(index, nameU_);
		string scriptName = CT2A(nameU_);
		if(scriptName[0] == '<')
		{ // hakemiston poisto
			string tmp(scriptName);
			NFmiStringTools::TrimL(tmp, '<');
			NFmiStringTools::TrimR(tmp, '>');
			string totalPath = itsSmartToolInfo->LoadDirectory();
			totalPath += tmp;
			list<string> fileList = NFmiFileSystem::DirectoryFiles(totalPath);
			if(fileList.empty())
			{
				if(::rmdir(totalPath.c_str()) != 0)
                    ::MessageBox(this->GetSafeHwnd(), _TEXT("Directory deletion failed!"), _TEXT("Cannot remove wanted directory!"), MB_OK | MB_ICONERROR);
			}
			else
                ::MessageBox(this->GetSafeHwnd(), _TEXT("There are macros in the directory, you have to remove \nthem first before you can remove directory!"), _TEXT("Cannot remove wanted directory!"), MB_OK | MB_ICONERROR);
			UpdateScriptNameList();
			UpdateData(FALSE);
		}
		else
		{
			if(itsSmartToolInfo->ScriptExist(scriptName))
			{
                if (::MessageBox(this->GetSafeHwnd(), CA2T(string(string("Do you want to remove: '") + scriptName + string("'?")).c_str()), _TEXT("Macro remove"), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
				{
					itsSmartToolInfo->RemoveScript(scriptName);
					UpdateScriptNameList();
                    CFmiSmartToolLoadingDlg::itsLastLoadedMacroIndex = index - 1; // Laitetaan muisti osoittamaan poistettua viimeist‰ edelliseen makroon
                    UpdateData(FALSE);
				}
			}
		}
	}
}

BOOL CFmiSmartToolLoadingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	UpdateScriptNameList();
    itsScriptNameList.SetCurSel(CFmiSmartToolLoadingDlg::itsLastLoadedMacroIndex); // Laitetaan muistista dialogi osoittamaan viimeksi ladattua makroa

	UpdateData(FALSE);

    if(CFmiSmartToolLoadingDlg::itsStartingPlacement.Width() && CFmiSmartToolLoadingDlg::itsStartingPlacement.Height())
        MoveWindow(CFmiSmartToolLoadingDlg::itsStartingPlacement);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiSmartToolLoadingDlg::UpdateScriptNameList(void)
{
	static const std::string cvsDirectoryName("<CVS>");
	itsScriptNameList.ResetContent();
	auto stringVector = itsSmartToolInfo->GetScriptNames();
	for(const auto &name : stringVector)
	{
		// ignoorataan CVS hakemisto
		if(name != cvsDirectoryName)
			itsScriptNameList.AddString(CA2T(name.c_str()));
	}
}

void CFmiSmartToolLoadingDlg::OnLbnDblclkListParamMacros()
{
	UpdateData(TRUE);
	int index = itsScriptNameList.GetCurSel();
	if(index != LB_ERR)
	{
        CString nameU_;
		itsScriptNameList.GetText(index, nameU_);
		std::string tmpName = CT2A(nameU_);
		if(tmpName[0] == '<')
		{	// koska nyt on tupla klikattu hakemistoa, siirryt‰‰n sinne
			itsSmartToolInfo->SetCurrentLoadDirectory(tmpName);
			UpdateScriptNameList();
		}
		else // t‰ll‰ timer kikalla estet‰‰n double clikin j‰lkeinen left-button up sanoma alla olevassa ikkunassa, kun t‰m‰ ei heti sulkeudukaan tupla klikkiin, vaan vasta 5 sadasosa sekunnin kuluttua
			itsCloseAfterDblClick = static_cast<UINT>(SetTimer(kFmiCloseAfterDblClick, 100, NULL));
	}
}

bool CFmiSmartToolLoadingDlg::CreateNewSmartToolDirectory(const std::string &thePath)
{
	std::string usedDirectoryName(thePath);
	NFmiStringTools::TrimL(usedDirectoryName, '<');
	NFmiStringTools::TrimR(usedDirectoryName, '>');
	if(NFmiFileSystem::CreateDirectory(itsSmartToolInfo->LoadDirectory() + usedDirectoryName))
	{
		return true;
	}
	return false;
}

void CFmiSmartToolLoadingDlg::OnBnClickedButtonAddDirectory()
{
	CfmiGetDirectoryNameDlg dlg;
	if(dlg.DoModal() == IDOK)
	{
		std::string tmp = CT2A(dlg.DirectoryName());
		CreateNewSmartToolDirectory(tmp);
		UpdateScriptNameList();
	}
}

#ifdef _WIN64
void CFmiSmartToolLoadingDlg::OnTimer(UINT_PTR nIDEvent)
#else
void CFmiSmartToolLoadingDlg::OnTimer(UINT nIDEvent)
#endif
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == kFmiCloseAfterDblClick)
		OnBnClickedOk();

	CDialog::OnTimer(nIDEvent);
}


void CFmiSmartToolLoadingDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if(itsScriptNameList.GetSafeHwnd() != NULL)
    {
	    CRect rect(CalcClientArea());
	    itsScriptNameList.MoveWindow(rect, TRUE);
    }
}

CRect CFmiSmartToolLoadingDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CWnd* win = GetDescendantWindow(IDC_BUTTON_ADD_DIRECTORY);
	if(win)
	{
		WINDOWPLACEMENT wplace;
		win->GetWindowPlacement(&wplace);
		CRect leftRect = wplace.rcNormalPosition;
        rect.left = leftRect.right + 3;
	}
	return rect;
}

void CFmiSmartToolLoadingDlg::DoWhenClosing(void)
{
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	// gets current window position
    BOOL bRet = GetWindowPlacement(&wndpl);
    CFmiSmartToolLoadingDlg::itsStartingPlacement = wndpl.rcNormalPosition;
}


void CFmiSmartToolLoadingDlg::OnCancel()
{
    DoWhenClosing();

    CDialog::OnCancel();
}


void CFmiSmartToolLoadingDlg::OnOK()
{
    DoWhenClosing();

    CDialog::OnOK();
}


void CFmiSmartToolLoadingDlg::OnClose()
{
    DoWhenClosing();

    CDialog::OnClose();
}
