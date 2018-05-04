#include "stdafx.h"
#include "MfcTools.h"

std::string GetFileVersionOfApplication(LPCTSTR theFileName)
{
    DWORD dwDummy;
    DWORD dwFVISize = ::GetFileVersionInfoSize(theFileName, &dwDummy);

    if (dwFVISize)
    {
        LPBYTE lpVersionInfo = new BYTE[dwFVISize];

        ::GetFileVersionInfo(theFileName, 0, dwFVISize, lpVersionInfo);

        UINT uLen;
        VS_FIXEDFILEINFO *lpFfi;

        ::VerQueryValue(lpVersionInfo, _TEXT("\\"), (LPVOID *)&lpFfi, &uLen);

        DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
        DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;

        delete[] lpVersionInfo;

        DWORD dwLeftMost = HIWORD(dwFileVersionMS);
        DWORD dwSecondLeft = LOWORD(dwFileVersionMS);
        DWORD dwSecondRight = HIWORD(dwFileVersionLS);
        DWORD dwRightMost = LOWORD(dwFileVersionLS);

        CString str;
        str.Format(_TEXT("%d.%d.%d.%d"), dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
        return std::string(CT2A(str));
    }
    return "";
}
