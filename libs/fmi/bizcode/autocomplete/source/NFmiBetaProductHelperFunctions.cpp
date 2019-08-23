#include "stdafx.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include "NFmiFileSystem.h"
#include "NFmiDictionaryFunction.h"
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

    bool GetFilePathFromUserTotal(const std::string& theFileFilter, const std::string& theInitialDirectory, std::string& theFilePathOut, bool fLoadFile, const std::string& theInitialFileName, const std::string& theFileExtension, const std::string& theRootDirectory, CWnd *theView)
    {
        if(BetaProduct::GetFilePathFromUser(theFileFilter, theInitialDirectory, theFilePathOut, fLoadFile, theInitialFileName))
        {
            auto originalFilePath = theFilePathOut;
            theFilePathOut = PathUtils::getTrueFilePath(theFilePathOut, theRootDirectory, theFileExtension);
            // Must check again if given file already exists, if user has given filename without extension
            if(originalFilePath != theFilePathOut && NFmiFileSystem::FileExists(theFilePathOut))
            {
                std::string message = ::GetDictionaryString("File:\n");
                message += theFilePathOut;
                message += "\n" + ::GetDictionaryString("already exists, do you want to overwrite it?");
                std::string messageBoxTitle = ::GetDictionaryString("File overwrite");
                if(::MessageBox(theView->GetSafeHwnd(), CA2T(message.c_str()), CA2T(messageBoxTitle.c_str()), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
                    return false;
            }
            return true;
        }
        return false;
    }
}