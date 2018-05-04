// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::RelativeHumidityStory::range
 */
// ======================================================================

#include "RelativeHumidityStory.h"
#include <calculator/GridForecaster.h>
#include "Integer.h"
#include "IntegerRange.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
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

Paragraph RelativeHumidityStory::range() const
{
  MessageLogger log("RelativeHumidityStory::range");

  using namespace Settings;

  const int precision = optional_percentage(itsVar + "::precision", 10);
  const string rangeseparator = optional_string(itsVar + "::rangeseparator", "-");

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  WeatherResult minresult = forecaster.analyze(
      itsVar + "::fake::minimum", itsSources, RelativeHumidity, Mean, Minimum, itsArea, itsPeriod);

  WeatherResult maxresult = forecaster.analyze(
      itsVar + "::fake::maximum", itsSources, RelativeHumidity, Mean, Maximum, itsArea, itsPeriod);

  WeatherResult meanresult = forecaster.analyze(
      itsVar + "::fake::mean", itsSources, RelativeHumidity, Mean, Mean, itsArea, itsPeriod);

  if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
      meanresult.value() == kFloatMissing)
    throw TextGenError("RelativeHumidity is not available for relativehumidity_range");

  log << "RelativeHumidity Mean(Min(Maximum())) = " << minresult << endl
      << "RelativeHumidity Mean(Mean(Maximum())) = " << meanresult << endl
      << "RelativeHumidity Mean(Max(Maximum())) = " << maxresult << endl;

  const int rhmin = MathTools::to_precision(minresult.value(), precision);
  const int rhmax = MathTools::to_precision(maxresult.value(), precision);
  const int rhmean = MathTools::to_precision(meanresult.value(), precision);

  if (rhmin == rhmax)
  {
    sentence << "suhteellinen kosteus"
             << "on" << Integer(rhmean) << *UnitFactory::create(Percent);
  }
  else
  {
    sentence << "suhteellinen kosteus"
             << "on" << IntegerRange(rhmin, rhmax, rangeseparator) << *UnitFactory::create(Percent);
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
