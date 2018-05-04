#include "NFmiFastInfoUtils.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiFileString.h"
#include "NFmiProducerName.h"

namespace
{
    float CalcWindVectorFromSpeedAndDirection(float WS, float WD)
    {
        if(WS != kFloatMissing && WD != kFloatMissing)
        {
            return std::round(WS) * 100 + std::round(WD / 10.f);
        }
        return kFloatMissing;
    }

    // Huomasin merkitt‰vi‰ eroja meta wind-vector visualisoinnissa, kun 
    // wind-vector:ia lasketaan u- ja v-komponenttien avulla. T‰llˆin
    // neliˆinti ja neliˆjuuri saivat arvot menem‰‰n juuri pikkuisen 
    // alle tiettyjen rajojen, jollain visualisointi oli erilainen. Nyt
    // WS pyˆristet‰‰n 1/1-milj. osissa l‰himp‰‰n kokonaislukuun, jolloin p‰‰st‰‰n ongelmasta eroon.
    float DoWindSpeedFromWindComponentsRoundingFix(float realWS)
    {
        const float roundingFactor = 1000000.f;
        float roundingFixerValue = std::round(realWS * roundingFactor) / roundingFactor;
        return roundingFixerValue;
    }

    bool AreWindComponentsOk(float u, float v)
    {
        return u != kFloatMissing && v != kFloatMissing;
    }

    float CalcWS(float u, float v)
    {
        if(!AreWindComponentsOk(u, v))
            return kFloatMissing;
        float WS = std::sqrt(u*u + v*v);
        WS = ::DoWindSpeedFromWindComponentsRoundingFix(WS);
        return WS;
    }

    float CalcWD(float u, float v)
    {
        NFmiWindDirection windDirection(u, v);
        return static_cast<float>(windDirection.Value());
    }

    float CalcU(float WS, float WD)
    {
        if(!AreWindComponentsOk(WS, WD))
            return kFloatMissing;
        // jos tuulensuunta on vaihtelevaa (999), palautetaan 0 arvo (voisi olla myˆs puuttuvaa)
        if(WD == 999)
            return 0;
        // huom! tuulen suunta pit‰‰ ensin k‰‰nt‰‰ 180 astetta ja sitten
        // muuttaa radiaaneiksi kulma/360 * 2*pii
        float value = WS * sin(((fmod(180.f + WD, 360.f) / 360.f) * (2.f * static_cast<float>(kPii))));
        return value;
    }

    float CalcV(float WS, float WD)
    {
        if(!AreWindComponentsOk(WS, WD))
            return kFloatMissing;
        // jos tuulensuunta on vaihtelevaa (999), palautetaan 0 arvo (voisi olla myˆs puuttuvaa)
        if(WD == 999)
            return 0;
        // Huom! tuulen suunta pit‰‰ ensin k‰‰nt‰‰ 180 astetta ja sitten
        // muuttaa radiaaneiksi kulma/360 * 2*pii
        float value = WS * cos(((fmod(180.f + WD, 360.f) / 360.f) * (2.f * static_cast<float>(kPii))));
        return value;
    }

    float CalcWindVectorFromWindComponents(float u, float v)
    {
        return ::CalcWindVectorFromSpeedAndDirection(::CalcWS(u, v), ::CalcWD(u, v));
    }
}

namespace NFmiFastInfoUtils
{

bool IsInfoShipTypeData(NFmiFastQueryInfo &theInfo)
{
  if (theInfo.IsGrid() == false)
  {
    FmiProducerName prodId = static_cast<FmiProducerName>(theInfo.Producer()->GetIdent());
    if (prodId == kFmiSHIP || prodId == kFmiBUOY) return true;
  }
  return false;
}

// synop-dataa ei tarvitsee asettaa mihink‰‰n, mutta
// mm. luotaus plotin yhteydess‰ pit‰‰ etsi‰ oikea korkeus/level
void SetSoundingDataLevel(const NFmiLevel &theWantedSoundingPressureLevel, NFmiFastQueryInfo &info)
{
  if (theWantedSoundingPressureLevel.LevelType() == kFmiSoundingLevel)
  {  // sounding plotin yhteydess‰ pit‰‰ asettaa level haluttuun korkeuteen
    float levelValue = theWantedSoundingPressureLevel.LevelValue();
    FmiParameterName parName = static_cast<FmiParameterName>(info.Param().GetParamIdent());
    unsigned long parIndex = info.ParamIndex();
    bool subParaUsed = info.IsSubParamUsed();
    if (info.Param(kFmiPressure))
    {
      for (info.ResetLevel(); info.NextLevel();)
        if (info.FloatValue() == levelValue) break;
    }
    if (subParaUsed)
      info.Param(parName);  // pakko vet‰‰ t‰m‰ hitaalla tavalla jostain syyst‰
    else
      info.ParamIndex(parIndex);
  }
}

std::string GetTotalDataFilePath(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  NFmiFileString totalFilePath(theInfo->DataFilePattern());
  totalFilePath.FileName(theInfo->DataFileName());
  return std::string(totalFilePath);
}

bool IsYearLongData(const boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  const auto &timeDescriptor = info->TimeDescriptor();
  auto timeDiffInDays = timeDescriptor.LastTime().DifferenceInDays(timeDescriptor.FirstTime());
  if (timeDiffInDays >= 364 && timeDiffInDays <= 366)
    return true;
  else
    return false;
}

bool IsModelClimatologyData(const boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  if (info && info->DataType() == NFmiInfoData::kClimatologyData)
  {
    if (info->IsGrid())
    {
      if (IsYearLongData(info))
      {
        return true;
      }
    }
  }
  return false;
}

NFmiMetTime GetUsedTimeIfModelClimatologyData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                              const NFmiMetTime &theTime)
{
  if (NFmiFastInfoUtils::IsModelClimatologyData(theInfo))
  {
    // For year long climatology data, used time must be fixed to data's own year
    auto usedTime(theTime);
    usedTime.SetYear(theInfo->TimeDescriptor().FirstTime().GetYear());
    return usedTime;
  }
  else
    return theTime;
}

bool IsMovingSoundingData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  if (theInfo && !theInfo->IsGrid())
  {
    if (theInfo->SizeLevels() > 7) return theInfo->HasLatlonInfoInData();
  }
  return false;
}

bool FindTimeIndicesForGivenTimeRange(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                      const NFmiMetTime &theStartTime,
                                      long minuteRange,
                                      unsigned long &timeIndex1,
                                      unsigned long &timeIndex2)
{
  theInfo->FindNearestTime(theStartTime, kForward);
  timeIndex1 = theInfo->TimeIndex();
  NFmiMetTime endTime(theStartTime);
  endTime.ChangeByMinutes(minuteRange);
  theInfo->FindNearestTime(endTime, kBackward);
  timeIndex2 = theInfo->TimeIndex();

  if(timeIndex1 == gMissingIndex || timeIndex2 == gMissingIndex)
      return false;
  else if (timeIndex1 == timeIndex2)  // pit‰‰ testata erikois tapaus, koska TimeToNearestStep-palauttaa
                                 // aina jotain, jos on dataa
  {
    theInfo->TimeIndex(timeIndex1);
    NFmiMetTime foundTime(theInfo->Time());
    if (foundTime > endTime || foundTime < theStartTime)  // jos lˆydetty aika on alku ja loppu ajan
                                                          // ulkopuolella ei piirret‰ salamaa
      return false;
  }
  return true;
}

bool FindMovingSoundingDataTime(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                const NFmiMetTime &theTime,
                                NFmiLocation &theLocation)
{
  theInfo->FirstLocation();  // liikkuvissa luotauksissa vain yksi dummy paikka, laitetaan se p‰‰lle
  NFmiMetTime timeStart(theTime);
  timeStart.ChangeByMinutes(-30);
  unsigned long timeIndex1 = 0;
  unsigned long timeIndex2 = 0;
  if (FindTimeIndicesForGivenTimeRange(theInfo, timeStart, 60, timeIndex1, timeIndex2) == false)
    return false;

  float lat = 0;
  float lon = 0;
  theInfo->Param(kFmiLatitude);
  unsigned long latIndex = theInfo->ParamIndex();
  theInfo->Param(kFmiLongitude);
  unsigned long lonIndex = theInfo->ParamIndex();
  double minDistance = 99999999;
  unsigned long minDistTimeInd = static_cast<unsigned long>(-1);
  for (unsigned long i = timeIndex1; i <= timeIndex2; i++)
  {
    theInfo->TimeIndex(i);

    for (theInfo->ResetLevel(); theInfo->NextLevel();)
    {
      theInfo->ParamIndex(latIndex);
      lat = theInfo->FloatValue();
      theInfo->ParamIndex(lonIndex);
      lon = theInfo->FloatValue();

      if (lat != kFloatMissing && lon != kFloatMissing)
      {
        NFmiLocation loc(NFmiPoint(lon, lat));
        double currDist = theLocation.Distance(loc);
        if (currDist < minDistance)
        {
          minDistance = currDist;
          minDistTimeInd = i;
        }
      }
    }
  }
  if (minDistance <
      1000 * 1000)  // jos liikkuva luotaus paikka lˆytyi v‰hint‰in 1000 km s‰teelt‰ hiiren
                    // klikkauspaikasta, otetaan kyseinen luotaus piirtoon
  {
    theInfo->TimeIndex(minDistTimeInd);
    // pit‰‰ lis‰ksi asettaa locationiksi luotauksen alkupiste
    theInfo->FirstLevel();
    theInfo->ParamIndex(latIndex);
    lat = theInfo->FloatValue();
    theInfo->ParamIndex(lonIndex);
    lon = theInfo->FloatValue();
    theLocation.SetLatitude(lat);
    theLocation.SetLongitude(lon);

    return true;
  }

  return false;
}

MetaWindVectorParamUsage CheckMetaWindVectorParamUsage(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
    MetaWindVectorParamUsage metaWindVectorParamUsage;
    if(theInfo)
    {
        if(!theInfo->Param(kFmiWindVectorMS))
        {
            if(theInfo->Param(kFmiWindDirection) && theInfo->Param(kFmiWindSpeedMS))
            {
                metaWindVectorParamUsage.fUseMetaWindVectorParam = true;
                metaWindVectorParamUsage.fHasWsAndWd = true;
            }

            if(theInfo->Param(kFmiWindUMS) && theInfo->Param(kFmiWindVMS))
            {
                metaWindVectorParamUsage.fUseMetaWindVectorParam = true;
                metaWindVectorParamUsage.fHasWindComponents = true;
            }
        }
    }
    return metaWindVectorParamUsage;
}

float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const MetaWindVectorParamUsage &metaWindVectorParamUsage)
{
    if(metaWindVectorParamUsage.fHasWsAndWd)
    {
        theInfo->Param(kFmiWindSpeedMS);
        auto WS = theInfo->FloatValue();
        theInfo->Param(kFmiWindDirection);
        auto WD = theInfo->FloatValue();
        return ::CalcWindVectorFromSpeedAndDirection(WS, WD);
    }
    else if(metaWindVectorParamUsage.fHasWindComponents)
    {
        theInfo->Param(kFmiWindUMS);
        auto u = theInfo->FloatValue();
        theInfo->Param(kFmiWindVMS);
        auto v = theInfo->FloatValue();
        return ::CalcWindVectorFromWindComponents(u, v);
    }

    return kFloatMissing;
}

float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const MetaWindVectorParamUsage &metaWindVectorParamUsage)
{
    if(metaWindVectorParamUsage.fHasWsAndWd)
    {
        theInfo->Param(kFmiWindSpeedMS);
        auto WS = theInfo->InterpolatedValue(theTime);
        theInfo->Param(kFmiWindDirection);
        auto WD = theInfo->InterpolatedValue(theTime);
        return ::CalcWindVectorFromSpeedAndDirection(WS, WD);
    }
    else if(metaWindVectorParamUsage.fHasWindComponents)
    {
        theInfo->Param(kFmiWindUMS);
        auto u = theInfo->InterpolatedValue(theTime);
        theInfo->Param(kFmiWindVMS);
        auto v = theInfo->InterpolatedValue(theTime);
        return ::CalcWindVectorFromWindComponents(u, v);
    }

    return kFloatMissing;
}

float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const NFmiPoint& theLatlon, const MetaWindVectorParamUsage &metaWindVectorParamUsage)
{
    if(metaWindVectorParamUsage.fHasWsAndWd)
    {
        theInfo->Param(kFmiWindSpeedMS);
        auto WS = theInfo->InterpolatedValue(theLatlon, theTime);
        theInfo->Param(kFmiWindDirection);
        auto WD = theInfo->InterpolatedValue(theLatlon, theTime);
        return ::CalcWindVectorFromSpeedAndDirection(WS, WD);
    }
    else if(metaWindVectorParamUsage.fHasWindComponents)
    {
        theInfo->Param(kFmiWindUMS);
        auto u = theInfo->InterpolatedValue(theLatlon, theTime);
        theInfo->Param(kFmiWindVMS);
        auto v = theInfo->InterpolatedValue(theLatlon, theTime);
        return ::CalcWindVectorFromWindComponents(u, v);
    }

    return kFloatMissing;
}

void CalcWindSpeedAndDirectionFromComponents(const std::deque<float> &u, const std::deque<float> &v, std::deque<float> &wsOut, std::deque<float> &wdOut)
{
    if(u.size() == v.size())
    {
        wsOut.resize(u.size(), kFloatMissing);
        wdOut.resize(u.size(), kFloatMissing);
        for(size_t index = 0; index < u.size(); index++)
        {
            wsOut[index] = ::CalcWS(u[index], v[index]);
            wdOut[index] = ::CalcWD(u[index], v[index]);
        }
    }
}

void CalcWindComponentsFromSpeedAndDirection(const std::deque<float> &ws, const std::deque<float> &wd, std::deque<float> &uOut, std::deque<float> &vOut)
{
    if(ws.size() == wd.size())
    {
        uOut.resize(ws.size(), kFloatMissing);
        vOut.resize(ws.size(), kFloatMissing);
        for(size_t index = 0; index < ws.size(); index++)
        {
            uOut[index] = ::CalcU(ws[index], wd[index]);
            vOut[index] = ::CalcV(ws[index], wd[index]);
        }
    }
}

void CalcWindComponentsFromSpeedAndDirection(const NFmiDataMatrix<float> &ws, const NFmiDataMatrix<float> &wd, NFmiDataMatrix<float> &uOut, NFmiDataMatrix<float> &vOut)
{
    if(ws.NX() == wd.NX() && ws.NY() == wd.NY())
    {
        uOut.Resize(ws.NX(), ws.NY(), kFloatMissing);
        vOut.Resize(ws.NX(), ws.NY(), kFloatMissing);
        for(size_t yIndex = 0; yIndex < ws.NY(); yIndex++)
        {
            for(size_t xIndex = 0; xIndex < ws.NX(); xIndex++)
            {
                uOut[xIndex][yIndex] = ::CalcU(ws[xIndex][yIndex], wd[xIndex][yIndex]);
                vOut[xIndex][yIndex] = ::CalcV(ws[xIndex][yIndex], wd[xIndex][yIndex]);
            }
        }
    }
}

void CalcWindVectorFromSpeedAndDirection(const std::deque<float> &ws, const std::deque<float> &wd, std::deque<float> &windVectorOut)
{
    if(ws.size() == wd.size())
    {
        windVectorOut.resize(ws.size(), kFloatMissing);
        for(size_t index = 0; index < ws.size(); index++)
        {
            windVectorOut[index] = ::CalcWindVectorFromSpeedAndDirection(ws[index], wd[index]);
        }
    }
}

void CalcWindVectorFromSpeedAndDirection(const NFmiDataMatrix<float> &ws, const NFmiDataMatrix<float> &wd, NFmiDataMatrix<float> &windVectorOut, unsigned int theStartColumnIndex)
{
    if(ws.NX() == wd.NX() && ws.NY() == wd.NY())
    {
        windVectorOut.Resize(ws.NX(), ws.NY(), kFloatMissing);
        for(size_t yIndex = 0; yIndex < ws.NY(); yIndex++)
        {
            for(size_t xIndex = theStartColumnIndex; xIndex < ws.NX(); xIndex++)
            {
                windVectorOut[xIndex][yIndex] = ::CalcWindVectorFromSpeedAndDirection(ws[xIndex][yIndex], wd[xIndex][yIndex]);
            }
        }
    }
}

void CalcWindVectorFromWindComponents(const NFmiDataMatrix<float> &u, const NFmiDataMatrix<float> &v, NFmiDataMatrix<float> &windVectorOut, unsigned int theStartColumnIndex)
{
    if(u.NX() == v.NX() && u.NY() == v.NY())
    {
        windVectorOut.Resize(u.NX(), u.NY(), kFloatMissing);
        for(size_t yIndex = 0; yIndex < u.NY(); yIndex++)
        {
            for(size_t xIndex = theStartColumnIndex; xIndex < u.NX(); xIndex++)
            {
                windVectorOut[xIndex][yIndex] = ::CalcWindVectorFromWindComponents(u[xIndex][yIndex], v[xIndex][yIndex]);
            }
        }
    }
}

bool SetInfoToGridPoint(boost::shared_ptr<NFmiFastQueryInfo> &info, unsigned long gridPointX, unsigned long gridPointY)
{
    // Pit‰‰ tarkistaa erikseen ett‰ hilaindeksit eiv‰t ole hilakoon ulkopuolella, koska unsigned long yli/alivuotojen (tuplana) takia voikin tulla hyv‰ksytt‰vi‰ indekseja
    if(gridPointX >= info->GridXNumber() || gridPointY >= info->GridYNumber())
        return false;
    auto locationIndex = info->Grid()->DataIndex(gridPointX, gridPointY);
    return info->LocationIndex(locationIndex);
}

}
