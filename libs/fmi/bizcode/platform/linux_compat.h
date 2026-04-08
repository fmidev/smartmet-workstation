#pragma once
// Minimal Windows/MFC type compatibility layer for Linux builds.
// Replaces the types normally provided by <afxwin.h> and Win32 SDK.
// Full MFC functionality requires porting to Qt (in progress).

#ifdef UNIX

#include <string>
#include <cstring>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

// Basic Windows integer types
using BOOL      = int;
using COLORREF  = unsigned long;

// Stub registry value types (normally from NFmiCachedRegistryValue.h on Windows)
using CachedRegInt    = int;
using CachedRegString = std::string;
using CachedRegBool   = bool;
using CachedRegDouble = double;
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

// ---- TCHAR compatibility macros (TCHAR = char on Linux) ----
#ifndef _T
#define _T(x) x
#endif
#ifndef _TEXT
#define _TEXT(x) x
#endif
#define _ttol(s)     atol(s)
#define _istspace(c) isspace((unsigned char)(c))
// CA2T/CT2A: on Linux TCHAR=char, so these are identity conversions
#ifndef CA2T
#define CA2T(s) (s)
#endif
#ifndef CT2A
#define CT2A(s) (s)
#endif
// TCHAR string function mappings to POSIX equivalents
#define _tcschr   strchr
#define _tcspbrk  strpbrk
#define _tcslen   strlen
#define _tcsncpy  strncpy
#define _tcsnicmp strncasecmp
#define _tcsncmp  strncmp
#define _tcscmp   strcmp
#define _tcsicmp  strcasecmp

// ---- Minimal MFC CString for Linux: wraps std::string ----
class CString {
public:
    CString() = default;
    CString(const char* s) : s_(s ? s : "") {} // NOLINT: implicit, required for MFC compat
    CString(const CString&) = default;
    CString(CString&&) = default;
    CString& operator=(const CString&) = default;
    CString& operator=(CString&&) = default;
    CString& operator=(const char* s) { s_ = s ? s : ""; return *this; }

    // Implicit conversions — required by MFC idiom (os << (LPCTSTR)name, std::string tmp = CT2A(...))
    operator const char*() const { return s_.c_str(); }
    operator std::string()  const { return s_; }

    bool operator==(const CString& o) const { return s_ == o.s_; }
    bool operator==(const char* s)    const { return s_ == (s ? s : ""); }
    bool operator!=(const CString& o) const { return !(*this == o); }
    bool operator!=(const char* s)    const { return !(*this == s); }

    bool IsEmpty()    const { return s_.empty(); }
    int  GetLength()  const { return static_cast<int>(s_.size()); }

    void TrimLeft() {
        s_.erase(s_.begin(), std::find_if(s_.begin(), s_.end(),
            [](unsigned char c){ return !std::isspace(c); }));
    }
    void TrimRight() {
        s_.erase(std::find_if(s_.rbegin(), s_.rend(),
            [](unsigned char c){ return !std::isspace(c); }).base(), s_.end());
    }

    // MFC buffer-access pattern: resize and return writable pointer
    char* GetBufferSetLength(int len) {
        s_.resize(static_cast<std::size_t>(len));
        return &s_[0];
    }

    // Printf-style formatting
    void Format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int n = std::vsnprintf(nullptr, 0, fmt, args);
        va_end(args);
        if (n > 0) {
            s_.resize(static_cast<std::size_t>(n));
            va_start(args, fmt);
            std::vsnprintf(&s_[0], static_cast<std::size_t>(n) + 1, fmt, args);
            va_end(args);
        }
    }

    const std::string& str() const { return s_; }

private:
    std::string s_;
};

inline bool operator==(const char* s, const CString& cs) { return cs == s; }
inline bool operator!=(const char* s, const CString& cs) { return cs != s; }

// ---- Minimal MFC forward declarations (implementations will be Qt-based) ----
// These allow code guarded by #ifndef UNIX to co-exist with cross-platform code.
// Note: CRect is a typedef (= NFmiRect) defined in NFmiToolBox.h, not a class.
class CWnd;
class CDC;
class CBitmap;
class CPen;
class CBrush;
class CFont;
class CPoint;
class CSize;

// Windows process/window constants for Linux (values are passed to Linux stubs and ignored)
#define SW_HIDE            0
#define SW_SHOW            5
#define SW_SHOWMINIMIZED   2
#define SW_SHOWNORMAL      1
#define NORMAL_PRIORITY_CLASS 0x00000020
#define INFINITE          0xFFFFFFFF

// Virtual key codes (subset used in the codebase)
#ifndef VK_SHIFT
#define VK_SHIFT   0x10
#define VK_CONTROL 0x11
#define VK_MENU    0x12
#endif

// GetAsyncKeyState stub — always returns "not pressed" on Linux
inline short GetAsyncKeyState(int) { return 0; }

// RGB macro (Win32 GDI)
#ifndef RGB
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif

#endif // UNIX
