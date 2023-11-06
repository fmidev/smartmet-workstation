#pragma once

#include <newbase/NFmiInfoAreaMask.h>

#include <mutex>

class NFmiFastQueryInfo;

struct LocalExtreme
{
  LocalExtreme(bool isMinimum = false)
      : itsLatlon(NFmiPoint::gMissingLatlon),
        itsOrigDataGridPoint(NFmiPoint::gMissingLatlon),
        fIsMinimum(isMinimum)
  {
  }

  // Lokaalin ääriarvo arvo
  float itsValue = kFloatMissing;
  NFmiPoint itsLatlon;
  NFmiPoint itsOrigDataGridPoint;
  // true -> minimi, false -> maksimi
  bool fIsMinimum;
  // Indeksi sille kuinka merkittävä lokaali min/max oli (kuinka laaja-alainen ja kuinka
  // syvä/korkea)
  float itsSignificance = 0;
  // 8 ilmansuunnan koko km keskiarvo
  float itsAreaSizeAvgInKM = 0;
  // 8 ilmansuunnan min/max alueen reunojen erotus keskikohtaan (itseisarvo) keskiarvo
  float itsDeepnessAvg = 0;
  // Lasketaan 8 ilmansuunnan extreme area piituuksien ja niiden keskiarvon välistä eroa ja
  // lasketaan niistä kerroin, joka on välillä 0-1. Käytetään laskuissa keskihajonnan kaavaa ja
  // lasketaan siitä 0-1 arvoväli. 1 tarkoittaa täydellistä symmetriaa (alue on ympyrä) ja 0
  // epäsymmetriaa (jonkinlainen kapea ellipsi ehkä). Tällä kertoimella kerrotaan lopullinen
  // significance.
  float itsSymmetryIndex = 0;
  // Tämä poisto flagi on tarkoitettu helpottamaan koodia, millä poistetaan toisiaan liian lähellä
  // olevia ääripisteitä. Jos tämä on false (normaali tila), tämä ääripiste on tarkoitus ainakin
  // toistaiseksi pitää mukana. Jos true, se on tarkoitus poistaa tulos vektorista ja sitä ei
  // tarvitse ottaa enää huomioon näissä laskuissa.
  bool fRemoveFromResults = false;
};

// Tämä maski etsii annetusta datasta sellaiset lokaalit minimi ja maksimi kohdat
// jotka haetaan annetun säteen kokoisilta alihilalaatikoista ja jotka ovat merkittäviä.
// Jos lokaali min/max löytyy, annetaan sellaisen pisteen arvoiksi -1/1, kaikissa
// muissa pisteissä arvo on puuttuvaa.
// Koko kenttä lasketaan ensin yhdellä kertaa DataCache:en ja sitten Value -metodilla
// vain palautetaan arvoja em. cachesta.
class NFmiLocalAreaMinMaxMask : public NFmiInfoAreaMask
{
 public:
  typedef std::map<NFmiMetTime, NFmiDataMatrix<float> > DataCache;

  NFmiLocalAreaMinMaxMask(Type theMaskType,
                          NFmiInfoData::Type theDataType,
                          boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          int theArgumentCount,
                          const NFmiGrid &theCalculationGrid,
                          unsigned long thePossibleMetaParamId);
  ~NFmiLocalAreaMinMaxMask();
  NFmiLocalAreaMinMaxMask(const NFmiLocalAreaMinMaxMask &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiLocalAreaMinMaxMask &operator=(const NFmiLocalAreaMinMaxMask &theMask) = delete;

  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  void SetArguments(std::vector<float> &theArgumentVector) override;

 protected:
  void DoCalculationCheck(const NFmiCalculationParams &theCalculationParams);
  NFmiDataMatrix<float> CalculateLocalMinMaxMatrix();
  void InserDataToCache(const NFmiDataMatrix<float> &theMatrix, const NFmiMetTime &theTime);
  std::vector<NFmiRect> CalculateLocalAreaCalculationBoundaries();
  std::vector<NFmiRect> CalculateLocalAreaCalculationBoundaries(int subGridCountX,
                                                                int subGridCountY);
  std::vector<NFmiRect> CalculateLocalAreaCalculationBoundaries(int subGridCountX,
                                                                int subGridCountY,
                                                                const NFmiPoint &subGridBaseSize,
                                                                int subGridDecreaseIndexX,
                                                                int subGridDecreaseIndexY);
  void FillResultMatrixWithLocalExtremePlaces(std::vector<LocalExtreme> &localExtremeResults,
                                              NFmiDataMatrix<float> &resultMatrix,
                                              float localAreaSearchRangeInKm);
  bool InitializeFromArguments();

  // Vertikaali funktion loput argumentit
  std::vector<float> itsArgumentVector;
  // 1. arvo itsArgumentVector:ista, tällä määrätään kuinka moneen osaan laskut jaetaan ja kuinka
  // suuria äärialueiden pitää minimissään olla
  float itsLocalAreaSearchRangeInKm;
  // 2. arvo itsArgumentVector:ista, mikä arvo annetaan minimi ääriarvoille
  float itsMinValue;
  // 3. arvo itsArgumentVector:ista, mikä arvo annetaan maksimi ääriarvoille
  float itsMaxValue;
  // Tämä jaetaan kaikkien kopioiden kesken, jotta multi-thread -koodi saa jaettua työtä
  std::shared_ptr<DataCache> itsDataCache;
  // tähän on laitettu se matriisi, joka sisältää halutun ajan asemadatasta lasketut hilatut arvot
  NFmiDataMatrix<float> *itsCurrentDataMatrix;
  // tälle ajanhetkelle on station data laskettu (tai puuttuva aika), mutta onko se sama kuin
  // itsTime, jos ei ole, pitää laskea juuri tälle ajalle
  NFmiMetTime itsLastCalculatedTime;
  // Tämän kokoiseen hilaan ja alueeseen lokaalit minimit ja maksimit lasketaan.
  NFmiGrid itsCalculationGrid;

  // Kun itsCurrentGriddedStationData -muuttujaa lasketaan tai asetetaan, sen saa tehdä kullekin
  // ajalle vain kerran. Tämä lukko systeemi takaa sen.
  typedef std::mutex MutexType;
  // Read-lockia ei oikeasti tarvita, mutta laitan sen tähän, jos joskus tarvitaankin
  typedef std::lock_guard<MutexType> UniqueLock;
  // TÄMÄ jaetaan kaikkien kopioiden kesken, jotta multi-thread -koodi saa jaettua työtä
  std::shared_ptr<MutexType> itsCacheMutex;
};
