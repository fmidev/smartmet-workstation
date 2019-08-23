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

    // theInitialFileName on siis vain polku/xxx.ext -tyyppisessa polussa xxx -osio.
    // Kun CFileDialog:ille annetaan alkuarvaus tiedoston nimestä täysine polkuineen, avataan dialogi aina halutussa kansiossa.
    bool GetFilePathFromUser(const std::string &theFileFilter, const std::string &theInitialDirectory, std::string &theFilePathOut, bool fLoadFile, const std::string& theInitialFileName)
    {
        CString initialFilePath = CA2T(theInitialDirectory.c_str());
        initialFilePath += CA2T(theInitialFileName.c_str());
        CFileDialog dlg(fLoadFile, NULL, initialFilePath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CA2T(theFileFilter.c_str()));
        if(dlg.DoModal() == IDOK)
        {
            CString loadFilePath = dlg.GetPathName();
            theFilePathOut = CT2A(loadFilePath);
            return true;
        }
        return false;
    }

}