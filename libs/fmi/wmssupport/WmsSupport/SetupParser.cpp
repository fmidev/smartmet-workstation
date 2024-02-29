#include "wmssupport/SetupParser.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    namespace
    {

        ServerSetup parseServer(const std::string& path)
        {
            auto serverSetup = ServerSetup{};
            serverSetup.scheme = NFmiSettings::Optional(path + "::Scheme", std::string("http"));
            serverSetup.host = NFmiSettings::Require<std::string>(path + "::Host");
            serverSetup.path = NFmiSettings::Require<std::string>(path + "::Path");
            serverSetup.map = NFmiSettings::Optional(path + "::Map", std::string(""));
            serverSetup.stereo00 = NFmiSettings::Optional(path + "::Stereo00", std::string(""));
            serverSetup.stereo10 = NFmiSettings::Optional(path + "::Stereo10", std::string(""));
            serverSetup.stereo20 = NFmiSettings::Optional(path + "::Stereo20", std::string(""));
            serverSetup.token = NFmiSettings::Optional(path + "::Token", std::string(""));
            serverSetup.useCrs = NFmiSettings::Optional(path + "::UseCrs", true);
            return serverSetup;
        }

        DynamicServerSetup parseDynamic(const std::string& path, bool doVerboseLogging)
        {
            auto dynamicSetup = DynamicServerSetup{};
            dynamicSetup.producer = NFmiProducer{
                NFmiSettings::Require<unsigned long>(path + "::ProducerId"),
                NFmiSettings::Require<std::string>(path + "::ProducerName")
            };
            dynamicSetup.version = NFmiSettings::Optional(path + "::Version", std::string("1.3.0"));
            dynamicSetup.transparency = NFmiSettings::Optional(path + "::Transparency", true);
            dynamicSetup.delimiter = NFmiSettings::Optional(path + "::Delimiter", std::string(","));
            dynamicSetup.generic = parseServer(path);
            dynamicSetup.doVerboseLogging = doVerboseLogging;
            dynamicSetup.acceptTimeDimensionalLayersOnly = NFmiSettings::Optional(path + "::AcceptTimeDimensionalLayersOnly", false);

            return dynamicSetup;
        }

        void parseDynamics(const std::string& nspace, std::unordered_map<int, DynamicServerSetup>& dynamics, bool doVerboseLogging)
        {
            auto dynamicKeys = NFmiSettings::ListChildren(nspace);
            for(const auto& key : dynamicKeys)
            {
                auto dynamicSetup = parseDynamic(nspace + "::" + key, doVerboseLogging);
                dynamics[dynamicSetup.producer.GetIdent()] = dynamicSetup;
            }
        }

        std::map<std::string, ServerSetup> parseKnownServers(const std::string& nspace)
        {
            auto knownServers = std::map<std::string, ServerSetup>{};
            auto knownKeys = NFmiSettings::ListChildren(nspace);
            for(const auto& key : knownKeys)
            {
                auto serverSetup = parseServer(nspace + "::" + key);
                knownServers[serverSetup.host + serverSetup.path] = serverSetup;
            }
            return knownServers;
        }

        ServerSetup parseUrl(const std::string& url)
        {
            auto setupBase = ServerSetup{};
            auto baseRequest = cppext::split(url, "?");
            auto base = baseRequest.front();
            auto request = baseRequest.back();

            for(const auto& kv : cppext::split(request, '&'))
            {
                auto keyValue = cppext::split(kv, '=');

                if (keyValue.front() == "map")
                {
                    setupBase.map = keyValue.back();
                }
                else if (keyValue.front() == "token")
                {
                    setupBase.token = keyValue.back();
                }
                else if(keyValue.front() == "layers")
                {
                    setupBase.layerGroup = cppext::split(keyValue.back(), ',');
                }
            }

            setupBase.scheme = cppext::split(base, ':').front();

            auto pos1 = base.find_first_of('/');
            auto startHost = base.find_first_not_of('/', pos1);
            auto endHost = base.find_first_of('/', startHost);

            setupBase.host = base.substr(startHost, endHost - startHost);

            setupBase.path = base.substr(endHost);
            return setupBase;
        }

        // The real Url had to be in coded form in configurations, because "http://wms.fmi.fi/..." contains start of 
        // one line comment with "//" in it and to avoid that we replace "//" characters in configurations with "**" 
        // characters, which now has to be replaced with correct characters to be usefull. So basically this does the following:
        // "http:**wms.fmi.fi/..." => "http://wms.fmi.fi/..."
        std::string fixUrlFromNFmiSettingConfiguration(std::string url)
        {
            // In C++ std::regex you have to escape certain regex specific characters with double slashes (\\),
            // so "**" must be fixed into "\\*\\*"
            return std::regex_replace(url, std::regex("\\*\\*"), "//");
        }

        UserUrlServerSetup parseUserUrl(const std::string& nspace, std::map<std::string, ServerSetup> knownServers, UserUrlServerSetup setupBase)
        {
            auto keys = NFmiSettings::ListChildren(nspace);
            for(const auto& key : keys)
            {
                auto baseMapLayerKey = nspace + "::" + key;
                auto url = NFmiSettings::Require<std::string>(baseMapLayerKey);
                url = fixUrlFromNFmiSettingConfiguration(url);
                auto server = parseUrl(url);
                server.descriptiveName = NFmiSettings::Optional<std::string>(baseMapLayerKey + "::DescriptiveName", "");
                server.macroReference = NFmiSettings::Optional<std::string>(baseMapLayerKey + "::MacroReference", "");

                auto res = knownServers.find(server.host + server.path);
                if(res != knownServers.cend())
                {
                    server.stereo00 = res->second.stereo00;
                    server.stereo10 = res->second.stereo10;
                    server.stereo20 = res->second.stereo20;
                    server.useCrs = res->second.useCrs;
                }
                setupBase.parsedServers.push_back(server);
            }
            return setupBase;
        }

        UserUrlServerSetup parseBase(const std::string& nspace)
        {
            auto serverSetup = UserUrlServerSetup{};
            serverSetup.transparency = NFmiSettings::Optional(nspace + "::Transparency", false);
            serverSetup.version = NFmiSettings::Optional(nspace + "::Version", std::string(""));
            return serverSetup;
        }
    }

    Setup SetupParser::parse(bool doVerboseLogging)
    {
        auto settings = Setup{};
        settings.backgroundBackwardAmount = NFmiSettings::Optional("SmartMet::Wms2::BackgroundFetches::Backward", 1);
        settings.backgroundForwardAmount = NFmiSettings::Optional("SmartMet::Wms2::BackgroundFetches::Forward", 1);
        settings.imageTimeoutInSeconds = NFmiSettings::Optional("SmartMet::Wms2::BackgroundFetches::ImageTimeoutInSeconds", 60);
        settings.legendTimeoutInSeconds = NFmiSettings::Optional("SmartMet::Wms2::BackgroundFetches::LegendTimeoutInSeconds", 30);
        settings.getCapabilitiesTimeoutInSeconds = NFmiSettings::Optional("SmartMet::Wms2::BackgroundFetches::GetCapabilitiesTimeoutInSeconds", 30);
        settings.numberOfCaches = NFmiSettings::Optional("SmartMet::Wms2::Cache::NumberOfCaches", 0);
        settings.numberOfLayersPerCache = NFmiSettings::Optional("SmartMet::Wms2::Cache::NumberOfLayersPerCache", 0);
        settings.proxyUrl = "http://" + NFmiSettings::Optional("SmartMet::Wms2::ProxyUrl", std::string(""));
        settings.intervalToPollGetCapabilities = std::chrono::seconds{ NFmiSettings::Optional("SmartMet::Wms2::GetCapabilities::PollInterval", 5 * 60) };
        settings.renewWmsSystemIntervalInHours = NFmiSettings::Optional<float>("SmartMet::Wms2::GetCapabilities::RenewWmsSystemIntervalInHours", 6.f);
        parseDynamics("SmartMet::Wms2::DynamicDatas", settings.dynamics, doVerboseLogging);

        auto knownServers = parseKnownServers("SmartMet::Wms2::KnownServers");

        auto backgroundBase = parseBase("SmartMet::Wms2::Backgrounds");
        auto overlayBase = parseBase("SmartMet::Wms2::Overlays");

        settings.background = parseUserUrl("SmartMet::Wms2::UserUrls::Backgrounds", knownServers, backgroundBase);
        settings.overlay = parseUserUrl("SmartMet::Wms2::UserUrls::Overlays", knownServers, overlayBase);

        settings.checkForMeaningfulConfigurations();
        return settings;
    }
}