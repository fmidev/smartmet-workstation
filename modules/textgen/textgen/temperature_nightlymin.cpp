// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::TemperatureStory::nightlymin
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
 * \brief Generate story on nightly minimum temperature
 *
 * \return The story
 *
 * \see page_temperature_nightlymin
 */
// ----------------------------------------------------------------------

Paragraph TemperatureStory::nightlymin() const
{
  MessageLogger log("TemperatureStory::nightlymin");

  using namespace Settings;
  using namespace WeatherPeriodTools;

  Paragraph paragraph;

  const int starthour = require_hour(itsVar + "::night::starthour");
  const int endhour = require_hour(itsVar + "::night::endhour");
  const int maxstarthour = optional_hour(itsVar + "::night::maxstarthour", starthour);
  const int minendhour = optional_hour(itsVar + "::night::minendhour", endhour);

  const int mininterval = optional_int(itsVar + "::mininterval", 2);
  const bool interval_zero = optional_bool(itsVar + "::always_interval_zero", false);

  const string rangeseparator = optional_string(itsVar + "::rangeseparator", "...");

  const int nights = countPeriods(itsPeriod, starthour, endhour, maxstarthour, minendhour);

  if (nights == 0)
  {
    log << "No night periods!" << endl;
    return paragraph;
  }

  WeatherPeriod period = getPeriod(itsPeriod, 1, starthour, endhour, maxstarthour, minendhour);

  GridForecaster forecaster;

  WeatherResult minresult = forecaster.analyze(itsVar + "::fake::night1::minimum",
                                               itsSources,
                                               Temperature,
                                               Minimum,
                                               Minimum,
                                               itsArea,
                                               period);

  WeatherResult meanresult = forecaster.analyze(
      itsVar + "::fake::night1::mean", itsSources, Temperature, Mean, Minimum, itsArea, period);

  WeatherResult maxresult = forecaster.analyze(itsVar + "::fake::night1::maximum",
                                               itsSources,
                                               Temperature,
                                               Maximum,
                                               Minimum,
                                               itsArea,
                                               period);

  if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
      meanresult.value() == kFloatMissing)
    throw TextGenError("TemperatureStory: MinTemperature is not available");

  log << "Temperature Minimum(Minimum) night 1 = " << minresult << endl;
  log << "Temperature Mean(Minimum) night 1 = " << meanresult << endl;
  log << "Temperature Maximum(Minimum) night 1 = " << maxresult << endl;

  const int min1 = static_cast<int>(round(minresult.value()));
  const int max1 = static_cast<int>(round(maxresult.value()));
  const int mean1 = static_cast<int>(round(meanresult.value()));

  Sentence sentence;
  sentence << "yon alin lampotila"
           << "on" << PeriodPhraseFactory::create("tonight", itsVar, itsForecastTime, period)
           << temperature_sentence(min1, mean1, max1, mininterval, interval_zero, rangeseparator);

  // Remaining nights

  for (int p = 2; p <= nights; p++)
  {
    period = getPeriod(itsPeriod, p, starthour, endhour, maxstarthour, minendhour);

    const string var = (itsVar + "::fake::night" + lexical_cast<string>(p));

    minresult = forecaster.analyze(
        var + "::minimum", itsSources, Temperature, Minimum, Minimum, itsArea, period);

    maxresult = forecaster.analyze(
        var + "::maximum", itsSources, Temperature, Maximum, Minimum, itsArea, period);

    meanresult =
        forecaster.analyze(var + "::mean", itsSources, Temperature, Mean, Minimum, itsArea, period);

    if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
        meanresult.value() == kFloatMissing)
      throw TextGenError("TemperatureStory: MinTemperature is not available for night " +
                         lexical_cast<string>(p));

    log << "Temperature Minimum(Minimum) night " << p << " = " << minresult << endl;
    log << "Temperature Mean(Minimum) night " << p << " = " << meanresult << endl;
    log << "Temperature Maximum(Minimum) night " << p << " = " << maxresult << endl;

    const int min2 = static_cast<int>(round(minresult.value()));
    const int max2 = static_cast<int>(round(maxresult.value()));
    const int mean2 = static_cast<int>(round(meanresult.value()));

    // For second night:
    //
    // "seuraavana yona [komparatiivi]" tai
    // "[viikonpaivan vastaisena yona] [komparatiivi]"
    //
    // For third and so on
    //
    // "[viikonpaivan vastaisena yona] [noin x|x...y] astetta"

    sentence << Delimiter(",");

    if (p == 2)
    {
      sentence << PeriodPhraseFactory::create("next_night", itsVar, itsForecastTime, period);
      sentence << temperature_comparison_phrase(mean1, mean2, itsVar);
    }
    else
    {
      sentence << WeekdayTools::night_against_weekday(period.localEndTime())
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
