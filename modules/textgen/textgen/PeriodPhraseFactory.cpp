// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::PeriodPhraseFactory
 */
// ======================================================================
/*!
 * \namespace TextGen::PeriodPhraseFactory
 *
 * \brief Tools for generating phrases to describe periods
 *
 * \see page_periodifraasit
 *
 */
// ======================================================================
#include <boost/lexical_cast.hpp>

#include "PeriodPhraseFactory.h"
#include <calculator/HourPeriodGenerator.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherHistory.h>
#include <calculator/WeatherPeriod.h>
#include "WeekdayTools.h"

#include <newbase/NFmiStringTools.h>
#include <calculator/TextGenPosixTime.h>

using TextGen::WeatherPeriod;
using TextGen::WeatherArea;
using namespace TextGen::TimeTools;
using namespace boost;
using namespace std;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Reorganizes preferred phrases
 *
 * The input strings contain preferred word orders such as
 * \code
 * today,tomorrow,weekday
 * \endcode
 * This function makes sure the output contains exactly those
 * words given in the second input parameter. However, the
 * first parameter may specify a different preferred ordering
 * for the variables.
 *
 * For example, given two parameters
 * \code
 * foo,atnight,bar,weekday
 * tonight,atnight,followingnight,weekday
 * \endcode
 * the output would be
 * \code
 * atnight,weekday,tonight,followingnight
 * \endcode
 *
 * \param thePreference The preferred order
 * \param theDefault The default order
 * \return List of words
 */
// ----------------------------------------------------------------------

vector<string> reorder_preferences(const string& thePreference, const string& theDefault)
{
  const vector<string> preferences = NFmiStringTools::Split(thePreference);
  const vector<string> defaults = NFmiStringTools::Split(theDefault);

  // fast special case
  if (preferences.empty()) return defaults;

  typedef vector<string>::const_iterator const_iterator;

  vector<string> output;

  // first the allowed ones from preferences in correct order
  for (const_iterator it = preferences.begin(); it != preferences.end(); ++it)
  {
    if (find(defaults.begin(), defaults.end(), *it) != defaults.end())
      if (find(output.begin(), output.end(), *it) == output.end()) output.push_back(*it);
  }
  // then the remaining defaults
  for (const_iterator jt = defaults.begin(); jt != defaults.end(); ++jt)
  {
    if (find(output.begin(), output.end(), *jt) == output.end()) output.push_back(*jt);
  }
  return output;
}
}

namespace TextGen
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "until_tonight"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence until_tonight(const string& theVariable,
                       const TextGenPosixTime& theForecastTime,
                       const WeatherPeriod& thePeriod,
                       TextGen::WeatherHistory* theHistory = 0)
{
  using WeekdayTools::on_weekday;
  Sentence sentence;

  const string var = theVariable + "::until_tonight::phrases";
  const string preferences = Settings::optional_string(var, "");
  const string defaults("none,today,atday,weekday,none!");
  vector<string> order = reorder_preferences(preferences, defaults);

  for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
  {
    if (*it == "none")
    {
      if (isSameDay(theForecastTime, thePeriod.localStartTime())) return sentence;
    }
    else if (*it == "atday")
    {
      if (isSameDay(theForecastTime, thePeriod.localStartTime())) return (sentence << "paivalla");
    }
    else if (*it == "today")
    {
      if (isSameDay(theForecastTime, thePeriod.localStartTime())) return (sentence << "tanaan");
    }
    else if (*it == "weekday")
    {
      if (theHistory)
      {
        return (sentence << on_weekday(thePeriod.localStartTime(), *theHistory));
      }
      else
      {
        return (sentence << on_weekday(thePeriod.localStartTime()));
      }
    }
    else if (*it == "none!")
      return sentence;
    else
      throw TextGenError("PeriodPhraseFactory::until_tonight does not accept phrase " + *it);
  }

  throw TextGenError("PeriodPhrasefactory::until_tonight run out of options");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "until_morning"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence until_morning(const string& theVariable,
                       const TextGenPosixTime& theForecastTime,
                       const WeatherPeriod& thePeriod,
                       TextGen::WeatherHistory* theHistory = 0)
{
  using WeekdayTools::night_against_weekday;
  Sentence sentence;

  const string var = theVariable + "::until_morning::phrases";
  const string preferences = Settings::optional_string(var, "");
  const string defaults("none,tonight,atnight,weekday,none!");
  vector<string> order = reorder_preferences(preferences, defaults);

  for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
  {
    if (*it == "none")
    {
      if (isNextDay(theForecastTime, thePeriod.localEndTime())) return sentence;
    }
    else if (*it == "atnight")
    {
      if (isNextDay(theForecastTime, thePeriod.localEndTime())) return (sentence << "yolla");
    }
    else if (*it == "tonight")
    {
      if (isNextDay(theForecastTime, thePeriod.localEndTime())) return (sentence << "ensi yona");
    }
    else if (*it == "weekday")
    {
      if (theHistory)
      {
        return (sentence << night_against_weekday(thePeriod.localEndTime(), *theHistory));
      }
      else
      {
        return (sentence << night_against_weekday(thePeriod.localEndTime()));
      }
    }
    else if (*it == "none!")
      return sentence;
    else
      throw TextGenError("PeriodPhrasefactory::until_morning does not accept phrase " + *it);
  }

  throw TextGenError("PeriodPhrasefactory::until_morning run out of options");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "today"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence today(const string& theVariable,
               const TextGenPosixTime& theForecastTime,
               const WeatherPeriod& thePeriod,
               TextGen::WeatherHistory* theHistory = 0)
{
  using WeekdayTools::on_weekday;
  Sentence sentence;

  const string var = theVariable + "::today::phrases";
  const string preferences = Settings::optional_string(var, "");
  const string defaults("none,today,tomorrow,atday,weekday,none!");
  vector<string> order = reorder_preferences(preferences, defaults);

  for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
  {
    if (*it == "weekday")
    {
      if (theHistory)
      {
        return (sentence << on_weekday(thePeriod.localStartTime(), *theHistory));
      }
      else
      {
        return (sentence << on_weekday(thePeriod.localStartTime()));
      }
    }
    else if (*it == "atday")
    {
      if (isSameDay(theForecastTime, thePeriod.localStartTime())) return (sentence << "paivalla");
    }
    else if (*it == "none")
    {
      if (isSameDay(theForecastTime, thePeriod.localStartTime())) return sentence;
    }
    else if (*it == "today")
    {
      if (isSameDay(theForecastTime, thePeriod.localStartTime())) return (sentence << "tanaan");
    }
    else if (*it == "tomorrow")
    {
      if (isNextDay(theForecastTime, thePeriod.localStartTime()))
      {
        if (theHistory)
        {
          return (sentence << WeekdayTools::get_time_phrase(
                      thePeriod.localStartTime(), "huomenna", *theHistory));
        }
        else
        {
          return (sentence << "huomenna");
        }
      }
    }
    else if (*it == "none!")
      return sentence;
    else
      throw TextGenError("PeriodPhrasefactory::today does not accept phrase " + *it);
  }

  throw TextGenError("PeriodPhrasefactory::today run out of options");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "tonight"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence tonight(const string& theVariable,
                 const TextGenPosixTime& theForecastTime,
                 const WeatherPeriod& thePeriod,
                 TextGen::WeatherHistory* theHistory = 0)
{
  using WeekdayTools::night_against_weekday;
  Sentence sentence;

  const string var = theVariable + "::tonight::phrases";
  const string preferences = Settings::optional_string(var, "");
  const string defaults("none,tonight,atnight,weekday,none!");
  vector<string> order = reorder_preferences(preferences, defaults);

  for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
  {
    if (*it == "weekday")
    {
      if (theHistory)
      {
        return (sentence << night_against_weekday(thePeriod.localEndTime(), *theHistory));
      }
      else
      {
        return (sentence << night_against_weekday(thePeriod.localEndTime()));
      }
    }
    else if (*it == "atnight")
    {
      if (isNextDay(theForecastTime, thePeriod.localEndTime())) return (sentence << "yolla");
    }
    else if (*it == "none")
    {
      if (isNextDay(theForecastTime, thePeriod.localEndTime())) return sentence;
    }
    else if (*it == "tonight")
    {
      if (isNextDay(theForecastTime, thePeriod.localEndTime())) return (sentence << "ensi yona");
    }
    else if (*it == "none!")
      return sentence;
    else
      throw TextGenError("PeriodPhrasefactory::tonight does not accept phrase " + *it);
  }

  throw TextGenError("PeriodPhrasefactory::tonight run out of options");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "next_night"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence next_night(const string& theVariable,
                    const TextGenPosixTime& theForecastTime,
                    const WeatherPeriod& thePeriod,
                    TextGen::WeatherHistory* theHistory = 0)
{
  using WeekdayTools::night_against_weekday;
  Sentence sentence;

  const string var = theVariable + "::next_night::phrases";
  const string preferences = Settings::optional_string(var, "");
  const string defaults("tonight,atnight,followingnight,weekday,none!");
  vector<string> order = reorder_preferences(preferences, defaults);

  for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
  {
    if (*it == "weekday")
    {
      if (theHistory)
      {
        return (sentence << night_against_weekday(thePeriod.localEndTime(), *theHistory));
      }
      else
      {
        return (sentence << night_against_weekday(thePeriod.localEndTime()));
      }
    }
    else if (*it == "followingnight")
      return (sentence << "seuraavana yona");
    else if (*it == "atnight")
    {
      if (isNextDay(theForecastTime, thePeriod.localEndTime())) return (sentence << "yolla");
    }
    else if (*it == "tonight")
    {
      if (isNextDay(theForecastTime, thePeriod.localEndTime())) return (sentence << "ensi yona");
    }
    else if (*it == "none!")
      return sentence;
    else
      throw TextGenError("PeriodPhrasefactory::next_night does not accept phrase " + *it);
  }

  throw TextGenError("PeriodPhrasefactory::next_night run out of options");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "next_day"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence next_day(const string& theVariable,
                  const TextGenPosixTime& theForecastTime,
                  const WeatherPeriod& thePeriod,
                  TextGen::WeatherHistory* theHistory = 0)
{
  using WeekdayTools::on_weekday;
  Sentence sentence;

  const string var = theVariable + "::next_day::phrases";
  const string preferences = Settings::optional_string(var, "");
  const string defaults("tomorrow,followingday,weekday,none!");
  vector<string> order = reorder_preferences(preferences, defaults);

  for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
  {
    if (*it == "weekday")
    {
      if (theHistory)
      {
        return (sentence << on_weekday(thePeriod.localStartTime(), *theHistory));
      }
      else
      {
        return (sentence << on_weekday(thePeriod.localStartTime()));
      }
    }
    else if (*it == "followingday")
      return (sentence << "seuraavana paivana");
    else if (*it == "tomorrow")
    {
      if (isNextDay(theForecastTime, thePeriod.localStartTime()))
      {
        if (theHistory)
        {
          return (sentence << WeekdayTools::get_time_phrase(
                      thePeriod.localStartTime(), "huomenna", *theHistory));
        }
        else
        {
          return (sentence << "huomenna");
        }
      }
    }
    else if (*it == "none!")
      return sentence;
    else
      throw TextGenError("PeriodPhrasefactory::next_day does not accept phrase " + *it);
  }

  throw TextGenError("PeriodPhrasefactory::next_day run out of options");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "next_days"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence next_days(const string& theVariable,
                   const TextGenPosixTime& theForecastTime,
                   const WeatherPeriod& thePeriod,
                   TextGen::WeatherHistory* theHistory = 0)
{
  using WeekdayTools::from_weekday;
  Sentence sentence;

  const string var = theVariable + "::next_days::phrases";
  const string preferences = Settings::optional_string(var, "");
  const string defaults("tomorrow,weekday,none!");
  vector<string> order = reorder_preferences(preferences, defaults);

  for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
  {
    if (*it == "weekday")
    {
      if (theHistory)
      {
        return (sentence << from_weekday(thePeriod.localStartTime(), *theHistory));
      }
      else
      {
        return (sentence << from_weekday(thePeriod.localStartTime()));
      }
    }
    else if (*it == "tomorrow")
    {
      // Sonera-sanakirjasta puuttuu "huomisesta alkaen"
      // if(isNextDay(theForecastTime,thePeriod.localStartTime()))
      // return (sentence << "huomisesta alkaen");
    }
    else if (*it == "none!")
      return sentence;
    else
      throw TextGenError("PeriodPhrasefactory::next_days does not accept phrase " + *it);
  }

  throw TextGenError("PeriodPhrasefactory::next_days run out of options");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "remaining_days"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence remaining_days(const string& theVariable,
                        const TextGenPosixTime& theForecastTime,
                        const WeatherPeriod& thePeriod,
                        TextGen::WeatherHistory* theHistory = 0)
{
  if (isSeveralDays(thePeriod.localStartTime(), thePeriod.localEndTime()))
    return next_days(theVariable, theForecastTime, thePeriod, theHistory);
  else
    return next_day(theVariable, theForecastTime, thePeriod, theHistory);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "remaining_day"
 *
 * \param thePeriod The time period
 *
 * \see \ref sec_periodifraasit_remaining_day
 */
// ----------------------------------------------------------------------

Sentence remaining_day(const WeatherPeriod& thePeriod)
{
  static const char* phrases[24] = {
      "",                      // 00
      "",                      // 01
      "",                      // 02
      "aamuyosta alkaen",      // 03
      "aamuyosta alkaen",      // 04
      "aamuyosta alkaen",      // 05
      "aamusta alkaen",        // 06
      "aamusta alkaen",        // 07
      "aamusta alkaen",        // 08
      "aamupaivasta alkaen",   // 09
      "aamupaivasta alkaen",   // 10
      "keskipaivasta alkaen",  // 11
      "keskipaivasta alkaen",  // 12
      "iltapaivasta alkaen",   // 13
      "iltapaivasta alkaen",   // 14
      "iltapaivasta alkaen",   // 15
      "iltapaivasta alkaen",   // 16
      "iltapaivasta alkaen",   // 17
      "illasta alkaen",        // 18
      "illasta alkaen",        // 19
      "illasta alkaen",        // 20
      "illasta alkaen",        // 21
      "yosta alkaen",          // 22
      "yosta alkaen"           // 23
  };

  const int hour = thePeriod.localStartTime().GetHour();

  Sentence sentence;
  sentence << phrases[hour];
  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing period of type "days"
 *
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence days(const string& theVariable,
              const TextGenPosixTime& theForecastTime,
              const WeatherPeriod& thePeriod,
              TextGen::WeatherHistory* theHistory = 0)
{
  using WeekdayTools::from_weekday;
  using WeekdayTools::on_weekday;
  using TextGen::HourPeriodGenerator;

  Sentence sentence;

  const string defaults("none,today,tomorrow,followingday,weekday,none!");

  const string var = theVariable + "::days::phrases";
  string preferences = Settings::optional_string(var, "");

  HourPeriodGenerator hours(thePeriod, theVariable + "::day");
  const int ndays = hours.size();

  if (ndays == 0) return sentence;

  const string nvar = var + "::days" + lexical_cast<string>(ndays);
  preferences = Settings::optional_string(nvar, preferences);
  vector<string> order = reorder_preferences(preferences, defaults);

  // the first day may not be the same as thePeriod.localStartTime
  // due to starthour etc settings

  const TextGenPosixTime starttime = hours.period(1).localStartTime();

  if (ndays == 1)
  {
    for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
    {
      if (*it == "none")
      {
        if (isSameDay(theForecastTime, starttime)) return sentence;
      }
      else if (*it == "today")
      {
        if (isSameDay(theForecastTime, starttime)) return (sentence << "tanaan");
      }
      else if (*it == "tomorrow")
      {
        if (isNextDay(theForecastTime, starttime))
        {
          if (theHistory)
          {
            return (sentence << WeekdayTools::get_time_phrase(starttime, "huomenna", *theHistory));
          }
          else
          {
            return (sentence << "huomenna");
          }
        }
      }
      else if (*it == "followingday")
        ;
      else if (*it == "weekday")
      {
        if (theHistory)
        {
          return (sentence << on_weekday(starttime, *theHistory));
        }
        else
        {
          return (sentence << on_weekday(starttime));
        }
      }
      else if (*it == "none!")
        return sentence;
      else
        throw TextGenError("PeriodPhrasefactory::days does not accept phrase " + *it);
    }
  }
  else if (ndays == 2)
  {
    TextGenPosixTime nextday(starttime);
    nextday.ChangeByDays(1);
    for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
    {
      if (*it == "none")
      {
        if (isSameDay(theForecastTime, starttime)) return sentence;
      }
      else if (*it == "today")
      {
        if (isSameDay(theForecastTime, starttime))
        {
          sentence << "tanaan"
                   << "ja";
          for (vector<string>::const_iterator jt = order.begin(); jt != order.end(); ++jt)
          {
            if (*jt == "tomorrow") return (sentence << "huomenna");
            if (*jt == "followingday") return (sentence << "seuraavana paivana");
            if (*jt == "weekday")
            {
              if (theHistory)
              {
                return (sentence << on_weekday(nextday, *theHistory));
              }
              else
              {
                return (sentence << on_weekday(nextday));
              }
            }
          }
        }
      }
      else if (*it == "tomorrow")
      {
        if (isNextDay(theForecastTime, starttime))
        {
          sentence << "huomenna"
                   << "ja";
          for (vector<string>::const_iterator jt = order.begin(); jt != order.end(); ++jt)
          {
            if (*jt == "followingday") return (sentence << "seuraavana paivana");
            if (*jt == "weekday")
            {
              if (theHistory)
              {
                return (sentence << on_weekday(nextday, *theHistory));
              }
              else
              {
                return (sentence << on_weekday(nextday));
              }
            }
          }
        }
      }
      else if (*it == "followingday")
        ;
      else if (*it == "weekday")
      {
        if (theHistory)
        {
          sentence << on_weekday(starttime, *theHistory) << "ja"
                   << on_weekday(nextday, *theHistory);
        }
        else
        {
          sentence << on_weekday(starttime) << "ja" << on_weekday(nextday);
        }
        return sentence;
      }
      else if (*it == "none!")
        return sentence;
      else
        throw TextGenError("PeriodPhrasefactory::days does not accept phrase " + *it);
    }
  }
  else
  {
    for (vector<string>::const_iterator it = order.begin(); it != order.end(); ++it)
    {
      if (*it == "none")
        return sentence;
      else if (*it == "today")
        ;
      else if (*it == "tomorrow")
      {
        // Sonera-sanakirjasta puuttuu "huomisesta alkaen"
        // if(isNextDay(theForecastTime, starttime))
        // return (sentence << "huomisesta alkaen");
      }
      else if (*it == "followingday")
        ;
      else if (*it == "weekday")
      {
        if (theHistory)
        {
          /*
                if(thePeriod.localEndTime().DifferenceInHours(thePeriod.localStartTime()) > 24)
                theHistory->updateTimePhrase("",thePeriod.localStartTime());
          */
          return (sentence << from_weekday(starttime, *theHistory));
        }
        else
        {
          return (sentence << from_weekday(starttime));
        }
      }
      else if (*it == "none!")
        return sentence;
      else
        throw TextGenError("PeriodPhrasefactory::days does not accept phrase " + *it);
    }
  }

  throw TextGenError("PeriodPhrasefactory::days run out of options");
}

}  // namespace anonymous

namespace PeriodPhraseFactory
{
Sentence create_sentence(const string& theType,
                         const string& theVariable,
                         const TextGenPosixTime& theForecastTime,
                         const WeatherPeriod& thePeriod,
                         TextGen::WeatherHistory* theHistory = 0)
{
  if (theType == "until_tonight")
    return until_tonight(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "until_morning")
    return until_morning(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "today") return today(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "tonight") return tonight(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "next_night")
    return next_night(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "next_day") return next_day(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "next_days") return next_days(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "remaining_days")
    return remaining_days(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "days") return days(theVariable, theForecastTime, thePeriod, theHistory);

  if (theType == "remaining_day") return remaining_day(thePeriod);

  throw TextGenError("PeriodPhraseFactory::create does not recognize type " + theType);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence describing the period
 *
 * \param theType The name of the period type
 * \param theVariable The Settings variable containing extra details
 * \param theForecastTime The forecast time
 * \param thePeriod The time period itself
 */
// ----------------------------------------------------------------------

Sentence create(const string& theType,
                const string& theVariable,
                const TextGenPosixTime& theForecastTime,
                const WeatherPeriod& thePeriod)
{
  return create_sentence(theType, theVariable, theForecastTime, thePeriod);

#if 0
	  if(theType == "until_tonight")
		return until_tonight(theVariable,theForecastTime,thePeriod);

	  if(theType == "until_morning")
		return until_morning(theVariable,theForecastTime,thePeriod);

	  if(theType == "today")
		return today(theVariable,theForecastTime,thePeriod);

	  if(theType == "tonight")
		return tonight(theVariable,theForecastTime,thePeriod);

	  if(theType == "next_night")
		return next_night(theVariable,theForecastTime,thePeriod);

	  if(theType == "next_day")
		return next_day(theVariable,theForecastTime,thePeriod);

	  if(theType == "next_days")
		return next_days(theVariable,theForecastTime,thePeriod);

	  if(theType == "remaining_days")
		return remaining_days(theVariable,theForecastTime,thePeriod);

	  if(theType == "days")
		return days(theVariable,theForecastTime,thePeriod);

	  if(theType == "remaining_day")
		return remaining_day(thePeriod);

	  throw TextGenError("PeriodPhraseFactory::create does not recognize type "+theType);
#endif
}

Sentence create(const string& theType,
                const string& theVariable,
                const TextGenPosixTime& theForecastTime,
                const WeatherPeriod& thePeriod,
                const WeatherArea& theArea)
{
  TextGen::WeatherHistory* theHistory = const_cast<TextGen::WeatherHistory*>(&(theArea.history()));

  return create_sentence(theType, theVariable, theForecastTime, thePeriod, theHistory);

#if 0
	  if(theType == "until_tonight")
		return until_tonight(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "until_morning")
		return until_morning(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "today")
		return today(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "tonight")
		return tonight(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "next_night")
		return next_night(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "next_day")
		return next_day(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "next_days")
		return next_days(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "remaining_days")
		return remaining_days(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "days")
		return days(theVariable,theForecastTime,thePeriod, theHistory);

	  if(theType == "remaining_day")
		return remaining_day(thePeriod, theHistory);

	  throw TextGenError("PeriodPhraseFactory::create does not recognize type "+theType);
#endif
}

}  // namespace PeriodPhraseFactory
}  // namespace TextGen
