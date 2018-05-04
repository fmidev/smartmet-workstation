// SmartMetAtlUtils.h : main header file for the SmartMetAtlUtils DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "SmartMetAtlUtils_resource.h"		// main symbols


// CSmartMetAtlUtilsApp
// See SmartMetAtlUtils.cpp for the implementation of this class
//

class CSmartMetAtlUtilsApp : public CWinApp
{
public:
	CSmartMetAtlUtilsApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
