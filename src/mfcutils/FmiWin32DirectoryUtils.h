#pragma once

#include "stdafx.h"

// T�ss� on pari hakemiston k�sittely win32 funktiota.

namespace CFmiWin32DirectoryUtils
{
    int BruteDirCopy(CString &strFolderSrcU_, CString &strFolderDestU_);
    bool DirDelete(CString &strFolderU_);

}
