// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::TemperatureStory::meanmax
 */
// ======================================================================

#include "TemperatureStory.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
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
 * \brief Generate story on mean maximum temperature
 *
 * \return The story
 *
 * \see page_temperature_meanmax
 */
// ----------------------------------------------------------------------

Paragraph TemperatureStory::meanmax() const
{
  MessageLogger log("TemperatureStory::meanmax");

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  HourPeriodGenerator periods(itsPeriod, 06, 18, 06, 18);

  WeatherResult result = forecaster.analyze(
      itsVar + "::fake::mean", itsSources, Temperature, Mean, Mean, Maximum, itsArea, periods);

  if (result.value() == kFloatMissing)
    throw TextGenError("Mean daily maximum temperature not available");

  log << "Temperature Mean(Mean(Maximum())) = " << result << endl;

  sentence << "keskimaarainen ylin lampotila" << Integer(static_cast<int>(round(result.value())))
           << *UnitFactory::create(DegreesCelsius);
  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
