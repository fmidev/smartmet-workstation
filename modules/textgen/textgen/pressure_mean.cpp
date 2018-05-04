// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::PressureStory::range
 */
// ======================================================================

#include "PressureStory.h"
#include <calculator/GridForecaster.h>
#include "Integer.h"
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
 * \brief Generate story on pressure mean
 *
 * \return The story
 *
 * \see page_pressure_range
 */
// ----------------------------------------------------------------------

const Paragraph PressureStory::mean() const
{
  MessageLogger log("PressureStory::range");

  using namespace Settings;

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  WeatherResult meanresult = forecaster.analyze(
      itsVar + "::fake::mean", itsSources, Pressure, Mean, Mean, itsArea, itsPeriod);

  if (meanresult.value() == kFloatMissing)
    throw TextGenError("Pressure is not available for pressure_mean");

  log << "Pressure Mean(Mean(Maximum())) = " << meanresult << endl;

  const int pmean = static_cast<int>(round(meanresult.value()));

  sentence << "paine"
           << "on" << Integer(pmean) << *UnitFactory::create(HectoPascal);

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
