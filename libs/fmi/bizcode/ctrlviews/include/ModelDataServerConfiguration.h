#pragma once

#include "NFmiCachedRegistryValue.h"
#include "NFmiParameterName.h"

class NFmiArea;

const std::string kelvinToCelsiusFunction = "K2C";
const std::string normalToPercentFunction = "NORM2PERCENT";

// Muodostetaan esim. suraavasta mappaus stringistä: T-K;4;K2C
// 1. T-K on parametrin nimi mitä käytetään grib-data haussa.
// 2. 4 on sen vastine id newbasessa, esim. T-K (lämpötila kelvineitä) muutetaan Temperature:ksi eli id 4.
// 3. K2C on kelvin-to-celsius muunnos funktion nimi, kaikilla parametreilla ei ole muunnosfunktiota käytössä.
class GribDataParameterMapping
{
    std::string gribDataName_;
    FmiParameterName newbaseParameterId_ = kFmiBadParameter;
    std::string conversionFunctionName_;
public:
    GribDataParameterMapping();
    void init(const std::string& mappingStr);

    const std::string& gribDataName() const { return gribDataName_; }
    FmiParameterName newbaseParameterId() const { return newbaseParameterId_; }
    const std::string& conversionFunctionName() const { return conversionFunctionName_; }

    static double doValueConversion(double value, const std::string& conversionFunctionName);
};

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

    // Grib pohjaisia luotausdatoja pitää nykyään hakea erityishaulla.
    // Näitä ei talletetan rekistereihin, koska näitä pitää pystyä säätämään lennossa.
    // Onko joku tuottaja grib-data pohjainen vai ei, jos on, niin myös seuraaville asetuksille pitäisi saada arvoja
    bool gribDataCase_;
    // Grib tapauksissa pitää antaa myös shortname
    std::string gribShortName_;
    // timeseries grib-data haussa pitää tehdä server/data kohtaiset &param=xxx asetukset url:iin. ECG:n tapauksessa esim:
    // LIST{T-K:ECG:1007:3:120-137:1},LIST{RH-PRCNT:ECGMTA:1007:3:120-137:1},LIST{U-MS:ECG:1007:3:120-137:1},
    // LIST{V-MS:ECG:1007:3:120-137:1},LIST{P-HPA:ECGMTA:1007:3:120-137:1},LIST{HL-M:ECGMTA:1007:3:120-137:1},
    // LIST{N-0TO1:ECG:1007:3:120-137:1}
    // ######### tai jos käytetään %levels% placeholderia:
    // LIST{T-K:ECG:1007:3:%levels%:1},LIST{RH-PRCNT:ECGMTA:1007:3:%levels%:1},LIST{U-MS:ECG:1007:3:%levels%:1},
    // LIST{V-MS:ECG:1007:3:%levels%:1},LIST{P-HPA:ECGMTA:1007:3:%levels%:1},LIST{HL-M:ECGMTA:1007:3:%levels%:1},
    // LIST{N-0TO1:ECG:1007:3:%levels%:1}
    std::string gribDataParams_;
    // Tähän tehtaillaan lopullinen url parametrin arvo, kun siihen on ensin yhdistetty mahdollinen gribDataLevels_:in sisältämä tieto
    std::string finalGribDataParamsStr_;
    // timeseries grid-data haussa haetaan yksi parametri seuraavalla tavalla
    // LIST{T-K:ECG:1007:3:120-137:1}, missä LIST on joukko leveleitä, T-K on grib-datan parametrin nimi,
    // 1007 on alue eli tässä globaali data, 3 on mallipintadata, 120-137 on halutut levelit (joista lista muodostuu),
    // 1 on mysteeri.
    // Jos gribDataLevels_:ille löytyy arvo, esim. 120-137, sen sisältö laitetaan gribDataParams_:issa kaikkiin %levels% kohtiin,
    // jotta haetut levelit voidaan halutessa säätää kerralla konffissa.
    std::string gribDataLevels_;
    // Koska grib-datojen kanssa on erilaiset parametri mappaukset, kuin querydata tapauksissa, pitää parametrit mapata.
    // Lisäksi pitää olla mahdollista tehdä parametreille konversioita, koska LIST datoja ei voi komentorivillä konvertoida.
    // Esim. T-K;4;K2C,RH-PRCNT;13;,U-MS;23;,V-MS;24;,P-HPA;1;,HL-M;3;,N-0TO1;79;NORM2PERCENT
    // Pilkuilla ',' erotellaan eri parametrit ja puolipilkuilla ';' erotellaan yhden parametrin seuraavat asiat esim. T-K;4;K2C:
    // T-K on grib-data nimi, 4 on parametrin newbase id, K2C on mahdollinen muunnosfunktio eli kelvin-to-celsius, voi olla 
    // tyhjä jos ei ole mitään muunnosta.
    std::string gribDataParamsMappingStr_;
    // Lista GribDataParameterMapping olioita, joilla grib-data muutetaan haluttuun muotoon newbase maailmassa.
    std::vector<GribDataParameterMapping> gribDataParamsMapping_;

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

    bool gribDataCase() const { return gribDataCase_; }
    const std::string& gribShortName() const { return gribShortName_; }
    const std::string& finalGribDataParamsStr() const { return finalGribDataParamsStr_; }
    const std::vector<GribDataParameterMapping>& gribDataParamsMapping() const { return gribDataParamsMapping_; }

private:
    bool initGribDataPart(const std::string& usedConfigurationPath);
    std::string makeFinalGribDataParamsStr(const std::string& originalParamStr, const std::string& possibleLevelsStr);
    std::vector<GribDataParameterMapping> makeGribDataParamsMappings(const std::string& gribDataParamsMappingStr);
};

