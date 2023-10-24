#pragma once
// ======================================================================
/*!
 * \file NFmiIgnoreStationsData.h
 * \brief Class keeps list of observation stations that should be ignored
 * in certain situations like drawing on map (due errourness observations).
 */
// ======================================================================

#include <list>
#include <string>

class NFmiLocation;

class NFmiIgnoreStation
{
 public:
  NFmiIgnoreStation();
  std::string MakeStationString();
  bool IsRange() const;
  bool GetIdValues(const std::string &theStationIdstr);

  unsigned long itsId;   // halutun aseman id (wmo tai muu vastaava)
  unsigned long itsId2;  // jos tässä on arvo, kyse on rangesta joka kattaa kaikki asemat väliltä

  std::string itsName;  // haetaan id:tä vastaava nimi, jos löytyy
  unsigned long
      itsLastLocationIndex;  // tältä kohdalta asema löytyi viimeksi jostain datasta (optimointia)
  bool fEnabled;             // onko tämä nyt käytössä vai ei
};

class NFmiIgnoreStationsData
{
 public:
  NFmiIgnoreStationsData();

  void InitFromSettings(const std::string &theBaseNameSpace);
  void StoreToSettings();
  void Clear();
  void Add(const NFmiIgnoreStation &theStation);
  void Remove(unsigned long theId);
  bool IgnoreStationsDialogOn() const { return fIgnoreStationsDialogOn; }
  void IgnoreStationsDialogOn(bool newValue) { fIgnoreStationsDialogOn = newValue; }
  bool UseListWithContourDraw() const { return fUseListWithContourDraw; }
  void UseListWithContourDraw(bool newValue) { fUseListWithContourDraw = newValue; }
  bool UseListWithSymbolDraw() const { return fUseListWithSymbolDraw; }
  void UseListWithSymbolDraw(bool newValue) { fUseListWithSymbolDraw = newValue; }
  std::list<NFmiIgnoreStation> &StationList() { return itsStationList; }
  bool IsStationBlocked(const NFmiLocation &theLocation, bool theSymbolCase);
  bool IsIdInList(unsigned long theStationId);

 private:
  std::string MakeStationListString();
  void AddStationsFromString(const std::string &theStationListStr);

  bool fIgnoreStationsDialogOn;  // onko dialogi näkyvissä vai ei
  bool fUseListWithContourDraw;
  bool fUseListWithSymbolDraw;
  std::list<NFmiIgnoreStation> itsStationList;

  std::string itsBaseNameSpace;
};
