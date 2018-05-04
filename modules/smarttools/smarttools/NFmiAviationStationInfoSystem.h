// ==========================================================================
/*!
 * \file
 * \brief Interface of NFmiAviationStation and NFmiAviationStationInfoSystem -classes
 */
// ==========================================================================

#pragma once

#include <newbase/NFmiStation.h>

#include <string>
#include <map>

class NFmiAviationStation : public NFmiStation
{
 public:
  NFmiAviationStation(void) : NFmiStation(), itsIcaoStr() {}
  NFmiAviationStation(long theIdent,
                      const NFmiString &theName,
                      double theLongitude,
                      double theLatitude,
                      const std::string &theIcaoStr)
      : NFmiStation(theIdent, theName, theLongitude, theLatitude, kFloatMissing, kWMO),
        itsIcaoStr(theIcaoStr)
  {
  }
  ~NFmiAviationStation(void) {}
  const std::string &IcaoStr(void) const { return itsIcaoStr; }
  void IcaoStr(const std::string &newValue) { itsIcaoStr = newValue; }
  NFmiLocation *Clone(void) const { return new NFmiAviationStation(*this); }
 private:
  std::string itsIcaoStr;  // icao tunnus (esim. EFHK)
};

class NFmiAviationStationInfoSystem
{
 public:
  NFmiAviationStationInfoSystem(bool theWmoStationsWanted, bool theVerboseMode)
      : itsInitLogMessage(),
        itsIcaoStations(),
        itsWmoStations(),
        fWmoStationsWanted(theWmoStationsWanted),
        fVerboseMode(theVerboseMode)
  {
  }

  const std::string &InitLogMessage(void) const { return itsInitLogMessage; }
  void InitFromMasterTableCsv(const std::string &theInitFileName);
  void InitFromWmoFlatTable(const std::string &theInitFileName);
  NFmiAviationStation *FindStation(const std::string &theIcaoId);
  NFmiAviationStation *FindStation(long theWmoId);
  bool WmoStationsWanted(void) const { return fWmoStationsWanted; }
 private:
  std::string
      itsInitLogMessage;  // onnistuneen initialisoinnin viesti, missä voi olla varoituksia lokiin.
  std::map<std::string, NFmiAviationStation> itsIcaoStations;
  std::map<long, NFmiAviationStation> itsWmoStations;
  bool fWmoStationsWanted;  // tämä päättää, käytetäänkö luokassa WMO vai ICAO asemia
  bool fVerboseMode;
};


// ======================================================================
