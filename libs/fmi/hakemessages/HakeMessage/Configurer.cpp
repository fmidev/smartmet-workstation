#include "HakeMessage/Configurer.h"
#include "HakeMessage/Io.h"
#include "HakeMessage/Main.h"
#include "HakeMessage/Common.h"

#include "NFmiSettings.h"
#include "NFmiPathUtils.h"

#include <boost/math/special_functions/round.hpp>
#include <boost/lexical_cast.hpp>

using milliseconds = std::chrono::milliseconds;

namespace HakeMessage
{
    namespace
    {
        namespace Defaults
        {
            const unsigned int killCheckDelay = 250;
            const unsigned int directoryCheckDelay = 250;
        }

        milliseconds getMinutesFromSettings(const std::string& configurationKey, const milliseconds& defaultValueInMS)
        {
            return minutesToMilliSeconds
            (
                NFmiSettings::Optional(configurationKey, milliSecondsToMinutes(defaultValueInMS))
            );
        }

        void setMinutesInSettings(const std::string& configurationKey, const milliseconds& valueInMS)
        {
            NFmiSettings::Set(
                configurationKey,
                boost::lexical_cast<std::string>(milliSecondsToMinutes(valueInMS)),
                true
            );
        }

        void logUsedWarningDataFormatPath(const std::string& path, const std::string& filter, const std::string& dataFormatName)
        {
            if(path.empty() && filter.empty())
                return;
            if(!path.empty() && !filter.empty())
            {
                std::string pathMessage = "HAKE message with ";
                pathMessage += dataFormatName;
                pathMessage += " data format has '";
                pathMessage += path;
                pathMessage += "' search path with '";
                pathMessage += filter;
                pathMessage += "' regex based file filter";
                CatLog::logMessage(pathMessage, CatLog::Severity::Debug, CatLog::Category::Configuration);
                return;
            }

            // path or filter was empty, make warning
            std::string pathWarningMessage = "HAKE message with ";
            pathWarningMessage += dataFormatName;
            pathWarningMessage += " data format had missing configuration: ";
            if(path.empty())
                pathWarningMessage += "search path was empty";
            else
                pathWarningMessage += "search path was '" + path + "'";
            pathWarningMessage += " and ";
            if(filter.empty())
                pathWarningMessage += "regex based file filter was empty";
            else
                pathWarningMessage += "regex based file filter was '" + filter + "'";
            CatLog::logMessage(pathWarningMessage, CatLog::Severity::Warning, CatLog::Category::Configuration, true);
        }

        void logUsedWarningPaths(const Configurer& configurer)
        {
            logUsedWarningDataFormatPath(configurer.jsonPath, configurer.jsonFilter, "json");
            logUsedWarningDataFormatPath(configurer.xmlPath, configurer.xmlFilter, "xml");
        }

        // If used HAKE message paths are not fixed properly, there migth be problems with them over
        // runtime of application, if user changes the actual working directory with some file/path browser
        // in some of the dialog's that have those options.
        void fixUsedWarningPaths(std::string& fixedPathInOut, const std::string& usedAbsoluteBaseDirectory)
        {
            // 1. Set absolute path with drive letter from usedAbsoluteBaseDirectory (on windows) if needed.
            fixedPathInOut = PathUtils::getAbsoluteFilePath(fixedPathInOut, usedAbsoluteBaseDirectory);
            // 2. Let's also simplify path to get rid of possible relative path jumps (xxx/yyy/../zzz => xxx/zzz)
            fixedPathInOut = PathUtils::simplifyWindowsPath(fixedPathInOut);
        }

        void fixUsedWarningPaths(Configurer& fixedConfigurerInOut, const std::string& usedAbsoluteBaseDirectory)
        {
            fixUsedWarningPaths(fixedConfigurerInOut.jsonPath, usedAbsoluteBaseDirectory);
            fixUsedWarningPaths(fixedConfigurerInOut.xmlPath, usedAbsoluteBaseDirectory);
        }
    }

    void Configurer::configure(const std::string& usedAbsoluteBaseDirectory)
    {
        jsonPath = NFmiSettings::Optional(baseConfigurationNamespace + "pathForJson", jsonPath);
        xmlPath = NFmiSettings::Optional(baseConfigurationNamespace + "pathForXml", xmlPath);
        xmlFilter = NFmiSettings::Optional(baseConfigurationNamespace + "xmlFilter", xmlFilter);
        jsonFilter = NFmiSettings::Optional(baseConfigurationNamespace + "jsonFilter", jsonFilter);
        killCheckDelay = milliseconds(
            NFmiSettings::Optional(
                baseConfigurationNamespace + "killCheckDelayInMS",
                Defaults::killCheckDelay
            )
        );
        checkForNewMessagesDelay = getMinutesFromSettings(
            baseConfigurationNamespace + "directoryCheckDelayInMinutes",
            milliseconds(Defaults::directoryCheckDelay)
        );
        maxNumberOfMessagesReadAtOnce = NFmiSettings::Optional(
            baseConfigurationNamespace + "maxNumberOfMessagesReadAtOnce",
            maxNumberOfMessagesReadAtOnce
        );

        fixUsedWarningPaths(*this, usedAbsoluteBaseDirectory);
        logUsedWarningPaths(*this);
    }

    void Configurer::storeConfigures() const
    {
        NFmiSettings::Set(baseConfigurationNamespace + "pathForJson", jsonPath, true);
        NFmiSettings::Set(baseConfigurationNamespace + "pathForXml", xmlPath, true);
        NFmiSettings::Set(baseConfigurationNamespace + "xmlFilter", xmlFilter, true);
        NFmiSettings::Set(baseConfigurationNamespace + "jsonFilter", jsonFilter, true);
        NFmiSettings::Set(
            baseConfigurationNamespace + "killCheckDelayInMS",
            boost::lexical_cast<std::string>(killCheckDelay.count()),
            true
        );
        setMinutesInSettings(baseConfigurationNamespace + "directoryCheckDelayInMinutes", checkForNewMessagesDelay);
    }
}
