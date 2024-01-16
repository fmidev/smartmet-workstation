#include "NFmiGriddingProperties.h"
#include <boost/algorithm/string.hpp>
#include <newbase/NFmiArea.h>

NFmiGriddingProperties::NFmiGriddingProperties(bool toolMasterAvailable)
    : toolMasterAvailable_(toolMasterAvailable)
{
}

// Muodostetaan yhtenäinen string esitys kaikista asetuksista (paitsi toolMasterAvailable_
// asetuksesta), jossa arvot on eroteltu pilkuilla. Arvot ovat luokan esittelyjärjestyksessä.
// Esimerkiksi oletus arvoilla saatu tulos: 1,0,1,0.5,0,1.25,0.15
std::string NFmiGriddingProperties::toString() const
{
  std::ostringstream out;
  out << static_cast<int>(function_) << "," << rangeLimitInKm_ << "," << localFitMethod_ << ","
      << localFitDelta_ << "," << smoothLevel_ << "," << localFitFilterRadius_ << ","
      << localFitFilterFactor_;
  return out.str();
}

// Lukee arvot luokan annetusta stringistä (paitsi toolMasterAvailable_ asetusta).
// Palauttaa true, jos luku onnistui ja asetukset otetaan käyttöön.
// Jos luvussa tulee mitään ongelmia, säilytetään olion originaali arvot ja palautetaan false.
bool NFmiGriddingProperties::fromString(const std::string &str)
{
  NFmiGriddingProperties origValues = *this;
  try
  {
    std::vector<std::string> strs;
    boost::split(strs, str, boost::is_any_of(","));
    if (strs.size() >= 7)
    {
      auto strsIndex = 0;
      function_ = static_cast<FmiGriddingFunction>(std::stoi(strs[strsIndex++]));
      rangeLimitInKm_ = std::stod(strs[strsIndex++]);
      localFitMethod_ = std::stoi(strs[strsIndex++]);
      localFitDelta_ = std::stod(strs[strsIndex++]);
      smoothLevel_ = std::stoi(strs[strsIndex++]);
      localFitFilterRadius_ = std::stod(strs[strsIndex++]);
      localFitFilterFactor_ = std::stod(strs[strsIndex++]);
      return true;
    }
  }
  catch (...)
  {
  }
  *this = origValues;
  return false;
}

FmiGriddingFunction NFmiGriddingProperties::function() const
{
  if (toolMasterAvailable_)
    return function_;
  return kFmiMarkoGriddingFunction;
}

// Palauta annettu pituus [km] annetun arean suhteellisessa koordinaatistossa. Suhteellinen pituus
// lasketaan x ja y suhteen. Jos area on 0-pointer, palautetaan puuttuvaa.
double NFmiGriddingProperties::ConvertLengthInKmToRelative(double lengthInKm, const NFmiArea *area)
{
  if (lengthInKm > 0)
  {
    if (area)
    {
      const float kmInMeters = 1000.f;
      auto x = kmInMeters * lengthInKm / area->WorldXYWidth();
      auto y = kmInMeters * lengthInKm / area->WorldXYHeight();
      return std::sqrt(x * x + y * y);
    }
  }

  return kFloatMissing;
}
