#include "HakeMessage/Configurer.h"
#include "HakeMessage/Io.h"
#include "HakeMessage/Main.h"
#include "HakeMessage/Common.h"

#include "NFmiSettings.h"

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

        milliseconds getMinutesFromSettings(const std::string &configurationKey, const milliseconds &defaultValueInMS)
        {
            return minutesToMilliSeconds
            (
                NFmiSettings::Optional(configurationKey, milliSecondsToMinutes(defaultValueInMS))
            );
        }

        void setMinutesInSettings(const std::string &configurationKey, const milliseconds &valueInMS)
        {
            NFmiSettings::Set(
                configurationKey,
                boost::lexical_cast<std::string>(milliSecondsToMinutes(valueInMS)),
                true
            );
        }
    }

    void Configurer::configure()
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
