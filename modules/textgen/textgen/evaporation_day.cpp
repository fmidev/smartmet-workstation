// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::ForestStory::evaporation_day
 */
// ======================================================================

#include "ForestStory.h"
#include "Delimiter.h"
#include "Real.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherResult.h>

using namespace std;
using namespace TextGen;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on one day evaporation
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph ForestStory::evaporation_day() const
{
  MessageLogger log("ForestStory::evaporation_day");

  Paragraph paragraph;

  HourPeriodGenerator generator(itsPeriod, itsVar + "::day");

  // Too late for this day? Return empty story then

  if (generator.size() == 0) return paragraph;

  // Too many days is an error

  if (generator.size() != 1)
    throw TextGenError("Cannot use evaporationt_day story for periods longer than 1 day");

  // Exactly one period is fine

  WeatherPeriod period = generator.period(1);

  // Calculate mean evaporation for the area

  GridForecaster forecaster;

  WeatherResult evaporation = forecaster.analyze(
      itsVar + "::fake::area::mean", itsSources, Evaporation, Mean, Sum, itsArea, period);

  if (evaporation.value() == kFloatMissing)
  {
    log << "No forecast available, returning empty story" << endl;
    return paragraph;
  }

  log << "Evaporation Mean(Sum) for area " << evaporation << endl;

  // Generate the story

  Sentence sentence;

  float eva = evaporation.value();

  if (eva < 1.5)
    sentence << "haihdunta on erittain huono";
  else if (eva < 3)
    sentence << "haihdunta on huono";
  else if (eva < 5)
    sentence << "haihdunta on tyydyttava";
  else if (eva < 7.5)
    sentence << "haihdunta on hyva";
  else
    sentence << "haihdunta on erinomainen";

  sentence << Delimiter(",") << Real(eva) << *UnitFactory::create(Millimeters);

  paragraph << sentence;

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
