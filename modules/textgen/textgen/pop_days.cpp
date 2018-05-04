// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::PrecipitationStory::pop_days
 */
// ======================================================================

#include "PrecipitationStory.h"
#include <calculator/DefaultAcceptor.h>
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "Integer.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include <calculator/NullPeriodGenerator.h>
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherResult.h>
#include <calculator/WeatherResultTools.h>
#include "PeriodPhraseFactory.h"

#include <boost/lexical_cast.hpp>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on maximum probability of precipitation
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph PrecipitationStory::pop_days() const
{
  MessageLogger log("PrecipitationStory::pop_days");

  using MathTools::to_precision;

  Paragraph paragraph;

  const int minimum = Settings::optional_percentage(itsVar + "::minimum", 10);
  const int maximum = Settings::optional_percentage(itsVar + "::maximum", 100);
  const int precision = Settings::optional_percentage(itsVar + "::precision", 10);
  const bool negate = Settings::optional_bool(itsVar + "::negate", false);

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

  HourPeriodGenerator generator(itsPeriod, itsVar + "::day");

  const int days = generator.size();

  if (days <= 0)
  {
    log << paragraph;
    return paragraph;
  }

  WeatherPeriod firstperiod = generator.period(1);

  GridForecaster forecaster;

  WeatherResult pop1max = forecaster.analyze(itsVar + "::fake::day1::meanmax",
                                             itsSources,
                                             PrecipitationProbability,
                                             Mean,
                                             Maximum,
                                             itsArea,
                                             firstperiod);

  WeatherResult pop1mean = forecaster.analyze(itsVar + "::fake::day1::meanmean",
                                              itsSources,
                                              PrecipitationProbability,
                                              Mean,
                                              Mean,
                                              itsArea,
                                              firstperiod);

  if (pop1max.value() == kFloatMissing || pop1mean.value() == kFloatMissing)
    throw TextGenError("PrecipitationProbability not available");

  WeatherResult result1 = WeatherResultTools::mean(pop1max, pop1mean);

  log << "PoP Mean(Mean) for day 1 " << pop1mean << endl
      << "PoP Mean(Max) for day 1 " << pop1max << endl
      << "Pop for day 1 is the mean value " << result1 << endl;

  const int pop1 = to_precision(result1.value(), precision);

  Sentence sentence;

  if (pop1 >= minimum && pop1 <= maximum)
  {
    sentence << (negate ? "poudan todennakoisyys" : "sateen todennakoisyys") << "on"
             << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, firstperiod)
             << (negate ? Integer(100 - pop1) : Integer(pop1)) << *UnitFactory::create(Percent);
  }

  if (days >= 2)
  {
    WeatherPeriod secondperiod = generator.period(2);

    WeatherResult pop2max = forecaster.analyze(itsVar + "::fake::day2::meanmax",
                                               itsSources,
                                               PrecipitationProbability,
                                               Mean,
                                               Maximum,
                                               itsArea,
                                               secondperiod);

    WeatherResult pop2mean = forecaster.analyze(itsVar + "::fake::day2::meanmean",
                                                itsSources,
                                                PrecipitationProbability,
                                                Mean,
                                                Mean,
                                                itsArea,
                                                secondperiod);

    if (pop2max.value() == kFloatMissing || pop2mean.value() == kFloatMissing)
      throw TextGenError("PrecipitationProbability not available");

    WeatherResult result2 = WeatherResultTools::mean(pop2max, pop2mean);

    log << "PoP Mean(Mean) for day 2 " << pop2mean << endl
        << "PoP Mean(Max) for day 2 " << pop2max << endl
        << "Pop for day 2 is the mean value " << result2 << endl;

    const int pop2 = to_precision(result2.value(), precision);

    if (pop2 >= minimum && pop2 <= maximum)
    {
      if (sentence.empty())
      {
        sentence << (negate ? "poudan todennakoisyys" : "sateen todennakoisyys") << "on"
                 << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, secondperiod)
                 << (negate ? Integer(100 - pop2) : Integer(pop2)) << *UnitFactory::create(Percent);
      }
      else
      {
        sentence << Delimiter(",")
                 << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, secondperiod);
        int difference = (negate ? (100 - pop2) - (100 - pop1) : (pop2 - pop1));

        if (difference >= limit_significantly_greater)
          sentence << "huomattavasti suurempi";
        else if (difference >= limit_greater)
          sentence << "suurempi";
        else if (difference >= limit_somewhat_greater)
          sentence << "hieman suurempi";
        else if (-difference >= limit_significantly_smaller)
          sentence << "huomattavasti pienempi";
        else if (-difference >= limit_smaller)
          sentence << "pienempi";
        else if (-difference >= limit_somewhat_smaller)
          sentence << "hieman pienempi";
        else
          sentence << "sama";
      }
    }
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
