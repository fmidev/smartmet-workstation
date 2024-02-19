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
        // Vanhemmissa servereiss� saatetaan k�ytt�� sRS=EPSG:1120 (versio 1.0.0?) tyyli� ja uudemmissa CRS=EPSG:1120
        bool useCrs = true;
        // Optionaalinen nimi karttalayereille ja overlay kuville, jota on tarkoitus k�ytt�� ainakin k�ytt�liittymien kanssa, 
        // kun k�ytt�j� esim. vaihtaa karttapohjaa suoraan (nimet menee popup valikkoon tai tooltippiin).
        std::string descriptiveName;
        // Optionaalinen makro referenssi nimi karttalayereille ja overlay kuville, jota k�ytet��n viitt��m��n
        // k�ytettyyn karttakuvaan n�ytt�makrojen yhteydess�. T�ll�in n�ytt�makron s��t� on joustavampaa kuin suoraan kuvan 
        // indeksiin (vektorissa) viittaaminen, jos esim. vaihdetaan kuvien j�rjestyst� tai lis�t��n/poistetaan kuvia listoilta.
        std::string macroReference;
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
        bool acceptTimeDimensionalLayersOnly = false;
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
        int imageTimeoutInSeconds;
        int legendTimeoutInSeconds;
        int getCapabilitiesTimeoutInSeconds;
        // Wms systeemi on mennyt monesti rikki jonkun k�ynniss� oloajan j�lkeen (n. 2-3 vrk), t�m�n 
        // est�miseksi, smartmetiin tehtiin timer, joka laukaustaan tietyin v�liajoin, joka generoi 
        // uuden WmsSupport systeemin ja laittaa sen k�yt�ss� olleen tilalle (vanha tuhotaan). Oletusarvo 
        // on 6 h mutta jos t�lle antaa arvoksi 0 tai negatiivinen luku, ei timeria k�ynnistet� ollenkaan.
        float renewWmsSystemIntervalInHours = 6;

        std::chrono::seconds intervalToPollGetCapabilities;

        UserUrlServerSetup background;
        UserUrlServerSetup overlay;
        std::unordered_map<int, DynamicServerSetup> dynamics;

        void checkForMeaningfulConfigurations();
    };
}

