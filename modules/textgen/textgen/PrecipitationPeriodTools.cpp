// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::PrecipitationPeriodTools
 */
// ======================================================================
/*!
 * \namespace TextGen::PrecipitationPeriodTools
 *
 * \brief Tools related to finding precipitation time intervals
 *
 * The namespace contains tools for analyzing a single time
 * interval, finding all times when it rains, joining them
 * into precipitation periods, joining precipitation periods
 * which can be considered to be the same rain period and so on.
 *
 */
// ======================================================================

#include "PrecipitationPeriodTools.h"

#include <calculator/AnalysisSources.h>
#include <calculator/MaskSource.h>
#include <calculator/PercentageCalculator.h>
#include <calculator/QueryDataIntegrator.h>
#include <calculator/QueryDataTools.h>
#include <calculator/RangeAcceptor.h>
#include <calculator/Settings.h>
#include <calculator/TimeTools.h>
#include <calculator/TextGenError.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherPeriod.h>
#include <calculator/WeatherPeriodTools.h>
#include <calculator/WeatherSource.h>

#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiQueryData.h>
#include <calculator/TextGenPosixTime.h>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;

namespace TextGen
{
namespace PrecipitationPeriodTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Analyze rain periods from data
 *
 * \param theSources The analysis sources
 * \param theArea The relevant area
 * \param thePeriod The time interval to be analyzed
 * \param theVar The variable controlling the algorithm
 * \return Sorted list of rainy periods
 */
// ----------------------------------------------------------------------

RainPeriods analyze(const AnalysisSources& theSources,
                    const WeatherArea& theArea,
                    const WeatherPeriod& thePeriod,
                    const std::string& theVar)
{
  RainTimes times = findRainTimes(theSources, theArea, thePeriod, theVar);
  RainPeriods periods1 = findRainPeriods(times, theVar);
  RainPeriods periods2 = mergeNightlyRainPeriods(periods1, theVar);
  RainPeriods periods3 = mergeLargeRainPeriods(periods2, theVar);
  return periods3;
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract all rains overlapping given period
 *
 * \param thePeriods The periods from which to extract
 * \param thePeriod The period to extract
 * \return The periods in thePeriods which overlap thePeriod
 */
// ----------------------------------------------------------------------

RainPeriods overlappingPeriods(const RainPeriods& thePeriods, const WeatherPeriod& thePeriod)
{
  RainPeriods out;
  for (RainPeriods::const_iterator it = thePeriods.begin(); it != thePeriods.end(); ++it)
  {
    if (it->localStartTime() < thePeriod.localEndTime() &&
        it->localEndTime() > thePeriod.localStartTime())
    {
      out.push_back(*it);
    }
  }
  return out;
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract all rains inclusive for the given period
 *
 * \param thePeriods The periods from which to extract
 * \param thePeriod The period to extract
 * \return The periods in thePeriods which are inclusive to thePeriod
 */
// ----------------------------------------------------------------------

RainPeriods inclusivePeriods(const RainPeriods& thePeriods, const WeatherPeriod& thePeriod)
{
  RainPeriods out;
  for (RainPeriods::const_iterator it = thePeriods.begin(); it != thePeriods.end(); ++it)
  {
    if (it->localStartTime() >= thePeriod.localStartTime() &&
        it->localEndTime() <= thePeriod.localEndTime())
    {
      out.push_back(*it);
    }
  }
  return out;
}

// ----------------------------------------------------------------------
/*!
 * \brief Find all moments when it rains for given area
 *
 * The variables controlling the algorithm are
 * \code
 * ::rainytime::minimum_rain = <0->     (default = 0.1)
 * ::rainytime::minimum_area = <0-100>  (default = 10)
 * \endcode
 * The first variable is the required rain amount in a single
 * point for the point to be considered rainy. Variable
 * \c minimum_area is the minimum required areal coverage of
 * rainy points for the area to be considered rainy. This
 * number should be fairly small.
 *
 * \param theSources The analysis sources
 * \param theArea The relevant area
 * \param thePeriod The time interval to be analyzed
 * \param theVar The variable controlling the algorithm
 * \return Sorted list of rainy times
 */
// ----------------------------------------------------------------------

RainTimes findRainTimes(const AnalysisSources& theSources,
                        const WeatherArea& theArea,
                        const WeatherPeriod& thePeriod,
                        const std::string& theVar)
{
  // Establish the settings

  const double minimum_rain = Settings::optional_double(theVar + "::rainytime::minimum_rain", 0.1);
  const double minimum_area = Settings::optional_double(theVar + "::rainytime::minimum_area", 10);

  // Establish the data
  const string datavar = "textgen::precipitation_forecast";
  const string dataname = Settings::require_string(datavar);

  // Get the data into use

  boost::shared_ptr<WeatherSource> wsource = theSources.getWeatherSource();
  boost::shared_ptr<NFmiQueryData> qd = wsource->data(dataname);
  NFmiFastQueryInfo qi = NFmiFastQueryInfo(qd.get());

  // Try activating the parameter

  if (!qi.Param(kFmiPrecipitation1h))
    throw TextGenError("Precipitation1h is not available in " + dataname);

  // Handle points and areas separately

  if (!QueryDataTools::firstTime(qi, thePeriod.utcStartTime(), thePeriod.utcEndTime()))
    throw TextGenError("The required time period is not available in " + dataname);

  RainTimes times;

  if (!theArea.isPoint())
  {
    boost::shared_ptr<MaskSource> msource = theSources.getMaskSource();
    MaskSource::mask_type mask = msource->mask(theArea, dataname, *wsource);
    RangeAcceptor acceptor;
    acceptor.lowerLimit(minimum_rain);
    PercentageCalculator calculator;
    calculator.condition(acceptor);
    do
    {
      const float tmp = QueryDataIntegrator::Integrate(qi, *mask, calculator);
      if (tmp != kFloatMissing && tmp >= minimum_area)
      {
        NFmiMetTime metTime(qi.Time());
        TextGenPosixTime textgenTime(metTime.GetYear(),
                                     metTime.GetMonth(),
                                     metTime.GetDay(),
                                     metTime.GetHour(),
                                     metTime.GetMin(),
                                     metTime.GetSec());
        times.push_back(TextGenPosixTime::LocalTime(textgenTime));
      }
    } while (qi.NextTime() && qi.Time() <= thePeriod.utcEndTime());
  }
  else
  {
    if (!(qi.Location(theArea.point())))
    {
      ostringstream msg;
      msg << "Could not set desired coordinate (" << theArea.point().X() << ','
          << theArea.point().Y() << ')';
      if (theArea.isNamed()) msg << " named " << theArea.name();
      msg << " in " << dataname;
      throw TextGenError(msg.str());
    }

    do
    {
      const float tmp = qi.FloatValue();
      if (tmp != kFloatMissing && tmp >= minimum_rain)
      {
        NFmiMetTime metTime(qi.Time());
        TextGenPosixTime textgenTime(metTime.GetYear(),
                                     metTime.GetMonth(),
                                     metTime.GetDay(),
                                     metTime.GetHour(),
                                     metTime.GetMin(),
                                     metTime.GetSec());
        times.push_back(TextGenPosixTime::LocalTime(textgenTime));
      }
    } while (qi.NextTime() && qi.Time() <= thePeriod.utcEndTime());
  }

  return times;
}

// ----------------------------------------------------------------------
/*!
 * \brief Join rainy times into rain periods
 *
 * The variables controlling the algorithm are
 * \code
 * ::rainyperiod::maximum_interval = <1->  (default = 1)
 * \endcode
 * Variable \c maximum_interval determines the maximum allowed
 * separation between any two rainy moments in time for them
 * to be considered to belong into the same rain period.
 * Typically one might use a value like 3 hours, but the default
 * is nonetheless 1 hour. This forces the used to make a
 * conscious choice on the maximum separation.
 *
 * \param theTimes The rainy times to be joined (must be sorted)
 * \param theVar The variable controlling the algorithm
 * \return Sorted list of rainy periods
 */
// ----------------------------------------------------------------------

RainPeriods findRainPeriods(const RainTimes& theTimes, const std::string& theVar)
{
  RainPeriods periods;

  // Establish the settings

  const int maximum_interval =
      Settings::optional_int(theVar + "::rainyperiod::maximum_interval", 1);

  // Handle special cases

  if (theTimes.empty()) return periods;

  // Initialize current period to consist of first time only
  RainTimes::const_iterator it = theTimes.begin();
  TextGenPosixTime first_time = *it;
  TextGenPosixTime last_time = first_time;

  // Then append new times until interval becomes too long,
  // at which point we save the period and start a new one.

  for (++it; it != theTimes.end(); ++it)
  {
    if (it->DifferenceInHours(last_time) > maximum_interval)
    {
      WeatherPeriod period(first_time, last_time);
      periods.push_back(period);
      first_time = *it;
    }
    last_time = *it;
  }
  WeatherPeriod period(first_time, last_time);
  periods.push_back(period);

  return periods;
}

// ----------------------------------------------------------------------
/*!
 * \brief Merge nigthly rain periods
 *
 * The function merges rain periods which are separated by
 * relatively short no-rain periods during the night.
 *
 * The idea is that reporting the timing of nightly rains
 * is not that important, we probably prefer to say the
 * rain is intermittent.
 *
 * The algorithm is controlled with variables
 * \code
 * ::rainyperiod::night::starthour = <0-23>       (default = 21)
 * ::rainyperiod::night::endhour = <0-23>         (default = 9)
 * ::rainyperiod::night::maximum_interval = <1->  (default = 1)
 * \endcode
 * If a no-rain period is at most \c maximum_interval hours
 * long, and the period falls completely within the given
 * hours, the two surrounding rain periods are collapsed into
 * one.
 *
 * Note the default value for \c maximum_interval is only 1,
 * this in practise means no rains are merged. A sensible
 * value when merging is desired is something of the order
 * of 5 hours. Naturally it does not make sense to define
 * a maximum length longer than the night itself, then no
 * merge can ever occur.
 *
 * \param thePeriods The periods to be merged
 * \param theVar The control variable
 * \return The merged periods
 */
// ----------------------------------------------------------------------

RainPeriods mergeNightlyRainPeriods(const RainPeriods& thePeriods, const std::string& theVar)
{
  // Quick exit if there are no no-rain periods between rain periods
  if (thePeriods.size() <= 1) return thePeriods;

  // Establish the settings

  const string var = theVar + "::rainyperiod::night::";
  const int starthour = Settings::optional_hour(var + "starthour", 21);
  const int endhour = Settings::optional_hour(var + "endhour", 9);
  const int maximum_interval = Settings::optional_int(var + "maximum_interval", 1);
  RainPeriods periods;

  // We start with the first period, then merge the next one
  // to it if possible. When it is no longer possible to merge,
  // we move on to the next period and continue.

  RainPeriods::const_iterator it = thePeriods.begin();
  WeatherPeriod lastperiod = *it;

  for (++it; it != thePeriods.end(); ++it)
  {
    WeatherPeriod norain(lastperiod.localEndTime(), it->localStartTime());
    const int length = WeatherPeriodTools::hours(norain);
    if (length <= maximum_interval &&
        WeatherPeriodTools::countPeriods(norain, starthour, endhour) == 1)
    {
      lastperiod = WeatherPeriod(lastperiod.localStartTime(), it->localEndTime());
    }
    else
    {
      periods.push_back(lastperiod);
      lastperiod = *it;
    }
  }
  periods.push_back(lastperiod);

  return periods;
}

// ----------------------------------------------------------------------
/*!
 * \brief Merge large scale precipitation periods
 *
 * The function merges precipitation periods which can
 * be considered to belong into the same large scale
 * precipitation. The basic criteria is that the interval
 * between the periods must be short enough, and that
 * atleast one of the periods must be long enough to be
 * considered large scale.
 *
 * The idea is to avoid reporting on multiple rain periods
 * when the rain is in principle coming from the same
 * precipitation system.
 *
 * The algorithm is controlled with variables
 * \code
 * ::rainyperiod::minimum_large_rain_length = <1->      (default = 8)
 * ::rainyperiod::maximum_large_rain_interval = <1->  (default = 1)
 * \endcode
 * Variable \c minimum_large_rain_length is the required rain period
 * length for the rain to be considered large scale. Variable
 * \c maximum_large_rain_interval is then the maximum time interval
 * between any other period for that period to be joined into
 * the same large scale rain.
 *
 * \param thePeriods The rainy periods to be merged
 * \param theVar The variable controlling the algorithm
 * \return Sorted list of rainy periods
 *
 */
// ----------------------------------------------------------------------

RainPeriods mergeLargeRainPeriods(const RainPeriods& thePeriods, const std::string& theVar)
{
  return thePeriods;
}

}  // namespace PrecipitationPeriodTools
}  // namespace TextGen

// ======================================================================
