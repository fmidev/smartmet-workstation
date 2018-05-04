
#include "NFmiAviationStationInfoSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiTEMPCode.h"
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiLocationBag.h>
#include <newbase/NFmiQueryDataUtil.h>
#include <newbase/NFmiStringTools.h>
#include <newbase/NFmiTimeList.h>
#include <newbase/NFmiTotalWind.h>
#include <set>
#include <sstream>

NFmiTEMPCode::NFmiTEMPCode(void)
    : itsOriginalCodeAStr(),
      itsOriginalCodeBStr(),
      itsOriginalCodeCStr(),
      itsOriginalCodeDStr(),
      itsTime(),
      itsStation(),
      fTempMobil(false),
      itsTempStations(0),
      itsUnknownStationLocation(),
      itsLevels()
{
}

NFmiTEMPCode::NFmiTEMPCode(NFmiAviationStationInfoSystem *theTempStations,
                           const NFmiPoint &theUnknownStationLocation)
    : itsOriginalCodeAStr(),
      itsOriginalCodeBStr(),
      itsOriginalCodeCStr(),
      itsOriginalCodeDStr(),
      itsTime(),
      itsStation(),
      fTempMobil(false),
      itsTempStations(theTempStations),
      itsUnknownStationLocation(theUnknownStationLocation),
      itsLevels()
{
}

NFmiTEMPCode::~NFmiTEMPCode(void)
{
}

void NFmiTEMPCode::Clear(void)
{
  itsOriginalCodeAStr.clear();
  itsOriginalCodeBStr.clear();
  itsOriginalCodeCStr.clear();
  itsOriginalCodeDStr.clear();
  itsStation = NFmiStation();  // joku dummy asema vain
  fTempMobil = false;
  itsLevels.clear();
}

static void InsertLevelData(std::map<double, TEMPLevelData> &theLevelMap,
                            const TEMPLevelData &theLevelData)
{
  theLevelMap.insert(std::make_pair(theLevelData.itsPressure, theLevelData));
}

// oletus painelevelit (theLevels) ovat 'nousevassa' järjestyksessä eli pienenevät
// toisin kuin itse level-data, joiden oletetaan alkavan ylhäältä
void NFmiTEMPCode::ForceWantedPressureLevels(NFmiVPlaceDescriptor &theLevels)
{
  size_t levelSize = itsLevels.size();
  if (levelSize <= 2)
    return;
  theLevels.Reset();
  theLevels.Next();
  double currentMainLevel = static_cast<double>(theLevels.Level()->LevelValue());
  std::map<double, TEMPLevelData>::reverse_iterator it = itsLevels.rbegin();
  std::map<double, TEMPLevelData>::reverse_iterator last =
      it;  // tähän sijoitetaan edellisen levelin data
  std::map<double, TEMPLevelData>::reverse_iterator endIt = itsLevels.rend();
  std::map<double, TEMPLevelData> resultLevels;
  InsertLevelData(resultLevels, (*last).second);  // 1. leveli laitetaan aina
  int lastTemperatureDir = 0;     // jos T laskee, tähän -1, jos nousee, 1, jos ei alustettu, 0
  int currentTemperatureDir = 0;  // jos T laskee, tähän -1, jos nousee, 1, jos ei alustettu, 0
  int minDirCounter = 6;
  int T_Dircounter = 0;
  int lastDewPointDir = 0;     // jos T laskee, tähän -1, jos nousee, 1, jos ei alustettu, 0
  int currentDewPointDir = 0;  // jos T laskee, tähän -1, jos nousee, 1, jos ei alustettu, 0
  int Td_Dircounter = 0;
  double lastTZRatio = 0;
  double currentTZRatio = 0;
  double lastTdZRatio = 0;
  double currentTdZRatio = 0;
  int counter = 0;
  do
  {
    counter++;
    lastTemperatureDir = currentTemperatureDir;
    lastDewPointDir = currentDewPointDir;
    last = it;
    ++it;
    double p1 = (*last).second.itsPressure;
    double p2 = (*it).second.itsPressure;
    if (p1 < currentMainLevel && p2 < currentMainLevel)
    {  // molemmat kerrokset ovat jo ylempänä kuin etsitty main level, etsitään sellainen main
       // level, mikä ei ole molempien yläpuolella
      do
      {
        if (theLevels.Next() == false)
          break;
        currentMainLevel = static_cast<double>(theLevels.Level()->LevelValue());
      } while (p1 < currentMainLevel && p2 < currentMainLevel);
    }

    if (p1 >= currentMainLevel && p2 <= currentMainLevel)
    {  // nyt etsitty main level on leveleiden välissä. katsotaan kumpi on lähempänä ja muutetaan se
      // tarvittaessa halutuksi leveliksi
      if (p1 == currentMainLevel || p1 == currentMainLevel)
      {  // ei tarvitse muuta kuin alkaa etsimään seuraavaa leveliä
      }
      else
      {
        double diff1 = ::fabs(currentMainLevel - p1);
        double diff2 = ::fabs(currentMainLevel - p2);
        if (diff1 < diff2)
        {
          (*last).second.itsPressure = currentMainLevel;
          InsertLevelData(resultLevels, (*last).second);
        }
        else
        {
          (*it).second.itsPressure = currentMainLevel;
          InsertLevelData(resultLevels, (*it).second);
        }
      }
      theLevels.Next();
      currentMainLevel = static_cast<double>(theLevels.Level()->LevelValue());
    }
    double z1 = (*last).second.itsHeight;
    int zFix = static_cast<int>(z1 / 1000.);
    double z2 = (*it).second.itsHeight;
    double T1 = (*last).second.itsTemperature;
    double T2 = (*it).second.itsTemperature;
    T_Dircounter++;
    if (T1 >= T2)
      currentTemperatureDir = -1;
    else
      currentTemperatureDir = 1;
    if (lastTemperatureDir != currentTemperatureDir && T_Dircounter > minDirCounter + zFix)
    {
      T_Dircounter = 0;
      InsertLevelData(resultLevels,
                      (*last).second);  // jos lämpötilan suunta vaihtuu, laitetaan leveli talteen
    }

    double Td1 = (*last).second.itsDewPoint;
    double Td2 = (*it).second.itsDewPoint;
    Td_Dircounter++;
    if (Td1 >= Td2)
      currentDewPointDir = -1;
    else
      currentDewPointDir = 1;
    if (lastDewPointDir != currentDewPointDir && Td_Dircounter > (minDirCounter + zFix) * 3)
    {
      Td_Dircounter = 0;
      InsertLevelData(resultLevels,
                      (*last).second);  // jos kastepisteen suunta vaihtuu, laitetaan leveli talteen
    }
    if (z1 != z2)
    {
      currentTZRatio = (T2 - T1) / (z2 - z1);
      if (::fabs(currentTZRatio - lastTZRatio) > 0.22)
      {
        InsertLevelData(resultLevels, (*last).second);  // jos lämpötilan 'kulmakerroin' muuttuu
                                                        // tarpeeksi, laitetaan leveli talteen
        lastTZRatio = currentTZRatio;
      }
      currentTdZRatio = (Td2 - Td1) / (z2 - z1);
      if (::fabs(currentTdZRatio - lastTdZRatio) > 0.22)
      {
        InsertLevelData(resultLevels, (*last).second);  // jos lämpötilan 'kulmakerroin' muuttuu
                                                        // tarpeeksi, laitetaan leveli talteen
        lastTdZRatio = currentTdZRatio;
      }
    }

  } while (it != endIt);
  InsertLevelData(resultLevels, (*it).second);  // viimeinen leveli laitetaan aina
  itsLevels = resultLevels;
}

// Saa A,B,C ja D koodit. Tarkistaa ne ja purkaa. Palauttaa luvun, joka kertoo kuinka monta
// osaa kelpasi (eli oli samalta asemalta, samaan aikaan jne. )
int NFmiTEMPCode::InsertCodeStrings(const std::string &theCodeAStr,
                                    const std::string &theCodeBStr,
                                    const std::string &theCodeCStr,
                                    const std::string &theCodeDStr,
                                    bool fNoEqualSignInCode)
{
  std::string ttStr;
  if (fNoEqualSignInCode)
    ttStr = "TT";
  itsOriginalCodeAStr = ttStr + theCodeAStr;
  itsOriginalCodeBStr = ttStr + theCodeBStr;
  itsOriginalCodeCStr = ttStr + theCodeCStr;
  itsOriginalCodeDStr = ttStr + theCodeDStr;
  fTempMobil = false;
  itsLevels.clear();

  return Decode();
}

int NFmiTEMPCode::Decode(void)
{
  int tempCount = 0;
  if (DecodeA())
  {
    tempCount++;
    if (DecodeB())
    {
      tempCount++;
      if (DecodeC())
      {
        tempCount++;
        if (DecodeD())
        {
          tempCount++;
        }
      }
    }
  }
  //	SortLevels();
  return tempCount;
}

template <typename T>
static T GetValue(const std::string &theValueStr)
{
  std::string tmpStr(theValueStr);
  NFmiStringTools::LowerCase(tmpStr);

  if (tmpStr != "//" && tmpStr != "xx")
    return NFmiStringTools::Convert<T>(tmpStr);
  else
    return static_cast<T>(kFloatMissing);
}

// laskee TnTnTanDnDn ryhmästä T:n ja Td:n. Palauttaa false jos jokin pielessä
static bool GetTandTd(const std::string &theTnTnTanDnDn_str, double &T, double &Td)
{
  T = kFloatMissing;
  Td = kFloatMissing;
  if (theTnTnTanDnDn_str == "/////")
    return true;  // ok, palautetaan puuttuvat arvot

  std::string TnTn_str(theTnTnTanDnDn_str.begin(), theTnTnTanDnDn_str.begin() + 2);
  double TintPart = ::GetValue<double>(TnTn_str);
  std::string Tan_str(theTnTnTanDnDn_str.begin() + 2, theTnTnTanDnDn_str.begin() + 3);
  int TdecimalPart = ::GetValue<int>(Tan_str);
  std::string DD_str(theTnTnTanDnDn_str.begin() + 3, theTnTnTanDnDn_str.begin() + 5);
  double TdDiff = ::GetValue<double>(DD_str);

  if (TintPart != kFloatMissing && TdecimalPart != kFloatMissing)
  {
    if (TdecimalPart % 2 == 0)  // jos parillinen
      T = TintPart + TdecimalPart / 10.;
    else
      T = -(TintPart + TdecimalPart / 10.);
  }

  if (T != kFloatMissing && TdDiff != kFloatMissing)
  {
    if (TdDiff <= 50)
      Td = T - TdDiff / 10.;
    else if (TdDiff >= 56)
      Td = T - (TdDiff - 50.);
    else
      return false;
  }

  return true;
}

static bool GetWDandWS(const std::string &thedddff_Str, double &WD, double &WS, bool fConvertKt2Ms)
{
  WD = kFloatMissing;
  WS = kFloatMissing;
  if (thedddff_Str == "/////")
    return true;  // ok, palautetaan puuttuvat arvot

  // HUOM! jos tuulen suunta tai nopeus puuttuu, niitä voisi hanskata paremmin ja laskea toisen
  // vaikka
  // toinen puuttuisi, nyt molemmat puuttuviksi jos toinen puuttuu (tuuliviiriä ei saa kuitenkaan
  // piirretyksi)
  std::string ddd_str(thedddff_Str.begin(), thedddff_Str.begin() + 3);
  int WDtmp = static_cast<int>(kFloatMissing);
  if (ddd_str.find('/') == std::string::npos)
    WDtmp = ::GetValue<int>(ddd_str);
  std::string ff_str(thedddff_Str.begin() + 3, thedddff_Str.begin() + 5);
  double WStmp = kFloatMissing;
  if (ff_str.find('/') == std::string::npos)
    WStmp = ::GetValue<double>(ff_str);

  if (WDtmp != kFloatMissing && WStmp != kFloatMissing)
  {
    if (WDtmp % 10 == 0 || WDtmp % 5 == 0)  // jos loppuu 0:aan tai 5:een
    {
      WD = WDtmp;
      WS = WStmp;
    }
    else
    {
      WD = WDtmp - 1;
      WS = (WStmp + 100) / 2;  // kt -> m/s konversio nyt jaetaan 2:lla!!!!!
    }
    if (fConvertKt2Ms)
      WS = WS / 2.0;  // 1.94384; // muunnetaan solmut -> m/s
  }

  return true;
}

static double GetMainPressureLevel(int thePident)
{
  switch (thePident)
  {
    case 0:
      return 1000;
    case 92:
      return 925;
    case 85:
      return 850;
    case 70:
      return 700;
    case 50:
      return 500;
    case 40:
      return 400;
    case 30:
      return 300;
    case 25:
      return 250;
    case 20:
      return 200;
    case 15:
      return 150;
    case 10:
      return 100;
    default:
      return kFloatMissing;
  }
}

static double GetMainPressureHeight(int thePident, int theH)
{
  switch (thePident)
  {
    case 0:
      return theH;
    case 92:
      return theH < 300 ? theH + 1000 : theH;
    case 85:
      return theH + 1000;
    case 70:
      return theH > 500 ? theH + 2000 : theH + 3000;
    case 50:
      return theH * 10;
    case 40:
      return theH * 10;
    case 30:
      return theH > 300 ? theH * 10 : (theH + 1000) * 10;
    case 25:
      return theH > 300 ? theH * 10 : (theH + 1000) * 10;
    case 20:
      return theH > 300 ? theH * 10 : (theH + 1000) * 10;
    case 15:
      return (theH + 1000) * 10;
    case 10:
      return (theH + 1000) * 10;
    default:
      return kFloatMissing;
  }
}

static bool GetPandH(const std::string &thePPhhh_Str, double &P, double &h)
{
  P = kFloatMissing;
  h = kFloatMissing;

  std::string PP_str(thePPhhh_Str.begin(), thePPhhh_Str.begin() + 2);
  double PPtmp = ::GetValue<double>(PP_str);
  std::string hhh_str(thePPhhh_Str.begin() + 2, thePPhhh_Str.begin() + 5);
  double hhhtmp = ::GetValue<double>(hhh_str);

  if (PPtmp != kFloatMissing)
    P = ::GetMainPressureLevel(static_cast<int>(PPtmp));
  if (P != kFloatMissing && hhhtmp != kFloatMissing)
    h = ::GetMainPressureHeight(static_cast<int>(PPtmp), static_cast<int>(hhhtmp));

  return true;
}

static bool DecodeHeader(std::stringstream &ssin,
                         NFmiStation &theStation,
                         NFmiMetTime &theTime,
                         bool &fTempMobil,
                         char theWantedSectionChar,
                         bool fInitializeValues,
                         NFmiAviationStationInfoSystem *theTempStations,
                         const NFmiPoint &theUnknownStationLocation,
                         bool &fConvertKt2Ms)
{
  std::string first_Str;  // esim. "USFI99"
  ssin >> first_Str;
  if (first_Str.size() != 6 && std::isdigit(first_Str[0]))
    ssin >> first_Str;  // joskus 1. stringinä on pitkä luku kutem esim. 0000034101 tai lyhyt luku
                        // kuten 999, tämä pitää ohittaa
  bool wyoming_raw_data = false;
  if (::toupper(first_Str[0]) == 'T' &&
      ::toupper(first_Str[1]) == 'T')  // wyoming raw data alkaa TTAA, TTBB jne. osioilla
    wyoming_raw_data = true;
  NFmiStation station;
  if (wyoming_raw_data == false)
  {
    std::string icao_Str;  // esim. "EFKL"
    ssin >> icao_Str;
    station.SetName(icao_Str);
    std::string third_Str;  // esim. "140200"
    ssin >> third_Str;
  }

  std::string MiMiMjMj_Str;
  if (wyoming_raw_data)
    MiMiMjMj_Str = first_Str;
  else
  {
    ssin >> MiMiMjMj_Str;
    if (MiMiMjMj_Str.size() != 4 && MiMiMjMj_Str.size() > 0 && std::isalpha(MiMiMjMj_Str[0]) &&
        MiMiMjMj_Str[0] != 'T')
      ssin >> MiMiMjMj_Str;  // joskus voi olla esim. RRB sana ennen TTxx -stringiä, tällöin vain
                             // luetaan seuraava sana
  }
  if (MiMiMjMj_Str.size() != 4)
    return false;
  bool doTempMobil = false;  // jos false, normaali TEMP, muuten TEMP MOBIL
  bool doShip = false;  // jos false, normaali TEMP, muuten TEMP MOBIL, joka on aseman korkeus info
                        // muuten sama kuin MOBIL
  if (::toupper(MiMiMjMj_Str[0]) == 'I' &&
      ::toupper(MiMiMjMj_Str[1]) == 'I')  // jos stringi muotoa II?? on kyseessä TEMP MOBIL
    doTempMobil = true;
  else if (::toupper(MiMiMjMj_Str[0]) == 'U' &&
           ::toupper(MiMiMjMj_Str[1]) == 'U')  // jos stringi muotoa UU?? on kyseessä SHIP, joka on
                                               // samanlainen kuin TEMP MOBIL
  {
    doTempMobil = true;
    doShip = true;
  }
  if (fInitializeValues)
    fTempMobil = doTempMobil;  // A-osiossa vain talletetaan onko mobiili temp kyseessä
  else
  {  // tarkistetaan että sama temp tyyppi, kuin A-osiossa ollut
    if (fTempMobil != doTempMobil)
      return false;  // pitäisi heittää poikkeus!!
  }

  if (::toupper(MiMiMjMj_Str[2]) == theWantedSectionChar &&
      ::toupper(MiMiMjMj_Str[3]) ==
          theWantedSectionChar)  // stringin pitää olla muotoa ??AA, ??BB, ??CC tai ??DD
  {
    if (doTempMobil)
    {
      std::string D___D_Str;  // esim. "SA_INT"
      ssin >> D___D_Str;
    }
    std::string YYGGId_Str;  // esim. "64022"
    ssin >> YYGGId_Str;
    if (YYGGId_Str.size() != 5)
      return false;
    std::string dayStr(YYGGId_Str.begin(), YYGGId_Str.begin() + 2);
    short day = NFmiStringTools::Convert<short>(dayStr);
    if (day > 50)
    {
      fConvertKt2Ms = true;
      day -= 50;  // jos havaintopäivä > 50, on tuulen nopeus solmuissa (vähennä myös se 50 että
                  // saat päivän)
    }
    NFmiMetTime tempTime;
    if (day > tempTime.GetDay())  // pitää päätellä ollaanko sittenkin edellisessä kuukaudessa (jos
                                  // saatu päivä suurempi kuin nyky päivä)
    {
      NFmiTimePerioid moveByMonth(0, 1, 0, 0, 0, 0);
      tempTime.PreviousMetTime(moveByMonth);
    }
    tempTime.SetDay(day);  // tämä on virhe altis, jos kuukaudessa ei ole näin monta päivää
    short daysInMonth = tempTime.DaysInMonth(tempTime.GetMonth(), tempTime.GetYear());
    if (tempTime.GetDay() > daysInMonth)  // jos ongelmia päivien kanssa siirrytttäessä kuukausissa
      // (esim. 31 ja kuussa ei ole kuin 30 päivää), laitetaan
      // paiväksi suurin sallittu päivä siinä kuussa
      tempTime.SetDay(daysInMonth);

    std::string hourStr(YYGGId_Str.begin() + 2, YYGGId_Str.begin() + 4);
    short hour = NFmiStringTools::Convert<short>(hourStr);  // UTC aikaa tämä havainto tunti
    tempTime.SetHour(hour);
    if (fInitializeValues)
      theTime = tempTime;  // A-osiossa vain talletetaan aika
    else
    {  // tarkistetaan että sama aika, kuin A-osiossa ollut
      if (theTime != tempTime)
        return false;  // pitäisi heittää poikkeus!!
    }

    if (doTempMobil)
    {
      std::string _99LaLaLa_Str;  // esim. "99613" // muuttujassa alaviiva alku, koska numero ei voi
                                  // aloittaa muuttujaa
      ssin >> _99LaLaLa_Str;
      if (_99LaLaLa_Str.size() != 5)
        return false;
      double lat = NFmiStringTools::Convert<double>(
          std::string(_99LaLaLa_Str.begin() + 2, _99LaLaLa_Str.begin() + 4));
      double latTenth = NFmiStringTools::Convert<double>(
                            std::string(_99LaLaLa_Str.begin() + 4, _99LaLaLa_Str.begin() + 5)) /
                        6.;
      lat += latTenth;
      std::string QcLoLoLoLo_Str;  // esim. "10280"
      ssin >> QcLoLoLoLo_Str;
      if (QcLoLoLoLo_Str.size() != 5)
        return false;
      double lon = NFmiStringTools::Convert<double>(
          std::string(QcLoLoLoLo_Str.begin() + 1, QcLoLoLoLo_Str.begin() + 4));
      double lonTenth = NFmiStringTools::Convert<double>(
                            std::string(QcLoLoLoLo_Str.begin() + 4, QcLoLoLoLo_Str.begin() + 5)) /
                        6.;
      lon += lonTenth;
      std::string MMMUlaUlo_Str;  // esim. "25018"
      ssin >> MMMUlaUlo_Str;
      if (doShip == false)
      {                              // luetaan aseman korkeus vain MOBIL sanomille, ei SHIP:eille
        std::string h0h0h0h0im_Str;  // esim. "01152"
        ssin >> h0h0h0h0im_Str;
      }
      station.SetLatitude(lat);
      station.SetLongitude(lon);
    }
    else  // luetaan aseman id-stringi
    {
      std::string IIiii_Str;  // IIiii asematunnuskoodi esim. "02963" II on alue koodi ja 963 on
                              // asema tunnus
      ssin >> IIiii_Str;
      if (IIiii_Str.size() != 5)
        return false;

      // HUOM! tässä pitäisi olla joku asema tietokanta käytössä, mistä saisi aseman sijainnin
      unsigned long stationId = NFmiStringTools::Convert<unsigned long>(IIiii_Str);
      NFmiAviationStation *tempStation =
          theTempStations ? theTempStations->FindStation(stationId) : 0;
      if (tempStation)
      {
        station = NFmiStation(*tempStation);
      }
      else if (theUnknownStationLocation != NFmiPoint::gMissingLatlon)
      {
        station.SetLatitude(theUnknownStationLocation.Y());
        station.SetLongitude(theUnknownStationLocation.X());
        station.SetName(IIiii_Str);
      }
      else
        return false;  // ei löytynyt asemaa asemalistasta, ja unknown-asema oli
                       // "missing"-lokaatiossa, hylätään sanoma kokonaan
    }

    if (fInitializeValues)
      theStation = station;  // A-osiossa vain talletetaan aika
    else
    {  // tarkistetaan että sama asema, kuin A-osiossa ollut
      if (theStation != station)
        return false;  // pitäisi heittää poikkeus!!
    }
    return true;
  }
  return false;
}

bool NFmiTEMPCode::DecodeA(void)
{
  bool fConvertKt2Ms = false;
  std::stringstream ssin(itsOriginalCodeAStr);
  if (::DecodeHeader(ssin,
                     itsStation,
                     itsTime,
                     fTempMobil,
                     'A',
                     true,
                     itsTempStations,
                     itsUnknownStationLocation,
                     fConvertKt2Ms))
  {
    if (ssin.good())
    {
      std::string _99PPP_Str;
      std::string PnPnhnhnhn_Str;
      std::string TTTaDD_Str;
      std::string dddff_Str;
      ssin >> _99PPP_Str;
      if (_99PPP_Str.size() != 5)
        return false;
      ssin >> TTTaDD_Str;
      if (TTTaDD_Str.size() != 5)
        return false;
      ssin >> dddff_Str;
      if (dddff_Str.size() != 5)
        return false;
      if (std::string(_99PPP_Str.begin(), _99PPP_Str.begin() + 2) ==
          "99")  // tarkistetaan että 99-taikasana löytyy
      {
        double surfPressure = NFmiStringTools::Convert<double>(
            std::string(_99PPP_Str.begin() + 2, _99PPP_Str.begin() + 5));
        if (surfPressure < 100.)
          surfPressure += 1000.;
        double T = kFloatMissing;
        double Td = kFloatMissing;
        bool status = ::GetTandTd(TTTaDD_Str, T, Td);
        double WD = kFloatMissing;
        double WS = kFloatMissing;
        status &= ::GetWDandWS(dddff_Str, WD, WS, fConvertKt2Ms);
        TEMPLevelData levelData;
        levelData.Clear();
        levelData.itsTemperature = T;
        levelData.itsDewPoint = Td;
        levelData.itsWD = WD;
        levelData.itsWS = WS;
        levelData.itsPressure = surfPressure;
        levelData.itsHeight = kFloatMissing;  // pitäisikö tähän laittaa asema korkeus?!?!?
        AddData(levelData);                   // laitetaan saatu leveli talteen

        double P = kFloatMissing;
        double h = kFloatMissing;
        // ok 1. level on hoidettu, sitten aletaan käymään dataa läpi loopissa
        ssin >> PnPnhnhnhn_Str;
        std::string PnPn_Str(PnPnhnhnhn_Str.begin(), PnPnhnhnhn_Str.begin() + 2);
        bool stopAfterThis = false;
        while (ssin.good() && stopAfterThis == false)
        {
          P = kFloatMissing;
          h = kFloatMissing;
          T = kFloatMissing;
          Td = kFloatMissing;
          WD = kFloatMissing;
          WS = kFloatMissing;

          if (PnPnhnhnhn_Str.size() != 5)
            return false;
          if (PnPnhnhnhn_Str == "77999")
            break;  // nyt tultiin loppuun, ei ole tietoa max tuuli jutuista
          else if (PnPnhnhnhn_Str == "88999")
          {
            // ei tropopaussia, ei tehdä mitään, jatketaan seuraavan rivin tulkkausta
            ssin >> PnPnhnhnhn_Str;  // tämä pitää lukea lopuksi, koska loopin lopetus riippuu tästä
            PnPn_Str = std::string(PnPnhnhnhn_Str.begin(), PnPnhnhnhn_Str.begin() + 2);
            continue;
          }
          else if (PnPn_Str == "88")  // trropopaussi merkki
          {
            // tropo pause Pressure
            P = NFmiStringTools::Convert<double>(
                std::string(PnPnhnhnhn_Str.begin() + 2, PnPnhnhnhn_Str.begin() + 5));
            h = kFloatMissing;
            ssin >> TTTaDD_Str;
            ssin >> dddff_Str;
          }
          else if (PnPn_Str == "77" ||
                   PnPn_Str == "66")  // max tuuli ilmoitetaan nyt ja siihen osio loppuu
          {
            // tropo pause Pressure
            P = NFmiStringTools::Convert<double>(
                std::string(PnPnhnhnhn_Str.begin() + 2, PnPnhnhnhn_Str.begin() + 5));
            h = kFloatMissing;
            T = kFloatMissing;  // t/Td kenttää ei anneta tässä tapauksessa
            Td = kFloatMissing;
            TTTaDD_Str = "/////";
            ssin >> dddff_Str;
            stopAfterThis = true;
          }
          else
          {
            ssin >> TTTaDD_Str;
            ssin >> dddff_Str;
          }

          if (TTTaDD_Str.size() != 5)
            return false;
          //					ssin >> dddff_Str;
          if (dddff_Str.size() != 5)
            return false;

          // TÄMÄ erikois kerroksien tarkastelu menee ihan pieleen
          status &= ::GetPandH(PnPnhnhnhn_Str, P, h);
          status &= ::GetTandTd(TTTaDD_Str, T, Td);
          status &= ::GetWDandWS(dddff_Str, WD, WS, fConvertKt2Ms);

          levelData.Clear();
          levelData.itsPressure = P;
          levelData.itsHeight = h;
          levelData.itsTemperature = T;
          levelData.itsDewPoint = Td;
          levelData.itsWD = WD;
          levelData.itsWS = WS;
          AddData(levelData);  // laitetaan saatu leveli talteen

          ssin >> PnPnhnhnhn_Str;  // tämä pitää lukea lopuksi, koska loopin lopetus riippuu tästä
          if (ssin.good() == false)
            break;
          // throw std::runtime_error("Data tiedostossa loppui kesken.");
          PnPn_Str = std::string(PnPnhnhnhn_Str.begin(), PnPnhnhnhn_Str.begin() + 2);
        }
        return true;
      }
    }
  }

  return false;
}

void NFmiTEMPCode::AddData(const TEMPLevelData &theLevelData)
{
  if (theLevelData.itsPressure != kFloatMissing)
    itsLevels.insert(
        std::make_pair(theLevelData.itsPressure, theLevelData));  // laitetaan saatu leveli talteen
}

bool NFmiTEMPCode::DecodeB(void)
{
  bool status = true;
  bool fConvertKt2Ms = false;
  std::stringstream ssin(itsOriginalCodeBStr);
  if (::DecodeHeader(ssin,
                     itsStation,
                     itsTime,
                     fTempMobil,
                     'B',
                     false,
                     itsTempStations,
                     itsUnknownStationLocation,
                     fConvertKt2Ms))
  {
    if (ssin.good())
    {
      bool _41414_groupMissing = false;
      TEMPLevelData levelData;
      std::string XXPPP_Str;  // esim. 11PPP juttu
      std::string TTTaDD_Str;
      std::string dddff_Str;
      do  // Käydään ensin osio, missä on on lämpötilojen käännepisteet
      {
        double P = kFloatMissing;
        double T = kFloatMissing;
        double Td = kFloatMissing;

        ssin >> XXPPP_Str;
        if (XXPPP_Str.size() != 5)
          return false;
        if (XXPPP_Str == "21212")
          break;  // nyt tultiin tämän osion loppuun
        if (XXPPP_Str == "31313")
          break;  // nyt tultiin tämän osion loppuun
        if (XXPPP_Str == "41414")
        {
          _41414_groupMissing =
              true;  // en ole varma miten tämä pitää handlata kun 21212-ryhmä puuttuu
          break;     // nyt tultiin tämän osion loppuun
        }

        std::string PPP_partStr(XXPPP_Str.begin() + 2, XXPPP_Str.begin() + 5);
        if (PPP_partStr != "///")  // joskus tämä voi olla puuttuvaa, skipataan vain conversio,
          // paine jää puuttuvaksi ja kaikki tämän levelin arvot jäävät
          // taltioimatta
          P = NFmiStringTools::Convert<double>(PPP_partStr);
        if (P < 50.)
          P += 1000.;
        ssin >> TTTaDD_Str;
        if (TTTaDD_Str.size() != 5)
          return false;
        status &= ::GetTandTd(TTTaDD_Str, T, Td);

        levelData.Clear();
        levelData.itsPressure = P;
        levelData.itsTemperature = T;
        levelData.itsDewPoint = Td;
        AddData(levelData);  // laitetaan saatu leveli talteen

      } while (ssin.good());

      if (XXPPP_Str ==
          "21212")  // etsitään vain merkittäviä tuulia, jotka tulevat 21212-ryhmän jälkeen
      {
        do  // on kaksi tuuli osiota, jotka molemmat käydään läpi
        {
          do  // Käydään sitten osio, missä on tuulien käännepisteet
          {
            double P = kFloatMissing;
            double WD = kFloatMissing;
            double WS = kFloatMissing;

            ssin >> XXPPP_Str;
            if (XXPPP_Str.size() != 5)
              return false;
            if (XXPPP_Str == "41414")
              break;                   // nyt tultiin tämän osion loppuun
            if (XXPPP_Str == "51515")  // joskus 51515 tulee ilman 41414:ää
            {
              _41414_groupMissing = true;
              break;  // nyt tultiin tämän osion loppuun
            }
            if (XXPPP_Str == "31313")
            {
              // en tiedä mikä tämä on, mutta luetaan pois
              std::string tmp1, tmp2;
              ssin >> tmp1;
              ssin >> tmp2;
              continue;  // jatketaan looppia, seuraavaksi pitäisi tulla 41414
            }

            P = NFmiStringTools::Convert<double>(
                std::string(XXPPP_Str.begin() + 2, XXPPP_Str.begin() + 5));
            if (P < 50.)
              P += 1000.;
            ssin >> dddff_Str;
            if (dddff_Str.size() != 5)
              return false;
            status &= ::GetWDandWS(dddff_Str, WD, WS, fConvertKt2Ms);

            levelData.Clear();
            levelData.itsPressure = P;
            levelData.itsWD = WD;
            levelData.itsWS = WS;
            AddData(levelData);  // laitetaan saatu leveli talteen

          } while (ssin.good());
          if (_41414_groupMissing == false)
          {
            std::string NhClCmCh_Str;
            ssin >> NhClCmCh_Str;
            std::string check_Str;
            ssin >> check_Str;
            if (check_Str != "51515")
              break;  // 1. tuuli osion jälkeen kun on tultu "41414" merkkiin, on kaksi juttu jotka
                      // luetaan
            // toinen on synop pilvi osuus ja toinen on 51515 merkki, josta alkaa 2. tuuli osuus
            // jos sitä ei löydy, voidaan olettaa että ollaan menty jo sen ohi ja lopetetaan.
          }
          else
            _41414_groupMissing = false;  // pitää 'nollata' tämä lippu jos se oli päällä
        } while (ssin.good());
      }
      return true;
    }
  }

  return false;
}

bool NFmiTEMPCode::DecodeC(void)
{
  bool fConvertKt2Ms = false;
  std::stringstream ssin(itsOriginalCodeCStr);
  if (::DecodeHeader(ssin,
                     itsStation,
                     itsTime,
                     fTempMobil,
                     'C',
                     false,
                     itsTempStations,
                     itsUnknownStationLocation,
                     fConvertKt2Ms))
  {
    if (ssin.good())
      return true;  // tarkistetaan vain että on C-osio ja sama luotaus kuin aiemmin, ei pureta
                    // vielä enempää

    // tähän loppu C-osion koodin purku
  }
  return false;
}

bool NFmiTEMPCode::DecodeD(void)
{
  bool fConvertKt2Ms = false;
  std::stringstream ssin(itsOriginalCodeDStr);
  if (::DecodeHeader(ssin,
                     itsStation,
                     itsTime,
                     fTempMobil,
                     'D',
                     false,
                     itsTempStations,
                     itsUnknownStationLocation,
                     fConvertKt2Ms))
  {
    if (ssin.good())
      return true;  // tarkistetaan vain että on D-osio ja sama luotaus kuin aiemmin, ei pureta
                    // vielä enempää

    // tähän loppu D-osion koodin purku
  }
  return false;
}

struct LevelSorter
{
  bool operator()(const TEMPLevelData &theLevel1, const TEMPLevelData &theLevel2)
  {
    if (theLevel1.itsPressure != kFloatMissing && theLevel2.itsPressure != kFloatMissing)
      if (theLevel1.itsPressure < theLevel2.itsPressure)
        return true;
    return false;
  }
};

static NFmiTimeList MakeTimeList(std::set<NFmiMetTime> &theTimes)
{
  NFmiTimeList times;
  std::set<NFmiMetTime>::iterator it = theTimes.begin();
  for (; it != theTimes.end(); ++it)
    times.Add(new NFmiMetTime(*it));
  return times;
}

struct TimeCollector
{
  void operator()(const NFmiTEMPCode &theTEMPCode) { itsTimes.insert(theTEMPCode.Time()); }
  std::set<NFmiMetTime> itsTimes;
};

struct LocationCollector
{
  void operator()(const NFmiTEMPCode &theTEMPCode) { itsLocations.insert(theTEMPCode.Station()); }
  std::set<NFmiStation> itsLocations;
};

static NFmiTimeList MakeTimeListForTEMP(std::vector<NFmiTEMPCode> &theSoundings)
{
  TimeCollector collector;
  collector = std::for_each(theSoundings.begin(), theSoundings.end(), TimeCollector());

  return MakeTimeList(collector.itsTimes);
}

static NFmiLocationBag MakeLocationBag(std::set<NFmiStation> &theStations)
{
  NFmiLocationBag locations;
  std::set<NFmiStation>::iterator it = theStations.begin();
  unsigned long ind = 0;
  std::set<NFmiPoint> points;
  std::vector<unsigned long> equalLocationIndexies;
  for (; it != theStations.end(); ++it)
  {
    size_t s1 = points.size();
    points.insert((*it).GetLocation());
    size_t s2 = points.size();
    if (s1 == s2)
    {
      // tämä on pikaviritys. Jos arvottu luotaus-asemat, niillä on samat
      // sijainnit, tässä pitää tehdä sijainteihin pienet erot
      ind++;
      const_cast<NFmiStation &>((*it)).SetLongitude((*it).GetLongitude() + ind);
    }

    locations.AddLocation(*it);
  }

  return locations;
}

static NFmiLocationBag MakeLocationBagForTEMP(std::vector<NFmiTEMPCode> &theSoundings)
{
  LocationCollector collector;
  collector = std::for_each(theSoundings.begin(), theSoundings.end(), LocationCollector());

  return MakeLocationBag(collector.itsLocations);
}

static NFmiParamBag MakeParamBag(const NFmiProducer &theWantedProducer)
{
  NFmiParamBag params;
  params.Add(NFmiDataIdent(NFmiParam(kFmiPressure, "P"), theWantedProducer));
  params.Add(NFmiDataIdent(NFmiParam(kFmiTemperature, "T"), theWantedProducer));
  params.Add(NFmiDataIdent(NFmiParam(kFmiDewPoint, "Td"), theWantedProducer));
  NFmiTotalWind totWind;
  NFmiDataIdent *newDataIdent = totWind.CreateParam(theWantedProducer);
  boost::shared_ptr<NFmiDataIdent> newDataIdentPtr(newDataIdent);
  params.Add(*newDataIdent);
  params.Add(NFmiDataIdent(NFmiParam(kFmiGeomHeight, "h"), theWantedProducer));
  return params;
}

static NFmiLevelBag MakeLevelBag(std::vector<NFmiTEMPCode> &theSoundings)
{
  size_t ssize = theSoundings.size();
  float maxLevelSize = 0;
  for (size_t i = 0; i < ssize; i++)
  {  // etsitää luotaus missä eniten levelejä, ja otetaan se 'pohjaksi' level bagiin
    if (theSoundings[i].LevelData().size() > maxLevelSize)
      maxLevelSize = static_cast<float>(theSoundings[i].LevelData().size());
  }
  NFmiLevelBag levels(kFmiSoundingLevel, 1, maxLevelSize, 1);
  return levels;
}

static NFmiQueryInfo *MakeNewInnerInfoForTEMP(std::vector<NFmiTEMPCode> &theSoundings,
                                              const NFmiProducer &theWantedProducer)
{
  NFmiQueryInfo *info = 0;
  if (theSoundings.size() > 0)
  {
    NFmiTimeList times(MakeTimeListForTEMP(theSoundings));
    NFmiMetTime origTime;
    NFmiTimeDescriptor timeDesc(origTime, times);

    NFmiLocationBag locations(MakeLocationBagForTEMP(theSoundings));
    NFmiHPlaceDescriptor hPlaceDesc(locations);

    NFmiParamBag params(MakeParamBag(theWantedProducer));
    NFmiParamDescriptor paramDesc(params);

    NFmiLevelBag levels(MakeLevelBag(theSoundings));
    NFmiVPlaceDescriptor vPlaceDesc(levels);

    info = new NFmiQueryInfo(paramDesc, timeDesc, hPlaceDesc, vPlaceDesc);
  }
  return info;
}

static std::string MakeTimeString(const NFmiMetTime &atime)
{
    return std::string(atime.ToStr("YYYY.MM.DD HH:mm"));
}

static std::string MakeCheckReport(NFmiQueryData *theData, int theSoundingCount)
{
  if (theData && theSoundingCount)
  {
    NFmiFastQueryInfo info(theData);
    if (info.SizeLevels() && info.SizeLocations() && info.SizeParams() && info.SizeTimes())
    {  // dataan oli saatu varmasti jotain, kun kaikki size:t ovat > 0
      std::string str;
      str += ::GetDictionaryString("TempCodeInsertDlgSoundingStr");
      str += ":";
      str += NFmiStringTools::Convert<int>(theSoundingCount);
      str += " ";
      str += ::GetDictionaryString("TempCodeInsertDlgPiecesStr");
      str += ", ";
      str += ::GetDictionaryString("TempCodeInsertDlgStationStr");
      str += ":";
      str += NFmiStringTools::Convert<int>(info.SizeLocations());
      str += " ";
      str += ::GetDictionaryString("TempCodeInsertDlgPiecesStr");
      str += ", ";
      str += ::GetDictionaryString("TempCodeInsertDlgTimeStr");
      str += ":";
      str += NFmiStringTools::Convert<int>(info.SizeTimes());
      str += " ";
      str += ::GetDictionaryString("TempCodeInsertDlgPiecesStr");
      str += ".\n";
      str += ::GetDictionaryString("Times: ");
      for(info.ResetTime(); info.NextTime(); )
          str += ::MakeTimeString(info.Time()) + " ";
      return str;
    }
  }

  return ::GetDictionaryString("Unable to genereate any TEMP data from give input string.");
}

NFmiQueryData *DecodeTEMP::MakeNewDataFromTEMPStr(const std::string &theTEMPStr,
                                                  std::string &theCheckReportStr,
                                                  NFmiAviationStationInfoSystem &theTempStations,
                                                  const NFmiPoint &theUnknownStationLocation,
                                                  const NFmiProducer &theWantedProducer,
                                                  bool fRoundTimesToNearestSynopticTimes)
{
  theCheckReportStr = ::GetDictionaryString("TempCodeInsertDlgStartCheckingStr");
  std::vector<std::string> codeParcels = NFmiStringTools::Split(theTEMPStr, "=");
  bool noEqualSignInCode = false;
  if (codeParcels.size() <= 1)
  {  // kokeillaan sitten jakaa TT -stringin avulla koodia osiin. =-merkkiä ei ole wyomingista
     // saatavista raaka luotaus koodeissa
    codeParcels = NFmiStringTools::Split(theTEMPStr, "TT");
    noEqualSignInCode = true;
  }

  if (codeParcels.size() < 2)
  {
    theCheckReportStr = ::GetDictionaryString("TempCodeInsertDlgTooFewSectionsErrorStr");
    return 0;
  }

  std::vector<NFmiTEMPCode> tempCodeVec;
  NFmiTEMPCode tempCode(&theTempStations, theUnknownStationLocation);
  for (int i = 0; i < static_cast<int>(codeParcels.size());)
  {
    // aina pitäisi olla neljä koodi osiota annettavissa, että yritetään purkaa luotaus tekstiä
    int decodeCount = 0;
    try
    {
      decodeCount = 0;
      if (codeParcels.size() - i >= 4)
        decodeCount = tempCode.InsertCodeStrings(codeParcels[i],
                                                 codeParcels[i + 1],
                                                 codeParcels[i + 2],
                                                 codeParcels[i + 3],
                                                 noEqualSignInCode);
      else if (codeParcels.size() - i == 3)
        decodeCount = tempCode.InsertCodeStrings(
            codeParcels[i], codeParcels[i + 1], codeParcels[i + 2], "", noEqualSignInCode);
      else if (codeParcels.size() - i == 2)
        decodeCount = tempCode.InsertCodeStrings(
            codeParcels[i], codeParcels[i + 1], "", "", noEqualSignInCode);
      else if (codeParcels.size() - i == 1)
        decodeCount = tempCode.InsertCodeStrings(codeParcels[i], "", "", "", noEqualSignInCode);
    }
    catch (std::exception & /* e */)
    {
    }
    if (decodeCount > 0)  // jos edes yksi osa on onnistuttu purkamaan (4:stä mahdollisesta),
                          // laitetaan luotaus listaan
    {
      if (fRoundTimesToNearestSynopticTimes)
      {
        NFmiMetTime aTime(tempCode.Time());
        aTime.SetTimeStep(180);
        tempCode.Time(aTime);
      }
      tempCodeVec.push_back(tempCode);
    }
    if (decodeCount <= 0)
      decodeCount = 1;  // jos ei onnistunut purkamaan mitään, mennään ainakin yksi blokki eteenpäin
                        // ja kokeillaan
    i += decodeCount;  // edetään purettujen blokkien määrän verran eteenpäin
  }

  NFmiQueryData *newData = DecodeTEMP::CreateNewQData(tempCodeVec, theWantedProducer);
  theCheckReportStr = MakeCheckReport(newData, static_cast<int>(tempCodeVec.size()));
  return newData;
}

NFmiQueryData *DecodeTEMP::CreateNewQData(std::vector<NFmiTEMPCode> &theTempCodeVec,
                                          const NFmiProducer &theWantedProducer)
{
  NFmiQueryData *newData = 0;
  NFmiQueryInfo *innerInfo = MakeNewInnerInfoForTEMP(theTempCodeVec, theWantedProducer);
  boost::shared_ptr<NFmiQueryInfo> innerInfoPtr(innerInfo);
  if (innerInfo)
  {
    newData = NFmiQueryDataUtil::CreateEmptyData(*innerInfo);
    if (newData)
    {  // sitten täytetään uusi data
      NFmiFastQueryInfo infoIter(newData);

      std::vector<NFmiTEMPCode>::iterator it = theTempCodeVec.begin();
      std::vector<NFmiTEMPCode>::iterator endIt = theTempCodeVec.end();
      for (; it != endIt; ++it)
      {
        const NFmiTEMPCode &tempCode = *it;
        if (infoIter.Time(tempCode.Time()))
        {
          if (infoIter.Location(tempCode.Station().GetLocation()))
          {
            const std::map<double, TEMPLevelData> &levelData = tempCode.LevelData();
            std::map<double, TEMPLevelData>::const_iterator levelIt = levelData.begin();
            std::map<double, TEMPLevelData>::const_iterator endLevelIt = levelData.end();
            infoIter.FirstLevel();
            for (; levelIt != endLevelIt; ++levelIt)
            {
              infoIter.Param(kFmiTemperature);
              infoIter.FloatValue(static_cast<float>((*levelIt).second.itsTemperature));
              infoIter.Param(kFmiDewPoint);
              infoIter.FloatValue(static_cast<float>((*levelIt).second.itsDewPoint));
              infoIter.Param(kFmiPressure);
              infoIter.FloatValue(static_cast<float>((*levelIt).second.itsPressure));
              infoIter.Param(kFmiGeomHeight);
              infoIter.FloatValue(static_cast<float>((*levelIt).second.itsHeight));
              infoIter.Param(kFmiWindDirection);
              infoIter.FloatValue(static_cast<float>((*levelIt).second.itsWD));
              infoIter.Param(kFmiWindSpeedMS);
              infoIter.FloatValue(static_cast<float>((*levelIt).second.itsWS));

              infoIter.NextLevel();
            }
          }
        }
      }
    }
  }
  return newData;
}

/*

Esimerkki RAW TEMP ja siitä tehty lista (luulen että sitä on täytetty joissain kohdin?!?!)

 TTAA 60121 02963 99992 01608 24005 00038 ///// ///// 92660 03107 24509 85325
06535 28003 70813 17557 29013 50524 36956 24509 40674 48357 21510 30865 47780
23016 25985 46185 23524 20132 50184 22528 15320 50584 24031 10582 51383 22532
88411 48335 23510 88210 50184 22021 77999 31313 48108 81130
 TTBB 60128 02963 00992 01608 11954 01504 22803 09341 33788 10533 44735 14759
55712 16947 66648 21758 77605 25336 88527 33922 99487 38357 11411 48335 22384
46768 33249 45785 44210 50184 55163 52183 66100 51384 21212 00992 24005 11894
23510 22784 25510 33767 28010 44712 30510 55686 29011 66671 24511 77623 23513
88606 25014 99557 23011 11521 25011 22508 23511 33494 25510 44444 22510 55410
23510 66393 20511 77383 23010 88373 21512 99353 23012 11346 21512 22328 22513
33313 21014 44290 24015 55277 21013 66260 21025 77256 22027 88242 25022 99227
23515 11215 21520 22172 23525 33165 22029 44160 23036 55145 25530 66134 25523
77130 23520 88114 23527 99108 21025 11100 22532 31313 48108 81130 41414 /////
51515 11875 24007 22800 26509 33600 25015
 TTCC 60125 02963 70814 49784 21533 50032 52183 23037 30366 ///// 88999 77999
31313 48108 81130
 TTDD 6012/ 02963 11686 49184 22546 53183 33421 51983 44368 48184 55302 51583
21212 11919 24030 22882 22018 33868 21021 44812 23034 55725 21032 66651 22547
77611 23537 88539 23031 99472 22537 11442 20544 22411 23049 33375 23034 44367
22033 55355 20534 66302 23542 31313 48108 81130


2963 Observations at 12Z 10 Apr 2006

-----------------------------------------------------------------------------
   PRES   HGHT   TEMP   DWPT   RELH   MIXR   DRCT   SKNT   THTA   THTE   THTV
    hPa     m      C      C      %    g/kg    deg   knot     K      K      K
-----------------------------------------------------------------------------
 1000.0     38
  992.0    103    1.6    0.8     94   4.11    240      5  275.4  286.7  276.1
  954.0    415   -1.5   -1.9     97   3.50    243      7  275.3  285.1  275.9
  925.0    660   -3.1   -3.8     95   3.13    245      9  276.1  284.9  276.7
  894.0    928   -4.5   -6.3     87   2.68    235     10  277.4  285.1  277.9
  850.0   1325   -6.5  -10.0     76   2.11    280      3  279.3  285.5  279.7
  803.0   1768   -9.3  -13.4     72   1.70    262      8  280.9  286.0  281.2
  788.0   1913  -10.5  -13.8     77   1.68    257     10  281.1  286.2  281.4
  784.0   1952  -10.8  -14.5     74   1.59    255     10  281.2  286.0  281.5
  767.0   2120  -12.1  -17.6     64   1.25    280     10  281.6  285.4  281.8
  735.0   2445  -14.7  -23.7     46   0.77    294     10  282.2  284.6  282.4
  712.0   2685  -16.9  -21.6     67   0.96    305     10  282.4  285.3  282.5
  700.0   2813  -17.5  -24.5     54   0.76    290     13  283.1  285.5  283.2
  686.0   2963  -18.6  -25.9     53   0.68    290     11  283.5  285.7  283.6
  671.0   3127  -19.8  -27.4     51   0.61    245     11  283.9  285.9  284.0
  648.0   3386  -21.7  -29.7     48   0.51    240     12  284.6  286.3  284.7
  623.0   3674  -23.8  -29.2     61   0.55    235     13  285.5  287.3  285.6
  606.0   3877  -25.2  -28.9     71   0.58    250     14  286.1  288.0  286.2
  605.0   3889  -25.3  -28.9     72   0.58    250     14  286.1  288.0  286.2
  557.0   4479  -30.4  -33.2     77   0.42    230     11  286.9  288.3  286.9
  527.0   4874  -33.9  -36.1     80   0.33    247     11  287.3  288.4  287.4
  521.0   4953  -34.5  -37.6     74   0.29    250     11  287.4  288.4  287.5
  508.0   5129  -36.0  -40.9     61   0.21    235     11  287.8  288.5  287.8
  500.0   5240  -36.9  -42.9     54   0.17    245      9  288.0  288.6  288.0
  494.0   5323  -37.5  -44.0     51   0.16    255     10  288.2  288.8  288.2
  487.0   5421  -38.3  -45.3     48   0.14    251     10  288.4  288.9  288.5
  444.0   6043  -43.8  -48.8     57   0.10    225     10  289.3  289.7  289.3
  411.0   6562  -48.3  -51.8     67   0.08    235     10  289.9  290.2  289.9
  410.0   6578  -48.3  -52.1     64   0.08    235     10  290.1  290.4  290.1
  400.0   6740  -48.3  -55.3     44   0.05    215     10  292.1  292.3  292.1
  393.0   6857  -47.6  -59.4     25   0.03    205     11  294.5  294.6  294.5
  384.0   7011  -46.7  -64.7     11   0.02    227     10  297.7  297.7  297.7
  383.0   7028  -46.7  -64.8     11   0.02    230     10  297.9  297.9  297.9
  373.0   7204  -46.8  -66.2      9   0.01    215     12  300.0  300.1  300.0
  353.0   7570  -47.0  -69.1      6   0.01    230     12  304.5  304.5  304.5
  346.0   7703  -47.1  -70.2      5   0.01    215     12  306.1  306.1  306.1
  328.0   8057  -47.3  -73.0      4   0.01    225     13  310.5  310.5  310.5
  313.0   8368  -47.5  -75.5      3   0.00    210     14  314.4  314.4  314.4
  300.0   8650  -47.7  -77.7      2   0.00    230     16  318.0  318.0  318.0
  290.0   8873  -47.4  -78.3      2   0.00    240     15  321.5  321.6  321.5
  277.0   9175  -47.0  -79.2      1   0.00    210     13  326.4  326.4  326.4
  260.0   9592  -46.4  -80.4      1   0.00    210     25  333.1  333.1  333.1
  256.0   9694  -46.3  -80.7      1   0.00    220     27  334.8  334.8  334.8
  250.0   9850  -46.1  -81.1      1   0.00    235     24  337.4  337.4  337.4
  249.0   9877  -45.7  -80.7      1   0.00    237     24  338.4  338.4  338.4
  242.0  10065  -46.4  -81.3      1   0.00    250     22  340.0  340.1  340.0
  227.0  10487  -48.1  -82.5      1   0.00    235     15  343.8  343.8  343.8
  215.0  10846  -49.5  -83.6      1   0.00    215     20  347.0  347.0  347.0
  210.0  11001  -50.1  -84.1      1   0.00    220     21  348.4  348.4  348.4
  200.0  11320  -50.1  -84.1      1   0.00    225     28  353.3  353.3  353.3
  172.0  12306  -51.6  -84.8      1   0.00    235     25  366.4  366.4  366.4
  165.0  12577  -52.0  -85.0      1   0.00    220     29  370.1  370.1  370.1
  163.0  12657  -52.1  -85.1      1   0.00    224     32  371.2  371.2  371.2
  160.0  12779  -51.7  -85.0      1   0.00    230     36  373.8  373.8  373.8
  150.0  13200  -50.5  -84.5      1   0.00    240     31  382.9  382.9  382.9
  145.0  13419  -50.6  -84.5      1   0.00    255     30  386.5  386.5  386.5
  134.0  13929  -50.7  -84.4      1   0.00    255     23  395.0  395.0  395.0
  130.0  14125  -50.8  -84.4      1   0.00    235     20  398.3  398.3  398.3
  114.0  14973  -51.0  -84.4      1   0.00    235     27  413.1  413.1  413.1
  108.0  15323  -51.1  -84.3      1   0.00    210     25  419.3  419.3  419.3
  100.0  15820  -51.3  -84.3      1   0.00    225     32  428.3  428.4  428.3
   92.0  16362  -50.9  -84.2      1   0.00    240     30  439.4  439.4  439.4
   88.0  16651  -50.7  -84.1      1   0.00    220     18  445.4  445.4  445.4
   87.0  16726  -50.7  -84.1      1   0.00    210     21  447.0  447.0  447.0
   81.0  17191  -50.4  -84.0      1   0.00    230     34  456.8  456.9  456.8
   73.0  17867  -49.9  -83.8      1   0.00    210     32  471.6  471.6  471.6
   70.0  18140  -49.7  -83.7      1   0.01    215     33  477.7  477.7  477.7
   68.6  18272  -49.1  -83.1      1   0.01    218     37  481.8  481.8  481.8
   65.0  18622  -50.0  -83.8      1   0.01    225     47  487.2  487.2  487.2
   61.0  19034  -51.2  -84.6      1   0.01    235     37  493.6  493.6  493.6
   54.6  19753  -53.1  -86.1      1   0.00    230     32  505.0  505.1  505.0
   54.0  19824  -53.0  -86.0      1   0.00    230     31  506.9  507.0  506.9
   50.0  20320  -52.1  -85.1      1   0.01    230     37  520.2  520.3  520.2
   47.0  20721  -52.0  -85.0      1   0.01    225     37  529.7  529.8  529.7
   44.0  21148  -52.0  -85.0      1   0.01    205     44  540.0  540.0  540.0
   42.1  21433  -51.9  -84.9      1   0.01    221     47  547.0  547.0  547.0
   41.0  21606  -51.1  -84.3      1   0.01    230     49  553.0  553.0  553.0
   38.0  22103  -49.0  -82.8      1   0.01    230     34  570.6  570.7  570.6
   37.0  22277  -48.2  -82.2      1   0.01    220     33  576.9  577.0  576.9
   36.8  22312  -48.1  -82.1      1   0.01    217     33  578.1  578.3  578.1
   36.0  22456  -48.5  -82.4      1   0.01    205     34  580.8  580.9  580.8
   30.2  23604  -51.5  -84.5      1   0.01    234     42  602.5  602.6  602.5
   30.0                                       235     42
*/
