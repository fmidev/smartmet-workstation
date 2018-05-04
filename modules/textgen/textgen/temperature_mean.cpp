// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::TemperatureStory::mean
 */
// ======================================================================

#include "TemperatureStory.h"
#include <calculator/GridForecaster.h>
#include "Integer.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherResult.h>

using namespace TextGen;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on mean temperature
 *
 * \return The generated paragraph
 *
 * \todo Is throwing the best way to handle missing results?
 *
 * \see page_temperature_mean
 */
// ----------------------------------------------------------------------

Paragraph TemperatureStory::mean() const
{
  MessageLogger log("TemperatureStory::mean");

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  WeatherResult result = forecaster.analyze(
      itsVar + "::fake::mean", itsSources, Temperature, Mean, Mean, itsArea, itsPeriod);

  if (result.value() == kFloatMissing) throw TextGenError("Mean temperature not available");

  log << "Temperature Mean(Mean) = " << result << endl;

  sentence << "keskilampotila" << Integer(static_cast<int>(round(result.value())))
           << *UnitFactory::create(DegreesCelsius);

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
