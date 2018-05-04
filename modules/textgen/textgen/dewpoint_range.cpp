// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::DewPointStory::range
 */
// ======================================================================

#include "DewPointStory.h"
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
 * \brief Generate story on dewpoint min/max
 *
 * \return The story
 *
 * \see page_dewpoint_range
 */
// ----------------------------------------------------------------------

Paragraph DewPointStory::range() const
{
  MessageLogger log("DewPointStory::range");

  using namespace Settings;

  const int mininterval = optional_int(itsVar + "::mininterval", 2);
  const string rangeseparator = optional_string(itsVar + "::rangeseparator", "...");

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  WeatherResult minresult = forecaster.analyze(
      itsVar + "::fake::minimum", itsSources, DewPoint, Mean, Minimum, itsArea, itsPeriod);

  WeatherResult maxresult = forecaster.analyze(
      itsVar + "::fake::maximum", itsSources, DewPoint, Mean, Maximum, itsArea, itsPeriod);

  WeatherResult meanresult = forecaster.analyze(
      itsVar + "::fake::mean", itsSources, DewPoint, Mean, Mean, itsArea, itsPeriod);

  if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
      meanresult.value() == kFloatMissing)
    throw TextGenError("DewPoint is not available for dewpoint_range");

  log << "DewPoint Mean(Min(Maximum())) = " << minresult << endl
      << "DewPoint Mean(Mean(Maximum())) = " << meanresult << endl
      << "DewPoint Mean(Max(Maximum())) = " << maxresult << endl;

  const int tmin = static_cast<int>(round(minresult.value()));
  const int tmax = static_cast<int>(round(maxresult.value()));
  const int tmean = static_cast<int>(round(meanresult.value()));

  sentence << "kastepiste"
           << "on" << TemperatureStoryTools::temperature_sentence(
                          tmin, tmean, tmax, mininterval, false, rangeseparator);

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
