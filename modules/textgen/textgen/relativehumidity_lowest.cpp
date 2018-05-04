// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::RelativeHumidityStory::lowest
 */
// ======================================================================

#include "RelativeHumidityStory.h"
#include <calculator/DefaultAcceptor.h>
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "Integer.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include <calculator/NullPeriodGenerator.h>
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherPeriodTools.h>
#include <calculator/WeatherResult.h>
#include "PeriodPhraseFactory.h"

#include <boost/lexical_cast.hpp>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on lowest relative humidity
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph RelativeHumidityStory::lowest() const
{
  MessageLogger log("RelativeHumidityStory::lowest");

  using MathTools::to_precision;

  Paragraph paragraph;

  const int starthour = Settings::optional_hour(itsVar + "::day::starthour", 0);
  const int endhour = Settings::optional_hour(itsVar + "::day::endhour", 0);

  const int maxstarthour = Settings::optional_hour(itsVar + "::day::maxstarthour", starthour);
  const int minendhour = Settings::optional_hour(itsVar + "::day::minendhour", endhour);

  const int precision = Settings::require_percentage(itsVar + "::precision");

  const int limit_significantly_greater =
      Settings::require_percentage(itsVar + "::comparison::significantly_greater");
  const int limit_significantly_smaller =
      Settings::require_percentage(itsVar + "::comparison::significantly_smaller");
  const int limit_greater = Settings::require_percentage(itsVar + "::comparison::greater");
  const int limit_smaller = Settings::require_percentage(itsVar + "::comparison::smaller");
  const int limit_somewhat_greater =
      Settings::require_percentage(itsVar + "::comparison::somewhat_greater");
  const int limit_somewhat_smaller =
      Settings::require_percentage(itsVar + "::comparison::somewhat_smaller");

  const int days =
      WeatherPeriodTools::countPeriods(itsPeriod, starthour, endhour, maxstarthour, minendhour);

  WeatherPeriod firstperiod =
      WeatherPeriodTools::getPeriod(itsPeriod, 1, starthour, endhour, maxstarthour, minendhour);

  GridForecaster forecaster;

  WeatherResult result = forecaster.analyze(itsVar + "::fake::day1::minimum",
                                            itsSources,
                                            RelativeHumidity,
                                            Minimum,
                                            Minimum,
                                            itsArea,
                                            firstperiod);

  if (result.value() == kFloatMissing) throw TextGenError("RelativeHumidity not available");

  log << "RelativeHumidity Minimum(Minimum) for day 1 " << result << endl;

  const int humidity1 = to_precision(result.value(), precision);

  Sentence sentence;

  sentence << "alin suhteellinen kosteus"
           << "on" << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, firstperiod)
           << Integer(humidity1) << *UnitFactory::create(Percent);

  if (days == 2)
  {
    WeatherPeriod secondperiod =
        WeatherPeriodTools::getPeriod(itsPeriod, 2, starthour, endhour, maxstarthour, minendhour);

    WeatherResult result2 = forecaster.analyze(itsVar + "::fake::day2::minimum",
                                               itsSources,
                                               RelativeHumidity,
                                               Minimum,
                                               Minimum,
                                               itsArea,
                                               secondperiod);

    if (result2.value() == kFloatMissing) throw TextGenError("RelativeHumidity not available");

    log << "RelativeHumidity Minimum(Minimum) for day 2 " << result2 << endl;

    const int humidity2 = to_precision(result2.value(), precision);

    sentence << Delimiter(",")
             << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, secondperiod);
    if (humidity2 - humidity1 >= limit_significantly_greater)
      sentence << "huomattavasti suurempi";
    else if (humidity2 - humidity1 >= limit_greater)
      sentence << "suurempi";
    else if (humidity2 - humidity1 >= limit_somewhat_greater)
      sentence << "hieman suurempi";
    else if (humidity1 - humidity2 >= limit_significantly_smaller)
      sentence << "huomattavasti pienempi";
    else if (humidity1 - humidity2 >= limit_smaller)
      sentence << "pienempi";
    else if (humidity1 - humidity2 >= limit_somewhat_smaller)
      sentence << "hieman pienempi";
    else
      sentence << "sama";
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
