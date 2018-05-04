// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::NightAndDayPeriodGenerator
 */
// ======================================================================
/*!
 * \class TextGen::NightAndDayPeriodGenerator
 *
 * \brief Generates a sequence of consecutive night/day periods
 *
 */
// ----------------------------------------------------------------------

#include "NightAndDayPeriodGenerator.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/WeatherPeriodTools.h>

#include <algorithm>

using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theMainPeriod The period to iterate
 * \param theDayStartHour The start hour of each day
 * \param theDayEndHour The end hour of each day
 * \param theDayMaxStartHour The maximum delayed start hour of each day
 * \param theDayMinEndHour The minimum early end hour of each day
 * \param theNightMaxStartHour The maximum delayed start hour of each night
 * \param theNightMinEndHour The minimum early end hour of each night
 */
// ----------------------------------------------------------------------

NightAndDayPeriodGenerator::NightAndDayPeriodGenerator(const WeatherPeriod& theMainPeriod,
                                                       int theDayStartHour,
                                                       int theDayEndHour,
                                                       int theDayMaxStartHour,
                                                       int theDayMinEndHour,
                                                       int theNightMaxStartHour,
                                                       int theNightMinEndHour)
    : itsMainPeriod(theMainPeriod),
      itsDayStartHour(theDayStartHour),
      itsDayEndHour(theDayEndHour),
      itsDayMaxStartHour(theDayMaxStartHour),
      itsDayMinEndHour(theDayMinEndHour),
      itsNightStartHour(itsDayEndHour),
      itsNightEndHour(itsDayStartHour),
      itsNightMaxStartHour(theNightMaxStartHour),
      itsNightMinEndHour(theNightMinEndHour)
{
  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * The variable is expected to contain definitions for
 * \code
 * [variable]::day::starthour = [0-23]
 * [variable]::day::endhour = [0-23]
 * \endcode
 * The variables
 * \code
 * [variable]::day::maxstarthour = [0-23]
 * [variable]::day::minendhour = [0-23]
 * [variable]::night::maxstarthour = [0-23]
 * [variable]::night::minendhour = [0-23]
 * \endcode
 * are optional.
 *
 * \param theMainPeriod The period to iterate
 * \param theVariable The variable containing the period definitions
 */
// ----------------------------------------------------------------------

NightAndDayPeriodGenerator::NightAndDayPeriodGenerator(const WeatherPeriod& theMainPeriod,
                                                       const string& theVariable)
    : itsMainPeriod(theMainPeriod),
      itsDayStartHour(Settings::require_hour(theVariable + "::day::starthour")),
      itsDayEndHour(Settings::require_hour(theVariable + "::day::endhour")),
      itsDayMaxStartHour(
          Settings::optional_hour(theVariable + "::day::maxstarthour", itsDayStartHour)),
      itsDayMinEndHour(Settings::optional_hour(theVariable + "::day::minendhour", itsDayEndHour)),
      itsNightStartHour(Settings::optional_hour(theVariable + "::night::starthour", itsDayEndHour)),
      itsNightEndHour(Settings::optional_hour(theVariable + "::night::endhour", itsDayStartHour)),
      itsNightMaxStartHour(
          Settings::optional_hour(theVariable + "::night::maxstarthour", itsDayEndHour)),
      itsNightMinEndHour(
          Settings::optional_hour(theVariable + "::night::minendhour", itsDayStartHour))
{
  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the internal list of sorted periods
 */
// ----------------------------------------------------------------------

void NightAndDayPeriodGenerator::init()
{
  using namespace WeatherPeriodTools;

  const int days = countPeriods(
      itsMainPeriod, itsDayStartHour, itsDayEndHour, itsDayMaxStartHour, itsDayMinEndHour);

  const int nights = countPeriods(
      itsMainPeriod, itsDayEndHour, itsDayStartHour, itsNightMaxStartHour, itsNightMinEndHour);

  for (int d = 1; d <= days; d++)
  {
    itsPeriods.push_back(getPeriod(
        itsMainPeriod, d, itsDayStartHour, itsDayEndHour, itsDayMaxStartHour, itsDayMinEndHour));
  }
  for (int n = 1; n <= nights; n++)
  {
    itsPeriods.push_back(getPeriod(itsMainPeriod,
                                   n,
                                   itsNightStartHour,
                                   itsNightEndHour,
                                   itsNightMaxStartHour,
                                   itsNightMinEndHour));
  }
  sort(itsPeriods.begin(), itsPeriods.end());
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period is undivided
 *
 * \return Always false, night&day period is never in principle the original
 */
// ----------------------------------------------------------------------

bool NightAndDayPeriodGenerator::undivided() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the number of subperiods
 *
 * \return The number of subperiods
 */
// ----------------------------------------------------------------------

NightAndDayPeriodGenerator::size_type NightAndDayPeriodGenerator::size() const
{
  return itsPeriods.size();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the minimal period covered by the generator
 *
 * \return The minimal period
 */
// ----------------------------------------------------------------------

WeatherPeriod NightAndDayPeriodGenerator::period() const
{
  return WeatherPeriod(itsPeriods.front().localStartTime(), itsPeriods.back().localEndTime());
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

WeatherPeriod NightAndDayPeriodGenerator::period(size_type thePeriod) const
{
  if (thePeriod < 1 || thePeriod > itsPeriods.size())
    throw TextGen::TextGenError("NightAndDayPeriodGenerator::period(): invalid argument");
  return itsPeriods[thePeriod - 1];
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the given period is day or night
 */
// ----------------------------------------------------------------------

bool NightAndDayPeriodGenerator::isday(size_type thePeriod) const
{
  if (thePeriod < 1 || thePeriod > itsPeriods.size())
    throw TextGen::TextGenError("NightAndDayPeriodGenerator::isday(): invalid argument");

  const int starthour = itsPeriods[thePeriod - 1].localStartTime().GetHour();
  const int endhour = itsPeriods[thePeriod - 1].localEndTime().GetHour();

  return (starthour <= endhour && starthour <= itsDayMaxStartHour && endhour >= itsDayMinEndHour);
}

}  // namespace TextGen

// ======================================================================
