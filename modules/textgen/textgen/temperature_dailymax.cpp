// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::TemperatureStory::dailymax
 */
// ======================================================================

#include "TemperatureStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "PeriodPhraseFactory.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "TemperatureStoryTools.h"
#include <calculator/TextGenError.h>
#include <calculator/WeatherPeriodTools.h>
#include <calculator/WeatherResult.h>
#include "WeekdayTools.h"

#include <boost/lexical_cast.hpp>

using namespace TextGen::TemperatureStoryTools;
using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on daily maximum temperature
 *
 * \return The story
 *
 * \see page_temperature_dailymax
 */
// ----------------------------------------------------------------------

Paragraph TemperatureStory::dailymax() const
{
  MessageLogger log("TemperatureStory::dailymax");

  using namespace Settings;
  using namespace WeatherPeriodTools;

  Paragraph paragraph;

  const int starthour = require_hour(itsVar + "::day::starthour");
  const int endhour = require_hour(itsVar + "::day::endhour");
  const int maxstarthour = optional_hour(itsVar + "::day::maxstarthour", starthour);
  const int minendhour = optional_hour(itsVar + "::day::minendhour", endhour);

  const int mininterval = optional_int(itsVar + "::mininterval", 2);
  const bool interval_zero = optional_bool(itsVar + "::always_interval_zero", false);

  const string rangeseparator = optional_string(itsVar + "::rangeseparator", "...");

  const int days = countPeriods(itsPeriod, starthour, endhour, maxstarthour, minendhour);

  WeatherPeriod period = getPeriod(itsPeriod, 1, starthour, endhour, maxstarthour, minendhour);

  GridForecaster forecaster;

  WeatherResult minresult = forecaster.analyze(
      itsVar + "::fake::day1::minimum", itsSources, Temperature, Minimum, Maximum, itsArea, period);

  WeatherResult meanresult = forecaster.analyze(
      itsVar + "::fake::day1::mean", itsSources, Temperature, Mean, Maximum, itsArea, period);

  WeatherResult maxresult = forecaster.analyze(
      itsVar + "::fake::day1::maximum", itsSources, Temperature, Maximum, Maximum, itsArea, period);

  if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
      meanresult.value() == kFloatMissing)
    throw TextGenError("TemperatureStory: MaxTemperature is not available");

  log << "Temperature Minimum(Maximum) day 1 = " << minresult << endl;
  log << "Temperature Mean(Maximum) day 1 = " << meanresult << endl;
  log << "Temperature Maximum(Maximum) day 1 = " << maxresult << endl;

  const int min1 = static_cast<int>(round(minresult.value()));
  const int max1 = static_cast<int>(round(maxresult.value()));
  const int mean1 = static_cast<int>(round(meanresult.value()));

  Sentence sentence;
  sentence << "paivan ylin lampotila"
           << "on" << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, period)
           << temperature_sentence(min1, mean1, max1, mininterval, interval_zero, rangeseparator);

  // Remaining days

  for (int p = 2; p <= days; p++)
  {
    period = getPeriod(itsPeriod, p, starthour, endhour, maxstarthour, minendhour);

    const string var = (itsVar + "::fake::day" + lexical_cast<string>(p));

    minresult = forecaster.analyze(
        var + "::minimum", itsSources, Temperature, Minimum, Maximum, itsArea, period);

    maxresult = forecaster.analyze(
        var + "::maximum", itsSources, Temperature, Maximum, Maximum, itsArea, period);

    meanresult =
        forecaster.analyze(var + "::mean", itsSources, Temperature, Mean, Maximum, itsArea, period);

    if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
        meanresult.value() == kFloatMissing)
      throw TextGenError("TemperatureStory: MaxTemperature is not available for day " +
                         lexical_cast<string>(p));

    log << "Temperature Minimum(Maximum) day " << p << " = " << minresult << endl;
    log << "Temperature Mean(Maximum) day " << p << " = " << meanresult << endl;
    log << "Temperature Maximum(Maximum) day " << p << " = " << maxresult << endl;

    const int min2 = static_cast<int>(round(minresult.value()));
    const int max2 = static_cast<int>(round(maxresult.value()));
    const int mean2 = static_cast<int>(round(meanresult.value()));

    // For second day:
    //
    // "seuraavana paivana [komparatiivi]" tai
    // "[viikonpaivana] [komparatiivi]"
    //
    // For third and so on
    //
    // "[viikonpaivana] [noin x|x...y] astetta"

    sentence << Delimiter(",");

    if (p == 2)
    {
      sentence << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, period);
      sentence << temperature_comparison_phrase(mean1, mean2, itsVar);
    }
    else
    {
      sentence << WeekdayTools::on_weekday(period.localStartTime())
               << temperature_sentence(
                      min2, mean2, max2, mininterval, interval_zero, rangeseparator);
    }
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
