// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::WindStory::daily_ranges
 */
// ======================================================================

#include "WindStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "PeriodPhraseFactory.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/WeatherResult.h>
#include <calculator/WeatherResultTools.h>
#include "WeekdayTools.h"
#include "WindStoryTools.h"

#include <boost/lexical_cast.hpp>

#include <vector>

using namespace TextGen::WindStoryTools;
using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Return wind_daily_ranges story
 *
 * \return Paragraph containing the story
 */
// ----------------------------------------------------------------------

Paragraph WindStory::daily_ranges() const
{
  MessageLogger log("WindStory::daily_ranges");

  using WeekdayTools::on_weekday;
  using WeekdayTools::from_weekday;

  using WeatherResultTools::min;
  using WeatherResultTools::max;
  using WeatherResultTools::mean;
  using WeatherResultTools::isSimilarRange;

  // Establish options

  using namespace Settings;

  // Generate the story

  Paragraph paragraph;

  GridForecaster forecaster;

  // All day periods

  const HourPeriodGenerator periodgenerator(itsPeriod, itsVar + "::day");
  const int ndays = periodgenerator.size();

  log << "Period covers " << ndays << " days" << endl;

  if (ndays <= 0)
  {
    log << paragraph;
    return paragraph;
  }

  // Calculate wind speeds for max 3 days

  vector<WeatherPeriod> periods;
  vector<WeatherResult> meanspeeds;
  vector<WeatherResult> maxspeeds;
  vector<WeatherResult> minspeeds;
  vector<WeatherResult> directions;
  vector<WindDirectionAccuracy> accuracies;

  for (int day = 1; day <= std::min(ndays, 3); day++)
  {
    const WeatherPeriod period(periodgenerator.period(day));

    const string daystr = "day" + lexical_cast<string>(day);

    const WeatherResult minspeed =
        forecaster.analyze(itsVar + "::fake::" + daystr + "::speed::minimum",
                           itsSources,
                           WindSpeed,
                           Minimum,
                           Mean,
                           itsArea,
                           period);

    const WeatherResult maxspeed =
        forecaster.analyze(itsVar + "::fake::" + daystr + "::speed::maximum",
                           itsSources,
                           WindSpeed,
                           Maximum,
                           Mean,
                           itsArea,
                           period);

    const WeatherResult meanspeed =
        forecaster.analyze(itsVar + "::fake::" + daystr + "::speed::mean",
                           itsSources,
                           WindSpeed,
                           Mean,
                           Mean,
                           itsArea,
                           period);
    const WeatherResult direction =
        forecaster.analyze(itsVar + "::fake::" + daystr + "::direction::mean",
                           itsSources,
                           WindDirection,
                           Mean,
                           Mean,
                           itsArea,
                           period);

    log << "WindSpeed Minimum(Mean) " << daystr << " = " << minspeed << endl;
    log << "WindSpeed Maximum(Mean) " << daystr << " = " << maxspeed << endl;
    log << "WindSpeed Mean(Mean) " << daystr << " = " << meanspeed << endl;

    periods.push_back(period);
    minspeeds.push_back(minspeed);
    maxspeeds.push_back(maxspeed);
    meanspeeds.push_back(meanspeed);
    directions.push_back(direction);
    accuracies.push_back(direction_accuracy(direction.error(), itsVar));
  }

  switch (ndays)
  {
    case 1:
    {
      Sentence sentence;
      sentence << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, periods[0])
               << directed_speed_sentence(
                      minspeeds[0], maxspeeds[0], meanspeeds[0], directions[0], itsVar);
      paragraph << sentence;
      break;
    }
    case 2:
    {
      Sentence sentence;

      const WeatherResult direction12 =
          forecaster.analyze(itsVar + "::fake::days1-2::direction::mean",
                             itsSources,
                             WindDirection,
                             Mean,
                             Mean,
                             Mean,
                             itsArea,
                             periodgenerator);

      const WindDirectionAccuracy accuracy12 = direction_accuracy(direction12.error(), itsVar);

      bool similar_speeds =
          isSimilarRange(minspeeds[0], maxspeeds[0], minspeeds[1], maxspeeds[1], itsVar);

      const string var = "textgen::units::meterspersecond::format";
      const string opt = Settings::optional_string(var, "SI");
      if (opt == "textphrase")
      {
        string speed_string0(speed_string(meanspeeds[0]));
        string speed_string1(speed_string(meanspeeds[1]));

        similar_speeds = (speed_string0.compare(speed_string1) == 0);
      }

      if (accuracy12 != bad_accuracy ||
          (accuracies[0] == bad_accuracy && accuracies[1] == bad_accuracy))
      {
        if (similar_speeds)
        {
          WeatherPeriod days12(periods[0].localStartTime(), periods[1].localEndTime());
          sentence << PeriodPhraseFactory::create("days", itsVar, itsForecastTime, days12)
                   << directed_speed_sentence(min(minspeeds[0], minspeeds[1]),
                                              max(maxspeeds[0], maxspeeds[1]),
                                              mean(meanspeeds[0], meanspeeds[1]),
                                              direction12,
                                              itsVar);
        }
        else
        {
          if (opt == "textphrase")
          {
            Sentence todaySentence;
            Sentence nextdaySentence;
            Sentence nextdaySpeedSentence;
            const std::string speed_str(speed_string(meanspeeds[0]));

            if (!speed_str.empty())
              todaySentence << PeriodPhraseFactory::create(
                                   "today", itsVar, itsForecastTime, periods[0])
                            << speed_str;
            nextdaySpeedSentence << directed_speed_sentence(
                minspeeds[1], maxspeeds[1], meanspeeds[1], direction12, itsVar);
            if (!nextdaySpeedSentence.empty())
              nextdaySentence << PeriodPhraseFactory::create(
                                     "next_day", itsVar, itsForecastTime, periods[1])
                              << nextdaySpeedSentence;

            // esim. Tänään heikkoa, huomenna kohtalaista etelätuulta
            if (!todaySentence.empty() && !nextdaySentence.empty())
              sentence << todaySentence << Delimiter(",") << nextdaySentence;
            else if (todaySentence.empty() && !nextdaySentence.empty())
              sentence << nextdaySentence;
          }
          else
          {
            sentence << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, periods[0])
                     << directed_speed_sentence(
                            minspeeds[0], maxspeeds[0], meanspeeds[0], direction12, itsVar)
                     << Delimiter(",")
                     << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[1])
                     << speed_range_sentence(minspeeds[1], maxspeeds[1], meanspeeds[1], itsVar);
          }
        }
      }
      else
      {
        Sentence todaySpeedSentence;
        Sentence todaySentence;

        todaySpeedSentence << directed_speed_sentence(
            minspeeds[0], maxspeeds[0], meanspeeds[0], directions[0], itsVar);

        if (!todaySpeedSentence.empty())
          sentence << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, periods[0])
                   << todaySpeedSentence << Delimiter(",")
                   << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[1]);
        else
          sentence << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[1]);
        if (similar_speeds)
        {
          sentence << direction_sentence(directions[1], itsVar);
        }
        else
        {
          sentence << directed_speed_sentence(
              minspeeds[1], maxspeeds[1], meanspeeds[1], directions[1], itsVar);
        }
      }
      paragraph << sentence;
      break;
    }
    default:
    {
      Sentence sentence;

      // various useful subperiods

      const WeatherPeriod days123(periods[0].localStartTime(), periods[2].localEndTime());
      const WeatherPeriod days12(periods[0].localStartTime(), periods[1].localEndTime());
      const WeatherPeriod days23(periods[1].localStartTime(), periods[2].localEndTime());

      const WeatherResult direction123 =
          forecaster.analyze(itsVar + "::fake::days1-3::direction::mean",
                             itsSources,
                             WindDirection,
                             Mean,
                             Mean,
                             Mean,
                             itsArea,
                             HourPeriodGenerator(days123, itsVar + "::day"));

      const WeatherResult direction12 =
          forecaster.analyze(itsVar + "::fake::days1-2::direction::mean",
                             itsSources,
                             WindDirection,
                             Mean,
                             Mean,
                             Mean,
                             itsArea,
                             HourPeriodGenerator(days12, itsVar + "::day"));

      const WeatherResult direction23 =
          forecaster.analyze(itsVar + "::fake::days2-3::direction::mean",
                             itsSources,
                             WindDirection,
                             Mean,
                             Mean,
                             Mean,
                             itsArea,
                             HourPeriodGenerator(days23, itsVar + "::day"));

      const WindDirectionAccuracy accuracy123 = direction_accuracy(direction123.error(), itsVar);
      const WindDirectionAccuracy accuracy12 = direction_accuracy(direction12.error(), itsVar);
      const WindDirectionAccuracy accuracy23 = direction_accuracy(direction23.error(), itsVar);

      const bool similar_speeds12 =
          isSimilarRange(minspeeds[0], maxspeeds[0], minspeeds[1], maxspeeds[1], itsVar);

      const bool similar_speeds23 =
          isSimilarRange(minspeeds[1], maxspeeds[1], minspeeds[2], maxspeeds[2], itsVar);

      const bool similar_speeds13 =
          isSimilarRange(minspeeds[0], maxspeeds[0], minspeeds[2], maxspeeds[2], itsVar);

      // All pairs 1&2,2&3 and 1&3 must be similar
      const bool similar_speeds123 = (similar_speeds12 && similar_speeds23 && similar_speeds13);

      if (accuracy123 != bad_accuracy ||
          (accuracies[0] == bad_accuracy && accuracies[1] == bad_accuracy &&
           accuracies[2] == bad_accuracy))
      {
        if (!similar_speeds12)
        {
          sentence << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, periods[0])
                   << directed_speed_sentence(
                          minspeeds[0], maxspeeds[0], meanspeeds[0], direction123, itsVar);
        }
        else if (!similar_speeds123)
        {
          sentence << directed_speed_sentence(min(minspeeds[0], minspeeds[1]),
                                              max(maxspeeds[0], maxspeeds[1]),
                                              mean(meanspeeds[0], meanspeeds[1]),
                                              direction123,
                                              itsVar);
        }
        else
        {
          sentence << PeriodPhraseFactory::create("days", itsVar, itsForecastTime, days123)
                   << directed_speed_sentence(min(minspeeds[0], minspeeds[1], minspeeds[2]),
                                              max(maxspeeds[0], maxspeeds[1], maxspeeds[2]),
                                              mean(meanspeeds[0], meanspeeds[1], maxspeeds[2]),
                                              direction123,
                                              itsVar);
        }

        if (!similar_speeds123)
        {
          // second day
          if (!similar_speeds12)
          {
            sentence << Delimiter(",");
            if (similar_speeds23)
              sentence << PeriodPhraseFactory::create(
                              "next_days", itsVar, itsForecastTime, periods[1])
                       << speed_range_sentence(min(minspeeds[1], minspeeds[2]),
                                               max(maxspeeds[1], maxspeeds[2]),
                                               mean(meanspeeds[1], meanspeeds[2]),
                                               itsVar);
            else
              sentence << PeriodPhraseFactory::create(
                              "next_day", itsVar, itsForecastTime, periods[1])
                       << speed_range_sentence(minspeeds[1], maxspeeds[1], meanspeeds[1], itsVar);
          }
          // third day
          if (!similar_speeds23)
          {
            sentence << Delimiter(",")
                     << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[2])
                     << speed_range_sentence(minspeeds[2], maxspeeds[2], meanspeeds[2], itsVar);
          }
        }
      }
      else if (accuracy12 != bad_accuracy ||
               (accuracies[0] == bad_accuracy && accuracies[1] == bad_accuracy))
      {
        if (!similar_speeds12)
        {
          sentence << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, periods[0])
                   << directed_speed_sentence(
                          minspeeds[0], maxspeeds[0], meanspeeds[0], direction12, itsVar);
        }
        else if (!similar_speeds123)
        {
          sentence << directed_speed_sentence(min(minspeeds[0], minspeeds[1]),
                                              max(maxspeeds[0], maxspeeds[1]),
                                              mean(meanspeeds[0], meanspeeds[1]),
                                              direction12,
                                              itsVar);
        }
        else
        {
          sentence << directed_speed_sentence(min(minspeeds[0], minspeeds[1], minspeeds[2]),
                                              max(maxspeeds[0], maxspeeds[1], maxspeeds[2]),
                                              mean(meanspeeds[0], meanspeeds[1], maxspeeds[2]),
                                              direction12,
                                              itsVar);
        }

        // second day
        if (!similar_speeds12)
        {
          sentence << Delimiter(",")
                   << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[1]);
          if (!similar_speeds23)
            sentence << speed_range_sentence(minspeeds[1], maxspeeds[1], meanspeeds[1], itsVar);
          else
            sentence << speed_range_sentence(min(minspeeds[1], minspeeds[2]),
                                             max(maxspeeds[1], maxspeeds[2]),
                                             mean(meanspeeds[1], meanspeeds[2]),
                                             itsVar);
        }

        // third day
        sentence << Delimiter(",");
        if (!similar_speeds12)
          sentence << on_weekday(periods[2].localStartTime());
        else
          sentence << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[2]);
        if (similar_speeds23)
          sentence << direction_sentence(directions[2], itsVar);
        else
          sentence << directed_speed_sentence(
              minspeeds[2], maxspeeds[2], meanspeeds[2], directions[2], itsVar);
      }
      else if (accuracy23 != bad_accuracy ||
               (accuracies[1] == bad_accuracy && accuracies[2] == bad_accuracy))
      {
        sentence << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, periods[0]);
        if (!similar_speeds12)
          sentence << directed_speed_sentence(
              minspeeds[0], maxspeeds[0], meanspeeds[0], directions[0], itsVar);
        else if (!similar_speeds123)
          sentence << directed_speed_sentence(min(minspeeds[0], minspeeds[1]),
                                              max(maxspeeds[0], maxspeeds[1]),
                                              mean(meanspeeds[0], meanspeeds[1]),
                                              directions[0],
                                              itsVar);
        else
          sentence << directed_speed_sentence(min(minspeeds[0], minspeeds[1], minspeeds[2]),
                                              max(maxspeeds[0], maxspeeds[1], maxspeeds[2]),
                                              mean(meanspeeds[0], meanspeeds[1], maxspeeds[2]),
                                              directions[0],
                                              itsVar);

        sentence << Delimiter(",");
        if (similar_speeds23)
          sentence << PeriodPhraseFactory::create("next_days", itsVar, itsForecastTime, periods[1]);
        else
          sentence << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[1]);
        sentence << direction_sentence(direction23, itsVar);

        if (!similar_speeds12)
        {
          if (similar_speeds23)
            sentence << speed_range_sentence(min(minspeeds[1], minspeeds[2]),
                                             max(maxspeeds[1], maxspeeds[2]),
                                             mean(meanspeeds[1], meanspeeds[2]),
                                             itsVar);
          else
            sentence << speed_range_sentence(minspeeds[1], maxspeeds[1], meanspeeds[1], itsVar);
        }

        if (!similar_speeds23)
        {
          sentence << Delimiter(",")
                   << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[2])
                   << speed_range_sentence(minspeeds[2], maxspeeds[2], meanspeeds[2], itsVar);
        }
      }
      else
      {
        sentence << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, periods[0]);
        sentence << direction_sentence(directions[0], itsVar);

        if (similar_speeds123)
          sentence << speed_range_sentence(min(minspeeds[0], minspeeds[1], minspeeds[2]),
                                           max(maxspeeds[0], maxspeeds[1], maxspeeds[2]),
                                           mean(meanspeeds[0], meanspeeds[1], meanspeeds[2]),
                                           itsVar);
        else if (similar_speeds12)
          sentence << speed_range_sentence(min(minspeeds[0], minspeeds[1]),
                                           max(maxspeeds[0], maxspeeds[1]),
                                           mean(meanspeeds[0], meanspeeds[1]),
                                           itsVar);
        else
          sentence << speed_range_sentence(minspeeds[0], maxspeeds[0], meanspeeds[0], itsVar);

        sentence << Delimiter(",");
        sentence << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[1]);
        sentence << direction_sentence(directions[1], itsVar);

        if (!similar_speeds12)
        {
          if (similar_speeds23)
            sentence << speed_range_sentence(min(minspeeds[1], minspeeds[2]),
                                             max(maxspeeds[1], maxspeeds[2]),
                                             mean(meanspeeds[1], meanspeeds[2]),
                                             itsVar);
          else
            sentence << speed_range_sentence(minspeeds[1], maxspeeds[1], meanspeeds[1], itsVar);
        }

        sentence << Delimiter(",");
        sentence << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[2]);
        sentence << direction_sentence(directions[2], itsVar);
        if (!similar_speeds23)
          sentence << speed_range_sentence(minspeeds[2], maxspeeds[2], meanspeeds[2], itsVar);
      }

      paragraph << sentence;
    }
  }

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
