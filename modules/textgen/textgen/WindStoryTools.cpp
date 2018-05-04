// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::WindStoryTools
 */
// ======================================================================
/*!
 * \namespace TextGen::WindStoryTools
 *
 * \brief Utility enumerations and functions for WindStory methods
 *
 */
// ======================================================================

#include "WindStoryTools.h"
#include "Integer.h"
#include "IntegerRange.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "UnitFactory.h"
#include <calculator/GridForecaster.h>
#include <calculator/RangeAcceptor.h>

#include <newbase/NFmiGlobals.h>

#include <boost/lexical_cast.hpp>
#include <cmath>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
namespace WindStoryTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Calculate the wind direction accuracy class
 *
 * \param theError The error estimate for the wind direction
 * \param theVariable The control variable
 * \return The accuracy class
 */
// ----------------------------------------------------------------------

WindDirectionAccuracy direction_accuracy(double theError, const string& theVariable)
{
  using Settings::optional_double;

  double accurate_limit = optional_double(theVariable + "::wind_direction::accurate_limit", 22.5);
  double variable_limit = optional_double(theVariable + "::wind_direction::variable_limit", 45);

  if (theError <= accurate_limit) return good_accuracy;
  if (theError <= variable_limit) return moderate_accuracy;
  return bad_accuracy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate the generic wind 8th direction from angle
 *
 * \param theDirection The direction value
 * \return The direction in units of 1-8 (north ... north-west)
 */
// ----------------------------------------------------------------------

int direction8th(double theDirection) { return 1 + (int(theDirection / 45.0 + 0.5) % 8); }
// ----------------------------------------------------------------------
/*!
 * \brief Calculate the generic wind 16th direction from angle
 *
 * \param theDirection The direction value
 * \return The direction in units of 1-16
 */
// ----------------------------------------------------------------------

int direction16th(double theDirection) { return 1 + (int(theDirection / 22.5 + 0.5) % 16); }
// ----------------------------------------------------------------------
/*!
 * \brief Return a sentence on wind direction
 *
 * \param theDirection The direction
 * \param theVariable The control variable
 * \return The sentence
 */
// ----------------------------------------------------------------------

Sentence direction_sentence(const WeatherResult& theDirection, const string& theVariable)
{
  Sentence sentence;

  int n = direction8th(theDirection.value());
  switch (direction_accuracy(theDirection.error(), theVariable))
  {
    case good_accuracy:
      sentence << lexical_cast<string>(n) + "-tuulta";
      break;
    case moderate_accuracy:
      sentence << lexical_cast<string>(n) + "-puoleista tuulta";
      break;
    case bad_accuracy:
      sentence << "suunnaltaan vaihtelevaa"
               << "tuulta";
      break;
  }
  return sentence;
}

string direction_string(const WeatherResult& theDirection, const string& theVariable)
{
  string retval;

  int n = direction8th(theDirection.value());
  switch (direction_accuracy(theDirection.error(), theVariable))
  {
    case good_accuracy:
    {
      switch (n)
      {
        case 1:
          retval = "pohjoistuulta";
          break;
        case 2:
          retval = "koillistuulta";
          break;
        case 3:
          retval = "itatuulta";
          break;
        case 4:
          retval = "kaakkoistuulta";
          break;
        case 5:
          retval = "etelatuulta";
          break;
        case 6:
          retval = "lounaistuulta";
          break;
        case 7:
          retval = "lansituulta";
          break;
        case 8:
          retval = "luoteistuulta";
          break;
        default:
          break;
      }
    }
    break;
    case moderate_accuracy:
    {
      switch (n)
      {
        case 1:
          retval = "pohjoisen puoleista tuulta";
          break;
        case 2:
          retval = "koillisen puoleista tuulta";
          break;
        case 3:
          retval = "idan puoleista tuulta";
          break;
        case 4:
          retval = "kaakon puoleista tuulta";
          break;
        case 5:
          retval = "etelan puoleista tuulta";
          break;
        case 6:
          retval = "lounaan puoleista tuulta";
          break;
        case 7:
          retval = "lannen puoleista tuulta";
          break;
        case 8:
          retval = "luoteen puoleista tuulta";
          break;
        default:
          break;
      }
    }
    break;
    case bad_accuracy:
      retval = "suunnaltaan vaihtelevaa tuulta";
      break;
  }
  return retval;
}

string direction16_string(const WeatherResult& theDirection, const string& theVariable)
{
  string retval;

  int n = direction16th(theDirection.value());

  switch (direction_accuracy(theDirection.error(), theVariable))
  {
    case good_accuracy:
    {
      switch (n)
      {
        case 1:
          retval = "pohjoistuulta";
          break;
        case 2:
          retval = "pohjoisen ja koillisen valista tuulta";
          break;
        case 3:
          retval = "koillistuulta";
          break;
        case 4:
          retval = "idan ja koillisen valista tuulta";
          break;
        case 5:
          retval = "itatuulta";
          break;
        case 6:
          retval = "idan ja kaakon valista tuulta";
          break;
        case 7:
          retval = "kaakkoistuulta";
          break;
        case 8:
          retval = "etelan ja kaakon valista tuulta";
          break;
        case 9:
          retval = "etelatuulta";
          break;
        case 10:
          retval = "etelan ja lounaan valista tuulta";
          break;
        case 11:
          retval = "lounaistuulta";
          break;
        case 12:
          retval = "lannen ja lounaan valista tuulta";
          break;
        case 13:
          retval = "lansituulta";
          break;
        case 14:
          retval = "lannen ja luoteen valista tuulta";
          break;
        case 15:
          retval = "luoteistuulta";
          break;
        case 16:
          retval = "pohjoisen ja luoteen valista tuulta";
          break;
        default:
          break;
      }
    }
    break;
    case moderate_accuracy:
    {
      switch (n)
      {
        case 1:
          retval = "pohjoisen puoleista";
          break;
        case 2:
        {
          if (theDirection.value() > 337.5 || theDirection.value() < 22.5)
            retval = "pohjoisen puoleista";
          else
            retval = "koillisen puoleista";
        }
        break;
        case 3:
          retval = "koillisen puoleista";
          break;
        case 4:
        {
          if (theDirection.value() < 67.5)
            retval = "koillisen puoleista";
          else
            retval = "idan puoleista";
        }
        break;
        case 5:
          retval = "idan puoleista";
          break;
        case 6:
        {
          if (theDirection.value() < 112.5)
            retval = "idan puoleista";
          else
            retval = "kaakon puoleista";
        }
        break;
        case 7:
          retval = "kaakon puoleista";
          break;
        case 8:
        {
          if (theDirection.value() < 157.5)
            retval = "kaakon puoleista";
          else
            retval = "etelan puoleista";
        }
        break;
        case 9:
          retval = "etelan puoleista";
          break;
        case 10:
        {
          if (theDirection.value() < 202.5)
            retval = "etelan puoleista";
          else
            retval = "lounaan puoleista";
        }
        break;
        case 11:
          retval = "lounaan puoleista";
          break;
        case 12:
        {
          if (theDirection.value() < 247.5)
            retval = "lounaan puoleista";
          else
            retval = "lannen puoleista";
        }
        break;
        case 13:
          retval = "lannen puoleista";
          break;
        case 14:
        {
          if (theDirection.value() < 292.5)
            retval = "lannen puoleista";
          else
            retval = "luoteen puoleista";
        }
        break;
        case 15:
          retval = "luoteen puoleista";
          break;
        case 16:
        {
          if (theDirection.value() < 337.5)
            retval = "luoteen puoleista";
          else
            retval = "pohjoisen puoleista";
        }
        break;
        default:
          break;
      }
    }
    break;
    case bad_accuracy:
      retval = "suunnaltaan vaihtelevaa tuulta";
      break;
  }
  return retval;
}
// ----------------------------------------------------------------------
/*!
 * \brief Return a sentence on wind speed range
 *
 * \param theMinSpeed The minimum speed
 * \param theMaxSpeed The maximum speed
 * \param theMeanSpeed The mean speed
 * \param theVariable The control variable
 * \return The sentence
 */
// ----------------------------------------------------------------------

Sentence speed_range_sentence(const WeatherResult& theMinSpeed,
                              const WeatherResult& theMaxSpeed,
                              const WeatherResult& theMeanSpeed,
                              const string& theVariable)
{
  using Settings::optional_int;

  Sentence sentence;

  const int minvalue = static_cast<int>(round(theMinSpeed.value()));
  const int maxvalue = static_cast<int>(round(theMaxSpeed.value()));
  const int meanvalue = static_cast<int>(round(theMeanSpeed.value()));

  string var = "textgen::units::meterspersecond::format";
  string opt = Settings::optional_string(var, "SI");

  if (opt == "textphrase")
  {
    sentence << *UnitFactory::create(MetersPerSecond, meanvalue) << "tuulta";
  }
  else
  {
    const int mininterval = optional_int(theVariable + "::mininterval", 0);
    const string rangeseparator = Settings::optional_string(theVariable + "::rangeseparator", "-");

    if (maxvalue - minvalue < mininterval)
    {
      sentence << "noin" << Integer(static_cast<int>(round(theMeanSpeed.value())));
    }
    else
    {
      sentence << IntegerRange(minvalue, maxvalue, rangeseparator);
    }
    sentence << *UnitFactory::create(MetersPerSecond);
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return a sentence on wind speed and direction
 *
 * The sentence is of form "[suuntatuulta] X...Y m/s"
 *
 * \param theMinSpeed The minimum speed
 * \param theMaxSpeed The maximum speed
 * \param theMeanSpeed The mean speed
 * \param theDirection The direction
 * \param theVariable The control variable
 * \return The sentence
 */
// ----------------------------------------------------------------------

Sentence directed_speed_sentence(const WeatherResult& theMinSpeed,
                                 const WeatherResult& theMaxSpeed,
                                 const WeatherResult& theMeanSpeed,
                                 const WeatherResult& theDirection,
                                 const string& theVariable)
{
  Sentence sentence;

  const string var = "textgen::units::meterspersecond::format";
  const string opt = Settings::optional_string(var, "SI");

  if (opt == "textphrase")
  {
    int int_value(static_cast<int>(round(theMeanSpeed.value())));
    if (int_value >= 1)
    {
      sentence << *UnitFactory::create(MetersPerSecond, int_value);
      if (int_value <= 20) sentence << direction_sentence(theDirection, theVariable);
    }
  }
  else
  {
    sentence << direction_sentence(theDirection, theVariable)
             << speed_range_sentence(theMinSpeed, theMaxSpeed, theMeanSpeed, theVariable);
  }

  return sentence;
}

std::string speed_string(const WeatherResult& theMeanSpeed)
{
  std::string retval;

  int int_value(static_cast<int>(round(theMeanSpeed.value())));

  if (int_value >= 1)
  {
    if (int_value >= 1 && int_value <= 3)
    {
      retval = "heikkoa";
    }
    else if (int_value >= 4 && int_value <= 7)
    {
      retval = "kohtalaista";
    }
    else if (int_value >= 8 && int_value <= 13)
    {
      retval = "navakkaa";
    }
    else if (int_value >= 14 && int_value <= 20)
    {
      retval = "kovaa";
    }
    else if (int_value >= 21 && int_value <= 32)
    {
      retval = "myrskya";
    }
    else
    {
      retval = "hirmumyrskya";
    }
  }
  return retval;
}

string directed_speed_string(const WeatherResult& theMeanSpeed,
                             const WeatherResult& theDirection,
                             const string& theVariable)
{
  string retval;

  int int_value(static_cast<int>(round(theMeanSpeed.value())));

  if (int_value >= 1)
  {
    if (int_value >= 1 && int_value <= 3)
    {
      retval = "heikkoa";
    }
    else if (int_value >= 4 && int_value <= 7)
    {
      retval = "kohtalaista";
    }
    else if (int_value >= 8 && int_value <= 13)
    {
      retval = "navakkaa";
    }
    else if (int_value >= 14 && int_value <= 20)
    {
      retval = "kovaa";
    }
    else if (int_value >= 21 && int_value <= 32)
    {
      retval = "myrskya";
    }
    else if (int_value > 32)
    {
      retval = "hirmumyrskya";
    }

    if (int_value <= 20)
    {
      retval += " ";
      retval += direction_string(theDirection, theVariable);
    }
  }
  return retval;
}

string directed16_speed_string(const WeatherResult& theMeanSpeed,
                               const WeatherResult& theDirection,
                               const string& theVariable)
{
  string retval;

  int int_value(static_cast<int>(round(theMeanSpeed.value())));

  if (int_value >= 1)
  {
    if (int_value >= 1 && int_value <= 3)
    {
      retval = "heikkoa";
    }
    else if (int_value >= 4 && int_value <= 7)
    {
      retval = "kohtalaista";
    }
    else if (int_value >= 8 && int_value <= 13)
    {
      retval = "navakkaa";
    }
    else if (int_value >= 14 && int_value <= 20)
    {
      retval = "kovaa";
    }
    else if (int_value >= 21 && int_value <= 32)
    {
      retval = "myrskya";
    }
    else if (int_value > 32)
    {
      retval = "hirmumyrskya";
    }

    retval += " ";
    retval += direction16_string(theDirection, theVariable);
  }

  return retval;
}

WindSpeedId wind_speed_id(const WeatherResult& theWindSpeed)
{
  return wind_speed_id(theWindSpeed.value());
}

WindSpeedId wind_speed_id(float theWindSpeed)
{
  if (theWindSpeed < HEIKKO_LOWER_LIMIT)
    return TYYNI;
  else if (theWindSpeed >= HEIKKO_LOWER_LIMIT && theWindSpeed < HEIKKO_UPPER_LIMIT)
    return HEIKKO;
  else if (theWindSpeed >= KOHTALAINEN_LOWER_LIMIT && theWindSpeed < KOHTALAINEN_UPPER_LIMIT)
    return KOHTALAINEN;
  else if (theWindSpeed >= NAVAKKA_LOWER_LIMIT && theWindSpeed < NAVAKKA_UPPER_LIMIT)
    return NAVAKKA;
  else if (theWindSpeed >= KOVA_LOWER_LIMIT && theWindSpeed < KOVA_UPPER_LIMIT)
    return KOVA;
  else if (theWindSpeed >= MYRSKY_LOWER_LIMIT && theWindSpeed < MYRSKY_UPPER_LIMIT)
    return MYRSKY;
  else
    return HIRMUMYRSKY;
}

std::string wind_speed_string(const WindSpeedId& theWindSpeedId)
{
  std::string retval;

  switch (theWindSpeedId)
  {
    case TYYNI:
      retval = TYYNI_WORD;
      break;
    case HEIKKO:
      retval = HEIKKO_WORD;
      break;
    case KOHTALAINEN:
      retval = KOHTALAINEN_WORD;
      break;
    case NAVAKKA:
      retval = NAVAKKA_WORD;
      break;
    case KOVA:
      retval = KOVA_WORD;
      break;
    case MYRSKY:
      retval = MYRSKY_WORD;
      break;
    case HIRMUMYRSKY:
      retval = HIRMUMYRSKY_WORD;
      break;
  }

  return retval;
}

pair<int, int> wind_speed_interval(const wind_speed_vector& theWindSpeedVector)
{
  pair<int, int> retval;

  float min_value(kFloatMissing);
  float max_value(kFloatMissing);

  for (unsigned int i = 0; i < theWindSpeedVector.size(); i++)
  {
    if (i == 0)
    {
      min_value = theWindSpeedVector[i].first;
      max_value = theWindSpeedVector[i].second;
    }
    else
    {
      if (theWindSpeedVector[i].first < min_value) min_value = theWindSpeedVector[i].first;
      if (theWindSpeedVector[i].second < max_value) max_value = theWindSpeedVector[i].second;
    }
  }
  retval.first = static_cast<int>(round(min_value));
  retval.second = static_cast<int>(round(max_value));

  return retval;
}

WindDirectionId direction_between_id(float theWindDirection)
{
  WindDirectionId retval(MISSING_WIND_DIRECTION_ID);

  if (theWindDirection > 0.0 && theWindDirection <= 45)
    retval = POHJOINEN_KOILLINEN;
  else if (theWindDirection > 45 && theWindDirection <= 90.0)
    retval = ITA_KOILLINEN;
  else if (theWindDirection > 90.0 && theWindDirection <= 135.0)
    retval = ITA_KAAKKO;
  else if (theWindDirection > 135.0 && theWindDirection <= 180.0)
    retval = ETELA_KAAKKO;
  else if (theWindDirection > 180.0 && theWindDirection <= 225.0)
    retval = ETELA_LOUNAS;
  else if (theWindDirection > 225.0 && theWindDirection <= 270.0)
    retval = LANSI_LOUNAS;
  else if (theWindDirection > 270.0 && theWindDirection <= 315.0)
    retval = LANSI_LUODE;
  else if (theWindDirection > 315.0 && theWindDirection <= 360.0)
    retval = LANSI_LUODE;

  return retval;
}

WindDirectionId puoleinen_direction_id(float theWindDirection,
                                       const WindDirectionId& theWindDirectionId)
{
  WindDirectionId windDirectionId(MISSING_WIND_DIRECTION_ID);

  switch (theWindDirectionId)
  {
    case 1:
    {
      windDirectionId = POHJOISEN_PUOLEINEN;
    }
    break;
    case 2:
    {
      if (theWindDirection > 337.5 || theWindDirection < 22.5)
        windDirectionId = POHJOISEN_PUOLEINEN;
      else
        windDirectionId = KOILLISEN_PUOLEINEN;
    }
    break;
    case 3:
    {
      windDirectionId = KOILLISEN_PUOLEINEN;
    }
    break;
    case 4:
    {
      if (theWindDirection < 67.5)
        windDirectionId = KOILLISEN_PUOLEINEN;
      else
        windDirectionId = IDAN_PUOLEINEN;
    }
    break;
    case 5:
    {
      windDirectionId = IDAN_PUOLEINEN;
    }
    break;
    case 6:
    {
      if (theWindDirection < 112.5)
        windDirectionId = IDAN_PUOLEINEN;
      else
        windDirectionId = KAAKON_PUOLEINEN;
    }
    break;
    case 7:
    {
      windDirectionId = KAAKON_PUOLEINEN;
    }
    break;
    case 8:
    {
      if (theWindDirection < 157.5)
        windDirectionId = KAAKON_PUOLEINEN;
      else
        windDirectionId = ETELAN_PUOLEINEN;
    }
    break;
    case 9:
    {
      windDirectionId = ETELAN_PUOLEINEN;
    }
    break;
    case 10:
    {
      if (theWindDirection < 202.5)
        windDirectionId = ETELAN_PUOLEINEN;
      else
        windDirectionId = LOUNAAN_PUOLEINEN;
    }
    break;
    case 11:
    {
      windDirectionId = LOUNAAN_PUOLEINEN;
    }
    break;
    case 12:
    {
      if (theWindDirection < 247.5)
        windDirectionId = LOUNAAN_PUOLEINEN;
      else
        windDirectionId = LANNEN_PUOLEINEN;
    }
    break;
    case 13:
    {
      windDirectionId = LANNEN_PUOLEINEN;
    }
    break;
    case 14:
    {
      if (theWindDirection < 292.5)
        windDirectionId = LANNEN_PUOLEINEN;
      else
        windDirectionId = LUOTEEN_PUOLEINEN;
    }
    break;
    case 15:
    {
      windDirectionId = LUOTEEN_PUOLEINEN;
    }
    break;
    case 16:
    {
      if (theWindDirection < 337.5)
        windDirectionId = LUOTEEN_PUOLEINEN;
      else
        windDirectionId = POHJOISEN_PUOLEINEN;
    }
    break;
    default:
      break;
  }

  return windDirectionId;
}

WindDirectionId wind_direction_id(const TextGen::WeatherResult& theWindDirection,
                                  const TextGen::WeatherResult& theMaxWindSpeed,
                                  const string& theVariable,
                                  double theWindDirectionMinSpeed)
{
  WindDirectionAccuracy accuracy(direction_accuracy(theWindDirection.error(), theVariable));

  WindDirectionId windDirectionId(MISSING_WIND_DIRECTION_ID);

  // wind speed is >= theWindDirectionMinSpeed it can not be vaihteleva
  if (accuracy == bad_accuracy && theMaxWindSpeed.value() < theWindDirectionMinSpeed)
  {
    windDirectionId = VAIHTELEVA;
  }
  else
  {
    windDirectionId = static_cast<WindDirectionId>(direction16th(theWindDirection.value()));

    if (accuracy != good_accuracy)
      windDirectionId = puoleinen_direction_id(theWindDirection.value(), windDirectionId);
  }

  return windDirectionId;
}

WindDirectionId plain_wind_direction_id(const TextGen::WeatherResult& theWindDirection,
                                        const TextGen::WeatherResult& theMaxWindSpeed,
                                        const string& theVariable,
                                        double theWindDirectionMinSpeed)

{
  WindDirectionAccuracy accuracy(direction_accuracy(theWindDirection.error(), theVariable));

  WindDirectionId windDirectionId(MISSING_WIND_DIRECTION_ID);

  // wind speed is >= theWindDirectionMinSpeed m/s it can not be vaihteleva
  if (accuracy == bad_accuracy && theMaxWindSpeed.value() < theWindDirectionMinSpeed)
  {
    windDirectionId = VAIHTELEVA;
  }
  else
  {
    windDirectionId = static_cast<WindDirectionId>(direction16th(theWindDirection.value()));
  }

  return windDirectionId;
}

std::string wind_direction_string(const WindDirectionId& theWindDirectionId)
{
  std::string retval;

  switch (theWindDirectionId)
  {
    case POHJOINEN:
      retval = "pohjoinen";
      break;
    case POHJOISEN_PUOLEINEN:
      retval = "pohjoisen puoleinen";
      break;
    case POHJOINEN_KOILLINEN:
      retval = "pohjoisen ja koillisen valinen";
      break;
    case KOILLINEN:
      retval = "koillinen";
      break;
    case KOILLISEN_PUOLEINEN:
      retval = "koillisen puoleinen";
      break;
    case ITA_KOILLINEN:
      retval = "idan ja koillisen valinen";
      break;
    case ITA:
      retval = "ita";
      break;
    case IDAN_PUOLEINEN:
      retval = "idan puoleinen";
      break;
    case ITA_KAAKKO:
      retval = "idan ja kaakon valinen";
      break;
    case KAAKKO:
      retval = "kaakko";
      break;
    case KAAKON_PUOLEINEN:
      retval = "kaakon puoleinen";
      break;
    case ETELA_KAAKKO:
      retval = "etelan ja kaakon valinen";
      break;
    case ETELA:
      retval = "etela";
      break;
    case ETELAN_PUOLEINEN:
      retval = "etelan puoleinen";
      break;
    case ETELA_LOUNAS:
      retval = "etelan ja lounaan valinen";
      break;
    case LOUNAS:
      retval = "lounas";
      break;
    case LOUNAAN_PUOLEINEN:
      retval = "lounaan puoleinen";
      break;
    case LANSI_LOUNAS:
      retval = "lannen ja lounaan valinen";
      break;
    case LANSI:
      retval = "lansi";
      break;
    case LANNEN_PUOLEINEN:
      retval = "lannen puoleinen";
      break;
    case LANSI_LUODE:
      retval = "lannen ja luoteen valinen";
      break;
    case LUODE:
      retval = "luode";
      break;
    case LUOTEEN_PUOLEINEN:
      retval = "luoteen puoleinen";
      break;
    case POHJOINEN_LUODE:
      retval = "pohjoisen ja luoteen valinen";
      break;
    case VAIHTELEVA:
    case MISSING_WIND_DIRECTION_ID:
      retval = "vaihteleva";
      break;
  }

  return retval;
}

bool wind_direction_item_sort(pair<float, WeatherResult> firstItem,
                              pair<float, WeatherResult> secondItem)
{
  return firstItem.second.value() > secondItem.second.value();
}

pair<float, WeatherResult> get_share_item(
    const AnalysisSources& theSources,
    const WeatherArea& theArea,
    const WeatherPeriod& thePeriod,
    const string& theVar,
    const WindDirectionId& theWindDirection,
    WindStoryTools::CompassType compass_type = sixteen_directions)
{
  GridForecaster forecaster;
  RangeAcceptor acceptor;
  float ws_lower_limit(0.0);
  float ws_upper_limit(360.0);

  if (compass_type == sixteen_directions)
  {
    switch (theWindDirection)
    {
      case POHJOINEN:
      {
        ws_lower_limit = 0.0;
        ws_upper_limit = 11.25;
      }
      break;
      case POHJOINEN_KOILLINEN:
      {
        ws_lower_limit = 11.25;
        ws_upper_limit = 33.75;
      }
      break;
      case KOILLINEN:
      {
        ws_lower_limit = 33.75;
        ws_upper_limit = 56.25;
      }
      break;
      case ITA_KOILLINEN:
      {
        ws_lower_limit = 56.25;
        ws_upper_limit = 78.75;
      }
      break;
      case ITA:
      {
        ws_lower_limit = 78.75;
        ws_upper_limit = 101.25;
      }
      break;
      case ITA_KAAKKO:
      {
        ws_lower_limit = 101.25;
        ws_upper_limit = 123.75;
      }
      break;
      case KAAKKO:
      {
        ws_lower_limit = 123.75;
        ws_upper_limit = 146.25;
      }
      break;
      case ETELA_KAAKKO:
      {
        ws_lower_limit = 146.25;
        ws_upper_limit = 168.75;
      }
      break;
      case ETELA:
      {
        ws_lower_limit = 168.75;
        ws_upper_limit = 191.25;
      }
      break;
      case ETELA_LOUNAS:
      {
        ws_lower_limit = 191.25;
        ws_upper_limit = 213.75;
      }
      break;
      case LOUNAS:
      {
        ws_lower_limit = 213.75;
        ws_upper_limit = 236.25;
      }
      break;
      case LANSI_LOUNAS:
      {
        ws_lower_limit = 236.25;
        ws_upper_limit = 258.75;
      }
      break;
      case LANSI:
      {
        ws_lower_limit = 258.75;
        ws_upper_limit = 281.25;
      }
      break;
      case LANSI_LUODE:
      {
        ws_lower_limit = 281.25;
        ws_upper_limit = 303.75;
      }
      break;
      case LUODE:
      {
        ws_lower_limit = 303.75;
        ws_upper_limit = 326.25;
      }
      break;
      case POHJOINEN_LUODE:
      {
        ws_lower_limit = 326.25;
        ws_upper_limit = 348.75;
      }
      break;
      default:
        break;
    }
  }
  else
  {
    switch (theWindDirection)
    {
      case POHJOINEN:
      {
        ws_lower_limit = 0.0;
        ws_upper_limit = 22.50;
      }
      break;
      case KOILLINEN:
      {
        ws_lower_limit = 22.50;
        ws_upper_limit = 67.50;
      }
      break;
      case ITA:
      {
        ws_lower_limit = 67.50;
        ws_upper_limit = 112.50;
      }
      break;
      case KAAKKO:
      {
        ws_lower_limit = 112.50;
        ws_upper_limit = 157.50;
      }
      break;
      case ETELA:
      {
        ws_lower_limit = 157.50;
        ws_upper_limit = 202.50;
      }
      break;
      case LOUNAS:
      {
        ws_lower_limit = 202.50;
        ws_upper_limit = 247.50;
      }
      break;
      case LANSI:
      {
        ws_lower_limit = 247.50;
        ws_upper_limit = 292.50;
      }
      break;
      case LUODE:
      {
        ws_lower_limit = 292.50;
        ws_upper_limit = 337.50;
      }
      break;
      default:
        break;
    }
  }

  acceptor.lowerLimit(ws_lower_limit);
  acceptor.upperLimit(ws_upper_limit - 0.0001);
  WeatherResult share = forecaster.analyze(theVar + "::fake::tyyni::share",
                                           theSources,
                                           WindDirection,
                                           Mean,
                                           Percentage,
                                           theArea,
                                           thePeriod,
                                           DefaultAcceptor(),
                                           DefaultAcceptor(),
                                           acceptor);

  if (theWindDirection == POHJOINEN)
  {
    ws_lower_limit = (compass_type == sixteen_directions ? 348.75 : 337.50);
    ws_upper_limit = 360.0;
    acceptor.lowerLimit(ws_lower_limit);
    acceptor.upperLimit(ws_upper_limit);
    WeatherResult share2 = forecaster.analyze(theVar + "::fake::tyyni::share",
                                              theSources,
                                              WindDirection,
                                              Mean,
                                              Percentage,
                                              theArea,
                                              thePeriod,
                                              DefaultAcceptor(),
                                              DefaultAcceptor(),
                                              acceptor);
    share = WeatherResult(share.value() + share2.value(), 0.0);
  }

  pair<float, WeatherResult> shareItem(
      ws_lower_limit + (compass_type == sixteen_directions ? 11.25 : 22.50), share);

  return shareItem;
}

void populate_winddirection_distribution_time_series(
    const AnalysisSources& theSources,
    const WeatherArea& theArea,
    const WeatherPeriod& thePeriod,
    const std::string& theVar,
    std::vector<std::pair<float, WeatherResult> >& theWindDirectionDistribution,
    WindStoryTools::CompassType compass_type /* = sixteen_directions*/)
{
  theWindDirectionDistribution.push_back(
      get_share_item(theSources, theArea, thePeriod, theVar, POHJOINEN, compass_type));
  if (compass_type == sixteen_directions)
    theWindDirectionDistribution.push_back(
        get_share_item(theSources, theArea, thePeriod, theVar, POHJOINEN_KOILLINEN, compass_type));
  theWindDirectionDistribution.push_back(
      get_share_item(theSources, theArea, thePeriod, theVar, KOILLINEN, compass_type));
  if (compass_type == sixteen_directions)
    theWindDirectionDistribution.push_back(
        get_share_item(theSources, theArea, thePeriod, theVar, ITA_KOILLINEN, compass_type));
  theWindDirectionDistribution.push_back(
      get_share_item(theSources, theArea, thePeriod, theVar, ITA, compass_type));
  if (compass_type == sixteen_directions)
    theWindDirectionDistribution.push_back(
        get_share_item(theSources, theArea, thePeriod, theVar, ITA_KAAKKO, compass_type));
  theWindDirectionDistribution.push_back(
      get_share_item(theSources, theArea, thePeriod, theVar, KAAKKO, compass_type));
  if (compass_type == sixteen_directions)
    theWindDirectionDistribution.push_back(
        get_share_item(theSources, theArea, thePeriod, theVar, ETELA_KAAKKO, compass_type));
  theWindDirectionDistribution.push_back(
      get_share_item(theSources, theArea, thePeriod, theVar, ETELA, compass_type));
  if (compass_type == sixteen_directions)
    theWindDirectionDistribution.push_back(
        get_share_item(theSources, theArea, thePeriod, theVar, ETELA_LOUNAS, compass_type));
  theWindDirectionDistribution.push_back(
      get_share_item(theSources, theArea, thePeriod, theVar, LOUNAS, compass_type));
  if (compass_type == sixteen_directions)
    theWindDirectionDistribution.push_back(
        get_share_item(theSources, theArea, thePeriod, theVar, LANSI_LOUNAS, compass_type));
  theWindDirectionDistribution.push_back(
      get_share_item(theSources, theArea, thePeriod, theVar, LANSI, compass_type));
  if (compass_type == sixteen_directions)
    theWindDirectionDistribution.push_back(
        get_share_item(theSources, theArea, thePeriod, theVar, LANSI_LUODE, compass_type));
  theWindDirectionDistribution.push_back(
      get_share_item(theSources, theArea, thePeriod, theVar, LUODE, compass_type));
  if (compass_type == sixteen_directions)
    theWindDirectionDistribution.push_back(
        get_share_item(theSources, theArea, thePeriod, theVar, POHJOINEN_LUODE, compass_type));
}

float calculate_mean_wind_direction(const float& direction1,
                                    const float& direction2,
                                    const float& share1,
                                    const float& share2)
{
  float firstDirection(direction1);
  float secondDirection(direction2);

  if ((firstDirection > 180.0 && firstDirection <= 360.0) &&
      (secondDirection >= 0 && secondDirection <= 180))
  {
    if (firstDirection == 360.0)
    {
      firstDirection = 0;
    }
    else if (firstDirection - secondDirection > 180.0)
    {
      secondDirection += 360.0;
    }
  }
  else if ((secondDirection > 180.0 && secondDirection <= 360.0) &&
           (firstDirection >= 0 && firstDirection <= 180))
  {
    if (secondDirection == 360.0)
    {
      secondDirection = 0;
    }
    else if (secondDirection - firstDirection > 180.0)
    {
      firstDirection += 360.0;
    }
  }

  float totalShare(share1 + share2);
  float firstWeightedDirection(firstDirection * share1);
  float secondWeightedDirection(secondDirection * share2);
  float calculatedWeightedDirection((firstWeightedDirection + secondWeightedDirection) /
                                    totalShare);

  while (calculatedWeightedDirection > 360.0)
    calculatedWeightedDirection -= 360.0;

  return calculatedWeightedDirection;
}

float calculate_wind_direction_from_distribution(
    const std::vector<std::pair<float, WeatherResult> >& theDirectionDistribution)

{
  float cumulativeShare(theDirectionDistribution[0].second.value());
  float cumulativeWeightedDirection(theDirectionDistribution[0].first);

  // calculate mean direction until share is more than 85%
  if (cumulativeShare < 85.0)
  {
    for (unsigned int i = 1; i < theDirectionDistribution.size(); i++)
    {
      cumulativeWeightedDirection =
          calculate_mean_wind_direction(cumulativeWeightedDirection,
                                        theDirectionDistribution[i].first,
                                        cumulativeShare,
                                        theDirectionDistribution[i].second.value());
      cumulativeShare += theDirectionDistribution[i].second.value();

      if (cumulativeShare >= 85.0) break;
    }
  }

  return cumulativeWeightedDirection;
}

WeatherResult mode_wind_direction(const AnalysisSources& theSources,
                                  const WeatherArea& theArea,
                                  const WeatherPeriod& thePeriod,
                                  const WeatherResult& theMedianWind,
                                  const WeatherResult& theTopWind,
                                  const string& theVar)
{
  GridForecaster forecaster;

  WeatherResult meanDirection = forecaster.analyze(
      theVar + "::fake::wind:direction", theSources, WindDirection, Mean, Mean, theArea, thePeriod);

  float error(meanDirection.error());
  if (direction_accuracy(meanDirection.error(), theVar) == bad_accuracy)
  {
    if (theTopWind.value() > WEAK_WIND_SPEED_UPPER_LIMIT)
    {
      if (theMedianWind.value() >= WEAK_WIND_SPEED_UPPER_LIMIT)
      {
        error = 30.0;
      }
      else
      {
        float underWeakLimit = abs(theMedianWind.value() - WEAK_WIND_SPEED_UPPER_LIMIT);
        float overWeakLimit = abs(theTopWind.value() - WEAK_WIND_SPEED_UPPER_LIMIT);
        // if most of the wind speed range is over WEAK_WIND_SPEED_UPPER_LIMIT, direction must be
        // mentioned
        if (overWeakLimit >= underWeakLimit) error = 30;
      }
    }
  }

  std::vector<std::pair<float, WeatherResult> > directionDistribution;

  populate_winddirection_distribution_time_series(
      theSources, theArea, thePeriod, theVar, directionDistribution);

  std::sort(directionDistribution.begin(), directionDistribution.end(), wind_direction_item_sort);

  return WeatherResult(calculate_wind_direction_from_distribution(directionDistribution), error);
}

}  // namespace WindStoryTools
}  // namespace TextGen

// ======================================================================
