#pragma once

#include "stdafx.h"

// Tässä on pari hakemiston käsittely win32 funktiota.

namespace CFmiWin32DirectoryUtils
{
    int BruteDirCopy(CString &strFolderSrcU_, CString &strFolderDestU_);
    bool DirDelete(CString &strFolderU_);

}
