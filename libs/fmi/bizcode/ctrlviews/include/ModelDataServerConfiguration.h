#pragma once

#include "NFmiCachedRegistryValue.h"

class NFmiArea;

// T�ss� luokassa on yhden mallin (luotausdatan) serverihakuihin liittyv�t konfiguraatiot:
// Mallin nimi ja id, onko server haku k�yt�ss� ja mik� on serverill� 
// k�ytetty datan nimi.
// Asetukset luetaan sek� lokaali konfiguraatiotiedostosta ett� Windows 
// rekisterist� seuraavilla s��nn�ill�:
// 1. Jos Win-rekisteriss� ei ole asetuksia, k�ytet��n suoraan konffi asetuksia ja talletetaan ne Win-rekisteriin.
// 2. Jos Win-rekisterit� l�ytyy arvot, k�ytet��n niit�, paitsi jos ollaan konffien override-moodissa.
// 3. Jos ollaan konffien override-moodissa, otetaan konffeista kaikki muu paitsi useServerData arvo.
class ModelDataServerConfiguration
{
    boost::shared_ptr<CachedRegInt> producerId_;
    boost::shared_ptr<CachedRegString> dataNameOnServer_;

    // Producer namea ei laiteta rekisteriin suoraan, vaan siit� tehd��n hakemisto/section, johon loput rekisteri arvot laitetaan.
    std::string producerName_;
    std::string baseRegistryPath_;
    std::string baseConfigurationPath_;
    bool initialized_ = false;
public:
    ModelDataServerConfiguration()
    {}

    bool init(const std::string &configurationModelName, const std::string &baseRegistryPath, const std::string &baseConfigurationPath, bool configurationOverride);

    const std::string& producerName() const { return producerName_; }
    int producerId() const { return *producerId_; }
    void setProducerId(int producerId) { *producerId_ = producerId; }
    std::string dataNameOnServer() const { return *dataNameOnServer_; }
    void setDataNameOnServer(const std::string &dataNameOnServer) { *dataNameOnServer_ = dataNameOnServer; }
};

