// CFmiProcessHelpers.cpp : implementation file
//

#include "execute-command-in-separate-process.h"
#include "catlog/catlog.h"
#include "NFmiSettings.h"
#include "NFmiPathUtils.h"
#include "NFmiFileSystem.h"

namespace
{
    // Laitetaan tahan kerran rakennettu polku jatkokayttoa varten
    std::string g_cached7zipExePath;

    std::string GetOptional7zipExePath()
    {
        return NFmiSettings::Optional<std::string>("SmartMet::Optional7zipExePath", "");
    }

    std::string MakeOptional7zipExePath(const std::string& workingDirectory)
    {
        auto optionalPath = ::GetOptional7zipExePath();
        if(optionalPath.empty())
            return "";
        else
        {
            // 1. Laitetaan absoluuttinen polku kuntoon
            auto zipPath = PathUtils::getAbsoluteFilePath(optionalPath, workingDirectory);
            // 2. Siivotaan polkua, poistetaan mahd. suhteelliset polkuhypyt (..\.. jutut) pois
            zipPath = PathUtils::simplifyWindowsPath(zipPath);
            // 3. Jos polku osoittaa johonkin tiedostoon, silloin se hyväksytään
            if(NFmiFileSystem::FileExists(zipPath))
                return zipPath;
            else
            {
                std::string zipPathError = "7zip path from SmartMet::Optional7zipExePath -option with final value ";
                zipPathError += zipPath + " didn't exist, using default 7zip path instead";
                CatLog::logMessage(zipPathError, CatLog::Severity::Error, CatLog::Category::Configuration, true);
                // 5. Palautetaan virhetilanteessa tyhjää, jotta otetaan default polku käyttöön
                return "";
            }
        }
    }

    std::string MakeHardCoded7zipExePath(const std::string& workingDirectory)
    {
        std::string zipPath = workingDirectory;
        zipPath += "\\utils\\";
        zipPath += "7z.exe";

        if(!NFmiFileSystem::FileExists(zipPath))
        {
            std::string zipPathError = "7zip executable from default path (smartmet-path\\utils\\7z.exe): ";
            zipPathError += zipPath + " didn't exist, unpacking operations won't work!";
            CatLog::logMessage(zipPathError, CatLog::Severity::Error, CatLog::Category::Configuration, true);
        }

        return zipPath;
    }

    // Oletus funktiota kutsutaan vain kerran, eli ennen kutsua, tarkistetaan etta g_cached7zipExePath ei ole tyhja.
    std::string Construct7zipExePath(const std::string& workingDirectory)
    {
        auto used7zipExePath = MakeOptional7zipExePath(workingDirectory);
        if(used7zipExePath.empty())
        {
            used7zipExePath = MakeHardCoded7zipExePath(workingDirectory);
        }
        used7zipExePath = PathUtils::simplifyWindowsPath(used7zipExePath);
        return used7zipExePath;
    }
}

namespace CFmiProcessHelpers
{

    bool ExecuteCommandInSeparateProcess(std::string& theCommand, bool logEvents, bool showErrorMessageBox, WORD theShowWindow, bool waitExecutionToStop, DWORD dwCreationFlags, const std::string* theWorkingDirectory)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = theShowWindow;
        CString usedWorkingDirectory;
        LPCTSTR usedWorkingDirectoryPtr = NULL;
        if(theWorkingDirectory)
        {
            usedWorkingDirectory = CA2T(theWorkingDirectory->c_str());
            usedWorkingDirectoryPtr = usedWorkingDirectory;
        }

        // Start the child process.
        if(!CreateProcess(
            NULL, // No module name (use command line).
            CA2T(theCommand.c_str()),
            NULL,             // Process handle not inheritable.
            NULL,             // Thread handle not inheritable.
            FALSE,            // Set handle inheritance to FALSE.
            dwCreationFlags,  // Creation flags.
            NULL,             // Use parent's environment block.
            usedWorkingDirectoryPtr,  // NULL = use parent's starting directory.
            &si,              // Pointer to STARTUPINFO structure.
            &pi)             // Pointer to PROCESS_INFORMATION structure.
            )
        {
            LPTSTR lpMsgBuf = 0;
            FormatMessage
            (
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                lpMsgBuf,
                0,
                NULL
            );

            if(showErrorMessageBox)
                MessageBox(NULL, lpMsgBuf, _TEXT("Error with CreateProcess call"), MB_OK | MB_ICONWARNING);

            LocalFree(lpMsgBuf);
            if(logEvents)
            {
                std::string virheStr("ExecuteCommandInSeparateProcess: Command '");
                virheStr += theCommand;
                virheStr += "' was unable to execute.";
                CatLog::logMessage(virheStr, CatLog::Severity::Error, CatLog::Category::Operational);
            }
            return false;
        }
        DWORD waitTimeInMS = waitExecutionToStop ? INFINITE : 0; // joko odotetaan että prosessi saadaan päätökseen, tai ei odoteta ollenkaan
        WaitForSingleObject(pi.hProcess, waitTimeInMS);

        CloseHandle(pi.hProcess); // Close process and thread handles.
        CloseHandle(pi.hThread);
        if(logEvents)
        {
            std::string logStr("ExecuteCommandInSeparateProcess: Command '");
            logStr += theCommand;
            logStr += "' executed.";
            CatLog::logMessage(logStr, CatLog::Severity::Info, CatLog::Category::Operational);
        }
        return true;
    }

    std::string Make7zipExePath(const std::string& workingDirectory)
    {
        if(g_cached7zipExePath.empty())
        {
            auto zipExePath = ::Construct7zipExePath(workingDirectory);
            CatLog::logMessage(std::string("Used 7z.exe path is: ") + zipExePath, CatLog::Severity::Info, CatLog::Category::Configuration);

            // Lopullisessa polussa pitää olla vielä lainausmerkit ympärillä, koska tätä käytetään komentoriviltä
            // ja jos polussa spaceja, menee homma muuten pieleen eli:
            // D:\polku jonnekin\7z.exe ==>> "D:\polku jonnekin\7z.exe"
            g_cached7zipExePath = "\"" + zipExePath + "\"";
        }

        return g_cached7zipExePath;
    }

    std::string GetStoreMessagesString() 
    { 
        return "store-messages="; 
    }

} // CFmiProcessHelpers namespace
