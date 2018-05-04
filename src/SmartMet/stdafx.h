
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// T‰m‰ stdint vs intsafe -viritys est‰‰ lukemattomien Visual C++ 2010 header vikojen aiheuttamia varoituksia,
// joka on tarkoitus korjata tulevissa p‰ivityksiss‰.
#if _MSC_VER == 1600 // T‰m‰ korjaus tarvitaan vain Visual C++ 2010 k‰‰nt‰j‰ss‰ (= 1600, SP1 ei korjannut onhelmaa), vanhemmissa versioissa ei ole koko stdint.h -headeria
#define _INTSAFE_H_INCLUDED_
#include <stdint.h>
#endif

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <afxsock.h>            // MFC socket extensions
#endif // FMI_DISABLE_MFC_FEATURE_PACK

