
// CaseStudyExe.cpp : Defines the class behaviors for the application.
////

#include "stdafx.h"
#include "CaseStudyExe.h"
#include "CaseStudyExeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCaseStudyExeApp

BEGIN_MESSAGE_MAP(CCaseStudyExeApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCaseStudyExeApp construction

CCaseStudyExeApp::CCaseStudyExeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CCaseStudyExeApp object

CCaseStudyExeApp theApp;


// CCaseStudyExeApp initialization

BOOL CCaseStudyExeApp::InitInstance()
{
	CWinAppEx::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	std::string simpleCommandLineStr = CT2A(cmdInfo.m_strFileName);
	CCaseStudyExeDlg dlg(simpleCommandLineStr);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
