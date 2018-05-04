// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::TemperatureStory::day
 */
// ======================================================================

#include "TemperatureStory.h"

#include <calculator/DefaultAcceptor.h>
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "NightAndDayPeriodGenerator.h"
#include "Integer.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "TemperatureStoryTools.h"
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherPeriodTools.h>
#include <calculator/WeatherResult.h>
#include "PeriodPhraseFactory.h"

#include <newbase/NFmiStringTools.h>

#include <boost/lexical_cast.hpp>

using namespace TextGen;
using namespace TextGen::TemperatureStoryTools;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Calculate area results
 */
// ----------------------------------------------------------------------

void calculate_mean_min_max(MessageLogger& theLog,
                            const WeatherForecaster& theForecaster,
                            const string& theVar,
                            const AnalysisSources& theSources,
                            const WeatherArea& theArea,
                            const WeatherPeriod& thePeriod,
                            WeatherFunction theFunction,
                            WeatherResult& theMean,
                            WeatherResult& theMin,
                            WeatherResult& theMax)
{
  theMean = theForecaster.analyze(
      theVar + "::mean", theSources, Temperature, Mean, theFunction, theArea, thePeriod);

  theMin = theForecaster.analyze(
      theVar + "::min", theSources, Temperature, Minimum, theFunction, theArea, thePeriod);

  theMax = theForecaster.analyze(
      theVar + "::max", theSources, Temperature, Maximum, theFunction, theArea, thePeriod);

  theLog << "Temperature Mean(Maximum) " << theMean << endl;
  theLog << "Temperature Min(Maximum) " << theMin << endl;
  theLog << "Temperature Max(Maximum) " << theMax << endl;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate story on temperature for the day and night
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph TemperatureStory::day() const
{
  MessageLogger log("TemperatureStory::day");

  using MathTools::to_precision;
  using Settings::optional_int;
  using Settings::optional_bool;
  using Settings::optional_string;
  using NFmiStringTools::Convert;

  Paragraph paragraph;

  // Period generator

  NightAndDayPeriodGenerator generator(itsPeriod, itsVar);

  // Too late? Return empty story then

  if (generator.size() == 0)
  {
    log << paragraph;
    return paragraph;
  }

  log << "Period contains " << generator.size() << " days or nights" << endl;

  // The options

  const int day_night_limit = optional_int(itsVar + "::day_night_limit", 3);
  const int coast_numeric_limit = optional_int(itsVar + "::coast_numeric_limit", 8);
  const int coast_not_below = optional_int(itsVar + "::coast_not_below", -15);
  const int coast_limit = optional_int(itsVar + "::coast_limit", 2);

  const int mininterval = optional_int(itsVar + "::mininterval", 2);
  const bool interval_zero = optional_bool(itsVar + "::always_interval_zero", false);

  const string rangeseparator = optional_string(itsVar + "::rangeseparator", "...");

  WeatherArea coast = itsArea;
  coast.type(WeatherArea::Coast);

  WeatherArea inland = itsArea;
  inland.type(WeatherArea::Inland);

  // Calculate the results

  GridForecaster forecaster;

  unsigned int periodnum = 1;
  unsigned int part = 0;
  while (periodnum <= generator.size())
  {
    ++part;

    log << "Sentence " << part << ":" << endl;

    WeatherResult areameanday(kFloatMissing, 0);
    WeatherResult areaminday(kFloatMissing, 0);
    WeatherResult areamaxday(kFloatMissing, 0);

    WeatherResult coastmeanday(kFloatMissing, 0);
    WeatherResult coastminday(kFloatMissing, 0);
    WeatherResult coastmaxday(kFloatMissing, 0);

    WeatherResult inlandmeanday(kFloatMissing, 0);
    WeatherResult inlandminday(kFloatMissing, 0);
    WeatherResult inlandmaxday(kFloatMissing, 0);

    WeatherResult areameannight(kFloatMissing, 0);
    WeatherResult areaminnight(kFloatMissing, 0);
    WeatherResult areamaxnight(kFloatMissing, 0);

    WeatherResult coastmeannight(kFloatMissing, 0);
    WeatherResult coastminnight(kFloatMissing, 0);
    WeatherResult coastmaxnight(kFloatMissing, 0);

    WeatherResult inlandmeannight(kFloatMissing, 0);
    WeatherResult inlandminnight(kFloatMissing, 0);
    WeatherResult inlandmaxnight(kFloatMissing, 0);

    // Daytime results

    bool hasday = false;
    const int daynum = periodnum;
    if (generator.isday(periodnum))
    {
      hasday = true;

      WeatherPeriod period = generator.period(periodnum++);
      log << "Day: " << period.localStartTime() << " ... " << period.localEndTime() << endl;

      log << "Day results for area:" << endl;
      calculate_mean_min_max(log,
                             forecaster,
                             itsVar + "::fake::day" + Convert(part) + "::area",
                             itsSources,
                             itsArea,
                             period,
                             Maximum,
                             areameanday,
                             areaminday,
                             areamaxday);

      log << "Day results for coast:" << endl;
      calculate_mean_min_max(log,
                             forecaster,
                             itsVar + "::fake::day" + Convert(part) + "::coast",
                             itsSources,
                             coast,
                             period,
                             Maximum,
                             coastmeanday,
                             coastminday,
                             coastmaxday);

      log << "Day results for inland:" << endl;
      calculate_mean_min_max(log,
                             forecaster,
                             itsVar + "::fake::day" + Convert(part) + "::inland",
                             itsSources,
                             inland,
                             period,
                             Maximum,
                             inlandmeanday,
                             inlandminday,
                             inlandmaxday);
    }

    // Nighttime results

    bool hasnight = false;
    const int nightnum = periodnum;
    if (periodnum <= generator.size() && generator.isnight(periodnum))
    {
      hasnight = true;
      WeatherPeriod period = generator.period(periodnum++);

      log << "Night: " << period.localStartTime() << " ... " << period.localEndTime() << endl;

      log << "Night results for area:" << endl;
      calculate_mean_min_max(log,
                             forecaster,
                             itsVar + "::fake::night" + Convert(part) + "::area",
                             itsSources,
                             itsArea,
                             period,
                             Minimum,
                             areameannight,
                             areaminnight,
                             areamaxnight);

      log << "Night results for coast:" << endl;
      calculate_mean_min_max(log,
                             forecaster,
                             itsVar + "::fake::night" + Convert(part) + "::coast",
                             itsSources,
                             coast,
                             period,
                             Minimum,
                             coastmeannight,
                             coastminnight,
                             coastmaxnight);

      log << "Night results for inland:" << endl;
      calculate_mean_min_max(log,
                             forecaster,
                             itsVar + "::fake::night" + Convert(part) + "::inland",
                             itsSources,
                             inland,
                             period,
                             Minimum,
                             inlandmeannight,
                             inlandminnight,
                             inlandmaxnight);
    }

    // Rounded values

    const int aminday = static_cast<int>(round(areaminday.value()));
    const int ameanday = static_cast<int>(round(areameanday.value()));
    const int amaxday = static_cast<int>(round(areamaxday.value()));

    const int cminday = static_cast<int>(round(coastminday.value()));
    const int cmeanday = static_cast<int>(round(coastmeanday.value()));
    const int cmaxday = static_cast<int>(round(coastmaxday.value()));

    const int iminday = static_cast<int>(round(inlandminday.value()));
    const int imeanday = static_cast<int>(round(inlandmeanday.value()));
    const int imaxday = static_cast<int>(round(inlandmaxday.value()));

    const int aminnight = static_cast<int>(round(areaminnight.value()));
    const int ameannight = static_cast<int>(round(areameannight.value()));
    const int amaxnight = static_cast<int>(round(areamaxnight.value()));

    const int cminnight = static_cast<int>(round(coastminnight.value()));
    const int cmeannight = static_cast<int>(round(coastmeannight.value()));
    const int cmaxnight = static_cast<int>(round(coastmaxnight.value()));

    const int iminnight = static_cast<int>(round(inlandminnight.value()));
    const int imeannight = static_cast<int>(round(inlandmeannight.value()));
    const int imaxnight = static_cast<int>(round(inlandmaxnight.value()));

    const int bad = static_cast<int>(kFloatMissing);

    const bool hascoast =
        ((cmeanday != bad && imeanday != bad) || (cmeannight != bad && imeannight != bad));

    // See if we can report on day and night simultaneously
    // (when the difference is very small)

    const bool canmerge = (hasday && hasnight && abs(ameanday - ameannight) < day_night_limit);

    if (canmerge)
    {
      Sentence sentence;
      sentence << "lampotila"
               << "on" << PeriodPhraseFactory::create(
                              "today", itsVar, itsForecastTime, generator.period(daynum));
      if (!hascoast || ameanday < coast_not_below ||
          (abs(cmeanday - imeanday) < coast_limit && abs(cmeannight - imeannight) < coast_limit))
      {
        sentence << temperature_sentence(min(aminday, aminnight),
                                         static_cast<int>(round((ameanday + ameannight) / 2)),
                                         max(amaxday, amaxnight),
                                         mininterval,
                                         interval_zero,
                                         rangeseparator);
      }
      else
      {
        const int imean =
            static_cast<int>(round((inlandmeanday.value() + inlandmeannight.value()) / 2));
        const int cmean =
            static_cast<int>(round((coastmeanday.value() + coastmeannight.value()) / 2));
        sentence << temperature_sentence(min(iminday, iminnight),
                                         imean,
                                         max(imaxday, imaxnight),
                                         mininterval,
                                         interval_zero,
                                         rangeseparator)
                 << Delimiter(",") << "rannikolla"
                 << temperature_comparison_phrase(imean, cmean, itsVar);
      }
      paragraph << sentence;
    }

    // Report daytime

    if (hasday && !canmerge)
    {
      Sentence sentence;

      sentence << "paivan ylin lampotila"
               << "on" << PeriodPhraseFactory::create(
                              "today", itsVar, itsForecastTime, generator.period(daynum));

      // Do not report on the coast separately if there's no coast,
      // the mean temperature is very low (atleast freezing), or if the
      // difference between inland and the coast is small

      if (!hascoast || ameanday < coast_not_below || abs(cmeanday - imeanday) < coast_limit)
      {
        sentence << temperature_sentence(
            aminday, ameanday, amaxday, mininterval, interval_zero, rangeseparator);
      }
      else
      {
        sentence << temperature_sentence(
                        iminday, imeanday, imaxday, mininterval, interval_zero, rangeseparator)
                 << Delimiter(",") << "rannikolla";

        if (abs(imeanday - cmeanday) >= coast_numeric_limit)
          sentence << temperature_sentence(
              cminday, cmeanday, cmaxday, mininterval, interval_zero, rangeseparator);
        else
          sentence << temperature_comparison_phrase(imeanday, cmeanday, itsVar);
      }
      paragraph << sentence;
    }

    // Report nighttime

    if (hasnight && !canmerge)
    {
      Sentence sentence;

      sentence << "yon alin lampotila"
               << "on" << PeriodPhraseFactory::create(
                              "tonight", itsVar, itsForecastTime, generator.period(nightnum));

      // Do not report on the coast separately if there's no coast,
      // the mean temperature is very low (atleast freezing), or if the
      // difference between inland and the coast is small

      if (!hascoast || ameannight < coast_not_below || abs(cmeannight - imeannight) < coast_limit)
      {
        sentence << temperature_sentence(
            aminnight, ameannight, amaxnight, mininterval, interval_zero, rangeseparator);
      }
      else
      {
        sentence
            << temperature_sentence(
                   iminnight, imeannight, imaxnight, mininterval, interval_zero, rangeseparator)
            << Delimiter(",") << "rannikolla";

        if (abs(imeannight - cmeannight) >= coast_numeric_limit)
          sentence << temperature_sentence(
              cminnight, cmeannight, cmaxnight, mininterval, interval_zero, rangeseparator);
        else
          sentence << temperature_comparison_phrase(imeannight, cmeannight, itsVar);
      }
      paragraph << sentence;
    }
  }

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
