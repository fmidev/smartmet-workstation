
#include "FmiWin32DirectoryUtils.h"

// Kopioidaan hakemisto rekursiivisesti.
int CFmiWin32DirectoryUtils::BruteDirCopy(CString &strFolderSrcU_, CString &strFolderDestU_)
{
	strFolderSrcU_ += '\0'; // n‰m‰ toiset nollat pit‰‰ lis‰t‰ loppuun, muuten ei toimi
    strFolderDestU_ += '\0'; // n‰m‰ toiset nollat pit‰‰ lis‰t‰ loppuun, muuten ei toimi
	//Directory Copying 
	SHFILEOPSTRUCT SHDirOp;
	ZeroMemory(&SHDirOp, sizeof(SHFILEOPSTRUCT));
	SHDirOp.hwnd = NULL;
	SHDirOp.wFunc = FO_COPY;
    SHDirOp.pFrom = strFolderSrcU_;
    SHDirOp.pTo = strFolderDestU_;
	SHDirOp.fFlags = FOF_MULTIDESTFILES|FOF_NOCONFIRMMKDIR|FOF_NOCONFIRMATION;

	//The Copying Function
	return SHFileOperation(&SHDirOp);
}

bool CFmiWin32DirectoryUtils::DirDelete(CString &strFolderU_)
{
    strFolderU_ += '\0'; // n‰m‰ toiset nollat pit‰‰ lis‰t‰ loppuun, muuten ei toimi
	//Directory Copying 
	SHFILEOPSTRUCT SHDirOp;
	ZeroMemory(&SHDirOp, sizeof(SHFILEOPSTRUCT));
	SHDirOp.hwnd = NULL;
	SHDirOp.wFunc = FO_DELETE;
    SHDirOp.pFrom = strFolderU_;
	SHDirOp.pTo = NULL;
	SHDirOp.fFlags = FOF_MULTIDESTFILES|FOF_ALLOWUNDO|FOF_NOCONFIRMATION;

	//The Copying Function
	int result = SHFileOperation(&SHDirOp);

	return result == 0;
}
