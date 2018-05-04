// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::ForestStory::forestfireindex_twodays
 */
// ======================================================================

#include "ForestStory.h"
#include <calculator/DefaultAcceptor.h>
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "Real.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include <calculator/NullPeriodGenerator.h>
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
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
 * \brief Generate story on forest fire index
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph ForestStory::forestfireindex_twodays() const
{
  MessageLogger log("ForestStory::forestfireindex_twodays");

  using MathTools::to_precision;

  Paragraph paragraph;

  HourPeriodGenerator generator(itsPeriod, itsVar + "::day");

  const int days = generator.size();

  if (days <= 0)
  {
    log << paragraph;
    return paragraph;
  }

  WeatherPeriod firstperiod = generator.period(1);

  GridForecaster forecaster;

  WeatherResult result = forecaster.analyze(itsVar + "::fake::day1::maximum",
                                            itsSources,
                                            ForestFireIndex,
                                            Maximum,
                                            Maximum,
                                            itsArea,
                                            firstperiod);

  if (result.value() == kFloatMissing)
  {
    log << "ForestFireIndex Maximum(Maximum) for day 1 is missing" << endl;
    return paragraph;
  }

  log << "ForestFirewarning Maximum(Maximum) for day 1 " << result << endl;

  const float index1 = result.value();

  Sentence sentence;

  sentence << "metsapaloindeksi"
           << "on" << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, firstperiod)
           << Real(index1);

  if (days >= 2)
  {
    WeatherPeriod secondperiod = generator.period(2);

    WeatherResult result2 = forecaster.analyze(itsVar + "::fake::day2::maximum",
                                               itsSources,
                                               ForestFireIndex,
                                               Maximum,
                                               Maximum,
                                               itsArea,
                                               secondperiod);

    // Note: The model may not reach the second day. Hence a missing
    // value is not an error, we simply omit a story if day2 is missing
    if (result2.value() == kFloatMissing)
    {
      log << "ForestFireIndex Maximum(Maximum) for day 2 is missing" << endl;
    }
    else
    {
      log << "ForestFireIndex Maximum(Maximum) for day 2 " << result2 << endl;

      const float index2 = result2.value();

      if (sentence.empty())
      {
        sentence << "metsapaloindeksi"
                 << "on"
                 << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, secondperiod)
                 << Real(index2);
      }
      else
      {
        sentence << Delimiter(",")
                 << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, secondperiod);

        if (index1 == index2)
          sentence << "sama";
        else
          sentence << Real(index2);
      }
    }
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
