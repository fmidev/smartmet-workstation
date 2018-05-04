// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::PrecipitationStory::sums
 */
// ======================================================================

#include "PrecipitationStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "PrecipitationStoryTools.h"
#include <calculator/RangeAcceptor.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include "UnitFactory.h"
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
 * \brief Generate story on total precipitation range
 *
 * \return The story
 */
// ----------------------------------------------------------------------

Paragraph PrecipitationStory::sums() const
{
  MessageLogger log("PrecipitationStory::sums");

  const double minrain = Settings::optional_double(itsVar + "::minrain", 0);
  const int mininterval = Settings::optional_int(itsVar + "::mininterval", 1);
  const string rangeseparator = Settings::optional_string(itsVar + "::rangeseparator", "-");

  Paragraph paragraph;

  GridForecaster forecaster;

  const TextGenPosixTime time1 = itsPeriod.localStartTime();
  const TextGenPosixTime time2 = TimeTools::addHours(time1, 12);
  const TextGenPosixTime time3 = TimeTools::addHours(time1, 24);

  log << "Period 1 = " << time1 << " ... " << time2 << endl;
  log << "Period 2 = " << time2 << " ... " << time3 << endl;

  vector<WeatherPeriod> periods;
  periods.push_back(WeatherPeriod(time1, time2));
  periods.push_back(WeatherPeriod(time2, time3));

  RangeAcceptor rainlimits;
  rainlimits.lowerLimit(minrain);

  vector<WeatherResult> minima;
  vector<WeatherResult> maxima;
  vector<WeatherResult> means;

  for (vector<int>::size_type i = 0; i < periods.size(); i++)
  {
    const string fake = itsVar + "::fake::period" + lexical_cast<string>(i + 1);

    WeatherResult minresult = forecaster.analyze(fake + "::minimum",
                                                 itsSources,
                                                 Precipitation,
                                                 Minimum,
                                                 Sum,
                                                 itsArea,
                                                 periods[i],
                                                 DefaultAcceptor(),
                                                 rainlimits);

    WeatherResult maxresult = forecaster.analyze(fake + "::maximum",
                                                 itsSources,
                                                 Precipitation,
                                                 Maximum,
                                                 Sum,
                                                 itsArea,
                                                 periods[i],
                                                 DefaultAcceptor(),
                                                 rainlimits);

    WeatherResult meanresult = forecaster.analyze(fake + "::mean",
                                                  itsSources,
                                                  Precipitation,
                                                  Mean,
                                                  Sum,
                                                  itsArea,
                                                  periods[i],
                                                  DefaultAcceptor(),
                                                  rainlimits);

    log << "Precipitation Minimum(Sum) for period " << i + 1 << " = " << minresult << endl;
    log << "Precipitation Maximum(Sum) for period " << i + 1 << " = " << maxresult << endl;
    log << "Precipitation Mean(Sum) for period " << i + 1 << " = " << meanresult << endl;

    if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
        meanresult.value() == kFloatMissing)
      throw TextGenError("Total precipitation not available");

    minima.push_back(minresult);
    maxima.push_back(maxresult);
    means.push_back(meanresult);
  }

  Sentence sentence;
  if (round(maxima[0].value()) == 0 && round(maxima[1].value()) == 0)
  {
    const WeatherResult zero(0, 0);
    sentence << "seuraavan 24 tunnin sademaara"
             << "on"
             << PrecipitationStoryTools::sum_phrase(zero, zero, zero, mininterval, rangeseparator);
  }
  else
  {
    sentence << "ensimmaisen 12 tunnin sademaara"
             << "on" << PrecipitationStoryTools::sum_phrase(
                            minima[0], maxima[0], means[0], mininterval, rangeseparator)
             << Delimiter(",") << "seuraavan 12 tunnin"
             << PrecipitationStoryTools::sum_phrase(
                    minima[1], maxima[1], means[1], mininterval, rangeseparator);
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
