// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::CloudinessStory::overview
 */
// ======================================================================

#include "CloudinessStory.h"
#include "CloudinessStoryTools.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "PeriodPhraseFactory.h"
#include <calculator/RangeAcceptor.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/WeatherResult.h>

#include <boost/lexical_cast.hpp>

#include <vector>

using namespace boost;
using namespace std;
using namespace TextGen;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on cloudiness
 *
 * \return The story
 *
 */
// ----------------------------------------------------------------------

const Paragraph CloudinessStory::overview() const
{
  MessageLogger log("CloudinessStory::overview");

  using namespace Settings;
  using namespace CloudinessStoryTools;

  // Establish optional control settings

  const int clear = optional_percentage(itsVar + "::clear", 40);
  const int cloudy = optional_percentage(itsVar + "::cloudy", 70);

  const bool merge_same = optional_bool(itsVar + "::merge_same", true);
  const bool merge_similar = optional_bool(itsVar + "::merge_similar", true);

  // Calculate the percentages for each day in the period
  // We do not allow forecasts longer than 3 days in order
  // to limit the complexity of the algorithm.

  GridForecaster forecaster;
  Paragraph paragraph;

  const HourPeriodGenerator periodgenerator(itsPeriod, itsVar + "::day");
  const int ndays = min(periodgenerator.size(), 3u);

  log << "Period covers " << ndays << " days" << endl;

  if (ndays <= 0)
  {
    log << paragraph;
    return paragraph;
  }

  vector<WeatherPeriod> periods;
  vector<CloudinessType> types;

  for (int day = 1; day <= ndays; day++)
  {
    const WeatherPeriod period(periodgenerator.period(day));
    const string daystr = "day" + lexical_cast<string>(day);

    RangeAcceptor cloudylimits;
    cloudylimits.lowerLimit(cloudy);

    RangeAcceptor clearlimits;
    clearlimits.upperLimit(clear);

    const WeatherResult cloudy_percentage =
        forecaster.analyze(itsVar + "::fake::" + daystr + "::cloudy",
                           itsSources,
                           Cloudiness,
                           Mean,
                           Percentage,
                           itsArea,
                           period,
                           DefaultAcceptor(),
                           DefaultAcceptor(),
                           cloudylimits);

    const WeatherResult clear_percentage =
        forecaster.analyze(itsVar + "::fake::" + daystr + "::clear",
                           itsSources,
                           Cloudiness,
                           Mean,
                           Percentage,
                           itsArea,
                           period,
                           DefaultAcceptor(),
                           DefaultAcceptor(),
                           clearlimits);

    const WeatherResult trend = forecaster.analyze(itsVar + "::fake::" + daystr + "::trend",
                                                   itsSources,
                                                   Cloudiness,
                                                   Mean,
                                                   Trend,
                                                   itsArea,
                                                   period);

    log << "Cloudiness Mean(Percentage(cloudy)) " << daystr << " = " << cloudy_percentage << endl;
    log << "Cloudiness Mean(Percentage(clear)) " << daystr << " = " << clear_percentage << endl;
    log << "Cloudiness Mean(Trend) " << daystr << " = " << trend << endl;

    CloudinessType ctype =
        cloudiness_type(itsVar, cloudy_percentage.value(), clear_percentage.value(), trend.value());

    periods.push_back(period);
    types.push_back(ctype);
  }

  // Now 'periods' contains the period for each day
  // and 'types' contains the weather type for each day.

  unsigned int startday = 0;
  Sentence sentence;
  while (startday < periods.size())
  {
    // seek the end for a sequence of similar days,
    // simultaneously establishing the merged cloudiness type

    unsigned int endday = startday;
    CloudinessType cloudiness = types[startday];

    if (merge_similar)
    {
      CommonCloudiness tmp(VariableCloudiness, false);
      while (endday + 1 < periods.size() &&
             (tmp = similar_type(types, startday, endday + 1)).second)
      {
        cloudiness = tmp.first;
        ++endday;
      }
    }
    else if (merge_same)
    {
      while (endday + 1 < periods.size() && types[endday + 1] == types[startday])
      {
        ++endday;
      }
    }

    // generate sentence for the found period

    WeatherPeriod fullperiod(periods[startday].localStartTime(), periods[endday].localEndTime());

    if (!sentence.empty()) sentence << Delimiter(",");

    sentence << PeriodPhraseFactory::create("days", itsVar, itsForecastTime, fullperiod)
             << cloudiness_phrase(cloudiness);

    startday = endday + 1;
  }
  paragraph << sentence;

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
