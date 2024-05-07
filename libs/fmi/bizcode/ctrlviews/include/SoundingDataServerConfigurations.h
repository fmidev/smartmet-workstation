#pragma once

#include "NFmiCachedRegistryValue.h"
#include "NFmiParameterName.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "ModelDataServerConfiguration.h"
#include <vector>

class NFmiArea;
class ModelDataServerConfiguration;

// T‰m‰ luokka tiet‰‰ mille mallidatoille luotaukset haetaan smartmet-serverilt‰ eik‰ k‰ytet‰ lokaali querydatoja.
// Se tiet‰‰ onko server optio k‰ytˆss‰ ja mink‰ niminen data on serverill‰.
// Konffit luetaan lokaali konffitiedostoista ja talletetaan Windows rekisteriin.
// Lokaali konffeja voidaan p‰ivitt‰‰ 'pakolla' kasvattamalla n‰ihin konffeihin liittyv‰‰
// versionumeroa. Jos lokaali tiedostossa on isompi versio kuin rekiteriss‰, 
// otetaan kaikki arvot lokaalitiedostosta.
class SoundingDataServerConfigurations
{
    std::vector<ModelDataServerConfiguration> modelConfigurations_;
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
    std::vector<ModelDataServerConfiguration>& modelConfigurations() { return modelConfigurations_; }
    std::string makeFinalServerRequestUrl(int producerId, const NFmiMetTime &validTime, const NFmiPoint &latlon) const;
    const std::vector<FmiParameterName>& wantedParameters() const { return wantedParameters_; }
    const std::vector<std::string>& serverBaseUrls() const { return serverBaseUrls_; }
    int selectedBaseUrlIndex() const;
    void setSelectedBaseUrlIndex(int newValue);
    const std::string& getSelectedBaseUrl() const;
    const ModelDataServerConfiguration* getServerConfiguration(int producerId) const;

private:
    bool mustDoConfigurationOverride(HKEY usedKey);
    ModelDataServerConfiguration MakeModelConfiguration(const std::string &modelName, bool configurationOverride);
    std::string makeWantedParametersString() const;
    std::string dataNameOnServer(int producerId) const;
    void initBaseUrlVector();
    std::string makeFinalGribDataServerRequestUrl(const std::string& baseRequestStr, const ModelDataServerConfiguration& gribDataServerConfiguration, const NFmiMetTime& validTime, const NFmiPoint& latlon) const;
    std::string makeBaseUrlParameterString(const NFmiMetTime& validTime, const NFmiPoint& latlon) const;
};

