// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::WindStory::simple_overview
 */
// ======================================================================

#include "WindStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "PeriodPhraseFactory.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/WeatherPeriodTools.h>
#include <calculator/WeatherResult.h>
#include "WindStoryTools.h"

using namespace TextGen::WindStoryTools;
using namespace TextGen;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Return wind short_overview story
 *
 * \return Paragraph containing the story
 */
// ----------------------------------------------------------------------

Paragraph WindStory::simple_overview() const
{
  MessageLogger log("WeatherStory::simple_overview");

  // Establish options

  using namespace Settings;

  const int day_starthour = optional_hour(itsVar + "::day::starthour", 6);
  const int day_maxstarthour = optional_hour(itsVar + "::day::maxstarthour", day_starthour);
  const int night_starthour = optional_hour(itsVar + "::night::starthour", 18);
  const int night_maxstarthour = optional_hour(itsVar + "::night::maxstarthour", night_starthour);
  const int night_endhour = optional_hour(itsVar + "::night::endhour", 6);
  const int night_minendhour = optional_hour(itsVar + "::night::minendhour", night_endhour);

  // Generate the story

  Paragraph paragraph;

  GridForecaster forecaster;

  // The period until next morning should always be possible

  const int starthour = itsPeriod.localStartTime().GetHour();
  WeatherPeriod morning = WeatherPeriodTools::getPeriod(
      itsPeriod, 1, starthour, night_endhour, starthour, night_minendhour);

  if (itsPeriod.localStartTime().GetHour() > day_maxstarthour)
  {
    // Story until next morning

    Sentence sentence;

    sentence << PeriodPhraseFactory::create("until_morning", itsVar, itsForecastTime, morning);

    WeatherResult minspeed = forecaster.analyze(itsVar + "::fake::until_morning::speed::minimum",
                                                itsSources,
                                                WindSpeed,
                                                Minimum,
                                                Mean,
                                                itsArea,
                                                morning);

    WeatherResult maxspeed = forecaster.analyze(itsVar + "::fake::until_morning::speed::maximum",
                                                itsSources,
                                                WindSpeed,
                                                Maximum,
                                                Mean,
                                                itsArea,
                                                morning);

    WeatherResult meanspeed = forecaster.analyze(itsVar + "::fake::until_morning::speed::mean",
                                                 itsSources,
                                                 WindSpeed,
                                                 Mean,
                                                 Mean,
                                                 itsArea,
                                                 morning);

    WeatherResult direction = forecaster.analyze(itsVar + "::fake::until_morning::direction",
                                                 itsSources,
                                                 WindDirection,
                                                 Mean,
                                                 Mean,
                                                 itsArea,
                                                 morning);

    log << "WindSpeed Minimum(Mean) until_morning  = " << minspeed << endl;
    log << "WindSpeed Maximum(Mean) until_morning  = " << maxspeed << endl;
    log << "WindSpeed Mean(Mean) until_morning  = " << meanspeed << endl;
    log << "WindDirection Mean(Mean) until_morning  = " << direction << endl;

    sentence << directed_speed_sentence(minspeed, maxspeed, meanspeed, direction, itsVar);
    paragraph << sentence;
  }
  else
  {
    // First until night, then until morning

    Sentence sentence;

    // from now until night
    WeatherPeriod day = WeatherPeriodTools::getPeriod(itsPeriod, 1, starthour, night_starthour);

    WeatherResult minspeed = forecaster.analyze(itsVar + "::fake::until_tonight::speed::minimum",
                                                itsSources,
                                                WindSpeed,
                                                Minimum,
                                                Mean,
                                                itsArea,
                                                day);

    WeatherResult maxspeed = forecaster.analyze(itsVar + "::fake::until_tonight::speed::maximum",
                                                itsSources,
                                                WindSpeed,
                                                Maximum,
                                                Mean,
                                                itsArea,
                                                day);

    WeatherResult meanspeed = forecaster.analyze(itsVar + "::fake::until_tonight::speed::mean",
                                                 itsSources,
                                                 WindSpeed,
                                                 Mean,
                                                 Mean,
                                                 itsArea,
                                                 day);

    WeatherResult direction = forecaster.analyze(itsVar + "::fake::until_tonight::direction",
                                                 itsSources,
                                                 WindDirection,
                                                 Mean,
                                                 Mean,
                                                 itsArea,
                                                 day);

    log << "WindSpeed Minimum(Mean) until_tonight  = " << minspeed << endl;
    log << "WindSpeed Maximum(Mean) until_tonight  = " << maxspeed << endl;
    log << "WindSpeed Mean(Mean) until_tonight  = " << meanspeed << endl;
    log << "WindDirection Mean(Mean) until_tonight  = " << direction << endl;

    sentence << PeriodPhraseFactory::create("until_tonight", itsVar, itsForecastTime, day);

    sentence << directed_speed_sentence(minspeed, maxspeed, meanspeed, direction, itsVar);

    sentence << Delimiter(",");

    // next night
    WeatherPeriod night = WeatherPeriodTools::getPeriod(
        itsPeriod, 1, night_starthour, night_endhour, night_maxstarthour, night_minendhour);

    minspeed = forecaster.analyze(itsVar + "::fake::tonight::speed::minimum",
                                  itsSources,
                                  WindSpeed,
                                  Minimum,
                                  Mean,
                                  itsArea,
                                  night);

    maxspeed = forecaster.analyze(itsVar + "::fake::tonight::speed::maximum",
                                  itsSources,
                                  WindSpeed,
                                  Maximum,
                                  Mean,
                                  itsArea,
                                  night);

    meanspeed = forecaster.analyze(
        itsVar + "::fake::tonight::speed::mean", itsSources, WindSpeed, Mean, Mean, itsArea, night);

    direction = forecaster.analyze(itsVar + "::fake::tonight::direction",
                                   itsSources,
                                   WindDirection,
                                   Mean,
                                   Mean,
                                   itsArea,
                                   night);

    log << "WindSpeed Minimum(Mean) tonight  = " << minspeed << endl;
    log << "WindSpeed Maximum(Mean) tonight  = " << maxspeed << endl;
    log << "WindSpeed Mean(Mean) tonight  = " << meanspeed << endl;
    log << "WindDirection Mean(Mean) tonight  = " << direction << endl;

    sentence << PeriodPhraseFactory::create("tonight", itsVar, itsForecastTime, night);

    sentence << directed_speed_sentence(minspeed, maxspeed, meanspeed, direction, itsVar);

    paragraph << sentence;
  }

  // Remaining time after the morning

  if (morning.localEndTime().IsLessThan(itsPeriod.localEndTime()))
  {
    const WeatherPeriod remainder(morning.localEndTime(), itsPeriod.localEndTime());

    WeatherResult minspeed = forecaster.analyze(itsVar + "::fake::remaining_days::speed::minimum",
                                                itsSources,
                                                WindSpeed,
                                                Minimum,
                                                Mean,
                                                itsArea,
                                                remainder);

    WeatherResult maxspeed = forecaster.analyze(itsVar + "::fake::remaining_days::speed::maximum",
                                                itsSources,
                                                WindSpeed,
                                                Maximum,
                                                Mean,
                                                itsArea,
                                                remainder);

    WeatherResult meanspeed = forecaster.analyze(itsVar + "::fake::remaining_days::speed::mean",
                                                 itsSources,
                                                 WindSpeed,
                                                 Mean,
                                                 Mean,
                                                 itsArea,
                                                 remainder);

    WeatherResult direction = forecaster.analyze(itsVar + "::fake::remaining_days::direction",
                                                 itsSources,
                                                 WindDirection,
                                                 Mean,
                                                 Mean,
                                                 itsArea,
                                                 remainder);

    log << "WindSpeed Minimum(Mean) remaining_days  = " << minspeed << endl;
    log << "WindSpeed Maximum(Mean) remaining_days  = " << maxspeed << endl;
    log << "WindSpeed Mean(Mean) remaining_days  = " << meanspeed << endl;
    log << "WindDirection Mean(Mean) remaining_days  = " << direction << endl;

    Sentence sentence;
    sentence << PeriodPhraseFactory::create("remaining_days", itsVar, itsForecastTime, remainder);

    sentence << directed_speed_sentence(minspeed, maxspeed, meanspeed, direction, itsVar);

    paragraph << sentence;
  }

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
