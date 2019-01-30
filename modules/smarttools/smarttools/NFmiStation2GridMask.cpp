#include "NFmiStation2GridMask.h"
#include "NFmiDrawParam.h"
#include "NFmiGriddingHelperInterface.h"
#include "NFmiGriddingProperties.h"
#include <newbase/NFmiFastInfoUtils.h>
#include <newbase/NFmiFastQueryInfo.h>

#include <boost/math/special_functions/round.hpp>

// ****************************************************************************
// ****************** NFmiStation2GridMask ************************************
// ****************************************************************************

NFmiStation2GridMask::GriddingFunctionCallBackType NFmiStation2GridMask::itsGridStationDataCallback;

NFmiStation2GridMask::NFmiStation2GridMask(Type theMaskType,
                                           NFmiInfoData::Type theDataType,
                                           boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                           unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMask(
          NFmiCalculationCondition(), theMaskType, theDataType, theInfo, thePossibleMetaParamId, NFmiAreaMask::kNoValue),
      itsGriddedStationData(new DataCache()),
      itsCurrentGriddedStationData(0),
      itsLastCalculatedTime(NFmiMetTime::gMissingTime),
      itsAreaPtr(),
      itsGriddingHelper(0),
      itsStation2GridSize(1, 1),
      itsObservationRadiusInKm(kFloatMissing),
      itsCacheMutex(new MutexType())
{
}

NFmiStation2GridMask::~NFmiStation2GridMask(void)
{
}

NFmiStation2GridMask::NFmiStation2GridMask(const NFmiStation2GridMask &theOther)
    : NFmiInfoAreaMask(theOther),
      itsGriddedStationData(theOther.itsGriddedStationData),
      itsCurrentGriddedStationData(0)  // tämä laitetaan aina 0:ksi
      ,
      itsLastCalculatedTime(theOther.itsLastCalculatedTime),
      itsAreaPtr(theOther.itsAreaPtr.get() ? theOther.itsAreaPtr.get()->Clone() : 0),
      itsGriddingHelper(theOther.itsGriddingHelper),
      itsStation2GridSize(theOther.itsStation2GridSize),
      itsObservationRadiusInKm(theOther.itsObservationRadiusInKm),
      itsCacheMutex(theOther.itsCacheMutex)
{
}

NFmiAreaMask *NFmiStation2GridMask::Clone(void) const
{
  return new NFmiStation2GridMask(*this);
}

double NFmiStation2GridMask::Value(const NFmiCalculationParams &theCalculationParams,
                                   bool /* fUseTimeInterpolationAlways */)
{
  DoGriddingCheck(theCalculationParams);
  if (itsCurrentGriddedStationData)
    return itsCurrentGriddedStationData->GetValue(theCalculationParams.itsLocationIndex,
                                                  kFloatMissing);
  else
    return kFloatMissing;
}

void NFmiStation2GridMask::SetGriddingHelpers(NFmiArea *theArea,
                                              NFmiGriddingHelperInterface *theGriddingHelper,
                                              const NFmiPoint &theStation2GridSize,
                                              float theObservationRadiusInKm)
{
  itsAreaPtr.reset(theArea->Clone());
  itsGriddingHelper = theGriddingHelper;
  itsStation2GridSize = theStation2GridSize;
  itsObservationRadiusInKm = theObservationRadiusInKm;
}

void NFmiStation2GridMask::DoGriddingCheck(const NFmiCalculationParams &theCalculationParams)
{
  if (itsLastCalculatedTime != theCalculationParams.itsTime)
  {
    WriteLock lock(*itsCacheMutex);  // tästä saa edetä vain yksi säie kerrallaan, eli joku joka
                                     // ehtii, laskee cache-matriisin ja asettaa sen
                                     // itsCurrentGriddedStationData:n arvoksi, muut sitten vain
                                     // käyttävät sitä

    // katsotaanko löytyykö valmiiksi laskettua hilaa halutulle ajalle
    DataCache::iterator it = itsGriddedStationData->find(theCalculationParams.itsTime);
    if (it != itsGriddedStationData->end())
      itsCurrentGriddedStationData = &((*it).second);
    else
    {
      // lasketaan halutun ajan hila
      if (itsGriddingHelper && itsAreaPtr.get())
      {
        boost::shared_ptr<NFmiDrawParam> drawParam(
            new NFmiDrawParam(itsDataIdent, itsLevel, 0, itsDataType));
        NFmiDataMatrix<float> griddedData(
            static_cast<NFmiDataMatrix<float>::size_type>(itsStation2GridSize.X()),
            static_cast<NFmiDataMatrix<float>::size_type>(itsStation2GridSize.Y()),
            kFloatMissing);
        if(itsGridStationDataCallback)
        {
            auto griddingProperties = itsGriddingHelper->GriddingProperties(false);
            griddingProperties.rangeLimitInKm(itsObservationRadiusInKm); // overridataan käytetty km-rajoitin arvosta, joka saadaan macroParam kaavasta
            itsGridStationDataCallback(itsGriddingHelper,
                itsAreaPtr,
                drawParam,
                griddedData,
                theCalculationParams.itsTime,
                griddingProperties);
        }
        std::pair<DataCache::iterator, bool> insertResult = itsGriddedStationData->insert(
            std::make_pair(theCalculationParams.itsTime, griddedData));
        if (insertResult.second)
          itsCurrentGriddedStationData = &((*insertResult.first).second);
        else
          itsCurrentGriddedStationData = 0;
      }
    }
    itsLastCalculatedTime = theCalculationParams.itsTime;
  }
}
// ****************************************************************************
// ****************** NFmiStation2GridMask ************************************
// ****************************************************************************

// ****************************************************************************
// *************** NFmiNearestObsValue2GridMask *******************************
// ****************************************************************************

NFmiNearestObsValue2GridMask::NFmiNearestObsValue2GridMask(
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMask(
          NFmiCalculationCondition(), theMaskType, theDataType, theInfo, thePossibleMetaParamId, NFmiAreaMask::kNoValue),
      itsNearestObsValuesData(new DataCache()),
      itsCurrentNearestObsValuesData(0),
      itsLastCalculatedTime(NFmiMetTime::gMissingTime),
      itsAreaPtr(),
      itsGriddingHelper(0),
      itsResultGridSize(1, 1),
      itsCacheMutex(new MutexType())
{
  itsFunctionArgumentCount = theArgumentCount;
}

NFmiNearestObsValue2GridMask::~NFmiNearestObsValue2GridMask(void)
{
}

NFmiNearestObsValue2GridMask::NFmiNearestObsValue2GridMask(
    const NFmiNearestObsValue2GridMask &theOther)
    : NFmiInfoAreaMask(theOther),
      itsNearestObsValuesData(theOther.itsNearestObsValuesData),
      itsCurrentNearestObsValuesData(0),  // tämä laitetaan aina 0:ksi
      itsLastCalculatedTime(theOther.itsLastCalculatedTime),
      itsAreaPtr(theOther.itsAreaPtr.get() ? theOther.itsAreaPtr.get()->Clone() : 0),
      itsGriddingHelper(theOther.itsGriddingHelper),
      itsResultGridSize(theOther.itsResultGridSize),
      itsCacheMutex(theOther.itsCacheMutex)
{
}

NFmiAreaMask *NFmiNearestObsValue2GridMask::Clone(void) const
{
  return new NFmiNearestObsValue2GridMask(*this);
}

double NFmiNearestObsValue2GridMask::Value(const NFmiCalculationParams &theCalculationParams,
                                           bool /* fUseTimeInterpolationAlways */)
{
  DoNearestValueGriddingCheck(theCalculationParams);
  if (itsCurrentNearestObsValuesData)
    return itsCurrentNearestObsValuesData->GetValue(theCalculationParams.itsLocationIndex,
                                                    kFloatMissing);
  else
    return kFloatMissing;
}

void NFmiNearestObsValue2GridMask::SetGriddingHelpers(
    NFmiArea *theArea,
    NFmiGriddingHelperInterface *theGriddingHelper,
    const NFmiPoint &theResultGridSize)
{
  itsAreaPtr.reset(theArea->Clone());
  itsGriddingHelper = theGriddingHelper;
  itsResultGridSize = theResultGridSize;
}

void NFmiNearestObsValue2GridMask::SetArguments(std::vector<float> &theArgumentVector)
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

static NFmiDataMatrix<float> CalcNearestValueMatrix(
    const NFmiLevel &theLevel,
    const NFmiCalculationParams &theCalculationParams,
    const NFmiPoint &theResultGridSize,
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector,
    boost::shared_ptr<NFmiArea> &theAreaPtr,
    float theTimePeekInHours)
{
  // Luodaan tulos matriisi täytettynä puuttuvilla arvoilla
  NFmiDataMatrix<float> nearestValueMatrix(
      static_cast<NFmiDataMatrix<float>::size_type>(theResultGridSize.X()),
      static_cast<NFmiDataMatrix<float>::size_type>(theResultGridSize.Y()),
      kFloatMissing);
  if (theAreaPtr && theInfoVector.size())
  {
    const double kMissingDistanceValue =
        9999999999.;  // etäisyys matriisi alustetaan tällä suurella luvulla, jolloin aina riittää
                      // yksi < -vertailu, kun etsitään lähintä asemaa
    NFmiDataMatrix<double> distanceMatrix(
        nearestValueMatrix.NX(), nearestValueMatrix.NY(), kMissingDistanceValue);
    NFmiGrid grid(theAreaPtr.get(),
                  static_cast<unsigned long>(nearestValueMatrix.NX()),
                  static_cast<unsigned long>(nearestValueMatrix.NY()));
    for (auto infoIter : theInfoVector)
    {
      // data ei saa olla hiladataa, eikä ns. laivadataa (lokaatio muuttuu ajan myötä ja lat/lon
      // arvot ovat erillisiä parametreja)
      if (!infoIter->IsGrid() && !NFmiFastInfoUtils::IsInfoShipTypeData(*infoIter))
      {
        NFmiMetTime wantedTime(theCalculationParams.itsTime);
        if (theTimePeekInHours)
        {  // jos halutaan kurkata ajassa eteen/taakse, tehdään ajan asetus tässä
          long changeByMinutesValue = FmiRound(theTimePeekInHours * 60);
          wantedTime.ChangeByMinutes(changeByMinutesValue);
        }
        if (infoIter->Time(wantedTime))
        {
          for (infoIter->ResetLocation(); infoIter->NextLocation();)
          {
            const NFmiLocation *location = infoIter->Location();
            const NFmiPoint &latlon(location->GetLocation());
            if (theAreaPtr->IsInside(latlon))  // aseman pitää olla kartta-alueen sisällä
            {
              // Katsotaan mitä tuloshilan hilapistettä lähinnä tämä asema oli.
              if (grid.NearestLatLon(latlon.X(), latlon.Y()))
              {
                double distance = infoIter->Location()->Distance(grid.LatLon());
                if (distance < distanceMatrix[grid.CurrentX()][grid.CurrentY()])
                {
                  distanceMatrix[grid.CurrentX()][grid.CurrentY()] = distance;
                  NFmiFastInfoUtils::SetSoundingDataLevel(
                      theLevel, *infoIter);  // Tämä tehdään vain luotaus datalle: tämä level pitää
                                             // asettaa joka pisteelle erikseen, koska vakio
                  // painepinnat eivät ole kaikille luotaus parametreille
                  // samoilla leveleillä
                  nearestValueMatrix[grid.CurrentX()][grid.CurrentY()] =
                      infoIter->FloatValue();  // ei ole väliä onko lähimmän aseman arvo puuttuva
                                               // vai ei, se halutaan aina tulokseen!!!
                }
              }
            }
          }
        }
      }
    }
  }
  return nearestValueMatrix;
}

void NFmiNearestObsValue2GridMask::DoNearestValueGriddingCheck(
    const NFmiCalculationParams &theCalculationParams)
{
  if (itsLastCalculatedTime != theCalculationParams.itsTime)
  {
    WriteLock lock(*itsCacheMutex);  // tästä saa edetä vain yksi säie kerrallaan, eli joku joka
                                     // ehtii, laskee cache-matriisin ja asettaa sen
                                     // itsCurrentGriddedStationData:n arvoksi, muut sitten vain
                                     // käyttävät sitä

    // katsotaanko löytyykö valmiiksi laskettua hilaa halutulle ajalle
    DataCache::iterator it = itsNearestObsValuesData->find(theCalculationParams.itsTime);
    if (it != itsNearestObsValuesData->end())
      itsCurrentNearestObsValuesData = &((*it).second);
    else
    {
      // lasketaan halutun ajan hila
      if (itsGriddingHelper && itsAreaPtr.get())
      {
        // otetaan argumenttina annettu arvo aika hyppäykseen
        float timePeekInHours = itsArgumentVector[0];  // kuinka paljon kurkataan ajassa
                                                       // eteen/taakse kun arvoa haetaan tähän
                                                       // ajanhetkeen

        boost::shared_ptr<NFmiDrawParam> drawParam(
            new NFmiDrawParam(itsDataIdent, itsLevel, 0, itsDataType));
        checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;  // tähän haetaan
                                                                          // tarvittavat datat
                                                                          // (synopin tapauksessa
                                                                          // mahdollisesti lista)
        itsGriddingHelper->MakeDrawedInfoVectorForMapView(infoVector, drawParam, itsAreaPtr);
        NFmiDataMatrix<float> nearestValueData = ::CalcNearestValueMatrix(itsLevel,
                                                                          theCalculationParams,
                                                                          itsResultGridSize,
                                                                          infoVector,
                                                                          itsAreaPtr,
                                                                          timePeekInHours);
        std::pair<DataCache::iterator, bool> insertResult = itsNearestObsValuesData->insert(
            std::make_pair(theCalculationParams.itsTime, nearestValueData));
        if (insertResult.second)
          itsCurrentNearestObsValuesData = &((*insertResult.first).second);
        else
          itsCurrentNearestObsValuesData = 0;
      }
    }
    itsLastCalculatedTime = theCalculationParams.itsTime;
  }
}
// ****************************************************************************
// *************** NFmiNearestObsValue2GridMask *******************************
// ****************************************************************************

// ****************************************************************************
// *************** NFmiLastTimeValueMask **************************************
// ****************************************************************************

NFmiLastTimeValueMask::NFmiLastTimeValueMask(Type theMaskType,
    NFmiInfoData::Type theDataType,
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    :NFmiStation2GridMask(theMaskType, theDataType, theInfo, thePossibleMetaParamId)
    , itsLastTimeIndex(gMissingIndex)
    ,itsLastTimeOfData(NFmiMetTime::gMissingTime)
{
    itsFunctionArgumentCount = theArgumentCount;
}

NFmiLastTimeValueMask::~NFmiLastTimeValueMask(void) = default;

NFmiLastTimeValueMask::NFmiLastTimeValueMask(const NFmiLastTimeValueMask &theOther)
    :NFmiStation2GridMask(theOther)
    , itsLastTimeIndex(theOther.itsLastTimeIndex)
    , itsLastTimeOfData(theOther.itsLastTimeOfData)
{}

NFmiAreaMask* NFmiLastTimeValueMask::Clone(void) const
{
    return new NFmiLastTimeValueMask(*this);
}

double NFmiLastTimeValueMask::Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways)
{
    if(itsInfo->IsGrid())
    {
        itsInfo->TimeIndex(itsLastTimeIndex);
        return itsInfo->InterpolatedValue(theCalculationParams.itsLatlon);
    }
    else
    {
        NFmiCalculationParams calculationParams(theCalculationParams);
        calculationParams.itsTime = itsLastTimeOfData;
        return NFmiStation2GridMask::Value(calculationParams, fUseTimeInterpolationAlways);
    }
}

void NFmiLastTimeValueMask::SetGriddingHelpers(NFmiArea *theArea,
    NFmiGriddingHelperInterface *theGriddingHelper,
    const NFmiPoint &theStation2GridSize,
    float theObservationRadiusInKm)
{
    NFmiStation2GridMask::SetGriddingHelpers(theArea,
        theGriddingHelper,
        theStation2GridSize,
        theObservationRadiusInKm);
    itsLastTimeOfData = FindLastTime();
}

NFmiMetTime NFmiLastTimeValueMask::FindLastTime()
{
    if(itsInfo->IsGrid())
    {
        // Asetetaan info viimeiseen aikaa ja otetaan sen aikaindeksi talteen
        itsInfo->LastTime();
        itsLastTimeIndex = itsInfo->TimeIndex();
        return itsInfo->Time();
    }
    else
    {
        boost::shared_ptr<NFmiDrawParam> drawParam(new NFmiDrawParam(itsDataIdent, itsLevel, 0, itsDataType));
        // tähän haetaan tarvittavat datat (synopin tapauksessa mahdollisesti lista)
        checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;  
        itsGriddingHelper->MakeDrawedInfoVectorForMapView(infoVector, drawParam, itsAreaPtr);
        if(infoVector.size() >= 1)
        {
            // Käytetään prioriteetti 1 infoa (se mikä on ensimmäisena vektorissa) pohjadatatna ja siitä otetaan haluttu aika
            return infoVector[0]->TimeDescriptor().LastTime();
        }
        else
            return NFmiMetTime::gMissingTime;
    }
}

// ****************************************************************************
// *************** NFmiLastTimeValueMask **************************************
// ****************************************************************************

// ****************************************************************************
// *************** NFmiStation2GridTimeShiftMask ******************************
// ****************************************************************************

NFmiStation2GridTimeShiftMask::NFmiStation2GridTimeShiftMask(Type theMaskType,
    NFmiInfoData::Type theDataType,
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    float theTimeOffsetInHours,
    unsigned long thePossibleMetaParamId)
    :NFmiStation2GridMask(theMaskType, theDataType, theInfo, thePossibleMetaParamId)
    ,itsTimeOffsetInHours(theTimeOffsetInHours)
    ,itsChangeByMinutesValue(boost::math::lround(theTimeOffsetInHours*60.f))
{}

NFmiStation2GridTimeShiftMask::~NFmiStation2GridTimeShiftMask(void)
{}

NFmiStation2GridTimeShiftMask::NFmiStation2GridTimeShiftMask(const NFmiStation2GridTimeShiftMask &theOther)
    :NFmiStation2GridMask(theOther)
    , itsTimeOffsetInHours(theOther.itsTimeOffsetInHours)
    , itsChangeByMinutesValue(theOther.itsChangeByMinutesValue)
{}

NFmiAreaMask* NFmiStation2GridTimeShiftMask::Clone(void) const
{
    return new NFmiStation2GridTimeShiftMask(*this);
}

double NFmiStation2GridTimeShiftMask::Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways)
{
    return NFmiStation2GridMask::Value(GetUsedCalculationParams(theCalculationParams), fUseTimeInterpolationAlways);
}

NFmiCalculationParams NFmiStation2GridTimeShiftMask::GetUsedCalculationParams(const NFmiCalculationParams &theCalculationParams)
{
    NFmiCalculationParams usedCalculationParams = theCalculationParams;
    usedCalculationParams.itsTime.ChangeByMinutes(itsChangeByMinutesValue);
    return usedCalculationParams;
}

// ****************************************************************************
// *************** NFmiStation2GridTimeShiftMask ******************************
// ****************************************************************************
