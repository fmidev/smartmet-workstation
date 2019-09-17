#pragma once

#include "NFmiProducer.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>

namespace Wms
{
    class ServerSetup
    {
    public:
        std::string scheme;
        std::string host;
        std::string path;
        std::string map;
        std::string stereo00;
        std::string stereo10;
        std::string stereo20;
        std::string token;
        std::vector<std::string> layerGroup;
    };

    class UserUrlServerSetup
    {
    public:
        std::string version;
        bool transparency;
        std::vector<ServerSetup> parsedServers;
    };

    class DynamicServerSetup
    {
    public:
        NFmiProducer producer;
        std::string version;
        bool transparency;
        bool logFetchCapabilitiesRequest = true;
        bool doVerboseLogging = false;
        std::string delimiter;
        ServerSetup generic;
    };

    class Setup
    {
    public:
        bool isConfigured = false;

        size_t numberOfCaches;
        size_t numberOfLayersPerCache;
        std::string proxyUrl;

        int backgroundBackwardAmount;
        int backgroundForwardAmount;

        std::chrono::seconds intervalToPollGetCapabilities;

        UserUrlServerSetup background;
        UserUrlServerSetup overlay;
        std::unordered_map<int, DynamicServerSetup> dynamics;

        // If there is some configurations given, then we must be able
        // to set this WMS system in on -mode (=> isConfigured = true).
        void checkForMeaningfulConfigurations()
        {
            if(numberOfCaches && numberOfLayersPerCache)
            {
                if(background.parsedServers.size() || dynamics.size())
                {
                    isConfigured = true;
                }
            }
        }
    };
}

