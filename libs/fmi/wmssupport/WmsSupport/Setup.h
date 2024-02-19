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
        // Vanhemmissa servereissä saatetaan käyttää sRS=EPSG:1120 (versio 1.0.0?) tyyliä ja uudemmissa CRS=EPSG:1120
        bool useCrs = true;
        // Optionaalinen nimi karttalayereille ja overlay kuville, jota on tarkoitus käyttää ainakin käyttöliittymien kanssa, 
        // kun käyttäjä esim. vaihtaa karttapohjaa suoraan (nimet menee popup valikkoon tai tooltippiin).
        std::string descriptiveName;
        // Optionaalinen makro referenssi nimi karttalayereille ja overlay kuville, jota käytetään viittäämään
        // käytettyyn karttakuvaan näyttömakrojen yhteydessä. Tällöin näyttömakron säätö on joustavampaa kuin suoraan kuvan 
        // indeksiin (vektorissa) viittaaminen, jos esim. vaihdetaan kuvien järjestystä tai lisätään/poistetaan kuvia listoilta.
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
        // Wms systeemi on mennyt monesti rikki jonkun käynnissä oloajan jälkeen (n. 2-3 vrk), tämän 
        // estämiseksi, smartmetiin tehtiin timer, joka laukaustaan tietyin väliajoin, joka generoi 
        // uuden WmsSupport systeemin ja laittaa sen käytössä olleen tilalle (vanha tuhotaan). Oletusarvo 
        // on 6 h mutta jos tälle antaa arvoksi 0 tai negatiivinen luku, ei timeria käynnistetä ollenkaan.
        float renewWmsSystemIntervalInHours = 6;

        std::chrono::seconds intervalToPollGetCapabilities;

        UserUrlServerSetup background;
        UserUrlServerSetup overlay;
        std::unordered_map<int, DynamicServerSetup> dynamics;

        void checkForMeaningfulConfigurations();
    };
}

