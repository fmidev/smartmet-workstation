#pragma once
#include "NFmiCachedRegistryValue.h"
#include <string>

// T�ll� talletetaan paljon MacroParamDataGenerator dialogin juttuja 
// Windows rekisteriin pysyv��n muistiin jossa k�ytet��n
// boost::shared_ptr<CachedRegString> datarakennetta.
class NFmiMacroParamDataGenerator
{
    // General MacroParam Data Generator dialog options
    // Perus smartmet polku Windows rekistereiss� (t�h�n tulee SmartMetin konfiguraatio kohtainen polku)
    std::string mBaseRegistryPath; 
    // Juuri smarttools hakemisto, jossa on \ merkki lopussa
    std::string mRootSmarttoolsDirectory;

    // Pohjadata valitaan antamalla parametri+tuottaja+mahdollinen-level tieto
    // Esim1: T_ec eli Ecmwf:n pinta data, miss� l�mp�tilaparametri
    // Esim2: par4_prod240 eli sama Ec-pinta data, mutta annettuna par+prod id:en avulla
    // Esim2: par4_prod240_lev120 eli ecmwf hybrid data, miss� hybrid-leveli 120
    boost::shared_ptr<CachedRegString> mDialogBaseDataParamProducerLevelString; // PAKOLLINEN
    // Talletettavan datan tuottajan id,name
    // Esim: "1278,My Producer Name"
    boost::shared_ptr<CachedRegString> mDialogUsedProducerString; // PAKOLLINEN
    // Polku (suhteellinen/absoluuttinen) siihen smarttool tiedostoon, mit� k�ytet��n 
    // generoimaan lopullinen data ja sen kaikki parametrit.
    // T�h�n pyrit��n saamaan aina suhteellinen polku, jos se osoittaa smarttool kansioon Dropboxissa.
    boost::shared_ptr<CachedRegString> mDialogDataGeneratingSmarttoolPathString; // PAKOLLINEN
    // Pohjadatasta otetaan muut dimensiot (hplace/vplace/time), mutta k�ytt�j� m��r��
    // mitk� parametrit dataan generoidaan. T�ss� on pilkuilla eroteltuna id1,name1,id2,name2,...
    // Esim. 2501,MyParam1,2502,MyParam2,2503,MyParam3,2504,MyParam4
    boost::shared_ptr<CachedRegString> mDialogUsedParameterListString; // PAKOLLINEN
    // Mihin hakemistoon ja mill� nimell� data talletetaan, kun se generoidaan.
    // T�m� voi olla my�s tyhj�, jolloin data otetaan vain lokaali k�ytt��n Smartmetin sis�iseen muistiin.
    // Esim. D:\data\*_mydata.sqd
    // T�ss� * kohtaan laitetaan lokaaliajan sein�kelloaika sekunteja my�ten:
    // Esim. D:\data\20240912183423_mydata.sqd
    boost::shared_ptr<CachedRegString> mDialogDataStorageFileFilter; // PAKOLLINEN
    // Initialisoinnista raportoiva teksti
    std::string itsInitializeLogStr;
public:
    NFmiMacroParamDataGenerator();
    bool Init(const std::string& theBaseRegistryPath, const std::string& rootSmarttoolsDirectory);

    std::string DialogBaseDataParamProducerLevelString() const;
    void DialogBaseDataParamProducerLevelString(const std::string& newValue);
    std::string DialogDataGeneratingSmarttoolPathString() const;
    void DialogDataGeneratingSmarttoolPathString(const std::string& newValue);
    std::string MakeUsedAbsoluteSmarttoolPathString(const std::string& smarttoolPath) const;
    std::string MakeCleanedSmarttoolPathString(const std::string &smarttoolPath) const;
    std::string DialogUsedParameterListString() const;
    void DialogUsedParameterListString(const std::string& newValue);
    std::string DialogUsedProducerString() const;
    void DialogUsedProducerString(const std::string& newValue);
    std::string DialogDataStorageFileFilter() const;
    void DialogDataStorageFileFilter(const std::string& newValue);

    const std::string GetInitializeLogStr() const { return itsInitializeLogStr; }

private:
};
