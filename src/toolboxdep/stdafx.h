// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// T‰m‰ stdint vs intsafe -viritys est‰‰ lukemattomien Visual C++ 2010 header vikojen aiheuttamia varoituksia,
// joka on tarkoitus korjata tulevissa p‰ivityksiss‰.
#if _MSC_VER == 1600 // T‰m‰ korjaus tarvitaan vain Visual C++ 2010 k‰‰nt‰j‰ss‰ (= 1600, SP1 ei korjannut onhelmaa), vanhemmissa versioissa ei ole koko stdint.h -headeria
#define _INTSAFE_H_INCLUDED_
#include <stdint.h>
#endif

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

#define _MARKO_SKIP_THE_BLOODY_INCLUDING_TOOLKITPRO_MESSAGES // Muuten n‰it‰ 'include' messuja tulee joka ikiselle k‰‰nnetylle cpp-tiedostolle pitk‰ liuta...
#define _XTP_STATICLINK
#include <XTToolkitPro.h>   // Codejock Software Components
