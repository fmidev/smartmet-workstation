#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCaseStudyExeApp:
// See CaseStudyExe.cpp for the implementation of this class
//

class CCaseStudyExeApp : public CWinAppEx
{
public:
	CCaseStudyExeApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCaseStudyExeApp theApp;