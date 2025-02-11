#pragma once
#include "NFmiCachedRegistryValue.h"
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

// Tämä on yksi erillinen MacroParam dataan liittyvien datojen kokoelma.
// Eli systeemissä voi olla useita erilaisia MacroParam datojen reseptejä,
// ja nee laitetaan tälläisiin otuksiin.
// Inputit tarkistetään tämän avulla.
class NFmiMacroParamDataInfo
{
    // Pohjadata valitaan antamalla parametri+tuottaja tieto (ei level tietoa ainakaan toistaiseksi)
    // Esim1: T_ec eli Ecmwf:n pinta data, missä lämpötilaparametri
    // Esim2: par4_prod240 eli sama Ec-pinta data, mutta annettuna par+prod id:en avulla
    std::string mBaseDataParamProducerString; // PAKOLLINEN
    // Talletettavan datan tuottajan id,name
    // Esim: "1278,ProducerName"
    std::string mUsedProducerString; // PAKOLLINEN
    // Polku (suhteellinen/absoluuttinen) siihen smarttool tiedostoon, mitä käytetään 
    // generoimaan lopullinen data ja sen kaikki parametrit.
    // Tähän pyritään saamaan aina suhteellinen polku, jos se osoittaa smarttool kansioon Dropboxissa.
    // Tässä voi olla pilkulla eroteltu lista polkuja, koska joskus pitää generoida data useammasta eri 
    // smarttoolista, koska niissä tulisi muuten saman nimisten var muuttujien törmäyksiä.
    std::string mDataGeneratingSmarttoolPathListString; // PAKOLLINEN
    // Pohjadatasta otetaan muut dimensiot (hplace/vplace/time), mutta käyttäjä määrää
    // mitkä parametrit dataan generoidaan. Tässä on pilkuilla eroteltuna id1,name1,id2,name2,...
    // Esim. 2501,MyParam1,2502,MyParam2,2503,MyParam3,2504,MyParam4
    std::string mUsedParameterListString; // PAKOLLINEN
    // Mihin hakemistoon ja millä nimellä data talletetaan, kun se generoidaan.
    // Tämä voi olla myös tyhjä, jolloin data otetaan vain lokaali käyttöön Smartmetin sisäiseen muistiin.
    // Esim. D:\data\*_mydata.sqd
    // Tässä * kohtaan laitetaan lokaaliajan seinäkelloaika sekunteja myöten:
    // Esim. D:\data\20240912183423_mydata.sqd
    std::string mDataStorageFileFilter; // PAKOLLINEN
    // Lista parametreja eri datoihin, mitkä laukaisevat datan generoinnin, jos sellaisia on
    // käytössä ja automaattinen datan generaatio on käytössä.
    // Esim. T_ec[,T_gfs_500, ...] ja myöhästetty laukaisu T_ec[0.5h] (-> 0.5 h myöhästys)
    std::string mDataTriggerList; // Ei pakollinen
    std::vector<NFmiDefineWantedData> mWantedDataTriggerList;
    // Kun dataa generoidaan, kuinka monta viimeisintä kyseistä dataa pidetään
    // kohdehakemistossa. Jos datan luomisen jälkeen siellä on enemmän kyseisiä tiedostoja,
    // deletoidaan vanhimmat niistä pois kyseisellä filefilterillä.
    int mMaxGeneratedFilesKept = 2;
    // Jos käyttäjä haluaa harventaa basedatasta saatua laskentahilaa, tällä kertoimella 
    // se voidaan tehdä. Jos oletusarvossa eli 1, tällöin mitään skaalausta ei tehdä hilankokoon.
    // Jos arvo 2, tällöin x- ja y-suuntaista hilakoko jaetaan 2:lla ja pyöristetään lähimpään kokonaislukuun.
    // Jos arvo 2,1.5, tällöi x koko jaetaan 2:lla ja y suunta 1.5:llä.
    // Skaalausarvojen pitää olla välillä 1-10.
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

    void DoFullChecks(bool automationModeOn); // Tätä kutsutaan kun esim. luetaan data tiedostosta ja tehdään täysi tarkistus kaikille osille
    bool IsEmpty() const;
    std::string AutomationName() const;
    std::string ShortStatusText() const;
    MacroParamDataStatus GetErrorStatus() const;
    std::string FullAutomationPath() const;

    static json_spirit::Object MakeJsonObject(const NFmiMacroParamDataAutomationListItem& listItem);
    void ParseJsonPair(json_spirit::Pair& thePair);

    bool fEnable;
    std::string itsMacroParamDataAutomationPath; // Tähän yritetaan saada beta-product -base-directoria vastaava suhteellinen polku jos mahdollista. Tämä talletetaan myös json-objectiin tiedostoon
    std::string itsMacroParamDataAutomationAbsolutePath;
    std::shared_ptr<NFmiMacroParamDataInfo> itsMacroParamDataAutomation;
    MacroParamDataStatus itsStatus = MacroParamDataStatus::kFmiListItemOk;
    NFmiMetTime itsLastRunTime; // Milloin tämä tuote on ajettu viimeksi, tai milloin tämä tuote luotiin (= olio luotiin muistiin)
    bool fProductsHaveBeenGenerated; // Onko tätä tuotetta oikeasti luotu tämän ohjelman ajon aikana
//    NFmiMetTime itsNextRunTime; // Milloin tämä tuote pitäisi ajaa seuraavaksi
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

// Tämän tyylistä listaa käytetään kun käyttäjä antaa joukon MacroParam-data 
// työtehtäviä Smartmetille dialogista käsin.
using NFmiUserWorkAutomationContainer = std::vector<std::shared_ptr<NFmiMacroParamDataAutomationListItem>>;
// Kun tehdään thread turvallista Automaatio työtehtäviä, pitää sitä 
// varten tehdä fifo toimintaa tukeva std::list rakenne.
// Lisäksi käynnissä olevan data-generation operaation fifo-rakenteeseen voidaan lisätä
// uusia töitä, kun triggerit niitä laukaisevat, joten sen käsittely pitää tehdä
// thread turvallisesti eli kaikki siihen tehdyt operaatiot pitää turvata mutexin avulla.
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
    // Tätä kutsutaan kun esim. luetaan data tiedostosta ja tehdään täysi tarkistus kaikille osille
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

// Tällä talletetaan paljon MacroParamDataGenerator dialogin juttuja 
// Windows rekisteriin pysyvään muistiin jossa käytetään
// boost::shared_ptr<CachedRegString> datarakennetta.
// Luokan tarkoitus on luoda uusia MacroParam datoja halutuilla skripteillä ja asetuksilla.
class NFmiMacroParamDataGenerator
{
    // General MacroParam Data Generator dialog options
    // Perus smartmet polku Windows rekistereissä (tähän tulee SmartMetin konfiguraatio kohtainen polku)
    std::string mBaseRegistryPath; 
    // Juuri smarttool hakemisto, jossa on \ merkki lopussa
    static std::string mRootSmarttoolDirectory;
    // Juuri MacroParam data hakemisto, jossa on \ merkki lopussa,
    // eli hakemisto, josta MacroParam data infot ja niiden listat löytyvät yleensä.
    static std::string mRootMacroParamDataDirectory;

    // HUOM! mDialog -alkuiset data memberit pitävät sisällään vastaavan dialogin kenttien sisällöt.

    // Pohjadata valitaan antamalla parametri+tuottaja tieto (ei level tietoa ainakaan toistaiseksi)
    // Esim1: T_ec eli Ecmwf:n pinta data, missä lämpötilaparametri
    // Esim2: par4_prod240 eli sama Ec-pinta data, mutta annettuna par+prod id:en avulla
    boost::shared_ptr<CachedRegString> mDialogBaseDataParamProducerString; // PAKOLLINEN
    // Talletettavan datan tuottajan id,name
    // Esim: "1278,My Producer Name"
    boost::shared_ptr<CachedRegString> mDialogUsedProducerString; // PAKOLLINEN
    // Polku (suhteellinen/absoluuttinen) siihen smarttool tiedostoon, mitä käytetään 
    // generoimaan lopullinen data ja sen kaikki parametrit.
    // Tähän pyritään saamaan aina suhteellinen polku, jos se osoittaa smarttool kansioon Dropboxissa.
    // Tässä voi olla pilkulla eroteltu lista polkuja, koska joskus pitää generoida data useammasta eri 
    // smarttoolista, koska niissä tulisi muuten saman nimisten var muuttujien törmäyksiä.
    boost::shared_ptr<CachedRegString> mDialogDataGeneratingSmarttoolPathListString; // PAKOLLINEN
    // Pohjadatasta otetaan muut dimensiot (hplace/vplace/time), mutta käyttäjä määrää
    // mitkä parametrit dataan generoidaan. Tässä on pilkuilla eroteltuna id1,name1,id2,name2,...
    // Esim. 2501,MyParam1,2502,MyParam2,2503,MyParam3,2504,MyParam4
    boost::shared_ptr<CachedRegString> mDialogUsedParameterListString; // PAKOLLINEN
    // Mihin hakemistoon ja millä nimellä data talletetaan, kun se generoidaan.
    // Tämä voi olla myös tyhjä, jolloin data otetaan vain lokaali käyttöön Smartmetin sisäiseen muistiin.
    // Esim. D:\data\*_mydata.sqd
    // Tässä * kohtaan laitetaan lokaaliajan seinäkelloaika sekunteja myöten:
    // Esim. D:\data\20240912183423_mydata.sqd
    boost::shared_ptr<CachedRegString> mDialogDataStorageFileFilter; // PAKOLLINEN
    // Dialogi muistaa minne/mistä on talletettu/ladattu viimeksi MacroParam data info tiedosto (hakemisto)
    boost::shared_ptr<CachedRegString> mMacroParamDataInfoSaveInitialPath;
    // Lista parametreja eri datoihin, mitkä laukaisevat datan generoinnin, jos sellaisia on
    // käytössä ja automaattinen datan generaatio on käytössä.
    // Esim. T_ec[,T_gfs_500, ...] ja myöhästetty laukaisu T_ec[0.5h] (-> 0.5 h myöhästys)
    boost::shared_ptr<CachedRegString> mDialogDataTriggerList; // Ei pakollinen
    // Kun dataa generoidaan, kuinka monta viimeisintä kyseistä dataa pidetään
    // kohdehakemistossa. Jos datan luomisen jälkeen siellä on enemmän kyseisiä tiedostoja,
    // deletoidaan vanhimmat niistä pois kyseisellä filefilterillä.
    boost::shared_ptr<CachedRegInt> mDialogMaxGeneratedFilesKept;
    // Polku mistä viimeksi ladattu MacroParam data Automation luetaan
    boost::shared_ptr<CachedRegString> mAutomationPath;
    // Dialogi muistaa minne/mistä on talletettu/ladattu viimeksi MacroParam data automaatio lista (hakemisto)
    boost::shared_ptr<CachedRegString> mMacroParamDataAutomationListSaveInitialPath;
    // Polku mistä viimeksi ladattu MacroParam data Automation list luetaan
    boost::shared_ptr<CachedRegString> mAutomationListPath;
    // Dialogi muistaa mistä on talletettu/ladattu viimeksi smarttool st tiedoston polku (hakemisto)
    boost::shared_ptr<CachedRegString> mMacroParamDataAutomationAddSmarttoolInitialPath;
    // Jos käyttäjä haluaa harventaa basedatasta saatua laskentahilaa, tällä kertoimella 
    // se voidaan tehdä. Jos oletusarvossa eli 1, tällöin mitään skaalausta ei tehdä hilankokoon.
    // Jos arvo 2, tällöin x- ja y-suuntaista hilakoko jaetaan 2:lla ja pyöristetään lähimpään kokonaislukuun.
    // Jos arvo 2,1.5, tällöi x koko jaetaan 2:lla ja y suunta 1.5:llä.
    // Skaalausarvojen pitää olla välillä 1-10.
    boost::shared_ptr<CachedRegString> mDialogBaseDataGridScaleString; // pakollinen
    // Kuinka paljon CPU kapasiteetista halutaan laitta MacroParam-datojen laskentoihin.
    boost::shared_ptr<CachedRegDouble> mDialogCpuUsagePercentage;
    // Dialogi muistaa mikä on viimeisin generoituvan datan talletuspolku
    boost::shared_ptr<CachedRegString> mGeneratedDataStorageInitialPath;

    // Käytetty automaatiolista
    NFmiMacroParamDataAutomationList itsUsedMacroParamDataAutomationList;

    static const std::string itsMacroParamDataInfoFileExtension;
    static const std::string itsMacroParamDataInfoFileFilter;
    static const std::string itsMacroParamDataListFileExtension;
    static const std::string itsMacroParamDataListFileFilter;
    static const std::string itsGeneratedDataFileExtension;
    static const std::string itsGeneratedDataFileFilter;

    // Initialisoinnista raportoiva teksti
    std::string itsInitializeLogStr;
    // Mahdollinen smarttool laskuihin tai sen alustuksiin liittyviä ongelmia laitetaan tähän talteen
    std::string itsSmarttoolCalculationLogStr;
    // Absoluutti polut tiedostoihin joista luettiin datan rakentamiseen käytetyt smarttoolit
    std::vector<std::string> mUsedAbsoluteSmarttoolPathList;
    // Onko smartmet moodissa missä automaatiolistan datoja tuotetaan.
    boost::shared_ptr<CachedRegBool> mAutomationModeOn;
    // Onko automaatio systeemi käynnissä vai ei
    bool fDataGenerationIsOn = false;
    // Tämä on smartmetin käyttämän lokaali cachen perushakemisto
    std::string mLocalDataBaseDirectory;
    // Tämä on macroParamDatojen tmp tiedostojen generointi hakemisto,
    // tänne luodaan aina uudet tiedostot ja kun ne on kirjoitettu kokonaisuudessaan
    // levylle, tehdään file-move lopulliseen (dropbox) hakemistoon
    std::string mMacroParamDataTmpDirectory;
    // Tällä viestitään kuinka kauan viimeisen datan tekeminen kesti, sitä päivitetään 
    // ja luetaan eri threadeista, joten se on synkronisoitava mutexilla
    std::string mLastGeneratedDataMakeTime;
    mutable std::mutex mLastGeneratedDataMakeTimeMutex;
    // Lisäksi käynnissä olevan data-generation operaation fifo-rakenteeseen voidaan lisätä
    // uusia töitä, kun triggerit niitä laukaisevat, joten sen käsittely pitää tehdä
    // thread turvallisesti eli kaikki siihen tehdyt operaatiot pitää turvata mutexin avulla.
    NFmiAutomationWorkFifoContainer mAutomationWorkFifo;
    std::mutex mAutomationWorkFifoMutex;
    // Lisäksi jotta tiedetään että pitääkö uudet triggeröidyt työt lisätä jo työn alla 
    // olevalle threadille vaiko pitääkö käynnistää uusi working-thread, on käynnissä olevien 
    // töiden lukumäärä atomic rakenteessa. Eli jos sen arvo on 0, laukaistaan uusi working-thread
    // ja jos se on > 0, tällöin uudet työt vain lisätään fifo:on.
    std::atomic<int> mAutomationWorksLeftToProcessCounter;
    // Automaatioiden lopetukseen (Smartmetia suljettaessa) liittyviä dataosioita
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
