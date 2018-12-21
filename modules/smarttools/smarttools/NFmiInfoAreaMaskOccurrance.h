#pragma once

#include <newbase/NFmiInfoAreaMask.h>

class NFmiDrawParam;

// Näitä kolmea areamaks luokkaa yhdistää se että ne kaikki voivat käsitellä havaintoja (nearest tekniikalla) ja 
// havaintodatat voivat tulla multi-data-sourcesta (esim. synop voi koostua jopa viidesta eri datasta suomi/euro/maailma/ship/poiju).

class _FMI_DLL NFmiInfoAreaMaskOccurrance : public NFmiInfoAreaMaskProbFunc
{
 public:
  ~NFmiInfoAreaMaskOccurrance(void);
  NFmiInfoAreaMaskOccurrance(const NFmiCalculationCondition &theOperation,
                             Type theMaskType,
                             NFmiInfoData::Type theDataType,
                             const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                             NFmiAreaMask::FunctionType thePrimaryFunc,
                             NFmiAreaMask::FunctionType theSecondaryFunc,
                             int theArgumentCount,
                             const boost::shared_ptr<NFmiArea> &theCalculationArea,
                             bool synopXCase,
                             unsigned long thePossibleMetaParamId);
  NFmiInfoAreaMaskOccurrance(const NFmiInfoAreaMaskOccurrance &theOther);
  void Initialize(void) override;  // Tätä kutsutaan konstruktorin jälkeen, tässä alustetaan tietyille
                          // datoille mm. käytetyt aikaindeksit ja käytetyt locaaion indeksit
  NFmiAreaMask *Clone(void) const override;
  NFmiInfoAreaMaskProbFunc &operator=(const NFmiInfoAreaMaskProbFunc &theMask) = delete;

  static void SetMultiSourceDataGetterCallback(
      const std::function<void(checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &,
                               boost::shared_ptr<NFmiDrawParam> &,
                               const boost::shared_ptr<NFmiArea> &)> &theCallbackFunction);
  static std::function<void(checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &,
      boost::shared_ptr<NFmiDrawParam> &,
      const boost::shared_ptr<NFmiArea> &)> GetMultiSourceDataGetterCallback() { return itsMultiSourceDataGetter; }
  // Nyt ainakin synop ja salama datat ovat tälläisiä
  static bool IsKnownMultiSourceData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  static checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> GetMultiSourceData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiArea> &calculationArea, bool getSynopXData);
  static checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> CreateShallowCopyOfInfoVector(const checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &infoVector);

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways) override;

 protected:
  bool IsGridData() const;
  void DoCalculations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                      const NFmiCalculationParams &theCalculationParams,
                      const NFmiLocation &theLocation,
                      const std::vector<unsigned long> &theLocationIndexCache,
                      int &theOccurranceCountInOut);
  virtual void DoCalculateCurrentLocation(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                  const NFmiLocation &theLocation,
                                  bool theIsStationLocationsStoredInData,
                                  int &theOccurranceCountInOut);
  void InitializeLocationIndexCaches();
  std::vector<unsigned long> CalcLocationIndexCache(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);

  // halutaanko vain normaali asemat (true), ei liikkuvia asemia (laivat, poijut)
  bool fSynopXCase;
  bool fUseMultiSourceData;
  // Joitain laskuja optimoidaan ja niillä lähdedatasta laskut rajataan laskettavan kartta-alueen sisälle
  boost::shared_ptr<NFmiArea> itsCalculationArea;  
  // Tähän laitetaan laskuissa käytettävät datat, joko se joko on jo emoluokassa
  // oleva itsInfo, tai multisource tapauksissa joukko datoja  
  checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> itsInfoVector;  

  // Jokaiselle käytössä olevalle datalle lasketaan locationIndex cache, eli kaikki ne pisteet
  // kustakin datasta,
  // joita käytetään laskuissa. Jos jollekin datalle on tyhjä vektori, lasketaan siitä kaikki. Jos
  // jostain datasta
  // ei käytetä yhtään pistettä, on siihen kuuluvassa vektorissa vain yksi luku (gMissingIndex).
  // Tämä alustetaan Initialize -metodissa.
  std::vector<std::vector<unsigned long>> itsCalculatedLocationIndexies;

  static std::function<void(checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &,
                            boost::shared_ptr<NFmiDrawParam> &,
                            const boost::shared_ptr<NFmiArea> &)> itsMultiSourceDataGetter;

};

class _FMI_DLL NFmiInfoAreaMaskOccurranceSimpleCondition : public NFmiInfoAreaMaskOccurrance
{
public:
    ~NFmiInfoAreaMaskOccurranceSimpleCondition(void);
    NFmiInfoAreaMaskOccurranceSimpleCondition(const NFmiCalculationCondition &theOperation,
        Type theMaskType,
        NFmiInfoData::Type theDataType,
        const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
        NFmiAreaMask::FunctionType thePrimaryFunc,
        NFmiAreaMask::FunctionType theSecondaryFunc,
        int theArgumentCount,
        const boost::shared_ptr<NFmiArea> &theCalculationArea,
        bool synopXCase,
        unsigned long thePossibleMetaParamId);
    NFmiInfoAreaMaskOccurranceSimpleCondition(const NFmiInfoAreaMaskOccurranceSimpleCondition &theOther);
    NFmiAreaMask *Clone(void) const override;
    NFmiInfoAreaMaskOccurranceSimpleCondition &operator=(const NFmiInfoAreaMaskOccurranceSimpleCondition &theMask) = delete;

    double Value(const NFmiCalculationParams &theCalculationParams,
        bool fUseTimeInterpolationAlways) override;

protected:
    float CalculationPointValue(int theOffsetX, int theOffsetY, const NFmiMetTime &theInterpolationTime, bool useInterpolatedTime) override;
    void DoIntegrationCalculations(float value) override;
    void DoCalculateCurrentLocation(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
        const NFmiLocation &theLocation,
        bool theIsStationLocationsStoredInData,
        int &theOccurranceCountInOut) override;
};


// NFmiPeekTimeMask -luokka 'kurkkaa' datasta annetun tunti offsetin verran ajassa eteen/taaksepäin.
class NFmiPeekTimeMask : public NFmiInfoAreaMask
{
public:
    NFmiPeekTimeMask(Type theMaskType,
        NFmiInfoData::Type theDataType,
        const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
        int theArgumentCount,
        double observationRadiusInKm,
        unsigned long thePossibleMetaParamId);
    ~NFmiPeekTimeMask(void);
    NFmiPeekTimeMask(const NFmiPeekTimeMask &theOther);
    NFmiAreaMask *Clone(void) const override;
    void Initialize(void) override;

    double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways) override;
    void SetArguments(std::vector<float> &theArgumentVector) override;

protected:
    double CalcValueFromObservation(const NFmiPoint &theLatlon, const NFmiMetTime &thePeekTime);

    // kuinka paljon kurkataan ajassa eteen/taakse
    long itsTimeOffsetInMinutes;  
    bool fUseMultiSourceData;
    // Tähän laitetaan havainto laskuissa käytettävät datat, joko se joko on jo emoluokassa
    // oleva itsInfo, tai multisource tapauksissa joukko datoja  
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> itsInfoVector;
    // Lähintä havaintodataa etsitään tämän säteen sisältä. 
    // Jos arvo on kFloatMissing, etsinnässä ei ole rajoja.
    double itsObservationRadiusInKm;
};

class _FMI_DLL NFmiInfoAreaMaskTimeRange : public NFmiPeekTimeMask
{
public:
    ~NFmiInfoAreaMaskTimeRange(void);
    NFmiInfoAreaMaskTimeRange(const NFmiCalculationCondition &theOperation,
        Type theMaskType,
        NFmiInfoData::Type theDataType,
        const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
        NFmiAreaMask::FunctionType theIntegrationFunc,
        int theArgumentCount,
        double observationRadiusInKm,
        unsigned long thePossibleMetaParamId);
    NFmiInfoAreaMaskTimeRange(const NFmiInfoAreaMaskTimeRange &theOther);
    NFmiAreaMask *Clone(void) const override;
    NFmiInfoAreaMaskTimeRange &operator=(const NFmiInfoAreaMaskTimeRange &theMask) = delete;
    void SetArguments(std::vector<float> &theArgumentVector) override;

    // tätä kaytetaan smarttool-modifierin yhteydessä
    double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways) override;

protected:
    virtual void InitializeFromArguments(void);
    void InitializeIntegrationValues() override;
    virtual void CalcValueFromGridData(const NFmiCalculationParams &theCalculationParams);
    virtual void CalcValueFromObservationData(const NFmiCalculationParams &theCalculationParams);
    void DoTimeLoopCalculationsForGridData(unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, const NFmiLocationCache &theLocationCache, NFmiCalculationParams &theCalculationParams);
    void DoTimeLoopCalculationsForObservationData(boost::shared_ptr<NFmiFastQueryInfo> &info, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, NFmiCalculationParams &theCalculationParams);

    NFmiAreaMask::FunctionType itsIntegrationFunc;  // esim. max, min, avg, sum
    boost::shared_ptr<NFmiDataModifier>
        itsFunctionModifier;               // tämä luodaan itsIntegrationFunc-dataosan mukaan
    std::vector<float> itsArgumentVector;  // tähän lasketaan lennossa laskuissa tarvittavat
                                           // argumentit (alueen säde ja raja(t))

    double itsStartTimeOffsetInHours;  // kuinka monta tuntia aikaiteroinnin alkuaika poikkeaa
                                       // current-timesta
    double itsEndTimeOffsetInHours;    // kuinka monta tuntia aikaiteroinnin loppuaika poikkeaa
                                       // current-timesta
};

// Maski joka laskee halutun määrän edellisten päivien yli valitun integraatio funktion.
// Maski toimii lokaali ajassa ja jos tarkastelu aika on lokaalia aikaa klo 15 ja päivien lukumäärä on 1, tällöin
// lasketaan datasta kustakin paikasta kerrallaan arvot aikavälillä 0 - 15 sinä päivänä (lokaali tunteja).
// Jos paivien määrä oli 2, tällöin laskettaisiin tarkastelupäivän tunnit 0-15 ja koko edellinen päivä 0-23.59.
// Jos valittu parametri on hiladataa, käytetään sitä. 
// Jos valittu parametri asema dataa, lasketaan arvot lähimmästä asemasta kustakin pisteeseen. 
// Havainto hakuja voidaan rajoittaa halutulla kilometri säteellä.
class _FMI_DLL NFmiInfoAreaMaskPreviousFullDays : public NFmiInfoAreaMaskTimeRange
{
public:
    ~NFmiInfoAreaMaskPreviousFullDays(void);
    NFmiInfoAreaMaskPreviousFullDays(const NFmiCalculationCondition &theOperation,
        Type theMaskType,
        NFmiInfoData::Type theDataType,
        const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
        NFmiAreaMask::FunctionType theIntegrationFunc,
        int theArgumentCount,
        double observationRadiusInKm,
        unsigned long thePossibleMetaParamId);
    NFmiInfoAreaMaskPreviousFullDays(const NFmiInfoAreaMaskPreviousFullDays &theOther);
    NFmiAreaMask *Clone(void) const override;
    NFmiInfoAreaMaskPreviousFullDays &operator=(const NFmiInfoAreaMaskPreviousFullDays &theMask) = delete;

protected:
    void InitializeFromArguments(void) override;
    void CalcValueFromGridData(const NFmiCalculationParams &theCalculationParams) override;
    void CalcValueFromObservationData(const NFmiCalculationParams &theCalculationParams) override;

    // Kuinka monta edellistä päivää otetaan laskuissa mukaan
    int itsPreviousDayCount;
};

// Tutkitaan kuinka kauan haluttu simple-condition on voimassa alkaen laskentahetkestä joko
// eteen/taaksepäin.
class _FMI_DLL NFmiInfoAreaMaskTimeDuration : public NFmiInfoAreaMaskTimeRange
{
public:
    ~NFmiInfoAreaMaskTimeDuration(void);
    NFmiInfoAreaMaskTimeDuration(const NFmiCalculationCondition &theOperation,
        Type theMaskType,
        NFmiInfoData::Type theDataType,
        const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
        int theArgumentCount,
        double observationRadiusInKm,
        unsigned long thePossibleMetaParamId);
    NFmiInfoAreaMaskTimeDuration(const NFmiInfoAreaMaskTimeDuration &theOther);
    NFmiAreaMask *Clone(void) const override;
    NFmiInfoAreaMaskTimeDuration &operator=(const NFmiInfoAreaMaskTimeDuration &theMask) = delete;

    // tätä kaytetaan smarttool-modifierin yhteydessä
    double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways);
protected:
    void InitializeFromArguments(void) override;
    void InitializeIntegrationValues() override;
    void CalcValueFromGridData(const NFmiCalculationParams &theCalculationParams) override;
    void CalcValueFromObservationData(const NFmiCalculationParams &theCalculationParams) override;
    void CalcDurationTime(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiCalculationParams &theCalculationParams);

    // Kuinka pitkälle eteen/taaksepäin maksimissaan tehdään tarkasteluja alkaen laskenta hetkestä.
    float itsSeekTimeInHours;
    int itsSeekTimeInMinutes; // pyöristetty seek-time minuuteiksi laskettuna
    int itsCalculatedTimeDurationInMinutes; // Tähän lasketaan tapahtuman kesto
    // Lasketaanko tapahtuman kesto koko annetun ajan yli (1 eli true) tai lasketaanko 
    // vain alkuhetkestä siihen asti kuin sitä aluksi kestää (0 eli false)
    bool fUseCumulativeCalculation;
};
