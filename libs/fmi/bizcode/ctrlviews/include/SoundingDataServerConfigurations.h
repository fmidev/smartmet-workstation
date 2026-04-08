#pragma once

#ifndef UNIX
#include "NFmiCachedRegistryValue.h"
#endif // UNIX
#include "NFmiParameterName.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "ModelDataServerConfiguration.h"
#include <vector>

class NFmiArea;
class ModelDataServerConfiguration;

// T�m� luokka tiet�� mille mallidatoille luotaukset haetaan smartmet-serverilt� eik� k�ytet� lokaali querydatoja.
// Se tiet�� onko server optio k�yt�ss� ja mink� niminen data on serverill�.
// Konffit luetaan lokaali konffitiedostoista ja talletetaan Windows rekisteriin.
// Lokaali konffeja voidaan p�ivitt�� 'pakolla' kasvattamalla n�ihin konffeihin liittyv��
// versionumeroa. Jos lokaali tiedostossa on isompi versio kuin rekiteriss�, 
// otetaan kaikki arvot lokaalitiedostosta.
class SoundingDataServerConfigurations
{
    std::vector<ModelDataServerConfiguration> modelConfigurations_;
#ifndef UNIX
    boost::shared_ptr<CachedRegInt> versionNumber_;
#else
    int versionNumber_ = 0;
#endif
    std::string baseRegistryPath_;
    std::string registrySectionName_ = "\\SoundingDataServerConfigurations";
    std::string baseConfigurationPath_;
    std::vector<std::string> serverBaseUrls_;
#ifndef UNIX
    boost::shared_ptr<CachedRegInt> selectedBaseUrlIndex_;
#else
    int selectedBaseUrlIndex_ = 0;
#endif
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
#ifndef UNIX
    bool mustDoConfigurationOverride(HKEY usedKey);
#endif
    ModelDataServerConfiguration MakeModelConfiguration(const std::string &modelName, bool configurationOverride);
    std::string makeWantedParametersString() const;
    std::string dataNameOnServer(int producerId) const;
    void initBaseUrlVector();
    std::string makeFinalGribDataServerRequestUrl(const std::string& baseRequestStr, const ModelDataServerConfiguration& gribDataServerConfiguration, const NFmiMetTime& validTime, const NFmiPoint& latlon) const;
    std::string makeBaseUrlParameterString(const NFmiMetTime& validTime, const NFmiPoint& latlon) const;
};

