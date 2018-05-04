// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::RoadStory::shortrange
 */
// ======================================================================

#include "RoadStory.h"
#include <calculator/HourPeriodGenerator.h>
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "PeriodPhraseFactory.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "TemperatureStoryTools.h"
#include <calculator/WeatherResult.h>

#include <boost/lexical_cast.hpp>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on road temperature variations
 *
 * \return The story
 *
 * \see page_roadtemperature_shortrange
 */
// ----------------------------------------------------------------------

Paragraph RoadStory::shortrange() const
{
  MessageLogger log("RoadStory::shortrange");

  using namespace Settings;

  const unsigned int maxperiods = optional_int(itsVar + "::maxperiods", 3);
  const unsigned int mininterval = optional_int(itsVar + "::mininterval", 2);
  const bool interval_zero = optional_bool(itsVar + "::always_interval_zero", false);
  const string rangeseparator = optional_string(itsVar + "::rangeseparator", "...");

  Paragraph paragraph;

  HourPeriodGenerator generator(itsPeriod, itsVar);

  GridForecaster forecaster;

  Sentence sentence;

  int day = 0;
  int night = 0;

  for (unsigned int p = 1; p <= maxperiods && p <= generator.size(); p++)
  {
    WeatherPeriod period = generator.period(p);

    const bool isday = (period.localEndTime().GetHour() >= period.localStartTime().GetHour());

    const string periodstr =
        (isday ? "day" + lexical_cast<string>(day) : "night" + lexical_cast<string>(night));

    const string fake = itsVar + "::fake::" + periodstr;

    WeatherResult minresult = forecaster.analyze(
        fake + "::minimum", itsSources, RoadTemperature, Minimum, Minimum, itsArea, period);

    WeatherResult meanresult = forecaster.analyze(
        fake + "::mean", itsSources, RoadTemperature, Mean, Mean, itsArea, period);

    WeatherResult maxresult = forecaster.analyze(
        fake + "::maximum", itsSources, RoadTemperature, Maximum, Maximum, itsArea, period);

    // abort generating further periods if the forecast runs out of data
    if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
        meanresult.value() == kFloatMissing)
    {
      Sentence s;
      s << "tieto puuttuu";
      paragraph << s;
      log << paragraph;
      return paragraph;
    }
    //	  break;

    log << "Troad Minimum(Minimum) " << periodstr << " = " << minresult << endl;
    log << "Troad Mean(Minimum) " << periodstr << " = " << meanresult << endl;
    log << "Troad Maximum(Minimum) " << periodstr << " = " << maxresult << endl;

    const int tmin = static_cast<int>(round(minresult.value()));
    const int tmax = static_cast<int>(round(maxresult.value()));
    const int tmean = static_cast<int>(round(meanresult.value()));

    sentence << TemperatureStoryTools::temperature_sentence(
        tmin, tmean, tmax, mininterval, interval_zero, rangeseparator);
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
