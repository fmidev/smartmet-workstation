#pragma once
// Minimal Windows/MFC type compatibility layer for Linux builds.
// Replaces the types normally provided by <afxwin.h> and Win32 SDK.
// Full MFC functionality requires porting to Qt (in progress).

#ifdef UNIX

#include <string>
#include <cstring>

// Basic Windows integer types
using BOOL      = int;
using UINT      = unsigned int;
using DWORD     = unsigned long;
using LONG      = long;
using WORD      = unsigned short;
using BYTE      = unsigned char;
using LONGLONG  = long long;
using ULONGLONG = unsigned long long;
using PUINT     = UINT*;
using LPVOID    = void*;
using LPCVOID   = const void*;
using LPSTR     = char*;
using LPCSTR    = const char*;

// On Linux wchar_t strings — keep as-is, define the Windows aliases
using WCHAR   = wchar_t;
using LPWSTR  = wchar_t*;
using LPCWSTR = const wchar_t*;
// TCHAR maps to char in Linux builds (UTF-8)
using TCHAR   = char;
using LPTSTR  = char*;
using LPCTSTR = const char*;

// Boolean constants
#ifndef TRUE
constexpr int TRUE  = 1;
constexpr int FALSE = 0;
#endif

// Opaque handle types
using HWND     = void*;
using HANDLE   = void*;
using HBITMAP  = void*;
using HGDIOBJ  = void*;
using HBRUSH   = void*;
using HPEN     = void*;
using HFONT    = void*;
using HICON    = void*;
using HCURSOR  = void*;
using HMODULE  = void*;
using HINSTANCE = void*;
using HDC      = void*;
using HMENU    = void*;
using HRGN     = void*;
using LPUINT   = UINT*;

// Common return/size types
using LRESULT = long;
using UINT_PTR = unsigned long long;
using INT_PTR  = long long;
using WPARAM   = UINT_PTR;
using LPARAM   = LONG;

// Minimal MFC forward declarations (implementations will be Qt-based)
// These allow code guarded by #ifndef UNIX to co-exist with cross-platform code.
class CWnd;
class CDC;
class CString;
class CBitmap;
class CPen;
class CBrush;
class CFont;
class CRect;
class CPoint;
class CSize;

#endif // UNIX
