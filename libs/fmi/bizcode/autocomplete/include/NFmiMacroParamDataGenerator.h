#pragma once
#include "NFmiCachedRegistryValue.h"
#include "NFmiExtraMacroParamData.h"
#include "NFmiParamBag.h"
#include "json_spirit_value.h"
#include <string>

class NFmiQueryData;
class NFmiFastQueryInfo;
class NFmiInfoOrganizer;
class NFmiThreadCallBacks;

// T�m� on yksi erillinen MacroParam dataan liittyvien datojen kokoelma.
// Eli systeemiss� voi olla useita erilaisia MacroParam datojen reseptej�,
// ja nee laitetaan t�ll�isiin otuksiin.
// Inputit tarkistet��n t�m�n avulla.
class NFmiMacroParamDataInfo
{
    // Pohjadata valitaan antamalla parametri+tuottaja tieto (ei level tietoa ainakaan toistaiseksi)
    // Esim1: T_ec eli Ecmwf:n pinta data, miss� l�mp�tilaparametri
    // Esim2: par4_prod240 eli sama Ec-pinta data, mutta annettuna par+prod id:en avulla
    std::string mBaseDataParamProducerString; // PAKOLLINEN
    // Talletettavan datan tuottajan id,name
    // Esim: "1278,ProducerName"
    std::string mUsedProducerString; // PAKOLLINEN
    // Polku (suhteellinen/absoluuttinen) siihen smarttool tiedostoon, mit� k�ytet��n 
    // generoimaan lopullinen data ja sen kaikki parametrit.
    // T�h�n pyrit��n saamaan aina suhteellinen polku, jos se osoittaa smarttool kansioon Dropboxissa.
    std::string mDataGeneratingSmarttoolPathString; // PAKOLLINEN
    // Pohjadatasta otetaan muut dimensiot (hplace/vplace/time), mutta k�ytt�j� m��r��
    // mitk� parametrit dataan generoidaan. T�ss� on pilkuilla eroteltuna id1,name1,id2,name2,...
    // Esim. 2501,MyParam1,2502,MyParam2,2503,MyParam3,2504,MyParam4
    std::string mUsedParameterListString; // PAKOLLINEN
    // Mihin hakemistoon ja mill� nimell� data talletetaan, kun se generoidaan.
    // T�m� voi olla my�s tyhj�, jolloin data otetaan vain lokaali k�ytt��n Smartmetin sis�iseen muistiin.
    // Esim. D:\data\*_mydata.sqd
    // T�ss� * kohtaan laitetaan lokaaliajan sein�kelloaika sekunteja my�ten:
    // Esim. D:\data\20240912183423_mydata.sqd
    std::string mDataStorageFileFilter; // PAKOLLINEN

    // Initialisoinnista raportoiva teksti
    std::string itsInitializeLogStr;
    bool fDataChecked = false;
public:
    NFmiMacroParamDataInfo();
    NFmiMacroParamDataInfo(const std::string &baseDataParamProducerLevelString, const std::string &usedProducerString, const std::string &dataGeneratingSmarttoolPathString, const std::string &usedParameterListString, const std::string &dataStorageFileFilter);

    bool CheckData();
    bool DataChecked() const { return fDataChecked; }
    const std::string GetInitializeLogStr() const { return itsInitializeLogStr; }

    const std::string& BaseDataParamProducerString() const { return mBaseDataParamProducerString; }
    const std::string& UsedProducerString() const { return mUsedProducerString; }
    const std::string& DataGeneratingSmarttoolPathString() const { return mDataGeneratingSmarttoolPathString; }
    const std::string& UsedParameterListString() const { return mUsedParameterListString; }
    const std::string& DataStorageFileFilter() const { return mDataStorageFileFilter; }

    static json_spirit::Object MakeJsonObject(const NFmiMacroParamDataInfo& macroParamDataInfo);
    void ParseJsonPair(json_spirit::Pair& thePair);
    static bool StoreInJsonFormat(const NFmiMacroParamDataInfo& macroParamDataInfo, const std::string& theFilePath, std::string& theErrorStringOut);
    static bool ReadInJsonFormat(NFmiMacroParamDataInfo& macroParamDataInfoOut, const std::string& theFilePath, std::string& theErrorStringOut);

    static std::pair<std::string, NFmiDefineWantedData> CheckBaseDataParamProducerString(const std::string& baseDataParamProducerString);
    static std::pair<std::string, std::vector<std::string>> CheckUsedProducerString(const std::string& usedProducerString);
    static std::pair<std::string, NFmiParamBag> CheckUsedParameterListString(const std::string usedParameterListString, const NFmiProducer &wantedProducer);
    static std::string CheckDataStorageFileFilter(const std::string& dataStorageFileFilter);
    static std::string CheckDataGeneratingSmarttoolPathString(const std::string& dataGeneratingSmarttoolPathString);
    static std::string MakeDataStorageFilePath(const std::string& dataStorageFileFilter);
};

inline unsigned int ID_MACRO_PARAM_DATA_GENERATION_FINISHED = 23423;

// T�ll� talletetaan paljon MacroParamDataGenerator dialogin juttuja 
// Windows rekisteriin pysyv��n muistiin jossa k�ytet��n
// boost::shared_ptr<CachedRegString> datarakennetta.
// Luokan tarkoitus on luoda uusia MacroParam datoja halutuilla skripteill� ja asetuksilla.
class NFmiMacroParamDataGenerator
{
    // General MacroParam Data Generator dialog options
    // Perus smartmet polku Windows rekistereiss� (t�h�n tulee SmartMetin konfiguraatio kohtainen polku)
    std::string mBaseRegistryPath; 
    // Juuri smarttool hakemisto, jossa on \ merkki lopussa
    static std::string mRootSmarttoolDirectory;

    // HUOM! mDialog -alkuiset data memberit pit�v�t sis�ll��n vastaavan dialogin kenttien sis�ll�t.

    // Pohjadata valitaan antamalla parametri+tuottaja tieto (ei level tietoa ainakaan toistaiseksi)
    // Esim1: T_ec eli Ecmwf:n pinta data, miss� l�mp�tilaparametri
    // Esim2: par4_prod240 eli sama Ec-pinta data, mutta annettuna par+prod id:en avulla
    boost::shared_ptr<CachedRegString> mDialogBaseDataParamProducerString; // PAKOLLINEN
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
    // Dialogi muistaa minne/mist� on talletettu/ladattu viimeksi MacroParam data info tiedosto
    boost::shared_ptr<CachedRegString> mMacroParamDataInfoSaveInitialPath;


    // Initialisoinnista raportoiva teksti
    std::string itsInitializeLogStr;
    // Mahdollinen smarttool laskuihin tai sen alustuksiin liittyvi� ongelmia laitetaan t�h�n talteen
    std::string itsSmarttoolCalculationLogStr;
    // Tiedosto josta luettiin smarttool
    std::string mUsedAbsoluteSmarttoolPath;
public:
    static const std::string itsMacroParamDataInfoFileExtension;
    static const std::string itsMacroParamDataInfoFileFilter;

    NFmiMacroParamDataGenerator();
    bool Init(const std::string& theBaseRegistryPath, const std::string& rootSmarttoolDirectory);

    bool GenerateMacroParamData(NFmiThreadCallBacks* threadCallBacks);

    std::string DialogBaseDataParamProducerString() const;
    void DialogBaseDataParamProducerString(const std::string& newValue);
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

    const std::string& GetInitializeLogStr() const { return itsInitializeLogStr; }
    const std::string& GetSmarttoolCalculationLogStr() const { return itsSmarttoolCalculationLogStr; }
    NFmiMacroParamDataInfo MakeDataInfo() const;
    std::string MacroParamDataInfoSaveInitialPath();
    void MacroParamDataInfoSaveInitialPath(const std::string& newValue);

    static const std::string& RootSmarttoolDirectory() { return mRootSmarttoolDirectory; }

private:
    bool CalculateDataWithSmartTool(boost::shared_ptr<NFmiFastQueryInfo>& wantedMacroParamInfoPtr, NFmiInfoOrganizer* infoOrganizer, const std::string& smartToolText, NFmiThreadCallBacks *threadCallBacks);
    std::string ReadSmarttoolContentFromFile(const std::string& filePath);
    bool StoreMacroParamData(boost::shared_ptr<NFmiQueryData>& macroParamDataPtr, const std::string& dataStorageFileFilter);
    bool GenerateMacroParamData(const NFmiMacroParamDataInfo &dataInfo, NFmiThreadCallBacks* threadCallBacks);
};
