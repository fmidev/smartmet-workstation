// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::RelativeHumidityStory::day
 */
// ======================================================================

#include "RelativeHumidityStory.h"

#include <calculator/DefaultAcceptor.h>
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "Integer.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherPeriodTools.h>
#include <calculator/WeatherResult.h>
#include "PeriodPhraseFactory.h"

#include <boost/lexical_cast.hpp>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on lowest relative humidity for the day
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph RelativeHumidityStory::day() const
{
  MessageLogger log("RelativeHumidityStory::day");

  using MathTools::to_precision;

  Paragraph paragraph;

  HourPeriodGenerator generator(itsPeriod, itsVar + "::day");

  // Too late for this day? Return empty story then
  if (generator.size() == 0) return paragraph;

  // Too many days is an error

  if (generator.size() != 1)
    throw TextGenError("Cannot use relativehumidity_day story for periods longer than 1 day");

  // Exactly one period is fine

  WeatherPeriod period = generator.period(1);

  // The options

  const int precision = Settings::optional_percentage(itsVar + "::precision", 10);
  const int coastlimit = Settings::optional_percentage(itsVar + "::coast_limit", 30);

  GridForecaster forecaster;

  // Result for the area

  WeatherResult arearesult = forecaster.analyze(itsVar + "::fake::area::minimum",
                                                itsSources,
                                                RelativeHumidity,
                                                Mean,
                                                Minimum,
                                                itsArea,
                                                period);

  if (arearesult.value() == kFloatMissing) throw TextGenError("RelativeHumidity not available");

  log << "RelativeHumidity Mean(Minimum) for area " << arearesult << endl;

  const int areahumidity = to_precision(arearesult.value(), precision);

  // The beginning of the story is always the same

  Sentence sentence;
  sentence << "alin suhteellinen kosteus"
           << "on" << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, period);

  // Possible coastal result

  WeatherArea coast = itsArea;
  coast.type(WeatherArea::Coast);

  WeatherResult coastresult = forecaster.analyze(itsVar + "::fake::coast::minimum",
                                                 itsSources,
                                                 RelativeHumidity,
                                                 Mean,
                                                 Minimum,
                                                 coast,
                                                 period);

  // Done if there is no coast

  if (coastresult.value() == kFloatMissing)
  {
    sentence << Integer(areahumidity) << *UnitFactory::create(Percent);
  }
  else
  {
    // Possible inland result

    WeatherArea inland = itsArea;
    inland.type(WeatherArea::Inland);

    WeatherResult inlandresult = forecaster.analyze(itsVar + "::fake::inland::minimum",
                                                    itsSources,
                                                    RelativeHumidity,
                                                    Mean,
                                                    Minimum,
                                                    inland,
                                                    period);

    // Done if there is no inland (area is fully coastal)

    if (inlandresult.value() == kFloatMissing)
    {
      sentence << Integer(areahumidity) << *UnitFactory::create(Percent);
    }
    else
    {
      log << "RelativeHumidity Mean(Minimum) for coast " << coastresult << endl;
      log << "RelativeHumidity Mean(Minimum) for inland " << inlandresult << endl;

      const int coasthumidity = to_precision(coastresult.value(), precision);
      const int inlandhumidity = to_precision(inlandresult.value(), precision);

      // Choose the story based on the difference

      if (abs(inlandhumidity - coasthumidity) < coastlimit)
      {
        sentence << Integer(areahumidity) << *UnitFactory::create(Percent);
      }
      else
      {
        sentence << Integer(inlandhumidity) << *UnitFactory::create(Percent) << Delimiter(",")
                 << "rannikolla" << Integer(coasthumidity) << *UnitFactory::create(Percent);
      }
    }
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
