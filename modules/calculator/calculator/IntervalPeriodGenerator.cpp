// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::IntervalPeriodGenerator
 */
// ======================================================================
/*!
 * \class TextGen::IntervalPeriodGenerator
 *
 * \brief Generates a sequence of periods
 *
 * The class divides the entire time interval into equal size
 * subintervals. The first interval starts at the desired
 * start hour. The interval length must divide 24 or must be divisible
 * by 24.
 */
// ----------------------------------------------------------------------

#include "IntervalPeriodGenerator.h"
#include "Settings.h"
#include "TextGenError.h"

using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theMainPeriod The period to iterate
 * \param theStartHour The start hour of the first generated period
 * \param theInterval The desired interval length
 * \param theMinimumInterval The minimum allowed interval length
 */
// ----------------------------------------------------------------------

IntervalPeriodGenerator::IntervalPeriodGenerator(const WeatherPeriod& theMainPeriod,
                                                 int theStartHour,
                                                 int theInterval,
                                                 int theMinimumInterval)
    : itsMainPeriod(theMainPeriod),
      itsStartHour(theStartHour),
      itsInterval(theInterval),
      itsMinimumInterval(theMinimumInterval)
{
  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * The variable is expected to contain definitions for
 * \code
 * [variable]::starthour = [0-23] (= 0)
 * [variable]::interval    = [0|1|2|3|4|6|8|12|24|N*24]
 * [variable]::mininterval = [0-X] (= interval)
 * \endcode
 **
 * \param theMainPeriod The period to iterate
 * \param theVariable The variable containing the period definitions
 */
// ----------------------------------------------------------------------

IntervalPeriodGenerator::IntervalPeriodGenerator(const WeatherPeriod& theMainPeriod,
                                                 const string& theVariable)
    : itsMainPeriod(theMainPeriod),
      itsStartHour(Settings::require_hour(theVariable + "::starthour")),
      itsInterval(Settings::require_int(theVariable + "::interval")),
      itsMinimumInterval(Settings::optional_int(theVariable + "::mininterval", itsInterval))
{
  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period is undivided
 *
 * \return Always false, interval period is never in principle the original
 */
// ----------------------------------------------------------------------

bool IntervalPeriodGenerator::undivided() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Initialize the internal list of sorted periods
 */
// ----------------------------------------------------------------------

void IntervalPeriodGenerator::init()
{
  if ((itsInterval != 0) && (24 % itsInterval != 0 && itsInterval % 24 != 0))
    throw TextGenError(
        "IntervalPeriodGenerator: Interval must divide 24 evenly or be divisible by 24");
  if (itsInterval < 0)
    throw TextGenError("IntervalPeriodGenerator: Interval length must be positive");

  if (itsMinimumInterval < 0)
    throw TextGenError("IntervalPeriodGenerator: Minimum interval must be positive");
  if (itsMinimumInterval > itsInterval)
    throw TextGenError(
        "IntervalPeriodGenerator: Minimum interval must be in range 1-interval or be zero");

  // Now we can generate the periods in advance

  TextGenPosixTime time(itsMainPeriod.localStartTime());
  time.ChangeByDays(-1);
  time.SetHour(itsStartHour);

  while (time <= itsMainPeriod.localEndTime())
  {
    TextGenPosixTime starttime(time);
    TextGenPosixTime endtime(time);
    endtime.ChangeByHours(itsInterval);

    if (starttime.IsLessThan(itsMainPeriod.localStartTime()))
      starttime = itsMainPeriod.localStartTime();
    if (itsMainPeriod.localEndTime().IsLessThan(endtime)) endtime = itsMainPeriod.localEndTime();

    const int diff = endtime.DifferenceInHours(starttime);
    if (diff >= itsMinimumInterval) itsPeriods.push_back(WeatherPeriod(starttime, endtime));

    if (itsInterval <= 0)
      time.ChangeByHours(1);
    else
    {
      // Do not advance N*24 hours but 24 hours until we reach start of data
      if (itsPeriods.empty() && itsInterval >= 24)
        time.ChangeByHours(24);
      else
        time.ChangeByHours(itsInterval);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the number of subperiods
 *
 * \return The number of subperiods
 */
// ----------------------------------------------------------------------

IntervalPeriodGenerator::size_type IntervalPeriodGenerator::size() const
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

WeatherPeriod IntervalPeriodGenerator::period() const
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

WeatherPeriod IntervalPeriodGenerator::period(size_type thePeriod) const
{
  if (thePeriod < 1 || thePeriod > itsPeriods.size())
    throw TextGen::TextGenError("IntervalPeriodGenerator::period(): invalid argument");
  return itsPeriods[thePeriod - 1];
}

}  // namespace TextGen

// ======================================================================
