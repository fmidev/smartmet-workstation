
#pragma once

#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiStation.h>

#include <vector>
#include <map>

class NFmiQueryData;
class NFmiAviationStationInfoSystem;
class NFmiProducer;
class NFmiTEMPCode;
class NFmiVPlaceDescriptor;

struct TEMPLevelData
{
  void Clear(void)
  {
    itsPressure = kFloatMissing;
    itsHeight = kFloatMissing;
    itsTemperature = kFloatMissing;
    itsDewPoint = kFloatMissing;
    itsWS = kFloatMissing;
    itsWD = kFloatMissing;
  }

  double itsPressure;
  double itsHeight;
  double itsTemperature;
  double itsDewPoint;
  double itsWS;
  double itsWD;
};

namespace DecodeTEMP
{
// Tälle funktiolle annetaan haluttu läjä TEMP koodia
// (stringissä voi olla vaikka kuinka monta eri TEMP sanomaa peräkkäin).
// Funktio purkaa ne ja laittaa datan new:lla luotuun QueryData-olioon.
NFmiQueryData *MakeNewDataFromTEMPStr(const std::string &theTEMPStr,
                                      std::string &theCheckReportStr,
                                      NFmiAviationStationInfoSystem &theTempStations,
                                      const NFmiPoint &theUnknownStationLocation,
                                      const NFmiProducer &theWantedProducer,
                                      bool fRoundTimesToNearestSynopticTimes);

// Tämä funktio tekee annetusta Tempcode-vectorista querydatan.
// TempCode-vektorissa on siis luotauksia tempcode-olioihin talletettuna.
NFmiQueryData *CreateNewQData(std::vector<NFmiTEMPCode> &theTempCodeVec,
                              const NFmiProducer &theWantedProducer);
}

// Luokka joka saa 4 stringiä, missä on koko temp-koodi.
// Purkaa koodin ja laskee leveleiksi ja muuksi dataksi.
class NFmiTEMPCode
{
 public:
  NFmiTEMPCode(void);
  NFmiTEMPCode(NFmiAviationStationInfoSystem *theTempStations,
               const NFmiPoint &theUnknownStationLocation);
  ~NFmiTEMPCode(void);

  // Saa A,B,C ja D koodit. Tarkistaa ne ja purkaa. Palauttaa luvun, joka kertoo kuinka monta
  // osaa kelpasi (eli oli samalta asemalta, samaan aikaan jne. )
  int InsertCodeStrings(const std::string &theCodeAStr,
                        const std::string &theCodeBStr,
                        const std::string &theCodeCStr,
                        const std::string &theCodeDStr,
                        bool fNoEqualSignInCode);
  void AddData(const TEMPLevelData &theLevelData);
  void Clear(void);
  void ForceWantedPressureLevels(NFmiVPlaceDescriptor &theLevels);

  void Time(const NFmiMetTime &theTime) { itsTime = theTime; }
  const NFmiMetTime &Time(void) const { return itsTime; }
  const NFmiStation &Station(void) const { return itsStation; }
  void Station(const NFmiStation &theStation) { itsStation = theStation; }
  const std::map<double, TEMPLevelData> &LevelData(void) const { return itsLevels; }
 private:
  int Decode(void);
  bool DecodeA(void);
  bool DecodeB(void);
  bool DecodeC(void);
  bool DecodeD(void);

  std::string itsOriginalCodeAStr;  // tähän talletetaan koko koodi osa A
  std::string itsOriginalCodeBStr;  // tähän talletetaan koko koodi osa B
  std::string itsOriginalCodeCStr;  // tähän talletetaan koko koodi osa C
  std::string itsOriginalCodeDStr;  // tähän talletetaan koko koodi osa D

  NFmiMetTime itsTime;
  NFmiStation itsStation;
  bool fTempMobil;  // onko kyseessä temp mobil koodia, joka pitää purkaa hieman erilailla
  NFmiAviationStationInfoSystem *itsTempStations;  // Ei omista.  Jos tässä on luotausinfo, sitä
                                                   // voidaan pitää luotaus asemien "tietokantana"
  NFmiPoint itsUnknownStationLocation;

  std::map<double, TEMPLevelData> itsLevels;
};

