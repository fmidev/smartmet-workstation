#pragma once

#include <string>

namespace ExePathHelper
{
    std::string MakeOptionalExePath(const std::string& workingDirectory, const std::string& optionalSettingsExePathKey, const std::string& exeNameForErrorMessage);
    std::string MakeHardCodedExePath(const std::string& workingDirectory, const std::string& relativePathToExe, const std::string& exeNameForErrorMessage);
}

