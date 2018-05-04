// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::PrecipitationStory::total
 */
// ======================================================================

#include "PrecipitationStory.h"
#include <calculator/GridForecaster.h>
#include "Integer.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include <calculator/RangeAcceptor.h>
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
 * \brief Generate story on mean total precipitation
 *
 * Sample story: "Sadesumma 10 millimetria."
 *
 * \return The story
 */
// ----------------------------------------------------------------------

Paragraph PrecipitationStory::total() const
{
  MessageLogger log("PrecipitationStory::total");

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  RangeAcceptor rainlimits;
  rainlimits.lowerLimit(Settings::optional_double(itsVar + "::minrain", 0));

  WeatherResult result = forecaster.analyze(itsVar + "::fake::mean",
                                            itsSources,
                                            Precipitation,
                                            Mean,
                                            Sum,
                                            itsArea,
                                            itsPeriod,
                                            DefaultAcceptor(),
                                            rainlimits);

  if (result.value() == kFloatMissing) throw TextGenError("Total precipitation not available");

  log << "Precipitation Mean(Sum) " << result << endl;

  sentence << "sadesumma" << Integer(static_cast<int>(round(result.value())))
           << *UnitFactory::create(Millimeters);
  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
