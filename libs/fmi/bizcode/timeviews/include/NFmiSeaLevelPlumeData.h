#pragma once

#include "NFmiStation.h"
#include "NFmiProducer.h"
#include "NFmiParam.h"
#include "NFmiColor.h"
#include "NFmiParameterName.h"
#include "NFmiInfoData.h"

class NFmiDataIdent;

    // SeaLevelProbData luokka pit‰‰ sis‰ll‰‰n sea-level parametriin liittyvien probability rajojen vakio arvoja.
    // Jokaiselle tunnetulle seaLevel asemalle annetaan erikseen siihen liittyv‰t rajat.
    // Kun Kartalta valitaan paikka, joka halutaan piirt‰‰ aikasarjaan, etsit‰‰n 300 km l‰heisyydest‰ l‰hin asema. Jos kyseisen aseman 
    // stationId lˆytyy oliosta, k‰ytet‰‰n sen prob rajoja piirt‰m‰‰n rajat aikasarjaan.
class NFmiSeaLevelProbData
{
public:
    NFmiSeaLevelProbData() = default;
    NFmiSeaLevelProbData(const NFmiStation& station, float prob1, float prob2, float prob3, float prob4)
        :station_(station),
        prob1_(prob1),
        prob2_(prob2),
        prob3_(prob3),
        prob4_(prob4)
    {}

    NFmiStation station_;
    float prob1_ = 0;
    float prob2_ = 0;
    float prob3_ = 0;
    float prob4_ = 0;
};

class NFmiSingleSeaLevelPlumeData
{
    unsigned long seaLevelParameterId_ = kFmiBadParameter;
    NFmiProducer producer_;
    NFmiInfoData::Type dataType_ = NFmiInfoData::kNoDataType;
    std::vector<NFmiParam> fractileParams_;
    std::vector<NFmiColor> fractileParamColors_;
    std::vector<NFmiSeaLevelProbData> probabilityStationData_;
    // ProbLimit parametrit 1-4
    std::vector<NFmiParam> probLimitParams_;
    // ProbLimit 1-4 viiva v‰rit: keltainen, keltainen, oranssi ja punainen
    std::vector<NFmiColor> probabilityLineColors_;
    const double probabilityMaxSearchRangeInMetres_ = 250 * 1000;
    std::string settingsBaseKey_;
    bool initialized_ = false;
    bool dataOk_ = false;
    bool foundAnySettings_ = false;
    std::string configurationErrorMessage_;
public:
    NFmiSingleSeaLevelPlumeData();
    void InitFromSettings(const std::string& baseKey);
    bool dataOk() const;
    bool foundAnySettings() const { return foundAnySettings_; }
    const std::string& configurationErrorMessage() const { return configurationErrorMessage_; }

    const NFmiProducer& producer() const { return producer_; }
    NFmiInfoData::Type dataType() const { return dataType_; }
    const std::vector<NFmiParam>& fractileParams() const { return fractileParams_; }
    const std::vector<NFmiColor>& fractileParamColors() const { return fractileParamColors_; }
    const std::vector<NFmiParam>& probLimitParams() const { return probLimitParams_; }
    const std::vector<NFmiColor>& probabilityLineColors() const { return probabilityLineColors_; }
    double probabilityMaxSearchRangeInMetres() const { return probabilityMaxSearchRangeInMetres_; }

    const NFmiSeaLevelProbData* FindSeaLevelProbabilityStationData(const NFmiLocation* location, const NFmiPoint& latlon) const;

    bool IsSeaLevelPlumeParam(unsigned long checkedParameterId) const;
    bool IsSeaLevelProbLimitParam(const NFmiDataIdent& dataIdent) const;
};

class NFmiSeaLevelPlumeData
{
    std::vector<NFmiSingleSeaLevelPlumeData> plumeDataContainer_;
    std::string settingsBaseKey_;
    bool initialized_ = false;
    std::string baseConfigurationMessage_;
    std::string configurationErrorMessage_;
public:
    NFmiSeaLevelPlumeData();
    void InitFromSettings(const std::string& baseKey);
    const std::string& baseConfigurationMessage() const { return baseConfigurationMessage_; }
    const std::string& configurationErrorMessage() const { return configurationErrorMessage_; }

    const NFmiSingleSeaLevelPlumeData* getSeaLevelPlumeData(unsigned long checkedParameterId) const;
};
