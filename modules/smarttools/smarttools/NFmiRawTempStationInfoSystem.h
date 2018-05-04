// NFmiRawTempStationInfoSystem.h
//
// Luokka jossa on lista luotausasemia eri puolilta maailmaa.
// Tätä käytetään silloin kun käytetään MetEditorin ominaisuutta,
// jolloin katsotaan raaka TEMP sanomista tulkattuja luotaus sanomia.
// Näissä sanomissa on vain aseman id. Tuon id:n avulla nimi ja
// sijainti saadaan selville, jos se on asema lista tiedostossa.
//
// Asemalista tiedoston formaatti:
// Kommentit sallittuja.
// Aseman tiedot esitetään aina yhdellä rivillä, rivejä voi olla kuinka monta tahansa.
// Asema_is lon lat nimi(rivin loppuun asti)

#pragma once

#include <newbase/NFmiHPlaceDescriptor.h>
#include <newbase/NFmiStation.h>

class NFmiRawTempStationInfoSystem
{
 public:
  NFmiRawTempStationInfoSystem(void) : itsInitLogMessage(), itsLocations() {}
  void Init(const std::string &theInitFileName);
  NFmiHPlaceDescriptor &Locations(void) { return itsLocations; }
  const std::string &InitLogMessage(void) const { return itsInitLogMessage; }
 private:
  std::string
      itsInitLogMessage;  // onnistuneen initialisoinnin viesti, missä voi olla varoituksia lokiin.
  NFmiHPlaceDescriptor itsLocations;
};

class NFmiSilamStationList
{
 public:
  class Station
  {
   public:
    NFmiPoint itsLatlon;
    std::string itsCountry;
    std::string itsType;
    std::string itsInfo;
  };

  NFmiSilamStationList(void) : itsInitLogMessage(), itsLocations() {}
  void Init(const std::string &theInitFileName);
  void Clear(void);
  checkedVector<NFmiSilamStationList::Station> &Locations(void) { return itsLocations; }
  const std::string &InitLogMessage(void) const { return itsInitLogMessage; }
 private:
  std::string
      itsInitLogMessage;  // onnistuneen initialisoinnin viesti, missä voi olla varoituksia lokiin.
  checkedVector<NFmiSilamStationList::Station> itsLocations;
};

// Tähän puretaan NOAA:n taulukost asema rivi, esim:
// 02;974;EFHK;Helsinki-Vantaa;;Finland;6;60-19N;024-58E;60-19N;024-58E;51;56;P
class NFmiWmoStation
{
 public:
  NFmiWmoStation(void)
      : itsWmoId(0)  // jostain syystä "Buckland, Buckland Airport" asemalla on 0 id, joten 0:aa ei
                     // voi pitää puuttuvan aseman arvona
        ,
        itsName()  // tämä on pakollinen kenttä, jos tämä on tyhjä, asemaa ei ole määritetty
        ,
        itsIcaoStr(),
        itsState(),
        itsCountry(),
        itsLatlon(NFmiPoint::gMissingLatlon)
  {
  }

  bool IsValid(void) const { return itsName.empty() == false; }
  long itsWmoId;           // wmo-id esim 2792 (= 02-974)
  std::string itsName;     // Helsinki-Vantaa
  std::string itsIcaoStr;  // icao tunnus EFHK
  std::string itsState;    // ainakin US asemilla on state, muilla ei kai ole
  std::string itsCountry;  // Finland
  NFmiPoint itsLatlon;
};

class NFmiWmoStationLookUpSystem
{
 public:
  NFmiWmoStationLookUpSystem(void);
  void Init(const std::string &theInitFileName, int theStationCountHint = -1);
  const std::string &InitLogMessage(void) const { return itsInitLogMessage; }
  const NFmiWmoStation &GetStation(long theWmoId);

 private:
  std::map<long, NFmiWmoStation>
      itsStations;                // Used to be a vector, but this should be fast enough
  std::string itsInitLogMessage;  // onnistuneen tai epäonnistuneen initialisoinnin viesti, missä
                                  // voi olla varoituksia lokiin.
};

