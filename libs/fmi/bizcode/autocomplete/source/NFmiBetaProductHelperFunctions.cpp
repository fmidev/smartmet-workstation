#include "stdafx.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include "NFmiFileSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMacroParamfunctions.h"
#include "CtrlViewFunctions.h"
#include <afxdlgs.h>

namespace BetaProduct
{
    namespace
    {
        LogAndWarnFunctionType gLoggerFunction;

        void NormalizePathDirectorySeparators(std::string& aPath)
        {
            boost::replace_all(aPath, "/", "\\");
        }

        void RemoveDirectorySeparatorFromEnd(std::string& aPath)
        {
            // aPath must be atleast 2 characters long before removing anything, otherwise "/" root directory won't work
            if(aPath.size() > 1 && PathUtils::lastCharacterIsSeparator(aPath))
            {
                aPath.resize(aPath.size() - 1);
            }
        }

        // Function that checks if given absolutePath (file or directory) is located in the 
        // absoluteDirectory or it's any subdirectory.
        bool IsPathInGivenDirectory(std::string absolutePath, std::string absoluteDirectory)
        {
            NormalizePathDirectorySeparators(absolutePath);
            NormalizePathDirectorySeparators(absoluteDirectory);
            RemoveDirectorySeparatorFromEnd(absoluteDirectory);

            if(absolutePath.empty() || absoluteDirectory.empty())
                return false;
            // Tämä etsintä työ pitää tehdä case-insensitiivisti, Koska Windows käsittelee polkuja siten.
            auto pos = MacroParam::ci_find_substr(absolutePath, absoluteDirectory); 
            return pos != MacroParam::ci_string_not_found;
        }

    }

    void SetLoggerFunction(LogAndWarnFunctionType &theLoggerFunction)
    {
        gLoggerFunction = theLoggerFunction;
    }

    LogAndWarnFunctionType GetLoggerFunction()
    {
        return gLoggerFunction;
    }

    // theInitialFileName on siis vain polku/xxx.ext -tyyppisessa polussa xxx -osio.
    // Kun CFileDialog:ille annetaan alkuarvaus tiedoston nimestä täysine polkuineen, avataan dialogi aina halutussa kansiossa.
    bool GetFilePathFromUser(const std::string &theFileFilter, const std::string &theInitialDirectory, std::string &theFilePathOut, bool fLoadFile, const std::string& theInitialFileName, CWnd* parentView)
    {
        auto originalPathString = theInitialDirectory + theInitialFileName;
        // Esim. "D:\\smartmet\\Dropbox (FMI)\\SmartMet\\MetEditor_5_13_2_0\\..\\..\\Macros\\FMI\\ViewMacros\\"
        // => "D:\\smartmet\\Dropbox (FMI)\\Macros\\FMI\\ViewMacros\\", muuten CFileDialog ei toimi kuten pitää.
        std::string simplyfiedPathString = PathUtils::simplifyWindowsPath(originalPathString);

        CString initialFilePath = CA2T(simplyfiedPathString.c_str());
        CFileDialog dlg(fLoadFile, NULL, initialFilePath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CA2T(theFileFilter.c_str()), parentView);
        if(dlg.DoModal() == IDOK)
        {
            CString loadFilePath = dlg.GetPathName();
            theFilePathOut = CT2A(loadFilePath);
            return true;
        }
        return false;
    }

    bool GetFilePathFromUserTotal(const std::string& theFileFilter, const std::string& theInitialDirectory, std::string& theFilePathOut, bool fLoadFile, const std::string& theInitialFileName, const std::string& theFileExtension, const std::string& theRootDirectory, CWnd * parentView)
    {
        if(BetaProduct::GetFilePathFromUser(theFileFilter, theInitialDirectory, theFilePathOut, fLoadFile, theInitialFileName, parentView))
        {
            auto originalFilePath = theFilePathOut;
            auto simplifiedRootDirectory = PathUtils::simplifyWindowsPath(theRootDirectory);
            theFilePathOut = PathUtils::getTrueFilePath(theFilePathOut, theRootDirectory, theFileExtension);

            // If user gives path outside the given root-path, we must reject the path
            if(!IsPathInGivenDirectory(theFilePathOut, simplifiedRootDirectory))
            {
                std::string message = ::GetDictionaryString("File");
                message += ":\n";
                message += theFilePathOut;
                message += "\n" + ::GetDictionaryString("was outside of the root path");
                message += ":\n";
                message += simplifiedRootDirectory;
                message += "\n" + ::GetDictionaryString("cannot continue saving the file");
                std::string messageBoxTitle = ::GetDictionaryString("File outside root path");
                ::MessageBox(parentView->GetSafeHwnd(), CA2T(message.c_str()), CA2T(messageBoxTitle.c_str()), MB_OK | MB_ICONERROR);
                return false;
            }

            // Must check again if given file already exists, if user has given filename without extension
            if(originalFilePath != theFilePathOut && NFmiFileSystem::FileExists(theFilePathOut))
            {
                std::string message = ::GetDictionaryString("File:\n");
                message += theFilePathOut;
                message += "\n" + ::GetDictionaryString("already exists, do you want to overwrite it?");
                std::string messageBoxTitle = ::GetDictionaryString("File overwrite");
                if(::MessageBox(parentView->GetSafeHwnd(), CA2T(message.c_str()), CA2T(messageBoxTitle.c_str()), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
                    return false;
            }
            return true;
        }
        return false;
    }

}