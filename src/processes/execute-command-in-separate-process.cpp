// CFmiProcessHelpers.cpp : implementation file
//

#include "execute-command-in-separate-process.h"
#include "catlog/catlog.h"
#include "NFmiSettings.h"
#include "NFmiPathUtils.h"
#include "NFmiFileSystem.h"
#include "MakeOptionalExePath.h"

namespace
{
    // Laitetaan tahan kerran rakennettu polku jatkokayttoa varten
    std::string g_cached7zipExePath;

    // Oletus funktiota kutsutaan vain kerran, eli ennen kutsua, tarkistetaan etta g_cached7zipExePath ei ole tyhja.
    std::string Construct7zipExePath(const std::string& workingDirectory)
    {
        std::string exeNameInErrorMessages = "7zip";
        auto used7zipExePath = ExePathHelper::MakeOptionalExePath(workingDirectory, "SmartMet::Optional7zipExePath", exeNameInErrorMessages);
        if(used7zipExePath.empty())
        {
            used7zipExePath = ExePathHelper::MakeHardCodedExePath(workingDirectory, "\\utils\\7z.exe", exeNameInErrorMessages);
        }
        return used7zipExePath;
    }
}

namespace CFmiProcessHelpers
{
#ifndef UNIX
    // theWorkingDirectory parametri: Ei saa olla ymp�r�ivi� lainausmerkkej� v�lily�nti polkujen varalla, t�ll�in koko CreateProcess funktio ei toimi silloin ollenkaan.
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
            if(showErrorMessageBox)
            {
                LPVOID lpMsgBuf;
                DWORD dw = GetLastError();

                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dw,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpMsgBuf,
                    0, NULL);

                MessageBox(NULL, (LPCTSTR)lpMsgBuf, _TEXT("Error with CreateProcess call"), MB_OK | MB_ICONERROR);
                LocalFree(lpMsgBuf);
            }
            if(logEvents)
            {
                std::string virheStr("ExecuteCommandInSeparateProcess: Command '");
                virheStr += theCommand;
                virheStr += "' was unable to execute.";
                CatLog::logMessage(virheStr, CatLog::Severity::Error, CatLog::Category::Operational);
            }
            return false;
        }
        DWORD waitTimeInMS = waitExecutionToStop ? INFINITE : 0; // joko odotetaan ett� prosessi saadaan p��t�kseen, tai ei odoteta ollenkaan
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
#else // UNIX
    bool ExecuteCommandInSeparateProcess(std::string& theCommand, bool logEvents, bool showErrorMessageBox, int /*theShowWindow*/, bool waitExecutionToStop, int /*dwCreationFlags*/, const std::string* theWorkingDirectory)
    {
        std::string cmd = theCommand;
        if(theWorkingDirectory && !theWorkingDirectory->empty())
            cmd = "cd \"" + *theWorkingDirectory + "\" && " + cmd;
        if(!waitExecutionToStop)
            cmd += " &";
        int ret = std::system(cmd.c_str());
        if(logEvents)
        {
            std::string logStr("ExecuteCommandInSeparateProcess: Command '");
            logStr += theCommand;
            logStr += (ret == 0 ? "' executed." : "' failed.");
            CatLog::logMessage(logStr, (ret == 0 ? CatLog::Severity::Info : CatLog::Severity::Error), CatLog::Category::Operational);
        }
        return ret == 0;
    }
#endif // UNIX

    std::string Make7zipExePath(const std::string& workingDirectory)
    {
        if(g_cached7zipExePath.empty())
        {
            auto zipExePath = ::Construct7zipExePath(workingDirectory);
            CatLog::logMessage(std::string("Used 7z.exe path is: ") + zipExePath, CatLog::Severity::Info, CatLog::Category::Configuration);

            // Lopullisessa polussa pit�� olla viel� lainausmerkit ymp�rill�, koska t�t� k�ytet��n komentorivilt�
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
