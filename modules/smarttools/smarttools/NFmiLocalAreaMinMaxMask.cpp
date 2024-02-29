#include "NFmiLocalAreaMinMaxMask.h"
#include <boost/math/special_functions/round.hpp>
#include <newbase/NFmiDataModifierAvg.h>
#include <newbase/NFmiFastInfoUtils.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <future>

static float GetTimeInterpolatedValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                      const MetaParamDataHolder &metaParamDataHolder,
                                      const NFmiMetTime &theTime)
{
  if (metaParamDataHolder.isMetaParameterCalculationNeeded())
    return NFmiFastInfoUtils::GetMetaWindValue(theInfo,
                                               theTime,
                                               metaParamDataHolder.metaWindParamUsage(),
                                               metaParamDataHolder.possibleMetaParamId());
  else
    return theInfo->InterpolatedValue(theTime);
}

NFmiLocalAreaMinMaxMask::NFmiLocalAreaMinMaxMask(Type theMaskType,
                                                 NFmiInfoData::Type theDataType,
                                                 boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                                 int theArgumentCount,
                                                 const NFmiGrid &theCalculationGrid,
                                                 unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMask(NFmiCalculationCondition(),
                       theMaskType,
                       theDataType,
                       theInfo,
                       thePossibleMetaParamId,
                       kNoValue),
      itsLocalAreaSearchRangeInKm(0),
      itsDataCache(std::make_shared<DataCache>()),
      itsCurrentDataMatrix(nullptr),
      itsLastCalculatedTime(NFmiMetTime::gMissingTime),
      itsCalculationGrid(theCalculationGrid),
      itsCacheMutex(std::make_shared<MutexType>())
{
  itsFunctionArgumentCount = theArgumentCount;
}

NFmiLocalAreaMinMaxMask::~NFmiLocalAreaMinMaxMask() = default;

NFmiLocalAreaMinMaxMask::NFmiLocalAreaMinMaxMask(const NFmiLocalAreaMinMaxMask &theOther)
    : NFmiInfoAreaMask(theOther),
      itsLocalAreaSearchRangeInKm(theOther.itsLocalAreaSearchRangeInKm),
      itsDataCache(theOther.itsDataCache),
      itsCurrentDataMatrix(theOther.itsCurrentDataMatrix),
      itsLastCalculatedTime(theOther.itsLastCalculatedTime),
      itsCalculationGrid(theOther.itsCalculationGrid),
      itsCacheMutex(theOther.itsCacheMutex)
{
}

NFmiAreaMask *NFmiLocalAreaMinMaxMask::Clone() const
{
  return new NFmiLocalAreaMinMaxMask(*this);
}

double NFmiLocalAreaMinMaxMask::Value(const NFmiCalculationParams &theCalculationParams,
                                      bool fUseTimeInterpolationAlways)
{
  InitializeFromArguments();
  DoCalculationCheck(theCalculationParams);
  if (itsCurrentDataMatrix)
    return itsCurrentDataMatrix->GetValue(theCalculationParams.itsLocationIndex, kFloatMissing);
  else
    return kFloatMissing;
}

void NFmiLocalAreaMinMaxMask::DoCalculationCheck(const NFmiCalculationParams &theCalculationParams)
{
  if (itsLastCalculatedTime != theCalculationParams.itsTime)
  {
    // Tästä saa edetä vain yksi säie kerrallaan, eli joku joka ehtii, laskee cache-matriisin ja
    // asettaa sen itsCurrentGriddedStationData:n arvoksi, muut sitten vain käyttävät sitä
    UniqueLock lock(*itsCacheMutex);

    // Katsotaanko löytyykö valmiiksi laskettua hilaa halutulle ajalle
    DataCache::iterator it = itsDataCache->find(theCalculationParams.itsTime);
    if (it != itsDataCache->end())
      itsCurrentDataMatrix = &((*it).second);
    else
    {
      auto localMinMaxMatrix = CalculateLocalMinMaxMatrix();
      InserDataToCache(localMinMaxMatrix, theCalculationParams.itsTime);
    }
    itsLastCalculatedTime = theCalculationParams.itsTime;
  }
}

class LocalExtremesSearcher
{
  LocalExtreme minimum;
  LocalExtreme maximum;

 public:
  LocalExtremesSearcher() : minimum(true), maximum(false) {}

  const LocalExtreme &getMinimum() const { return minimum; }
  const LocalExtreme &getMaximum() const { return maximum; }

  void checkForExtremes(float value,
                        unsigned long gridPointX,
                        unsigned long gridPointY,
                        const NFmiPoint &latlon)
  {
    checkForExtreme(value, gridPointX, gridPointY, latlon, minimum, std::less<float>());
    checkForExtreme(value, gridPointX, gridPointY, latlon, maximum, std::greater<float>());
  }

 private:
  template <typename CheckingPredicate>
  static void checkForExtreme(float value,
                              unsigned long gridPointX,
                              unsigned long gridPointY,
                              const NFmiPoint &latlon,
                              LocalExtreme &localExtreme,
                              const CheckingPredicate &predicate)
  {
    if (value != kFloatMissing)
    {
      if (localExtreme.itsValue == kFloatMissing || predicate(value, localExtreme.itsValue))
      {
        localExtreme.itsValue = value;
        localExtreme.itsLatlon = latlon;
        localExtreme.itsOrigDataGridPoint = NFmiPoint(gridPointX, gridPointY);
      }
    }
  }
};

// Tätä enumia käytetään ilmansuunta vektorien indeksinä.
// Ilmansuunnat ovat tässä alkaen pohjoisesta ja kääntyen myötäpäivään ympäri.
enum SearchDirections
{
  SearchDirection_N = 0,
  SearchDirection_NNE,  // north-north-east
  SearchDirection_NE,
  SearchDirection_ENE,
  SearchDirection_E,
  SearchDirection_ESE,
  SearchDirection_SE,
  SearchDirection_SSE,
  SearchDirection_S,
  SearchDirection_SSW,
  SearchDirection_SW,
  SearchDirection_WSW,
  SearchDirection_W,
  SearchDirection_WNW,
  SearchDirection_NW,
  SearchDirection_NNW,
  LastSearchDirection = SearchDirection_NNW,
  SearchDirectionCount = LastSearchDirection + 1
};

// Luokan avulla pidetään kirjaa miten kunkin äärialueen laajuus tai syvyys menee eri
// tarkasteltaviin ilmansuuntiin. Tämän avulla voidaan tehdä erilaisia päätelmiä äärialueen
// merkittävyydestä ja symmetriasta.
class LocalExtremeSearcherData
{
  // Laske jokaiselle pää- ja sivuilmansuunnalle että kuinka monta hilapistettä lokaali max/min
  // jatkuu
  std::vector<int> lengthInGridPoints;
  // Hae arvot äärialueen reunoilta, jotta niitä voidaan verrata ääripisteessä olevaan arvoon ja
  // laskea voimakkuus
  std::vector<float> valueOnExtremeAreaEdge;
  // Jatkuuko laskut kyseiseen ilmansuuntaan enää (1 = jatkuu ja 0 = ei jatku)
  std::vector<int> continueIterations;
  // Menikö äärialueen reunan etsintä datan rajalle asti (0 = ei, 1 = kyllä)
  std::vector<int> searchReachedToDataEdge;

 public:
  LocalExtremeSearcherData()
      : lengthInGridPoints(SearchDirectionCount, 0),
        valueOnExtremeAreaEdge(SearchDirectionCount, kFloatMissing),
        continueIterations(SearchDirectionCount, 1),
        searchReachedToDataEdge(SearchDirectionCount, 0)
  {
  }

  int &LengthInGridPoints(SearchDirections searchDirection)
  {
    return lengthInGridPoints[searchDirection];
  }
  float &ValueOnExtremeAreaEdge(SearchDirections searchDirection)
  {
    return valueOnExtremeAreaEdge[searchDirection];
  }
  int &ContinueIterations(SearchDirections searchDirection)
  {
    return continueIterations[searchDirection];
  }
  int &SearchReachedToDataEdge(SearchDirections searchDirection)
  {
    return searchReachedToDataEdge[searchDirection];
  }

  bool CheckIsAnyDirectionStopped() const
  {
    for (auto continueIteration : continueIterations)
    {
      if (!continueIteration)
        return true;
    }
    return false;
  }

  // Vaaka ja pysty suunnassa kerroin on 1 ja diagonaali suunnissa kerroin on neliöjuuri 2.
  static double GetGridSizeFactor(SearchDirections searchDirection)
  {
    switch (searchDirection)
    {
      case SearchDirection_N:
      case SearchDirection_S:
      case SearchDirection_W:
      case SearchDirection_E:
        return 1;
      case SearchDirection_NW:
      case SearchDirection_NE:
      case SearchDirection_SW:
      case SearchDirection_SE:
        return std::sqrt(2.);
      case SearchDirection_NNW:
      case SearchDirection_WNW:
      case SearchDirection_WSW:
      case SearchDirection_SSW:
      case SearchDirection_SSE:
      case SearchDirection_ESE:
      case SearchDirection_ENE:
      case SearchDirection_NNE:
        return std::sqrt(3.);
      default:
        return 0;
    }
  }

  static double GetGridSizeInKM(SearchDirections searchDirection,
                                const NFmiPoint &gridPointSizeInKM)
  {
    switch (searchDirection)
    {
      case SearchDirection_N:
      case SearchDirection_S:
        return gridPointSizeInKM.Y();
      case SearchDirection_W:
      case SearchDirection_E:
        return gridPointSizeInKM.X();
      case SearchDirection_NW:
      case SearchDirection_NE:
      case SearchDirection_SW:
      case SearchDirection_SE:
        return (gridPointSizeInKM.X() + gridPointSizeInKM.Y()) / 2.;
      case SearchDirection_NNW:
      case SearchDirection_NNE:
      case SearchDirection_SSW:
      case SearchDirection_SSE:
        return (gridPointSizeInKM.X() + (2 * gridPointSizeInKM.Y())) / 3.;
      case SearchDirection_WNW:
      case SearchDirection_WSW:
      case SearchDirection_ESE:
      case SearchDirection_ENE:
        return ((2 * gridPointSizeInKM.X()) + gridPointSizeInKM.Y()) / 3.;
      default:
        return 0;
    }
  }

  float CalcAreaSizeAvg(const NFmiPoint &gridPointSizeInKM) const
  {
    NFmiDataModifierAvg avg;
    for (int searchDirection = 0; searchDirection <= LastSearchDirection; searchDirection++)
    {
      auto gridSizeFactor = GetGridSizeFactor(static_cast<SearchDirections>(searchDirection));
      auto gridSizeInKM =
          GetGridSizeInKM(static_cast<SearchDirections>(searchDirection), gridPointSizeInKM);
      avg.Calculate(
          static_cast<float>(gridSizeFactor * gridSizeInKM * lengthInGridPoints[searchDirection]));
    }
    return static_cast<float>(avg.FloatValue());
  }

  float CalcDeepnessAvg(float localExtremeValue) const
  {
    NFmiDataModifierAvg avg;
    for (auto valueOnEdge : valueOnExtremeAreaEdge)
    {
      // On mahdollista että väli-väli-ilmansuunnissa on puuttuvaa (jos ollaan tarpeeksi lähellä
      // datan reunaa) ja silti halutaan pitää ääripiste mukana
      if (valueOnEdge != kFloatMissing)
        avg.Calculate(std::abs(localExtremeValue - valueOnEdge));
    }
    return static_cast<float>(avg.FloatValue());
  }

  bool HasTooOneSidedLengthVector(int lengthLimit, int maxCount) const
  {
    int hitCounter = 0;
    for (size_t searchDirectionIndex = 0; searchDirectionIndex < lengthInGridPoints.size();
         searchDirectionIndex++)
    {
      if (lengthInGridPoints[searchDirectionIndex] <= lengthLimit &&
          !searchReachedToDataEdge[searchDirectionIndex])
        hitCounter++;
      if (hitCounter >= maxCount)
        return true;
    }
    return false;
  }

  int GetMaxLength() const
  {
    return *std::max_element(lengthInGridPoints.begin(), lengthInGridPoints.end());
  }

  int GetMinNonDataEdgeLength() const
  {
    int minLength = 999999;
    for (size_t index = 0; index < lengthInGridPoints.size(); index++)
    {
      if (lengthInGridPoints[index] < minLength && !searchReachedToDataEdge[index])
        minLength = lengthInGridPoints[index];
    }
    return minLength;
  }

  // Etsi pisin pätkä suunta lukemista, missä pituus on alle annetun rajan
  int FindContinuousUnderLimitCount(int lengthLimit)
  {
    bool isUnderLimit = false;
    int maxCount = 0;
    int currentCount = 0;
    // Käydään pituus vektori kahdesti läpi, jotta ei tarvitse erikseen pähäillä miten
    // käsitellään tilanne jossa mennään rajan alle vektorin lopussa ja siitä tullaan pois vektorin
    // alussa.
    for (size_t index = 0; index < lengthInGridPoints.size() * 2; index++)
    {
      auto actuallyUsedIndex = index % lengthInGridPoints.size();
      if (lengthInGridPoints[actuallyUsedIndex] <= lengthLimit &&
          !searchReachedToDataEdge[actuallyUsedIndex])
      {
        if (!isUnderLimit)
        {
          isUnderLimit = true;
        }
        currentCount++;
      }
      else
      {
        isUnderLimit = false;
        if (currentCount > maxCount)
        {
          maxCount = currentCount;
        }
        currentCount = 0;
      }
    }
    return maxCount;
  }

  bool DiscardOneSidedLocalExtremes(float localExtremeValue, double gridPointsInSearchRange)
  {
    // Hylkää kaikki joissa jollain äärialueen reunalla on ääripisteen arvo
    for (auto valueOnEdge : valueOnExtremeAreaEdge)
    {
      if (valueOnEdge == localExtremeValue)
        return true;
    }

    auto maxLength = GetMaxLength();
    auto minLength = GetMinNonDataEdgeLength();
    auto oneSidedLengthLimit = boost::math::iround(maxLength * 0.135);
    auto maxContinuouslyUnderLimitCount = FindContinuousUnderLimitCount(oneSidedLengthLimit);
    // Jos oli tarpeeksi pitkä suunta alue alle limitin, hylätään ääripiste
    if (maxContinuouslyUnderLimitCount >= 5)
      return true;
    // minimi pituus oli alle tietyn osan maksimista, hylätään ääripiste
    if (maxLength * 0.065 >= minLength)
      return true;

    // Hylätään extreme alueet, missä alueen reunat tulee liian nopeasti joiltain reunoilta vastaan
    // (datan oikeilla reunoille menemistä ei lasketa mukaan) Tutkitaan tapaus jossa 1 suunnasta
    // tulee reuna liian nopeasti vastaan
    // auto gridPointLengthLimitFor1Direction = std::max(1,
    // boost::math::iround(gridPointsInSearchRange / 30.));
    // if(HasTooOneSidedLengthVector(gridPointLengthLimitFor1Direction, 1))
    //    return true;

    // auto gridPointLengthLimitFor2Direction = std::max(1,
    // boost::math::iround(gridPointsInSearchRange / 20.));
    // if(HasTooOneSidedLengthVector(gridPointLengthLimitFor2Direction, 2))
    //    return true;

    // auto gridPointLengthLimitFor3Direction = std::max(1,
    // boost::math::iround(gridPointsInSearchRange / 15.));
    // if(HasTooOneSidedLengthVector(gridPointLengthLimitFor3Direction, 3))
    //    return true;

    // auto gridPointLengthLimitFor4Direction = std::max(1,
    // boost::math::iround(gridPointsInSearchRange / 10.));
    // if(HasTooOneSidedLengthVector(gridPointLengthLimitFor4Direction, 4))
    //    return true;

    return false;
  }

  // Lasketaan symmetria indeksi laskemalla äärialueen reunojen etäisyyksien keskihajonta.
  float CalcExtremeAreaSymmetryIndex() const
  {
    NFmiDataModifierAvg lengthAvgCalculator;
    for (auto length : lengthInGridPoints)
      lengthAvgCalculator.Calculate(static_cast<float>(length));
    float lengthAvg = lengthAvgCalculator.CalculationResult();
    float variance = 0;
    for (size_t index = 0; index < lengthInGridPoints.size(); index++)
    {
      float diff = lengthAvg - lengthInGridPoints[index];
      if (searchReachedToDataEdge[index])
        diff *= 0.7f;  // pienennetään erotusta jonkin verran, jos etsintä meni datan reunalle
      variance += diff * diff;
    }
    variance = variance / static_cast<float>(lengthInGridPoints.size());
    float standardDeviation = std::sqrt(variance);
    float variationFactor = standardDeviation / lengthAvg;
    float symmetryIndex = 1 - variationFactor;
    if (symmetryIndex < 0)
      symmetryIndex = 0;
    return symmetryIndex;
  }

  // Tutkitaan jatkuuko trendi kun edetään johonkin ilman suuntaan eli arvojen pitää koko ajan
  // laskea (jos kyse oli maksimista) ja nousta (jos kyse oli minimistä). Jos previousGridPointValue
  // on kFloatMissing, silloin ollaan vielä 1. kierroksella ja pitää käyttää itse localExtreme:sta
  // saatavaa arvoa.
  static bool ExtremeAreaContinues(const LocalExtreme &localExtreme,
                                   float value,
                                   float previousGridPointValue)
  {
    if (value != kFloatMissing)
    {
      auto usedBaseValue = (previousGridPointValue == kFloatMissing) ? localExtreme.itsValue
                                                                     : previousGridPointValue;
      if (localExtreme.fIsMinimum)
        return usedBaseValue <= value;
      else
        return usedBaseValue >= value;
    }
    return false;
  }

  bool IsSubInterCardinalDirection(SearchDirections searchDirection)
  {
    switch (searchDirection)
    {
      case SearchDirection_NNW:
      case SearchDirection_NNE:
      case SearchDirection_SSW:
      case SearchDirection_SSE:
      case SearchDirection_WNW:
      case SearchDirection_WSW:
      case SearchDirection_ESE:
      case SearchDirection_ENE:
        return true;
      default:
        return false;
    }
  }

  bool CheckExtremeConditionFromOffset(const LocalExtreme &localExtreme,
                                       boost::shared_ptr<NFmiFastQueryInfo> &info,
                                       SearchDirections searchDirection,
                                       unsigned long checkGridPointX,
                                       unsigned long checkGridPointY,
                                       const NFmiMetTime &interpolationTime,
                                       const MetaParamDataHolder &metaParamDataHolder)
  {
    if (continueIterations[searchDirection])
    {
      if (NFmiFastInfoUtils::SetInfoToGridPoint(info, checkGridPointX, checkGridPointY))
      {
        auto value = ::GetTimeInterpolatedValue(info, metaParamDataHolder, interpolationTime);
        if (ExtremeAreaContinues(localExtreme, value, valueOnExtremeAreaEdge[searchDirection]))
        {
          lengthInGridPoints[searchDirection]++;
          // Väli-väli ilmansuunnille pitää lisätä tuplana arvoa, koska niitä tarkastellaa vain joka
          // toisella kierroksella
          if (IsSubInterCardinalDirection(searchDirection))
            lengthInGridPoints[searchDirection]++;
          valueOnExtremeAreaEdge[searchDirection] = value;
          return true;
        }
      }
      else
        searchReachedToDataEdge[searchDirection] = 1;
      continueIterations[searchDirection] = 0;
    }
    return false;
  }
};

static LocalExtremesSearcher SearchLocalMinAndMax(boost::shared_ptr<NFmiFastQueryInfo> &info,
                                                  const NFmiRect &gridPointBoundings,
                                                  const NFmiMetTime &interpolationTime,
                                                  const MetaParamDataHolder &metaParamDataHolder)
{
  // 1. Käy läpi jokainen aliruudukon hilapiste
  auto left = static_cast<unsigned long>(gridPointBoundings.Left());
  auto right = static_cast<unsigned long>(gridPointBoundings.Right());
  // Bottom ja top menevät tässä ristiin, koska NFmiRect:in top on sama kuin grid pointeissa bottom
  auto bottomRowIndex = static_cast<unsigned long>(gridPointBoundings.Top());
  auto topRowIndex = static_cast<unsigned long>(gridPointBoundings.Bottom());
  LocalExtremesSearcher extremesSearcher;
  for (auto gridPointY = bottomRowIndex; gridPointY <= topRowIndex; gridPointY++)
  {
    for (auto gridPointX = left; gridPointX <= right; gridPointX++)
    {
      if (NFmiFastInfoUtils::SetInfoToGridPoint(info, gridPointX, gridPointY))
      {
        // 2. Etsi siitä minimi ja maksimi (jos samoja arvoja, 1. löytynyt otetaan)
        extremesSearcher.checkForExtremes(
            ::GetTimeInterpolatedValue(info, metaParamDataHolder, interpolationTime),
            gridPointX,
            gridPointY,
            info->LatLonFast());
      }
    }
  }
  return extremesSearcher;
}

// Jos löydetty lokaali piste onkin originaali datan reunalla, sitä ei voi pitää aitona lokaalina
// min/max pisteenä, koska datan jatkuessa (jos data leikattu esim. skandi alueelle) voisi löytyä
// aito ääripiste kauempaa.
static bool IsExtremePointOnEdge(const LocalExtreme &localExtreme,
                                 boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  if (localExtreme.itsOrigDataGridPoint.X() == 0 || localExtreme.itsOrigDataGridPoint.Y() == 0)
    return true;
  if (localExtreme.itsOrigDataGridPoint.X() == info->GridXNumber() - 1 ||
      localExtreme.itsOrigDataGridPoint.Y() == info->GridYNumber() - 1)
    return true;
  return false;
}

static NFmiPoint CalcBaseGridPointSizeInKM(boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  double gridSizeXinKM = 0.001 * info->Area()->WorldRect().Width() / (info->GridXNumber() - 1);
  double gridSizeYinKM = 0.001 * info->Area()->WorldRect().Height() / (info->GridYNumber() - 1);
  return NFmiPoint(gridSizeXinKM, gridSizeYinKM);
}

// Laskee ääripisteen alueen koon 8 suuntaa (vaaka/pysty/diagonaalit) kilometreissa ja laskee niistä
// keskiarvon. Jos palautettu indeksi on 0, ei kyseessä ollut oikea lokaali ääripiste (se saattoi
// olla alihilan reunalla ollut ääripiste, mutta alihilan ulkopuolelta löytyy isompia/pienempiä
// arvoja)
static bool CalcExtremesAreaSizeIndex(LocalExtreme &localExtreme,
                                      boost::shared_ptr<NFmiFastQueryInfo> &info,
                                      const NFmiMetTime &interpolationTime,
                                      float localAreaSearchRangeInKm,
                                      const MetaParamDataHolder &metaParamDataHolder)
{
  auto gridPointSizeInKM = ::CalcBaseGridPointSizeInKM(info);
  auto centralGridPointX = static_cast<unsigned long>(localExtreme.itsOrigDataGridPoint.X());
  auto centralGridPointY = static_cast<unsigned long>(localExtreme.itsOrigDataGridPoint.Y());
  auto avgGridSizeInKm = (gridPointSizeInKM.X() + gridPointSizeInKM.Y()) / 2.;
  double gridPointsInSearchRange = localAreaSearchRangeInKm / avgGridSizeInKm;
  unsigned long maxGridPointOffset =
      boost::math::lround(std::max(4., localAreaSearchRangeInKm / avgGridSizeInKm));
  if (NFmiFastInfoUtils::SetInfoToGridPoint(info, centralGridPointX, centralGridPointY))
  {
    LocalExtremeSearcherData localExtremeSearcherData;
    for (unsigned long gridPointOffset = 1; gridPointOffset < maxGridPointOffset; gridPointOffset++)
    {
      bool anyDirectionContinues = false;
      anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
          localExtreme,
          info,
          SearchDirection_N,
          centralGridPointX,
          centralGridPointY + gridPointOffset,
          interpolationTime,
          metaParamDataHolder);
      anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
          localExtreme,
          info,
          SearchDirection_S,
          centralGridPointX,
          centralGridPointY - gridPointOffset,
          interpolationTime,
          metaParamDataHolder);
      anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
          localExtreme,
          info,
          SearchDirection_W,
          centralGridPointX - gridPointOffset,
          centralGridPointY,
          interpolationTime,
          metaParamDataHolder);
      anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
          localExtreme,
          info,
          SearchDirection_E,
          centralGridPointX + gridPointOffset,
          centralGridPointY,
          interpolationTime,
          metaParamDataHolder);
      anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
          localExtreme,
          info,
          SearchDirection_NW,
          centralGridPointX - gridPointOffset,
          centralGridPointY + gridPointOffset,
          interpolationTime,
          metaParamDataHolder);
      anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
          localExtreme,
          info,
          SearchDirection_NE,
          centralGridPointX + gridPointOffset,
          centralGridPointY + gridPointOffset,
          interpolationTime,
          metaParamDataHolder);
      anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
          localExtreme,
          info,
          SearchDirection_SW,
          centralGridPointX - gridPointOffset,
          centralGridPointY - gridPointOffset,
          interpolationTime,
          metaParamDataHolder);
      anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
          localExtreme,
          info,
          SearchDirection_SE,
          centralGridPointX + gridPointOffset,
          centralGridPointY - gridPointOffset,
          interpolationTime,
          metaParamDataHolder);
      // Kahdella jaollisille kohdille voidaan laskea väli-väli ilmansuunnat mukaan
      if (gridPointOffset % 2 == 0)
      {
        anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
            localExtreme,
            info,
            SearchDirection_WNW,
            centralGridPointX - gridPointOffset,
            centralGridPointY + (gridPointOffset / 2),
            interpolationTime,
            metaParamDataHolder);
        anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
            localExtreme,
            info,
            SearchDirection_ENE,
            centralGridPointX + gridPointOffset,
            centralGridPointY + (gridPointOffset / 2),
            interpolationTime,
            metaParamDataHolder);
        anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
            localExtreme,
            info,
            SearchDirection_NNW,
            centralGridPointX - (gridPointOffset / 2),
            centralGridPointY + gridPointOffset,
            interpolationTime,
            metaParamDataHolder);
        anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
            localExtreme,
            info,
            SearchDirection_NNE,
            centralGridPointX + (gridPointOffset / 2),
            centralGridPointY + gridPointOffset,
            interpolationTime,
            metaParamDataHolder);

        anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
            localExtreme,
            info,
            SearchDirection_WSW,
            centralGridPointX - gridPointOffset,
            centralGridPointY - (gridPointOffset / 2),
            interpolationTime,
            metaParamDataHolder);
        anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
            localExtreme,
            info,
            SearchDirection_ESE,
            centralGridPointX + gridPointOffset,
            centralGridPointY - (gridPointOffset / 2),
            interpolationTime,
            metaParamDataHolder);
        anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
            localExtreme,
            info,
            SearchDirection_SSW,
            centralGridPointX - (gridPointOffset / 2),
            centralGridPointY - gridPointOffset,
            interpolationTime,
            metaParamDataHolder);
        anyDirectionContinues |= localExtremeSearcherData.CheckExtremeConditionFromOffset(
            localExtreme,
            info,
            SearchDirection_SSE,
            centralGridPointX + (gridPointOffset / 2),
            centralGridPointY - gridPointOffset,
            interpolationTime,
            metaParamDataHolder);
      }

      if (!anyDirectionContinues)
        break;
      // Jos jo 1. kierroksella millä tahansa suunnalla ei enää jatketa, ei kyseessä ollut oikea
      // lokaali ääriarvo
      if (gridPointOffset == 1 && localExtremeSearcherData.CheckIsAnyDirectionStopped())
        return false;
    }
    if (localExtremeSearcherData.DiscardOneSidedLocalExtremes(localExtreme.itsValue,
                                                              gridPointsInSearchRange))
      return false;
    localExtreme.itsAreaSizeAvgInKM = localExtremeSearcherData.CalcAreaSizeAvg(gridPointSizeInKM);
    localExtreme.itsDeepnessAvg = localExtremeSearcherData.CalcDeepnessAvg(localExtreme.itsValue);
    localExtreme.itsSymmetryIndex = localExtremeSearcherData.CalcExtremeAreaSymmetryIndex();
    return true;
  }
  return false;
}

static void CheckLocalExtreme(LocalExtreme localExtremeCopy,
                              boost::shared_ptr<NFmiFastQueryInfo> &info,
                              float localAreaSearchRangeInKm,
                              const NFmiMetTime &interpolationTime,
                              std::vector<LocalExtreme> &acceptedLocalExtremesInOut,
                              const MetaParamDataHolder &metaParamDataHolder)
{
  // 3. Tutki onko löytynyt min/max koko data-alueen reunalla (jos on, eliminoi)
  if (!::IsExtremePointOnEdge(localExtremeCopy, info))
  {
    // 4. Tutki min/max:in alueen suuruus (km)
    // 5. Tutki min/max:in syvyys (gradienttien tai ero keskustan ja reunojen välillä)
    if (::CalcExtremesAreaSizeIndex(localExtremeCopy,
                                    info,
                                    interpolationTime,
                                    localAreaSearchRangeInKm,
                                    metaParamDataHolder))
    {
      // 6. Laske suuruuden ja syvyyden avulla joku merkitsevyys indeksi
      localExtremeCopy.itsSignificance = localExtremeCopy.itsAreaSizeAvgInKM *
                                         localExtremeCopy.itsDeepnessAvg *
                                         localExtremeCopy.itsSymmetryIndex;
      acceptedLocalExtremesInOut.push_back(localExtremeCopy);
    }
  }
}

static const float kMaxAllowedMissingPercentage = 10.f;
static const float kMinNonEqualPercentage = 50.f;

class ExtremeSearchCoreCounter
{
 public:
  // Tällä nollataan laskurit samalla kun asetetaan uusi tarkastelu arvo laskentaan.
  bool SetCenterValue(float centerValue)
  {
    *this = ExtremeSearchCoreCounter();
    if (centerValue == kFloatMissing)
      return false;
    itsCenterValue = centerValue;
    return true;
  }
  void AddCheckedValue(float value)
  {
    itsTotalCounter++;
    if (value == kFloatMissing)
      itsMissingValueCounter++;
    else if (value > itsCenterValue)
      itsOverValueCounter++;
    else if (value == itsCenterValue)
      itsEqualValueCounter++;
    else if (value < itsCenterValue)
      itsUnderValueCounter++;
  }

  bool IsMaximum() const
  {
    if (OverPercentage() == 0)
    {
      // Halutaan mielellään ei-puuttuvasta datasta tehtyjä laskelmia
      if (MissingPercentage() < kMaxAllowedMissingPercentage)
      {
        if (UnderPercentage() > kMinNonEqualPercentage)
          return true;
      }
    }
    return false;
  }
  bool IsMinimum() const
  {
    if (UnderPercentage() == 0)
    {
      // Halutaan mielellään ei-puuttuvasta datasta tehtyjä laskelmia
      if (MissingPercentage() < kMaxAllowedMissingPercentage)
      {
        if (OverPercentage() > kMinNonEqualPercentage)
          return true;
      }
    }
    return false;
  }

  static float CalculatePercentage(int totalCounter, int wantedcounter)
  {
    return 100.f * wantedcounter / totalCounter;
  }

  float OverPercentage() const { return CalculatePercentage(itsTotalCounter, itsOverValueCounter); }
  float UnderPercentage() const
  {
    return CalculatePercentage(itsTotalCounter, itsUnderValueCounter);
  }
  float EqualPercentage() const
  {
    return CalculatePercentage(itsTotalCounter, itsEqualValueCounter);
  }
  float MissingPercentage() const
  {
    return CalculatePercentage(itsTotalCounter, itsMissingValueCounter);
  }

  float itsCenterValue = kFloatMissing;
  // Kuinka monta tarkastelu arvoa oli isompia kuin itsCenterValue
  int itsOverValueCounter = 0;
  // Kuinka monta tarkastelu arvoa oli yhtäsuuria kuin itsCenterValue
  int itsEqualValueCounter = 0;
  // Kuinka monta tarkastelu arvoa oli pienempiä kuin itsCenterValue
  int itsUnderValueCounter = 0;
  // Kuinka monta tarkastelu arvoa oli puuttuvia
  int itsMissingValueCounter = 0;
  // Kuinka monta lukua tarkastettiin kaiken kaikkiaan
  int itsTotalCounter = 0;
};

static void AddAdditionalLocalExtreme(std::vector<LocalExtreme> &localExtremesInOut,
                                      float centerValue,
                                      int subGridOrigoX,
                                      int subGridOrigoY,
                                      int columnIndex,
                                      int rowIndex,
                                      bool isMinimum)
{
  LocalExtreme localExtreme;
  localExtreme.itsValue = centerValue;
  localExtreme.fIsMinimum = isMinimum;
  localExtreme.itsOrigDataGridPoint = NFmiPoint(static_cast<double>(subGridOrigoX + columnIndex),
                                                static_cast<double>(subGridOrigoY + rowIndex));
  localExtremesInOut.push_back(localExtreme);
}

// Käy läpi annettu subGridValues matriisi ja etsi siitä lokaaleja min/max pisteitä.
// Etsintä tehdään annetun kokoisessa searchCore:ssa eli jos searchCoreRadiusInGridPoints = 1
// etsintä tehdään 3x3 liukuvalla searchCore:lla. Jos searchCoreRadiusInGridPoints = 2
// etsintä tehdään 5x5 liukuvalla searchCore : lla.
static std::vector<LocalExtreme> LookForAdditionalLocalExtremes(
    const NFmiDataMatrix<float> &subGridValues,
    int subGridOrigoX,
    int subGridOrigoY,
    int searchCoreRadiusInGridPoints)
{
  std::vector<LocalExtreme> localExtremes;
  ExtremeSearchCoreCounter searchCore;

  // Ei voi etsiä jos matriisi on liian pieni
  if (static_cast<int>(subGridValues.NX()) > 3 * searchCoreRadiusInGridPoints &&
      static_cast<int>(subGridValues.NY()) > 3 * searchCoreRadiusInGridPoints)
  {
    // Käydään läpi kaikki ne pisteet mille voidaan tehdä laskut täydellä searchCore:lla
    for (size_t rowIndex = searchCoreRadiusInGridPoints;
         rowIndex < subGridValues.NY() - searchCoreRadiusInGridPoints;
         rowIndex++)
    {
      for (size_t columnIndex = searchCoreRadiusInGridPoints;
           columnIndex < subGridValues.NX() - searchCoreRadiusInGridPoints;
           columnIndex++)
      {
        float centerValue = subGridValues[columnIndex][rowIndex];
        searchCore.SetCenterValue(centerValue);
        // Käydään tässä pisteessä läpi searchCore:n arvot eli pistettä ympäröivän searchCore:n
        // kokoisen laatikon arvot
        for (size_t searchCoreRowIndex = rowIndex - searchCoreRadiusInGridPoints;
             searchCoreRowIndex <= rowIndex + searchCoreRadiusInGridPoints;
             searchCoreRowIndex++)
        {
          for (size_t searchCoreColumnIndex = columnIndex - searchCoreRadiusInGridPoints;
               searchCoreColumnIndex <= columnIndex + searchCoreRadiusInGridPoints;
               searchCoreColumnIndex++)
          {
            float checkedValue = subGridValues[searchCoreColumnIndex][searchCoreRowIndex];
            searchCore.AddCheckedValue(checkedValue);
          }
        }
        if (searchCore.IsMaximum())
          ::AddAdditionalLocalExtreme(localExtremes,
                                      centerValue,
                                      subGridOrigoX,
                                      subGridOrigoY,
                                      static_cast<int>(columnIndex),
                                      static_cast<int>(rowIndex),
                                      false);
        else if (searchCore.IsMinimum())
          ::AddAdditionalLocalExtreme(localExtremes,
                                      centerValue,
                                      subGridOrigoX,
                                      subGridOrigoY,
                                      static_cast<int>(columnIndex),
                                      static_cast<int>(rowIndex),
                                      true);
      }
    }
  }
  return localExtremes;
}

// Oletus molemmat on jo tarkastettu että ne ovat joko min tai max tyyppiä.
// Tällöin merkittävyyteen vaikuttaa se kummassa on isompi/pienempi arvo, koska meitä kiinnostaa
// kahden läheisen min/max ääripisteen kanssa se äärevämpi tapaus.
static void DoSameTypeLocalExtremeRemovalMarking(LocalExtreme &localExtreme1,
                                                 LocalExtreme &localExtreme2)
{
  if (localExtreme1.fIsMinimum)
  {
    if (localExtreme1.itsValue <= localExtreme2.itsValue)
      localExtreme2.fRemoveFromResults = true;
    else
      localExtreme1.fRemoveFromResults = true;
  }
  else
  {
    if (localExtreme1.itsValue >= localExtreme2.itsValue)
      localExtreme2.fRemoveFromResults = true;
    else
      localExtreme1.fRemoveFromResults = true;
  }
}

static void CheckIsAdditionalLocalExtremeRemovedByProximity(
    LocalExtreme &localExtreme1,
    LocalExtreme &localExtreme2,
    double proximityLimitForAdditionalLocalExtremesInGridPoints)
{
  if (localExtreme1.fRemoveFromResults || localExtreme2.fRemoveFromResults)
    return;
  double xDiff = localExtreme1.itsOrigDataGridPoint.X() - localExtreme2.itsOrigDataGridPoint.X();
  double yDiff = localExtreme1.itsOrigDataGridPoint.Y() - localExtreme2.itsOrigDataGridPoint.Y();
  auto distanceInGridPoints = std::sqrt(xDiff * xDiff + yDiff * yDiff);
  if (distanceInGridPoints <= proximityLimitForAdditionalLocalExtremesInGridPoints)
  {
    if (localExtreme1.fIsMinimum == localExtreme2.fIsMinimum)
    {
      ::DoSameTypeLocalExtremeRemovalMarking(localExtreme1, localExtreme2);
    }
  }
}

static void RemoveMarkedLocalExtremesFromVector(std::vector<LocalExtreme> &localExtremesInOut)
{
  // Poista tuloksista poistettaviksi merkityt ääripisteet
  localExtremesInOut.erase(
      std::remove_if(localExtremesInOut.begin(),
                     localExtremesInOut.end(),
                     [](LocalExtreme &localExtreme) { return localExtreme.fRemoveFromResults; }),
      localExtremesInOut.end());
}

static void RemoveTooCloseAdditionalLocalExtremes(std::vector<LocalExtreme> &localExtremesInOut,
                                                  boost::shared_ptr<NFmiFastQueryInfo> &info,
                                                  float localAreaSearchRangeInKm)
{
  if (localExtremesInOut.size() > 1)
  {
    auto gridPointSizeInKm = ::CalcBaseGridPointSizeInKM(info);
    auto avgGridPointSizeInKm = (gridPointSizeInKm.X() + gridPointSizeInKm.Y()) / 2.;
    double proximityLimitForAdditionalLocalExtremesInGridPoints =
        (localAreaSearchRangeInKm / 10.) / avgGridPointSizeInKm;
    for (size_t firstIndex = 0; firstIndex < localExtremesInOut.size() - 1; firstIndex++)
    {
      for (size_t secondIndex = firstIndex + 1; secondIndex < localExtremesInOut.size();
           secondIndex++)
      {
        ::CheckIsAdditionalLocalExtremeRemovedByProximity(
            localExtremesInOut[firstIndex],
            localExtremesInOut[secondIndex],
            proximityLimitForAdditionalLocalExtremesInGridPoints);
      }
    }
    ::RemoveMarkedLocalExtremesFromVector(localExtremesInOut);
  }
}

static void FillLocationInfoToAdditionalLocalExtreme(LocalExtreme &localExtreme,
                                                     boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  if (NFmiFastInfoUtils::SetInfoToGridPoint(
          info,
          static_cast<unsigned long>(localExtreme.itsOrigDataGridPoint.X()),
          static_cast<unsigned long>(localExtreme.itsOrigDataGridPoint.Y())))
    localExtreme.itsLatlon = info->LatLonFast();
}

static void FillAdditionalLocalExtremesWithData(
    std::vector<LocalExtreme> &additionalLocalExtremesInOut,
    boost::shared_ptr<NFmiFastQueryInfo> &info,
    float localAreaSearchRangeInKm,
    const NFmiMetTime &interpolationTime,
    const MetaParamDataHolder &metaParamDataHolder)
{
  std::vector<LocalExtreme> finalLocalExtremes;
  for (auto &localExtreme : additionalLocalExtremesInOut)
  {
    ::FillLocationInfoToAdditionalLocalExtreme(localExtreme, info);
    ::CheckLocalExtreme(localExtreme,
                        info,
                        localAreaSearchRangeInKm,
                        interpolationTime,
                        finalLocalExtremes,
                        metaParamDataHolder);
  }
  additionalLocalExtremesInOut.swap(finalLocalExtremes);
}

// Käydään vielä läpi alihilaruudukko (jonka gridPointBoundings määrittelee) 3x3 hilaruudun etsintä
// ytimellä. Riippuen ytimen arvoista (keskusarvo vs reuna arvot), voidaan saada uusia lokaaleja
// min/max arvoja ja pisteitä.
static void LookForAdditionalLocalExtremes(std::vector<LocalExtreme> &localExtremesInOut,
                                           boost::shared_ptr<NFmiFastQueryInfo> &info,
                                           const NFmiRect &gridPointBoundings,
                                           float localAreaSearchRangeInKm,
                                           const NFmiMetTime &interpolationTime,
                                           const MetaParamDataHolder &metaParamDataHolder)
{
  NFmiDataMatrix<float> subGridValues;
  int x1 = static_cast<int>(gridPointBoundings.Left());
  // Alihilaa haettaessa pitää kääntää y-akseli
  int y1 = static_cast<int>(gridPointBoundings.Top());
  // Alihilan koko on annetun gridPointBoundings koko - 1, jotta alihilat eivät menisi päällekkäin.
  int x2 = static_cast<int>(gridPointBoundings.Right() - 1);
  int y2 = static_cast<int>(gridPointBoundings.Bottom() - 1);
  info->CroppedValues(subGridValues, interpolationTime, x1, y1, x2, y2);
  auto additionalLocalExtremes = ::LookForAdditionalLocalExtremes(subGridValues, x1, y1, 2);
  ::RemoveTooCloseAdditionalLocalExtremes(additionalLocalExtremes, info, localAreaSearchRangeInKm);
  if (!additionalLocalExtremes.empty())
  {
    ::FillAdditionalLocalExtremesWithData(additionalLocalExtremes,
                                          info,
                                          localAreaSearchRangeInKm,
                                          interpolationTime,
                                          metaParamDataHolder);
    // Lisätään mahdolliset löytyneet extra ääripisteet lopulliseen vektoriin
    localExtremesInOut.insert(
        localExtremesInOut.end(), additionalLocalExtremes.begin(), additionalLocalExtremes.end());
  }
}

static std::vector<LocalExtreme> CalculateLocalExtremesFromGivenSubGrid(
    boost::shared_ptr<NFmiFastQueryInfo> &info,
    const NFmiRect &gridPointBoundings,
    float localAreaSearchRangeInKm,
    const NFmiMetTime &interpolationTime,
    const MetaParamDataHolder &metaParamDataHolder)
{
  std::vector<LocalExtreme> localExtremes;
  auto extremeSearcher =
      ::SearchLocalMinAndMax(info, gridPointBoundings, interpolationTime, metaParamDataHolder);
  // Jos minimin ja maksimin arvot samoja (myös puuttuvia), ei ole olemassa lokaaleja min/max
  // paikkoja
  if (extremeSearcher.getMinimum().itsValue != extremeSearcher.getMaximum().itsValue)
  {
    ::CheckLocalExtreme(extremeSearcher.getMinimum(),
                        info,
                        localAreaSearchRangeInKm,
                        interpolationTime,
                        localExtremes,
                        metaParamDataHolder);
    ::CheckLocalExtreme(extremeSearcher.getMaximum(),
                        info,
                        localAreaSearchRangeInKm,
                        interpolationTime,
                        localExtremes,
                        metaParamDataHolder);
    ::LookForAdditionalLocalExtremes(localExtremes,
                                     info,
                                     gridPointBoundings,
                                     localAreaSearchRangeInKm,
                                     interpolationTime,
                                     metaParamDataHolder);
  }
  return localExtremes;
}

// Tuotanto koodin kanssa tämä define laitetaan kommenttiin, mutta jos haluaa debugata koodia,
// ota tämä pois kommentista, jolloin kaikki toiminta tehdään yhdessä säikeessä ja sarjassa.
// #define DEBUG_LOCAL_EXTREMES 1

NFmiDataMatrix<float> NFmiLocalAreaMinMaxMask::CalculateLocalMinMaxMatrix()
{
  NFmiDataMatrix<float> matrix(
      itsCalculationGrid.XNumber(), itsCalculationGrid.YNumber(), kFloatMissing);
  // Turha tehdä laskuja, jos aika ei osu ollenkaan datan sisään
  if (itsInfo->IsInside(itsTime))
  {
    auto boundaryVector = CalculateLocalAreaCalculationBoundaries();
    if (!boundaryVector.empty())
    {
#ifdef DEBUG_LOCAL_EXTREMES
      std::vector<std::vector<LocalExtreme>> results;
#else
      std::vector<std::future<std::vector<LocalExtreme>>> results;
#endif
      results.reserve(boundaryVector.size());
      std::vector<boost::shared_ptr<NFmiFastQueryInfo>> infos(boundaryVector.size(), nullptr);
      for (size_t index = 0; index < boundaryVector.size(); index++)
      {
#ifdef DEBUG_LOCAL_EXTREMES
        results.push_back(::CalculateLocalExtremesFromGivenSubGrid(itsInfo,
                                                                   boundaryVector[index],
                                                                   itsLocalAreaSearchRangeInKm,
                                                                   itsTime,
                                                                   std::ref(metaParamDataHolder)));
#else
        // Jokaiselle säikeelle pitää tehdä oma 'kevyt' kopio datan iteraattorista
        infos[index] = NFmiAreaMask::DoShallowCopy(itsInfo);
        // launch a task asynchronously
        results.emplace_back(std::async(std::launch::async,
                                        ::CalculateLocalExtremesFromGivenSubGrid,
                                        infos[index],
                                        boundaryVector[index],
                                        itsLocalAreaSearchRangeInKm,
                                        itsTime,
                                        std::ref(metaParamDataHolder)));
#endif
      }
      // Odotetaan että kaikki taskit tekevät työnsä tekemälle kaikille future:ille get
      std::vector<LocalExtreme> localExtremeResults;
      for (auto &extremeFuture : results)
      {
#ifdef DEBUG_LOCAL_EXTREMES
        auto &result = extremeFuture;
#else
        auto result = extremeFuture.get();
#endif
        if (!result.empty())
          localExtremeResults.insert(localExtremeResults.end(), result.begin(), result.end());
      }
      // Täytetään tulosmatriisia lokaali min/max paikkojen kohdilla
      FillResultMatrixWithLocalExtremePlaces(
          localExtremeResults, matrix, itsLocalAreaSearchRangeInKm);
    }
  }
  return matrix;
}

static bool IsMoreSignificantLocalExtreme(const LocalExtreme &extreme1,
                                          const LocalExtreme &extreme2)
{
  return extreme1.itsSignificance > extreme2.itsSignificance;
}

static void CheckForExtremeRemovalByProximity(LocalExtreme &importantExtreme,
                                              LocalExtreme &lessImportantExtreme,
                                              float sameTypeExtremeRangeLimitInKm,
                                              float differentTypeExtremeRangeLimitInKm)
{
  // Ei tarvitse tehdä tarkasteluja, jos jompi kumpi on jo merkitty poistettavaksi
  if (importantExtreme.fRemoveFromResults || lessImportantExtreme.fRemoveFromResults)
    return;
  NFmiLocation location(importantExtreme.itsLatlon);
  float distanceInKm =
      static_cast<float>(location.Distance(lessImportantExtreme.itsLatlon) / 1000.);
  if (importantExtreme.fIsMinimum == lessImportantExtreme.fIsMinimum)
  {
    if (distanceInKm <= sameTypeExtremeRangeLimitInKm)
    {
      ::DoSameTypeLocalExtremeRemovalMarking(importantExtreme, lessImportantExtreme);
    }
  }
  else
  {
    if (distanceInKm <= differentTypeExtremeRangeLimitInKm)
      lessImportantExtreme.fRemoveFromResults = true;
  }
}

// Eli aliruudukoista voi tulla lähekkäisiä ääripisteitä, jotka joskus ovat jopa samasta
// ääripisteestä (jossa täsmälleen samoja arvoja monessa vierekkäisessä pisteessä).
static void RemoveTooCloseExtremes(std::vector<LocalExtreme> &localExtremes,
                                   float localAreaSearchRangeInKm)
{
  if (localExtremes.size() > 1)
  {
    // Jos ääripisteet ovat samaa tyyppiä, laitetaan etäisyysraja tiukemmaksi (eli isommaksi
    // eäisyydeksi)
    auto sameTypeExtremeRangeLimitInKm = localAreaSearchRangeInKm / 1.7f;
    auto differentTypeExtremeRangeLimitInKm = localAreaSearchRangeInKm / 2.5f;
    // Tehdään loopit jossa kaikkia merkittävimpiä ääripisteitä verrataan kaikkiin vähemmän
    // merkittäviin ääripisteisiin ja katsotaan pitääkö jotain niistä eliminoida lyhyiden
    // etäisyyksien perusteella.
    for (size_t importantExtremeIndex = 0; importantExtremeIndex < localExtremes.size() - 1;
         importantExtremeIndex++)
    {
      for (size_t lessImportantExtremeIndex = importantExtremeIndex + 1;
           lessImportantExtremeIndex < localExtremes.size();
           lessImportantExtremeIndex++)
      {
        ::CheckForExtremeRemovalByProximity(localExtremes[importantExtremeIndex],
                                            localExtremes[lessImportantExtremeIndex],
                                            sameTypeExtremeRangeLimitInKm,
                                            differentTypeExtremeRangeLimitInKm);
      }
    }
    ::RemoveMarkedLocalExtremesFromVector(localExtremes);
  }
}

void NFmiLocalAreaMinMaxMask::FillResultMatrixWithLocalExtremePlaces(
    std::vector<LocalExtreme> &localExtremeResults,
    NFmiDataMatrix<float> &resultMatrix,
    float localAreaSearchRangeInKm)
{
  if (!localExtremeResults.empty())
  {
    // Sortataan tulokset merkittävyys järjestykseen
    std::sort(
        localExtremeResults.begin(), localExtremeResults.end(), ::IsMoreSignificantLocalExtreme);
    ::RemoveTooCloseExtremes(localExtremeResults, localAreaSearchRangeInKm);
    auto significanceLimit = localExtremeResults[0].itsSignificance * 0.15;
    auto areaSizeLimit = localAreaSearchRangeInKm * 0.4;
    for (const auto &localExtreme : localExtremeResults)
    {
      if (localExtreme.itsSignificance >= significanceLimit &&
          localExtreme.itsAreaSizeAvgInKM >= areaSizeLimit)
      {
        if (itsCalculationGrid.NearestLatLon(localExtreme.itsLatlon.X(),
                                             localExtreme.itsLatlon.Y()))
        {
          float value = localExtreme.fIsMinimum ? itsMinValue : itsMaxValue;
          resultMatrix[itsCalculationGrid.CurrentX()][itsCalculationGrid.CurrentY()] = value;
        }
      }
    }
  }
}

void NFmiLocalAreaMinMaxMask::InserDataToCache(const NFmiDataMatrix<float> &theMatrix,
                                               const NFmiMetTime &theTime)
{
  auto insertResult = itsDataCache->insert(std::make_pair(theTime, theMatrix));
  if (insertResult.second)
    itsCurrentDataMatrix = &((*insertResult.first).second);
  else
    itsCurrentDataMatrix = nullptr;
}

static int CalcSubGridCount(double dataLengthInKM, double searchRangeInKM)
{
  int subGridCount = boost::math::iround(dataLengthInKM / searchRangeInKM);
  if (subGridCount < 1)
    subGridCount = 1;
  if (subGridCount > 8)
    subGridCount = 8;
  return subGridCount;
}

static std::unique_ptr<NFmiArea> CalcBoundaryArea(const NFmiRect &calculationBoundary,
                                                  boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  auto grid = info->Grid();
  // HUOM! y-akselin käännös rect:in ja grid-pointtien kanssa
  auto bottomLeftLatlon = grid->GridToLatLon(calculationBoundary.TopLeft());
  auto topRightLatlon = grid->GridToLatLon(calculationBoundary.BottomRight());
  std::unique_ptr<NFmiArea> areaPtr(grid->Area()->NewArea(bottomLeftLatlon, topRightLatlon));
  return areaPtr;
}

// NFmiArea::IsInside tutkii onko kaikki toisen arean kulmapisteet alueen sisällä.
// Tässä tarvitaan tietoa että onko mikään area2:n kulmapisteistä area1:n sisällä.
static bool IsAreaInsideAtAll(NFmiArea *area1, NFmiArea *area2)
{
  return area1->IsInside(area2->BottomLeftLatLon()) || area1->IsInside(area2->TopLeftLatLon()) ||
         area1->IsInside(area2->TopRightLatLon()) || area1->IsInside(area2->BottomRightLatLon());
}

// Tutkitaan molempiin suuntiin että onko kumpikaan areoista yhtään toisen sisällä.
static bool AreAreasOverlapping(NFmiArea *boundaryArea, NFmiArea *calculationArea)
{
  return ::IsAreaInsideAtAll(boundaryArea, calculationArea) ||
         ::IsAreaInsideAtAll(calculationArea, boundaryArea);
}

static bool IsInsideCalculationArea(const NFmiRect &calculationBoundary,
                                    boost::shared_ptr<NFmiFastQueryInfo> &info,
                                    const NFmiGrid &calculationGrid)
{
  auto boundaryArea = ::CalcBoundaryArea(calculationBoundary, info);
  return ::AreAreasOverlapping(boundaryArea.get(), calculationGrid.Area());
}

// Käy läpi calculationBoundaries rect:it ja katsoo originaali infon kanssa niiden latlon laatikot
// ja katsoo meneekö nuo laatikot calculationGrid:in määrittämän alueen sisälle ollenkaan.
static std::vector<NFmiRect> RemoveOutOfCalculationAreaBoundaries(
    const std::vector<NFmiRect> &calculationBoundaries,
    boost::shared_ptr<NFmiFastQueryInfo> &info,
    const NFmiGrid &calculationGrid)
{
  std::vector<NFmiRect> insideCalculationAreaBoundaries;
  for (const auto &calculationBoundary : calculationBoundaries)
  {
    if (::IsInsideCalculationArea(calculationBoundary, info, calculationGrid))
      insideCalculationAreaBoundaries.push_back(calculationBoundary);
  }
  return insideCalculationAreaBoundaries;
}

// Jakaa itsInfo:n alueen alialueisiin (gridPointIndex rajat NFmiRect:iin) käyttäen
// itsCalculationGrid:in aluetta pohjana (laskettava alue, joka voi olla eri kuin itsInfo:n alue) ja
// alialueet ovat suhteessa itsLocalAreaSearchRangeInKm:in kanssa.
std::vector<NFmiRect> NFmiLocalAreaMinMaxMask::CalculateLocalAreaCalculationBoundaries()
{
  // Oletus: itsInfo:n pitää olla hiladataa
  double dataWidthInKM = itsInfo->Area()->WorldRect().Width() / 1000.;
  double dataHeigthInKM = itsInfo->Area()->WorldRect().Height() / 1000.;
  // Kuinka moneen aliruutuun data jaetaan
  int subGridCountX = ::CalcSubGridCount(dataWidthInKM, itsLocalAreaSearchRangeInKm);
  int subGridCountY = ::CalcSubGridCount(dataHeigthInKM, itsLocalAreaSearchRangeInKm);
  auto calculationBoundaries =
      CalculateLocalAreaCalculationBoundaries(subGridCountX, subGridCountY);
  return ::RemoveOutOfCalculationAreaBoundaries(calculationBoundaries, itsInfo, itsCalculationGrid);
}

// Kuinka monta hilapistetta kuuluu perus aliruudukkoon
static NFmiPoint CalcSubGridBaseSize(int subGridCountX, int subGridCountY, const NFmiGrid &dataGrid)
{
  double sizeX = std::ceil(dataGrid.XNumber() / static_cast<double>(subGridCountX));
  double sizeY = std::ceil(dataGrid.YNumber() / static_cast<double>(subGridCountY));
  return NFmiPoint(sizeX, sizeY);
}

static int CalcSubGridDecreaseIndex(int subGridCount,
                                    int subGridBaseSize,
                                    unsigned long actualOriginalGridSize)
{
  int nonDecreasedTotalGridSize = subGridCount * subGridBaseSize;
  int numberOfDecreasedSubGrids = nonDecreasedTotalGridSize - actualOriginalGridSize;
  return subGridCount - numberOfDecreasedSubGrids;
}

std::vector<NFmiRect> NFmiLocalAreaMinMaxMask::CalculateLocalAreaCalculationBoundaries(
    int subGridCountX, int subGridCountY)
{
  auto subGridBaseSize = ::CalcSubGridBaseSize(subGridCountX, subGridCountY, *itsInfo->Grid());
  // SubGridBaseSize:ia ei voi käyttää aina jokaiselle ruudukon ruudulle, vaan sitä pitää pienentää
  // x- ja y-suunnissa yhdellä tiettyjen pisteiden jälkeen (eri pisteet x- ja y-suunnissa).
  auto subGridDecreaseIndexX = ::CalcSubGridDecreaseIndex(
      subGridCountX, static_cast<int>(subGridBaseSize.X()), itsInfo->GridXNumber());
  auto subGridDecreaseIndexY = ::CalcSubGridDecreaseIndex(
      subGridCountY, static_cast<int>(subGridBaseSize.Y()), itsInfo->GridYNumber());
  return CalculateLocalAreaCalculationBoundaries(
      subGridCountX, subGridCountY, subGridBaseSize, subGridDecreaseIndexX, subGridDecreaseIndexY);
}

static int CalcGridPointIncrement(int subGridBaseSize, int subGridDecreaseIndex, int subGridIndex)
{
  int gridPointIncrement = subGridBaseSize;
  if (subGridIndex >= subGridDecreaseIndex)
    gridPointIncrement--;
  return gridPointIncrement;
}

// Laskettavista hilalaatikoista halutaan ei-päällekkäisiä. Eli esim. jos hilan x koko on 12 ja
// halutaan 3 alihilaa, siitä seuraa x-suunnassa seuraava:
// 1. väli = 0 - 3, 2. väli 4 - 7, 3. väli 8 - 11
// Tämä tarkoittaa että gridPointIncrementX/Y:ein kanssa saa tehdä hieman kikkailua eri kohdissa
// loppeja...
std::vector<NFmiRect> NFmiLocalAreaMinMaxMask::CalculateLocalAreaCalculationBoundaries(
    int subGridCountX,
    int subGridCountY,
    const NFmiPoint &subGridBaseSize,
    int subGridDecreaseIndexX,
    int subGridDecreaseIndexY)
{
  std::vector<NFmiRect> boundaryVector;
  int gridPointIndexY = 0;
  for (int subGridYIndex = 0; subGridYIndex < subGridCountY; subGridYIndex++)
  {
    int gridPointIncrementY = ::CalcGridPointIncrement(
        static_cast<int>(subGridBaseSize.Y()), subGridDecreaseIndexY, subGridYIndex);
    int gridPointIndexX = 0;
    for (int subGridXIndex = 0; subGridXIndex < subGridCountX; subGridXIndex++)
    {
      int gridPointIncrementX = ::CalcGridPointIncrement(
          static_cast<int>(subGridBaseSize.X()), subGridDecreaseIndexX, subGridXIndex);
      // boundingRect:in kokoa pitää vähentää 1:llä, jotta aliruudut eivät menisi päällekkäin.
      NFmiRect boundingRect(gridPointIndexX,
                            gridPointIndexY,
                            gridPointIndexX + gridPointIncrementX - 1,
                            gridPointIndexY + gridPointIncrementY - 1);
      // TODO: Tee tarkastelu, että onko kyseinen grid-point-laatikko laskenta-alueen sisällä
      // ollenkaan
      boundaryVector.push_back(boundingRect);
      gridPointIndexX += gridPointIncrementX;
    }
    gridPointIndexY += gridPointIncrementY;
  }
  return boundaryVector;
}

void NFmiLocalAreaMinMaxMask::SetArguments(std::vector<float> &theArgumentVector)
{
  // jokaiselle pisteelle ja ajanhetkelle annetaan eri argumentit tässä
  itsArgumentVector = theArgumentVector;
  // -1 tarkoittaa että funktion 1. argumentti tulee suoraan itsIfo:sta, eli sitä ei anneta
  // argumentti-listassa
  if (static_cast<int>(itsArgumentVector.size()) != itsFunctionArgumentCount - 1)
    throw std::runtime_error(
        "Internal SmartMet error: Vertical function was given invalid number of arguments, cannot "
        "calculate the macro.");
}

bool NFmiLocalAreaMinMaxMask::InitializeFromArguments()
{
  itsLocalAreaSearchRangeInKm = itsArgumentVector[0];
  itsMinValue = itsArgumentVector[1];
  itsMaxValue = itsArgumentVector[2];
  return true;
}
