
#include "NFmiInfoAreaMaskOccurrance.h"
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiDataModifier.h>
#include "NFmiDrawParam.h"
#include <newbase/NFmiProducerName.h>
#include "NFmiSmartInfo.h"
#include "NFmiSimpleCondition.h"

// **********************************************************
// *****    NFmiInfoAreaMaskOccurrance  *********************
// **********************************************************

std::function<void(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &,
                   boost::shared_ptr<NFmiDrawParam> &,
                   const boost::shared_ptr<NFmiArea> &)>
    NFmiInfoAreaMaskOccurrance::itsMultiSourceDataGetter;  // Alustetaan tyhjäksi ensin

NFmiInfoAreaMaskOccurrance::~NFmiInfoAreaMaskOccurrance(void)
{
}
NFmiInfoAreaMaskOccurrance::NFmiInfoAreaMaskOccurrance(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    const boost::shared_ptr<NFmiArea> &theCalculationArea,
    bool synopXCase,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskProbFunc(theOperation,
                               theMaskType,
                               theDataType,
                               theInfo,
                               thePrimaryFunc,
                               theSecondaryFunc,
                               theArgumentCount, 
                               thePossibleMetaParamId),
      fSynopXCase(synopXCase),
      fUseMultiSourceData(false),
      itsCalculationArea(theCalculationArea),
      itsInfoVector(),
      itsCalculatedLocationIndexies()
{
  fUseMultiSourceData = NFmiInfoAreaMaskOccurrance::IsKnownMultiSourceData(itsInfo);
}

NFmiInfoAreaMaskOccurrance::NFmiInfoAreaMaskOccurrance(const NFmiInfoAreaMaskOccurrance &theOther)
    : NFmiInfoAreaMaskProbFunc(theOther),
      fSynopXCase(theOther.fSynopXCase),
      fUseMultiSourceData(theOther.fUseMultiSourceData),
      itsCalculationArea(theOther.itsCalculationArea),
      itsInfoVector(),
      itsCalculatedLocationIndexies(theOther.itsCalculatedLocationIndexies)
{
    itsInfoVector = NFmiInfoAreaMaskOccurrance::CreateShallowCopyOfInfoVector(theOther.itsInfoVector);
}

NFmiAreaMask *NFmiInfoAreaMaskOccurrance::Clone(void) const
{
  return new NFmiInfoAreaMaskOccurrance(*this);
}

void NFmiInfoAreaMaskOccurrance::SetMultiSourceDataGetterCallback(
    const std::function<void(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &,
                             boost::shared_ptr<NFmiDrawParam> &,
                             const boost::shared_ptr<NFmiArea> &)> &theCallbackFunction)
{
  NFmiInfoAreaMaskOccurrance::itsMultiSourceDataGetter = theCallbackFunction;
}

checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> NFmiInfoAreaMaskOccurrance::GetMultiSourceData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiArea> &calculationArea, bool getSynopXData)
{
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> infoVector;
    boost::shared_ptr<NFmiDrawParam> drawParam(
        new NFmiDrawParam(theInfo->Param(), *theInfo->Level(), 0, theInfo->DataType()));
    if(getSynopXData)
        drawParam->Param().GetProducer()->SetIdent(NFmiInfoData::kFmiSpSynoXProducer);
    itsMultiSourceDataGetter(infoVector, drawParam, calculationArea);
    return infoVector;
}

checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> NFmiInfoAreaMaskOccurrance::CreateShallowCopyOfInfoVector(const checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &infoVector)
{
    // tehdään matala kopio info-vektorista
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> shallowCopyVector;
    for(const auto & info : infoVector)
        shallowCopyVector.push_back(
            NFmiSmartInfo::CreateShallowCopyOfHighestInfo(info));
    return shallowCopyVector;
}

void NFmiInfoAreaMaskOccurrance::Initialize(void)
{
  // cachejen alustuksia tehdään vain asemadatoille. Hila datat hanskataan emoluokassa ja sitä en
  // lähde tässä vielä optinmoimaan.
  if (!fUseMultiSourceData)
    itsInfoVector.push_back(itsInfo);
  else
  {
      itsInfoVector = NFmiInfoAreaMaskOccurrance::GetMultiSourceData(itsInfo, itsCalculationArea, fSynopXCase);
  }
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

// Nyt synop ja salama datat ovat tälläisiä. Tämä on yritys tehdä vähän optimointia muutenkin jo
// pirun raskaaseen koodiin.
// HUOM! Tämä on riippuvainen NFmiEditMapGeneralDataDoc::MakeDrawedInfoVectorForMapView -metodin
// erikoistapauksista.
bool NFmiInfoAreaMaskOccurrance::IsKnownMultiSourceData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  if (theInfo)
  {
    if (theInfo->DataType() == NFmiInfoData::kFlashData)
      return true;
    // HUOM! kaikkien synop datojen käyttö on aivan liian hidasta, käytetään vain primääri synop
    // dataa laskuissa.
    if (theInfo->Producer()->GetIdent() == kFmiSYNOP)
      return true;
  }
  return false;
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
    NFmiLocation location(theCalculationParams.itsLatlon);
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
    if(NFmiInfoAreaMaskProbFunc::CheckTimeIndicesForLoopBreak(startTimeIndex, endTimeIndex))
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

NFmiInfoAreaMaskOccurranceSimpleCondition::~NFmiInfoAreaMaskOccurranceSimpleCondition(void) = default;

NFmiInfoAreaMaskOccurranceSimpleCondition::NFmiInfoAreaMaskOccurranceSimpleCondition(const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    const boost::shared_ptr<NFmiArea> &theCalculationArea,
    bool synopXCase,
    unsigned long thePossibleMetaParamId)
    :NFmiInfoAreaMaskOccurrance(theOperation,
        theMaskType,
        theDataType,
        theInfo,
        thePrimaryFunc,
        theSecondaryFunc,
        theArgumentCount,
        theCalculationArea,
        synopXCase,
        thePossibleMetaParamId)
{}

NFmiInfoAreaMaskOccurranceSimpleCondition::NFmiInfoAreaMaskOccurranceSimpleCondition(const NFmiInfoAreaMaskOccurranceSimpleCondition &theOther)
    :NFmiInfoAreaMaskOccurrance(theOther)
{}

NFmiAreaMask* NFmiInfoAreaMaskOccurranceSimpleCondition::Clone(void) const
{
    return new NFmiInfoAreaMaskOccurranceSimpleCondition(*this);
}

double NFmiInfoAreaMaskOccurranceSimpleCondition::Value(const NFmiCalculationParams &theCalculationParams,
    bool fUseTimeInterpolationAlways)
{
    // Nämä occurance tetit tehdään erilailla riippuen onko kyse hila vai asema datasta
    if(IsGridData())
    {
        return NFmiInfoAreaMaskOccurrance::Value(theCalculationParams, fUseTimeInterpolationAlways);
    }
    else
        throw std::runtime_error("Internal error in program: observation data given to occurance with simple-condition calculations");
}

float NFmiInfoAreaMaskOccurranceSimpleCondition::CalculationPointValue(int theOffsetX, int theOffsetY, const NFmiMetTime &theInterpolationTime, bool useInterpolatedTime)
{
    // Jos päästään tänne asti kasvatetaan itsTotalCalculatedGridPoints -laskuri.
    // Tänne pääsy tarkoittaa että laskenta piste on datan alueen sisällä ja laskenta ympyrän alueella
    itsTotalCalculatedGridPoints++;
    return NFmiInfoAreaMaskProbFunc::CalculationPointValue(theOffsetX, theOffsetY, theInterpolationTime, useInterpolatedTime);
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
    if(distanceInKM > itsSearchRangeInKM)
        return;  // kyseinen piste oli ympyrän ulkopuolella

    float value = theInfo->FloatValue();
    NFmiCalculationParams calculationParams(theInfo->LatLon(), theInfo->LocationIndex(), theInfo->Time(), theInfo->TimeIndex());
    if(SimpleConditionCheck(calculationParams))
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
    double observationRadiusInKm,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMask(
        NFmiCalculationCondition(), theMaskType, theDataType, theInfo, thePossibleMetaParamId, NFmiAreaMask::kNoValue),
    itsTimeOffsetInMinutes(0),
    fUseMultiSourceData(false),
    itsInfoVector(),
    itsObservationRadiusInKm(observationRadiusInKm)
{
    itsFunctionArgumentCount = theArgumentCount;
    fUseMultiSourceData = NFmiInfoAreaMaskOccurrance::IsKnownMultiSourceData(itsInfo);
}

NFmiPeekTimeMask::~NFmiPeekTimeMask(void)
{
}

NFmiPeekTimeMask::NFmiPeekTimeMask(const NFmiPeekTimeMask &theOther)
    : NFmiInfoAreaMask(theOther), 
    itsTimeOffsetInMinutes(theOther.itsTimeOffsetInMinutes),
    fUseMultiSourceData(theOther.fUseMultiSourceData),
    itsInfoVector(),
    itsObservationRadiusInKm(theOther.itsObservationRadiusInKm)
{
    itsInfoVector = NFmiInfoAreaMaskOccurrance::CreateShallowCopyOfInfoVector(theOther.itsInfoVector);
}

NFmiAreaMask *NFmiPeekTimeMask::Clone(void) const
{
    return new NFmiPeekTimeMask(*this);
}

void NFmiPeekTimeMask::Initialize(void)
{
    if(fUseMultiSourceData)
    {
        boost::shared_ptr<NFmiArea> dummyArea;
        bool getOnlyStationarySynopData = true;
        itsInfoVector = NFmiInfoAreaMaskOccurrance::GetMultiSourceData(itsInfo, dummyArea, getOnlyStationarySynopData);
    }
    else
        itsInfoVector.push_back(itsInfo);
}

double NFmiPeekTimeMask::Value(const NFmiCalculationParams &theCalculationParams,
    bool fUseTimeInterpolationAlways)
{
    NFmiMetTime peekTime(theCalculationParams.itsTime);
    peekTime.ChangeByMinutes(itsTimeOffsetInMinutes);
    if(itsInfo->IsGrid())
    {
        NFmiCalculationParams peekCalculationParams(theCalculationParams);
        peekCalculationParams.itsTime = peekTime;
        return NFmiInfoAreaMask::Value(peekCalculationParams, fUseTimeInterpolationAlways);
    }
    else
        return CalcValueFromObservation(theCalculationParams.itsLatlon, peekTime);
}

static double GetSearchRadiusInMetres(double observationRadiusInKm)
{
    if(observationRadiusInKm == kFloatMissing)
        return kFloatMissing * 1000.; // Tämä on rajaton etsintä NFmiFastInfo::NearestLocation metodissa
    else
        return observationRadiusInKm * 1000.;
}


double NFmiPeekTimeMask::CalcValueFromObservation(const NFmiPoint &theLatlon, const NFmiMetTime &thePeekTime)
{
    NFmiLocation wantedLocation(theLatlon);
    double valueFromMinDistance = kFloatMissing;
    double minDistanceInMetres = 99999999999;
    double searchRadiusInMetres = ::GetSearchRadiusInMetres(itsObservationRadiusInKm);
    for(auto &info : itsInfoVector)
    {
        // Datasta pitää löytyä etsitty aika suoraan ilman aikainterpolaatioita
        if(info->Time(thePeekTime))
        {
            if(info->NearestLocation(wantedLocation, searchRadiusInMetres))
            {
                double currentDistanceInMetres = wantedLocation.Distance(info->LatLon());
                if(currentDistanceInMetres < minDistanceInMetres)
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
    if(static_cast<long>(theArgumentVector.size()) == itsFunctionArgumentCount - 1)
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
    double observationRadiusInKm,
    unsigned long thePossibleMetaParamId)
    : NFmiPeekTimeMask(theMaskType, theDataType, theInfo, theArgumentCount, observationRadiusInKm, thePossibleMetaParamId),
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
    if(static_cast<int>(itsArgumentVector.size()) !=
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
    if(itsInfo->IsGrid())
        CalcValueFromGridData(theCalculationParams);
    else
        CalcValueFromObservationData(theCalculationParams);

    return itsFunctionModifier->CalculationResult();
}

void NFmiInfoAreaMaskTimeRange::InitializeIntegrationValues()
{
    itsFunctionModifier->Clear();
}

void NFmiInfoAreaMaskTimeRange::CalcValueFromGridData(const NFmiCalculationParams &theCalculationParams)
{
    NFmiCalculationParams calculationParams = theCalculationParams;
    NFmiLocationCache locationCache = itsInfo->CalcLocationCache(calculationParams.itsLatlon);
    if(!locationCache.NoValue())
    {
        // Lasketaan aikaloopitus rajat
        unsigned long startTimeIndex = gMissingIndex;
        unsigned long endTimeIndex = gMissingIndex;
        if(NFmiInfoAreaMask::CalcTimeLoopIndexies(itsInfo,
            calculationParams,
            itsStartTimeOffsetInHours,
            itsEndTimeOffsetInHours,
            &startTimeIndex,
            &endTimeIndex))
        {
            DoTimeLoopCalculationsForGridData(startTimeIndex, endTimeIndex, locationCache, calculationParams);
        }
    }
}

void NFmiInfoAreaMaskTimeRange::DoTimeLoopCalculationsForGridData(unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, const NFmiLocationCache &theLocationCache, NFmiCalculationParams &theCalculationParams)
{
    for(unsigned long timeIndex = theStartTimeIndex; timeIndex <= theEndTimeIndex; timeIndex++)
    {
        itsInfo->TimeIndex(timeIndex);
        if(itsSimpleCondition)
            theCalculationParams.itsTime = itsInfo->Time();
        if(SimpleConditionCheck(theCalculationParams))
            AddValuesToFunctionModifier(itsInfo, itsFunctionModifier, theLocationCache, itsIntegrationFunc);
    }
}

static bool FindClosestStationData(const checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &infoVector, const NFmiPoint &latlon, double observationRadiusInKm, size_t &dataIndexOut, unsigned long &locationIndexOut)
{
    NFmiLocation wantedLocation(latlon);
    double minDistanceInMetres = 99999999999;
    double searchRadiusInMetres = ::GetSearchRadiusInMetres(observationRadiusInKm);
    for(size_t dataCounter = 0; dataCounter < infoVector.size(); dataCounter++)
    {
        const auto &info = infoVector[dataCounter];
        if(info->NearestLocation(latlon, searchRadiusInMetres))
        {
            double currentDistanceInMetres = wantedLocation.Distance(info->LatLon());
            if(currentDistanceInMetres < minDistanceInMetres)
            {
                minDistanceInMetres = currentDistanceInMetres;
                dataIndexOut = dataCounter;
                locationIndexOut = info->LocationIndex();
            }
        }
    }
    return minDistanceInMetres <= searchRadiusInMetres;
}

void NFmiInfoAreaMaskTimeRange::CalcValueFromObservationData(const NFmiCalculationParams &theCalculationParams)
{
    NFmiCalculationParams calculationParams = theCalculationParams;
    size_t dataIndex = 0;
    unsigned long locationIndex = 0;
    if(::FindClosestStationData(itsInfoVector, calculationParams.itsLatlon, itsObservationRadiusInKm, dataIndex, locationIndex))
    {
        auto &info = itsInfoVector[dataIndex];
        info->LocationIndex(locationIndex);

        unsigned long startTimeIndex = 0;
        unsigned long endTimeIndex = 0;
        if(NFmiInfoAreaMask::CalcTimeLoopIndexies(info,
            calculationParams,
            itsStartTimeOffsetInHours,
            itsEndTimeOffsetInHours,
            &startTimeIndex,
            &endTimeIndex))
        {
            DoTimeLoopCalculationsForObservationData(info, startTimeIndex, endTimeIndex, calculationParams);
        }
    }
}

void NFmiInfoAreaMaskTimeRange::DoTimeLoopCalculationsForObservationData(boost::shared_ptr<NFmiFastQueryInfo> &info, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, NFmiCalculationParams &theCalculationParams)
{
    for(unsigned long timeIndex = theStartTimeIndex; timeIndex <= theEndTimeIndex; timeIndex++)
    {
        info->TimeIndex(timeIndex);
        if(itsSimpleCondition)
            theCalculationParams.itsTime = info->Time();
        if(SimpleConditionCheck(theCalculationParams))
            itsFunctionModifier->Calculate(info->FloatValue());
    }
}

// **********************************************************
// *****    NFmiInfoAreaMaskTimeRange  **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskPreviousFullDays  ***************
// **********************************************************

NFmiInfoAreaMaskPreviousFullDays::~NFmiInfoAreaMaskPreviousFullDays(void) = default;
NFmiInfoAreaMaskPreviousFullDays::NFmiInfoAreaMaskPreviousFullDays(const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType theIntegrationFunc,
    int theArgumentCount,
    double observationRadiusInKm,
    unsigned long thePossibleMetaParamId)
    :NFmiInfoAreaMaskTimeRange(theOperation,
        theMaskType,
        theDataType,
        theInfo,
        theIntegrationFunc,
        theArgumentCount,
        observationRadiusInKm,
        thePossibleMetaParamId)
    , itsPreviousDayCount(0)
{}

NFmiInfoAreaMaskPreviousFullDays::NFmiInfoAreaMaskPreviousFullDays(const NFmiInfoAreaMaskPreviousFullDays &theOther)
    :NFmiInfoAreaMaskTimeRange(theOther)
    , itsPreviousDayCount(theOther.itsPreviousDayCount)
{}

NFmiAreaMask* NFmiInfoAreaMaskPreviousFullDays::Clone(void) const
{
    return new NFmiInfoAreaMaskPreviousFullDays(*this);
}

void NFmiInfoAreaMaskPreviousFullDays::InitializeFromArguments(void)
{
    itsPreviousDayCount = static_cast<int>(itsArgumentVector[0]);
}

static bool CalcTimeLoopIndexiesForPreviousFullDays(boost::shared_ptr<NFmiFastQueryInfo> &info,
    const NFmiCalculationParams &calculationParams,
    int previousDayCount,
    unsigned long *startTimeIndexOut,
    unsigned long *endTimeIndexOut)
{
    if(previousDayCount <= 0)
        return false;
    auto localCalculationTime = calculationParams.itsTime.LocalTime();
    auto localHour = localCalculationTime.GetHour();
    // Jos localHour on 0, lasketaan täydet päivät taaksepäin, jos > 0, lasketaan täysiä päiviä yksi vähemmän.
    if(localHour > 0)
        previousDayCount--;
    auto hourDifference = localHour + (previousDayCount * 24);
    NFmiMetTime startTime(calculationParams.itsTime);
    startTime.ChangeByHours(-hourDifference);
    return NFmiInfoAreaMask::CalcTimeLoopIndexies(info, calculationParams, startTime, calculationParams.itsTime, startTimeIndexOut, endTimeIndexOut);
}


void NFmiInfoAreaMaskPreviousFullDays::CalcValueFromGridData(const NFmiCalculationParams &theCalculationParams)
{
    NFmiCalculationParams calculationParams = theCalculationParams;
    NFmiLocationCache locationCache = itsInfo->CalcLocationCache(calculationParams.itsLatlon);
    if(!locationCache.NoValue())
    {
        // Lasketaan aikaloopitus rajat
        unsigned long startTimeIndex = gMissingIndex;
        unsigned long endTimeIndex = gMissingIndex;
        if(::CalcTimeLoopIndexiesForPreviousFullDays(itsInfo,
            calculationParams,
            itsPreviousDayCount,
            &startTimeIndex,
            &endTimeIndex))
        {
            DoTimeLoopCalculationsForGridData(startTimeIndex, endTimeIndex, locationCache, calculationParams);
        }
    }
}

void NFmiInfoAreaMaskPreviousFullDays::CalcValueFromObservationData(const NFmiCalculationParams &theCalculationParams)
{
    NFmiCalculationParams calculationParams = theCalculationParams;
    size_t dataIndex = 0;
    unsigned long locationIndex = 0;
    if(::FindClosestStationData(itsInfoVector, calculationParams.itsLatlon, itsObservationRadiusInKm, dataIndex, locationIndex))
    {
        auto &info = itsInfoVector[dataIndex];
        info->LocationIndex(locationIndex);

        unsigned long startTimeIndex = 0;
        unsigned long endTimeIndex = 0;
        if(::CalcTimeLoopIndexiesForPreviousFullDays(info,
            calculationParams,
            itsPreviousDayCount,
            &startTimeIndex,
            &endTimeIndex))
        {
            DoTimeLoopCalculationsForObservationData(info, startTimeIndex, endTimeIndex, calculationParams);
        }
    }
}

// **********************************************************
// *****    NFmiInfoAreaMaskPreviousFullDays  ***************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskTimeDuration    *****************
// **********************************************************

NFmiInfoAreaMaskTimeDuration::~NFmiInfoAreaMaskTimeDuration(void) = default;

NFmiInfoAreaMaskTimeDuration::NFmiInfoAreaMaskTimeDuration(const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    int theArgumentCount,
    double observationRadiusInKm,
    unsigned long thePossibleMetaParamId)
    :NFmiInfoAreaMaskTimeRange(theOperation,
        theMaskType,
        theDataType,
        theInfo,
        NFmiAreaMask::Min, // tähän on vain pakko antaa joku integraatio funktio, vaikka sitä ei käytetäkään tässä luokassa
        theArgumentCount,
        observationRadiusInKm,
        thePossibleMetaParamId)
    ,itsSeekTimeInHours(0)
    ,itsSeekTimeInMinutes(0)
    ,itsCalculatedTimeDurationInMinutes(0)
    ,fUseCumulativeCalculation(true)
{}

NFmiInfoAreaMaskTimeDuration::NFmiInfoAreaMaskTimeDuration(const NFmiInfoAreaMaskTimeDuration &theOther)
    :NFmiInfoAreaMaskTimeRange(theOther)
    , itsSeekTimeInHours(theOther.itsSeekTimeInHours)
    , itsSeekTimeInMinutes(theOther.itsSeekTimeInMinutes)
    , itsCalculatedTimeDurationInMinutes(theOther.itsCalculatedTimeDurationInMinutes)
    , fUseCumulativeCalculation(theOther.fUseCumulativeCalculation)
{}

NFmiAreaMask* NFmiInfoAreaMaskTimeDuration::Clone(void) const
{
    return new NFmiInfoAreaMaskTimeDuration(*this);
}

void NFmiInfoAreaMaskTimeDuration::InitializeFromArguments(void)
{
    itsSeekTimeInHours = itsArgumentVector[0];
    itsSeekTimeInMinutes = boost::math::iround(itsSeekTimeInHours * 60.f);
    fUseCumulativeCalculation = itsArgumentVector[1] != 0;
}

double NFmiInfoAreaMaskTimeDuration::Value(const NFmiCalculationParams &theCalculationParams, bool /* fUseTimeInterpolationAlways */)
{
    InitializeFromArguments();
    InitializeIntegrationValues();
    if(itsInfo->IsGrid())
        CalcValueFromGridData(theCalculationParams);
    else
        CalcValueFromObservationData(theCalculationParams);

    // Palautetaan kesto tunneissa
    return itsCalculatedTimeDurationInMinutes / 60.f;
}

void NFmiInfoAreaMaskTimeDuration::InitializeIntegrationValues()
{
    itsCalculatedTimeDurationInMinutes = 0;
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
    if(startTime < endTime)
        indexStatus = NFmiInfoAreaMask::CalcTimeLoopIndexies(info, calculationParams, startTime, endTime, startTimeIndexOut, endTimeIndexOut);
    else
        indexStatus = NFmiInfoAreaMask::CalcTimeLoopIndexies(info, calculationParams, endTime, startTime, startTimeIndexOut, endTimeIndexOut);

    // indeksit pitää kääntää, jos halutaan mennä taaksepäin ajassa, koska NFmiInfoAreaMask::CalcTimeLoopIndexies -metodi laittaa indeksit 'nousevaan' järjestykseen
    if(startTime > endTime)
        std::swap(*startTimeIndexOut, *endTimeIndexOut);
    return indexStatus;
}

void NFmiInfoAreaMaskTimeDuration::CalcValueFromGridData(const NFmiCalculationParams &theCalculationParams)
{
    NFmiCalculationParams calculationParams = theCalculationParams;
    NFmiLocationCache locationCache = itsInfo->CalcLocationCache(calculationParams.itsLatlon);
    if(!locationCache.NoValue())
    {
        CalcDurationTime(itsInfo, theCalculationParams);
    }
}

void NFmiInfoAreaMaskTimeDuration::CalcValueFromObservationData(const NFmiCalculationParams &theCalculationParams)
{
    size_t dataIndex = 0;
    unsigned long locationIndex = 0;
    if(::FindClosestStationData(itsInfoVector, theCalculationParams.itsLatlon, itsObservationRadiusInKm, dataIndex, locationIndex))
    {
        auto &info = itsInfoVector[dataIndex];
        info->LocationIndex(locationIndex);
        CalcDurationTime(info, theCalculationParams);
    }
}

static int CalcTotalTimeDurationFromIntervalInMinutes(const std::pair<NFmiMetTime, NFmiMetTime> &timeInterval)
{
    if(timeInterval.first != NFmiMetTime::gMissingTime && timeInterval.second != NFmiMetTime::gMissingTime)
        return std::abs(timeInterval.first.DifferenceInMinutes(timeInterval.second));
    else
        return 0;
}

static int CalcTotalTimeDurationFromIntervalsInMinutes(const std::vector<std::pair<NFmiMetTime, NFmiMetTime>> &totalTimeIntervals)
{
    int totalDurationInMinutes = 0;
    for(const auto &timeInterval : totalTimeIntervals)
    {
        totalDurationInMinutes += ::CalcTotalTimeDurationFromIntervalInMinutes(timeInterval);
    }
    return totalDurationInMinutes;
}

void NFmiInfoAreaMaskTimeDuration::CalcDurationTime(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiCalculationParams &theCalculationParams)
{
    std::vector<std::pair<NFmiMetTime, NFmiMetTime>> totalTimeIntervals;
    std::pair<NFmiMetTime, NFmiMetTime> currentTimeInterval(NFmiMetTime::gMissingTime, NFmiMetTime::gMissingTime);
    // Lasketaan aikaloopitus rajat
    unsigned long startTimeIndex = gMissingIndex;
    unsigned long endTimeIndex = gMissingIndex;
    if(::CalcTimeLoopIndexiesForTimeDuration(theInfo,
        theCalculationParams,
        itsSeekTimeInMinutes,
        &startTimeIndex,
        &endTimeIndex))
    {
        int timeIndexIncrement = 1;
        bool reverserTime = false;
        if(startTimeIndex > endTimeIndex)
        {
            timeIndexIncrement = -1;
            reverserTime = true;
        }
        if(!fUseCumulativeCalculation)
            currentTimeInterval.first = theCalculationParams.itsTime;
        NFmiCalculationParams calculationParams = theCalculationParams;
        for(int timeIndex = startTimeIndex;
            reverserTime ? timeIndex >= static_cast<int>(endTimeIndex) : timeIndex <= static_cast<int>(endTimeIndex);
            timeIndex += timeIndexIncrement)
        {
            theInfo->TimeIndex(timeIndex);
            calculationParams.itsTime = theInfo->Time();
            currentTimeInterval.second = calculationParams.itsTime;
            if(fUseCumulativeCalculation)
            {
                if(SimpleConditionCheck(calculationParams))
                {
                    if(currentTimeInterval.first == NFmiMetTime::gMissingTime)
                        currentTimeInterval.first = calculationParams.itsTime;
                }
                else
                {
                    if(currentTimeInterval.first != NFmiMetTime::gMissingTime)
                    {
                        totalTimeIntervals.push_back(currentTimeInterval);
                        currentTimeInterval = std::pair<NFmiMetTime, NFmiMetTime>(NFmiMetTime::gMissingTime, NFmiMetTime::gMissingTime);
                    }
                }
            }
            else
            {
                if(!SimpleConditionCheck(calculationParams))
                {
                    totalTimeIntervals.push_back(currentTimeInterval);
                    currentTimeInterval = std::pair<NFmiMetTime, NFmiMetTime>(NFmiMetTime::gMissingTime, NFmiMetTime::gMissingTime);
                    break;
                }
            }
        }
    }
    totalTimeIntervals.push_back(currentTimeInterval);
    itsCalculatedTimeDurationInMinutes = ::CalcTotalTimeDurationFromIntervalsInMinutes(totalTimeIntervals);
}

// **********************************************************
// *****    NFmiInfoAreaMaskTimeDuration    *****************
// **********************************************************
