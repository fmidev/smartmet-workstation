// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::HourPeriodGenerator
 */
// ======================================================================
/*!
 * \class TextGen::HourPeriodGenerator
 *
 * \brief Generates a sequence of periods
 *
 * This class uses WeatherPeriodTools countPeriods and getPeriod
 * as the basis for generating a sequence of subperiods. Hence
 * the basic idea is to define the acceptable hour interval in
 * the constructor, the accessors will then take care of the rest.
 */
// ----------------------------------------------------------------------

#include "HourPeriodGenerator.h"
#include "WeatherPeriodTools.h"
#include "Settings.h"

using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theMainPeriod The period to iterate
 * \param theStartHour The start hour of each generated period
 * \param theEndHour The end hour of each generated period
 * \param theMaxStartHour The maximum delayed start hour of each period
 * \param theMinEndHour The minimum early end hour of each period
 */
// ----------------------------------------------------------------------

HourPeriodGenerator::HourPeriodGenerator(const WeatherPeriod& theMainPeriod,
                                         int theStartHour,
                                         int theEndHour,
                                         int theMaxStartHour,
                                         int theMinEndHour)
    : itsMainPeriod(theMainPeriod),
      itsStartHour(theStartHour),
      itsEndHour(theEndHour),
      itsMaxStartHour(theMaxStartHour),
      itsMinEndHour(theMinEndHour),
      itsSize(WeatherPeriodTools::countPeriods(
          theMainPeriod, theStartHour, theEndHour, theMaxStartHour, theMinEndHour))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * The variable is expected to contain definitions for
 * \code
 * [variable]::starthour = [0-23]
 * [variable]::endhour = [0-23]
 * \endcode
 * The variables
 * \code
 * [variable]::maxstarthour = [0-23]
 * [variable]::minendhour = [0-23]
 * \endcode
 * are optional.
 *
 * \param theMainPeriod The period to iterate
 * \param theVariable The variable containing the period definitions
 */
// ----------------------------------------------------------------------

HourPeriodGenerator::HourPeriodGenerator(const WeatherPeriod& theMainPeriod,
                                         const string& theVariable)
    : itsMainPeriod(theMainPeriod),
      itsStartHour(Settings::require_hour(theVariable + "::starthour")),
      itsEndHour(Settings::require_hour(theVariable + "::endhour")),
      itsMaxStartHour(Settings::optional_hour(theVariable + "::maxstarthour", itsStartHour)),
      itsMinEndHour(Settings::optional_hour(theVariable + "::minendhour", itsEndHour)),
      itsSize(WeatherPeriodTools::countPeriods(
          itsMainPeriod, itsStartHour, itsEndHour, itsMaxStartHour, itsMinEndHour))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period is undivided
 *
 * \return Always false, huour period is never in principle the original
 */
// ----------------------------------------------------------------------

bool HourPeriodGenerator::undivided() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the number of subperiods
 *
 * \return The number of subperiods
 */
// ----------------------------------------------------------------------

HourPeriodGenerator::size_type HourPeriodGenerator::size() const { return itsSize; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the minimal period covered by the generator
 *
 * \return The minimal period
 */
// ----------------------------------------------------------------------

WeatherPeriod HourPeriodGenerator::period() const
{
  const TextGenPosixTime start = period(1).localStartTime();
  const TextGenPosixTime end = period(size()).localEndTime();
  return WeatherPeriod(start, end);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the desired subperiod
 *
 * Throws if there is no such subperiod
 *
 * \param thePeriod The index of the subperiod
 * \return The subperiod
 */
// ----------------------------------------------------------------------

WeatherPeriod HourPeriodGenerator::period(size_type thePeriod) const
{
  return WeatherPeriodTools::getPeriod(
      itsMainPeriod, thePeriod, itsStartHour, itsEndHour, itsMaxStartHour, itsMinEndHour);
}

}  // namespace TextGen

// ======================================================================
