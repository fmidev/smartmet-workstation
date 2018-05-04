// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::PrecipitationStory::range
 */
// ======================================================================

#include "PrecipitationStory.h"
#include <calculator/GridForecaster.h>
#include "Integer.h"
#include "IntegerRange.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include <calculator/RangeAcceptor.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherResult.h>

using namespace boost;
using namespace std;
using namespace TextGen;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on total precipitation range
 *
 * Sample story 1: "Sadesumma yli LIMIT millimetria."	(when min>=LIMIT)
 * Sample story 2: "Sadesumma 4 millimetria."			(when min==max)
 * Sample story 3: "Sadesumma 1-5 millimetria."		(otherwise)
 *
 * where LIMIT is the value of textgen::precipitation_range::maxrain
 *
 * \return The story
 */
// ----------------------------------------------------------------------

Paragraph PrecipitationStory::range() const
{
  MessageLogger log("PrecipitationStory::range");

  const string rangeseparator = Settings::optional_string(itsVar + "::rangeseparator", "-");

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  RangeAcceptor rainlimits;
  rainlimits.lowerLimit(Settings::optional_double(itsVar + "::minrain", 0));

  WeatherResult minresult = forecaster.analyze(itsVar + "::fake::minimum",
                                               itsSources,
                                               Precipitation,
                                               Minimum,
                                               Sum,
                                               itsArea,
                                               itsPeriod,
                                               DefaultAcceptor(),
                                               rainlimits);

  WeatherResult maxresult = forecaster.analyze(itsVar + "::fake::maximum",
                                               itsSources,
                                               Precipitation,
                                               Maximum,
                                               Sum,
                                               itsArea,
                                               itsPeriod,
                                               DefaultAcceptor(),
                                               rainlimits);

  if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing)
    throw TextGenError("Total precipitation not available");

  log << "Precipitation Minimum(Sum) " << minresult << endl;
  log << "Precipitation Maximum(Sum) " << maxresult << endl;

  const int minrain = static_cast<int>(round(minresult.value()));
  const int maxrain = static_cast<int>(round(maxresult.value()));

  // optionaalinen maksimisade

  const string variable = itsVar + "::maxrain";
  const int rainlimit = Settings::optional_int(variable, -1);

  if (minrain >= rainlimit && rainlimit > 0)
  {
    sentence << "sadesumma"
             << "on"
             << "yli" << Integer(rainlimit) << *UnitFactory::create(Millimeters);
  }
  else
  {
    sentence << "sadesumma"
             << "on" << IntegerRange(minrain, maxrain, rangeseparator)
             << *UnitFactory::create(Millimeters);
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
