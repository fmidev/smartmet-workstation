
#pragma once

#include "NFmiMetTime.h"
#include "json_spirit_value.h"
#include "NFmiCachedRegistryValue.h"
#include "NFmiExtraMacroParamData.h"
#include "NFmiMilliSecondTimer.h"

#include <set>

using BaseDirectoryGetterFunctionType = std::function<std::string()>;

enum class BetaProductViewIndex
{
    MainMapView = 0,
    MapView2 = 1,
    MapView3 = 2,
    TimeSerialView = 3,
    SoundingView = 4,
    CrossSectionView = 5
};

// Luokka tiet‰‰ halutusta ajoajasta vain tunnin ja minuutin [Utc aika]. Sit‰ ei ole sidottu muuten mitenk‰‰n kalenteriin.
// Luokka osaa parsia listan ajoaikoja string:ist‰.
// Luokka k‰sittelee aikoja aina lokaali moodissa, eli annetut NFmiMetTime oliot muutetaan ennen vertailuja lokaaleiksi.
class FixedRunTime
{
    short itsHour = -1;
    short itsMinute = -1;
public:
    FixedRunTime();
    FixedRunTime(short hour, short minute);

    short Hour() const { return itsHour; }
    short Minute() const { return itsMinute; }

    bool operator==(const FixedRunTime& other) const;
    bool operator<(const NFmiMetTime& metTime) const;
    bool operator>(const NFmiMetTime& metTime) const;
    bool IsValid() const;
    NFmiMetTime MakeMetTime() const;

    // fixedRunTimeString pit‰‰ olla muotoa HH:mm esim. 4:15 tai 04:15
    // Oletus: fixedRunTimeString on jo trimmattu whitespace:n suhteen.
    static FixedRunTime ParseFixedRunTimeString(const std::string& fixedRunTimeString, std::string& possibleErrorString);
    static std::vector<FixedRunTime> ParseFixedRunTimesString(const std::string& fixedRunTimesString, std::string& possibleErrorString);
};

// NFmiBetaProduct -luokka pit‰‰ sis‰ll‰‰n yhden beta-tuotteen tiedot.
// Tuotteessa on mm. seuraavia tietoja: k‰ytetty viewMacro, aika-askel ja pituus. 
// Piirretyt v‰lilehdet (= n‰yttˆ rivit), eli tuottteessa voi olla esim. 1-50 kpl animaatio sarjoja.
// Mist‰ n‰ytˆst‰ tuote tehd‰‰n. 
// Lis‰ksi mahdollista web-site title+description teksti‰.
class NFmiBetaProduct
{
public:
    NFmiBetaProduct();
    
    bool CheckTimeRelatedInputs(const NFmiMetTime &theStartingTime, const std::string &theTimeLengthInHoursString, const std::string &theTimeStepInMinutesString, bool theUseUtcTimesInTimeBox); // K‰y l‰pi kaikki tarkastelut ja tekee virheilmoituksia
    bool CheckRowRelatedInputs(const std::string &theRowIndexListString, const std::string &theRowSubdirectoryTemplate, const std::string &theFileNameTemplate, bool useAutoFileNames, FmiDirection theParamBoxLocation); // K‰y l‰pi kaikki tarkastelut ja tekee virheilmoituksia
    bool CheckSynopStationIdListRelatedInputs(const std::string &theSynopStationIdListString); // K‰y l‰pi kaikki tarkastelut ja tekee status asetuksia
    bool InputWasGood(); // Palauttaa vain tiedon onko viimeinen tarkastelu mennyt hyvin vai ei

    const std::string& ImageStoragePath() const { return itsImageStoragePath; }
    void ImageStoragePath(const std::string &newValue) { itsImageStoragePath = newValue; }
    const std::string& FileNameTemplate() const { return itsFileNameTemplate; }
    std::string GetUsedFileNameTemplate(bool addStationIdMarker) const;
    void FileNameTemplate(const std::string &newValue) { itsFileNameTemplate = newValue; }
    bool UseAutoFileNames() const { return fUseAutoFileNames; }
    void UseAutoFileNames(bool newValue) { fUseAutoFileNames = newValue; }
    const std::string& TimeLengthInHoursString() const { return itsTimeLengthInHoursString; }
    void TimeLengthInHoursString(const std::string &newValue) { itsTimeLengthInHoursString = newValue; }
    double TimeLengthInHours() const { return itsTimeLengthInHours; } // T‰lle vain getter, koska sen arvo saadaan itsTimeLengthInHoursString:ist‰
    const std::string& TimeStepInMinutesString() const { return itsTimeStepInMinutesString; }
    void TimeStepInMinutesString(const std::string &newValue) { itsTimeStepInMinutesString = newValue; }
    int TimeStepInMinutes() const { return itsTimeStepInMinutes; } // T‰lle vain getter, koska sen arvo saadaan itsTimeStepInMinutesString:ist‰
    bool UseUtcTimesInTimeBox() const { return fUseUtcTimesInTimeBox; }
    void UseUtcTimesInTimeBox(bool newValue) { fUseUtcTimesInTimeBox = newValue; }
    FmiDirection ParamBoxLocation() const { return itsParamBoxLocation; }
    void ParamBoxLocation(FmiDirection newValue) { itsParamBoxLocation = newValue; }
    const std::string& TimeRangeInfoText() const { return itsTimeRangeInfoText; }
    const NFmiMetTime& StartingTime() const { return itsStartingTime; }
    int TimeStepCount() const { return itsTimeStepCount; }
    static const std::string& TimeLengthLabel() { return itsTimeLengthLabel; }
    static const std::string& TimeStepLabel() { return itsTimeStepLabel; }
    static void Language(FmiLanguage newValue) { itsLanguage = newValue; }

    const std::string& RowIndexListString() const { return itsRowIndexListString; }
    void RowIndexListString(const std::string &newValue) { itsRowIndexListString = newValue; }
    std::vector<int> GetUsedRowIndexies(int theOrigRowIndex) const; // Riippuu asetuksista, mink‰lainen indeksi lista palautetaan
    const std::vector<int>& GetOriginalRowIndexies() const { return itsRowIndexies; }
    const std::string& RowSubdirectoryTemplate() const { return itsRowSubdirectoryTemplate; }
    void RowSubdirectoryTemplate(const std::string &newValue) { itsRowSubdirectoryTemplate = newValue; }
    const std::string& RowIndexListInfoText() const { return itsRowIndexListInfoText; }

    bool TimeInputOk() const { return fTimeInputOk; }
    bool RowIndexInputOk() const { return fRowIndexInputOk; }
    bool SynopStationIdListInputOk() const { return fSynopStationIdListInputOk; }

    BetaProductViewIndex SelectedViewIndex() const { return itsSelectedViewIndex; }
    void SelectedViewIndex(BetaProductViewIndex newValue) { itsSelectedViewIndex = newValue; }

    const std::string& ViewMacroPath() const { return  itsViewMacroPath; }
    void ViewMacroPath(const std::string &newValue) { itsViewMacroPath = newValue; }
    bool GivenViewMacroOk() const { return  fGivenViewMacroOk; }
    void GivenViewMacroOk(bool newValue) { fGivenViewMacroOk = newValue; }
    static void RootViewMacroPath(const std::string &newValue) { itsRootViewMacroPath = newValue; }
    const std::string& WebSiteTitleString() const { return itsWebSiteTitleString; }
    void WebSiteTitleString(const std::string &newValue) { itsWebSiteTitleString = newValue; }
    const std::string& WebSiteDescriptionString() const { return itsWebSiteDescriptionString; }
    void WebSiteDescriptionString(const std::string &newValue) { itsWebSiteDescriptionString = newValue; }
    const std::string& CommandLineString() const { return itsCommandLineString; }
    void CommandLineString(const std::string &newValue) { itsCommandLineString = newValue; }

    const std::string& MakeViewMacroInfoText(const std::string &theOriginalPath);
    const std::string& ViewMacroInfoText() const { return itsViewMacroInfoText; }
    const std::string& OriginalViewMacroPath(void) const { return itsOriginalViewMacroPath; }

    bool DisplayRunTimeInfo() const { return fDisplayRunTimeInfo; }
    void DisplayRunTimeInfo(bool newValue) { fDisplayRunTimeInfo = newValue; }
    bool ShowModelOriginTime() const { return fShowModelOriginTime; }
    void ShowModelOriginTime(bool newValue) { fShowModelOriginTime = newValue; }

    const std::string& SynopStationIdListString() const { return itsSynopStationIdListString; }
    void SynopStationIdListString(const std::string &newValue) { itsSynopStationIdListString = newValue; }
    const std::vector<int>& SynopStationIdList() const { return itsSynopStationIdList; }
    bool PackImages() const { return fPackImages; }
    void PackImages(bool newValue) { fPackImages = newValue; }
    bool EnsureCurveVisibility() const { return fEnsureCurveVisibility; }
    void EnsureCurveVisibility(bool newState) { fEnsureCurveVisibility = newState; }

    static bool ContainsStringCaseInsensitive(const std::string &searchThis, const std::string &findThis);

    static json_spirit::Object MakeJsonObject(const NFmiBetaProduct &betaProduct);
    void ParseJsonPair(json_spirit::Pair &thePair);
    static bool StoreInJsonFormat(const NFmiBetaProduct &betaProduct, const std::string &theFilePath, std::string &theErrorStringOut);
    static bool ReadInJsonFormat(NFmiBetaProduct &betaProductOut, const std::string &theFilePath, std::string &theErrorStringOut);
    void InitFromJsonRead(const NFmiMetTime &theStartingTime);

private:
    std::string MakeTimeRangeInfoString();
    NFmiMetTime CalcEndTime();
    std::string MakeRowIndexInfoString();
    std::vector<int> CalcRowIndexies();
    std::string CheckUsedRowSubdirectoryTemplate();

    NFmiMetTime itsStartingTime; // t‰t‰ ei talleteta mihink‰‰n, t‰m‰ on vain aikalaskuissa mukana v‰liaikaisesti
    static FmiLanguage itsLanguage; // t‰t‰ tarvitaan v‰liaikaisesti aikoihin liittyvien tekstien tekemisess‰
    std::string itsImageStoragePath;
    std::string itsFileNameTemplate;
    bool fUseAutoFileNames; // Jos automaatio on p‰‰ll‰, tulee tiedostoista validTime_obs.png ja validTime_for.png nimisi‰ riippuen siit‰ onko kullekin ajalle havaintoja vai ei
    std::string itsTimeLengthInHoursString;
    static const std::string itsTimeLengthLabel; // T‰m‰n avulla tehd‰‰n info/virhe tekstej‰
    double itsTimeLengthInHours; // Saatu konvertoimalla itsTimeLengthInHoursString
    std::string itsTimeStepInMinutesString;
    bool fUseUtcTimesInTimeBox; // K‰ytet‰‰nkˆ kuvissa olevissa vaaleissa aikaa merkitseviss‰ laatikoissa UTC aikaa vai lokaali aikaa
    FmiDirection itsParamBoxLocation; // Mihin kohtaa karttaa parametriboxi laitetaan (disabled, bottom-left, top-center, jne.)
    static const std::string itsTimeStepLabel; // T‰m‰n avulla tehd‰‰n info/virhe tekstej‰
    int itsTimeStepInMinutes; // Saatu konvertoimalla itsTimeStepInMinutesString
    int itsTimeStepCount; // Johdettu suure, saatu time-length:in ja time-stepin avulla
    bool fTimeInputOk;
    std::string itsTimeInputErrorString; // Aikaan liittyvien input arvojen virheraportointia
    std::string itsTimeRangeInfoText; // T‰h‰n p‰‰tell‰‰n kuvien tuotannon aika-ranget ja niihin liittyv‰t mahdolliset virhetekstit

    std::string itsRowIndexListString; // T‰h‰n annetaan rivin indeksit joko pilkulla erotettuna listana (1,3,4) tai rangena (1-5)
    std::vector<int> itsRowIndexies; // T‰h‰n lasketaan indeksit itsRowIndexListString -muuttujasta
    std::string itsRowSubdirectoryTemplate; // Jos eri rivien kuvat halutaan omiin alihakemistoihin, t‰h‰n tulee alihakemiston templaatti (Row# => Row1, Row2, ...)
    bool fRowIndexInputOk;
    std::string itsRowInputErrorString; // Riveihin liittyvien input arvojen virheraportointia
    std::string itsRowIndexListInfoText; // T‰h‰n p‰‰tell‰‰n kuvien tuotannon rivi-indeksit, alihakemisto infoa ja niihin liittyv‰t mahdolliset virhetekstit

    BetaProductViewIndex itsSelectedViewIndex;
    std::string itsViewMacroPath;
    std::string itsOriginalViewMacroPath; // T‰h‰n talletetaan se polku, mink‰ k‰ytt‰j‰ on antanut dialogissa, t‰m‰ talletetaan myˆs lopulta tiedostoon
    bool fGivenViewMacroOk;
    static std::string itsRootViewMacroPath;
    std::string itsViewMacroInfoText;

    std::string itsWebSiteTitleString;
    std::string itsWebSiteDescriptionString;

    std::string itsCommandLineString; // Kun kuvat on tuotettu kohde hakemistoon, mik‰ on k‰ytt‰j‰n haluama komentorivi ajo, jolla esim. generoidaan avi-tiedosto, tai pakataan ja kopsataan kuvat jonnekin erikois paikkaan.
    // Is image generation run time information displayed in beta-product parambox or not.
    bool fDisplayRunTimeInfo;
    // Is model's origin time shown in parameter box or not.
    bool fShowModelOriginTime;
    std::string itsSynopStationIdListString; // Mist‰ kaikista synop asemista halutaan tehd‰ kuvia (k‰y vain tietyille n‰ytˆille ja moodeille)
    bool fSynopStationIdListInputOk;
    std::vector<int> itsSynopStationIdList; // T‰h‰n puretaan synop station id:t itsSynopStationIdListString -muuttujasta
    bool fPackImages = false; // Pakataanko tuotetut kuvat vai ei
    bool fEnsureCurveVisibility = false; // Aikasarja- ja luotausk‰yr‰t voidaan varmistaa n‰kyviksi s‰‰t‰m‰ll‰ asteikoita
};

// NFmiBetaProductAutomation -luokka pit‰‰ tietoa yhdest‰ automaatio tuotteesta. Se pit‰‰ tietoa mm. seuraavista asioista:
// K‰ytetty Beta-product (polku).
// Miten tuote triggerˆid‰‰n (joku kolmesta vaihtoehdosta): 
//  - fiksatut ajat (vuorokauden tunti+minuutti lista)
//  - haluttu steppi ja offset minuutti vuorokauden 00:00 ajasta
//  - data triggerˆinti: joku tietty data/datat laukaisee/laukaisevat tuotannon
// Miten animaation alkuaika k‰ytt‰ytyy (joku kolmesta vaihtoehdosta):
//  - Beta-product m‰‰r‰‰ (ajetun viewMacron alkuaika m‰‰r‰‰)
//  - sein‰kello offset, esim. -6 h eli halutaan mukaan n. 6 tuntia havaintoja
//  - v‰lilehden 1. lˆytyneen mallidatan alkuaika
// Miten animaatio sarjan pituus m‰‰r‰t‰‰n (kolme vaihtoehtoa)
//  - Beta-productissa ollut pituus m‰‰r‰‰ 
//  - sein‰kello offset, esim. +48 h eli halutaan mukaan viel‰ n. 2 vrk ennusteita
//  - v‰lilehden 1. lˆytyneen mallidatan loppuaika
class NFmiBetaProductAutomation
{
public:
    enum TriggerMode
    {
        kFmiFixedTimes = 0,
        kFmiTimeStep = 1,
        kFmiDataTrigger = 2
    };
    enum TimeMode
    {
        kFmiBetaProductTime = 0,
        kFmiWallClockOffsetTime = 1,
        kFmiFirstModelDataTime = 2
    };

    class NFmiTriggerModeInfo
    {
    public:
        NFmiTriggerModeInfo();
        void CheckTriggerModeInfo(int theTriggerMode, const std::string &theFixedRunTimesString, const std::string &theRunTimeStepInHoursString, const std::string &theFirstRunTimeOfDayString, const std::string &theTriggerDataString);
        void CheckFixedRunTimes();
        void CheckTimeStep();
        void CheckTriggerData();
        NFmiMetTime CalcNextDueTimeWithFixedTimes(const NFmiMetTime& theLastRunTime) const;
        NFmiMetTime CalcNextDueTimeWithTimeSteps(const NFmiMetTime& theLastRunTime) const;
        NFmiMetTime CalcNextDueTime(const NFmiMetTime &theLastRunTime, bool automationModeOn) const;
        static bool HasDataTriggerBeenLoaded(const std::vector<NFmiDefineWantedData>& triggerDataList, const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer, const std::string& automationName, int &postponeTriggerInMinutesOut, bool betaProductCase);
        NFmiMetTime MakeFirstRunTimeOfGivenDay(const NFmiMetTime &theTime) const;
        bool operator==(const NFmiTriggerModeInfo &other) const;
        bool operator!=(const NFmiTriggerModeInfo &other) const;

        static json_spirit::Object MakeJsonObject(const NFmiTriggerModeInfo &triggerModeInfo);
        void ParseJsonPair(json_spirit::Pair &thePair);

        TriggerMode itsTriggerMode;
        // Formaatti on muotoa: hh:mm[,hh:mm,...] eli listattuna on vuorokauden kaikki ajoajat
        std::string itsFixedRunTimesString; 
        std::vector<FixedRunTime> itsFixedRunTimes;
        std::string itsRunTimeStepInHoursString;
        double itsRunTimeStepInHours;
        std::string itsFirstRunTimeOfDayString;
        int itsFirstRunTimeOffsetInMinutes;
        // T‰ss‰ on listassa triggeri datojen lista, miss‰ haluttu data 
        // on kerrottu sen fileFilterill‰ ja ne on pilkulla eroteltuna.
        std::string itsTriggerDataString; 
        std::vector<NFmiDefineWantedData> itsTriggerDataList;
        bool fTriggerModeInfoStatus;
        std::string itsTriggerModeInfoStatusString;
    };

    class NFmiTimeModeInfo
    {
    public:
        NFmiTimeModeInfo();
        NFmiTimeModeInfo(TimeMode theTimeMode, double theWallClockOffsetInHours = 0);
        void CheckTimeModeInfo(TimeMode theTimeMode, const std::string &theWallClockOffsetInHoursString);
        bool operator==(const NFmiTimeModeInfo &other) const;
        bool operator!=(const NFmiTimeModeInfo &other) const;

        static json_spirit::Object MakeJsonObject(const NFmiTimeModeInfo &timeModeInfo);
        void ParseJsonPair(json_spirit::Pair &thePair);

        TimeMode itsTimeMode;
        std::string itsWallClockOffsetInHoursString;
        double itsWallClockOffsetInHours;
        bool fTimeModeInfoStatus;
        std::string itsTimeModeInfoStatusString;
    };

    NFmiBetaProductAutomation();

    bool InputWasGood(); // Palauttaa vain tiedon onko viimeinen tarkastelu mennyt hyvin vai ei
    void CheckBetaProductPath(const std::string &theBetaProductPath);
    const std::string& BetaProductPath() const { return itsBetaProductPath; }
    const std::string& OriginalBetaProductPath() const { return itsOriginalBetaProductPath; }
    bool BetaProductPathStatus() const { return fBetaProductPathStatus; }
    const std::string BetaProductPathStatusString() const { return  itsBetaProductPathStatusString; }

    void CheckTriggerModeInfo(int theTriggerMode, const std::string &theFixedRunTimesString, const std::string &theRunTimeStepInHoursString, const std::string &theRunTimeOffsetInHoursString, const std::string &theTriggerDataString);
    const NFmiTriggerModeInfo& TriggerModeInfo() const { return itsTriggerModeInfo; }
    bool TriggerModeInfoStatus() const;
    const std::string& TriggerModeInfoStatusString() const;

    void CheckStartTimeModeInfo(TimeMode theTimeMode, const std::string &theWallClockOffsetInHoursString);
    const NFmiTimeModeInfo& StartTimeModeInfo() const { return itsStartTimeModeInfo; }
    bool StartTimeModeInfoStatus() const;
    const std::string& StartTimeModeInfoStatusString() const;

    void CheckEndTimeModeInfo(TimeMode theTimeMode, const std::string &theWallClockOffsetInHoursString);
    const NFmiTimeModeInfo& EndTimeModeInfo() const { return itsEndTimeModeInfo; }
    bool EndTimeModeInfoStatus() const;
    const std::string& EndTimeModeInfoStatusString() const;
    void DoFullChecks(); // T‰t‰ kutsutaan kun esim. luetaan data tiedostosta ja tehd‰‰n t‰ysi tarkistus kaikille osille
    std::string MakeShortStatusErrorString();

    std::shared_ptr<NFmiBetaProduct> GetBetaProduct(bool fReloadFromFile = false); // Vain t‰t‰ saanti metodia saa k‰ytt‰‰ kun k‰ytet‰‰n itsBetaProduct -dataosaa

    static json_spirit::Object MakeJsonObject(const NFmiBetaProductAutomation &betaProductAutomation);
    void ParseJsonPair(json_spirit::Pair &thePair);
    static bool StoreInJsonFormat(const NFmiBetaProductAutomation &betaProductAutomation, const std::string &theFilePath, std::string &theErrorStringOut);
    static bool ReadInJsonFormat(NFmiBetaProductAutomation &betaProductAutomation, const std::string &theFilePath, std::string &theErrorStringOut);

    static void SetBetaProductionBaseDirectoryGetter(BaseDirectoryGetterFunctionType &getterFunction);
    static const std::string& FirstRunTimeOfDayTitle() { return itsFirstRunTimeOfDayTitle; }
    static const std::string& RunTimeStepInHoursTitle() { return itsRunTimeStepInHoursTitle; }

    bool operator==(const NFmiBetaProductAutomation &other) const;
    bool operator!=(const NFmiBetaProductAutomation &other) const;

private:
    // Time-step-hours -kontrolliin liittyv‰ vakio teksti. T‰m‰n avulla tehd‰‰n info/virhe 
    // tekstej‰ ja t‰m‰ tulee myˆs CFmiBetaAutomationDialog -dialogin teksteihin
    static const std::string itsRunTimeStepInHoursTitle;
    // First-run-of-day -kontrolliin liittyv‰ vakio teksti. T‰m‰n avulla tehd‰‰n info/virhe 
    // tekstej‰ ja t‰m‰ tulee myˆs CFmiBetaAutomationDialog -dialogin teksteihin
    static const std::string itsFirstRunTimeOfDayTitle; 
    std::string itsBetaProductPath;
    // T‰h‰n talletetaan se polku, mink‰ k‰ytt‰j‰ on antanut dialogissa, 
    // t‰m‰ talletetaan myˆs lopulta tiedostoon
    std::string itsOriginalBetaProductPath; 
    bool fBetaProductPathStatus;
    std::string itsBetaProductPathStatusString;
    // Mik‰ laukaisee t‰m‰n tuotteen tuotannon
    NFmiTriggerModeInfo itsTriggerModeInfo;
    // Miten m‰‰r‰t‰‰n t‰m‰n tuotteen alkuaika
    NFmiTimeModeInfo itsStartTimeModeInfo; 
    // Miten m‰‰r‰t‰‰n t‰m‰n tuotteen pituus
    NFmiTimeModeInfo itsEndTimeModeInfo; 
    // T‰h‰n luetaan tarvittaessa itsBetaProductPath:in osoittaman tiedoston Beta-product -olio
    std::shared_ptr<NFmiBetaProduct> itsBetaProduct; 
    // T‰h‰n talletetaan luetun Beta-productin polku, jos t‰m‰ poikkeaa itsBetaProductPath:in 
    // arvosta, pit‰‰ GetBetaProduct -metodissa lukea uusi olio uudesta tiedostosta
    std::string itsLoadedBetaProductAbsolutePath; 
    // T‰m‰ tieto lˆytyy NFmiBetaProductionSystem -luokasta. Annan siis n‰ille luokille k‰yttˆˆn 
    // kyseisen luokan metodin, jolta polku tarvittaessa pyydet‰‰n (n‰in luokien ei tarvitse tiet‰‰ toisistaan mit‰‰n)
    static BaseDirectoryGetterFunctionType itsBetaProductionBaseDirectoryGetter; 
};

class NFmiBetaProductAutomationListItem
{
public:
    enum ErrorStatus
    {
        kFmiListItemOk = 0,
        kFmiListItemReadError = 1,
        kFmiListItemAutomationError = 2,
        kFmiListItemSameAutomationsInList = 3
    };

    NFmiBetaProductAutomationListItem();
    NFmiBetaProductAutomationListItem(const std::string &theBetaAutomationPath);

    void DoFullChecks(bool automationModeOn); // T‰t‰ kutsutaan kun esim. luetaan data tiedostosta ja tehd‰‰n t‰ysi tarkistus kaikille osille
    bool IsEmpty() const;
    std::string AutomationName() const;
    std::string ShortStatusText() const;
    ErrorStatus GetErrorStatus() const;
    std::string FullAutomationPath() const;

    static json_spirit::Object MakeJsonObject(const NFmiBetaProductAutomationListItem &listItem);
    void ParseJsonPair(json_spirit::Pair &thePair);

    bool fEnable;
    std::string itsBetaProductAutomationPath; // T‰h‰n yritetaan saada beta-product -base-directoria vastaava suhteellinen polku jos mahdollista. T‰m‰ talletetaan myˆs json-objectiin tiedostoon
    std::string itsBetaProductAutomationAbsolutePath;
    std::shared_ptr<NFmiBetaProductAutomation> itsBetaProductAutomation;
    ErrorStatus itsStatus;
    NFmiMetTime itsLastRunTime; // Milloin t‰m‰ tuote on ajettu viimeksi, tai milloin t‰m‰ tuote luotiin (= olio luotiin muistiin)
    bool fProductsHaveBeenGenerated; // Onko t‰t‰ tuotetta oikeasti luotu t‰m‰n ohjelman ajon aikana
    NFmiMetTime itsNextRunTime; // Milloin t‰m‰ tuote pit‰isi ajaa seuraavaksi
};

class NFmiPostponedBetaAutomation
{
public:
    NFmiMilliSecondTimer itsPostponeTimer;
    std::shared_ptr<NFmiBetaProductAutomationListItem> itsPostponedDataTriggeredAutomation;
    int itsPostponeTimeInMinutes;

    NFmiPostponedBetaAutomation(std::shared_ptr<NFmiBetaProductAutomationListItem> &postponedDataTriggeredAutomation, int postponeTimeInMinutes);
    bool IsPostponeTimeOver();
};

class NFmiBetaProductAutomationList
{
public:

    typedef std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> AutomationContainer;

    NFmiBetaProductAutomationList();
    bool Add(const std::string &theBetaAutomationPath);
    NFmiBetaProductAutomationListItem& Get(size_t theZeroBasedRowIndex);
    const NFmiBetaProductAutomationListItem& Get(size_t theZeroBasedRowIndex) const;
    bool Remove(size_t theZeroBasedRowIndex);
    void DoFullChecks(bool fAutomationModeOn); // T‰t‰ kutsutaan kun esim. luetaan data tiedostosta ja tehd‰‰n t‰ysi tarkistus kaikille osille
    AutomationContainer& AutomationVector() { return itsAutomationVector; }
    const AutomationContainer& AutomationVector() const { return itsAutomationVector; }
    bool IsOk() const;
    bool IsEmpty() const { return itsAutomationVector.empty(); }
    bool ContainsAutomationMoreThanOnce() const;
    bool HasAutomationAlready(const std::string &theFullFilePath) const;
    std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> GetDueAutomations(const NFmiMetTime &theCurrentTime, const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer, bool automationModeOn);
    std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> GetOnDemandAutomations(int selectedAutomationIndex, bool doOnlyEnabled);

    void RefreshAutomationList();

    static json_spirit::Object MakeJsonObject(const NFmiBetaProductAutomationList &theBetaProductAutomationList);
    void ParseJsonPair(json_spirit::Pair &thePair);
    static bool StoreInJsonFormat(const NFmiBetaProductAutomationList &theBetaProductAutomationList, const std::string &theFilePath, std::string &theErrorStringOut);
    static bool ReadInJsonFormat(NFmiBetaProductAutomationList &theBetaProductAutomationList, const std::string &theFilePath, std::string &theErrorStringOut);

    static void SetBetaProductionBaseDirectoryGetter(BaseDirectoryGetterFunctionType &getterFunction);

private:
    bool MakeListItemPathSettings(NFmiBetaProductAutomationListItem &theListItem);
    bool MakeListItemPathSettingsCheck(NFmiBetaProductAutomationListItem &theListItem, const std::string &theCheckedPath);
    bool PrepareListItemAfterJsonRead(NFmiBetaProductAutomationListItem &theListItem);
    std::set<std::string> GetUniqueFilePathSet() const;
    void RefreshAutomationIfNeeded(std::shared_ptr<NFmiBetaProductAutomationListItem> &automationListItem);

    AutomationContainer itsAutomationVector;
    std::list<NFmiPostponedBetaAutomation> itsPostponedDataTriggeredAutomations;

    static BaseDirectoryGetterFunctionType itsBetaProductionBaseDirectoryGetter; // T‰m‰ tieto lˆytyy NFmiBetaProductionSystem -luokasta. Annan siis n‰ille luokille k‰yttˆˆn kyseisen luokan metodin, jolta polku tarvittaessa pyydet‰‰n (n‰in luokien ei tarvitse tiet‰‰ toisistaan mit‰‰n)
};



class NFmiBetaProductionSystem
{
public:

    NFmiBetaProductionSystem();
    bool Init(const std::string &theBaseRegistryPath, const std::string& theAbsoluteWorkingDirectory, const std::string& possibleStartingBetaAutomationListPath);
    bool DoNeededBetaAutomation(const std::vector<std::string> &loadedDataTriggerList, NFmiInfoOrganizer &infoOrganizer);
    bool DoOnDemandBetaAutomations(int selectedAutomationIndex, bool doOnlyEnabled);

    bool BetaProductGenerationRunning() const { return fBetaProductGenerationRunning; }
    void BetaProductGenerationRunning(bool newValue) { fBetaProductGenerationRunning = newValue; }
    bool LoadUsedAutomationList(const std::string &thePath);
    NFmiBetaProductAutomationList& UsedAutomationList() { return itsUsedAutomationList; }
    const NFmiBetaProductAutomationList& UsedAutomationList() const { return itsUsedAutomationList; }
    const NFmiMetTime& BetaProductRuntime() const { return itsBetaProductRuntime; }
    void BetaProductRuntime(const NFmiMetTime &newTime) { itsBetaProductRuntime = newTime; }
    static const std::string& RunTimeTitleString() { return itsRunTimeTitleString; }
    static void RunTimeTitleString(const std::string &newValue) { itsRunTimeTitleString = newValue; }
    static const std::string& RunTimeFormatString() { return itsRunTimeFormatString; }
    static void RunTimeFormatString(const std::string &newValue) { itsRunTimeFormatString = newValue; }
    static std::string AddQuotationMarksToString(std::string paddedString);

    static void SetGenerateBetaProductsCallback(std::function<void(std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> &, const NFmiMetTime &)> theCallback) { itsGenerateBetaProductsCallback = theCallback; }

    static const std::string& BetaProductFileExtension() { return itsBetaProductFileExtension; }
    static const std::string& BetaProductFileFilter() { return itsBetaProductFileFilter; }
    static const std::string& BetaAutomationFileExtension() { return itsBetaAutomationFileExtension; }
    static const std::string& BetaAutomationFileFilter() { return itsBetaAutomationFileFilter; }
    static const std::string& BetaAutomationListFileExtension() { return itsBetaAutomationListFileExtension; }
    static const std::string& BetaAutomationListFileFilter() { return itsBetaAutomationListFileFilter; }
    static const std::string& BetaAutomationAutoFileNameTypeStamp() { return itsBetaAutomationAutoFileNameTypeStamp; }
    static const std::string& FileNameTemplateValidTimeStamp() { return itsFileNameTemplateValidTimeStamp; }
    static const std::string& FileNameTemplateStationIdStamp() { return itsFileNameTemplateStationIdStamp; }
    static const std::string& FileNameTemplateOrigTimeStamp() { return itsFileNameTemplateOrigTimeStamp; }
    static const std::string& FileNameTemplateMakeTimeStamp() { return itsFileNameTemplateMakeTimeStamp; }

    int BetaProductTabControlIndex();
    void BetaProductTabControlIndex(int newValue);

    // T‰ss‰ joukko metodeja, joilla dialogit muistavat talletus/lataus hakemistonsa eri olioille.
    // std::function -otuksilla annetaan tietyt get/set funktiot yleisille templaatti funktoille, jotka toimivat kaikille Beta-otuksille.
    std::string BetaProductSaveInitialPath();
    void BetaProductSaveInitialPath(const std::string &newValue);
    std::string BetaAutomationSaveInitialPath();
    void BetaAutomationSaveInitialPath(const std::string& newValue);
    std::string BetaAutomationListSaveInitialPath();
    void BetaAutomationListSaveInitialPath(const std::string& newValue);

    // Laitoin t‰lle BetaProductionBaseDirectory get-funktioille Get -etuliitteen, jotta k‰‰nt‰j‰ erottaa 
    // sen, kun teen NFmiBetaProductAutomation -luokalle t‰h‰n liittyv‰‰ getter-funktion:ia.
    std::string GetBetaProductionBaseDirectory() const;
    std::string UsedAutomationListPathString();
    void UsedAutomationListPathString(const std::string &newValue);

    // Beta Product dialog tab control settings
    int BetaProductTimeStepInMinutes();
    void BetaProductTimeStepInMinutes(int newValue);
    double BetaProductTimeLengthInHours();
    void BetaProductTimeLengthInHours(double newValue);
    bool BetaProductUseUtcTimesInTimeBox();
    void BetaProductUseUtcTimesInTimeBox(bool newValue);
    FmiDirection BetaProductParamBoxLocation();
    void BetaProductParamBoxLocation(FmiDirection newValue);
    std::string BetaProductStoragePath();
    void BetaProductStoragePath(const std::string &newValue);
    std::string BetaProductFileNameTemplate();
    void BetaProductFileNameTemplate(const std::string &newValue);
    bool BetaProductUseAutoFileNames();
    void BetaProductUseAutoFileNames(bool newValue);
    std::string BetaProductRowIndexListString();
    void BetaProductRowIndexListString(const std::string &newValue);
    std::string BetaProductRowSubDirectoryTemplate();
    void BetaProductRowSubDirectoryTemplate(const std::string &newValue);
    int BetaProductSelectedViewIndex();
    void BetaProductSelectedViewIndex(int newValue);
    std::string BetaProductViewMacroPath();
    void BetaProductViewMacroPath(const std::string &newValue);
    std::string BetaProductWebSiteTitle();
    void BetaProductWebSiteTitle(const std::string &newValue);
    std::string BetaProductWebSiteDescription();
    void BetaProductWebSiteDescription(const std::string &newValue);
    std::string BetaProductCommandLine();
    void BetaProductCommandLine(const std::string &newValue);
    bool BetaProductDisplayRuntime();
    void BetaProductDisplayRuntime(bool newValue);
    bool BetaProductShowModelOriginTime();
    void BetaProductShowModelOriginTime(bool newValue);
    std::string BetaProductSynopStationIdListString();
    void BetaProductSynopStationIdListString(const std::string &newValue);
    bool BetaProductPackImages();
    void BetaProductPackImages(bool newValue);
    bool BetaProductEnsureCurveVisibility();
    void BetaProductEnsureCurveVisibility(bool newValue);

    // Beta Product Automation dialog tab control settings
    bool AutomationModeOn();
    void AutomationModeOn(bool newValue);
    std::string BetaProductPath();
    void BetaProductPath(const std::string &newValue);
    int TriggerModeIndex();
    void TriggerModeIndex(int newValue);
    std::string FixedTimesString();
    void FixedTimesString(const std::string &newValue);
    std::string AutomationTimeStepInHoursString();
    void AutomationTimeStepInHoursString(const std::string &newValue);
    std::string FirstRunTimeOfDayString();
    void FirstRunTimeOfDayString(const std::string &newValue);
    int StartTimeModeIndex();
    void StartTimeModeIndex(int newValue);
    int EndTimeModeIndex();
    void EndTimeModeIndex(int newValue);
    std::string StartTimeClockOffsetInHoursString();
    void StartTimeClockOffsetInHoursString(const std::string &newValue);
    std::string EndTimeClockOffsetInHoursString();
    void EndTimeClockOffsetInHoursString(const std::string &newValue);
    std::string AutomationPath();
    void AutomationPath(const std::string &newValue);
    std::string TriggerDataString();
    void TriggerDataString(const std::string& newValue);

    const std::string& ImagePackingExePath() const { return itsImagePackingExePath; }
    const std::string& ImagePackingExeCommandLine() const { return itsImagePackingExeCommandLine; }

private:

    bool InitImagePackingExe(const std::string& theAbsoluteWorkingDirectory);
    void SetupFinalBataAutomationListPath(const std::string& possibleStartingBetaAutomationListPath);

    bool fBetaProductGenerationRunning; // Onko SmartMet juuri tekem‰ss‰ kuvia Beta product systeemill‰ (vaikuttaa mm. joihinkin piirtoihin)
    NFmiMetTime itsBetaProductRuntime; // Mill‰ hetkell‰ on kuvatuotantoa alettu tekem‰‰n (k‰ytet‰‰n jos display runtime info k‰ytˆss‰)
    static std::string itsRunTimeTitleString;
    static std::string itsRunTimeFormatString;
    NFmiBetaProductAutomationList itsUsedAutomationList;
    static std::function<void(std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> &, const NFmiMetTime &)> itsGenerateBetaProductsCallback;


    static const std::string itsBetaProductFileExtension;
    static const std::string itsBetaProductFileFilter;
    static const std::string itsBetaAutomationFileExtension;
    static const std::string itsBetaAutomationFileFilter;
    static const std::string itsBetaAutomationListFileExtension;
    static const std::string itsBetaAutomationListFileFilter;
    static const std::string itsBetaAutomationAutoFileNameTypeStamp;
    static const std::string itsFileNameTemplateValidTimeStamp;
    static const std::string itsFileNameTemplateStationIdStamp;
    static const std::string itsFileNameTemplateOrigTimeStamp;
    static const std::string itsFileNameTemplateMakeTimeStamp;

    // Perushakemisto, jonne talletetaan Beta-produt:eja ja automaatioita ja automaatiolistoja. T‰m‰ voi olla jaetulla verkkolevyll‰.
    std::string itsBetaProductionBaseDirectory;
    // Kuvien pakkaukseen k‰ytetyn ohjelman koko polku
    std::string itsImagePackingExePath;
    // Kuvien pakkaukseen k‰ytetyn ohjelman peruskomentorivi
    std::string itsImagePackingExeCommandLine;

    // General Beta Product dialog options
    std::string mBaseRegistryPath; // Perus smartmet polku Windows rekistereiss‰ (t‰h‰n tulee SmartMetin konfiguraatio kohtainen polku)
    boost::shared_ptr<CachedRegString> mBetaProductSaveInitialPath; // Beta product dialogi muistaa minne/mist‰ on talletettu/ladattu viimeksi Beta-product tiedosto
    boost::shared_ptr<CachedRegString> mBetaAutomationSaveInitialPath; // Beta product dialogi muistaa minne/mist‰ on talletettu/ladattu viimeksi Beta-automaatio tiedosto
    boost::shared_ptr<CachedRegString> mBetaAutomationListSaveInitialPath; // Beta product dialogi muistaa minne/mist‰ on talletettu/ladattu viimeksi Beta-automaatiolist tiedosto
    boost::shared_ptr<CachedRegInt> mBetaProductTabControlIndex; // Mik‰ Beta-product dialogin tabi on aktiivinen
    boost::shared_ptr<CachedRegString> mUsedAutomationListPathString; // Polku smartMetin k‰ytt‰m‰‰n automaatiolistaan, joka ladataan k‰ynnistyess‰
    // Beta Product dialog tab control settings
    boost::shared_ptr<CachedRegInt> mBetaProductTimeStepInMinutes; // K‰ytetty aika-steppi minuuteissa
    boost::shared_ptr<CachedRegDouble> mBetaProductTimeLengthInHours; // K‰ytetty aika-pituus tunneissa
    boost::shared_ptr<CachedRegBool> mBetaProductUseUtcTimesInTimeBox; // K‰ytet‰‰nkˆ kuvissa olevissa vaaleissa aikaa merkitseviss‰ laatikoissa UTC aikaa vai lokaali aikaa
    boost::shared_ptr<CachedRegInt> mBetaProductParamBoxLocation; // Mihin kohtaa kartan paramboxi laitetaan (disabled, bottom-left, top-center, jne.)
    boost::shared_ptr<CachedRegString> mBetaProductStoragePath; // Polku minne kuvat talletetaan
    boost::shared_ptr<CachedRegString> mBetaProductFileNameTemplate; // Talletettavien kuvien tiedosto nimien sapluuna
    boost::shared_ptr<CachedRegBool> mBetaProductUseAutoFileNames; // Nimet‰‰nkˆ tiedosto nimet automaattisesti tyyliin validtime_obs.png ja validTime_for.png, vai annetaanko k‰ytt‰j‰n antaa tiedostonimi templaatti
    boost::shared_ptr<CachedRegString> mBetaProductRowIndexListString; // Tallennuksessa mahdollisesti k‰ytetyt rivien indeksit stringin‰
    boost::shared_ptr<CachedRegString> mBetaProductRowSubDirectoryTemplate; // Tallennuksessa mahdollisesti k‰ytettyjen rivien alihakemisto templaatti stringin‰
    boost::shared_ptr<CachedRegInt> mBetaProductSelectedViewIndex; // Valitun n‰ytˆn indeksi (0=main-map-view, 1 = map-view-2, 2 = map-view-3, 3 = time-serial-view, 4 = sounding-view, 5 = cross-section-view)
    boost::shared_ptr<CachedRegString> mBetaProductViewMacroPath; // Mahdollisesti k‰ytetyn ViewMacron polku
    boost::shared_ptr<CachedRegString> mBetaProductWebSiteTitle; // Beta product web-site Title (otsikko, iso teksti), talletetaan erilliseen tekstitiedostoon kohde hakemistoissa
    boost::shared_ptr<CachedRegString> mBetaProductWebSiteDescription; // Beta product web-site Description (tarkempi kuvaus, pienempi teksti), talletetaan erilliseen tekstitiedostoon kohde hakemistoissa
    boost::shared_ptr<CachedRegString> mBetaProductCommandLine; // Mink‰laisen komentorivi komennnon k‰ytt‰j‰ haluaa ajaa jokaiselle tuottamalleen v‰lilehti tuotteelle
    boost::shared_ptr<CachedRegBool> mBetaProductDisplayRuntime;
    boost::shared_ptr<CachedRegBool> mBetaProductShowModelOriginTime;
    boost::shared_ptr<CachedRegString> mBetaProductSynopStationIdListString; // Mist‰ kaikista synop asemista halutaan tehd‰ kuvia (k‰y vain tietyille n‰ytˆille ja moodeille)
    boost::shared_ptr<CachedRegBool> mBetaProductPackImages; // Pakataanko tuotetut kuvat vai ei
    boost::shared_ptr<CachedRegBool> mBetaProductEnsureCurveVisibility; // Varmistetaanko ett‰ aikasarja- ja luotausk‰yr‰t tulevat n‰kyviin
    // Beta Product Automation dialog tab control settings
    boost::shared_ptr<CachedRegBool> mAutomationModeOn; // Onko SmartMet ns. Beta tuotanto automaatio moodissa, jolloin kuvia tuotetaan s‰‰dˆsten mukaan haluttuina aikoina.
    boost::shared_ptr<CachedRegString> mBetaProductPath; // Polku mist‰ Beta-product-automation lataa Beta-product tuotteensa
    boost::shared_ptr<CachedRegInt> mTriggerModeIndex; // fixed-times, time-step tai data-trigger -moodin indeksi
    boost::shared_ptr<CachedRegString> mFixedTimesString; // HH:mm[,HH:mm,...]
    boost::shared_ptr<CachedRegString> mAutomationTimeStepInHoursString; 
    boost::shared_ptr<CachedRegString> mFirstRunTimeOfDayString;
    boost::shared_ptr<CachedRegInt> mStartTimeModeIndex;
    boost::shared_ptr<CachedRegInt> mEndTimeModeIndex;
    boost::shared_ptr<CachedRegString> mStartTimeClockOffsetInHoursString;
    boost::shared_ptr<CachedRegString> mEndTimeClockOffsetInHoursString;
    boost::shared_ptr<CachedRegString> mAutomationPath; // Polku mist‰ viimeksi ladattu Automation luetaan
    boost::shared_ptr<CachedRegString> mTriggerDataString; // T_ec[,par10_prod240_500,...]

};
