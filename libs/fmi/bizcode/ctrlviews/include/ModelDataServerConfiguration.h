#pragma once

#include "NFmiCachedRegistryValue.h"

class NFmiArea;

// Tässä luokassa on yhden mallin (luotausdatan) serverihakuihin liittyvät konfiguraatiot:
// Mallin nimi ja id, onko server haku käytössä ja mikä on serverillä 
// käytetty datan nimi.
// Asetukset luetaan sekä lokaali konfiguraatiotiedostosta että Windows 
// rekisteristä seuraavilla säännöillä:
// 1. Jos Win-rekisterissä ei ole asetuksia, käytetään suoraan konffi asetuksia ja talletetaan ne Win-rekisteriin.
// 2. Jos Win-rekisteritä löytyy arvot, käytetään niitä, paitsi jos ollaan konffien override-moodissa.
// 3. Jos ollaan konffien override-moodissa, otetaan konffeista kaikki muu paitsi useServerData arvo.
class ModelDataServerConfiguration
{
    boost::shared_ptr<CachedRegInt> producerId_;
    boost::shared_ptr<CachedRegString> dataNameOnServer_;

    // Producer namea ei laiteta rekisteriin suoraan, vaan siitä tehdään hakemisto/section, johon loput rekisteri arvot laitetaan.
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

