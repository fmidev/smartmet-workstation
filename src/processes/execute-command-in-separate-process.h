#pragma once

#include "stdafx.h"
#include <string>

namespace CFmiProcessHelpers
{
    bool ExecuteCommandInSeparateProcess(std::string &theCommand, bool logEvents = false, bool showErrorMessageBox = false, WORD theShowWindow = SW_SHOW, bool waitExecutionToStop = false, DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS, const std::string *theWorkingDirectory = nullptr);
    std::string Make7zipExePath(const std::string &workingDirectory);
}

