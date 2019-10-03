#pragma once

#include "NFmiCachedRegistryValue.h"
#include "NFmiParameterName.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include <vector>

class NFmiArea;

// Tässä luokassa on yhden mallin luotausdatan serverihakuihin
// liittyvät konfiguraatiot:
// Mallin nimi ja id, onko server haku käytössä ja mikä on serverillä 
// käytetty datan nimi.
// Asetukset luetaan sekä lokaali konfiguraatiotiedostosta että Windows 
// rekisteristä seuraavilla säännöillä:
// 1. Jos Win-rekisterissä ei ole asetuksia, käytetään suoraan konffi asetuksia ja talletetaan ne Win-rekisteriin.
// 2. Jos Win-rekisteritä löytyy arvot, käytetään niitä, paitsi jos ollaan konffien override-moodissa.
// 3. Jos ollaan konffien override-moodissa, otetaan konffeista kaikki muu paitsi useServerData arvo.
class ModelSoundingDataServerConfigurations
{
    boost::shared_ptr<CachedRegInt> producerId_;
    boost::shared_ptr<CachedRegString> dataNameOnServer_;

    // Producer namea ei laiteta rekiteriin suoraan, vaan siitä tehdään hakemisto/section, johon loput rekisteri arvot laitetaan.
    std::string producerName_;
    std::string baseRegistryPath_;
    std::string baseConfigurationPath_;
    bool initialized_ = false;
public:
    ModelSoundingDataServerConfigurations()
    {}

    bool init(const std::string &configurationModelName, const std::string &baseRegistryPath, const std::string &baseConfigurationPath, bool configurationOverride);

    const std::string& producerName() const { return producerName_; }
    int producerId() const { return *producerId_; }
    void SetProducerId(int producerId) { *producerId_ = producerId; }
    std::string dataNameOnServer() const { return *dataNameOnServer_; }
    void SetDataNameOnServer(const std::string &dataNameOnServer) { *dataNameOnServer_ = dataNameOnServer; }
};


// Tämä luokka tietää mille mallidatoille luotaukset haetaan
// smartmet-serveriltä eikä käytetä lokaalia querydatoja.
// Se tietää onko server optio käytössä ja minkä niminen data on serverillä.
// Konffit luetaan lokaali konffitiedostoista ja talletetaan Windows rekisteriin.
// Lokaali konffeja voidaan päivittää 'pakolla' kasvattamalla näihin konffeihin liittyvää
// versionumeroa. Jos lokaali tiedostossa on isompi versio kuin rekiterissä, 
// otetaan kaikki arvot lokaalitiedostosta.
class SoundingDataServerConfigurations
{
    std::vector<ModelSoundingDataServerConfigurations> modelConfigurations_;
    boost::shared_ptr<CachedRegInt> versionNumber_;
    std::string baseRegistryPath_;
    std::string registrySectionName_ = "\\SoundingDataServerConfigurations";
    std::string baseConfigurationPath_;
    std::vector<std::string> serverBaseUrls_;
    boost::shared_ptr<CachedRegInt> selectedBaseUrlIndex_;
    std::vector<FmiParameterName> wantedParameters_;
    std::string wantedParametersString_;
    bool initialized_ = false;
public:
    SoundingDataServerConfigurations()
    {}

    bool init(const std::string &baseRegistryPath, const std::string &baseConfigurationPath);
    std::vector<ModelSoundingDataServerConfigurations>& modelConfigurations() { return modelConfigurations_; }
    std::string makeFinalServerRequestUrl(int producerId, const NFmiMetTime &validTime, const NFmiPoint &latlon) const;
    const std::vector<FmiParameterName>& wantedParameters() const { return wantedParameters_; }
    const std::vector<std::string>& serverBaseUrls() const { return serverBaseUrls_; }
    int selectedBaseUrlIndex() const;
    void setSelectedBaseUrlIndex(int newValue);
    const std::string& getSelectedBaseUrl() const;

private:
    bool mustDoConfigurationOverride(HKEY usedKey);
    ModelSoundingDataServerConfigurations MakeModelConfiguration(const std::string &modelName, bool configurationOverride);
    std::string makeWantedParametersString() const;
    std::string dataNameOnServer(int producerId) const;
    void initBaseUrlVector();
};

