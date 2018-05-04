// ==========================================================================
/*!
 * \file
 * \brief Implementation of NFmiAviationStation and NFmiAviationStationInfoSystem -classes
 */
// ==========================================================================

#include "NFmiAviationStationInfoSystem.h"
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiStringTools.h>

#include <sstream>
#include <vector>

NFmiAviationStation *NFmiAviationStationInfoSystem::FindStation(const std::string &theIcaoId)
{
  if (fWmoStationsWanted == false)
  {
    std::map<std::string, NFmiAviationStation>::iterator it = itsIcaoStations.find(theIcaoId);
    if (it != itsIcaoStations.end())
    {
      return &(it->second);
    }
  }
  return 0;
}

NFmiAviationStation *NFmiAviationStationInfoSystem::FindStation(long theWmoId)
{
  if (fWmoStationsWanted)
  {
    std::map<long, NFmiAviationStation>::iterator it = itsWmoStations.find(theWmoId);
    if (it != itsWmoStations.end())
    {
      return &(it->second);
    }
  }
  return 0;
}

// country3,country2,country,region,subregion,city,station_name_current,station_name_special,stn_key,icao_xref,national_id_xref,wmo_xref,wban_xref,iata_xref,status,icao,icao_quality,icao_source,national_id,national_quality,national_id_source,wmo;wmo_quality;wmo_source;maslib;maslib_source;wban;wban_source;special;lat_prp;lon_prp;ll_quality;llsrc;elev_prp;elev_prp_quality;elev_aerodrome_source;elev_baro;elev_baro_quality;elev_baro_source;tz;postal_code;start;start_source;end;end_source;remarks;latgrp;longrp
// FIN,FI,Finland,-,,Helsinki,Vantaa,,FIaaEFHK,EFHK,,2974,,HEL,m,EFHK,A,ICA09,,,,2974,A,WMO11,29740,,,,,60.31666667,24.96666667,G,WMO11
// WMO10,51,C,WMO11 WMO10,56,C,WMO11 WMO10,Europe/Helsinki,FI-01560,,,,,,60,24
static bool GetAviationStationFromCsvString(const std::string &theStationStr,
                                            NFmiAviationStation &theStationOut,
                                            bool fIcaoNeeded,
                                            bool fWmoNeeded)
{
  static long currentWmoIdCounter = 128000;
  if (theStationStr.size() > 2)
  {
    if (theStationStr[0] == '#')
      return false;
    if (theStationStr[0] == '/' && theStationStr[1] == '/')
      return false;

    std::vector<std::string> stationParts = NFmiStringTools::Split(theStationStr, ",");
    if (stationParts.size() >= 30)
    {
      bool latlonOk = false;
      bool icaoOk = false;
      bool wmoOk = false;
      std::string nameStr = stationParts[5];
      std::string icaoStr = stationParts[15];
      const long missingWmoId = 9999999;
      long wmoId = missingWmoId;
      double lat = -9999;
      double lon = -9999;
      if (icaoStr.size() == 4)
        icaoOk = true;
      try
      {
        wmoId = NFmiStringTools::Convert<long>(stationParts[21]);
        wmoOk = true;
      }
      catch (...)
      {
      }

      try
      {
        lat = NFmiStringTools::Convert<double>(stationParts[29]);
        lon = NFmiStringTools::Convert<double>(stationParts[30]);
        latlonOk = true;
      }
      catch (...)
      {
      }

      if (latlonOk && (fIcaoNeeded == false || (fIcaoNeeded && icaoOk)) &&
          (fWmoNeeded == false || (fWmoNeeded && wmoOk)))
      {
        if (wmoId == missingWmoId)
          wmoId = currentWmoIdCounter++;
        theStationOut.SetIdent(wmoId);
        theStationOut.SetLatitude(lat);
        theStationOut.SetLongitude(lon);
        theStationOut.IcaoStr(icaoStr);
        theStationOut.SetName(nameStr);
        return true;
      }
    }
  }
  return false;
}

void NFmiAviationStationInfoSystem::InitFromMasterTableCsv(const std::string &theInitFileName)
{
  itsInitLogMessage = "";
  // tyhjennetään ensin asemalistat
  itsIcaoStations.clear();
  itsWmoStations.clear();

  if (theInitFileName.empty())
    throw std::runtime_error(
        "NFmiAviationStationInfoSystem::InitFromMasterTableCsv - empty settings filename given.");

  std::string fileContent;
  if (NFmiFileSystem::ReadFile2String(theInitFileName, fileContent))
  {
    std::stringstream in(fileContent);

    const int maxBufferSize = 1024 + 1;  // kuinka pitkä yhden rivin maksimissaan oletetaan olevan
    std::string buffer;
    int i = 0;
    int counter = 0;
    do
    {
      buffer.resize(maxBufferSize);
      in.getline(&buffer[0], maxBufferSize);
      size_t realSize = strlen(buffer.c_str());
      buffer.resize(realSize);

      try
      {
        NFmiAviationStation station;
        if (::GetAviationStationFromCsvString(
                buffer, station, fWmoStationsWanted == false, fWmoStationsWanted == true))
        {
          counter++;
          if (fWmoStationsWanted)
            itsWmoStations.insert(std::make_pair(station.GetIdent(), station));
          else
            itsIcaoStations.insert(std::make_pair(station.IcaoStr(), station));
        }
      }
      catch (std::runtime_error & /* e */)
      {
      }

      i++;
    } while (in.good());

    if (fVerboseMode)
    {
      std::string stationTypeStr = fWmoStationsWanted ? "Wmo" : "Icao";
      int totalStationCount =
          static_cast<int>(fWmoStationsWanted ? itsWmoStations.size() : itsIcaoStations.size());
      itsInitLogMessage =
          std::string("Initializing ") + stationTypeStr + " station data went OK, from file: ";
      itsInitLogMessage += theInitFileName;
      if (totalStationCount == 0)
        itsInitLogMessage += std::string("\nWarning: 0 ") + stationTypeStr + " stations found.";
      else
        itsInitLogMessage += std::string("\nInfo: ") +
                             NFmiStringTools::Convert<int>(totalStationCount) + " " +
                             stationTypeStr + " stations found.";
      int diff = counter - totalStationCount;
      if (diff > 0)
      {
        itsInitLogMessage += "\nWarning: there were probably ";
        itsInitLogMessage += NFmiStringTools::Convert<int>(diff);
        itsInitLogMessage +=
            " 'extra' stations in file, meaning that there were more station lines than different "
            "station were found, use unique stations.";
      }
    }
  }
  else
    throw std::runtime_error(
        std::string(
            "NFmiAviationStationInfoSystem::InitFromMasterTableCsv - trouble reading file: ") +
        theInitFileName);
}

// theDegreeStr on puhtaasti arvo asteina (kokonaisluku)
// theMinutesWithOrientationStr on arvo asteiden minuutteina ja jossa on  pohjoisella
// pallonpuoliskolla N ja eteläisellä pallonpuoliskolla S kirjain perässä
static double GetLatOrLon(const std::string &theLatOrLonStr, bool fDoLatitude)
{
  if (theLatOrLonStr.empty())
    throw std::runtime_error(std::string("GetLatitude - invalid lat/lon string"));

  std::vector<std::string> stringParts = NFmiStringTools::Split(theLatOrLonStr, " ");
  if (stringParts.size() != 2)
    throw std::runtime_error(std::string("GetLatitude - invalid lat/lon string"));
  if (stringParts[0].size() < 2 || stringParts[1].size() != 3)
    throw std::runtime_error(std::string("GetLatitude - invalid lat/lon string"));

  double value = NFmiStringTools::Convert<double>(stringParts[0]);
  std::string minutesWithOrientationStr(stringParts[1]);
  std::string minutesStr(minutesWithOrientationStr.begin(), minutesWithOrientationStr.begin() + 2);
  std::string orientationStr(minutesWithOrientationStr.begin() + 2,
                             minutesWithOrientationStr.end());
  value += (NFmiStringTools::Convert<double>(minutesStr) / 60. * 100.) / 100.;
  if (fDoLatitude)
  {
    if (orientationStr == "S")
      value = -value;
  }
  else
  {
    if (orientationStr == "W")
      value = -value;
  }
  return value;
}

// ensimmäinen rivi tiedostossa on kommentti rivi kuten seuraava rivi ilman kommentti merkkejä '//'
// RegionId	RegionName	CountryArea	CountryCode	StationId	IndexNbr
// IndexSubNbr	StationName	Latitude	Longitude	Hp	HpFlag	Hha	HhaFlag
// PressureDefId	SO-1	SO-2	SO-3	SO-4	SO-5	SO-6	SO-7	SO-8	ObsHs
// UA-1	UA-2	UA-3	UA-4	ObsRems
//
// loput rivit tiedostossa on asema data rivejä, joissa erotin on tabulaattori ja sarake järjestys
// on kuten 1. kommentti rivillä on ilmoitettu
// 1	AFRICA / AFRIQUE	ALGERIA / ALGERIE	1030	57	60351	0	JIJEL-
// ACHOUAT
// 36 48N	05 53E	10		8			X	X	X	X	X
// X
// X
// X
// H00-24	.	.	.	.	A;CLIMAT(C);EVAP;M/B;METAR;SOILTEMP;SPECI;SUNDUR
//
// Kiinnostavat kohdat ovat (huom. tässä kuvauksessa sarakkeet alkaa 1.:sta, koodissa indeksit
// alkavat 0:sta):
// 6. sarake (wmo-id)
// 8. sarake (asema nimi)
// 9. sarake (lat asteet ja minuutit)
// 10. sarake (lon asteet ja minuutit)
static bool GetAviationStationFromWmoFlatTableString(const std::string &theStationStr,
                                                     NFmiAviationStation &theStationOut)
{
  static long currentWmoIdCounter = 128000;
  if (theStationStr.size() > 2)
  {
    // HUOM! vaikka data formaatti ei tuekaan kommentteja, annetaan kommenttien tarkistus koodin
    // olla tässä varmuuden vuoksi
    if (theStationStr[0] == '#')
      return false;
    if (theStationStr[0] == '/' && theStationStr[1] == '/')
      return false;

    std::vector<std::string> stationParts = NFmiStringTools::Split(theStationStr, "\t");
    if (stationParts.size() >= 13)
    {
      bool latlonOk = false;
      bool wmoOk = false;
      std::string nameStr = stationParts[7];
      const long missingWmoId = 9999999;
      long wmoId = missingWmoId;
      double lat = -9999;
      double lon = -9999;
      try
      {
        wmoId = NFmiStringTools::Convert<long>(stationParts[5]);
        wmoOk = true;
      }
      catch (...)
      {
      }

      try
      {
        lat = ::GetLatOrLon(stationParts[8], true);
        lon = ::GetLatOrLon(stationParts[9], false);
        latlonOk = true;
      }
      catch (...)
      {
      }

      if (latlonOk && wmoOk)
      {
        if (wmoId == missingWmoId)
          wmoId = currentWmoIdCounter++;
        theStationOut.SetIdent(wmoId);
        theStationOut.SetLatitude(lat);
        theStationOut.SetLongitude(lon);
        theStationOut.SetName(nameStr);
        return true;
      }
    }
  }
  return false;
}

void NFmiAviationStationInfoSystem::InitFromWmoFlatTable(const std::string &theInitFileName)
{
  itsInitLogMessage = "";
  // tyhjennetään ensin asemalistat
  itsIcaoStations.clear();
  itsWmoStations.clear();

  if (theInitFileName.empty())
    throw std::runtime_error(
        "NFmiAviationStationInfoSystem::InitFromWmoFlatTable - empty settings filename given.");
  if (fWmoStationsWanted == false)
    throw std::runtime_error(
        "NFmiAviationStationInfoSystem::InitFromWmoFlatTable - Wmo flat table supports only wmo "
        "stations, and now Icao stations are set to be the target.");

  std::string fileContent;
  if (NFmiFileSystem::ReadFile2String(theInitFileName, fileContent))
  {
    std::stringstream in(fileContent);

    const int maxBufferSize = 1024 + 1;  // kuinka pitkä yhden rivin maksimissaan oletetaan olevan
    std::string buffer;
    int i = 0;
    int counter = 0;
    do
    {
      buffer.resize(maxBufferSize);
      in.getline(&buffer[0], maxBufferSize);
      size_t realSize = strlen(buffer.c_str());
      buffer.resize(realSize);

      try
      {
        NFmiAviationStation station;
        if (::GetAviationStationFromWmoFlatTableString(buffer, station))
        {
          counter++;
          itsWmoStations.insert(std::make_pair(station.GetIdent(), station));
        }
        else
        {
          std::cerr << "Error with line: " << buffer << std::endl;
        }
      }
      catch (std::runtime_error & /* e */)
      {
      }

      i++;
    } while (in.good());

    if (fVerboseMode)
    {
      int totalStationCount = static_cast<int>(itsWmoStations.size());
      itsInitLogMessage = "Initializing wmo station data went OK, from file: ";
      itsInitLogMessage += theInitFileName;
      if (totalStationCount == 0)
        itsInitLogMessage += "\nWarning: 0 wmo stations found.";
      else
        itsInitLogMessage += std::string("\nInfo: ") +
                             NFmiStringTools::Convert<int>(totalStationCount) +
                             " wmo stations found.";
      int diff = counter - totalStationCount;
      if (diff > 0)
      {
        itsInitLogMessage += "\nWarning: there were probably ";
        itsInitLogMessage += NFmiStringTools::Convert<int>(diff);
        itsInitLogMessage +=
            " 'extra' stations in file, meaning that there were more station lines than different "
            "station were found, use unique stations.";
      }
    }
  }
  else
    throw std::runtime_error(
        std::string(
            "NFmiAviationStationInfoSystem::InitFromWmoFlatTable - trouble reading file: ") +
        theInitFileName);
}

// ======================================================================
