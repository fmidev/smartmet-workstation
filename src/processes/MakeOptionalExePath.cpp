#include "MakeOptionalExePath.h"
#include "catlog/catlog.h"
#include "NFmiSettings.h"
#include "NFmiPathUtils.h"
#include "NFmiFileSystem.h"

namespace
{
    std::string GetOptionalExePath(const std::string& optionalSettingsExePathKey)
    {
        return NFmiSettings::Optional<std::string>(optionalSettingsExePathKey, "");
    }
}

namespace ExePathHelper
{
    std::string MakeOptionalExePath(const std::string& workingDirectory, const std::string& optionalSettingsExePathKey, const std::string& exeNameForErrorMessage)
    {
        auto optionalPath = ::GetOptionalExePath(optionalSettingsExePathKey);
        if(optionalPath.empty())
            return "";
        else
        {
            // 1. Laitetaan absoluuttinen polku kuntoon
            auto path = PathUtils::getAbsoluteFilePath(optionalPath, workingDirectory);
            // 2. Siivotaan polkua, poistetaan mahd. suhteelliset polkuhypyt (..\.. jutut) pois
            path = PathUtils::simplifyWindowsPath(path);
            // 3. Jos polku osoittaa johonkin tiedostoon, silloin se hyväksytään
            if(NFmiFileSystem::FileExists(path))
                return path;
            else
            {
                std::string pathError = exeNameForErrorMessage;
                pathError += "  path from ";
                pathError += optionalSettingsExePathKey;
                pathError += " -option with final value '";
                pathError += path + "' didn't exist, using default path instead";
                CatLog::logMessage(pathError, CatLog::Severity::Error, CatLog::Category::Configuration, true);
                // 5. Palautetaan virhetilanteessa tyhjää, jotta otetaan default polku käyttöön
                return "";
            }
        }
    }

    std::string MakeHardCodedExePath(const std::string& workingDirectory, const std::string& relativePathToExe, const std::string& exeNameForErrorMessage)
    {
        std::string path = workingDirectory;
        path += relativePathToExe;
        path = PathUtils::simplifyWindowsPath(path);

        if(!NFmiFileSystem::FileExists(path))
        {
            std::string pathError = exeNameForErrorMessage;
            pathError += " executable from default path (smartmet-path";
            pathError += relativePathToExe;
            pathError += "): '";
            pathError += path + "' didn't exist, unable to use this executable at all!";
            CatLog::logMessage(pathError, CatLog::Severity::Error, CatLog::Category::Configuration, true);
        }

        return path;
    }
}

