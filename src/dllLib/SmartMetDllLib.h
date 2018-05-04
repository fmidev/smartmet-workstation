// SmartMetDllLib.h : main header file for the SmartMetDllLib DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSmartMetDllLibApp
// See SmartMetDllLib.cpp for the implementation of this class
//

class CSmartMetDllLibApp : public CWinApp
{
public:
	CSmartMetDllLibApp();
	~CSmartMetDllLibApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
