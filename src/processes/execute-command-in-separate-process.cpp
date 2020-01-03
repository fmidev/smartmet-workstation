// CFmiProcessHelpers.cpp : implementation file
//

#include "execute-command-in-separate-process.h"
#include "catlog/catlog.h"

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
        std::string zipCommandStr = "\""; // myös zip-commandin ympärille lainausmerkit
        zipCommandStr += workingDirectory;
        zipCommandStr += "\\utils\\";
        zipCommandStr += "7z.exe";
        zipCommandStr += "\"";
        return zipCommandStr;
    }

} // CFmiProcessHelpers namespace
