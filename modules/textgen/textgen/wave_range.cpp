// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::WaveStory::range
 */
// ======================================================================

#include "WaveStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include "Real.h"
#include "RealRange.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherResult.h>

#include <boost/lexical_cast.hpp>

#include <vector>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Return wave_ranges story
 *
 * \return Paragraph containing the story
 */
// ----------------------------------------------------------------------

Paragraph WaveStory::range() const
{
  MessageLogger log("WaveStory::range");

  // Establish options

  using namespace Settings;

  // Generate the story

  Paragraph paragraph;

  GridForecaster forecaster;

  // Calculate wave speeds

  const WeatherResult minresult = forecaster.analyze(itsVar + "::fake::height::minimum",
                                                     itsSources,
                                                     WaveHeight,
                                                     Minimum,
                                                     Maximum,
                                                     itsArea,
                                                     itsPeriod);

  const WeatherResult maxresult = forecaster.analyze(itsVar + "::fake::height::maximum",
                                                     itsSources,
                                                     WaveHeight,
                                                     Maximum,
                                                     Maximum,
                                                     itsArea,
                                                     itsPeriod);

  const WeatherResult meanresult = forecaster.analyze(
      itsVar + "::fake::height::mean", itsSources, WaveHeight, Mean, Maximum, itsArea, itsPeriod);

  if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
      meanresult.value() == kFloatMissing)
  {
    throw TextGenError("Wave height not available for story wave_range");
  }

  log << "WaveHeight Minimum(Mean)  = " << minresult << endl;
  log << "WaveHeight Maximum(Mean)  = " << maxresult << endl;
  log << "WaveHeight Mean(Mean)     = " << meanresult << endl;

  const float mininterval = Settings::optional_double(itsVar + "::mininterval", 0.5);
  const string rangeseparator = Settings::optional_string(itsVar + "::rangeseparator", "-");

  // 0.5 meter accuracy is sufficient according to Niko Tollman
  float n = round(2 * meanresult.value()) / 2;
  float x = round(2 * minresult.value()) / 2;
  float y = round(2 * maxresult.value()) / 2;

  Sentence sentence;

  if (y <= 0.5)
  {
    sentence << "aallonkorkeus on alle [N] [metria]" << Real(0.5) << *UnitFactory::create(Meters);
  }
  else if (y - x < mininterval)
  {
    sentence << "aallonkorkeus on noin [N] [metria]" << Real(n) << *UnitFactory::create(Meters);
  }
  else if (x == 0)
  {
    sentence << "aallonkorkeus on enimmillaan [N] [metria]" << Real(y)
             << *UnitFactory::create(Meters);
  }
  else
  {
    sentence << "aallonkorkeus on [N...M] [metria]" << RealRange(x, y)
             << *UnitFactory::create(Meters);
  }

  paragraph << sentence;

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
