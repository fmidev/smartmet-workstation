// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::TemperatureStory::range
 */
// ======================================================================

#include "TemperatureStory.h"
#include <calculator/GridForecaster.h>
#include "Integer.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "TemperatureStoryTools.h"
#include <calculator/WeatherResult.h>

using namespace TextGen;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on temperature min/max
 *
 * \return The story
 *
 * \see page_temperature_range
 */
// ----------------------------------------------------------------------

Paragraph TemperatureStory::range() const
{
  MessageLogger log("TemperatureStory::range");

  using namespace Settings;

  const int mininterval = optional_int(itsVar + "::mininterval", 2);
  const bool interval_zero = optional_bool(itsVar + "::always_interval_zero", false);
  const string rangeseparator = optional_string(itsVar + "::rangeseparator", "...");

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  WeatherResult minresult = forecaster.analyze(
      itsVar + "::fake::minimum", itsSources, Temperature, Mean, Minimum, itsArea, itsPeriod);

  WeatherResult maxresult = forecaster.analyze(
      itsVar + "::fake::maximum", itsSources, Temperature, Mean, Maximum, itsArea, itsPeriod);

  WeatherResult meanresult = forecaster.analyze(
      itsVar + "::fake::mean", itsSources, Temperature, Mean, Mean, itsArea, itsPeriod);

  if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
      meanresult.value() == kFloatMissing)
    throw TextGenError("Temperature is not available for temperature_range");

  log << "Temperature Mean(Min(Maximum())) = " << minresult << endl
      << "Temperature Mean(Mean(Maximum())) = " << meanresult << endl
      << "Temperature Mean(Max(Maximum())) = " << maxresult << endl;

  const int tmin = static_cast<int>(round(minresult.value()));
  const int tmax = static_cast<int>(round(maxresult.value()));
  const int tmean = static_cast<int>(round(meanresult.value()));

  sentence << "lampotila"
           << "on" << TemperatureStoryTools::temperature_sentence(
                          tmin, tmean, tmax, mininterval, interval_zero, rangeseparator);

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
