
// WorkerProcessTcpMFC.h : main header file for the WorkerProcessTcpMFC application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "ProgramOptions.h"

// CWorkerProcessTcpMFCApp:
// See WorkerProcessTcpMFC.cpp for the implementation of this class
//

class CWorkerProcessTcpMFCApp : public CWinApp
{
public:
	CWorkerProcessTcpMFCApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
    WorkerProcessMFCOptions mWorkerOptions;

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CWorkerProcessTcpMFCApp theApp;
