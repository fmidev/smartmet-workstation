// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::FrostStory::mean
 */
// ======================================================================

#include "FrostStory.h"
#include "FrostStoryTools.h"
#include <calculator/GridForecaster.h>
#include "Integer.h"
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
 * \brief Generate story on mean frost
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph FrostStory::mean() const
{
  MessageLogger log("FrostStory::mean");

  using MathTools::to_precision;

  Paragraph paragraph;

  if (!FrostStoryTools::is_frost_season())
  {
    log << "Frost season is not on";
    return paragraph;
  }

  Sentence sentence;

  const string var1 = itsVar + "::precision";
  const string var2 = itsVar + "::severe_frost_limit";
  const string var3 = itsVar + "::frost_limit";

  const int precision = Settings::require_percentage(var1);
  const int severelimit = Settings::require_percentage(var2);
  const int normallimit = Settings::require_percentage(var3);

  GridForecaster forecaster;

  WeatherResult frost = forecaster.analyze(
      itsVar + "::fake::mean", itsSources, Frost, Mean, Maximum, itsArea, itsPeriod);

  if (frost.value() == kFloatMissing) throw TextGenError("Frost is not available");

  log << "Frost Mean(Maximum) is " << frost << endl;

  // Quick exit if the mean is zero

  if (frost.value() == 0)
  {
    log << paragraph;
    return paragraph;
  }

  // Severe frost

  WeatherResult severefrost = forecaster.analyze(
      itsVar + "::fake::severe_mean", itsSources, SevereFrost, Mean, Maximum, itsArea, itsPeriod);

  log << "SevereFrost Mean(Maximum) is " << severefrost << endl;

  if (severefrost.value() == kFloatMissing) throw TextGenError("SevereFrost is not available");

  const int frost_value = to_precision(frost.value(), precision);

  const int severe_frost_value = to_precision(severefrost.value(), precision);

  if (severe_frost_value >= severelimit)
  {
    sentence << "ankaran hallan todennakoisyys"
             << "on" << Integer(severe_frost_value) << *UnitFactory::create(Percent);
    paragraph << sentence;
  }
  else if (frost_value >= normallimit)
  {
    sentence << "hallan todennakoisyys"
             << "on" << Integer(frost_value) << *UnitFactory::create(Percent);
    paragraph << sentence;
  }

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
