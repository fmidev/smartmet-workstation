// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::PrecipitationStory::daily_sums
 */
// ======================================================================

#include "PrecipitationStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "Integer.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include "PeriodPhraseFactory.h"
#include "PrecipitationStoryTools.h"
#include <calculator/RangeAcceptor.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include "UnitFactory.h"
#include <calculator/WeatherResult.h>
#include <calculator/WeatherResultTools.h>

#include <boost/lexical_cast.hpp>

#include <vector>

using namespace boost;
using namespace std;
using namespace TextGen;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on daily total precipitation ranges
 *
 * \return The story
 */
// ----------------------------------------------------------------------

Paragraph PrecipitationStory::daily_sums() const
{
  MessageLogger log("PrecipitationStory::daily_sums");

  const double minrain = Settings::optional_double(itsVar + "::minrain", 0);
  const int mininterval = Settings::optional_int(itsVar + "::mininterval", 1);
  const bool ignore_fair_days = Settings::optional_bool(itsVar + "::ignore_fair_days", true);
  const string rangeseparator = Settings::optional_string(itsVar + "::rangeseparator", "-");

  Paragraph paragraph;

  GridForecaster forecaster;

  // All the days

  HourPeriodGenerator generator(itsPeriod, itsVar + "::day");

  // Filter out too small hourly precipitation from the sum

  RangeAcceptor rainlimits;
  rainlimits.lowerLimit(minrain);

  // Calculate daily results

  vector<WeatherPeriod> periods;
  vector<WeatherResult> minima;
  vector<WeatherResult> maxima;
  vector<WeatherResult> means;

  const unsigned int ndays = generator.size();

  for (unsigned int day = 1; day <= ndays; day++)
  {
    WeatherPeriod period = generator.period(day);

    const string fake = itsVar + "::fake::day" + lexical_cast<string>(day);

    WeatherResult minresult = forecaster.analyze(fake + "::minimum",
                                                 itsSources,
                                                 Precipitation,
                                                 Minimum,
                                                 Sum,
                                                 itsArea,
                                                 period,
                                                 DefaultAcceptor(),
                                                 rainlimits);

    WeatherResult maxresult = forecaster.analyze(fake + "::maximum",
                                                 itsSources,
                                                 Precipitation,
                                                 Maximum,
                                                 Sum,
                                                 itsArea,
                                                 period,
                                                 DefaultAcceptor(),
                                                 rainlimits);

    WeatherResult meanresult = forecaster.analyze(fake + "::mean",
                                                  itsSources,
                                                  Precipitation,
                                                  Mean,
                                                  Sum,
                                                  itsArea,
                                                  period,
                                                  DefaultAcceptor(),
                                                  rainlimits);

    log << "Precipitation Minimum(Sum) for day " << day << " = " << minresult << endl;
    log << "Precipitation Maximum(Sum) for day " << day << " = " << maxresult << endl;
    log << "Precipitation Mean(Sum) for day " << day << " = " << meanresult << endl;

    if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
        meanresult.value() == kFloatMissing)
      throw TextGenError("Total precipitation not available");

    periods.push_back(period);
    minima.push_back(minresult);
    maxima.push_back(maxresult);
    means.push_back(meanresult);
  }

  // Generate a single sentence from the results

  Sentence sentence;

  bool same_enabled = true;
  unsigned int last = 99999;

  for (unsigned int i = 0; i < periods.size(); i++)
  {
    if (!ignore_fair_days || round(maxima[i].value()) > 0)
    {
      const bool empty = sentence.empty();
      if (i == 0)
      {
        sentence << "sadesumma"
                 << "on"
                 << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, periods[i])
                 << PeriodPhraseFactory::create(
                        "remaining_day", itsVar, itsForecastTime, periods[i])
                 << PrecipitationStoryTools::sum_phrase(
                        minima[i], maxima[i], means[i], mininterval, rangeseparator);
      }
      else
      {
        if (empty)
          sentence << "sadesumma"
                   << "on";
        else
          sentence << Delimiter(",");
        sentence << PeriodPhraseFactory::create("next_day", itsVar, itsForecastTime, periods[i]);

        if (same_enabled && !empty && WeatherResultTools::isSimilarRange(
                                          minima[last], maxima[last], minima[i], maxima[i], itsVar))
        {
          sentence << "sama";
          same_enabled = false;
        }
        else
        {
          sentence << PrecipitationStoryTools::sum_phrase(
              minima[i], maxima[i], means[i], mininterval, rangeseparator);
        }
      }

      last = i;
    }
  }

  paragraph << sentence;

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
