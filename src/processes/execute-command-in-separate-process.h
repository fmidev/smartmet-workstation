#pragma once

#include <string>

namespace CFmiProcessHelpers
{
#ifndef UNIX
    bool ExecuteCommandInSeparateProcess(std::string &theCommand, bool logEvents = false, bool showErrorMessageBox = false, WORD theShowWindow = SW_SHOW, bool waitExecutionToStop = false, DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS, const std::string *theWorkingDirectory = nullptr);
#else // UNIX
    bool ExecuteCommandInSeparateProcess(std::string &theCommand, bool logEvents = false, bool showErrorMessageBox = false, int theShowWindow = 1, bool waitExecutionToStop = false, int dwCreationFlags = 0, const std::string *theWorkingDirectory = nullptr);
#endif // UNIX
    std::string Make7zipExePath(const std::string &workingDirectory);
    std::string GetStoreMessagesString();
}

