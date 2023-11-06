
#include "NFmiInfoAreaMaskOccurrance.h"
#include "NFmiDrawParam.h"
#include "NFmiSimpleCondition.h"
#include "NFmiSmartInfo.h"
#include <boost/math/special_functions/round.hpp>
#include <newbase/NFmiDataModifier.h>
#include <newbase/NFmiDataModifierExtreme.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiProducerName.h>

// **********************************************************
// *****    NFmiInfoAreaMaskOccurrance  *********************
// **********************************************************

NFmiInfoAreaMaskOccurrance::~NFmiInfoAreaMaskOccurrance() {}
NFmiInfoAreaMaskOccurrance::NFmiInfoAreaMaskOccurrance(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    const boost::shared_ptr<NFmiArea> &theCalculationArea,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskProbFunc(theOperation,
                               theMaskType,
                               theDataType,
                               theInfo,
                               thePrimaryFunc,
                               theSecondaryFunc,
                               theArgumentCount,
                               thePossibleMetaParamId),
      itsCalculationArea(theCalculationArea),
      itsCalculatedLocationIndexies()
{
}

NFmiInfoAreaMaskOccurrance::NFmiInfoAreaMaskOccurrance(const NFmiInfoAreaMaskOccurrance &theOther)
    : NFmiInfoAreaMaskProbFunc(theOther),
      itsCalculationArea(theOther.itsCalculationArea),
      itsCalculatedLocationIndexies(theOther.itsCalculatedLocationIndexies)
{
}

NFmiAreaMask *NFmiInfoAreaMaskOccurrance::Clone() const
{
  return new NFmiInfoAreaMaskOccurrance(*this);
}

void NFmiInfoAreaMaskOccurrance::Initialize()
{
  NFmiInfoAreaMaskProbFunc::Initialize();
  // cachejen alustuksia tehdään vain asemadatoille. Hila datat hanskataan emoluokassa ja sitä en
  // lähde tässä vielä optinmoimaan.
  InitializeLocationIndexCaches();
}

void NFmiInfoAreaMaskOccurrance::InitializeLocationIndexCaches()
{
  for (auto &infoPtr : itsInfoVector)
    itsCalculatedLocationIndexies.push_back(CalcLocationIndexCache(infoPtr));
}

std::vector<unsigned long> NFmiInfoAreaMaskOccurrance::CalcLocationIndexCache(
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  if (theInfo->IsGrid())
    return std::vector<unsigned long>();  // Hiladatoista käydään kaikki pisteet toistaiseksi
                                          // (laskut tehdään emoluokassa ja sinne ei saa tätä
                                          // optimointia)
  else if (theInfo->HasLatlonInfoInData())
    return std::vector<unsigned long>();  // Liikkuvat asemadatat käydään läpi kokonaan
  else
  {
    std::vector<unsigned long> locationIndexCache;
    for (theInfo->ResetLocation(); theInfo->NextLocation();)
    {
      if (itsCalculationArea->IsInside(theInfo->LatLon()))
        locationIndexCache.push_back(theInfo->LocationIndex());
    }
    if (locationIndexCache.empty())
      locationIndexCache.push_back(gMissingIndex);  // Jos ei löytynyt laskettavan alueen sisältä
                                                    // yhtään paikkaa, lisätään yksi puuttuva
    // indeksi vektoriin, muuten data käydään läpi
    // kokonaisuudessaa.
    return locationIndexCache;
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskOccurrance::Value(const NFmiCalculationParams &theCalculationParams,
                                         bool fUseTimeInterpolationAlways)
{
  // Nämä occurance tetit tehdään erilailla riippuen onko kyse hila vai asema datasta
  if (IsGridData())
  {
    NFmiInfoAreaMaskProbFunc::Value(theCalculationParams, fUseTimeInterpolationAlways);
    return itsConditionFullfilledGridPointCount;  // Tähän on laskettu hiladatan tapauksessa
                                                  // emoluokan Value -metodissa halutun ehdon
                                                  // täyttymiset (tai puuttuva arvo)
  }
  else
  {
    // Asemadatalle tehdään omat laskut. Tätä ei ole toteutettu emoluokan laskuissa ollenkaan, koska
    // en oikein nähnyt hyödylliseksi/oikeelliseksi laskea asemadatalle todennäköisyyksiä.
    // Mutta esiintymislukumäärä on ihan ok laskea (mm. salamadatasta salamoiden esiintymiset jne.)

    InitializeFromArguments();
    // Jos käytössä on CalculationPoint, käytetään suoraan sitä, muuten itsLatlon arvoa, joka on
    // laskentahilan laskettava piste
    NFmiLocation location(theCalculationParams.itsActualCalculationPoint
                              ? *theCalculationParams.itsActualCalculationPoint
                              : theCalculationParams.UsedLatlon());
    int occurranceCount = 0;

    if (fUseMultiSourceData && itsMultiSourceDataGetter)
    {
      for (size_t i = 0; i < itsInfoVector.size(); i++)
        DoCalculations(itsInfoVector[i],
                       theCalculationParams,
                       location,
                       itsCalculatedLocationIndexies[i],
                       occurranceCount);
    }
    else
    {
      DoCalculations(itsInfo,
                     theCalculationParams,
                     location,
                     itsCalculatedLocationIndexies[0],
                     occurranceCount);
    }

    return occurranceCount;
  }
}

void NFmiInfoAreaMaskOccurrance::DoCalculations(
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const NFmiCalculationParams &theCalculationParams,
    const NFmiLocation &theLocation,
    const std::vector<unsigned long> &theLocationIndexCache,
    int &theOccurranceCountInOut)
{
  if (theLocationIndexCache.size() == 1 && theLocationIndexCache[0] == gMissingIndex)
    return;  // ei ole mitää laskettavaa tälle datalle

  bool stationLocationsStoredInData =
      theInfo->HasLatlonInfoInData();  // Eri multi source datoille voi olla erilaiset lokaatio
                                       // jutut (tosessa synop datassa vakio maaasema, toisessa
                                       // liikkuva laiva)
  // Lasketaan aikaloopitus rajat
  unsigned long origTimeIndex = theInfo->TimeIndex();  // Otetaan aikaindeksi talteen, jotta se
                                                       // voidaan lopuksi palauttaa takaisin
  unsigned long startTimeIndex = origTimeIndex;
  unsigned long endTimeIndex = origTimeIndex;
  bool doSpecialCalculationDummy = false;
  bool useCachedIndexies = !theLocationIndexCache.empty();

  NFmiMetTime interpolationTimeDummy =
      NFmiInfoAreaMaskProbFunc::CalcTimeLoopLimits(theInfo,
                                                   theCalculationParams,
                                                   itsStartTimeOffsetInHours,
                                                   itsEndTimeOffsetInHours,
                                                   &startTimeIndex,
                                                   &endTimeIndex,
                                                   &doSpecialCalculationDummy,
                                                   false);

  for (unsigned long timeIndex = startTimeIndex; timeIndex <= endTimeIndex; timeIndex++)
  {
    theInfo->TimeIndex(timeIndex);  // Käydään läpi aikajakso datan originaali aikaresoluutiossa

    if (useCachedIndexies)
    {
      for (auto index : theLocationIndexCache)
      {
        if (theInfo->LocationIndex(index))
          DoCalculateCurrentLocation(
              theInfo, theLocation, stationLocationsStoredInData, theOccurranceCountInOut);
      }
    }
    else
    {
      for (theInfo->ResetLocation(); theInfo->NextLocation();)  // Käydään läpi kaikki asemat
      {
        DoCalculateCurrentLocation(
            theInfo, theLocation, stationLocationsStoredInData, theOccurranceCountInOut);
      }
    }
    if (NFmiInfoAreaMaskProbFunc::CheckTimeIndicesForLoopBreak(startTimeIndex, endTimeIndex))
      break;
  }
  theInfo->TimeIndex(origTimeIndex);
}

void NFmiInfoAreaMaskOccurrance::DoCalculateCurrentLocation(
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const NFmiLocation &theLocation,
    bool theIsStationLocationsStoredInData,
    int &theOccurranceCountInOut)
{
  // Tarkastetaan jokainen piste erikseen, onko se halutun säteisen ympyrän sisällä
  double distanceInKM =
      theLocation.Distance(theIsStationLocationsStoredInData ? theInfo->GetLatlonFromData()
                                                             : theInfo->LatLon()) *
      0.001;
  if (distanceInKM > itsSearchRangeInKM)
    return;  // kyseinen piste oli ympyrän ulkopuolella

  float value = theInfo->FloatValue();
  if (value != kFloatMissing)
  {
    if (CheckProbabilityCondition(value))
      theOccurranceCountInOut++;
  }
}

bool NFmiInfoAreaMaskOccurrance::IsGridData() const
{
  if (itsInfo && itsInfo->Grid())
    return true;
  else
    return false;
}

// **********************************************************
// *****    NFmiInfoAreaMaskOccurrance  *********************
// **********************************************************

// **********************************************************
// *****  NFmiInfoAreaMaskOccurranceSimpleCondition *********
// **********************************************************

NFmiInfoAreaMaskOccurranceSimpleCondition::~NFmiInfoAreaMaskOccurranceSimpleCondition() = default;

NFmiInfoAreaMaskOccurranceSimpleCondition::NFmiInfoAreaMaskOccurranceSimpleCondition(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    const boost::shared_ptr<NFmiArea> &theCalculationArea,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskOccurrance(theOperation,
                                 theMaskType,
                                 theDataType,
                                 theInfo,
                                 thePrimaryFunc,
                                 theSecondaryFunc,
                                 theArgumentCount,
                                 theCalculationArea,
                                 thePossibleMetaParamId)
{
}

NFmiInfoAreaMaskOccurranceSimpleCondition::NFmiInfoAreaMaskOccurranceSimpleCondition(
    const NFmiInfoAreaMaskOccurranceSimpleCondition &theOther)
    : NFmiInfoAreaMaskOccurrance(theOther)
{
}

NFmiAreaMask *NFmiInfoAreaMaskOccurranceSimpleCondition::Clone() const
{
  return new NFmiInfoAreaMaskOccurranceSimpleCondition(*this);
}

double NFmiInfoAreaMaskOccurranceSimpleCondition::Value(
    const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways)
{
  // Nämä occurance tetit tehdään erilailla riippuen onko kyse hila vai asema datasta
  //    if(IsGridData())
  //    {
  return NFmiInfoAreaMaskOccurrance::Value(theCalculationParams, fUseTimeInterpolationAlways);
  //    }
  //    else
  //        throw std::runtime_error("Internal error in program: observation data given to occurance
  //        with simple-condition calculations");
}

float NFmiInfoAreaMaskOccurranceSimpleCondition::CalculationPointValue(
    int theOffsetX,
    int theOffsetY,
    const NFmiMetTime &theInterpolationTime,
    bool useInterpolatedTime)
{
  // Jos päästään tänne asti kasvatetaan itsTotalCalculatedGridPoints -laskuri.
  // Tänne pääsy tarkoittaa että laskenta piste on datan alueen sisällä ja laskenta ympyrän alueella
  itsTotalCalculatedGridPoints++;
  return NFmiInfoAreaMaskProbFunc::CalculationPointValue(
      theOffsetX, theOffsetY, theInterpolationTime, useInterpolatedTime);
}

void NFmiInfoAreaMaskOccurranceSimpleCondition::DoIntegrationCalculations(float value)
{
  // Jos päästään tänne asti kasvatetaan vain itsConditionFullfilledGridPointCount -laskuria, tänne
  // pääsy tarkoittaa että laskenta piste on datan alueen ja laskenta alihilan alueella ja lisäksi
  // simple-condition on päästänyt läpi (joiden osumia tässä etsitäänkin).
  itsConditionFullfilledGridPointCount++;
}

void NFmiInfoAreaMaskOccurranceSimpleCondition::DoCalculateCurrentLocation(
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const NFmiLocation &theLocation,
    bool theIsStationLocationsStoredInData,
    int &theOccurranceCountInOut)
{
  // Tarkastetaan jokainen piste erikseen, onko se halutun säteisen ympyrän sisällä
  double distanceInKM =
      theLocation.Distance(theIsStationLocationsStoredInData ? theInfo->GetLatlonFromData()
                                                             : theInfo->LatLon()) *
      0.001;
  if (distanceInKM > itsSearchRangeInKM)
    return;  // kyseinen piste oli ympyrän ulkopuolella

  float value = theInfo->FloatValue();
  NFmiCalculationParams calculationParams(
      theInfo->LatLon(), theInfo->LocationIndex(), theInfo->Time(), theInfo->TimeIndex());
  if (itsInfoVector.size() > 1)
    calculationParams.itsCurrentMultiInfoData = theInfo.get();
  if (SimpleConditionCheck(calculationParams))
    theOccurranceCountInOut++;
}

// **********************************************************
// *****  NFmiInfoAreaMaskOccurranceSimpleCondition *********
// **********************************************************

// *********************************************************************
// *************** NFmiPeekTimeMask ************************************
// *********************************************************************

NFmiPeekTimeMask::NFmiPeekTimeMask(Type theMaskType,
                                   NFmiInfoData::Type theDataType,
                                   const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                   int theArgumentCount,
                                   unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMask(NFmiCalculationCondition(),
                       theMaskType,
                       theDataType,
                       theInfo,
                       thePossibleMetaParamId,
                       NFmiAreaMask::kNoValue),
      itsTimeOffsetInMinutes(0)
{
  itsFunctionArgumentCount = theArgumentCount;
}

NFmiPeekTimeMask::~NFmiPeekTimeMask() {}

NFmiPeekTimeMask::NFmiPeekTimeMask(const NFmiPeekTimeMask &theOther)
    : NFmiInfoAreaMask(theOther), itsTimeOffsetInMinutes(theOther.itsTimeOffsetInMinutes)
{
}

NFmiAreaMask *NFmiPeekTimeMask::Clone() const
{
  return new NFmiPeekTimeMask(*this);
}

double NFmiPeekTimeMask::Value(const NFmiCalculationParams &theCalculationParams,
                               bool fUseTimeInterpolationAlways)
{
  NFmiMetTime peekTime(theCalculationParams.itsTime);
  peekTime.ChangeByMinutes(itsTimeOffsetInMinutes);
  if (itsInfo->IsGrid())
  {
    NFmiCalculationParams peekCalculationParams(theCalculationParams);
    peekCalculationParams.itsTime = peekTime;
    // Pitää aina olla aikainterpolaatio päällä (2. parametri true), koska tämä menee tuulen
    // meta-parametrien takia emoluokan metodin läpi
    return NFmiInfoAreaMask::Value(peekCalculationParams, true);
  }
  else
    return CalcValueFromObservation(theCalculationParams, peekTime);
}

double NFmiPeekTimeMask::CalcValueFromObservation(const NFmiCalculationParams &theCalculationParams,
                                                  const NFmiMetTime &thePeekTime)
{
  NFmiLocation wantedLocation(theCalculationParams.UsedLatlon());
  double valueFromMinDistance = kFloatMissing;
  double minDistanceInMetres = 99999999999;
  double searchRadiusInMetres =
      GetSearchRadiusInMetres(theCalculationParams.itsObservationRadiusInKm);
  for (auto &info : itsInfoVector)
  {
    // Datasta pitää löytyä etsitty aika suoraan ilman aikainterpolaatioita
    if (info->Time(thePeekTime))
    {
      if (info->NearestLocation(wantedLocation, searchRadiusInMetres))
      {
        double currentDistanceInMetres = wantedLocation.Distance(info->LatLon());
        if (currentDistanceInMetres < minDistanceInMetres)
        {
          minDistanceInMetres = currentDistanceInMetres;
          valueFromMinDistance = info->FloatValue();
        }
      }
    }
  }
  return valueFromMinDistance;
}

void NFmiPeekTimeMask::SetArguments(std::vector<float> &theArgumentVector)
{
  // jokaiselle pisteelle ja ajanhetkelle annetaan eri argumentit tässä
  if (static_cast<long>(theArgumentVector.size()) == itsFunctionArgumentCount - 1)
  {
    itsTimeOffsetInMinutes = static_cast<long>(std::round(theArgumentVector[0] * 60));
  }
  else
    throw std::runtime_error(
        "Internal SmartMet error: PeekTime function was given invalid number of arguments, "
        "cannot calculate the macro.");
}

// *********************************************************************
// *************** NFmiPeekTimeMask ************************************
// *********************************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskTimeRange  **********************
// **********************************************************

NFmiInfoAreaMaskTimeRange::~NFmiInfoAreaMaskTimeRange() = default;
NFmiInfoAreaMaskTimeRange::NFmiInfoAreaMaskTimeRange(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType theIntegrationFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiPeekTimeMask(theMaskType, theDataType, theInfo, theArgumentCount, thePossibleMetaParamId),
      itsIntegrationFunc(theIntegrationFunc),
      itsFunctionModifier(),
      itsArgumentVector(),
      itsStartTimeOffsetInHours(0),
      itsEndTimeOffsetInHours(0)
{
  itsMaskCondition = theOperation;
  itsFunctionArgumentCount = theArgumentCount;
  itsFunctionModifier = NFmiInfoAreaMask::CreateIntegrationFuction(itsIntegrationFunc);
}

NFmiInfoAreaMaskTimeRange::NFmiInfoAreaMaskTimeRange(const NFmiInfoAreaMaskTimeRange &theOther)
    : NFmiPeekTimeMask(theOther),
      itsIntegrationFunc(theOther.itsIntegrationFunc),
      itsFunctionModifier(theOther.itsFunctionModifier ? theOther.itsFunctionModifier->Clone()
                                                       : nullptr),
      itsArgumentVector(theOther.itsArgumentVector),
      itsStartTimeOffsetInHours(theOther.itsStartTimeOffsetInHours),
      itsEndTimeOffsetInHours(theOther.itsEndTimeOffsetInHours)
{
}

NFmiAreaMask *NFmiInfoAreaMaskTimeRange::Clone() const
{
  return new NFmiInfoAreaMaskTimeRange(*this);
}

// Tätä kutsutaan jokaiselle erillis pistelaskulle erikseen value-funktiossa.
void NFmiInfoAreaMaskTimeRange::InitializeFromArguments()
{
  itsStartTimeOffsetInHours = itsArgumentVector[0];
  itsEndTimeOffsetInHours = itsArgumentVector[1];
}

void NFmiInfoAreaMaskTimeRange::SetArguments(std::vector<float> &theArgumentVector)
{
  // jokaiselle pisteelle ja ajanhetkelle annetaan eri argumentit tässä
  itsArgumentVector = theArgumentVector;
  if (static_cast<int>(itsArgumentVector.size()) !=
      itsFunctionArgumentCount - 1)  // -1 tarkoittaa että funktion 1. argumentti tulee suoraan
                                     // itsIfo:sta, eli sitä ei anneta argumentti-listassa
    throw std::runtime_error(
        "Internal SmartMet error: Probability function was given invalid number of arguments, "
        "cannot calculate the macro.");
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskTimeRange::Value(const NFmiCalculationParams &theCalculationParams,
                                        bool /* fUseTimeInterpolationAlways */)
{
  InitializeFromArguments();
  InitializeIntegrationValues();
  if (itsInfo->IsGrid())
    CalcValueFromGridData(theCalculationParams);
  else
    CalcValueFromObservationData(theCalculationParams);

  return itsFunctionModifier->CalculationResult();
}

void NFmiInfoAreaMaskTimeRange::InitializeIntegrationValues()
{
  itsFunctionModifier->Clear();
}

void NFmiInfoAreaMaskTimeRange::CalcValueFromGridData(
    const NFmiCalculationParams &theCalculationParams)
{
  NFmiCalculationParams calculationParams = theCalculationParams;
  NFmiLocationCache locationCache = itsInfo->CalcLocationCache(calculationParams.UsedLatlon());
  if (!locationCache.NoValue())
  {
    // Lasketaan aikaloopitus rajat
    unsigned long startTimeIndex = gMissingIndex;
    unsigned long endTimeIndex = gMissingIndex;
    if (NFmiInfoAreaMask::CalcTimeLoopIndexies(itsInfo,
                                               calculationParams,
                                               itsStartTimeOffsetInHours,
                                               itsEndTimeOffsetInHours,
                                               &startTimeIndex,
                                               &endTimeIndex))
    {
      DoTimeLoopCalculationsForGridData(
          startTimeIndex, endTimeIndex, locationCache, calculationParams);
    }
  }
}

void NFmiInfoAreaMaskTimeRange::DoTimeLoopCalculationsForGridData(
    unsigned long theStartTimeIndex,
    unsigned long theEndTimeIndex,
    const NFmiLocationCache &theLocationCache,
    NFmiCalculationParams &theCalculationParams)
{
  for (unsigned long timeIndex = theStartTimeIndex; timeIndex <= theEndTimeIndex; timeIndex++)
  {
    itsInfo->TimeIndex(timeIndex);
    if (itsSimpleCondition)
      theCalculationParams.itsTime = itsInfo->Time();
    if (SimpleConditionCheck(theCalculationParams))
      AddValuesToFunctionModifier(
          itsInfo, itsFunctionModifier, theLocationCache, itsIntegrationFunc);
  }
}

void NFmiInfoAreaMaskTimeRange::CalcValueFromObservationData(
    const NFmiCalculationParams &theCalculationParams)
{
  NFmiCalculationParams calculationParams = theCalculationParams;
  size_t dataIndex = 0;
  unsigned long locationIndex = 0;
  if (FindClosestStationData(calculationParams, dataIndex, locationIndex))
  {
    // Otetaan käytetty info itsInfo:on, tarvitaan jos kyseessä multi-data tapaus (synop/salama)
    itsInfo = itsInfoVector[dataIndex];
    itsInfo->LocationIndex(locationIndex);
    // Tämä ei näytä toimivan oikein, koska itsCurrentMultiInfoData jää osoittamaan nullptr:ia.
    calculationParams.itsCurrentMultiInfoData = itsInfo.get();

    unsigned long startTimeIndex = 0;
    unsigned long endTimeIndex = 0;
    if (NFmiInfoAreaMask::CalcTimeLoopIndexies(itsInfo,
                                               calculationParams,
                                               itsStartTimeOffsetInHours,
                                               itsEndTimeOffsetInHours,
                                               &startTimeIndex,
                                               &endTimeIndex))
    {
      DoTimeLoopCalculationsForObservationData(
          itsInfo, startTimeIndex, endTimeIndex, calculationParams);
    }
  }
}

void NFmiInfoAreaMaskTimeRange::DoTimeLoopCalculationsForObservationData(
    boost::shared_ptr<NFmiFastQueryInfo> &info,
    unsigned long theStartTimeIndex,
    unsigned long theEndTimeIndex,
    NFmiCalculationParams &theCalculationParams)
{
  for (unsigned long timeIndex = theStartTimeIndex; timeIndex <= theEndTimeIndex; timeIndex++)
  {
    info->TimeIndex(timeIndex);
    if (itsSimpleCondition)
      theCalculationParams.itsTime = info->Time();
    if (SimpleConditionCheck(theCalculationParams))
      AddValueToModifier(info, itsFunctionModifier, info->FloatValue());
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskTimeRange  **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskPreviousFullDays  ***************
// **********************************************************

NFmiInfoAreaMaskPreviousFullDays::~NFmiInfoAreaMaskPreviousFullDays() = default;
NFmiInfoAreaMaskPreviousFullDays::NFmiInfoAreaMaskPreviousFullDays(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType theIntegrationFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskTimeRange(theOperation,
                                theMaskType,
                                theDataType,
                                theInfo,
                                theIntegrationFunc,
                                theArgumentCount,
                                thePossibleMetaParamId),
      itsPreviousDayCount(0)
{
}

NFmiInfoAreaMaskPreviousFullDays::NFmiInfoAreaMaskPreviousFullDays(
    const NFmiInfoAreaMaskPreviousFullDays &theOther)
    : NFmiInfoAreaMaskTimeRange(theOther), itsPreviousDayCount(theOther.itsPreviousDayCount)
{
}

NFmiAreaMask *NFmiInfoAreaMaskPreviousFullDays::Clone() const
{
  return new NFmiInfoAreaMaskPreviousFullDays(*this);
}

void NFmiInfoAreaMaskPreviousFullDays::InitializeFromArguments()
{
  itsPreviousDayCount = static_cast<int>(itsArgumentVector[0]);
}

static bool CalcTimeLoopIndexiesForPreviousFullDays(boost::shared_ptr<NFmiFastQueryInfo> &info,
                                                    const NFmiCalculationParams &calculationParams,
                                                    int previousDayCount,
                                                    unsigned long *startTimeIndexOut,
                                                    unsigned long *endTimeIndexOut)
{
  if (previousDayCount <= 0)
    return false;
  auto localCalculationTime = calculationParams.itsTime.LocalTime();
  auto localHour = localCalculationTime.GetHour();
  // Jos localHour on 0, lasketaan täydet päivät taaksepäin, jos > 0, lasketaan täysiä päiviä yksi
  // vähemmän.
  if (localHour > 0)
    previousDayCount--;
  auto hourDifference = localHour + (previousDayCount * 24);
  NFmiMetTime startTime(calculationParams.itsTime);
  startTime.ChangeByHours(-hourDifference);
  return NFmiInfoAreaMask::CalcTimeLoopIndexies(info,
                                                calculationParams,
                                                startTime,
                                                calculationParams.itsTime,
                                                startTimeIndexOut,
                                                endTimeIndexOut);
}

void NFmiInfoAreaMaskPreviousFullDays::CalcValueFromGridData(
    const NFmiCalculationParams &theCalculationParams)
{
  NFmiCalculationParams calculationParams = theCalculationParams;
  NFmiLocationCache locationCache = itsInfo->CalcLocationCache(calculationParams.UsedLatlon());
  if (!locationCache.NoValue())
  {
    // Lasketaan aikaloopitus rajat
    unsigned long startTimeIndex = gMissingIndex;
    unsigned long endTimeIndex = gMissingIndex;
    if (::CalcTimeLoopIndexiesForPreviousFullDays(
            itsInfo, calculationParams, itsPreviousDayCount, &startTimeIndex, &endTimeIndex))
    {
      DoTimeLoopCalculationsForGridData(
          startTimeIndex, endTimeIndex, locationCache, calculationParams);
    }
  }
}

void NFmiInfoAreaMaskPreviousFullDays::CalcValueFromObservationData(
    const NFmiCalculationParams &theCalculationParams)
{
  NFmiCalculationParams calculationParams = theCalculationParams;
  size_t dataIndex = 0;
  unsigned long locationIndex = 0;
  if (FindClosestStationData(calculationParams, dataIndex, locationIndex))
  {
    auto &info = itsInfoVector[dataIndex];
    info->LocationIndex(locationIndex);
    calculationParams.itsCurrentMultiInfoData = info.get();

    unsigned long startTimeIndex = 0;
    unsigned long endTimeIndex = 0;
    if (::CalcTimeLoopIndexiesForPreviousFullDays(
            info, calculationParams, itsPreviousDayCount, &startTimeIndex, &endTimeIndex))
    {
      DoTimeLoopCalculationsForObservationData(
          info, startTimeIndex, endTimeIndex, calculationParams);
    }
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskPreviousFullDays  ***************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskTimeDuration    *****************
// **********************************************************

NFmiInfoAreaMaskTimeDuration::~NFmiInfoAreaMaskTimeDuration() = default;

NFmiInfoAreaMaskTimeDuration::NFmiInfoAreaMaskTimeDuration(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskTimeRange(
          theOperation,
          theMaskType,
          theDataType,
          theInfo,
          NFmiAreaMask::Min,  // tähän on vain pakko antaa joku integraatio funktio, vaikka sitä ei
                              // käytetäkään tässä luokassa
          theArgumentCount,
          thePossibleMetaParamId),
      itsSeekTimeInHours(0),
      itsSeekTimeInMinutes(0),
      itsCalculatedTimeDurationInMinutes(0),
      fUseCumulativeCalculation(true)
{
}

NFmiInfoAreaMaskTimeDuration::NFmiInfoAreaMaskTimeDuration(
    const NFmiInfoAreaMaskTimeDuration &theOther)
    : NFmiInfoAreaMaskTimeRange(theOther),
      itsSeekTimeInHours(theOther.itsSeekTimeInHours),
      itsSeekTimeInMinutes(theOther.itsSeekTimeInMinutes),
      itsCalculatedTimeDurationInMinutes(theOther.itsCalculatedTimeDurationInMinutes),
      fUseCumulativeCalculation(theOther.fUseCumulativeCalculation),
      fHasLegitDataAvailable(theOther.fHasLegitDataAvailable)
{
}

NFmiAreaMask *NFmiInfoAreaMaskTimeDuration::Clone() const
{
  return new NFmiInfoAreaMaskTimeDuration(*this);
}

void NFmiInfoAreaMaskTimeDuration::InitializeFromArguments()
{
  itsSeekTimeInHours = itsArgumentVector[0];
  itsSeekTimeInMinutes = boost::math::iround(itsSeekTimeInHours * 60.f);
  fUseCumulativeCalculation = itsArgumentVector[1] != 0;
}

double NFmiInfoAreaMaskTimeDuration::Value(const NFmiCalculationParams &theCalculationParams,
                                           bool /* fUseTimeInterpolationAlways */)
{
  InitializeFromArguments();
  InitializeIntegrationValues();
  if (itsInfo->IsGrid())
    CalcValueFromGridData(theCalculationParams);
  else
    CalcValueFromObservationData(theCalculationParams);

  // Palautetaan kesto tunneissa
  if (fHasLegitDataAvailable)
    return itsCalculatedTimeDurationInMinutes / 60.f;
  else
    return kFloatMissing;
}

void NFmiInfoAreaMaskTimeDuration::InitializeIntegrationValues()
{
  itsCalculatedTimeDurationInMinutes = 0;
  fHasLegitDataAvailable = false;
}

static bool CalcTimeLoopIndexiesForTimeDuration(boost::shared_ptr<NFmiFastQueryInfo> &info,
                                                const NFmiCalculationParams &calculationParams,
                                                int seekTimeInMinutes,
                                                unsigned long *startTimeIndexOut,
                                                unsigned long *endTimeIndexOut)
{
  auto startTime = calculationParams.itsTime;
  auto endTime = startTime;
  endTime.ChangeByMinutes(seekTimeInMinutes);
  bool indexStatus = false;
  if (startTime < endTime)
    indexStatus = NFmiInfoAreaMask::CalcTimeLoopIndexies(
        info, calculationParams, startTime, endTime, startTimeIndexOut, endTimeIndexOut);
  else
    indexStatus = NFmiInfoAreaMask::CalcTimeLoopIndexies(
        info, calculationParams, endTime, startTime, startTimeIndexOut, endTimeIndexOut);

  // indeksit pitää kääntää, jos halutaan mennä taaksepäin ajassa, koska
  // NFmiInfoAreaMask::CalcTimeLoopIndexies -metodi laittaa indeksit 'nousevaan' järjestykseen
  if (startTime > endTime)
    std::swap(*startTimeIndexOut, *endTimeIndexOut);
  return indexStatus;
}

void NFmiInfoAreaMaskTimeDuration::CalcValueFromGridData(
    const NFmiCalculationParams &theCalculationParams)
{
  NFmiCalculationParams calculationParams = theCalculationParams;
  NFmiLocationCache locationCache = itsInfo->CalcLocationCache(calculationParams.UsedLatlon());
  if (!locationCache.NoValue())
  {
    CalcDurationTime(itsInfo, theCalculationParams);
  }
}

void NFmiInfoAreaMaskTimeDuration::CalcValueFromObservationData(
    const NFmiCalculationParams &theCalculationParams)
{
  size_t dataIndex = 0;
  unsigned long locationIndex = 0;
  if (FindClosestStationData(theCalculationParams, dataIndex, locationIndex))
  {
    auto &info = itsInfoVector[dataIndex];
    info->LocationIndex(locationIndex);
    theCalculationParams.itsCurrentMultiInfoData = info.get();
    CalcDurationTime(info, theCalculationParams);
  }
}

static int CalcTotalTimeDurationFromIntervalInMinutes(
    const std::pair<NFmiMetTime, NFmiMetTime> &timeInterval)
{
  if (timeInterval.first != NFmiMetTime::gMissingTime &&
      timeInterval.second != NFmiMetTime::gMissingTime)
    return std::abs(timeInterval.first.DifferenceInMinutes(timeInterval.second));
  else
    return 0;
}

static int CalcTotalTimeDurationFromIntervalsInMinutes(
    const std::vector<std::pair<NFmiMetTime, NFmiMetTime>> &totalTimeIntervals)
{
  int totalDurationInMinutes = 0;
  for (const auto &timeInterval : totalTimeIntervals)
  {
    totalDurationInMinutes += ::CalcTotalTimeDurationFromIntervalInMinutes(timeInterval);
  }
  return totalDurationInMinutes;
}

void NFmiInfoAreaMaskTimeDuration::CalcDurationTime(
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const NFmiCalculationParams &theCalculationParams)
{
  std::vector<std::pair<NFmiMetTime, NFmiMetTime>> totalTimeIntervals;
  std::pair<NFmiMetTime, NFmiMetTime> currentTimeInterval(NFmiMetTime::gMissingTime,
                                                          NFmiMetTime::gMissingTime);
  // Lasketaan aikaloopitus rajat
  unsigned long startTimeIndex = gMissingIndex;
  unsigned long endTimeIndex = gMissingIndex;
  if (::CalcTimeLoopIndexiesForTimeDuration(
          theInfo, theCalculationParams, itsSeekTimeInMinutes, &startTimeIndex, &endTimeIndex))
  {
    fHasLegitDataAvailable = true;
    int timeIndexIncrement = 1;
    bool reverserTime = false;
    if (startTimeIndex > endTimeIndex)
    {
      timeIndexIncrement = -1;
      reverserTime = true;
    }
    if (!fUseCumulativeCalculation)
      currentTimeInterval.first = theCalculationParams.itsTime;
    NFmiCalculationParams calculationParams = theCalculationParams;
    for (int timeIndex = startTimeIndex; reverserTime ? timeIndex >= static_cast<int>(endTimeIndex)
                                                      : timeIndex <= static_cast<int>(endTimeIndex);
         timeIndex += timeIndexIncrement)
    {
      theInfo->TimeIndex(timeIndex);
      calculationParams.itsTime = theInfo->Time();
      currentTimeInterval.second = calculationParams.itsTime;
      if (fUseCumulativeCalculation)
      {
        if (SimpleConditionCheck(calculationParams))
        {
          if (currentTimeInterval.first == NFmiMetTime::gMissingTime)
            currentTimeInterval.first = calculationParams.itsTime;
        }
        else
        {
          if (currentTimeInterval.first != NFmiMetTime::gMissingTime)
          {
            totalTimeIntervals.push_back(currentTimeInterval);
            currentTimeInterval = std::pair<NFmiMetTime, NFmiMetTime>(NFmiMetTime::gMissingTime,
                                                                      NFmiMetTime::gMissingTime);
          }
        }
      }
      else
      {
        if (!SimpleConditionCheck(calculationParams))
        {
          totalTimeIntervals.push_back(currentTimeInterval);
          currentTimeInterval = std::pair<NFmiMetTime, NFmiMetTime>(NFmiMetTime::gMissingTime,
                                                                    NFmiMetTime::gMissingTime);
          break;
        }
      }
    }
  }
  totalTimeIntervals.push_back(currentTimeInterval);
  itsCalculatedTimeDurationInMinutes =
      ::CalcTotalTimeDurationFromIntervalsInMinutes(totalTimeIntervals);
}

// **********************************************************
// *****    NFmiInfoAreaMaskTimeDuration    *****************
// **********************************************************

// **********************************************************
// *****  NFmiInfoAreaMaskTimeRangeSecondParValue  **********
// **********************************************************

NFmiInfoAreaMaskTimeRangeSecondParValue::~NFmiInfoAreaMaskTimeRangeSecondParValue() = default;

NFmiInfoAreaMaskTimeRangeSecondParValue::NFmiInfoAreaMaskTimeRangeSecondParValue(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const boost::shared_ptr<NFmiFastQueryInfo> &theSecondInfo,
    NFmiAreaMask::FunctionType theIntegrationFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskTimeRange(theOperation,
                                theMaskType,
                                theDataType,
                                theInfo,
                                theIntegrationFunc,
                                theArgumentCount,
                                thePossibleMetaParamId),
      itsFunctionModifierExtreme(),
      itsSecondInfo(theSecondInfo)
{
  itsFunctionModifierExtreme.reset(
      dynamic_cast<NFmiDataModifierExtreme *>(itsFunctionModifier->Clone()));
}

NFmiInfoAreaMaskTimeRangeSecondParValue::NFmiInfoAreaMaskTimeRangeSecondParValue(
    const NFmiInfoAreaMaskTimeRangeSecondParValue &theOther)
    : NFmiInfoAreaMaskTimeRange(theOther),
      itsFunctionModifierExtreme(theOther.itsFunctionModifierExtreme
                                     ? dynamic_cast<NFmiDataModifierExtreme *>(
                                           theOther.itsFunctionModifierExtreme->Clone())
                                     : nullptr),
      itsSecondInfo(NFmiAreaMask::DoShallowCopy(theOther.itsSecondInfo))
{
}

NFmiAreaMask *NFmiInfoAreaMaskTimeRangeSecondParValue::Clone() const
{
  return new NFmiInfoAreaMaskTimeRangeSecondParValue(*this);
}

double NFmiInfoAreaMaskTimeRangeSecondParValue::Value(
    const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways)
{
  if (!itsFunctionModifierExtreme || !itsSecondInfo)
  {
    throw std::runtime_error(
        "Error with get_2nd_par_value_from_extreme function: FunctionModifierExtreme or SecondInfo "
        "was null.");
  }
  // Äääriarvo pitää hakea ja laskea emoluokan Value metodilla, jotta saadaan
  // ääriarvon aika itsFunctionModifierExtreme oliolle.
  auto extremeValue =
      NFmiInfoAreaMaskTimeRange::Value(theCalculationParams, fUseTimeInterpolationAlways);
  return GetSecondParamValue(theCalculationParams);
}

double NFmiInfoAreaMaskTimeRangeSecondParValue::GetSecondParamValue(
    const NFmiCalculationParams &theCalculationParams)
{
  NFmiMetTime extremeTime(itsFunctionModifierExtreme->itsExtremeTime, 1);
  if (extremeTime != NFmiMetTime::gMissingTime)
  {
    // secondInfolle pitää vielä hanskata mahdollinen multi-data tapaus, eli
    // kyseessä havainto data ja ovat samalta tuottajalta, tällöin itsInfo:ssa on
    // aina oikea data, ja sitä pitää käyttää (theCalculationParams.itsCurrentMultiInfoData
    // mekaniikka ei näytä toimivan oikein).
    if (itsInfo != nullptr && !itsSecondInfo->IsGrid() &&
        *itsSecondInfo->Producer() == *itsInfo->Producer())
    {
      NFmiFastInfoUtils::QueryInfoTotalStateRestorer queryInfoTotalStateRestorer(*itsInfo);
      itsInfo->Param(itsSecondInfo->Param());
      return itsInfo->InterpolatedValue(theCalculationParams.UsedLatlon(), extremeTime);
    }
    else
    {
      return itsSecondInfo->InterpolatedValue(theCalculationParams.UsedLatlon(), extremeTime);
    }
  }
  return kFloatMissing;
}

void NFmiInfoAreaMaskTimeRangeSecondParValue::AddValueToModifier(
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    boost::shared_ptr<NFmiDataModifier> & /* theFunctionModifier */,
    float theValue)
{
  // In this virtual method the time of extreme value is stored.
  itsFunctionModifierExtreme->Calculate(theValue, theInfo.get());
}

void NFmiInfoAreaMaskTimeRangeSecondParValue::InitializeIntegrationValues()
{
  NFmiInfoAreaMaskTimeRange::InitializeIntegrationValues();
  itsFunctionModifierExtreme->Clear();
  itsFunctionModifierExtreme->itsExtremeTime = NFmiMetTime::gMissingTime;
}

// **********************************************************
// *****  NFmiInfoAreaMaskTimeRangeSecondParValue  **********
// **********************************************************
