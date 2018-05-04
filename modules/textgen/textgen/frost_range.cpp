// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::FrostStory::range
 */
// ======================================================================

#include "FrostStory.h"
#include "FrostStoryTools.h"
#include <calculator/GridForecaster.h>
#include "IntegerRange.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherResult.h>

using namespace std;
using namespace TextGen;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on frost probability range
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph FrostStory::range() const
{
  MessageLogger log("FrostStory::range");

  Paragraph paragraph;

  if (!FrostStoryTools::is_frost_season())
  {
    log << "Frost season is not on";
    return paragraph;
  }

  using MathTools::to_precision;

  Sentence sentence;

  const string var1 = itsVar + "::precision";
  const string var2 = itsVar + "::severe_frost_limit";
  const string var3 = itsVar + "::frost_limit";

  const int precision = Settings::require_percentage(var1);
  const int severelimit = Settings::require_percentage(var2);
  const int normallimit = Settings::require_percentage(var3);

  const string rangeseparator = Settings::optional_string(itsVar + "::rangeseparator", "-");

  GridForecaster forecaster;

  WeatherResult maxfrost = forecaster.analyze(
      itsVar + "::fake::maximum", itsSources, Frost, Maximum, Maximum, itsArea, itsPeriod);

  if (maxfrost.value() == kFloatMissing) throw TextGenError("Maximum Frost is not available");

  // Quick exit if maximum frost probability is zero

  if (maxfrost.value() == 0)
  {
    log << paragraph;
    return paragraph;
  }

  // Minimum frost

  WeatherResult minfrost = forecaster.analyze(
      itsVar + "::fake::minimum", itsSources, Frost, Minimum, Maximum, itsArea, itsPeriod);

  if (minfrost.value() == kFloatMissing) throw TextGenError("Minimum Frost is not available");

  log << "Frost Maximum(Maximum) is " << maxfrost << endl;
  log << "Frost Minimum(Maximum) is " << minfrost << endl;

  // Maximum severe frost

  WeatherResult maxseverefrost = forecaster.analyze(itsVar + "::fake::severe_maximum",
                                                    itsSources,
                                                    SevereFrost,
                                                    Maximum,
                                                    Maximum,
                                                    itsArea,
                                                    itsPeriod);

  if (maxseverefrost.value() == kFloatMissing)
    throw TextGenError("Maximum SevereFrost is not available");

  WeatherResult minseverefrost = forecaster.analyze(itsVar + "::fake::severe_minimum",
                                                    itsSources,
                                                    SevereFrost,
                                                    Minimum,
                                                    Maximum,
                                                    itsArea,
                                                    itsPeriod);

  if (minseverefrost.value() == kFloatMissing)
    throw TextGenError("Minimum SevereFrost is not available");

  log << "SevereFrost Maximum(Maximum) is " << maxseverefrost << endl;
  log << "SevereFrost Minimum(Maximum) is " << minseverefrost << endl;

  const int frost_min = to_precision(minfrost.value(), precision);
  const int frost_max = to_precision(maxfrost.value(), precision);

  const int severe_frost_min = to_precision(minseverefrost.value(), precision);
  const int severe_frost_max = to_precision(maxseverefrost.value(), precision);

  if (severe_frost_max >= severelimit)
  {
    sentence << "ankaran hallan todennakoisyys"
             << "on" << IntegerRange(severe_frost_min, severe_frost_max, rangeseparator)
             << *UnitFactory::create(Percent);
    paragraph << sentence;
  }
  else if (frost_max >= normallimit)
  {
    sentence << "hallan todennakoisyys"
             << "on" << IntegerRange(frost_min, frost_max, rangeseparator)
             << *UnitFactory::create(Percent);
    paragraph << sentence;
  }

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
