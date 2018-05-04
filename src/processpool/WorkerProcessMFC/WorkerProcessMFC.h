
// WorkerProcessMFC.h : main header file for the WorkerProcessMFC application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "ProgramOptions.h"

// CWorkerProcessMFCApp:
// See WorkerProcessMFC.cpp for the implementation of this class
//

class CWorkerProcessMFCApp : public CWinApp
{
public:
	CWorkerProcessMFCApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
    WorkerProcessMFCOptions mWorkerOptions;

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

private:
};

extern CWorkerProcessMFCApp theApp;
