// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include "afxtempl.h"
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxcontrolbars.h>
#include <afxdlgs.h>


// TODO: reference additional headers your program requires here

#ifndef DISABLE_EXTREME_TOOLKITPRO
#define _MARKO_SKIP_THE_BLOODY_INCLUDING_TOOLKITPRO_MESSAGES // Muuten n�it� 'include' messuja tulee joka ikiselle k��nnetylle cpp-tiedostolle pitk� liuta...
#define _XTP_STATICLINK
#include <XTToolkitPro.h>   // Codejock Software Components
#endif // DISABLE_EXTREME_TOOLKITPRO