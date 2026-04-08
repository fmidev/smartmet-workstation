#pragma once
#ifndef UNIX
#include "NFmiCachedRegistryValue.h"
#else
#include "linux_compat.h"
#endif
#include "NFmiExtraMacroParamData.h"
#include "NFmiParamBag.h"
#include "NFmiMetTime.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiQueryDataUtil.h"
#include "json_spirit_value.h"
#include <string>
#include <mutex>

class NFmiQueryData;
class NFmiFastQueryInfo;
class NFmiInfoOrganizer;
class NFmiThreadCallBacks;
class NFmiMilliSecondTimer;

const std::string gDefaultBaseDataGridScaleString = "1";
const NFmiPoint gDefaultBaseDataGridScaleValues = NFmiPoint(1, 1);

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
    // T�ss� voi olla pilkulla eroteltu lista polkuja, koska joskus pit�� generoida data useammasta eri 
    // smarttoolista, koska niiss� tulisi muuten saman nimisten var muuttujien t�rm�yksi�.
    std::string mDataGeneratingSmarttoolPathListString; // PAKOLLINEN
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
    // Lista parametreja eri datoihin, mitk� laukaisevat datan generoinnin, jos sellaisia on
    // k�yt�ss� ja automaattinen datan generaatio on k�yt�ss�.
    // Esim. T_ec[,T_gfs_500, ...] ja my�h�stetty laukaisu T_ec[0.5h] (-> 0.5 h my�h�stys)
    std::string mDataTriggerList; // Ei pakollinen
    std::vector<NFmiDefineWantedData> mWantedDataTriggerList;
    // Kun dataa generoidaan, kuinka monta viimeisint� kyseist� dataa pidet��n
    // kohdehakemistossa. Jos datan luomisen j�lkeen siell� on enemm�n kyseisi� tiedostoja,
    // deletoidaan vanhimmat niist� pois kyseisell� filefilterill�.
    int mMaxGeneratedFilesKept = 2;
    // Jos k�ytt�j� haluaa harventaa basedatasta saatua laskentahilaa, t�ll� kertoimella 
    // se voidaan tehd�. Jos oletusarvossa eli 1, t�ll�in mit��n skaalausta ei tehd� hilankokoon.
    // Jos arvo 2, t�ll�in x- ja y-suuntaista hilakoko jaetaan 2:lla ja py�ristet��n l�himp��n kokonaislukuun.
    // Jos arvo 2,1.5, t�ll�i x koko jaetaan 2:lla ja y suunta 1.5:ll�.
    // Skaalausarvojen pit�� olla v�lill� 1-10.
    std::string mBaseDataGridScaleString = gDefaultBaseDataGridScaleString; // pakollinen
    NFmiPoint mBaseDataGridScaleValues = gDefaultBaseDataGridScaleValues;
    // Initialisoinnista raportoiva teksti
    std::string itsCheckShortStatusStr;
    bool fDataChecked = false;
public:
    NFmiMacroParamDataInfo();
    NFmiMacroParamDataInfo(const std::string &baseDataParamProducerLevelString, const std::string &usedProducerString, const std::string &dataGeneratingSmarttoolPathListString, const std::string &usedParameterListString, const std::string &dataStorageFileFilter, const std::string& dataTriggerList, int maxGeneratedFilesKept, const std::string &baseDataGridScaleString);

    bool CheckData();
    bool DataChecked() const { return fDataChecked; }
    const std::string CheckShortStatusStr() const { return itsCheckShortStatusStr; }

    const std::string& BaseDataParamProducerString() const { return mBaseDataParamProducerString; }
    void BaseDataParamProducerString(const std::string& newValue) { mBaseDataParamProducerString = newValue; }
    const std::string& UsedProducerString() const { return mUsedProducerString; }
    void UsedProducerString(const std::string& newValue) { mUsedProducerString = newValue; }
    const std::string& DataGeneratingSmarttoolPathListString() const { return mDataGeneratingSmarttoolPathListString; }
    void DataGeneratingSmarttoolPathListString(const std::string& newValue) { mDataGeneratingSmarttoolPathListString = newValue; }
    const std::string& UsedParameterListString() const { return mUsedParameterListString; }
    void UsedParameterListString(const std::string& newValue) { mUsedParameterListString = newValue; }
    const std::string& DataStorageFileFilter() const { return mDataStorageFileFilter; }
    void DataStorageFileFilter(const std::string& newValue) { mDataStorageFileFilter = newValue; }
    const std::string& DataTriggerList() const { return mDataTriggerList; }
    void DataTriggerList(const std::string& newValue) { mDataTriggerList = newValue; }
    int MaxGeneratedFilesKept() const { return mMaxGeneratedFilesKept; }
    void MaxGeneratedFilesKept(int newValue);
    const std::string& BaseDataGridScaleString() const { return mBaseDataGridScaleString; }
    void BaseDataGridScaleString(const std::string& newValue);
    const NFmiPoint& BaseDataGridScaleValues() const { return mBaseDataGridScaleValues; }
    void CorrectMaxGeneratedFilesKeptValue();
    void CorrectBaseDataGridScaleValue();
    std::string MakeShortStatusErrorString();
    const std::vector<NFmiDefineWantedData>& WantedDataTriggerList() const { return mWantedDataTriggerList; }

    static json_spirit::Object MakeJsonObject(const NFmiMacroParamDataInfo& macroParamDataInfo);
    void ParseJsonPair(json_spirit::Pair& thePair);
    static bool StoreInJsonFormat(const NFmiMacroParamDataInfo& macroParamDataInfo, const std::string& theFilePath, std::string& theErrorStringOut);
    static bool ReadInJsonFormat(NFmiMacroParamDataInfo& macroParamDataInfoOut, const std::string& theFilePath, std::string& theErrorStringOut);

    static std::pair<std::string, NFmiDefineWantedData> CheckBaseDataParamProducerString(const std::string& baseDataParamProducerString, bool allowLevelData);
    static std::pair<std::string, std::vector<std::string>> CheckUsedProducerString(const std::string& usedProducerString);
    static std::pair<std::string, NFmiParamBag> CheckUsedParameterListString(const std::string usedParameterListString, const NFmiProducer &wantedProducer);
    static std::string CheckDataStorageFileFilter(const std::string& dataStorageFileFilter);
    static std::string CheckDataGeneratingSmarttoolPathListString(const std::string& dataGeneratingSmarttoolPathListString);
    static std::pair<std::string, std::vector<NFmiDefineWantedData>> CheckDataTriggerListString(const std::string& dataTriggerListString);
    static std::pair<std::string, NFmiPoint> CheckBaseDataGridScaleString(const std::string& baseDataGridScaleString);
    static std::string MakeDataStorageFilePath(const std::string& dataStorageFileFilter);
    static int FixMaxGeneratedFilesKeptValue(int newValue);
    static std::pair<bool, NFmiPoint> CalcBaseDataGridScaleValues(const std::string& baseDataGridScaleString);
    static std::string MakeBaseDataGridScaleString(NFmiPoint baseDataGridScaleValues);
};

enum class MacroParamDataStatus
{
    kFmiListItemOk = 0,
    kFmiListItemReadError = 1,
    kFmiListItemAutomationError = 2,
    kFmiListItemSameAutomationsInList = 3
};

class NFmiMacroParamDataAutomationListItem
{
public:

    NFmiMacroParamDataAutomationListItem();
    NFmiMacroParamDataAutomationListItem(const std::string& theBetaAutomationPath);

    void DoFullChecks(bool automationModeOn); // T�t� kutsutaan kun esim. luetaan data tiedostosta ja tehd��n t�ysi tarkistus kaikille osille
    bool IsEmpty() const;
    std::string AutomationName() const;
    std::string ShortStatusText() const;
    MacroParamDataStatus GetErrorStatus() const;
    std::string FullAutomationPath() const;

    static json_spirit::Object MakeJsonObject(const NFmiMacroParamDataAutomationListItem& listItem);
    void ParseJsonPair(json_spirit::Pair& thePair);

    bool fEnable;
    std::string itsMacroParamDataAutomationPath; // T�h�n yritetaan saada beta-product -base-directoria vastaava suhteellinen polku jos mahdollista. T�m� talletetaan my�s json-objectiin tiedostoon
    std::string itsMacroParamDataAutomationAbsolutePath;
    std::shared_ptr<NFmiMacroParamDataInfo> itsMacroParamDataAutomation;
    MacroParamDataStatus itsStatus = MacroParamDataStatus::kFmiListItemOk;
    NFmiMetTime itsLastRunTime; // Milloin t�m� tuote on ajettu viimeksi, tai milloin t�m� tuote luotiin (= olio luotiin muistiin)
    bool fProductsHaveBeenGenerated; // Onko t�t� tuotetta oikeasti luotu t�m�n ohjelman ajon aikana
//    NFmiMetTime itsNextRunTime; // Milloin t�m� tuote pit�isi ajaa seuraavaksi
};

class NFmiPostponedMacroParamDataAutomation
{
public:
    NFmiMilliSecondTimer itsPostponeTimer;
    std::shared_ptr<NFmiMacroParamDataAutomationListItem> itsPostponedDataTriggeredAutomation;
    int itsPostponeTimeInMinutes;

    NFmiPostponedMacroParamDataAutomation(std::shared_ptr<NFmiMacroParamDataAutomationListItem>& postponedDataTriggeredAutomation, int postponeTimeInMinutes);
    bool IsPostponeTimeOver();
};

// T�m�n tyylist� listaa k�ytet��n kun k�ytt�j� antaa joukon MacroParam-data 
// ty�teht�vi� Smartmetille dialogista k�sin.
using NFmiUserWorkAutomationContainer = std::vector<std::shared_ptr<NFmiMacroParamDataAutomationListItem>>;
// Kun tehd��n thread turvallista Automaatio ty�teht�vi�, pit�� sit� 
// varten tehd� fifo toimintaa tukeva std::list rakenne.
// Lis�ksi k�ynniss� olevan data-generation operaation fifo-rakenteeseen voidaan lis�t�
// uusia t�it�, kun triggerit niit� laukaisevat, joten sen k�sittely pit�� tehd�
// thread turvallisesti eli kaikki siihen tehdyt operaatiot pit�� turvata mutexin avulla.
using NFmiAutomationWorkFifoContainer = std::list<std::shared_ptr<NFmiMacroParamDataAutomationListItem>>;

class NFmiMacroParamDataAutomationList
{
    NFmiUserWorkAutomationContainer mAutomationVector;
    std::list<NFmiPostponedMacroParamDataAutomation> itsPostponedDataTriggeredAutomations;

public:
    bool Add(const std::string& theMacroParamDataAutomationPath);
    NFmiMacroParamDataAutomationListItem& Get(size_t theZeroBasedRowIndex);
    const NFmiMacroParamDataAutomationListItem& Get(size_t theZeroBasedRowIndex) const;
    bool Remove(size_t theZeroBasedRowIndex);
    // T�t� kutsutaan kun esim. luetaan data tiedostosta ja tehd��n t�ysi tarkistus kaikille osille
    MacroParamDataStatus DoFullChecks(bool fAutomationModeOn);
    NFmiUserWorkAutomationContainer& AutomationVector() { return mAutomationVector; }
    const NFmiUserWorkAutomationContainer& AutomationVector() const { return mAutomationVector; }
    bool IsOk() const;
    bool IsEmpty() const { return mAutomationVector.empty(); }
    bool ContainsAutomationMoreThanOnce() const;
    bool HasAutomationAlready(const std::string& theFullFilePath) const;

    void RefreshAutomationList();
    NFmiUserWorkAutomationContainer GetOnDemandAutomations(int selectedAutomationIndex, bool doOnlyEnabled);
    NFmiUserWorkAutomationContainer GetDueAutomations(const NFmiMetTime& theCurrentTime, const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer);

    static json_spirit::Object MakeJsonObject(const NFmiMacroParamDataAutomationList& theMacroParamDataAutomationList);
    void ParseJsonPair(json_spirit::Pair& thePair);
    static bool StoreInJsonFormat(const NFmiMacroParamDataAutomationList& macroParamDataAutomationList, const std::string& theFilePath, std::string& theErrorStringOut);
    static bool ReadInJsonFormat(NFmiMacroParamDataAutomationList& macroParamDataAutomationList, const std::string& theFilePath, std::string& theErrorStringOut);

private:
    bool MakeListItemPathSettings(NFmiMacroParamDataAutomationListItem& theListItem);
    bool MakeListItemPathSettingsCheck(NFmiMacroParamDataAutomationListItem& theListItem, const std::string& theCheckedPath);
    bool PrepareListItemAfterJsonRead(NFmiMacroParamDataAutomationListItem& theListItem);
    void RefreshAutomationIfNeeded(std::shared_ptr<NFmiMacroParamDataAutomationListItem>& automationListItem);
    std::set<std::string> GetUniqueFilePathSet() const;
};

inline unsigned int ID_MACRO_PARAM_DATA_GENERATION_FINISHED = 23423;
inline unsigned int ID_MACRO_PARAM_DATA_GENERATION_CANCELED = 23424;
inline unsigned int ID_MACRO_PARAM_DATA_GENERATION_FAILED = 23425;

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
    // Juuri MacroParam data hakemisto, jossa on \ merkki lopussa,
    // eli hakemisto, josta MacroParam data infot ja niiden listat l�ytyv�t yleens�.
    static std::string mRootMacroParamDataDirectory;

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
    // T�ss� voi olla pilkulla eroteltu lista polkuja, koska joskus pit�� generoida data useammasta eri 
    // smarttoolista, koska niiss� tulisi muuten saman nimisten var muuttujien t�rm�yksi�.
    boost::shared_ptr<CachedRegString> mDialogDataGeneratingSmarttoolPathListString; // PAKOLLINEN
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
    // Dialogi muistaa minne/mist� on talletettu/ladattu viimeksi MacroParam data info tiedosto (hakemisto)
    boost::shared_ptr<CachedRegString> mMacroParamDataInfoSaveInitialPath;
    // Lista parametreja eri datoihin, mitk� laukaisevat datan generoinnin, jos sellaisia on
    // k�yt�ss� ja automaattinen datan generaatio on k�yt�ss�.
    // Esim. T_ec[,T_gfs_500, ...] ja my�h�stetty laukaisu T_ec[0.5h] (-> 0.5 h my�h�stys)
    boost::shared_ptr<CachedRegString> mDialogDataTriggerList; // Ei pakollinen
    // Kun dataa generoidaan, kuinka monta viimeisint� kyseist� dataa pidet��n
    // kohdehakemistossa. Jos datan luomisen j�lkeen siell� on enemm�n kyseisi� tiedostoja,
    // deletoidaan vanhimmat niist� pois kyseisell� filefilterill�.
    boost::shared_ptr<CachedRegInt> mDialogMaxGeneratedFilesKept;
    // Polku mist� viimeksi ladattu MacroParam data Automation luetaan
    boost::shared_ptr<CachedRegString> mAutomationPath;
    // Dialogi muistaa minne/mist� on talletettu/ladattu viimeksi MacroParam data automaatio lista (hakemisto)
    boost::shared_ptr<CachedRegString> mMacroParamDataAutomationListSaveInitialPath;
    // Polku mist� viimeksi ladattu MacroParam data Automation list luetaan
    boost::shared_ptr<CachedRegString> mAutomationListPath;
    // Dialogi muistaa mist� on talletettu/ladattu viimeksi smarttool st tiedoston polku (hakemisto)
    boost::shared_ptr<CachedRegString> mMacroParamDataAutomationAddSmarttoolInitialPath;
    // Jos k�ytt�j� haluaa harventaa basedatasta saatua laskentahilaa, t�ll� kertoimella 
    // se voidaan tehd�. Jos oletusarvossa eli 1, t�ll�in mit��n skaalausta ei tehd� hilankokoon.
    // Jos arvo 2, t�ll�in x- ja y-suuntaista hilakoko jaetaan 2:lla ja py�ristet��n l�himp��n kokonaislukuun.
    // Jos arvo 2,1.5, t�ll�i x koko jaetaan 2:lla ja y suunta 1.5:ll�.
    // Skaalausarvojen pit�� olla v�lill� 1-10.
    boost::shared_ptr<CachedRegString> mDialogBaseDataGridScaleString; // pakollinen
    // Kuinka paljon CPU kapasiteetista halutaan laitta MacroParam-datojen laskentoihin.
    boost::shared_ptr<CachedRegDouble> mDialogCpuUsagePercentage;
    // Dialogi muistaa mik� on viimeisin generoituvan datan talletuspolku
    boost::shared_ptr<CachedRegString> mGeneratedDataStorageInitialPath;

    // K�ytetty automaatiolista
    NFmiMacroParamDataAutomationList itsUsedMacroParamDataAutomationList;

    static const std::string itsMacroParamDataInfoFileExtension;
    static const std::string itsMacroParamDataInfoFileFilter;
    static const std::string itsMacroParamDataListFileExtension;
    static const std::string itsMacroParamDataListFileFilter;
    static const std::string itsGeneratedDataFileExtension;
    static const std::string itsGeneratedDataFileFilter;

    // Initialisoinnista raportoiva teksti
    std::string itsInitializeLogStr;
    // Mahdollinen smarttool laskuihin tai sen alustuksiin liittyvi� ongelmia laitetaan t�h�n talteen
    std::string itsSmarttoolCalculationLogStr;
    // Absoluutti polut tiedostoihin joista luettiin datan rakentamiseen k�ytetyt smarttoolit
    std::vector<std::string> mUsedAbsoluteSmarttoolPathList;
    // Onko smartmet moodissa miss� automaatiolistan datoja tuotetaan.
    boost::shared_ptr<CachedRegBool> mAutomationModeOn;
    // Onko automaatio systeemi k�ynniss� vai ei
    bool fDataGenerationIsOn = false;
    // T�m� on smartmetin k�ytt�m�n lokaali cachen perushakemisto
    std::string mLocalDataBaseDirectory;
    // T�m� on macroParamDatojen tmp tiedostojen generointi hakemisto,
    // t�nne luodaan aina uudet tiedostot ja kun ne on kirjoitettu kokonaisuudessaan
    // levylle, tehd��n file-move lopulliseen (dropbox) hakemistoon
    std::string mMacroParamDataTmpDirectory;
    // T�ll� viestit��n kuinka kauan viimeisen datan tekeminen kesti, sit� p�ivitet��n 
    // ja luetaan eri threadeista, joten se on synkronisoitava mutexilla
    std::string mLastGeneratedDataMakeTime;
    mutable std::mutex mLastGeneratedDataMakeTimeMutex;
    // Lis�ksi k�ynniss� olevan data-generation operaation fifo-rakenteeseen voidaan lis�t�
    // uusia t�it�, kun triggerit niit� laukaisevat, joten sen k�sittely pit�� tehd�
    // thread turvallisesti eli kaikki siihen tehdyt operaatiot pit�� turvata mutexin avulla.
    NFmiAutomationWorkFifoContainer mAutomationWorkFifo;
    std::mutex mAutomationWorkFifoMutex;
    // Lis�ksi jotta tiedet��n ett� pit��k� uudet trigger�idyt ty�t lis�t� jo ty�n alla 
    // olevalle threadille vaiko pit��k� k�ynnist�� uusi working-thread, on k�ynniss� olevien 
    // t�iden lukum��r� atomic rakenteessa. Eli jos sen arvo on 0, laukaistaan uusi working-thread
    // ja jos se on > 0, t�ll�in uudet ty�t vain lis�t��n fifo:on.
    std::atomic<int> mAutomationWorksLeftToProcessCounter;
    // Automaatioiden lopetukseen (Smartmetia suljettaessa) liittyvi� dataosioita
    static NFmiStopFunctor itsStopFunctor;
    static NFmiThreadCallBacks itsThreadCallBacks;
public:

    NFmiMacroParamDataGenerator();
    bool Init(const std::string& theBaseRegistryPath, const std::string& rootSmarttoolDirectory, const std::string &rootMacroParamDataDirectory, const std::string& localDataBaseDirectory);

    bool GenerateMacroParamData(NFmiThreadCallBacks* threadCallBacks);
    bool DoOnDemandBetaAutomations(int selectedAutomationIndex, bool doOnlyEnabled, NFmiThreadCallBacks* threadCallBacks);
    void DoNeededMacroParamDataAutomations(const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer);
    void StopAutomationDueClosingApplication();
    bool WaitForAutomationWorksToStop(double waitForSeconds);

    std::string DialogBaseDataParamProducerString() const;
    void DialogBaseDataParamProducerString(const std::string& newValue);
    std::string DialogDataGeneratingSmarttoolPathListString() const;
    void DialogDataGeneratingSmarttoolPathListString(const std::string& newValue);
    std::string MakeUsedAbsoluteSmarttoolPathString(const std::string& smarttoolPath) const;
    std::string MakeCleanedSmarttoolPathString(const std::string &smarttoolPath) const;
    std::string DialogUsedParameterListString() const;
    void DialogUsedParameterListString(const std::string& newValue);
    std::string DialogUsedProducerString() const;
    void DialogUsedProducerString(const std::string& newValue);
    std::string DialogDataStorageFileFilter() const;
    void DialogDataStorageFileFilter(const std::string& newValue);
    std::string DialogDataTriggerList() const;
    void DialogDataTriggerList(const std::string& newValue);
    int DialogMaxGeneratedFilesKept() const;
    void DialogMaxGeneratedFilesKept(int newValue);
    NFmiMacroParamDataAutomationList& UsedMacroParamDataAutomationList() { return itsUsedMacroParamDataAutomationList; }
    bool AutomationModeOn() const;
    void AutomationModeOn(bool newState);
    std::string AutomationPath() const;
    void AutomationPath(const std::string &newValue) const;
    std::string MacroParamDataInfoSaveInitialPath();
    void MacroParamDataInfoSaveInitialPath(const std::string& newValue);
    std::string MacroParamDataAutomationListSaveInitialPath() const;
    void MacroParamDataAutomationListSaveInitialPath(const std::string &newValue);
    std::string AutomationListPath() const;
    void AutomationListPath(const std::string &newValue);
    bool DataGenerationIsOn() const { return fDataGenerationIsOn; }
    void DataGenerationIsOn(bool newState) { fDataGenerationIsOn = newState; }
    std::string LastGeneratedDataMakeTime() const;
    void LastGeneratedDataMakeTime(std::string newValue);
    std::string MacroParamDataAutomationAddSmarttoolInitialPath() const;
    void MacroParamDataAutomationAddSmarttoolInitialPath(const std::string &newValue);
    std::string DialogBaseDataGridScaleString() const;
    void DialogBaseDataGridScaleString(const std::string &newValue);
    double DialogCpuUsagePercentage() const;
    void DialogCpuUsagePercentage(double newValue) const;
    std::string GeneratedDataStorageInitialPath() const;
    void GeneratedDataStorageInitialPath(const std::string &newValue);

    const std::string& GetInitializeLogStr() const { return itsInitializeLogStr; }
    const std::string& GetSmarttoolCalculationLogStr() const { return itsSmarttoolCalculationLogStr; }
    NFmiMacroParamDataInfo MakeDataInfo() const;

    static const std::string& RootSmarttoolDirectory() { return mRootSmarttoolDirectory; }
    static const std::string& RootMacroParamDataDirectory() { return mRootMacroParamDataDirectory; }
    static const std::string& MacroParamDataInfoFileExtension() { return itsMacroParamDataInfoFileExtension; }
    static const std::string& MacroParamDataInfoFileFilter() { return itsMacroParamDataInfoFileFilter; }
    static const std::string& MacroParamDataListFileExtension() { return itsMacroParamDataListFileExtension; }
    static const std::string& MacroParamDataListFileFilter() { return itsMacroParamDataListFileFilter; }
    static const std::string& GeneratedDataFileExtension() { return itsGeneratedDataFileExtension; }
    static const std::string& GeneratedDataFileFilter() { return itsGeneratedDataFileFilter; }

private:
    bool CalculateDataWithSmartTool(boost::shared_ptr<NFmiFastQueryInfo>& wantedMacroParamInfoPtr, NFmiInfoOrganizer* infoOrganizer, const std::vector<std::string>& smartToolContentList, NFmiThreadCallBacks* threadCallBacks);
    bool CalculateDataWithSmartTool(boost::shared_ptr<NFmiFastQueryInfo>& wantedMacroParamInfoPtr, NFmiInfoOrganizer* infoOrganizer, const std::string& smartToolText, const std::string& usedSmartToolPath, NFmiThreadCallBacks *threadCallBacks);
    std::vector<std::string> ReadSmarttoolContentsFromFiles(const std::string& filePathList);
    bool StoreMacroParamData(boost::shared_ptr<NFmiQueryData>& macroParamDataPtr, const std::string& dataStorageFileFilter, int keepMaxFiles, const std::string& fullAutomationPath, NFmiMilliSecondTimer &timer);
    bool GenerateMacroParamData(const NFmiMacroParamDataInfo &dataInfo, const std::string &fullAutomationPath, NFmiThreadCallBacks* threadCallBacks);
    bool LoadUsedAutomationList(const std::string& thePath);
    bool GenerateAutomationsData(const NFmiUserWorkAutomationContainer& automations, NFmiThreadCallBacks* threadCallBacks);
    void InitMacroParamDataTmpDirectory();
    bool EnsureTmpDirectoryExists();
    void LaunchGenerateAutomationsData(NFmiThreadCallBacks* threadCallBacks);
    bool AddTriggeredWorksToFifoAndCheckIfNewWorkerThreadMustBeLaunched(const NFmiUserWorkAutomationContainer& automationWorkList);
    std::shared_ptr<NFmiMacroParamDataAutomationListItem> PopWorkFromFifo();
    bool MarkLastAutomationWorkAsDoneAndCheckIfMoreWorkLeft(const std::string& fullAutomationPath);
    void ClearAutomationWorksLeftToProcessCounter();
};
