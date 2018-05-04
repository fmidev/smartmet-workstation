// ======================================================================
/*!
 * \file NFmiIgnoreStationsData.cpp
 * \brief Implementation of NFmiIgnoreStationsData-class.
 */
// ======================================================================

#include "NFmiIgnoreStationsData.h"
#include <newbase/NFmiSettings.h>
#include <newbase/NFmiLocation.h>

NFmiIgnoreStation::NFmiIgnoreStation(void)
    : itsId(0), itsId2(0), itsName(), itsLastLocationIndex(-1), fEnabled(false)
{
}

bool NFmiIgnoreStation::GetIdValues(const std::string &theStationIdStr)
{
  itsId = 0;
  itsId2 = 0;
  if (theStationIdStr.empty() == false)
  {
    try
    {
      std::vector<std::string> rangeParts = NFmiStringTools::Split(theStationIdStr, "-");
      if (rangeParts.size() == 2)
      {
        itsId = NFmiStringTools::Convert<unsigned long>(rangeParts[0]);
        itsId2 = NFmiStringTools::Convert<unsigned long>(rangeParts[1]);
      }
      else
      {
        itsId = NFmiStringTools::Convert<unsigned long>(theStationIdStr);
        itsId2 = 0;
      }
      return true;
    }
    catch (...)
    {
    }
  }
  return false;
}

std::string NFmiIgnoreStation::MakeStationString(void)
{
  std::string usedStationName = itsName;  // aseman nimest‰ pit‰‰ poistaa ';' ja ':' merkit, lis‰ksi
                                          // jos se on tyhj‰, laitetaan nimeksi "?"
  usedStationName = NFmiStringTools::ReplaceChars(usedStationName, ';', ',');
  usedStationName = NFmiStringTools::ReplaceChars(usedStationName, ':', '.');
  if (usedStationName.empty())
    usedStationName = "?";
  if (IsRange())
    usedStationName = "...";
  std::stringstream out;
  out << itsId;
  if (IsRange())
    out << "-" << itsId2;
  out << ":" << usedStationName << ":" << fEnabled;  // ':' on erotin aseman tiedoissa
  return out.str();
}

bool NFmiIgnoreStation::IsRange(void) const
{
  if (itsId < itsId2)
    return true;
  return false;
}

// ****************************************************************

NFmiIgnoreStationsData::NFmiIgnoreStationsData(void)
    : fIgnoreStationsDialogOn(false),
      fUseListWithContourDraw(false),
      fUseListWithSymbolDraw(false),
      itsStationList()
{
}

void NFmiIgnoreStationsData::Clear(void)
{
  itsStationList.clear();
}

void NFmiIgnoreStationsData::Add(const NFmiIgnoreStation &theStation)
{
  itsStationList.push_back(theStation);
}

struct RemoveStation
{
  RemoveStation(unsigned long theId) : itsId(theId) {}
  bool operator()(const NFmiIgnoreStation &theStation)
  {
    if (theStation.itsId == itsId)
      return true;
    else
      return false;
  }

  unsigned long itsId;
};

void NFmiIgnoreStationsData::Remove(unsigned long theId)
{
  itsStationList.remove_if(RemoveStation(theId));
}

void NFmiIgnoreStationsData::InitFromSettings(const std::string &theBaseNameSpace)
{
  itsBaseNameSpace = theBaseNameSpace;

  fUseListWithContourDraw =
      NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseListWithContourDraw"));
  fUseListWithSymbolDraw =
      NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseListWithSymbolDraw"));
  std::string stationListStr =
      NFmiSettings::Require<std::string>(std::string(itsBaseNameSpace + "::IgnoreStationList"));
  AddStationsFromString(stationListStr);
}

void NFmiIgnoreStationsData::StoreToSettings(void)
{
  if (itsBaseNameSpace.empty() == false)
  {
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseListWithContourDraw"),
                      NFmiStringTools::Convert(fUseListWithContourDraw),
                      true);
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseListWithSymbolDraw"),
                      NFmiStringTools::Convert(fUseListWithSymbolDraw),
                      true);
    NFmiSettings::Set(
        std::string(itsBaseNameSpace + "::IgnoreStationList"), MakeStationListString(), true);
  }
  else
    throw std::runtime_error(
        "Error in NFmiIgnoreStationsData::StoreToSettings, unable to store setting.");
}

std::string NFmiIgnoreStationsData::MakeStationListString(void)
{
  std::string stationsStr;
  if (itsStationList.size() == 0)
    stationsStr += "0";  // t‰m‰ on tyhj‰n listan merkki
  else
  {
    int counter = 0;
    for (std::list<NFmiIgnoreStation>::iterator it = itsStationList.begin();
         it != itsStationList.end();
         ++it)
    {
      if (counter != 0)
        stationsStr += ";";  // t‰m‰ on asemien erotin merkki listassa
      stationsStr += (*it).MakeStationString();
      counter++;
    }
  }
  return stationsStr;
}

void NFmiIgnoreStationsData::AddStationsFromString(const std::string &theStationListStr)
{
  if (theStationListStr.empty() || theStationListStr == std::string("0"))
    return;  // ei tehd‰ mit‰‰n tyj‰n listan kanssa
  else
  {
    std::vector<std::string> stationStrList = NFmiStringTools::Split(theStationListStr, ";");
    for (size_t i = 0; i < stationStrList.size(); i++)
    {
      try
      {
        std::vector<std::string> stationParts = NFmiStringTools::Split(stationStrList[i], ":");
        if (stationParts.size() == 3)
        {
          NFmiIgnoreStation tmp;
          if (tmp.GetIdValues(stationParts[0]) == false)
            throw std::runtime_error("Unable to get station id value from string.");
          tmp.itsName = stationParts[1];
          tmp.fEnabled = NFmiStringTools::Convert<bool>(stationParts[2]);
          Add(tmp);
        }
      }
      catch (...)
      {
      }
    }
  }
}

bool NFmiIgnoreStationsData::IsStationBlocked(const NFmiLocation &theLocation, bool theSymbolCase)
{
  if (theSymbolCase)
  {  // tarkistetaan symboli tapaus
    if (fUseListWithSymbolDraw)
    {
      return IsIdInList(theLocation.GetIdent());
    }
  }
  else
  {  // tarkistetaan contour tapaus
    if (fUseListWithContourDraw)
    {
      return IsIdInList(theLocation.GetIdent());
    }
  }
  return false;
}

bool NFmiIgnoreStationsData::IsIdInList(unsigned long theStationId)
{
  for (std::list<NFmiIgnoreStation>::iterator it = itsStationList.begin();
       it != itsStationList.end();
       ++it)
  {
    if ((*it).IsRange())
    {
      if (theStationId >= (*it).itsId && theStationId <= (*it).itsId2)
        return (*it).fEnabled;
    }
    else if ((*it).itsId == theStationId)
      return (*it).fEnabled;
  }
  return false;
}
