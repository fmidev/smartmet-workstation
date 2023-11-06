#pragma once

#include <newbase/NFmiInfoAreaMask.h>

#include <boost/thread.hpp>

class NFmiDataModifier;
class NFmiDataIterator;
class NFmiFastQueryInfo;
class NFmiGriddingHelperInterface;
class NFmiDrawParam;
class NFmiGriddingProperties;
class NFmiIgnoreStationsData;

// tämä maski osaa laskea halutulle asemadatalle hilatut arvot halutulle alueelle
// Jos maskin itsInfo on station-dataa, sen laskut tehdään toisella tavalla kuin 'normaalin'
// hila-datan kanssa
// 1. Se pitää initilisoida kerran joka erillistä aikaa kohden eli lasketaan matriisiin valmiiksi
// kaikki arvot kerralla
// 2. Kun maskin arvoja pyydetään Value-metodissa, ne saadaan valmiiksi lasketusta taulukosta (aika
// initialisointi voi tapahtua myös siellä)
class NFmiStation2GridMask : public NFmiInfoAreaMask
{
 public:
  typedef std::map<NFmiMetTime, NFmiDataMatrix<float> > DataCache;

  using GriddingFunctionCallBackType =
      std::function<void(NFmiGriddingHelperInterface *,
                         const boost::shared_ptr<NFmiArea> &,
                         boost::shared_ptr<NFmiDrawParam> &,
                         NFmiDataMatrix<float> &,
                         const NFmiMetTime &,
                         const NFmiGriddingProperties &griddingProperties)>;

  NFmiStation2GridMask(Type theMaskType,
                       NFmiInfoData::Type theDataType,
                       boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                       unsigned long thePossibleMetaParamId);
  ~NFmiStation2GridMask();
  NFmiStation2GridMask(const NFmiStation2GridMask &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiStation2GridMask &operator=(const NFmiStation2GridMask &theMask) = delete;

  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  virtual void SetGriddingHelpers(NFmiArea *theArea,
                                  NFmiGriddingHelperInterface *theGriddingHelper,
                                  const NFmiPoint &theStation2GridSize,
                                  float theObservationRadiusInKm,
                                  bool useCalculationPoints);
  static void SetGriddingStationDataCallback(
      GriddingFunctionCallBackType theGridStationDataCallback)
  {
    itsGridStationDataCallback = theGridStationDataCallback;
  }

 protected:
  void DoGriddingCheck(const NFmiCalculationParams &theCalculationParams);
  bool IsNearestPointCalculationUsed() const;
  double DoNearestPointCalculations(const NFmiCalculationParams &theCalculationParams);
  void GetUsedObservationInfoVector();
  boost::shared_ptr<NFmiDrawParam> MakeUsedDataRetrievingDrawParam() const;
  double GetFinalValueFromNearestLocation(const boost::shared_ptr<NFmiFastQueryInfo> &info,
                                          NFmiIgnoreStationsData &ignoreStationData,
                                          const NFmiLocation &calculationLocation);
  double GetFinalValueFromNearestLocationWithMetaParameterChecks(
      const boost::shared_ptr<NFmiFastQueryInfo> &info);

  // tässä on asemadatasta lasketut hilatut arvot, tämä jaetaan kaikkien kopioiden kesken,
  // jotta multi-thread -koodi saa jaettua työtä
  boost::shared_ptr<DataCache> itsGriddedStationData;
  // tähän on laitettu se matriisi, joka/ sisältää halutun ajan asemadatasta lasketut hilatut arvot
  NFmiDataMatrix<float> *itsCurrentGriddedStationData;
  // tälle ajanhetkelle on station data laskettu (tai puuttuva aika), mutta onko se sama kuin
  // itsTime, jos ei ole, pitää laskea juuri tälle ajalle
  NFmiMetTime itsLastCalculatedTime;

  // Näille muuttujille pitää asettaa arvot erillisellä SetGridHelpers-funktiolla
  boost::shared_ptr<NFmiArea> itsAreaPtr;
  NFmiGriddingHelperInterface *itsGriddingHelper;
  // tämän kokoiseen hilaan asema data lasketaan (itsGriddedStationData -koko)
  NFmiPoint itsStation2GridSize;
  // Normaalisti havainto laskuissa ei rajoiteta käytettyjä havaintoja etäisyyden perusteellä.
  // Jos tähän annetaan jotain kFloatMissing:istä poikkeavaa, niin silloin rajoitetaan.
  float itsObservationRadiusInKm;

  // Kun itsCurrentGriddedStationData -muuttujaa lasketaan tai asetetaan, sen saa tehdä kullekin
  // ajalle vain kerran. Tämä lukko systeemi takaa sen.
  typedef boost::shared_mutex MutexType;
  // Read-lockia ei oikeasti tarvita, mutta laitan sen tähän, jos joskus tarvitaankin
  typedef boost::shared_lock<MutexType> ReadLock;
  typedef boost::unique_lock<MutexType> WriteLock;
  // TÄMÄ jaetaan kaikkien kopioiden kesken, jotta multi-thread -koodi saa jaettua työtä
  boost::shared_ptr<MutexType> itsCacheMutex;
  // Callback funktio asemadatan griddaus funktioon
  static GriddingFunctionCallBackType itsGridStationDataCallback;

  // Uusi ominaisuus eli käytetään nearest havaintoa laskuissa, kun käytössä yhtäaikaa
  // calculationpoint ja ObservationRadiusInKm. Tälläisessä tilanteessa laskenta pisteeseen etsitään
  // hakurajoituksen puitteissa lähin ei puuttuva arvo. Lisäksi hakuihion ei oteta mukaan ns.
  // liikuvaa asemadataa, kuten ship/buoy.
  bool fUseCalculationPoints = false;
  // Haetaan vain kerran käytetyt datat. Tätä muuttujaa käytetään siksi, että jos kyseistä dataa ei
  // löydy ollenkaan, tiedetään että haku on tehty kuitenkin (nyt ei siis riitä
  // itsUsedObservationInfoVector.empty -tarkastelu).
  bool fUsedObservationInfoVectorRetrieved = false;
  std::vector<boost::shared_ptr<NFmiFastQueryInfo> > itsUsedObservationInfoVector;
};

// NFmiNearestObsValue2GridMask -luokka laskee havainto datasta sellaisen
// hilan, mihin sijoitetaan kuhunkin hilapisteeseen vain sitä lähimmän aseman
// arvon (oli se puuttuvaa tai ei). Eli kaikkiin hilapisteisiin ei tule arvoa.
// Tämän avulla on tarkoitus voida tehdä 'asemapiste' -laskuja ja visualisoida
// niitä kartalla vain teksti muodossa. Tällöin teksti tulee näkyviin kartalla
// lähellä kunkin aseman omaa pistettä ja muut hilapisteet ovat puuttuvaa joten
// siihen ei tule näkyviin mitään.
class NFmiNearestObsValue2GridMask : public NFmiInfoAreaMask
{
 public:
  typedef std::map<NFmiMetTime, NFmiDataMatrix<float> > DataCache;

  NFmiNearestObsValue2GridMask(Type theMaskType,
                               NFmiInfoData::Type theDataType,
                               boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                               int theArgumentCount,
                               unsigned long thePossibleMetaParamId);
  ~NFmiNearestObsValue2GridMask();
  NFmiNearestObsValue2GridMask(const NFmiNearestObsValue2GridMask &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiStation2GridMask &operator=(const NFmiStation2GridMask &theMask) = delete;

  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  void SetGriddingHelpers(NFmiArea *theArea,
                          NFmiGriddingHelperInterface *theGriddingHelper,
                          const NFmiPoint &theResultGridSize);
  void SetArguments(std::vector<float> &theArgumentVector) override;

 private:
  void DoNearestValueGriddingCheck(const NFmiCalculationParams &theCalculationParams);

  boost::shared_ptr<DataCache> itsNearestObsValuesData;  // Tämä jaetaan kaikkien kopioiden kesken,
                                                         // jotta multi-thread -koodi saa jaettua
                                                         // työtä
  NFmiDataMatrix<float> *itsCurrentNearestObsValuesData;  // tähän on laitettu se matriisi, joka
                                                          // sisältää halutun ajan asemadatasta
                                                          // lasketut hilatut arvot
  NFmiMetTime itsLastCalculatedTime;  // tälle ajanhetkelle on station data laskettu (tai puuttuva
                                      // aika), mutta onko se sama kuin itsTime, jos ei ole, pitää
                                      // laskea juuri tälle ajalle

  // Näille muuttujille pitää asettaa arvot erillisellä SetGridHelpers-funktiolla
  boost::shared_ptr<NFmiArea> itsAreaPtr;  // omistaa ja tuhoaa!!
  NFmiGriddingHelperInterface *itsGriddingHelper;
  NFmiPoint itsResultGridSize;  // tämän kokoiseen hilaan asema data lasketaan
                                // (itsNearestObsValuesData -hilakoko)

  std::vector<float> itsArgumentVector;  // tähän lasketaan lennossa laskuissa tarvittavat
                                         // argumentit (1. aikahyppy)

  // Kun itsCurrentNearestObsValuesData -muuttujaa lasketaan tai asetetaan, sen saa tehdä kullekin
  // ajalle vain kerran. Tämä lukko systeemi takaa sen.
  typedef boost::shared_mutex MutexType;
  typedef boost::shared_lock<MutexType>
      ReadLock;  // Read-lockia ei oikeasti tarvita, mutta laitan sen tähän, jos joskus tarvitaankin
  typedef boost::unique_lock<MutexType> WriteLock;
  boost::shared_ptr<MutexType> itsCacheMutex;  // TÄMÄ jaetaan kaikkien kopioiden kesken, jotta
                                               // multi-thread -koodi saa jaettua työtä
};

// Maski luokka joka hakee datasta sen viimeisen ajanhetken arvot.
// Jos kyse hiladatasta data haetaan aina vain yhdestä datasta ja kaikki on helppoa.
// Jos kyse on asemadatasta, voi olla useita data lähteitä ja niillä voi olla erilaiset viimeiset
// ajat, tällöin valitaan viimeinen aika prioriteetti 1 datasta. Lisäksi asemadata pitää hilata,
// jossa käytetään emoluokan ominaisuuksia.
class NFmiLastTimeValueMask : public NFmiStation2GridMask
{
 public:
  NFmiLastTimeValueMask(Type theMaskType,
                        NFmiInfoData::Type theDataType,
                        boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                        int theArgumentCount,
                        unsigned long thePossibleMetaParamId);
  ~NFmiLastTimeValueMask();
  NFmiLastTimeValueMask(const NFmiLastTimeValueMask &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiLastTimeValueMask &operator=(const NFmiLastTimeValueMask &theMask) = delete;

  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  void SetGriddingHelpers(NFmiArea *theArea,
                          NFmiGriddingHelperInterface *theGriddingHelper,
                          const NFmiPoint &theStation2GridSize,
                          float theObservationRadiusInKm,
                          bool useCalculationPoints) override;

 protected:
  NFmiMetTime FindLastTime();

  // Hiladatan kanssa käytetään tätä aikaindeksiä
  unsigned long itsLastTimeIndex;
  // Tähän haetaan asemadata tapauksissa sopiva viimeinen aika riippuen siitä onko kyseessä yksi vai
  // useampi datalähteitä.
  NFmiMetTime itsLastTimeOfData;
};

class NFmiStation2GridTimeShiftMask : public NFmiStation2GridMask
{
 public:
  NFmiStation2GridTimeShiftMask(Type theMaskType,
                                NFmiInfoData::Type theDataType,
                                boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                float theTimeOffsetInHours,
                                unsigned long thePossibleMetaParamId);
  ~NFmiStation2GridTimeShiftMask();
  NFmiStation2GridTimeShiftMask(const NFmiStation2GridTimeShiftMask &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiStation2GridTimeShiftMask &operator=(const NFmiStation2GridTimeShiftMask &theMask) = delete;

  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  NFmiCalculationParams GetUsedCalculationParams(const NFmiCalculationParams &theCalculationParams);

  float itsTimeOffsetInHours;
  long itsChangeByMinutesValue;
};
