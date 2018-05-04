// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::MorningAndEveningPeriodGenerator
 */
// ======================================================================
/*!
 * \class TextGen::MorningAndEveningPeriodGenerator
 *
 * \brief Generates a sequence of consecutive morning/day/evening/night periods
 *
 * We assume all 4 periods start during the same day. Hence the night
 * starthour may be 23, but not 00. However, the maximum night start hour
 * may occur after midnight.
 */
// ----------------------------------------------------------------------

#include "MorningAndEveningPeriodGenerator.h"
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
 * \param theMorningStartHour The start hour of each morning
 * \param theDayStartHour The start hour of each day
 * \param theEveningStartHour The start hour of each evening
 * \param theNightStartHour The start hour of each night
 * \param theMorningMaxStartHour The latest start hour for mornings
 * \param theDayMaxStartHour The latest start hour for days
 * \param theEveningMaxStartHour The latest start hour for evenings
 * \param theNightMaxStartHour The latest start hour for nights
 */
// ----------------------------------------------------------------------

MorningAndEveningPeriodGenerator::MorningAndEveningPeriodGenerator(
    const WeatherPeriod& theMainPeriod,
    int theMorningStartHour,
    int theDayStartHour,
    int theEveningStartHour,
    int theNightStartHour,
    int theMorningMaxStartHour,
    int theDayMaxStartHour,
    int theEveningMaxStartHour,
    int theNightMaxStartHour)
    : itsMainPeriod(theMainPeriod),
      itsMorningStartHour(theMorningStartHour),
      itsDayStartHour(theDayStartHour),
      itsEveningStartHour(theEveningStartHour),
      itsNightStartHour(theNightStartHour),
      itsMorningMaxStartHour(theMorningMaxStartHour),
      itsDayMaxStartHour(theDayMaxStartHour),
      itsEveningMaxStartHour(theEveningMaxStartHour),
      itsNightMaxStartHour(theNightMaxStartHour)
{
  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * The variable is expected to contain definitions for
 * \code
 * [variable]::morning::starthour = [0-23]
 * [variable]::day::starthour     = [0-23]
 * [variable]::evening::starthour = [0-23]
 * [variable]::night::starthour   = [0-23]
 * \endcode
 * The variables
 * \code
 * [variable]::morning::maxstarthour = [0-23]
 * [variable]::day::maxstarthour     = [0-23]
 * [variable]::evening::maxstarthour = [0-23]
 * [variable]::night::maxstarthour   = [0-23]
 * \endcode
 * are optional.
 *
 * \param theMainPeriod The period to iterate
 * \param theVariable The variable containing the period definitions
 */
// ----------------------------------------------------------------------

MorningAndEveningPeriodGenerator::MorningAndEveningPeriodGenerator(
    const WeatherPeriod& theMainPeriod, const string& theVariable)
    : itsMainPeriod(theMainPeriod),
      itsMorningStartHour(Settings::require_hour(theVariable + "::morning::starthour")),
      itsDayStartHour(Settings::require_hour(theVariable + "::day::starthour")),
      itsEveningStartHour(Settings::require_hour(theVariable + "::evening::starthour")),
      itsNightStartHour(Settings::require_hour(theVariable + "::night::starthour")),
      itsMorningMaxStartHour(
          Settings::optional_hour(theVariable + "::morning::maxstarthour", itsMorningStartHour)),
      itsDayMaxStartHour(
          Settings::optional_hour(theVariable + "::day::maxstarthour", itsDayStartHour)),
      itsEveningMaxStartHour(
          Settings::optional_hour(theVariable + "::evening::maxstarthour", itsEveningStartHour)),
      itsNightMaxStartHour(
          Settings::optional_hour(theVariable + "::night::maxstarthour", itsNightStartHour))
{
  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the internal list of sorted periods
 */
// ----------------------------------------------------------------------

void MorningAndEveningPeriodGenerator::init()
{
  using namespace WeatherPeriodTools;

  // first we check that the period definitions are sane
  // Assumptions:
  // morning, day, evening and night must start during the same day

  if (itsDayStartHour <= itsMorningStartHour)
    throw TextGenError("MorningAndEveningPeriodGenerator: Morning must start before day");
  if (itsEveningStartHour <= itsDayStartHour)
    throw TextGenError("MorningAndEveningPeriodGenerator: Day must start before evening");
  if (itsNightStartHour <= itsEveningStartHour)
    throw TextGenError("MorningAndEveningPeriodGenerator: Evening must start before night");
  if (itsMorningStartHour >= itsNightStartHour)
    throw TextGenError("MorningAndEveningPeriodGenerator: Night must start before morning");

  if (itsMorningMaxStartHour < itsMorningStartHour || itsMorningMaxStartHour >= itsDayStartHour)
    throw TextGenError(
        "MorningAndEveningPeriodGenerator: Maximum morning start hour must be during the morning");

  if (itsDayMaxStartHour < itsDayStartHour || itsDayMaxStartHour >= itsEveningStartHour)
    throw TextGenError(
        "MorningAndEveningPeriodGenerator: Maximum day start hour must be during the day");

  if (itsEveningMaxStartHour < itsEveningStartHour || itsEveningMaxStartHour >= itsNightStartHour)
    throw TextGenError(
        "MorningAndEveningPeriodGenerator: Maximum evening start hour must be during the evening");

  if (itsNightMaxStartHour < itsNightStartHour && itsNightMaxStartHour >= itsMorningStartHour)
    throw TextGenError(
        "MorningAndEveningPeriodGenerator: Maximum night start hour must be during the night");

  const int mornings = countPeriods(
      itsMainPeriod, itsMorningStartHour, itsDayStartHour, itsMorningMaxStartHour, itsDayStartHour);

  const int days = countPeriods(
      itsMainPeriod, itsDayStartHour, itsEveningStartHour, itsDayMaxStartHour, itsEveningStartHour);

  const int evenings = countPeriods(itsMainPeriod,
                                    itsEveningStartHour,
                                    itsNightStartHour,
                                    itsEveningMaxStartHour,
                                    itsNightStartHour);

  const int nights = countPeriods(itsMainPeriod,
                                  itsNightStartHour,
                                  itsMorningStartHour,
                                  itsNightMaxStartHour,
                                  itsMorningStartHour);

  for (int m = 1; m <= mornings; m++)
  {
    itsPeriods.push_back(getPeriod(itsMainPeriod,
                                   m,
                                   itsMorningStartHour,
                                   itsDayStartHour,
                                   itsMorningMaxStartHour,
                                   itsDayStartHour));
  }

  for (int d = 1; d <= days; d++)
  {
    itsPeriods.push_back(getPeriod(itsMainPeriod,
                                   d,
                                   itsDayStartHour,
                                   itsEveningStartHour,
                                   itsDayMaxStartHour,
                                   itsEveningStartHour));
  }

  for (int e = 1; e <= evenings; e++)
  {
    itsPeriods.push_back(getPeriod(itsMainPeriod,
                                   e,
                                   itsEveningStartHour,
                                   itsNightStartHour,
                                   itsEveningMaxStartHour,
                                   itsNightStartHour));
  }

  for (int n = 1; n <= nights; n++)
  {
    itsPeriods.push_back(getPeriod(itsMainPeriod,
                                   n,
                                   itsNightStartHour,
                                   itsMorningStartHour,
                                   itsNightMaxStartHour,
                                   itsMorningStartHour));
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

bool MorningAndEveningPeriodGenerator::undivided() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the number of subperiods
 *
 * \return The number of subperiods
 */
// ----------------------------------------------------------------------

MorningAndEveningPeriodGenerator::size_type MorningAndEveningPeriodGenerator::size() const
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

WeatherPeriod MorningAndEveningPeriodGenerator::period() const
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

WeatherPeriod MorningAndEveningPeriodGenerator::period(size_type thePeriod) const
{
  if (thePeriod < 1 || thePeriod > itsPeriods.size())
    throw TextGen::TextGenError("MorningAndEveningPeriodGenerator::period(): invalid argument");
  return itsPeriods[thePeriod - 1];
}

}  // namespace TextGen

// ======================================================================
