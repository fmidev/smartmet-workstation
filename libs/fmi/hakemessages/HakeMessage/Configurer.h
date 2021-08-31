#pragma once

#include <chrono>
#include <string>

namespace HakeMessage
{
    class Configurer
    {
    public:
        std::string baseConfigurationNamespace = "SmartMet::HakeMessages::";
        std::string jsonPath;
        std::string xmlPath;
        std::string jsonFilter = ".*.json";
        std::string xmlFilter = ".*.xml";
        std::chrono::milliseconds checkForNewMessagesDelay;
        std::chrono::milliseconds killCheckDelay;
        unsigned int maxNumberOfMessagesReadAtOnce = 100;

        void configure(const std::string& usedAbsoluteBaseDirectory);
        void storeConfigures() const;
    };
}
