#include "WeatherStory.h"
#include "CloudinessStory.h"
#include "CloudinessStoryTools.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "NightAndDayPeriodGenerator.h"
#include "PeriodPhraseFactory.h"
#include "PrecipitationStoryTools.h"
#include <calculator/RangeAcceptor.h>
#include "ValueAcceptor.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include <calculator/WeatherResult.h>
#include "WeekdayTools.h"
#include <calculator/NullPeriodGenerator.h>
#include <calculator/WeatherPeriodTools.h>
#include "AreaTools.h"
#include <calculator/MathTools.h>
#include "SeasonTools.h"
#include "SubMaskExtractor.h"
#include "WeatherForecast.h"
#include "TemperatureStoryTools.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <map>

#include "DebugTextFormatter.h"

namespace TextGen
{
using namespace Settings;
using namespace TextGen;
using namespace TemperatureStoryTools;
using namespace AreaTools;
using namespace boost;
using namespace std;

std::string as_string(const GlyphContainer& gc);

const char* weather_event_string(weather_event_id theWeatherEventId)
{
  switch (theWeatherEventId)
  {
    case PILVISTYY:
      return PILVISTYVAA_WORD;
    case SELKENEE:
      return SELKENEVAA_WORD;
    case POUTAANTUU:
      return SAA_POUTAANTUU_PHRASE;
    case POUTAANTUU_WHEN_EXTENT_SMALL:
      return "saa poutaantuu kattavuudeltaan suppean sateen jalkeen";
    case SADE_ALKAA:
      return SADE_ALKAA_PHRASE;
    default:
      return "missing weather event";
  }
}

const char* precipitation_form_string(precipitation_form_id thePrecipitationForm)
{
  switch (thePrecipitationForm)
  {
    case WATER_FORM:
      return "WATER";
    case DRIZZLE_FORM:
      return "DRIZZLE";
    case SLEET_FORM:
      return "SLEET";
    case SNOW_FORM:
      return "SNOW";
    case FREEZING_FORM:
      return "FREEZING";
    case WATER_DRIZZLE_FORM:
      return "WATER_DRIZZLE";
    case WATER_SLEET_FORM:
      return "WATER_SLEET";
    case WATER_SNOW_FORM:
      return "WATER_SNOW";
    case WATER_FREEZING_FORM:
      return "WATER_FREEZING";
    case WATER_DRIZZLE_SLEET_FORM:
      return "WATER_DRIZZLE_SLEET";
    case WATER_DRIZZLE_SNOW_FORM:
      return "WATER_DRIZZLE_SNOW";
    case WATER_DRIZZLE_FREEZING_FORM:
      return "WATER_DRIZZLE_FREEZING";
    case WATER_SLEET_SNOW_FORM:
      return "WATER_SLEET_SNOW";
    case WATER_SLEET_FREEZING_FORM:
      return "WATER_SLEET_FREEZING";
    case WATER_SNOW_FREEZING_FORM:
      return "WATER_SNOW_FREEZING";
    case DRIZZLE_SLEET_FORM:
      return "DRIZZLE_SLEET";
    case DRIZZLE_SNOW_FORM:
      return "DRIZZLE_SNOW";
    case DRIZZLE_FREEZING_FORM:
      return "DRIZZLE_FREEZING";
    case DRIZZLE_SLEET_SNOW_FORM:
      return "DRIZZLE_SLEET_SNOW";
    case DRIZZLE_SLEET_FREEZING_FORM:
      return "DRIZZLE_SLEET_FREEZING";
    case DRIZZLE_SNOW_FREEZING_FORM:
      return "DRIZZLE_SNOW_FREEZING";
    case SLEET_SNOW_FORM:
      return "SLEET_SNOW";
    case SLEET_FREEZING_FORM:
      return "SLEET_FREEZING";
    case SLEET_SNOW_FREEZING_FORM:
      return "SLEET_SNOW_FREEZING";
    case SNOW_FREEZING_FORM:
      return "SNOW_FREEZING";
    default:
      return "MISSING_VALUE";
  }
}

const char* precipitation_type_string(precipitation_type thePrecipitationType)
{
  switch (thePrecipitationType)
  {
    case CONTINUOUS:
      return "continuous";
    case SHOWERS:
      return "showers";
    case MISSING_PRECIPITATION_TYPE:
      return "MISSING_PRECIPITATION_TYPE";
    default:
      return "";
  }
}

const char* precipitation_traverse_string(precipitation_traverse_id thePrecipitationTraverseId)
{
  switch (thePrecipitationTraverseId)
  {
    case FROM_SOUTH_TO_NORTH:
      return "etelasta pohjoiseen";
    case FROM_NORTH_TO_SOUTH:
      return "pohjoiseta etelaan";
    case FROM_EAST_TO_WEST:
      return "idasta lanteen";
    case FROM_WEST_TO_EAST:
      return "lannesta itaan";
    case FROM_NORTHEAST_TO_SOUTHWEST:
      return "koillisesta lounaaseen";
    case FROM_SOUTHWEST_TO_NORTHEAST:
      return "lounaasta koilliseen";
    case FROM_NORTHWEST_TO_SOUTHEAST:
      return "luoteesta kaakkoon";
    case FROM_SOUTHEAST_TO_NORTHWEST:
      return "kaakosta luoteeseen";
    case MISSING_TRAVERSE_ID:
      return "ei minnekaan";
    // For some reason g++ wants this even though all enum values are handled above
    default:
      return "";
  }
}

const char* part_of_the_day_string(part_of_the_day_id thePartOfTheDayId)
{
  switch (thePartOfTheDayId)
  {
    case AAMU:
      return "aamu";
    case AAMUPAIVA:
      return "aamupaiva";
    case ILTAPAIVA:
      return "iltapaiva";
    case ILTA:
      return "ilta";
    case ILTAYO:
      return "iltayo";
    case KESKIYO:
      return "keskiyo";
    case AAMUYO:
      return "aamuyo";
    case PAIVA:
      return "paiva";
    case YO:
      return "yo";
    case AAMU_JA_AAMUPAIVA:
      return "aamu ja aamupaiva";
    case ILTAPAIVA_JA_ILTA:
      return "iltapaiva ja ilta";
    case ILTA_JA_ILTAYO:
      return "ilta ja iltayo";
    case ILTAYO_JA_KESKIYO:
      return "iltayo ja keskiyo";
    case KESKIYO_JA_AAMUYO:
      return "keskiyo ja aamu";
    case AAMUYO_JA_AAMU:
      return "aamuyo ja aamu";
    default:
      return "missing";
  }
}

const char* story_part_id_string(story_part_id theStoryPartId)
{
  switch (theStoryPartId)
  {
    case PRECIPITATION_STORY_PART:
      return "precipitation story part";
    case CLOUDINESS_STORY_PART:
      return "cloudiness story part";
    case GETTING_CLOUDY_STORY_PART:
      return "getting cloudy story part";
    case CLEARING_UP_STORY_PART:
      return "clearing up story part";
    case PRECIPITATION_TYPE_CHANGE_STORY_PART:
      return "precipitation type change story part";
    default:
      return "missing story part";
  }
}

part_of_the_day_id get_part_of_the_day_id(const TextGenPosixTime& theTimestamp,
                                          bool ignoreKeskiyo /*= false*/)
{
  // if keskiyö is ignored start aamuyö from midnight
  int aamuyoStart = (ignoreKeskiyo ? KESKIYO_START : AAMUYO_START);

  if (theTimestamp.GetHour() >= AAMU_START && theTimestamp.GetHour() <= AAMU_END)
    return AAMU;
  else if (theTimestamp.GetHour() >= AAMUPAIVA_START && theTimestamp.GetHour() <= AAMUPAIVA_END)
    return AAMUPAIVA;
  else if (theTimestamp.GetHour() >= ILTA_START && theTimestamp.GetHour() <= ILTA_END)
    return ILTA;
  else if (theTimestamp.GetHour() >= ILTAPAIVA_START && theTimestamp.GetHour() <= ILTAPAIVA_END)
    return ILTAPAIVA;
  else if (theTimestamp.GetHour() >= ILTAYO_START)
    return ILTAYO;
  else if (!ignoreKeskiyo && theTimestamp.GetHour() <= KESKIYO_END)
    return KESKIYO;
  else if (theTimestamp.GetHour() >= aamuyoStart && theTimestamp.GetHour() <= AAMUYO_END)
    return AAMUYO;

  return MISSING_PART_OF_THE_DAY_ID;
}

part_of_the_day_id get_part_of_the_day_id_large(const WeatherPeriod& thePeriod)
{
  if (thePeriod.localEndTime().DifferenceInHours(thePeriod.localStartTime()) > 10)
    return MISSING_PART_OF_THE_DAY_ID;

  bool insideSameDay =
      thePeriod.localStartTime().GetJulianDay() == thePeriod.localEndTime().GetJulianDay();
  bool successiveDays = false;
  if (thePeriod.localStartTime().GetYear() == thePeriod.localEndTime().GetYear() - 1)
  {
    successiveDays =
        thePeriod.localStartTime().GetMonth() == 12 && thePeriod.localStartTime().GetDay() == 31 &&
        thePeriod.localEndTime().GetMonth() == 1 && thePeriod.localEndTime().GetDay() == 1;
  }
  else
  {
    successiveDays =
        thePeriod.localStartTime().GetJulianDay() == thePeriod.localEndTime().GetJulianDay() - 1;
  }

  part_of_the_day_id narrow_id(get_part_of_the_day_id_narrow(thePeriod));

  if (narrow_id != MISSING_PART_OF_THE_DAY_ID)
    return narrow_id;
  else if (thePeriod.localStartTime().GetHour() >= AAMU_START &&
           thePeriod.localEndTime().GetHour() <= AAMUPAIVA_END && insideSameDay)
    return AAMU_JA_AAMUPAIVA;
  else if (thePeriod.localStartTime().GetHour() >= ILTAPAIVA_START &&
           thePeriod.localEndTime().GetHour() <= ILTA_END && insideSameDay)
    return ILTAPAIVA_JA_ILTA;
  else if ((thePeriod.localStartTime().GetHour() >= ILTA_START &&
            thePeriod.localEndTime().GetHour() <= ILTAYO_END && insideSameDay))
    return ILTA_JA_ILTAYO;
  else if (thePeriod.localStartTime().GetHour() >= ILTAYO_START &&
           thePeriod.localEndTime().GetHour() <= KESKIYO_END && successiveDays)
    return YO;  // ILTAYO_JA_KESKIYO;
  else if (thePeriod.localStartTime().GetHour() >= KESKIYO_START &&
           thePeriod.localEndTime().GetHour() <= AAMUYO_END && insideSameDay)
    return YO;  // KESKIYO_JA_AAMUYO;
  else if (thePeriod.localStartTime().GetHour() >= AAMUYO_START &&
           thePeriod.localEndTime().GetHour() <= AAMU_END && insideSameDay)
    return AAMUYO_JA_AAMU;
  else if (thePeriod.localStartTime().GetHour() >= YO_START &&
           thePeriod.localEndTime().GetHour() <= YO_END && successiveDays)
    return YO;
  else if (thePeriod.localStartTime().GetHour() >= PAIVA_START - 2 &&
           thePeriod.localEndTime().GetHour() <= PAIVA_END && insideSameDay)
    return PAIVA;

  return MISSING_PART_OF_THE_DAY_ID;
}

part_of_the_day_id get_part_of_the_day_id_narrow(const WeatherPeriod& thePeriod,
                                                 bool ignoreKeskiyo /*= false*/)
{
  if (thePeriod.localEndTime().DifferenceInHours(thePeriod.localStartTime()) > 10)
    return MISSING_PART_OF_THE_DAY_ID;

  bool insideSameDay =
      thePeriod.localStartTime().GetJulianDay() == thePeriod.localEndTime().GetJulianDay();
  // if keskiyö is ignored start aamuyö from midnight
  int aamuyoStart = (ignoreKeskiyo ? KESKIYO_START : AAMUYO_START);

  if (thePeriod.localStartTime().GetHour() >= AAMU_START &&
      thePeriod.localEndTime().GetHour() <= AAMU_END && insideSameDay)
  {
    if (thePeriod.localStartTime().GetHour() >= AAMUPAIVA_START &&
        thePeriod.localEndTime().GetHour() <= AAMUPAIVA_END && insideSameDay)
      return AAMUPAIVA;
    else
      return AAMU;
  }
  else if (thePeriod.localStartTime().GetHour() >= AAMUPAIVA_START &&
           thePeriod.localEndTime().GetHour() <= AAMUPAIVA_END && insideSameDay)
  {
    return AAMUPAIVA;
  }
  else if (thePeriod.localStartTime().GetHour() >= ILTA_START &&
           thePeriod.localEndTime().GetHour() <= ILTA_END && insideSameDay)
  {
    if (thePeriod.localStartTime().GetHour() >= ILTAYO_START &&
        thePeriod.localEndTime().GetHour() <= ILTAYO_END && insideSameDay)
      return ILTAYO;
    else
      return ILTA;
  }
  else if (thePeriod.localStartTime().GetHour() >= ILTAPAIVA_START &&
           thePeriod.localEndTime().GetHour() <= ILTAPAIVA_END && insideSameDay)
    return ILTAPAIVA;
  else if (thePeriod.localStartTime().GetHour() >= ILTAYO_START &&
           thePeriod.localEndTime().GetHour() <= ILTAYO_END && insideSameDay)
    return ILTAYO;
  else if (!ignoreKeskiyo && thePeriod.localStartTime().GetHour() >= KESKIYO_START &&
           thePeriod.localEndTime().GetHour() <= KESKIYO_END && insideSameDay)
  {
    if (thePeriod.localStartTime().GetHour() >= AAMUYO_START &&
        thePeriod.localEndTime().GetHour() <= AAMUYO_END && insideSameDay)
      return AAMUYO;
    else
      return KESKIYO;
  }
  else if (thePeriod.localStartTime().GetHour() >= aamuyoStart &&
           thePeriod.localEndTime().GetHour() <= AAMUYO_END && insideSameDay)
    return AAMUYO;

  return MISSING_PART_OF_THE_DAY_ID;
}

bool is_same_part_of_day(const WeatherPeriod& thePeriod1, const WeatherPeriod& thePeriod2)
{
  part_of_the_day_id id1 = get_part_of_the_day_id_narrow(thePeriod1);
  part_of_the_day_id id2 = get_part_of_the_day_id_narrow(thePeriod2);
  int periodLength1 = get_period_length(thePeriod1);
  int periodLength2 = get_period_length(thePeriod2);
  int period1StartHour = thePeriod1.localStartTime().GetHour();
  int period2StartHour = thePeriod2.localStartTime().GetHour();

  if (id1 == KESKIYO) id1 = AAMUYO;
  if (id2 == KESKIYO) id2 = AAMUYO;

  if (id1 == id2) return true;

  // check spceial cases when previous period ends at the same time as next starts
  if ((id1 == AAMUYO && id2 == AAMU && periodLength1 == 0 && period1StartHour == 6) ||
      (id2 == AAMUYO && id1 == AAMU && periodLength2 == 0 && period2StartHour == 6) ||
      (id1 == AAMU && id2 == AAMUPAIVA && periodLength1 == 0 && period1StartHour == 9) ||
      (id2 == AAMU && id1 == AAMUPAIVA && periodLength2 == 0 && period2StartHour == 9) ||
      (id1 == AAMUPAIVA && id2 == ILTAPAIVA && periodLength1 == 0 && period1StartHour == 12) ||
      (id2 == AAMUPAIVA && id1 == ILTAPAIVA && periodLength2 == 0 && period2StartHour == 12))
    return true;

  return (get_part_of_the_day_id_narrow(thePeriod1) == get_part_of_the_day_id_narrow(thePeriod2));
}

void get_part_of_the_day(part_of_the_day_id thePartOfTheDayId, int& theStartHour, int& theEndHour)
{
  switch (thePartOfTheDayId)
  {
    case AAMU:
    {
      theStartHour = AAMU_START;
      theEndHour = AAMU_END;
    }
    break;
    case AAMUPAIVA:
    {
      theStartHour = AAMUPAIVA_START;
      theEndHour = AAMUPAIVA_END;
    }
    break;
    case ILTAPAIVA:
    {
      theStartHour = ILTAPAIVA_START;
      theEndHour = ILTAPAIVA_END;
    }
    break;
    case ILTA:
    {
      theStartHour = ILTA_START;
      theEndHour = ILTA_END;
    }
    break;
    case ILTAYO:
    {
      theStartHour = ILTAYO_START;
      theEndHour = ILTAYO_END;
    }
    break;
    case KESKIYO:
    {
      theStartHour = KESKIYO_START;
      theEndHour = KESKIYO_END;
    }
    break;
    case AAMUYO:
    {
      theStartHour = AAMUYO_START;
      theEndHour = AAMUYO_END;
    }
    break;
    case PAIVA:
    {
      theStartHour = PAIVA_START;
      theEndHour = PAIVA_END;
    }
    break;
    case YO:
    {
      theStartHour = YO_START;
      theEndHour = YO_END;
    }
    break;
    case AAMU_JA_AAMUPAIVA:
    {
      theStartHour = AAMU_START;
      theEndHour = AAMUPAIVA_END;
    }
    break;
    case ILTAPAIVA_JA_ILTA:
    {
      theStartHour = ILTAPAIVA_START;
      theEndHour = ILTA_END;
    }
    break;
    case ILTA_JA_ILTAYO:
    {
      theStartHour = ILTA_START;
      theEndHour = ILTAYO_END;
    }
    break;
    case ILTAYO_JA_KESKIYO:
    {
      theStartHour = ILTAYO_START;
      theEndHour = KESKIYO_END;
    }
    break;
    case KESKIYO_JA_AAMUYO:
    {
      theStartHour = KESKIYO_START;
      theEndHour = AAMUYO_END;
    }
    break;
    case AAMUYO_JA_AAMU:
    {
      theStartHour = AAMUYO_START;
      theEndHour = AAMU_END;
    }
    break;
    default:
    {
      theStartHour = -1;
      theEndHour = -1;
    }
  }
}

part_of_the_day_id get_most_relevant_part_of_the_day_id_narrow(const WeatherPeriod& thePeriod,
                                                               bool theAlkaenPhrase)
{
  part_of_the_day_id idLarge = get_part_of_the_day_id_large(thePeriod);

  if (idLarge < PAIVA) return idLarge;

  part_of_the_day_id idBeg = get_part_of_the_day_id_narrow(
      WeatherPeriod(thePeriod.localStartTime(), thePeriod.localStartTime()));
  part_of_the_day_id idEnd = get_part_of_the_day_id_narrow(
      WeatherPeriod(thePeriod.localEndTime(), thePeriod.localEndTime()));

  // if long period return id from start
  if (get_period_length(thePeriod) > 6)
  {
    int firstHour = thePeriod.localStartTime().GetHour();
    if (firstHour == 12)
      return ILTAPAIVA;
    else if (firstHour == 0)
      return AAMUYO;
    else if (firstHour == 17 || firstHour == 18)
      return ILTA;
    else if (firstHour == 6)
      return AAMU;

    return idBeg;
  }
  int hoursInTheFirstPart(0);
  int hoursInTheSecondPart(0);
  int startHour(0);
  int endHour(0);

  get_part_of_the_day(idBeg, startHour, endHour);

  // change endtime forwards till endHour for the part_of_the_day is reached
  TextGenPosixTime endTimeFirstPart = thePeriod.localStartTime();
  while (endTimeFirstPart.GetHour() != endHour)
    endTimeFirstPart.ChangeByHours(1);

  if (idBeg == YO && thePeriod.localStartTime().GetHour() > 6)
    hoursInTheFirstPart = (24 - thePeriod.localStartTime().GetHour()) + endHour;
  else
    hoursInTheFirstPart = endHour - thePeriod.localStartTime().GetHour();

  get_part_of_the_day(idEnd, startHour, endHour);

  if (idEnd == YO && thePeriod.localEndTime().GetHour() > 6)
    hoursInTheSecondPart = (24 - thePeriod.localEndTime().GetHour()) + endHour;
  else
    hoursInTheSecondPart = endHour - thePeriod.localEndTime().GetHour();

  TextGenPosixTime endTimeSecondPart = thePeriod.localEndTime();

  // change endtime backwards till startHour for the part_of_the_day is reached
  while (endTimeSecondPart.GetHour() != startHour)
    endTimeSecondPart.ChangeByHours(-1);

  hoursInTheFirstPart = abs(endTimeFirstPart.DifferenceInHours(thePeriod.localStartTime()));

  hoursInTheSecondPart = abs(endTimeSecondPart.DifferenceInHours(thePeriod.localEndTime()));

  return ((hoursInTheFirstPart >= hoursInTheSecondPart || theAlkaenPhrase) ? idBeg : idEnd);
}

WeatherPeriod intersecting_period(const WeatherPeriod& theWeatherPeriod1,
                                  const WeatherPeriod& theWeatherPeriod2)
{
  if (theWeatherPeriod1.localEndTime() < theWeatherPeriod2.localStartTime() ||
      theWeatherPeriod1.localStartTime() > theWeatherPeriod2.localEndTime())
    return WeatherPeriod(TextGenPosixTime(1970, 1, 1), TextGenPosixTime(1970, 1, 1));

  if (is_inside(theWeatherPeriod1.localStartTime(), theWeatherPeriod2) &&
      is_inside(theWeatherPeriod1.localEndTime(), theWeatherPeriod2))
    return theWeatherPeriod1;
  if (is_inside(theWeatherPeriod2.localStartTime(), theWeatherPeriod1) &&
      is_inside(theWeatherPeriod2.localEndTime(), theWeatherPeriod1))
    return theWeatherPeriod2;

  TextGenPosixTime startTime(theWeatherPeriod1.localStartTime() > theWeatherPeriod2.localStartTime()
                                 ? theWeatherPeriod1.localStartTime()
                                 : theWeatherPeriod2.localStartTime());
  TextGenPosixTime endTime(theWeatherPeriod1.localEndTime() < theWeatherPeriod2.localEndTime()
                               ? theWeatherPeriod1.localEndTime()
                               : theWeatherPeriod2.localEndTime());

  return WeatherPeriod(startTime, endTime);
}

bool is_inside(const WeatherPeriod& theWeatherPeriod1, const WeatherPeriod& theWeatherPeriod2)
{
  return (is_inside(theWeatherPeriod1.localStartTime(), theWeatherPeriod2) &&
          is_inside(theWeatherPeriod1.localEndTime(), theWeatherPeriod2));
}

bool is_inside(const TextGenPosixTime& theTimeStamp, const WeatherPeriod& theWeatherPeriod)
{
  return theTimeStamp >= theWeatherPeriod.localStartTime() &&
         theTimeStamp <= theWeatherPeriod.localEndTime();
}

bool same_period(const WeatherPeriod& theWeatherPeriod1, const WeatherPeriod& theWeatherPeriod2)
{
  return (theWeatherPeriod1.localStartTime() == theWeatherPeriod2.localStartTime() &&
          theWeatherPeriod1.localEndTime() == theWeatherPeriod2.localEndTime());
}

bool is_inside(const TextGenPosixTime& theTimeStamp, part_of_the_day_id thePartOfTheDayId)
{
  int startHour, endHour;
  int timestampHour(theTimeStamp.GetHour());
  get_part_of_the_day(thePartOfTheDayId, startHour, endHour);

  if (endHour == 0)
    return timestampHour >= startHour || endHour == timestampHour;
  else if (startHour == 0)
    return timestampHour <= endHour;
  else
  {
    if (thePartOfTheDayId == YO || thePartOfTheDayId == ILTAYO_JA_KESKIYO)
    {
      return timestampHour >= startHour || timestampHour <= endHour;
    }
    else
    {
      return (timestampHour >= startHour && timestampHour <= endHour);
    }
  }
}

bool is_inside(const WeatherPeriod& theWeatherPeriod, part_of_the_day_id thePartOfTheDayId)
{
  int numberOfDays(theWeatherPeriod.localEndTime().GetJulianDay() -
                   theWeatherPeriod.localStartTime().GetJulianDay());
  if (theWeatherPeriod.localEndTime().DifferenceInHours(theWeatherPeriod.localStartTime()) > 10)
  {
    return false;
  }
  else if (numberOfDays == 1)
  {
    if (thePartOfTheDayId != ILTAYO && thePartOfTheDayId != YO &&
        thePartOfTheDayId != ILTAYO_JA_KESKIYO && thePartOfTheDayId != ILTA_JA_ILTAYO)
      return false;
  }

  if (thePartOfTheDayId == YO && is_inside(theWeatherPeriod.localStartTime(), YO) &&
      is_inside(theWeatherPeriod.localEndTime(), YO))
    return true;
  else if (thePartOfTheDayId == ILTAYO_JA_KESKIYO &&
           is_inside(theWeatherPeriod.localStartTime(), ILTAYO_JA_KESKIYO) &&
           is_inside(theWeatherPeriod.localEndTime(), ILTAYO_JA_KESKIYO))
    return true;

  int startHour, endHour;
  get_part_of_the_day(thePartOfTheDayId, startHour, endHour);
  TextGenPosixTime startTimeCompare(theWeatherPeriod.localStartTime());
  TextGenPosixTime endTimeCompare(theWeatherPeriod.localStartTime());

  startTimeCompare.SetHour(startHour);
  startTimeCompare.SetMin(0);
  startTimeCompare.SetSec(0);
  endTimeCompare.SetHour(endHour);
  endTimeCompare.SetMin(0);
  endTimeCompare.SetSec(0);

  if (endHour == 0)
  {
    endTimeCompare.ChangeByDays(1);
  }

  return (theWeatherPeriod.localStartTime() >= startTimeCompare &&
          theWeatherPeriod.localStartTime() <= endTimeCompare &&
          theWeatherPeriod.localEndTime() >= startTimeCompare &&
          theWeatherPeriod.localEndTime() <= endTimeCompare);
}

std::string get_time_phrase_from_id(part_of_the_day_id thePartOfTheDayId,
                                    const std::string& theVar,
                                    bool theAlkaenPhrase /*= false*/)
{
  std::string retval("");

  bool specify_part_of_the_day =
      Settings::optional_bool(theVar + "::specify_part_of_the_day", true);

  if (!specify_part_of_the_day) return retval;

  switch (thePartOfTheDayId)
  {
    case AAMU:
    {
      retval = (theAlkaenPhrase ? AAMUSTA_ALKAEN_PHRASE : AAMULLA_WORD);
    }
    break;
    case AAMUPAIVA:
    {
      retval = (theAlkaenPhrase ? AAMUPAIVASTA_ALKAEN_PHRASE : AAMUPAIVALLA_WORD);
    }
    break;
    case ILTA:
    {
      retval = (theAlkaenPhrase ? ILLASTA_ALKAEN_PHRASE : ILLALLA_WORD);
    }
    break;
    case ILTAPAIVA:
    {
      retval = (theAlkaenPhrase ? ILTAPAIVASTA_ALKAEN_PHRASE : ILTAPAIVALLA_WORD);
    }
    break;
    case ILTAYO:
    {
      retval = (theAlkaenPhrase ? ILTAYOSTA_ALKAEN_PHRASE : ILTAYOLLA_WORD);
    }
    break;
    case KESKIYO:
    {
      retval = (theAlkaenPhrase ? KESKIYOSTA_ALKAEN_PHRASE : KESKIYOLLA_WORD);
    }
    break;
    case AAMUYO:
    {
      retval = (theAlkaenPhrase ? AAMUYOSTA_ALKAEN_PHRASE : AAMUYOLLA_WORD);
    }
    break;
    case AAMU_JA_AAMUPAIVA:
    {
      retval = (theAlkaenPhrase ? AAMUSTA_ALKAEN_PHRASE : AAMULLA_JA_AAMUPAIVALLA_PHRASE);
    }
    break;
    case ILTAPAIVA_JA_ILTA:
    {
      retval = (theAlkaenPhrase ? ILTAPAIVASTA_ALKAEN_PHRASE : ILTAPAIVALLA_JA_ILLALLA_PHRASE);
    }
    break;
    case ILTA_JA_ILTAYO:
    {
      retval = (theAlkaenPhrase ? ILLASTA_ALKAEN_PHRASE : ILLALLA_JA_ILTAYOLLA_PHRASE);
    }
    break;
    case ILTAYO_JA_KESKIYO:
    {
      retval = (theAlkaenPhrase ? ILTAYOSTA_ALKAEN_PHRASE : ILTAYOLLA_JA_KESKIYOLLA_PHRASE);
    }
    break;
    case KESKIYO_JA_AAMUYO:
    {
      retval = (theAlkaenPhrase ? KESKIYOSTA_ALKAEN_PHRASE : KESKIYOLLA_JA_AAMUYOLLA_PHRASE);
    }
    break;
    case AAMUYO_JA_AAMU:
    {
      retval = (theAlkaenPhrase ? AAMUYOSTA_ALKAEN_PHRASE : AAMUYOLLA_JA_AAMULLA_PHRASE);
    }
    break;
    default:
      break;
  };

  return retval;
}

std::string get_narrow_time_phrase(const WeatherPeriod& theWeatherPeriod,
                                   const std::string& theVar,
                                   part_of_the_day_id& thePartOfTheDay,
                                   bool theAlkaenPhrase /*= false*/)
{
  thePartOfTheDay = get_part_of_the_day_id_narrow(theWeatherPeriod);
  return get_time_phrase_from_id(thePartOfTheDay, theVar, theAlkaenPhrase);
}

Sentence get_large_time_phrase(const WeatherPeriod& theWeatherPeriod,
                               bool theSpecifyDayFlag,
                               std::string& thePhraseString,
                               part_of_the_day_id& thePartOfTheDay)
{
  Sentence sentence;

  std::ostringstream oss;

  short weekday = theWeatherPeriod.localStartTime().GetWeekday();

  if (is_inside(theWeatherPeriod, AAMU_JA_AAMUPAIVA))
  {
    if (theSpecifyDayFlag) oss << weekday << "-";
    oss << AAMULLA_WORD << SPACE_STRING << JA_WORD << SPACE_STRING << AAMUPAIVALLA_WORD;
    sentence << oss.str();
    thePhraseString = oss.str();
    thePartOfTheDay = AAMU_JA_AAMUPAIVA;
  }
  else if (is_inside(theWeatherPeriod, ILTAPAIVA_JA_ILTA))
  {
    if (theSpecifyDayFlag) oss << weekday << "-";
    oss << ILTAPAIVALLA_WORD << SPACE_STRING << JA_WORD << SPACE_STRING << ILLALLA_WORD;
    sentence << oss.str();
    thePhraseString = oss.str();
    thePartOfTheDay = ILTAPAIVA_JA_ILTA;
  }
  else if (is_inside(theWeatherPeriod, ILTA_JA_ILTAYO))
  {
    if (theSpecifyDayFlag) oss << weekday << "-";
    oss << ILLALLA_WORD << SPACE_STRING << JA_WORD << SPACE_STRING << ILTAYOLLA_WORD;
    sentence << oss.str();
    thePhraseString = oss.str();
    thePartOfTheDay = ILTA_JA_ILTAYO;
  }
  else if (is_inside(theWeatherPeriod, ILTAYO_JA_KESKIYO) ||
           is_inside(theWeatherPeriod, KESKIYO_JA_AAMUYO))
  {
    if (theSpecifyDayFlag)
    {
      oss << theWeatherPeriod.localEndTime().GetWeekday() << "-vastaisena yona";
    }
    else
    {
      oss << YOLLA_WORD;
    }
    sentence << oss.str();
    thePhraseString = oss.str();
    thePartOfTheDay = YO;
  }
  /*
  else if (is_inside(theWeatherPeriod, ILTAYO_JA_KESKIYO))
  {
    if (theSpecifyDayFlag)
    {
      if (theWeatherPeriod.localStartTime().GetHour() >= 23)
      {
        if (theSpecifyDayFlag) oss << weekday << "-";
        oss << KESKIYOLLA_WORD;

        thePhraseString = oss.str();
        sentence << oss.str();
      }
      else
      {
        // iltayo
        if (theSpecifyDayFlag) oss << weekday << "-";
        oss << ILTAYOLLA_WORD;

        thePhraseString = oss.str();
        sentence << oss.str();
      }
    }
    else
    {
      oss << ILTAYOLLA_WORD << SPACE_STRING << JA_WORD << SPACE_STRING << KESKIYOLLA_WORD;
      sentence << oss.str();
      thePhraseString = oss.str();
    }
  }
  else if (is_inside(theWeatherPeriod, KESKIYO_JA_AAMUYO))
  {
    if (theSpecifyDayFlag)
    {
      oss << theWeatherPeriod.localEndTime().GetWeekday() << "-vastaisena yona";
      sentence << oss.str();
      thePhraseString = oss.str();
    }
    else
    {
      thePhraseString = KESKIYOLLA_WORD;
      thePhraseString += SPACE_STRING;
      thePhraseString += JA_WORD;
      thePhraseString += SPACE_STRING;
      thePhraseString += AAMUYOLLA_WORD;
      sentence << thePhraseString;
    }
  }
  */
  else if (is_inside(theWeatherPeriod, AAMUYO_JA_AAMU))
  {
    if (theSpecifyDayFlag)
      oss << weekday << "-" << AAMUYOLLA_WORD << SPACE_STRING << JA_WORD << SPACE_STRING
          << AAMULLA_WORD;
    else
      oss << AAMUYOLLA_WORD << SPACE_STRING << JA_WORD << SPACE_STRING << AAMULLA_WORD;

    sentence << oss.str();
    thePhraseString = oss.str();
    thePartOfTheDay = AAMUYO_JA_AAMU;
  }
  else if (is_inside(theWeatherPeriod, YO))
  {
    if (theSpecifyDayFlag)
    {
      oss << theWeatherPeriod.localEndTime().GetWeekday() << "-vastaisena yona";
    }
    else
    {
      oss << YOLLA_WORD;
    }
    sentence << oss.str();
    thePhraseString = oss.str();
    thePartOfTheDay = YO;
  }
  else if (is_inside(theWeatherPeriod, PAIVA))
  {
    if (theSpecifyDayFlag)
      oss << weekday << "-" << PAIVALLA_WORD;
    else
      oss << PAIVALLA_WORD;

    sentence << oss.str();
    thePhraseString = oss.str();
    thePartOfTheDay = PAIVA;
  }

  return sentence;
}

std::string parse_time_phrase(short theWeekday,
                              bool theSpecifyDayFlag,
                              const std::string& theTimePhrase)
{
  std::ostringstream oss;

  if (!theTimePhrase.empty())
  {
    if (theWeekday > 0 && theSpecifyDayFlag) oss << theWeekday << "-";
    oss << theTimePhrase;
  }

  return oss.str();
}

bool fit_into_narrow_day_part(const WeatherPeriod& thePeriod)
{
  return get_part_of_the_day_id_narrow(thePeriod) != MISSING_PART_OF_THE_DAY_ID;
}

bool fit_into_large_day_part(const WeatherPeriod& thePeriod)
{
  return get_part_of_the_day_id_large(thePeriod) != MISSING_PART_OF_THE_DAY_ID;
}

Sentence get_time_phrase_large(const WeatherPeriod& theWeatherPeriod,
                               bool theSpecifyDayFlag,
                               const std::string& theVar,
                               std::string& thePhraseString,
                               bool theAlkaenPhrase,
                               part_of_the_day_id& thePartOfTheDay)
{
  Sentence sentence;

  bool specify_part_of_the_day =
      Settings::optional_bool(theVar + "::specify_part_of_the_day", true);

  if (!specify_part_of_the_day) return sentence;

  short weekday(theWeatherPeriod.localStartTime().GetWeekday());

  // if period is within one day and withing same part of the day, e.g. afternoon
  if (theWeatherPeriod.localStartTime().GetJulianDay() ==
          theWeatherPeriod.localEndTime().GetJulianDay() &&
      get_part_of_the_day_id(theWeatherPeriod.localStartTime()) ==
          get_part_of_the_day_id(theWeatherPeriod.localEndTime()))
  {
    thePhraseString = parse_time_phrase(
        weekday,
        theSpecifyDayFlag,
        get_narrow_time_phrase(theWeatherPeriod, theVar, thePartOfTheDay, theAlkaenPhrase));
    sentence << thePhraseString;
  }
  else
  {
    // first try the narrow part of the day (e.g. narrow: iltapäivä; large iltapäivä ja ilta)
    thePhraseString = parse_time_phrase(
        weekday,
        theSpecifyDayFlag,
        get_narrow_time_phrase(theWeatherPeriod, theVar, thePartOfTheDay, theAlkaenPhrase));
    sentence << thePhraseString;

    // if period does not fit into narrow part of the day
    if (sentence.size() == 0)
    {
      // try to adjust it to fit into narrow part of the day
      if (theWeatherPeriod.localEndTime().DifferenceInHours(theWeatherPeriod.localStartTime()) > 2)
      {
        // 1 hour tolerance
        TextGenPosixTime startTime(theWeatherPeriod.localStartTime());
        TextGenPosixTime endTime(theWeatherPeriod.localEndTime());
        startTime.ChangeByHours(1);
        endTime.ChangeByHours(-1);
        WeatherPeriod narrowerPeriod(startTime, endTime);

        if (endTime.DifferenceInHours(startTime) <= 4)
        {
          thePhraseString = parse_time_phrase(
              narrowerPeriod.localStartTime().GetWeekday(),
              theSpecifyDayFlag,
              get_narrow_time_phrase(narrowerPeriod, theVar, thePartOfTheDay, theAlkaenPhrase));
          sentence << thePhraseString;
        }

        if (sentence.size() == 0)
        {
          thePartOfTheDay =
              get_most_relevant_part_of_the_day_id_narrow(theWeatherPeriod, theAlkaenPhrase);

          thePhraseString = get_time_phrase_from_id(thePartOfTheDay, theVar, theAlkaenPhrase);

          if (!thePhraseString.empty())
          {
            std::stringstream sentence_ss;
            if (theSpecifyDayFlag) sentence_ss << weekday << "-";
            sentence_ss << thePhraseString;
            if (sentence_ss.str().size() > 2) sentence << sentence_ss.str();
          }
          else  // if (sentence.empty())
          {
            sentence << get_large_time_phrase(
                theWeatherPeriod, theSpecifyDayFlag, thePhraseString, thePartOfTheDay);

            if (sentence.empty())
            {
              sentence << get_large_time_phrase(
                  narrowerPeriod, theSpecifyDayFlag, thePhraseString, thePartOfTheDay);
            }
          }
        }
      }
      else
      {
        sentence << get_large_time_phrase(
            theWeatherPeriod, theSpecifyDayFlag, thePhraseString, thePartOfTheDay);
      }

      if (sentence.size() == 0 && theAlkaenPhrase)
      {
        thePhraseString = parse_time_phrase(
            weekday,
            theSpecifyDayFlag,
            get_time_phrase(theWeatherPeriod.localStartTime(), theVar, theAlkaenPhrase));
        sentence << thePhraseString;
      }
    }
  }

  return sentence;
}

std::string get_time_phrase(const TextGenPosixTime& theTimestamp,
                            const std::string& theVar,
                            bool theAlkaenPhrase /*= false*/)
{
  std::string retval("");

  bool specify_part_of_the_day =
      Settings::optional_bool(theVar + "::specify_part_of_the_day", true);

  if (!specify_part_of_the_day) return retval;

  if (is_inside(theTimestamp, AAMU))
  {
    if (is_inside(theTimestamp, AAMUPAIVA))
      retval = (theAlkaenPhrase ? AAMUPAIVASTA_ALKAEN_PHRASE : AAMUPAIVALLA_WORD);
    else
      retval = (theAlkaenPhrase ? AAMUSTA_ALKAEN_PHRASE : AAMULLA_WORD);
  }
  else if (is_inside(theTimestamp, AAMUPAIVA))
  {
    if (is_inside(theTimestamp, ILTAPAIVA))
      retval = (theAlkaenPhrase ? ILTAPAIVASTA_ALKAEN_PHRASE : ILTAPAIVALLA_WORD);
    else
      retval = (theAlkaenPhrase ? AAMUPAIVASTA_ALKAEN_PHRASE : AAMUPAIVALLA_WORD);
  }
  else if (is_inside(theTimestamp, ILTA))
  {
    /*
    if(is_inside(theTimestamp, ILTAPAIVA))
      retval = (theAlkaenPhrase ? ILTAPAIVASTA_ALKAEN_PHRASE : ILTAPAIVALLA_WORD);
    */
    if (is_inside(theTimestamp, ILTAYO))
      retval = (theAlkaenPhrase ? ILTAYOSTA_ALKAEN_PHRASE : ILTAYOLLA_WORD);
    else
      retval = (theAlkaenPhrase ? ILLASTA_ALKAEN_PHRASE : ILLALLA_WORD);
  }
  else if (is_inside(theTimestamp, ILTAPAIVA))
  {
    retval = (theAlkaenPhrase ? ILTAPAIVASTA_ALKAEN_PHRASE : ILTAPAIVALLA_WORD);
  }
  else if (is_inside(theTimestamp, ILTAYO))
  {
    retval = (theAlkaenPhrase ? ILTAYOSTA_ALKAEN_PHRASE : ILTAYOLLA_WORD);
  }
  else if (is_inside(theTimestamp, KESKIYO))
  {
    if (is_inside(theTimestamp, AAMUYO))
      retval = (theAlkaenPhrase ? AAMUYOSTA_ALKAEN_PHRASE : AAMUYOLLA_WORD);
    else
      retval = (theAlkaenPhrase ? KESKIYOSTA_ALKAEN_PHRASE : KESKIYOLLA_WORD);
  }
  else if (is_inside(theTimestamp, AAMUYO))
  {
    retval = (theAlkaenPhrase ? AAMUYOSTA_ALKAEN_PHRASE : AAMUYOLLA_WORD);
  }

  return retval;
}

Sentence get_direction_phrase(direction_id theDirectionId, bool theAlkaenPhrase /*= false*/)
{
  Sentence sentence;

  switch (theDirectionId)
  {
    case NORTH:
      sentence << (theAlkaenPhrase ? POHJOISESTA_ALKAEN_PHRASE : POHJOISESTA_WORD);
      break;
    case SOUTH:
      sentence << (theAlkaenPhrase ? ETELASTA_ALKAEN_PHRASE : ETELASTA_WORD);
      break;
    case EAST:
      sentence << (theAlkaenPhrase ? IDASTA_ALKAEN_PHRASE : IDASTA_WORD);
      break;
    case WEST:
      sentence << (theAlkaenPhrase ? LANNESTA_ALKAEN_PHRASE : LANNESTA_WORD);
      break;
    case NORTHEAST:
      sentence << (theAlkaenPhrase ? KOILLISESTA_ALKAEN_PHRASE : KOILLISESTA_WORD);
      break;
    case SOUTHEAST:
      sentence << (theAlkaenPhrase ? KAAKOSTA_ALKAEN_PHRASE : KAAKOSTA_WORD);
      break;
    case SOUTHWEST:
      sentence << (theAlkaenPhrase ? LOUNAASTA_ALKAEN_PHRASE : LOUNAASTA_WORD);
      break;
    case NORTHWEST:
      sentence << (theAlkaenPhrase ? LUOTEESTA_ALKAEN_PHRASE : LUOTEESTA_WORD);
      break;
    case NO_DIRECTION:
      break;
  }

  return sentence;
}

int get_period_length(const WeatherPeriod& thePeriod)
{
  return thePeriod.localEndTime().DifferenceInHours(thePeriod.localStartTime());
}

// returns the julian day of the first day period
int get_today_vector(const string& theVariable,
                     const WeatherArea& theArea,
                     const WeatherPeriod& thePeriod,
                     const TextGenPosixTime& theForecastTime,
                     vector<Sentence*>& theTodayVector)
{
  int retval = -1;

  NightAndDayPeriodGenerator generator(thePeriod, theVariable);

  for (unsigned int i = 0; i < generator.size(); i++)
  {
    WeatherPeriod wp(generator.period(i + 1));
    if (i == 0 ||
        generator.period(i).localStartTime().GetJulianDay() != wp.localStartTime().GetJulianDay())
    {
      Sentence* sentence = new Sentence();
      *sentence << PeriodPhraseFactory::create(
          "today", theVariable, theForecastTime, generator.period(i + 1), theArea);
      theTodayVector.push_back(sentence);

      if (i == 0) retval = wp.localStartTime().GetJulianDay();
    }
  }
  return retval;
}

Sentence get_today_phrase(const TextGenPosixTime& theEventTimestamp,
                          const string& theVariable,
                          const WeatherArea& theArea,
                          const WeatherPeriod thePeriod,
                          const TextGenPosixTime& theForecastTime)
{
  Sentence sentence;

  part_of_the_day_id partOfTheDayId = get_part_of_the_day_id(theEventTimestamp);

  if (partOfTheDayId == KESKIYO && theEventTimestamp.DifferenceInHours(theForecastTime) <= 24)
  {
    return sentence;
  }
  else
  {
    if (partOfTheDayId == KESKIYO || partOfTheDayId == AAMUYO)
    {
      const char* which_day = "";
      if (theEventTimestamp.GetJulianDay() == theForecastTime.GetJulianDay())
        which_day = "tonight";
      else
        which_day = "next_night";

      sentence << PeriodPhraseFactory::create(
          which_day, theVariable, theForecastTime, thePeriod, theArea);
    }
    else
    {
      const char* which_day = "";
      if (theEventTimestamp.GetJulianDay() == theForecastTime.GetJulianDay())
        which_day = "today";
      else
        which_day = "next_day";

      sentence << PeriodPhraseFactory::create(
          which_day, theVariable, theForecastTime, thePeriod, theArea);
    }
  }

  return sentence;
}

void get_precipitation_limit_value(const wf_story_params& theParameters,
                                   unsigned int thePrecipitationForm,
                                   precipitation_intesity_id theIntensityId,
                                   float& theLowerLimit,
                                   float& theUpperLimit)
{
  switch (thePrecipitationForm)
  {
    case WATER_FREEZING_FORM:
    case FREEZING_FORM:
    case WATER_FORM:
    case WATER_DRIZZLE_FREEZING_FORM:
    case WATER_DRIZZLE_FORM:
    case WATER_DRIZZLE_SLEET_FORM:
    case WATER_SLEET_FREEZING_FORM:
    case WATER_SLEET_FORM:
    case WATER_SNOW_FREEZING_FORM:
    case WATER_SNOW_FORM:
    case WATER_DRIZZLE_SNOW_FORM:
    case WATER_SLEET_SNOW_FORM:
    {
      switch (theIntensityId)
      {
        case DRY_WEATHER:
        {
          theLowerLimit = 0;
          theUpperLimit = theParameters.theDryWeatherLimitWater;
        }
        break;
        case WEAK_PRECIPITATION:
        {
          theLowerLimit = theParameters.theDryWeatherLimitWater;
          theUpperLimit = theParameters.theWeakPrecipitationLimitWater;
        }
        break;
        case MODERATE_PRECIPITATION:
        {
          theLowerLimit = theParameters.theWeakPrecipitationLimitWater;
          theUpperLimit = theParameters.theHeavyPrecipitationLimitWater;
        }
        break;
        case HEAVY_PRECIPITATION:
        {
          theLowerLimit = theParameters.theHeavyPrecipitationLimitWater;
          theUpperLimit = 1000.0;
        }
        break;
        default:
          break;
      }
    }
    break;
    case SLEET_FREEZING_FORM:
    case SLEET_FORM:
    case SLEET_SNOW_FREEZING_FORM:
    case SLEET_SNOW_FORM:
    {
      switch (theIntensityId)
      {
        case DRY_WEATHER:
        {
          theLowerLimit = 0;
          theUpperLimit = theParameters.theDryWeatherLimitSleet;
        }
        break;
        case WEAK_PRECIPITATION:
        {
          theLowerLimit = theParameters.theDryWeatherLimitSleet;
          theUpperLimit = theParameters.theWeakPrecipitationLimitSleet;
        }
        break;
        case MODERATE_PRECIPITATION:
        {
          theLowerLimit = theParameters.theWeakPrecipitationLimitSleet;
          theUpperLimit = theParameters.theHeavyPrecipitationLimitSleet;
        }
        break;
        case HEAVY_PRECIPITATION:
        {
          theLowerLimit = theParameters.theHeavyPrecipitationLimitSleet;
          theUpperLimit = 1000.0;
        }
        break;
        default:
          break;
      }
    }
    break;
    case SNOW_FORM:
    case SNOW_FREEZING_FORM:
    {
      switch (theIntensityId)
      {
        case DRY_WEATHER:
        {
          theLowerLimit = 0;
          theUpperLimit = theParameters.theDryWeatherLimitSnow;
        }
        break;
        case WEAK_PRECIPITATION:
        {
          theLowerLimit = theParameters.theDryWeatherLimitSnow;
          theUpperLimit = theParameters.theWeakPrecipitationLimitSnow;
        }
        break;
        case MODERATE_PRECIPITATION:
        {
          theLowerLimit = theParameters.theWeakPrecipitationLimitSnow;
          theUpperLimit = theParameters.theHeavyPrecipitationLimitSnow;
        }
        break;
        case HEAVY_PRECIPITATION:
        {
          theLowerLimit = theParameters.theHeavyPrecipitationLimitSnow;
          theUpperLimit = 1000.0;
        }
        break;
        default:
          break;
      }
    }
    break;
    case DRIZZLE_FORM:
    case DRIZZLE_FREEZING_FORM:
    case DRIZZLE_SLEET_FORM:
    case DRIZZLE_SLEET_FREEZING_FORM:
    case DRIZZLE_SNOW_FREEZING_FORM:
    case DRIZZLE_SNOW_FORM:
    case DRIZZLE_SLEET_SNOW_FORM:
    {
      switch (theIntensityId)
      {
        case DRY_WEATHER:
        {
          theLowerLimit = 0;
          theUpperLimit = theParameters.theDryWeatherLimitDrizzle;
        }
        break;
        case WEAK_PRECIPITATION:
        {
          theLowerLimit = theParameters.theDryWeatherLimitWater;
          theUpperLimit = theParameters.theWeakPrecipitationLimitWater;
        }
        break;
        case MODERATE_PRECIPITATION:
        {
          theLowerLimit = theParameters.theWeakPrecipitationLimitWater;
          theUpperLimit = theParameters.theHeavyPrecipitationLimitWater;
        }
        break;
        case HEAVY_PRECIPITATION:
        {
          theLowerLimit = theParameters.theHeavyPrecipitationLimitWater;
          theUpperLimit = 1000.0;
        }
        break;
        default:
          break;
      }
    }
    break;
    case MISSING_PRECIPITATION_FORM:
      break;
  }
}

void get_dry_and_weak_precipitation_limit(const wf_story_params& theParameters,
                                          unsigned int thePrecipitationForm,
                                          float& theDryWeatherLimit,
                                          float& theWeakPrecipitationLimit)
{
  switch (thePrecipitationForm)
  {
    case WATER_FREEZING_FORM:
    case FREEZING_FORM:
    case WATER_FORM:
    {
      theDryWeatherLimit = theParameters.theDryWeatherLimitWater;
      theWeakPrecipitationLimit = theParameters.theWeakPrecipitationLimitWater;
    }
    break;
    case SLEET_FREEZING_FORM:
    case SLEET_FORM:
    {
      theDryWeatherLimit = theParameters.theDryWeatherLimitSleet;
      theWeakPrecipitationLimit = theParameters.theWeakPrecipitationLimitSleet;
    }
    break;
    case SNOW_FORM:
    case SNOW_FREEZING_FORM:
    {
      theDryWeatherLimit = theParameters.theDryWeatherLimitSnow;
      theWeakPrecipitationLimit = theParameters.theWeakPrecipitationLimitSnow;
    }
    break;
    case DRIZZLE_FORM:
    case DRIZZLE_FREEZING_FORM:
    case WATER_DRIZZLE_FREEZING_FORM:
    case WATER_DRIZZLE_FORM:
    {
      theDryWeatherLimit = theParameters.theDryWeatherLimitDrizzle;
      theWeakPrecipitationLimit = theParameters.theWeakPrecipitationLimitWater;
    }
    break;
    case DRIZZLE_SLEET_FORM:
    case DRIZZLE_SLEET_FREEZING_FORM:
    case WATER_DRIZZLE_SLEET_FORM:
    case WATER_SLEET_FREEZING_FORM:
    case WATER_SLEET_FORM:
    {
      theDryWeatherLimit = theParameters.theDryWeatherLimitSleet;
      theWeakPrecipitationLimit = theParameters.theWeakPrecipitationLimitSleet;
    }
    break;
    case WATER_SNOW_FREEZING_FORM:
    case WATER_SNOW_FORM:
    case DRIZZLE_SNOW_FREEZING_FORM:
    case DRIZZLE_SNOW_FORM:
    case WATER_DRIZZLE_SNOW_FORM:
    case WATER_SLEET_SNOW_FORM:
    case DRIZZLE_SLEET_SNOW_FORM:
    case SLEET_SNOW_FREEZING_FORM:
    case SLEET_SNOW_FORM:
    {
      theDryWeatherLimit = theParameters.theDryWeatherLimitSnow;
      theWeakPrecipitationLimit = theParameters.theWeakPrecipitationLimitSnow;
    }
    break;
    case MISSING_PRECIPITATION_FORM:
      break;
  }
}

precipitation_form_id get_complete_precipitation_form(const string& theVariable,
                                                      float thePrecipitationFormWater,
                                                      float thePrecipitationFormDrizzle,
                                                      float thePrecipitationFormSleet,
                                                      float thePrecipitationFormSnow,
                                                      float thePrecipitationFormFreezing)
{
  unsigned int precipitation_form = 0;

  typedef std::pair<float, precipitation_form_id> precipitation_form_type;
  precipitation_form_type water(
      thePrecipitationFormWater,
      (thePrecipitationFormWater != kFloatMissing ? WATER_FORM : MISSING_PRECIPITATION_FORM));
  precipitation_form_type drizzle(
      thePrecipitationFormDrizzle,
      (thePrecipitationFormDrizzle != kFloatMissing ? DRIZZLE_FORM : MISSING_PRECIPITATION_FORM));
  precipitation_form_type sleet(
      thePrecipitationFormSleet,
      (thePrecipitationFormSleet != kFloatMissing ? SLEET_FORM : MISSING_PRECIPITATION_FORM));
  precipitation_form_type snow(
      thePrecipitationFormSnow,
      (thePrecipitationFormSnow != kFloatMissing ? SNOW_FORM : MISSING_PRECIPITATION_FORM));
  precipitation_form_type freezing(
      thePrecipitationFormFreezing,
      (thePrecipitationFormFreezing != kFloatMissing ? FREEZING_FORM : MISSING_PRECIPITATION_FORM));

  vector<precipitation_form_type> precipitation_forms;
  precipitation_forms.push_back(water);
  precipitation_forms.push_back(drizzle);
  precipitation_forms.push_back(sleet);
  precipitation_forms.push_back(snow);
  precipitation_forms.push_back(freezing);

  sort(precipitation_forms.begin(), precipitation_forms.end());

  precipitation_form_id primaryPrecipitationForm =
      (precipitation_forms[4].first != kFloatMissing &&
       precipitation_forms[4].first > PRECIPITATION_FORM_REPORTING_LIMIT)
          ? precipitation_forms[4].second
          : MISSING_PRECIPITATION_FORM;
  precipitation_form_id secondaryPrecipitationForm =
      (precipitation_forms[3].first != kFloatMissing &&
       precipitation_forms[3].first > PRECIPITATION_FORM_REPORTING_LIMIT)
          ? precipitation_forms[3].second
          : MISSING_PRECIPITATION_FORM;
  precipitation_form_id tertiaryPrecipitationForm =
      (precipitation_forms[2].first != kFloatMissing &&
       precipitation_forms[2].first > PRECIPITATION_FORM_REPORTING_LIMIT)
          ? precipitation_forms[2].second
          : MISSING_PRECIPITATION_FORM;

  // TODO: merge drizzle and water in some cases, when sleet is involved

  precipitation_form |= primaryPrecipitationForm;
  precipitation_form |= secondaryPrecipitationForm;
  precipitation_form |= tertiaryPrecipitationForm;

  return static_cast<precipitation_form_id>(precipitation_form);
}

void get_sub_time_series(const WeatherPeriod& thePeriod,
                         const weather_result_data_item_vector& theSourceVector,
                         weather_result_data_item_vector& theDestinationVector)
{
  for (unsigned int i = 0; i < theSourceVector.size(); i++)
  {
    WeatherResultDataItem* item = theSourceVector[i];
    if (item->thePeriod.localStartTime() >= thePeriod.localStartTime() &&
        item->thePeriod.localEndTime() <= thePeriod.localEndTime())
      theDestinationVector.push_back(item);
  }
}

void get_sub_time_series(const part_of_the_day_id& thePartOfTheDay,
                         const weather_result_data_item_vector& theSourceVector,
                         weather_result_data_item_vector& theDestinationVector)
{
  for (unsigned int i = 0; i < theSourceVector.size(); i++)
  {
    WeatherResultDataItem* item = theSourceVector[i];
    if (item->thePartOfTheDay == thePartOfTheDay) theDestinationVector.push_back(item);
  }
}

float get_mean(const weather_result_data_item_vector& theTimeSeries,
               int theStartIndex /*= 0*/,
               int theEndIndex /*= 0*/)
{
  float precipitation_sum = 0.0;
  int counter = 0;
  unsigned int startIndex = theStartIndex > 0 ? theStartIndex : 0;
  unsigned int endIndex = theEndIndex > 0 ? theEndIndex : theTimeSeries.size();

  if (startIndex == endIndex || endIndex < startIndex || endIndex > theTimeSeries.size())
    return kFloatMissing;

  for (unsigned int i = startIndex; i < endIndex; i++)
  {
    if (theTimeSeries[i]->theResult.value() == kFloatMissing) continue;
    precipitation_sum += theTimeSeries[i]->theResult.value();
    counter++;
  }

  if ((counter == 0 && theTimeSeries.size() > 0) || theTimeSeries.size() == 0)
    return kFloatMissing;
  else
    return precipitation_sum / counter;
}

float get_standard_deviation(const weather_result_data_item_vector& theTimeSeries)
{
  float deviation_sum_pow2 = 0.0;
  float mean = get_mean(theTimeSeries);
  int counter = 0;
  for (unsigned int i = 0; i < theTimeSeries.size(); i++)
  {
    if (theTimeSeries[i]->theResult.value() == kFloatMissing) continue;
    deviation_sum_pow2 += std::pow(mean - theTimeSeries[i]->theResult.value(), 2);
    counter++;
  }

  return std::sqrt(deviation_sum_pow2 / counter);
}

void get_min_max(const weather_result_data_item_vector& theTimeSeries, float& theMin, float& theMax)
{
  theMin = theMax = kFloatMissing;

  for (unsigned int i = 0; i < theTimeSeries.size(); i++)
  {
    if (theTimeSeries[i]->theResult.value() == kFloatMissing) continue;
    if (i == 0)
    {
      theMin = theTimeSeries[i]->theResult.value();
      theMax = theTimeSeries[i]->theResult.value();
    }
    else
    {
      if (theMin > theTimeSeries[i]->theResult.value())
        theMin = theTimeSeries[i]->theResult.value();
      else if (theMax < theTimeSeries[i]->theResult.value())
        theMax = theTimeSeries[i]->theResult.value();
    }
  }
}

double get_pearson_coefficient(const weather_result_data_item_vector& theTimeSeries,
                               unsigned int theStartIndex,
                               unsigned int theEndIndex,
                               bool theUseErrorValueFlag /*= false*/)
{
  vector<double> precipitation;

  for (unsigned int i = theStartIndex; i <= theEndIndex; i++)
  {
    if (theUseErrorValueFlag)
      precipitation.push_back(theTimeSeries[i]->theResult.error());
    else
      precipitation.push_back(theTimeSeries[i]->theResult.value());
  }

  return MathTools::pearson_coefficient(precipitation);
}

Sentence area_specific_sentence(float north,
                                float south,
                                float east,
                                float west,
                                float northEast,
                                float southEast,
                                float southWest,
                                float northWest,
                                bool mostlyFlag /*= true*/)
{
  Sentence sentence;

  area_specific_sentence_id sentenceId = get_area_specific_sentence_id(
      north, south, east, west, northEast, southEast, southWest, northWest, mostlyFlag);

  switch (sentenceId)
  {
    case ALUEEN_POHJOISOSASSA:
    {
      sentence << ALUEEN_POHJOISOSASSA_PHRASE;
    }
    break;
    case ALUEEN_ETELAOSASSA:
    {
      sentence << ALUEEN_ETELAOSASSA_PHRASE;
    }
    break;
    case ALUEEN_ITAOSASSA:
    {
      sentence << ALUEEN_ITAOSASSA_PHRASE;
    }
    break;
    case ALUEEN_LANSIOSASSA:
    {
      sentence << ALUEEN_LANSIOSASSA_PHRASE;
    }
    break;
    case ENIMMAKSEEN_ALUEEN_POHJOISOSASSA:
    {
      sentence << ENIMMAKSEEN_WORD << ALUEEN_POHJOISOSASSA_PHRASE;
    }
    break;
    case ENIMMAKSEEN_ALUEEN_ETELAOSASSA:
    {
      sentence << ENIMMAKSEEN_WORD << ALUEEN_ETELAOSASSA_PHRASE;
    }
    break;
    case ENIMMAKSEEN_ALUEEN_ITAOSASSA:
    {
      sentence << ENIMMAKSEEN_WORD << ALUEEN_ITAOSASSA_PHRASE;
    }
    break;
    case ENIMMAKSEEN_ALUEEN_LANSIOSASSA:
    {
      sentence << ENIMMAKSEEN_WORD << ALUEEN_LANSIOSASSA_PHRASE;
    }
    break;
    default:
      break;
  }

  return sentence;
}

area_specific_sentence_id get_area_specific_sentence_id(float north,
                                                        float south,
                                                        float east,
                                                        float west,
                                                        float northEast,
                                                        float southEast,
                                                        float southWest,
                                                        float northWest,
                                                        bool mostlyFlag /*= true*/)
{
  area_specific_sentence_id retval(MISSING_AREA_SPECIFIC_SENTENCE_ID);

  if (north >= 98.0)
  {
    retval = ALUEEN_POHJOISOSASSA;
  }
  else if (north >= 95.0 && mostlyFlag)
  {
    retval = ENIMMAKSEEN_ALUEEN_POHJOISOSASSA;
  }
  else if (south >= 98.0)
  {
    retval = ALUEEN_ETELAOSASSA;
  }
  else if (south >= 95.0 && mostlyFlag)
  {
    retval = ENIMMAKSEEN_ALUEEN_ETELAOSASSA;
  }
  else if (east >= 98.0)
  {
    retval = ALUEEN_ITAOSASSA;
  }
  else if (east >= 95.0 && mostlyFlag)
  {
    retval = ENIMMAKSEEN_ALUEEN_ITAOSASSA;
  }
  else if (west >= 98.0)
  {
    retval = ALUEEN_LANSIOSASSA;
  }
  else if (west >= 95.0 && mostlyFlag)
  {
    retval = ENIMMAKSEEN_ALUEEN_LANSIOSASSA;
  }
  return retval;
}

float get_area_percentage(const std::string& theVar,
                          const WeatherArea& theArea,
                          const TextGen::WeatherArea::Type& theType,
                          const AnalysisSources& theSources,
                          const WeatherPeriod& thePeriod)
{
  if (Settings::isset(theVar)) return Settings::require_double(theVar);

  WeatherArea maskArea = theArea;
  maskArea.type(theType);
  WeatherArea comparisonArea = theArea;

  NFmiIndexMask indexMask;
  NFmiIndexMask comparisonIndexMask;
  RangeAcceptor precipitationlimits;
  precipitationlimits.lowerLimit(0);
  precipitationlimits.upperLimit(1000);
  ExtractMask(theSources, Precipitation, maskArea, thePeriod, precipitationlimits, indexMask);
  ExtractMask(theSources,
              Precipitation,
              comparisonArea,
              thePeriod,
              precipitationlimits,
              comparisonIndexMask);

  if (comparisonIndexMask.size() == 0) return 0.0;

  // ARE 2.9.2011: take the intersection before comparing, because the comparison area can be
  // part (northern, southern, eastern, western) part of the full area
  indexMask &= comparisonIndexMask;

  return (static_cast<float>(indexMask.size()) / static_cast<float>(comparisonIndexMask.size())) *
         100.0;
}

std::string parse_weekday_phrase(short weekday, const std::string& part_of_the_day)
{
  std::ostringstream oss;

  if (weekday > 0 && part_of_the_day.compare(EMPTY_STRING) != 0 && !part_of_the_day.empty())
    oss << weekday << "-" << part_of_the_day;
  else
    oss << (part_of_the_day.size() > 0 ? part_of_the_day : EMPTY_STRING);

  return oss.str();
}

Sentence parse_weekday_phrase(short weekday, const Sentence& part_of_the_day)
{
  Sentence sentence;
  Sentence partOfTheDay(part_of_the_day);
  std::ostringstream oss;
  oss << weekday;

  if (weekday > 0 && !part_of_the_day.size() > 0)
    sentence << oss.str() << "-" << partOfTheDay;
  else
  {
    if (part_of_the_day.size() == 0) partOfTheDay << EMPTY_STRING;
    sentence << partOfTheDay;
  }

  return sentence;
}

WeatherPeriod get_intersection_period(const WeatherPeriod& thePeriod1,
                                      const WeatherPeriod& thePeriod2,
                                      bool& theIntersectionPeriodFound)
{
  int start_year(0), start_month(0), start_day(0), start_hour(0);
  int end_year(0), end_month(0), end_day(0), end_hour(0);
  theIntersectionPeriodFound = false;

  if (is_inside(thePeriod1.localStartTime(), thePeriod2) &&
      !is_inside(thePeriod1.localEndTime(), thePeriod2))
  {
    start_year = thePeriod1.localStartTime().GetYear();
    start_month = thePeriod1.localStartTime().GetMonth();
    start_day = thePeriod1.localStartTime().GetDay();
    start_hour = thePeriod1.localStartTime().GetHour();
    end_year = thePeriod2.localEndTime().GetYear();
    end_month = thePeriod2.localEndTime().GetMonth();
    end_day = thePeriod2.localEndTime().GetDay();
    end_hour = thePeriod2.localEndTime().GetHour();
    theIntersectionPeriodFound = true;
  }
  else if (is_inside(thePeriod1.localEndTime(), thePeriod2) &&
           !is_inside(thePeriod1.localStartTime(), thePeriod2))
  {
    start_year = thePeriod2.localStartTime().GetYear();
    start_month = thePeriod2.localStartTime().GetMonth();
    start_day = thePeriod2.localStartTime().GetDay();
    start_hour = thePeriod2.localStartTime().GetHour();
    end_year = thePeriod1.localEndTime().GetYear();
    end_month = thePeriod1.localEndTime().GetMonth();
    end_day = thePeriod1.localEndTime().GetDay();
    end_hour = thePeriod1.localEndTime().GetHour();
    theIntersectionPeriodFound = true;
  }
  else if (is_inside(thePeriod1.localEndTime(), thePeriod2) &&
           is_inside(thePeriod1.localStartTime(), thePeriod2))
  {
    start_year = thePeriod1.localStartTime().GetYear();
    start_month = thePeriod1.localStartTime().GetMonth();
    start_day = thePeriod1.localStartTime().GetDay();
    start_hour = thePeriod1.localStartTime().GetHour();
    end_year = thePeriod1.localEndTime().GetYear();
    end_month = thePeriod1.localEndTime().GetMonth();
    end_day = thePeriod1.localEndTime().GetDay();
    end_hour = thePeriod1.localEndTime().GetHour();
    theIntersectionPeriodFound = true;
  }
  else if (thePeriod1.localStartTime() <= thePeriod2.localStartTime() &&
           thePeriod1.localEndTime() >= thePeriod2.localEndTime())
  {
    start_year = thePeriod2.localStartTime().GetYear();
    start_month = thePeriod2.localStartTime().GetMonth();
    start_day = thePeriod2.localStartTime().GetDay();
    start_hour = thePeriod2.localStartTime().GetHour();
    end_year = thePeriod2.localEndTime().GetYear();
    end_month = thePeriod2.localEndTime().GetMonth();
    end_day = thePeriod2.localEndTime().GetDay();
    end_hour = thePeriod2.localEndTime().GetHour();
    theIntersectionPeriodFound = true;
  }

  if (theIntersectionPeriodFound)
  {
    TextGenPosixTime startTime(start_year, start_month, start_day, start_hour);
    TextGenPosixTime endTime(end_year, end_month, end_day, end_hour);
    WeatherPeriod resultPeriod(startTime, endTime);

    return resultPeriod;
  }

  return thePeriod1;
}

split_method split_the_area(const std::string& theVar,
                            const TextGen::WeatherArea& theArea,
                            const TextGen::WeatherPeriod& thePeriod,
                            const TextGen::AnalysisSources& theSources,
                            double& theDivisionLine,
                            MessageLogger& theLog)
{
  split_method retval(NO_SPLITTING);

  if (Settings::isset(theVar + "::areas_to_split"))
  {
    std::string areasToSplit(require_string(theVar + "::areas_to_split"));
    vector<string> areas = NFmiStringTools::Split(areasToSplit, ",");
    for (unsigned int i = 0; i < areas.size(); i++)
    {
      if (theArea.name().compare(areas[i]) == 0)
      {
        std::string split_method(optional_string(
            "textgen::split_the_area::" + theArea.name() + "::method", EMPTY_STRING));
        boost::trim(split_method);

        if (split_method.compare(0, strlen(HORIZONTAL_SPLIT_KEY), HORIZONTAL_SPLIT_KEY) == 0)
          retval = HORIZONTAL;
        else if (split_method.compare(0, strlen(VERTICAL_SPLIT_KEY), VERTICAL_SPLIT_KEY) == 0)
          retval = VERTICAL;

        if (retval != NO_SPLITTING)
        {
          Rect areaRect(theArea);
          NFmiPoint centerPoint = areaRect.getCenter();
          size_t valuePos = split_method.find(COLON_PUNCTUATION_MARK);
          std::string valueStr(valuePos != string::npos ? split_method.substr(valuePos + 1) : "");
          boost::trim(valueStr);
          if (!valueStr.empty())
          {
            double divisionLine = atof(valueStr.c_str());

            // the division line must be inside area
            if ((retval == HORIZONTAL &&
                 areaRect.contains(NFmiPoint(centerPoint.X(), divisionLine))) ||
                (retval == VERTICAL && areaRect.contains(NFmiPoint(divisionLine, centerPoint.Y()))))
              theDivisionLine = divisionLine;
            else
            {
              theLog << "The area " << theArea.name() << " is not split: the given "
                     << (retval == HORIZONTAL ? "latitude " : "longitude ") << divisionLine
                     << " is out of the area!" << endl;
              retval = NO_SPLITTING;
            }
          }
          else
          {
            // no longitude or latitude given -> split in the middle
            theDivisionLine = (retval == HORIZONTAL ? centerPoint.Y() : centerPoint.X());
          }
        }
        break;
      }
    }
  }

  return retval;
}

bool test_temperature_split_criterion(const std::string& theVar,
                                      bool morningTemperature,
                                      const TextGen::WeatherArea& theAreaOne,
                                      const TextGen::WeatherArea& theAreaTwo,
                                      const TextGen::WeatherPeriod& thePeriod,
                                      const TextGen::AnalysisSources& theSources,
                                      MessageLogger& theLog)
{
  bool retval = false;

  WeatherResult minAreaOne(kFloatMissing, 0.0);
  WeatherResult maxAreaOne(kFloatMissing, 0.0);
  WeatherResult meanAreaOne(kFloatMissing, 0.0);
  WeatherResult minAreaTwo(kFloatMissing, 0.0);
  WeatherResult maxAreaTwo(kFloatMissing, 0.0);
  WeatherResult meanAreaTwo(kFloatMissing, 0.0);

  // both have same name
  std::string nimi(theAreaOne.name());

  std::string split_section_name("textgen::split_the_area::" + nimi);

  if (!Settings::isset(split_section_name + "::method")) return false;

  const std::string criterion =
      optional_string(split_section_name + "::criterion", "temperature_difference");
  float difference = 5.0;
  size_t index = criterion.find(":");
  if (index != string::npos) difference = atof(criterion.substr(index + 1).c_str());

  if (morningTemperature)
  {
    morning_temperature(
        theVar, theSources, theAreaOne, thePeriod, minAreaOne, maxAreaOne, meanAreaOne);

    morning_temperature(
        theVar, theSources, theAreaTwo, thePeriod, minAreaTwo, maxAreaTwo, meanAreaTwo);
  }
  else
  {
    afternoon_temperature(
        theVar, theSources, theAreaOne, thePeriod, minAreaOne, maxAreaOne, meanAreaOne);

    afternoon_temperature(
        theVar, theSources, theAreaTwo, thePeriod, minAreaTwo, maxAreaTwo, meanAreaTwo);
  }

  int theMinimumIntAreaOne = static_cast<int>(round(minAreaOne.value()));
  int theMeanIntAreaOne = static_cast<int>(round(meanAreaOne.value()));
  int theMaximumIntAreaOne = static_cast<int>(round(maxAreaOne.value()));
  int theMinimumIntAreaTwo = static_cast<int>(round(minAreaTwo.value()));
  int theMeanIntAreaTwo = static_cast<int>(round(meanAreaTwo.value()));
  int theMaximumIntAreaTwo = static_cast<int>(round(maxAreaTwo.value()));
  const int mininterval = optional_int(theVar + "::mininterval", 2);
  const bool interval_zero = optional_bool(theVar + "::always_interval_zero", false);
  int intervalStartAreaOne;
  int intervalEndAreaOne;
  int intervalStartAreaTwo;
  int intervalEndAreaTwo;

  clamp_temperature(
      theVar,
      SeasonTools::isWinterHalf(thePeriod.localStartTime(), theVar),
      true,
      theMinimumIntAreaOne < theMaximumIntAreaOne ? theMinimumIntAreaOne : theMaximumIntAreaOne,
      theMaximumIntAreaOne > theMinimumIntAreaOne ? theMaximumIntAreaOne : theMinimumIntAreaOne);
  clamp_temperature(
      theVar,
      SeasonTools::isWinterHalf(thePeriod.localStartTime(), theVar),
      true,
      theMinimumIntAreaTwo < theMaximumIntAreaTwo ? theMinimumIntAreaTwo : theMaximumIntAreaTwo,
      theMaximumIntAreaTwo > theMinimumIntAreaTwo ? theMaximumIntAreaTwo : theMinimumIntAreaTwo);

  sort_out_temperature_interval(theMinimumIntAreaOne,
                                theMeanIntAreaOne,
                                theMaximumIntAreaOne,
                                mininterval,
                                interval_zero,
                                intervalStartAreaOne,
                                intervalEndAreaOne,
                                true);
  sort_out_temperature_interval(theMinimumIntAreaTwo,
                                theMeanIntAreaTwo,
                                theMaximumIntAreaTwo,
                                mininterval,
                                interval_zero,
                                intervalStartAreaTwo,
                                intervalEndAreaTwo,
                                true);

  bool differentInterval =
      (intervalStartAreaOne != intervalStartAreaTwo || intervalEndAreaOne != intervalEndAreaTwo);
  bool differentMeanTemperature = (abs(meanAreaOne.value() - meanAreaTwo.value()) >= difference);

  retval = (differentInterval && differentMeanTemperature);

  if (morningTemperature)
    theLog << "Morning ";
  else
    theLog << "Afternoon ";
  theLog << "mean temperature in southern part: " << meanAreaOne.value() << endl;

  if (morningTemperature)
    theLog << "Morning ";
  else
    theLog << "Afternoon ";
  theLog << "mean temperature in northern part: " << meanAreaTwo.value() << endl;

  theLog << "Mean temperature difference: " << abs(meanAreaOne.value() - meanAreaTwo.value())
         << endl;

  return retval;
}

split_method check_area_splitting(const std::string& theVar,
                                  const TextGen::WeatherArea& theArea,
                                  const TextGen::WeatherPeriod& thePeriod,
                                  const TextGen::AnalysisSources& theSources,
                                  TextGen::WeatherArea& theFirstArea,
                                  TextGen::WeatherArea& theSecondArea,
                                  MessageLogger& theLog)
{
  bool splitCriterionFulfilled = false;

  double divisionLine = 0.0;
  split_method splitMethod =
      split_the_area(theVar, theArea, thePeriod, theSources, divisionLine, theLog);
  if (HORIZONTAL == splitMethod)
  {
    theFirstArea.type(WeatherArea::Southern);
    theSecondArea.type(WeatherArea::Northern);
    theFirstArea.setLatitudeDivisionLine(divisionLine);
    theSecondArea.setLatitudeDivisionLine(divisionLine);
  }
  else if (VERTICAL == splitMethod)
  {
    theFirstArea.type(WeatherArea::Western);
    theSecondArea.type(WeatherArea::Eastern);
    theFirstArea.setLongitudeDivisionLine(divisionLine);
    theSecondArea.setLongitudeDivisionLine(divisionLine);
  }

  if (NO_SPLITTING != splitMethod)
  {
    // TODO: this is not good way of deciding if it is morning/afternoon
    splitCriterionFulfilled =
        test_temperature_split_criterion(theVar,
                                         theVar.find("morning") != string::npos,
                                         theFirstArea,
                                         theSecondArea,
                                         thePeriod,
                                         theSources,
                                         theLog);

    if (splitCriterionFulfilled)
    {
      if (HORIZONTAL == splitMethod)
      {
        theLog << "The area " << theArea.name() << " is split horizontally at " << abs(divisionLine)
               << (divisionLine < 0.0 ? " S" : " N") << endl;
      }
      else if (VERTICAL == splitMethod)
        theLog << "The area " << theArea.name() << " is split vertically at " << abs(divisionLine)
               << (divisionLine < 0.0 ? " E" : " W") << endl;
    }
    else
    {
      splitMethod = NO_SPLITTING;

      theLog << "The area " << theArea.name()
             << " is not split: temperature criterion not fulfilled!" << endl;
    }
  }

  return splitMethod;
}

}  // namespace TextGen
