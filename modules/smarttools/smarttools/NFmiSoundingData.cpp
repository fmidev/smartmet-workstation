// ======================================================================
/*!
 * \file NFmiSoundingData.cpp
 *
 * Apuluokka laskemaan ja tutkimaan luotaus dataa. Osaa täyttää itsensä
 * mm. mallipinta QueryDatasta (infosta).
 */
// ======================================================================

#include "NFmiSoundingData.h"
#include "NFmiSoundingFunctions.h"

#include <newbase/NFmiAngle.h>
#include <newbase/NFmiDataModifierAvg.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiInterpolation.h>
#include <newbase/NFmiValueString.h>
#include <newbase/NFmiFastInfoUtils.h>

using namespace NFmiSoundingFunctions;

// Tarkistetaan onko fastInfon datat nousevassa vai laskevassa suunnassa (korkeus tai paine),
// jos se ei ole nousevassa järjestyksessä, käännetään annettu data vektori.
// Parasta olisi tarkistaa, onko datassa oikeasti korkeus dataa, ennen kuin kysytään theInfolta
// löytyykö sitä ja miten päin se on. LAPS datan kanssa kävi niin että datassa on kyllä geopHeight param
// mutta se oli puuttuvaa ja sen mukaiset HeightParamIsRising kyselyt menivätkin sitten väärin päin.
void ReverseSoundingData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         std::deque<float> &theDataVector, bool hasActualGeopHeightData)
{
  if (hasActualGeopHeightData && theInfo->HeightDataAvailable())
  {                                               // jos on korkeus dataa
    if (theInfo->HeightParamIsRising() == false)  // ja korkeus parametri ei ole nousevassa
      // järjestyksessä, käännetään vektorissa olevat
      // arvot
      std::reverse(theDataVector.begin(), theDataVector.end());
  }
  else if (theInfo->PressureDataAvailable())
  {                                        // jos on paine dataa
    if (theInfo->PressureParamIsRising())  // ja paine on nousevassa järjestyksessä, käännetään
                                           // vektorissa olevat arvot
      std::reverse(theDataVector.begin(), theDataVector.end());
  }
}

// hakee lähimmän sopivan painepinnan, mistä löytyy halutuille parametreille arvot
// Mutta ei sallita muokkausta ennen 1. validia leveliä!
bool NFmiSoundingData::GetTandTdValuesFromNearestPressureLevel(double P,
                                                               double &theFoundP,
                                                               double &theT,
                                                               double &theTd)
{
  if (P != kFloatMissing)
  {
    std::deque<float> &pV = GetParamData(kFmiPressure);
    std::deque<float> &tV = GetParamData(kFmiTemperature);
    std::deque<float> &tdV = GetParamData(kFmiDewPoint);
    if (pV.size() > 0)  // oletus että parV on saman kokoinen kuin pV -vektori
    {
      double minDiffP = 999999;
      theFoundP = 999999;
      theT = kFloatMissing;
      theTd = kFloatMissing;
      bool foundLevel = false;
      for (int i = 0; i < static_cast<int>(pV.size()); i++)
      {
        if (i < 0)
          return false;  // jos 'tyhjä' luotaus, on tässä aluksi -1 indeksinä
        if (pV[i] != kFloatMissing)
        {
          double pDiff = ::fabs(pV[i] - P);
          if (pDiff < minDiffP)
          {
            theFoundP = pV[i];
            minDiffP = pDiff;
            theT = tV[i];
            theTd = tdV[i];
            foundLevel = true;
          }
        }
      }
      if (foundLevel)
        return true;

      theFoundP = kFloatMissing;  // 'nollataan' tämä vielä varmuuden vuoksi
    }
  }
  return false;
}

// hakee lähimmän sopivan painepinnan, mistä löytyy halutulle parametrille ei-puuttuva arvo
bool NFmiSoundingData::SetValueToPressureLevel(float P, float theParamValue, FmiParameterName theId)
{
  if (P != kFloatMissing)
  {
    std::deque<float> &pV = GetParamData(kFmiPressure);
    std::deque<float> &parV = GetParamData(theId);
    if (pV.size() > 0)  // oletus että parV on saman kokoinen kuin pV -vektori
    {
      std::deque<float>::iterator it = std::find(pV.begin(), pV.end(), P);
      if (it != pV.end())
      {
        int index = static_cast<int>(std::distance(pV.begin(), it));
        parV[index] = theParamValue;
        return true;
      }
    }
  }
  return false;
}

void NFmiSoundingData::SetTandTdSurfaceValues(float T, float Td)
{
  std::deque<float> &tV = GetParamData(kFmiTemperature);
  std::deque<float> &tdV = GetParamData(kFmiDewPoint);
  tV[0] = T;
  tdV[0] = Td;
}

// paluttaa paine arvon halutulle metri korkeudelle
float NFmiSoundingData::GetPressureAtHeight(double H)
{
  if (H == kFloatMissing)
    return kFloatMissing;

  double maxDiffInH = 100;  // jos ei voi interpoloida, pitää löydetyn arvon olla vähintäin näin
                            // lähellä, että hyväksytään
  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &hV = GetParamData(kFmiGeomHeight);
  float value = kFloatMissing;
  if (hV.size() > 0 && pV.size() == hV.size())
  {
    unsigned int ssize = static_cast<unsigned int>(pV.size());
    float lastP = kFloatMissing;
    float lastH = kFloatMissing;
    float currentP = kFloatMissing;
    float currentH = kFloatMissing;
    bool goneOverWantedHeight = false;
    for (unsigned int i = 0; i < ssize; i++)
    {
      currentP = pV[i];
      currentH = hV[i];
      if (currentH != kFloatMissing)
      {
        if (currentH > H)
        {
          if (currentP != kFloatMissing)
          {
            goneOverWantedHeight = true;
            break;
          }
        }
        if (currentP != kFloatMissing)
        {
          lastP = currentP;
          lastH = currentH;
          if (currentH == H)
            return currentP;  // jos oli tarkka osuma, turha jatkaa
        }
      }
    }
    if (goneOverWantedHeight && lastP != kFloatMissing && currentP != kFloatMissing &&
        lastH != kFloatMissing && currentH != kFloatMissing)
    {  // interpoloidaan arvo kun löytyi kaikki arvot
      value = static_cast<float>(NFmiInterpolation::Linear(H, currentH, lastH, currentP, lastP));
    }
    else if (lastP != kFloatMissing && lastH != kFloatMissing && ::fabs(lastH - H) < maxDiffInH)
      value = lastP;
    else if (currentP != kFloatMissing && currentH != kFloatMissing &&
             ::fabs(currentH - H) < maxDiffInH)
      value = currentP;
  }
  return value;
}

// Luotausten muokkausta varten pitää tietään onko ensimmäinen luotaus vaihtunut (paikka, aika,
// origin-aika).
// Jos ei ole ja luotausta on modifioitu, piirretään modifioitu, muuten nollataan 'modifioitu'
// luotaus ja täytetään se tällä uudella datalla.
bool NFmiSoundingData::IsSameSounding(const NFmiSoundingData &theOtherSounding)
{
  if (Location() == theOtherSounding.Location())
    if (Time() == theOtherSounding.Time())
      if (OriginTime() == theOtherSounding.OriginTime())
        return true;
  return false;
}

// hakee ne arvot h, u ja v parametreista, mitkä ovat samalta korkeudelta ja mitkä
// eivät ole puuttuvia. Haetaan alhaalta ylös päin arvoja
bool NFmiSoundingData::GetLowestNonMissingValues(float &H, float &U, float &V)
{
  std::deque<float> &hV = GetParamData(kFmiGeomHeight);
  std::deque<float> &uV = GetParamData(kFmiWindUMS);
  std::deque<float> &vV = GetParamData(kFmiWindVMS);
  if (hV.size() > 0 && hV.size() == uV.size() && hV.size() == vV.size())
  {
    for (int i = 0; i < static_cast<int>(hV.size()); i++)
    {
      if (hV[i] != kFloatMissing && uV[i] != kFloatMissing && vV[i] != kFloatMissing)
      {
        H = hV[i];
        U = uV[i];
        V = vV[i];
        return true;
      }
    }
  }
  return false;
}

// laskee halutun parametrin arvon haluttuun metri korkeuteen
float NFmiSoundingData::GetValueAtHeight(FmiParameterName theId, float H)
{
  float P = GetPressureAtHeight(H);
  if (P == kFloatMissing)
    return kFloatMissing;

  return GetValueAtPressure(theId, P);
}

// Hakee halutun parametrin arvon halutulta painekorkeudelta.
float NFmiSoundingData::GetValueAtPressure(FmiParameterName theId, float P)
{
  if (P == kFloatMissing)
    return kFloatMissing;

  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &paramV = GetParamData(theId);
  float value = kFloatMissing;
  if (paramV.size() > 0 && pV.size() == paramV.size())
  {
    unsigned int ssize = static_cast<unsigned int>(pV.size());
    float lastP = kFloatMissing;
    float lastValue = kFloatMissing;
    float currentP = kFloatMissing;
    float currentValue = kFloatMissing;
    for (unsigned int i = 0; i < ssize; i++)
    {
      currentP = pV[i];
      currentValue = paramV[i];
      if (currentP != kFloatMissing)
      {
        if (currentP < P)
        {
          if (currentValue != kFloatMissing)
            break;
        }
        if (currentValue != kFloatMissing)
        {
          lastP = currentP;
          lastValue = currentValue;
          if (currentP == P)
            return currentValue;  // jos oli tarkka osuma, turha jatkaa
        }
      }
    }
    float maxPDiff =
        10.;  // suurin sallittu ero, että arvo hyväksytään, jos pyydetty paine on 'asteikon ulkona'
    if (lastP != kFloatMissing && currentP != kFloatMissing && lastValue != kFloatMissing &&
        currentValue != kFloatMissing)
    {  // interpoloidaan arvo kun löytyi kaikki arvot
      if (theId == kFmiWindVectorMS)
        value = CalcLogInterpolatedWindWectorValue(lastP, currentP, P, lastValue, currentValue);
      else
        value = CalcLogInterpolatedValue(lastP, currentP, P, lastValue, currentValue);
    }
    else if (lastP != kFloatMissing && lastValue != kFloatMissing)
    {
      if (::fabs(lastP - P) < maxPDiff)
        value = lastValue;
    }
    else if (currentP != kFloatMissing && currentValue != kFloatMissing)
    {
      if (::fabs(currentP - P) < maxPDiff)
        value = currentValue;
    }
  }
  return value;
}

// Laskee u ja v komponenttien keskiarvot halutulla välillä
// Huom! z korkeudet interpoloidaan, koska havaituissa luotauksissa niitä ei ole aina ja varsinkaan
// samoissa
// väleissä kuin tuulia
bool NFmiSoundingData::CalcAvgWindComponentValues(double fromZ, double toZ, double &u, double &v)
{
  u = kFloatMissing;
  v = kFloatMissing;
  std::deque<float> &pV = GetParamData(kFmiPressure);
  if (pV.size() > 0)
  {
    double hStep = 100;  // käydään dataa läpi 100 metrin välein

    NFmiDataModifierAvg uAvg;
    NFmiDataModifierAvg vAvg;
    for (double h = fromZ; h < toZ + hStep / 2.; h += hStep)  // käydää layeria 100 metrin välein
    {
      double pressure = GetPressureAtHeight(h);
      float tmpU = GetValueAtPressure(kFmiWindUMS, static_cast<float>(pressure));
      float tmpV = GetValueAtPressure(kFmiWindVMS, static_cast<float>(pressure));
      if (tmpU != kFloatMissing && tmpV != kFloatMissing)
      {
        uAvg.Calculate(tmpU);
        vAvg.Calculate(tmpV);
      }
    }
    u = uAvg.CalculationResult();
    v = vAvg.CalculationResult();
    return (u != kFloatMissing && v != kFloatMissing);
  }
  return false;
}

float NFmiSoundingData::FindPressureWhereHighestValue(FmiParameterName theId,
                                                      float theMaxP,
                                                      float theMinP)
{
  float maxValuePressure = kFloatMissing;
  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &paramV = GetParamData(theId);
  if (pV.size() > 0)
  {
    float maxValue = -99999999.f;
    for (unsigned int i = 0; i < pV.size(); i++)
    {
      float tmpP = pV[i];
      float tmpParam = paramV[i];
      if (tmpP != kFloatMissing && tmpParam != kFloatMissing)
      {
        if (tmpP <= theMaxP && tmpP >= theMinP)  // eli ollaanko haluttujen pintojen välissä
        {
          if (tmpParam > maxValue)
          {
            maxValue = tmpParam;
            maxValuePressure = tmpP;
          }
        }

        if (tmpP < theMinP)  // jos oltiin jo korkeammalla kuin minimi paine, voidaan lopettaa
          break;
      }
    }
  }
  return maxValuePressure;
}

// Käy läpi luotausta ja etsi sen kerroksen arvot, jolta löytyy suurin theta-E ja
// palauta sen kerroksen T, Td ja P ja laskettu max Theta-e.
// Etsitään arvoja jos pinta on alle theMinP-tason (siis alle tuon tason fyysisesti).
// HUOM! theMinP ei voi olla kFloatMissing, jos haluat että kaikki levelit käydään läpi laita sen
// arvoksi 0.
bool NFmiSoundingData::FindHighestThetaE(
    double &T, double &Td, double &P, double &theMaxThetaE, double theMinP)
{
  T = kFloatMissing;
  Td = kFloatMissing;
  P = kFloatMissing;
  theMaxThetaE = kFloatMissing;
  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &tV = GetParamData(kFmiTemperature);
  std::deque<float> &tdV = GetParamData(kFmiDewPoint);
  if (pV.size() > 0)
  {
    theMaxThetaE = -99999999;
    for (unsigned int i = 0; i < pV.size(); i++)
    {
      float tmpP = pV[i];
      float tmpT = tV[i];
      float tmpTd = tdV[i];
      if (tmpP != kFloatMissing && tmpT != kFloatMissing && tmpTd != kFloatMissing)
      {
        if (tmpP >= theMinP)  // eli ollaanko lähempana maanpintaa kuin raja paine pinta on
        {
          double thetaE = CalcThetaE(tmpT, tmpTd, tmpP);
          if (thetaE != kFloatMissing && thetaE > theMaxThetaE)
          {
            theMaxThetaE = thetaE;
            T = tmpT;
            Td = tmpTd;
            P = tmpP;
          }
        }
        else  // jos oltiin korkeammalla, voidaan lopettaa
          break;
      }
    }
  }
  return theMaxThetaE != kFloatMissing;
}

// Tämä on Pieter Groenemeijerin ehdottama tapa laskea LCL-laskuihin tarvittavia T, Td ja P arvoja
// yli halutun layerin.
// Laskee keskiarvot T:lle, Td:lle ja P:lle haluttujen korkeuksien välille.
// Eli laskee keskiarvon lämpötilalle potentiaali lömpötilojen avulla.
// Laskee kastepisteen keskiarvon mixing valueiden avulla.
// Näille lasketaan keskiarvot laskemalla halutun layerin läpi 1 hPa askelissa, ettei epämääräiset
// näyte valit painota mitenkaan laskuja.
// Paineelle otetaan suoraan pohja kerroksen arvo.
// Oletus fromZ ja toZ eivät ole puuttuvia.
bool NFmiSoundingData::CalcLCLAvgValues(
    double fromZ, double toZ, double &T, double &Td, double &P, bool fUsePotTandMix)
{
  T = kFloatMissing;
  Td = kFloatMissing;
  P = kFloatMissing;
  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &tV = GetParamData(kFmiTemperature);
  std::deque<float> &tdV = GetParamData(kFmiDewPoint);
  if (pV.size() > 0)
  {
    for (unsigned int i = 0; i < pV.size(); i++)
      if (pV[i] != kFloatMissing && tV[i] != kFloatMissing &&
          tdV[i] != kFloatMissing)  // etsitään 1. ei puuttuva paine arvo eli alin paine arvo (missä
                                    // myös T ja Td arvot)
      {
        P = pV[i];
        break;
      }
    if (P == kFloatMissing)
      return false;
    int startP = static_cast<int>(round(GetPressureAtHeight(fromZ)));
    int endP = static_cast<int>(round(GetPressureAtHeight(toZ)));
    if (startP == kFloatMissing || endP == kFloatMissing || startP <= endP)
      return false;
    NFmiDataModifierAvg avgT;   // riippuen moodista tässä lasketaan T tai Tpot keskiarvoa
    NFmiDataModifierAvg avgTd;  // riippuen moodista tässä lasketaan Td tai w keskiarvoa
    for (int pressure = startP; pressure > endP;
         pressure--)  // käydää layeria yhden hPa:n välein läpi
    {
      float temperature = GetValueAtPressure(kFmiTemperature, static_cast<float>(pressure));
      float dewpoint = GetValueAtPressure(kFmiDewPoint, static_cast<float>(pressure));
      if (temperature != kFloatMissing && dewpoint != kFloatMissing)
      {
        if (fUsePotTandMix)
        {
          avgT.Calculate(static_cast<float>(T2tpot(temperature, pressure)));
          avgTd.Calculate(static_cast<float>(CalcMixingRatio(temperature, dewpoint, pressure)));
        }
        else
        {
          avgT.Calculate(temperature);
          avgTd.Calculate(dewpoint);
        }
      }
    }
    if (avgT.CalculationResult() != kFloatMissing && avgTd.CalculationResult() != kFloatMissing)
    {
      if (fUsePotTandMix)
      {
        T = Tpot2t(avgT.CalculationResult(), P);
        Td = CalcDewPoint(T, avgTd.CalculationResult(), P);
      }
      else
      {
        T = avgT.CalculationResult();
        Td = avgTd.CalculationResult();
      }
      return (P != kFloatMissing && T != kFloatMissing && Td != kFloatMissing);
    }
  }
  return false;
}

// Tämä hakee annettua painearvoa lähimmät arvot, jotka löytyvät kaikille halutuille parametreille.
// Palauttaa true, jos löytyy dataa ja false jos ei löydy.
// OLETUS: maanpinta arvot ovat vektorin alussa, pitäisi tarkistaa??
bool NFmiSoundingData::GetValuesStartingLookingFromPressureLevel(double &T, double &Td, double &P)
{
  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &tV = GetParamData(kFmiTemperature);
  std::deque<float> &tdV = GetParamData(kFmiDewPoint);
  if (pV.size() > 0)
  {
    for (unsigned int i = 0; i < pV.size(); i++)
    {
      if (pV[i] != kFloatMissing)  // && tV[i] != kFloatMissing && tdV[i] != kFloatMissing)
      {
        if (P >= pV[i] && tV[i] != kFloatMissing && tdV[i] != kFloatMissing)
        {
          T = tV[i];
          Td = tdV[i];
          P = pV[i];
          return true;
        }
      }
    }
  }
  return false;
}

// oletuksia paljon:
// theInfo on validi, aika ja paikka on jo asetettu
bool NFmiSoundingData::FillParamData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                     FmiParameterName theId)
{
  try
  {
    std::deque<float> &data = GetParamData(theId);
    data.resize(theInfo->SizeLevels(), kFloatMissing);  // alustetaan vektori puuttuvalla
    bool paramFound = theInfo->Param(theId);
    if (paramFound == false && theId == kFmiDewPoint)
      paramFound = theInfo->Param(kFmiDewPoint2M);  // kastepiste data voi tulla luotausten
    // yhteydessä tällä parametrilla ja mallidatan
    // yhteydessä toisella
    if (paramFound)
    {
      int i = 0;
      for (theInfo->ResetLevel(); theInfo->NextLevel(); i++)
        data[i] = theInfo->FloatValue();
      if (theInfo->HeightParamIsRising() ==
          false)  // jos ei nousevassa järjestyksessä, käännetään vektorissa olevat arvot
        std::reverse(data.begin(), data.end());
      if (theId == kFmiPressure)
        fPressureDataAvailable = true;
      if (theId == kFmiGeomHeight || theId == kFmiGeopHeight || theId == kFmiFlAltitude)
        fHeightDataAvailable = true;
      return true;
    }
  }
  catch (std::exception & /* e */)
  {
  }
  return false;
}

bool NFmiSoundingData::FillParamData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                     FmiParameterName theId,
                                     const NFmiMetTime &theTime,
                                     const NFmiPoint &theLatlon)
{
  bool status = false;
  std::deque<float> &data = GetParamData(theId);
  data.resize(theInfo->SizeLevels(), kFloatMissing);  // alustetaan vektori puuttuvalla
  if (theInfo->Param(theId))
  {
    int i = 0;
    for (theInfo->ResetLevel(); theInfo->NextLevel(); i++)
      data[i] = theInfo->InterpolatedValue(theLatlon, theTime);  // varmuuden vuoksi kaikki
                                                                 // interpoloinnit päälle, se
                                                                 // funktio tarkistaa tarvitseeko
                                                                 // sitä tehdä
    status = true;
  }
  else if (theId == kFmiDewPoint && theInfo->Param(kFmiHumidity))
  {
    unsigned int RHindex = theInfo->ParamIndex();
    if (!theInfo->Param(kFmiTemperature))
      return false;
    unsigned int Tindex = theInfo->ParamIndex();
    float T = 0;
    float RH = 0;
    int i = 0;
    for (theInfo->ResetLevel(); theInfo->NextLevel(); i++)
    {
      theInfo->ParamIndex(Tindex);
      T = theInfo->InterpolatedValue(theLatlon, theTime);  // varmuuden vuoksi kaikki interpoloinnit
                                                           // päälle, se funktio tarkistaa
                                                           // tarvitseeko sitä tehdä
      theInfo->ParamIndex(RHindex);
      RH = theInfo->InterpolatedValue(theLatlon, theTime);  // varmuuden vuoksi kaikki
                                                            // interpoloinnit päälle, se funktio
                                                            // tarkistaa tarvitseeko sitä tehdä
      data[i] = static_cast<float>(CalcDP(T, RH));
    }
    status = true;
  }
  else if (theId == kFmiPressure)
  {  // jos halutaan paine dataa ja parametria ei ollut datassa, oliko kyseessa painepinta data,
     // jolloin paine pitää irroittaa level-tiedosta
    if (theInfo->FirstLevel())
    {
      if (theInfo->Level()->LevelType() == kFmiPressureLevel)
      {
        int i = 0;
        for (theInfo->ResetLevel(); theInfo->NextLevel(); i++)
          data[i] = static_cast<float>(theInfo->Level()->LevelValue());
        status = true;
      }
    }
  }

  ::ReverseSoundingData(theInfo, data);

  return status;
}

unsigned int NFmiSoundingData::GetHighestNonMissingValueLevelIndex(FmiParameterName theParaId)
{
  std::deque<float> &vec = GetParamData(theParaId);
  std::deque<float>::size_type ssize = vec.size();
  unsigned int index = 0;
  for (unsigned int i = 0; i < ssize; i++)
    if (vec[i] != kFloatMissing)
      index = i;
  return index;
}

// tämä leikkaa Fill.. -metodeissa laskettuja data vektoreita niin että pelkät puuttuvat kerrokset
// otetaan pois
void NFmiSoundingData::CutEmptyData(void)
{
  std::vector<FmiParameterName> itsSoundingParameters;
  itsSoundingParameters.push_back(kFmiPressure);
  itsSoundingParameters.push_back(kFmiTemperature);
  itsSoundingParameters.push_back(kFmiDewPoint);
  itsSoundingParameters.push_back(kFmiWindSpeedMS);
  itsSoundingParameters.push_back(kFmiWindDirection);
  itsSoundingParameters.push_back(kFmiGeomHeight);

  unsigned int greatestNonMissingLevelIndex = 0;
  unsigned int maxLevelIndex =
      static_cast<unsigned int>(GetParamData(itsSoundingParameters[0]).size());
  for (unsigned int i = 0; i < itsSoundingParameters.size(); i++)
  {
    unsigned int currentIndex = GetHighestNonMissingValueLevelIndex(itsSoundingParameters[i]);
    if (currentIndex > greatestNonMissingLevelIndex)
      greatestNonMissingLevelIndex = currentIndex;
    if (greatestNonMissingLevelIndex >= maxLevelIndex)
      return;  // ei tarvitse leikata, kun dataa löytyy korkeimmaltakin leveliltä
  }

  // tässä pitää käydä läpi kaikki data vektorit!!!! Oikeasti nämä datavektori pitäisi laittaa omaan
  // vektoriin että sitä voitaisiin iteroida oikein!
  itsTemperatureData.resize(greatestNonMissingLevelIndex);
  itsDewPointData.resize(greatestNonMissingLevelIndex);
  itsHumidityData.resize(greatestNonMissingLevelIndex);
  itsPressureData.resize(greatestNonMissingLevelIndex);
  itsGeomHeightData.resize(greatestNonMissingLevelIndex);
  itsWindSpeedData.resize(greatestNonMissingLevelIndex);
  itsWindDirectionData.resize(greatestNonMissingLevelIndex);
  itsWindComponentUData.resize(greatestNonMissingLevelIndex);
  itsWindComponentVData.resize(greatestNonMissingLevelIndex);
  itsWindVectorData.resize(greatestNonMissingLevelIndex);
}

// Tälle anntaan asema dataa ja ei tehdä minkäänlaisia interpolointeja.
bool NFmiSoundingData::FillSoundingData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                        const NFmiMetTime &theTime,
                                        const NFmiMetTime &theOriginTime,
                                        const NFmiLocation &theLocation,
                                        int useStationIdOnly)
{
  NFmiMetTime usedTime = theTime;
  NFmiLocation usedLocation(theLocation);
  ClearDatas();
  if (theInfo && !theInfo->IsGrid())
  {
    fObservationData = true;
    theInfo->FirstLevel();
    bool amdarSounding = (theInfo->Level()->LevelType() == kFmiAmdarLevel);
    bool timeOk = false;
    if (amdarSounding)
    {
      timeOk = NFmiFastInfoUtils::FindMovingSoundingDataTime(theInfo, usedTime, usedLocation);
      usedTime = theInfo->Time();
    }
    else
      timeOk = theInfo->Time(usedTime);
    if (timeOk)
    {
      bool stationOk = false;
      if (amdarSounding)
        stationOk = true;  // asemaa ei etsitä, jokainen lento liittyy tiettyyn aikaa
      else
        stationOk = (useStationIdOnly ? theInfo->Location(usedLocation.GetIdent())
                                      : theInfo->Location(usedLocation));
      if (stationOk)
      {
        itsLocation = usedLocation;
        itsTime = usedTime;
        itsOriginTime = theOriginTime;

        FillParamData(theInfo, kFmiTemperature);
        FillParamData(theInfo, kFmiDewPoint);
        FillParamData(theInfo, kFmiPressure);
        if (!FillParamData(theInfo, kFmiGeomHeight))
          if (!FillParamData(theInfo, kFmiGeopHeight))
            FillParamData(theInfo, kFmiFlAltitude);  // eri datoissa on geom ja geop heightia,
                                                     // kokeillaan molempia tarvittaessa
        FillParamData(theInfo, kFmiWindSpeedMS);
        FillParamData(theInfo, kFmiWindDirection);
        FillParamData(theInfo, kFmiWindUMS);
        FillParamData(theInfo, kFmiWindVMS);
        FillParamData(theInfo, kFmiWindVectorMS);
        CalculateHumidityData();
        InitZeroHeight();
        return true;
      }
    }
  }
  return false;
}

// Tälle annetaan hiladataa, ja interpolointi tehdään tarvittaessa ajassa ja paikassa.
bool NFmiSoundingData::FillSoundingData(
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const NFmiMetTime &theTime,
    const NFmiMetTime &theOriginTime,
    const NFmiPoint &theLatlon,
    const NFmiString &theName,
    const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo)
{
  ClearDatas();
  if (theInfo && theInfo->IsGrid())
  {
    fObservationData = false;
    itsLocation = NFmiLocation(theLatlon);
    itsLocation.SetName(theName);
    itsTime = theTime;
    itsOriginTime = theOriginTime;

    FillParamData(theInfo, kFmiTemperature, theTime, theLatlon);
    FillParamData(theInfo, kFmiDewPoint, theTime, theLatlon);
    FillParamData(theInfo, kFmiPressure, theTime, theLatlon);
    if (!FillParamData(theInfo, kFmiGeomHeight, theTime, theLatlon))
      FillParamData(
          theInfo,
          kFmiGeopHeight,
          theTime,
          theLatlon);  // eri datoissa on geom ja geop heightia, kokeillaan molempia tarvittaessa
    FillParamData(theInfo, kFmiWindSpeedMS, theTime, theLatlon);
    FillParamData(theInfo, kFmiWindDirection, theTime, theLatlon);
    FillParamData(theInfo, kFmiWindUMS, theTime, theLatlon);
    FillParamData(theInfo, kFmiWindVMS, theTime, theLatlon);
    FillParamData(theInfo, kFmiWindVectorMS, theTime, theLatlon);
    CalculateHumidityData();
    InitZeroHeight();
    FixPressureDataSoundingWithGroundData(theGroundDataInfo);
    return true;
  }
  return false;
}

// theVec is vector that is to be erased from the start.
// theCutIndex is the index that ends the cutting operation. If 0 then nothing is done.
template <class vectorContainer>
static void CutStartOfVector(vectorContainer &theVec, int theCutIndex)
{
  if (theVec.size() > 0 && theCutIndex > 0 &&
      static_cast<unsigned long>(theCutIndex) < theVec.size() - 1)
    theVec.erase(theVec.begin(), theVec.begin() + theCutIndex);
}

// Jos kyseessä on painepinta dataa, mistä löytyy myös siihen liittyvä pinta data, jossa on
// mukana parametri 472 eli paine aseman korkeudella, laitetaan tämä uudeksi ala-paineeksi
// luotaus-dataan
// ja laitetaan pinta-arvot muutenkin alimmalle tasolle pinta-datan mukaisiksi.
// HUOM! Oletus että löytyi ainakin yksi kerros, joka oli alle tämän pintakerroksen, koska
// en tee taulukkojen resize:a ainakaan nyt, eli taulukossa pitää olla tilaa tälle uudelle
// pintakerrokselle.
void NFmiSoundingData::FixPressureDataSoundingWithGroundData(
    const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo)
{
  if (theGroundDataInfo)
  {
    NFmiPoint wantedLatlon(itsLocation.GetLocation());
    theGroundDataInfo->Param(kFmiTemperature);
    float groundT = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);
    theGroundDataInfo->Param(kFmiDewPoint);
    float groundTd = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);
    theGroundDataInfo->Param(kFmiWindSpeedMS);
    float groundWS = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);
    theGroundDataInfo->Param(kFmiWindDirection);
    float groundWD = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);
    theGroundDataInfo->Param(kFmiWindUMS);
    float groundU = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);
    theGroundDataInfo->Param(kFmiWindVMS);
    float groundV = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);
    theGroundDataInfo->Param(kFmiWindVectorMS);
    float groundWv = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);
    theGroundDataInfo->Param(kFmiPressureAtStationLevel);
    float groundStationPressure = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);
    theGroundDataInfo->Param(kFmiHumidity);
    float groundRH = theGroundDataInfo->InterpolatedValue(wantedLatlon, itsTime);

    if (groundStationPressure != kFloatMissing)
    {
      // oletus, kaikki vektorit on alustettu saman kokoisiksi kuin paine vektori
      if (theGroundDataInfo && itsPressureData.size() > 0)
      {
        // HUOM! luotausdatat ovat aina 'nousevassa' järjestyksessä eli maanpinta on taulukoissa
        // ensimmäisenä
        for (int i = 0; i < static_cast<int>(itsPressureData.size()); i++)
        {
          float currentPressure = itsPressureData[i];
          if (currentPressure != kFloatMissing && groundStationPressure >= currentPressure)
          {  // Eli nyt luotausdata taulukosta löytynyt paine on pienempi kuin paineasemakorkeudella
             // (eli ollaan juuri menty
            // groundStationPressure-lukeman yli siis pinnasta avaruutta kohden, ei isompaan paine
            // lukemaan), eli
            // uusi pinta on laitettava ennen tätä kerrosta (jos kyseessä on alin pinta taulukosta)
            if (i > 0)
            {
              // mennään siis edelliseen tasoon ja laitetaan se uudeksi pinta kerrokseksi
              i--;
              itsZeroHeightIndex = i;
              itsZeroHeight = 2;

              itsGeomHeightData[i] = 2;  // Oletus: pinta data on 2 metrin korkeudella
              itsPressureData[i] = groundStationPressure;
              itsTemperatureData[i] = groundT;
              itsDewPointData[i] = groundTd;
              itsWindSpeedData[i] = groundWS;
              itsWindDirectionData[i] = groundWD;
              itsWindVectorData[i] = groundWv;
              itsWindComponentUData[i] = groundU;
              itsWindComponentVData[i] = groundV;
              itsHumidityData[i] = groundRH;

              // pitää ottaa vektoreista alkuosa pois, kun tuota itsZeroHeightIndex -dataosaa
              // ei näemmä käytetäkään missään
              if (itsZeroHeightIndex > 0)
              {  // huonoa koodia pitäisi olla vektorin vektori, että ei tarvitse aina muistaa
                 // kuinka monta vektoria on olemassa
                ::CutStartOfVector(itsGeomHeightData, itsZeroHeightIndex);
                ::CutStartOfVector(itsPressureData, itsZeroHeightIndex);
                ::CutStartOfVector(itsTemperatureData, itsZeroHeightIndex);
                ::CutStartOfVector(itsDewPointData, itsZeroHeightIndex);
                ::CutStartOfVector(itsWindSpeedData, itsZeroHeightIndex);
                ::CutStartOfVector(itsWindDirectionData, itsZeroHeightIndex);
                ::CutStartOfVector(itsWindVectorData, itsZeroHeightIndex);
                ::CutStartOfVector(itsWindComponentUData, itsZeroHeightIndex);
                ::CutStartOfVector(itsWindComponentVData, itsZeroHeightIndex);
                ::CutStartOfVector(itsHumidityData, itsZeroHeightIndex);
                itsZeroHeightIndex = 0;
              }

              break;
            }
          }
        }
      }
    }
  }
}

// laskee jo laskettujen T ja Td avulla RH
void NFmiSoundingData::CalculateHumidityData(void)
{
  size_t tVectorSize = itsTemperatureData.size();
  if (tVectorSize > 0 && itsDewPointData.size() == tVectorSize)
  {
    itsHumidityData.resize(tVectorSize);
    for (size_t i = 0; i < tVectorSize; i++)
    {
      if (itsTemperatureData[i] != kFloatMissing && itsDewPointData[i] != kFloatMissing)
        itsHumidityData[i] = static_cast<float>(
            NFmiSoundingFunctions::CalcRH(itsTemperatureData[i], itsDewPointData[i]));
    }
  }
}

// tätä kutsutaan FillParamData-metodeista
// sillä katsotaan mistä korkeudesta luotaus oikeasti alkaa
// Tehdään aloitus korkeus seuraavasti:
// Se pinta, miltä kaikki löytyvät 1. kerran sekä paine, että korkeus arvot
// ja lisäksi joko lämpötila tai tuulennopeus.
// Tai jos sellaista ei löydy asetetaan arvoksi 0.
void NFmiSoundingData::InitZeroHeight(void)
{
  itsZeroHeight = 0;
  itsZeroHeightIndex = -1;
  // oletus, kaikki vektorit on alustettu saman kokoisiksi kuin paine vektori
  if (itsPressureData.size() > 0)
  {
    for (int i = 0; i < static_cast<int>(itsPressureData.size()); i++)
    {
      if (itsPressureData[i] != kFloatMissing && itsGeomHeightData[i] != kFloatMissing &&
          (itsTemperatureData[i] != kFloatMissing ||  // lämpötilaa ei ehkä tarvitse olla
           // etsittäessä ensimmäistä validia kerrosta
           itsWindSpeedData[i] !=
               kFloatMissing)  // tuulta ei tarvitse olla etsittäessä ensimmäistä validia kerrosta
          )
      {
        itsZeroHeight = itsGeomHeightData[i];
        itsZeroHeightIndex = i;
        break;  // lopetetaan kun 1. löytyi
      }
    }
  }
}

std::deque<float> &NFmiSoundingData::GetParamData(FmiParameterName theId)
{
  static std::deque<float> dummy;
  switch (theId)
  {
    case kFmiTemperature:
      return itsTemperatureData;
    case kFmiDewPoint:
      return itsDewPointData;
    case kFmiHumidity:
      return itsHumidityData;
    case kFmiPressure:
      return itsPressureData;
    case kFmiGeopHeight:
    case kFmiGeomHeight:
    case kFmiFlAltitude:
      return itsGeomHeightData;
    case kFmiWindSpeedMS:
      return itsWindSpeedData;
    case kFmiWindDirection:
      return itsWindDirectionData;
    case kFmiWindUMS:
      return itsWindComponentUData;
    case kFmiWindVMS:
      return itsWindComponentVData;
    case kFmiWindVectorMS:
      return itsWindVectorData;
    default:
      throw std::runtime_error(
          std::string(
              "NFmiSoundingData::GetParamData - wrong paramId given (Error in Program?): ") +
          NFmiStringTools::Convert<int>(theId));
  }
}

void NFmiSoundingData::ClearDatas(void)
{
  std::deque<float>().swap(itsTemperatureData);
  std::deque<float>().swap(itsDewPointData);
  std::deque<float>().swap(itsHumidityData);
  std::deque<float>().swap(itsPressureData);
  std::deque<float>().swap(itsGeomHeightData);
  std::deque<float>().swap(itsWindSpeedData);
  std::deque<float>().swap(itsWindDirectionData);
  std::deque<float>().swap(itsWindComponentUData);
  std::deque<float>().swap(itsWindComponentVData);
  std::deque<float>().swap(itsWindVectorData);

  fPressureDataAvailable = false;
  fHeightDataAvailable = false;
}

bool NFmiSoundingData::ModifyT2DryAdiapaticBelowGivenP(double P, double T)
{
  if (P == kFloatMissing || T == kFloatMissing)
    return false;

  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &tV = GetParamData(kFmiTemperature);
  if (pV.size() > 0 && pV.size() == tV.size())
  {
    unsigned int ssize = static_cast<unsigned int>(pV.size());
    float wantedTPot = static_cast<float>(::T2tpot(T, P));
    float currentP = 1000;
    for (unsigned int i = 0; i < ssize; i++)
    {
      currentP = pV[i];
      if (currentP >= P)
      {  // muutoksia siis tehtiin niin kauan kuin oltiin alle annetun paineen
        float wantedT = static_cast<float>(::Tpot2t(wantedTPot, currentP));
        tV[i] = wantedT;
      }
      else
        break;
    }
    return true;
  }
  return false;
}

bool NFmiSoundingData::ModifyTd2MixingRatioBelowGivenP(double P, double T, double Td)
{
  if (P == kFloatMissing || Td == kFloatMissing)
    return false;

  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &tV = GetParamData(kFmiTemperature);
  std::deque<float> &tdV = GetParamData(kFmiDewPoint);
  if (pV.size() > 0 && pV.size() == tV.size() && pV.size() == tdV.size())
  {
    unsigned int ssize = static_cast<unsigned int>(pV.size());

    float wantedMixRatio = static_cast<float>(::CalcMixingRatio(T, Td, P));
    for (unsigned int i = 0; i < ssize; i++)
    {
      float currentP = pV[i];
      if (currentP >= P)
      {  // muutoksia siis tehtiin niin kauan kuin oltiin alle annetun paineen
        float wantedTd = static_cast<float>(::CalcDewPoint(tV[i], wantedMixRatio, currentP));
        tdV[i] = wantedTd;
      }
      else
        break;
    }
    return true;
  }
  return false;
}

bool NFmiSoundingData::ModifyTd2MoistAdiapaticBelowGivenP(double P, double Td)
{
  if (P == kFloatMissing || Td == kFloatMissing)
    return false;

  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &tdV = GetParamData(kFmiDewPoint);
  if (pV.size() > 0 && pV.size() == tdV.size())
  {
    unsigned int ssize = static_cast<unsigned int>(pV.size());

    float AOS = static_cast<float>(NFmiSoundingFunctions::OS(Td, P));
    float currentP = 1000;
    for (unsigned int i = 0; i < ssize; i++)
    {
      currentP = pV[i];
      if (currentP >= P)
      {  // muutoksia siis tehtiin niin kauan kuin oltiin alle annetun paineen
        float ATSA = static_cast<float>(NFmiSoundingFunctions::TSA(AOS, currentP));
        tdV[i] = ATSA;
      }
      else
        break;
    }
    return true;
  }
  return false;
}

static float FixCircularValues(float theValue, float modulorValue)
{
  if (theValue != kFloatMissing)
  {
    if (theValue < 0)
      return modulorValue - ::fmod(-theValue, modulorValue);
    else
      return ::fmod(theValue, modulorValue);
  }
  return theValue;
}

static float FixValuesWithLimits(float theValue, float minValue, float maxValue)
{
  if (theValue != kFloatMissing)
  {
    if (minValue != kFloatMissing)
      theValue = FmiMax(theValue, minValue);
    if (maxValue != kFloatMissing)
      theValue = FmiMin(theValue, maxValue);
  }
  return theValue;
}

bool NFmiSoundingData::Add2ParamAtNearestP(float P,
                                           FmiParameterName parId,
                                           float addValue,
                                           float minValue,
                                           float maxValue,
                                           bool fCircularValue)
{
  if (P == kFloatMissing)
    return false;

  std::deque<float> &pV = GetParamData(kFmiPressure);
  std::deque<float> &paramV = GetParamData(parId);
  if (pV.size() > 0 && pV.size() == paramV.size())
  {
    float currentP = kFloatMissing;
    float currentParam = kFloatMissing;
    float closestPdiff = kFloatMissing;
    unsigned int closestIndex = 0;
    unsigned int ssize = static_cast<unsigned int>(pV.size());
    for (unsigned int i = 0; i < ssize; i++)
    {
      currentP = pV[i];
      currentParam = paramV[i];
      if (currentP != kFloatMissing && currentParam != kFloatMissing)
      {
        if (closestPdiff > ::fabs(P - currentP))
        {
          closestPdiff = ::fabs(P - currentP);
          closestIndex = i;
        }
      }
      if (currentP < P && closestPdiff != kFloatMissing)
        break;
    }

    if (closestPdiff != kFloatMissing)
    {
      float closestParamValue = paramV[closestIndex];
      closestParamValue += addValue;
      if (fCircularValue)
        closestParamValue = ::FixCircularValues(closestParamValue, maxValue);
      else
        closestParamValue = ::FixValuesWithLimits(closestParamValue, minValue, maxValue);

      paramV[closestIndex] = closestParamValue;
      return true;
    }
  }
  return false;
}

static float CalcU(float WS, float WD)
{
  if (WD ==
      999)  // jos tuulensuunta on vaihtelevaa (999), palautetaan 0 arvo (voisi olla myös puuttuvaa)
    return 0.f;
  float value = kFloatMissing;
  if (WS != kFloatMissing && WD != kFloatMissing)
  {
    value = WS * sin(((static_cast<int>(180 + WD) % 360) / 360.f) *
                     (2.f * static_cast<float>(kPii)));  // huom! tuulen suunta pitää ensin kääntää
                                                         // 180 astetta ja sitten muuttaa
                                                         // radiaaneiksi kulma/360 * 2*pii
  }
  return value;
}

static float CalcV(float WS, float WD)
{
  if (WD ==
      999)  // jos tuulensuunta on vaihtelevaa (999), palautetaan 0 arvo (voisi olla myös puuttuvaa)
    return 0;
  float value = kFloatMissing;
  if (WS != kFloatMissing && WD != kFloatMissing)
  {
    value = WS * cos(((static_cast<int>(180 + WD) % 360) / 360.f) *
                     (2.f * static_cast<float>(kPii)));  // huom! tuulen suunta pitää ensin kääntää
                                                         // 180 astetta ja sitten muuttaa
                                                         // radiaaneiksi kulma/360 * 2*pii
  }
  return value;
}

void NFmiSoundingData::UpdateUandVParams(void)
{
  std::deque<float> &wsV = GetParamData(kFmiWindSpeedMS);
  std::deque<float> &wdV = GetParamData(kFmiWindDirection);
  std::deque<float> &uV = GetParamData(kFmiWindUMS);
  std::deque<float> &vV = GetParamData(kFmiWindVMS);
  if (wsV.size() > 0 && wsV.size() == wdV.size() && wsV.size() == uV.size() &&
      wsV.size() == vV.size())
  {
    unsigned int ssize = static_cast<unsigned int>(wsV.size());
    for (unsigned int i = 0; i < ssize; i++)
    {
      uV[i] = ::CalcU(wsV[i], wdV[i]);
      vV[i] = ::CalcV(wsV[i], wdV[i]);
    }
  }
}

// tarkistaa onko kyseisellä ajanhetkellä ja asemalla ei puuttuvaa luotaus-dataa
bool NFmiSoundingData::HasRealSoundingData(NFmiFastQueryInfo &theSoundingLevelInfo)
{
  if (theSoundingLevelInfo.PressureDataAvailable() || theSoundingLevelInfo.HeightDataAvailable())
  {
    std::vector<FmiParameterName> checkedParams;
    checkedParams.push_back(kFmiTemperature);
    checkedParams.push_back(kFmiDewPoint);
    checkedParams.push_back(kFmiWindDirection);
    checkedParams.push_back(kFmiWindSpeedMS);
    checkedParams.push_back(kFmiPressure);
    for (size_t i = 0; i < checkedParams.size(); i++)
    {
      if (theSoundingLevelInfo.Param(checkedParams[i]))
      {
        int cc = 0;
        for (theSoundingLevelInfo.ResetLevel(); theSoundingLevelInfo.NextLevel(); cc++)
        {
          if (theSoundingLevelInfo.FloatValue() != kFloatMissing)
            return true;  // jos miltään alku leveliltä löytyy yhtään korkeusdataa, on käyrä
                          // 'piirrettävissä'
          if (cc > 10)    // pitää löytyä dataa 10 ensimmäisen kerroksen aikana
            break;
        }
        cc = 0;  // käydään dataa läpi myös toisesta päästä, jos ei löytynyt
        for (theSoundingLevelInfo.LastLevel(); theSoundingLevelInfo.PreviousLevel(); cc++)
        {
          if (theSoundingLevelInfo.FloatValue() != kFloatMissing)
            return true;  // jos miltään alku leveliltä löytyy yhtään korkeusdataa, on käyrä
                          // 'piirrettävissä'
          if (cc > 10)    // pitää löytyä dataa 10 ensimmäisen kerroksen aikana
            break;
        }
      }
    }
  }
  return false;
}

// SHOW Showalter index
// SHOW	= T500 - Tparcel
// T500 = Temperature in Celsius at 500 mb
// Tparcel = Temperature in Celsius at 500 mb of a parcel lifted from 850 mb
double NFmiSoundingData::CalcSHOWIndex(void)
{
  double indexValue = kFloatMissing;
  double T_850 = GetValueAtPressure(kFmiTemperature, 850);
  double Td_850 = GetValueAtPressure(kFmiDewPoint, 850);
  if (T_850 != kFloatMissing && Td_850 != kFloatMissing)
  {
    double Tparcel_from850to500 = CalcTOfLiftedAirParcel(T_850, Td_850, 850, 500);
    // 3. SHOW = T500 - Tparcel_from850to500
    double T500 = GetValueAtPressure(kFmiTemperature, 500);
    if (T500 != kFloatMissing && Tparcel_from850to500 != kFloatMissing)
      indexValue = T500 - Tparcel_from850to500;
  }
  return indexValue;
}

// LIFT Lifted index
// LIFT	= T500 - Tparcel
// T500 = temperature in Celsius of the environment at 500 mb
// Tparcel = 500 mb temperature in Celsius of a lifted parcel with the average pressure,
//			 temperature, and dewpoint of the layer 500 m above the surface.
double NFmiSoundingData::CalcLIFTIndex(void)
{
  double indexValue = kFloatMissing;
  double P_500m_avg = kFloatMissing;
  double T_500m_avg = kFloatMissing;
  double Td_500m_avg = kFloatMissing;

  // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
  double h1 = 0 + ZeroHeight();
  double h2 = 500 + ZeroHeight();

  if (CalcLCLAvgValues(h1, h2, T_500m_avg, Td_500m_avg, P_500m_avg, false))
  {
    double Tparcel_from500mAvgTo500 =
        CalcTOfLiftedAirParcel(T_500m_avg, Td_500m_avg, P_500m_avg, 500);
    // 3. LIFT	= T500 - Tparcel_from500mAvgTo500
    double T500 = GetValueAtPressure(kFmiTemperature, 500);
    if (T500 != kFloatMissing && Tparcel_from500mAvgTo500 != kFloatMissing)
      indexValue = T500 - Tparcel_from500mAvgTo500;
  }
  return indexValue;
}

// KINX K index
// KINX = ( T850 - T500 ) + TD850 - ( T700 - TD700 )
//	T850 = Temperature in Celsius at 850 mb
//	T500 = Temperature in Celsius at 500 mb
//	TD850 = Dewpoint in Celsius at 850 mb
//	T700 = Temperature in Celsius at 700 mb
//	TD700 = Dewpoint in Celsius at 700 mb
double NFmiSoundingData::CalcKINXIndex(void)
{
  double T850 = GetValueAtPressure(kFmiTemperature, 850);
  double T500 = GetValueAtPressure(kFmiTemperature, 500);
  double TD850 = GetValueAtPressure(kFmiDewPoint, 850);
  double T700 = GetValueAtPressure(kFmiTemperature, 700);
  double TD700 = GetValueAtPressure(kFmiDewPoint, 700);
  if (T850 != kFloatMissing && T500 != kFloatMissing && TD850 != kFloatMissing &&
      T700 != kFloatMissing && TD700 != kFloatMissing)
    return (T850 - T500) + TD850 - (T700 - TD700);
  return kFloatMissing;
}

// CTOT	Cross Totals index
//	CTOT	= TD850 - T500
//		TD850 	= Dewpoint in Celsius at 850 mb
//		T500 	= Temperature in Celsius at 500 mb
double NFmiSoundingData::CalcCTOTIndex(void)
{
  double T500 = GetValueAtPressure(kFmiTemperature, 500);
  double TD850 = GetValueAtPressure(kFmiDewPoint, 850);
  if (T500 != kFloatMissing && TD850 != kFloatMissing)
    return (TD850 - T500);
  return kFloatMissing;
}

// VTOT	Vertical Totals index
//	VTOT	= T850 - T500
//		T850	= Temperature in Celsius at 850 mb
//		T500	= Temperature in Celsius at 500 mb
double NFmiSoundingData::CalcVTOTIndex(void)
{
  double T500 = GetValueAtPressure(kFmiTemperature, 500);
  double T850 = GetValueAtPressure(kFmiTemperature, 850);
  if (T500 != kFloatMissing && T850 != kFloatMissing)
    return (T850 - T500);
  return kFloatMissing;
}

// TOTL	Total Totals index
//	TOTL	= ( T850 - T500 ) + ( TD850 - T500 )
//		T850 	= Temperature in Celsius at 850 mb
//		TD850	= Dewpoint in Celsius at 850 mb
//		T500 	= Temperature in Celsius at 500 mb
double NFmiSoundingData::CalcTOTLIndex(void)
{
  double T850 = GetValueAtPressure(kFmiTemperature, 850);
  double T500 = GetValueAtPressure(kFmiTemperature, 500);
  double TD850 = GetValueAtPressure(kFmiDewPoint, 850);
  if (T850 != kFloatMissing && T500 != kFloatMissing && TD850 != kFloatMissing)
    return (T850 - T500) + (TD850 - T500);
  return kFloatMissing;
}

// HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
bool NFmiSoundingData::GetValuesNeededInLCLCalculations(FmiLCLCalcType theLCLCalcType,
                                                        double &T,
                                                        double &Td,
                                                        double &P)
{
  // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
  double h1 = 0 + ZeroHeight();
  double h2 = 500 + ZeroHeight();

  bool status = false;
  if (theLCLCalcType == kLCLCalc500m)
    status = CalcLCLAvgValues(h1, h2, T, Td, P, false);
  else if (theLCLCalcType == kLCLCalc500m2)
    status = CalcLCLAvgValues(h1, h2, T, Td, P, true);
  else if (theLCLCalcType == kLCLCalcMostUnstable)
  {
    double maxThetaE = 0;
    status = FindHighestThetaE(
        T, Td, P, maxThetaE, 500);  // rajoitetaan max thetan etsintä 500 mb:en asti
  }

  if (status == false)  // jos muu ei auta, laske pinta suureiden avulla
  {
    P = 1100;
    if (!GetValuesStartingLookingFromPressureLevel(
            T, Td, P))  // sitten lähimmät pinta arvot, jotka joskus yli 500 m
      return false;
  }
  return true;
}

// LCL-levelin painepinnan lasku käyttäen luotauksen haluttuja arvoja
double NFmiSoundingData::CalcLCLPressureLevel(FmiLCLCalcType theLCLCalcType)
{
  // 1. calc T,Td,P values from 500 m layer avg or surface values
  double T = kFloatMissing, Td = kFloatMissing, P = kFloatMissing;
  if (!GetValuesNeededInLCLCalculations(theLCLCalcType, T, Td, P))
    return kFloatMissing;

  return CalcLCLPressure(T, Td, P);
}

// Claculates LCL (Lifted Condensation Level)
// halutulla tavalla
double NFmiSoundingData::CalcLCLIndex(FmiLCLCalcType theLCLCalcType)
{
  return CalcLCLPressureLevel(theLCLCalcType);
}
// palauttaa LCL:n korkeuden metreissä
double NFmiSoundingData::CalcLCLHeightIndex(FmiLCLCalcType theLCLCalcType)
{
  return GetValueAtPressure(kFmiGeomHeight,
                            static_cast<float>(CalcLCLPressureLevel(theLCLCalcType)));
}

// Claculates LFC (Level of Free Convection)
// which is the level above which the lifted parcel is warmer than environment
// parcel is avg from 500 m layer at surface
// HUOM! Lisäsin myös EL laskun eli EL on korkeus millä nostettu ilmapaketti muuttuu
// jälleen kylmemmäksi kuin ympäristö (jos se koskaan oli lämpimämpää)
// Tiedän tämä on ikävä kaksi vastuuta yhdellä metodilla, joista toinen ei edes näy metodin nimessä.
double NFmiSoundingData::CalcLFCIndex(FmiLCLCalcType theLCLCalcType, double &EL)
{
  // 1. calc T,Td,P values from 500 m layer avg or surface values
  double T = kFloatMissing, Td = kFloatMissing, P = kFloatMissing;
  if (!GetValuesNeededInLCLCalculations(theLCLCalcType, T, Td, P))
    return kFloatMissing;

  double LCLValue = CalcLCLPressure(T, Td, P);
  // 2. lift parcel until its warmer than environment
  // 2.1 first adiabatically till LCL and than moist adiabatically
  // iterate with CalcTOfLiftedAirParcel from 500 m avg P to next sounding pressure level
  // until T-parcel is warmer than T at that pressure level in sounding
  std::deque<float> &pValues = GetParamData(kFmiPressure);
  std::deque<float> &tValues = GetParamData(kFmiTemperature);
  size_t ssize = pValues.size();

  double foundPValue = kFloatMissing;
  double durrentDiff = 0;  // ilmapaketin ja ympäristön T ero
  double lastDiff = 0;     // ilmapaketin ja ympäristön T ero viime kierroksella
  for (size_t i = 0; i < ssize; i++)
  {
    if (pValues[i] != kFloatMissing &&
        pValues[i] <= P)  // aloitetaan LFC etsintä vasta 'aloitus' korkeuden jälkeen
    {
      if (tValues[i] != kFloatMissing)  // kaikilla painepinnoilla ei ole lämpötilaa
      {
        double TofLiftedParcer = CalcTOfLiftedAirParcel(T, Td, P, pValues[i]);

        durrentDiff = TofLiftedParcer - tValues[i];
        if (durrentDiff > 0 && foundPValue == kFloatMissing)  // vain alin korkeus talteen
          foundPValue = pValues[i];
        if (durrentDiff < 0 && lastDiff > 0)  // jos siis paketti muuttui lämpimämmästä kylmemmäksi
                                              // (ympäristöön verrattuna, ota talteen korkeus)
          EL = pValues[i];
        lastDiff = durrentDiff;
      }
    }
  }
  if (foundPValue != kFloatMissing && LCLValue < foundPValue)
    foundPValue = LCLValue;  // LFC:n pitää olla ainakin LCL korkeudessa tai korkeammalla eli kun
                             // paineesta kysymys LCL >= LFC
  return foundPValue;
}

// palauttaa LFC:n ja  EL:n korkeuden metreissä
double NFmiSoundingData::CalcLFCHeightIndex(FmiLCLCalcType theLCLCalcType, double &ELheigth)
{
  double tmpValue = CalcLFCIndex(theLCLCalcType, ELheigth);
  ELheigth = GetValueAtPressure(kFmiGeomHeight, static_cast<float>(ELheigth));
  return GetValueAtPressure(kFmiGeomHeight, static_cast<float>(tmpValue));
}

// Calculates CAPE (500 m mix)
// theHeightLimit jos halutaan, voidaan cape lasku rajoittaa alle jonkin korkeus arvon (esim. 3000
// m)
double NFmiSoundingData::CalcCAPE500Index(FmiLCLCalcType theLCLCalcType, double theHeightLimit)
{
  // 1. calc T,Td,P values from 500 m layer avg or surface values
  double T = kFloatMissing, Td = kFloatMissing, P = kFloatMissing;
  if (!GetValuesNeededInLCLCalculations(theLCLCalcType, T, Td, P))
    return kFloatMissing;

  // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
  if (theHeightLimit != kFloatMissing)
    theHeightLimit += ZeroHeight();

  std::deque<float> &pValues = GetParamData(kFmiPressure);
  std::deque<float> &tValues = GetParamData(kFmiTemperature);
  size_t ssize = pValues.size();
  double CAPE = 0;
  double g = 9.81;  // acceleration by gravity
  double Tparcel = 0;
  double Tenvi = 0;
  double currentZ = 0;
  double lastZ = kFloatMissing;
  double deltaZ = kFloatMissing;
  double TK = 273.15;
  for (size_t i = 0; i < ssize; i++)
  {
    if (pValues[i] != kFloatMissing &&
        pValues[i] < P)  // aloitetaan LFC etsintä vasta 'aloitus' korkeuden jälkeen
    {
      if (tValues[i] != kFloatMissing)  // kaikilla painepinnoilla ei ole lämpötilaa
      {
        double TofLiftedParcer = CalcTOfLiftedAirParcel(T, Td, P, pValues[i]);
        currentZ = GetValueAtPressure(kFmiGeomHeight, pValues[i]);  // interpoloidaan jos tarvis

        // integrate here if T parcel is greater than T environment
        if (TofLiftedParcer > tValues[i] && currentZ != kFloatMissing && lastZ != kFloatMissing)
        {
          Tparcel = TofLiftedParcer + TK;
          Tenvi = tValues[i] + TK;
          deltaZ = currentZ - lastZ;
          CAPE += g * deltaZ * ((Tparcel - Tenvi) / Tenvi);
        }
        lastZ = currentZ;

        if (theHeightLimit != kFloatMissing && currentZ > theHeightLimit)
          break;  // lopetetaan laskut jos ollaan menty korkeus rajan yli, kun ensin on laskettu
                  // tämä siivu vielä mukaan
      }
    }
  }
  return CAPE;
}

// Calculates CAPE in layer between two temperatures given
double NFmiSoundingData::CalcCAPE_TT_Index(FmiLCLCalcType theLCLCalcType, double Thigh, double Tlow)
{
  // 1. calc T,Td,P values from 500 m layer avg or surface values
  double T = kFloatMissing, Td = kFloatMissing, P = kFloatMissing;
  if (!GetValuesNeededInLCLCalculations(theLCLCalcType, T, Td, P))
    return kFloatMissing;

  std::deque<float> &pValues = GetParamData(kFmiPressure);
  std::deque<float> &tValues = GetParamData(kFmiTemperature);
  size_t ssize = pValues.size();
  double CAPE = 0;
  double g = 9.81;  // acceleration by gravity
  double Tparcel = 0;
  double Tenvi = 0;
  double currentZ = 0;
  double lastZ = kFloatMissing;
  double deltaZ = kFloatMissing;
  double TK = 273.15;
  for (size_t i = 0; i < ssize; i++)
  {
    if (pValues[i] != kFloatMissing &&
        pValues[i] < P)  // aloitetaan LFC etsintä vasta 'aloitus' korkeuden jälkeen
    {
      if (tValues[i] != kFloatMissing)  // kaikilla painepinnoilla ei ole lämpötilaa
      {
        currentZ = GetValueAtPressure(kFmiGeomHeight, pValues[i]);  // interpoloidaan jos tarvis
        if (Tlow < tValues[i] && Thigh > tValues[i])
        {
          double TofLiftedParcer = CalcTOfLiftedAirParcel(T, Td, P, pValues[i]);

          // integrate here if T parcel is greater than T environment
          if (TofLiftedParcer > tValues[i] && currentZ != kFloatMissing && lastZ != kFloatMissing)
          {
            Tparcel = TofLiftedParcer + TK;
            Tenvi = tValues[i] + TK;
            deltaZ = currentZ - lastZ;
            CAPE += g * deltaZ * ((Tparcel - Tenvi) / Tenvi);
          }
        }
        lastZ = currentZ;
      }
    }
  }
  return CAPE;
}

// Calculates CIN
// first layer of negative (TP - TE (= T-parcel - T-envi)) unless its the last also
double NFmiSoundingData::CalcCINIndex(FmiLCLCalcType theLCLCalcType)
{
  // 1. calc T,Td,P values from 500 m layer avg or surface values
  double T = kFloatMissing, Td = kFloatMissing, P = kFloatMissing;
  if (!GetValuesNeededInLCLCalculations(theLCLCalcType, T, Td, P))
    return kFloatMissing;

  std::deque<float> &pValues = GetParamData(kFmiPressure);
  std::deque<float> &tValues = GetParamData(kFmiTemperature);
  size_t ssize = pValues.size();
  double CIN = 0;
  double g = 9.81;  // acceleration by gravity
  double Tparcel = 0;
  double Tenvi = 0;
  double currentZ = 0;
  double lastZ = kFloatMissing;
  double deltaZ = kFloatMissing;
  double TK = 273.15;  // celsius/kelvin change
  bool firstCinLayerFound = false;
  bool capeLayerFoundAfterCin = false;
  for (size_t i = 0; i < ssize; i++)
  {
    if (pValues[i] != kFloatMissing &&
        pValues[i] < P)  // aloitetaan LFC etsintä vasta 'aloitus' korkeuden jälkeen
    {
      if (tValues[i] != kFloatMissing)  // kaikilla painepinnoilla ei ole lämpötilaa
      {
        double TofLiftedParcer = CalcTOfLiftedAirParcel(T, Td, P, pValues[i]);
        currentZ = GetValueAtPressure(kFmiGeomHeight, pValues[i]);  // interpoloidaan jos tarvis

        // integrate here if T parcel is less than T environment
        if (TofLiftedParcer <= tValues[i] && currentZ != kFloatMissing && lastZ != kFloatMissing)
        {
          Tparcel = TofLiftedParcer + TK;
          Tenvi = tValues[i] + TK;
          deltaZ = currentZ - lastZ;
          CIN += g * deltaZ * ((Tparcel - Tenvi) / Tenvi);
          firstCinLayerFound = true;
        }
        else if (firstCinLayerFound)
        {
          capeLayerFoundAfterCin = true;
          break;  // jos 1. CIN layer on löydetty ja osutaan CAPE layeriin, lopetetaan
        }
        lastZ = currentZ;
      }
    }
  }
  if (!(firstCinLayerFound && capeLayerFoundAfterCin))
    CIN = 0;
  return CIN;
}

// startH and endH are in kilometers
// HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
double NFmiSoundingData::CalcWindBulkShearComponent(double startH,
                                                    double endH,
                                                    FmiParameterName theParId)
{
  // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
  startH += ZeroHeight() / 1000.;  // zero height pitää muuttaa tässä metreistä kilometreiksi!
  endH += ZeroHeight() / 1000.;    // zero height pitää muuttaa tässä metreistä kilometreiksi!

  float startValue = GetValueAtHeight(theParId, static_cast<float>(startH * 1000));
  float endValue = GetValueAtHeight(theParId, static_cast<float>(endH * 1000));
  float shearComponent = endValue - startValue;
  if (endValue == kFloatMissing || startValue == kFloatMissing)
    shearComponent = kFloatMissing;
  return shearComponent;
}

// startH and endH are in kilometers
// HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
double NFmiSoundingData::CalcThetaEDiffIndex(double startH, double endH)
{
  // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
  startH += ZeroHeight() / 1000.;  // zero height pitää muuttaa tässä metreistä kilometreiksi!
  endH += ZeroHeight() / 1000.;    // zero height pitää muuttaa tässä metreistä kilometreiksi!

  float startT = GetValueAtHeight(kFmiTemperature, static_cast<float>(startH * 1000));
  float endT = GetValueAtHeight(kFmiTemperature, static_cast<float>(endH * 1000));
  float startTd = GetValueAtHeight(kFmiDewPoint, static_cast<float>(startH * 1000));
  float endTd = GetValueAtHeight(kFmiDewPoint, static_cast<float>(endH * 1000));
  float startP = GetValueAtHeight(kFmiPressure, static_cast<float>(startH * 1000));
  float endP = GetValueAtHeight(kFmiPressure, static_cast<float>(endH * 1000));
  if (startT == kFloatMissing || startTd == kFloatMissing || startP == kFloatMissing)
    return kFloatMissing;
  double startThetaE = CalcThetaE(startT, startTd, startP);
  if (endT == kFloatMissing || endTd == kFloatMissing || endP == kFloatMissing)
    return kFloatMissing;
  double endThetaE = CalcThetaE(endT, endTd, endP);
  if (startThetaE == kFloatMissing || endThetaE == kFloatMissing)
    return kFloatMissing;
  double diffValue = startThetaE - endThetaE;
  return diffValue;
}

// Calculates Bulk Shear between two given layers
// startH and endH are in kilometers
double NFmiSoundingData::CalcBulkShearIndex(double startH, double endH)
{
  // HUOM! asema korkeus otetaan huomioon CalcWindBulkShearComponent-funktiossa.
  double uTot = CalcWindBulkShearComponent(startH, endH, kFmiWindUMS);
  double vTot = CalcWindBulkShearComponent(startH, endH, kFmiWindVMS);
  double BS = ::sqrt(uTot * uTot + vTot * vTot);
  if (uTot == kFloatMissing || vTot == kFloatMissing)
    BS = kFloatMissing;
  return BS;
}

/* // **********  SRH calculation help from Pieter Groenemeijer ******************

Some tips here on how tyo calculate storm-relative helciity

How to calculate storm-relative helicity

Integrate the following from p = p_surface to p = p_top (or in case of height coordinates from h_surface to h_top):

storm_rel_helicity -= ((u_ID-u[p])*(v[p]-v[p+1]))-((v_ID - v[p])*(u[p]-u[p+1]));

Here, u_ID and v_ID are the forecast storm motion vectors calculated with the so-called ID-method. These can be calculated as follows:

where

/average wind
u0_6 = average 0_6 kilometer u-wind component
v0_6 = average 0_6 kilometer v-wind component
(you should use a pressure-weighted average in case you work with height coordinates)

/shear
shr_0_6_u = u_6km - u_surface;
shr_0_6_v = v_6km - v_surface;

/ shear unit vector
shr_0_6_u_n = shr_0_6_u / ((shr_0_6_u^2 + shr_0_6_v^2)**0.5);
shr_0_6_v_n = shr_0_6_v / ((shr_0_6_u^2 + shr_0_6_v^2)** 0.5);

/id-vector components
u_ID = u0_6 + shr_0_6_v_n * 7.5;
v_ID = v0_6 - shr_0_6_u_n * 7.5;

(7.5 are meters per second... watch out when you work with knots instead)

*/  // **********
                                                                                    // SRH
                                                                                    // calculation
                                                                                    // help from
                                                                                    // Pieter
                                                                                    // Groenemeijer
// ******************

// I use same variable names as with the Pieters help evem though calculations are not
// restricted to 0-6 km layer but they can be from any layer

// shear
// startH and endH are in kilometers
double NFmiSoundingData::CalcBulkShearIndex(double startH, double endH, FmiParameterName theParId)
{
  // HUOM! asema korkeus otetaan huomioon CalcWindBulkShearComponent-funktiossa.
  double shr_0_6_component = CalcWindBulkShearComponent(startH, endH, theParId);
  return shr_0_6_component;
}

void NFmiSoundingData::Calc_U_V_helpers(double &shr_0_6_u_n,
                                        double &shr_0_6_v_n,
                                        double &u0_6,
                                        double &v0_6)
{
  // HUOM! asema korkeus otetaan huomioon CalcWindBulkShearComponent-funktiossa.
  double shr_0_6_u = CalcWindBulkShearComponent(0, 6, kFmiWindUMS);
  double shr_0_6_v = CalcWindBulkShearComponent(0, 6, kFmiWindVMS);
  shr_0_6_u_n = NFmiSoundingFunctions::Calc_shear_unit_v_vector(shr_0_6_u, shr_0_6_v);
  shr_0_6_v_n = Calc_shear_unit_v_vector(shr_0_6_v, shr_0_6_u);

  // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
  double h1 = 0 + ZeroHeight();     // 0 m + aseman korkeus
  double h2 = 6000 + ZeroHeight();  // 6000 m + aseman korkeus
  CalcAvgWindComponentValues(h1, h2, u0_6, v0_6);
}

// lasketaan u ja v ID:t 0-6km layerissä
// tämä on hodografissa 'left'
void NFmiSoundingData::Calc_U_and_V_IDs_left(double &u_ID, double &v_ID)
{
  double shr_0_6_u_n = 0, shr_0_6_v_n = 0, u0_6 = 0, v0_6 = 0;
  Calc_U_V_helpers(shr_0_6_u_n, shr_0_6_v_n, u0_6, v0_6);

  u_ID = NFmiSoundingFunctions::CalcU_ID_left(u0_6, shr_0_6_v_n);
  v_ID = NFmiSoundingFunctions::CalcV_ID_left(v0_6, shr_0_6_u_n);
}

// lasketaan u ja v ID:t 0-6km layerissä
// tämä on hodografissa 'right'
void NFmiSoundingData::Calc_U_and_V_IDs_right(double &u_ID, double &v_ID)
{
  double shr_0_6_u_n = 0, shr_0_6_v_n = 0, u0_6 = 0, v0_6 = 0;
  Calc_U_V_helpers(shr_0_6_u_n, shr_0_6_v_n, u0_6, v0_6);

  u_ID = NFmiSoundingFunctions::CalcU_ID_right(u0_6, shr_0_6_v_n);
  v_ID = NFmiSoundingFunctions::CalcV_ID_right(v0_6, shr_0_6_u_n);
}

// lasketaan u ja v mean 0-6km layerissä
void NFmiSoundingData::Calc_U_and_V_mean_0_6km(double &u0_6, double &v0_6)
{
  // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
  double h1 = 0 + ZeroHeight();     // 0 m + aseman korkeus
  double h2 = 6000 + ZeroHeight();  // 6000 m + aseman korkeus
  CalcAvgWindComponentValues(h1, h2, u0_6, v0_6);
}

NFmiString NFmiSoundingData::Get_U_V_ID_IndexText(const NFmiString &theText,
                                                  FmiDirection theStormDirection)
{
  NFmiString str(theText);
  str += "=";
  double u_ID = kFloatMissing;
  double v_ID = kFloatMissing;
  if (theStormDirection == kRight)
    Calc_U_and_V_IDs_right(u_ID, v_ID);
  else if (theStormDirection == kLeft)
    Calc_U_and_V_IDs_left(u_ID, v_ID);
  else
    Calc_U_and_V_mean_0_6km(u_ID, v_ID);

  if (u_ID == kFloatMissing || v_ID == kFloatMissing)
    str += " -/-";
  else
  {
    double WS = ::sqrt(u_ID * u_ID + v_ID * v_ID);
    NFmiWindDirection theDirection(u_ID, v_ID);
    double WD = theDirection.Value();

    if (WD != kFloatMissing)
      str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(WD, 0);
    else
      str += "-";
    str += "/";
    str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(WS, 1);
  }
  return str;
}

// Calculates Storm-Relative Helicity (SRH) between two given layers
// startH and endH are in kilometers
// käytetään muuttujan niminä samoja mitä on Pieterin helpissä, vaikka kyseessä ei olekaan laskut
// layerille 0-6km vaan mille välille tahansa
// HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
double NFmiSoundingData::CalcSRHIndex(double startH, double endH)
{
  std::deque<float> &pV = GetParamData(kFmiPressure);
  if (pV.size() > 0)
  {
    // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
    startH += ZeroHeight() / 1000.;  // zero height pitää muuttaa tässä metreistä kilometreiksi!
    endH += ZeroHeight() / 1000.;  // zero height pitää muuttaa tässä metreistä kilometreiksi!

    size_t ssize = pV.size();
    std::deque<float> &uV = GetParamData(kFmiWindUMS);
    std::deque<float> &vV = GetParamData(kFmiWindVMS);
    std::deque<float> &tV = GetParamData(kFmiTemperature);
    std::deque<float> &tdV = GetParamData(kFmiDewPoint);

    double u_ID = kFloatMissing;
    double v_ID = kFloatMissing;
    Calc_U_and_V_IDs_right(u_ID, v_ID);
    double SRH = 0;
    double lastU = kFloatMissing;
    double lastV = kFloatMissing;
    bool foundAnyData = false;
    for (size_t i = 0; i < ssize; i++)
    {
      double p = pV[i];
      double u = uV[i];
      double v = vV[i];
      double t = tV[i];
      double td = tdV[i];
      if (p != kFloatMissing && u != kFloatMissing && v != kFloatMissing && t != kFloatMissing &&
          td != kFloatMissing)
      {
        double z = GetValueAtPressure(kFmiGeomHeight, static_cast<float>(p));
        if (z != kFloatMissing && z >= startH * 1000 && z <= endH * 1000)
        {
          if (lastU != kFloatMissing && lastV != kFloatMissing)
          {
            SRH -= NFmiSoundingFunctions::CalcSRH(u_ID, v_ID, lastU, u, lastV, v);
            foundAnyData = true;
          }
        }
        lastU = u;
        lastV = v;
      }
    }
    return foundAnyData ? SRH : kFloatMissing;
  }
  return kFloatMissing;
}

// theH in meters
double NFmiSoundingData::CalcWSatHeightIndex(double theH)
{
  // HUOM! Pitää ottaa huomioon aseman korkeus kun tehdään laskuja!!!!
  theH += ZeroHeight();

  return GetValueAtHeight(kFmiWindSpeedMS, static_cast<float>(theH));
}

// Laske ilmapaketin lämpötila nostamalla ilmapakettia
// Nosta kuiva-adiapaattisesti LCL-korkeuteen ja siitä eteenpäin kostea-adiapaattisesti
double NFmiSoundingData::CalcTOfLiftedAirParcel(double T, double Td, double fromP, double toP)
{
  // 1. laske LCL kerroksen paine alkaen fromP:stä
  double lclPressure = CalcLCLPressure(T, Td, fromP);
  if (lclPressure != kFloatMissing)
  {
    // Laske aloitus korkeuden lämpötilan potentiaali lämpötila.
    double TpotStart = NFmiSoundingFunctions::T2tpot(T, fromP);
    if (TpotStart != kFloatMissing)
    {
      if (lclPressure <= toP)  // jos lcl oli yli toP:n mb (eli pienempi kuin toP)
      {                        // nyt riittää pelkkä kuiva-adiapaattinen nosto fromP -> toP
        double Tparcel = NFmiSoundingFunctions::Tpot2t(TpotStart, toP);
        return Tparcel;
      }
      else
      {
        // Laske ilmapaketin lämpö lcl-korkeudella  kuiva-adiapaattisesti nostettuna
        double Tparcel_LCL = NFmiSoundingFunctions::Tpot2t(TpotStart, lclPressure);
        // laske kyseiselle korkeudelle vastaava kostea-adiapaatti arvo
        double TmoistLCL = NFmiSoundingFunctions::CalcMoistT(Tparcel_LCL, lclPressure);
        // kyseinen kostea-adiapaatti pitää konvertoida vielä (ADL-kielestä kopioitua koodia, ks.
        // OS- ja
        // TSA-funtioita) jotenkin 1000 mb:hen.
        if (TmoistLCL != kFloatMissing)
        {
          double AOS = NFmiSoundingFunctions::OS(TmoistLCL, 1000.);
          // Sitten lasketaan lämpötila viimein 500 mb:hen
          double Tparcel = NFmiSoundingFunctions::TSA(AOS, toP);
          return Tparcel;
        }
      }
    }
  }
  return kFloatMissing;
}
