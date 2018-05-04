// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::TemperatureStory::weekly_averages
 */
// ======================================================================

#include "TemperatureStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "TemperatureStoryTools.h"
#include <calculator/TextGenError.h>
#include <calculator/WeatherResult.h>

using namespace TextGen::TemperatureStoryTools;
using namespace TextGen;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on several day mean day/night temperatures
 *
 * \return The story
 *
 * \see page_temperature_weekly_averages
 *
 */
// ----------------------------------------------------------------------

Paragraph TemperatureStory::weekly_averages() const
{
  MessageLogger log("TemperatureStory::weekly_averages");

  using namespace Settings;

  Paragraph paragraph;

  const int daymininterval = optional_int(itsVar + "::day::mininterval", 2);
  const int nightmininterval = optional_int(itsVar + "::night::mininterval", 2);
  const bool day_interval_zero = optional_bool(itsVar + "::day::always_interval_zero", false);
  const bool night_interval_zero = optional_bool(itsVar + "::night::always_interval_zero", false);

  const string rangeseparator = optional_string(itsVar + "::rangeseparator", "...");

  const HourPeriodGenerator days(itsPeriod, itsVar + "::day");
  const HourPeriodGenerator nights(itsPeriod, itsVar + "::night");

  GridForecaster forecaster;

  const WeatherResult dayminresult = forecaster.analyze(
      itsVar + "::fake::day::minimum", itsSources, Temperature, Minimum, Mean, Mean, itsArea, days);

  const WeatherResult daymaxresult = forecaster.analyze(
      itsVar + "::fake::day::maximum", itsSources, Temperature, Maximum, Mean, Mean, itsArea, days);

  const WeatherResult daymeanresult = forecaster.analyze(
      itsVar + "::fake::day::mean", itsSources, Temperature, Mean, Mean, Mean, itsArea, days);

  const WeatherResult nightminresult = forecaster.analyze(itsVar + "::fake::night::minimum",
                                                          itsSources,
                                                          Temperature,
                                                          Minimum,
                                                          Mean,
                                                          Mean,
                                                          itsArea,
                                                          nights);

  const WeatherResult nightmaxresult = forecaster.analyze(itsVar + "::fake::night::maximum",
                                                          itsSources,
                                                          Temperature,
                                                          Maximum,
                                                          Mean,
                                                          Mean,
                                                          itsArea,
                                                          nights);

  const WeatherResult nightmeanresult = forecaster.analyze(
      itsVar + "::fake::night::mean", itsSources, Temperature, Mean, Mean, Mean, itsArea, nights);

  if (dayminresult.value() == kFloatMissing || daymaxresult.value() == kFloatMissing ||
      daymeanresult.value() == kFloatMissing || nightminresult.value() == kFloatMissing ||
      nightmaxresult.value() == kFloatMissing || nightmeanresult.value() == kFloatMissing)
    throw TextGenError("Daily maximum and nightly minimum not available");

  log << "Temperature Minimum(Mean(Maximum)) = " << dayminresult << endl;
  log << "Temperature Mean(Mean(Maximum)) = " << daymeanresult << endl;
  log << "Temperature Maximum(Mean(Maximum)) = " << daymaxresult << endl;

  log << "Temperature Minimum(Mean(Minimum)) = " << nightminresult << endl;
  log << "Temperature Mean(Mean(Minimum)) = " << nightmeanresult << endl;
  log << "Temperature Maximum(Mean(Minimum)) = " << nightmaxresult << endl;

  const int daymin = static_cast<int>(round(dayminresult.value()));
  const int daymax = static_cast<int>(round(daymaxresult.value()));
  const int daymean = static_cast<int>(round(daymeanresult.value()));
  const int nightmin = static_cast<int>(round(nightminresult.value()));
  const int nightmax = static_cast<int>(round(nightmaxresult.value()));
  const int nightmean = static_cast<int>(round(nightmeanresult.value()));

  Sentence sentence;
  sentence
      << "paivalampotila"
      << "on" << temperature_sentence(
                     daymin, daymean, daymax, daymininterval, day_interval_zero, rangeseparator)
      << Delimiter(",") << "yolampotila"
      << temperature_sentence(
             nightmin, nightmean, nightmax, nightmininterval, night_interval_zero, rangeseparator);

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
