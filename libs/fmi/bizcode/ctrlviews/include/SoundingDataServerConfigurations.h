#pragma once

#include "NFmiCachedRegistryValue.h"
#include "NFmiParameterName.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include <vector>

class NFmiArea;

// T�ss� luokassa on yhden mallin luotausdatan serverihakuihin
// liittyv�t konfiguraatiot:
// Mallin nimi ja id, onko server haku k�yt�ss� ja mik� on serverill� 
// k�ytetty datan nimi.
// Asetukset luetaan sek� lokaali konfiguraatiotiedostosta ett� Windows 
// rekisterist� seuraavilla s��nn�ill�:
// 1. Jos Win-rekisteriss� ei ole asetuksia, k�ytet��n suoraan konffi asetuksia ja talletetaan ne Win-rekisteriin.
// 2. Jos Win-rekisterit� l�ytyy arvot, k�ytet��n niit�, paitsi jos ollaan konffien override-moodissa.
// 3. Jos ollaan konffien override-moodissa, otetaan konffeista kaikki muu paitsi useServerData arvo.
class ModelSoundingDataServerConfigurations
{
    boost::shared_ptr<CachedRegInt> producerId_;
    boost::shared_ptr<CachedRegString> dataNameOnServer_;
    boost::shared_ptr<CachedRegBool> useServerData_;

    // Producer namea ei laiteta rekiteriin suoraan, vaan siit� tehd��n hakemisto/section, johon loput rekisteri arvot laitetaan.
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
    bool useServerData() const { return *useServerData_; }
    void SetUseServerData(bool useServerData) { *useServerData_ = useServerData; }
};


// T�m� luokka tiet�� mille mallidatoille luotaukset haetaan
// smartmet-serverilt� eik� k�ytet� lokaalia querydatoja.
// Se tiet�� onko server optio k�yt�ss� ja mink� niminen data on serverill�.
// Konffit luetaan lokaali konffitiedostoista ja talletetaan Windows rekisteriin.
// Lokaali konffeja voidaan p�ivitt�� 'pakolla' kasvattamalla n�ihin konffeihin liittyv��
// versionumeroa. Jos lokaali tiedostossa on isompi versio kuin rekiteriss�, 
// otetaan kaikki arvot lokaalitiedostosta.
class SoundingDataServerConfigurations
{
    std::vector<ModelSoundingDataServerConfigurations> modelConfigurations_;
    boost::shared_ptr<CachedRegInt> versionNumber_;
    std::string baseRegistryPath_;
    std::string registrySectionName_ = "\\SoundingDataServerConfigurations";
    std::string baseConfigurationPath_;
    std::string smartmetServerBaseUri_;
    std::vector<FmiParameterName> wantedParameters_;
    std::string wantedParametersString_;
    bool initialized_ = false;
public:
    SoundingDataServerConfigurations()
    {}

    bool init(const std::string &baseRegistryPath, const std::string &baseConfigurationPath);
    std::vector<ModelSoundingDataServerConfigurations>& modelConfigurations() { return modelConfigurations_; }
    bool useServerSoundingData(int producerId) const;
    std::string makeFinalServerRequestUri(int producerId, const NFmiMetTime &validTime, const NFmiMetTime &originTime, const NFmiPoint &latlon) const;
    const std::vector<FmiParameterName>& wantedParameters() const { return wantedParameters_; }

private:
    bool mustDoConfigurationOverride(HKEY usedKey);
    ModelSoundingDataServerConfigurations MakeModelConfiguration(const std::string &modelName, bool configurationOverride);
    std::string makeWantedParametersString() const;
    std::string dataNameOnServer(int producerId) const;
};

