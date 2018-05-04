#include "stdafx.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include <afxdlgs.h>

namespace BetaProduct
{
    namespace
    {
        LogAndWarnFunctionType gLoggerFunction;
    }

    void SetLoggerFunction(LogAndWarnFunctionType &theLoggerFunction)
    {
        gLoggerFunction = theLoggerFunction;
    }

    LogAndWarnFunctionType GetLoggerFunction()
    {
        return gLoggerFunction;
    }


    std::string& InitialSavePath()
    {
        static std::string initialSavePath;

        return initialSavePath;
    }

    bool GetFilePathFromUser(const std::string &theFileFilter, const std::string &theInitialDirectory, std::string &theFilePathOut, bool fLoadFile)
    {
        CFileDialog dlg(fLoadFile, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CA2T(theFileFilter.c_str()));
        dlg.m_ofn.lpstrInitialDir = CA2T(theInitialDirectory.c_str());
        if(dlg.DoModal() == IDOK)
        {
            CString loadFilePath = dlg.GetPathName();
            theFilePathOut = CT2A(loadFilePath);
            return true;
        }
        return false;
    }

}