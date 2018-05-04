// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::WeatherPeriodFactory
 */
// ======================================================================
/*!
 * \namespace TextGen::WeatherPeriodFactory
 *
 * \brief Producers for suitable weather periods
 *
 * The namespace consists of functions which given a reference time
 * in local time and a name of a conventional weather period, produce
 * the conventional weather period as a WeatherPeriod object.
 *
 * \see page_aikavalit
 *
 */
// ======================================================================

#include "WeatherPeriodFactory.h"
#include <calculator/WeatherPeriod.h>
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>

#include <calculator/TextGenPosixTime.h>

using namespace TextGen;
using namespace std;

// ======================================================================
//				IMPLEMENTATION HIDING FUNCTIONS
// ======================================================================

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Round up the given time to full hours
 *
 * \param theTime The time to round up
 * \return The time rounded up
 */
// ----------------------------------------------------------------------

TextGenPosixTime round_up(const TextGenPosixTime& theTime)
{
  TextGenPosixTime ret(theTime);
  if (ret.GetMin() > 0 || ret.GetSec() > 0) ret.ChangeByHours(1);
  ret.SetMin(0);
  ret.SetSec(0);
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Create period from now to now
 *
 * There are no required variables are
 *
 * \param theTime The local reference time
 * \param theVariable The variable containing optional extra settings
 * \return The period
 */
// ----------------------------------------------------------------------

WeatherPeriod period_now(const TextGenPosixTime& theTime, const string& theVariable)
{
  WeatherPeriod period(theTime, theTime);
  return period;
}

// ----------------------------------------------------------------------
/*!
 * \brief Create period from now until some given time
 *
 * Required variables are
 *
 *  - \c variable::days
 *  - \c variable::endhour
 *  - \c variable::switchhour
 *
 * The start hour is formed by rounding up the given time to the
 * next full hour. The end hour is formed by adding \c days to
 * the start hour. If the hour is after \c switchhour, an extra
 * day is added. The end hour is then set from \c endhour.
 *
 * For example, if days=1, switchhour=12, endhour=18 we get
 *
 * - On Monday 11:00 the period Monday 11:00 - Tuesday 18:00
 * - On Monday 13:00 the period Monday 13:00 - Wednesday 18:00
 *
 * \param theTime The local reference time
 * \param theVariable The variable containing optional extra settings
 * \return The period
 */
// ----------------------------------------------------------------------

WeatherPeriod period_until(const TextGenPosixTime& theTime, const string& theVariable)
{
  const int days = Settings::require_days(theVariable + "::days");
  const int endhour = Settings::require_hour(theVariable + "::endhour");
  const int switchhour = Settings::require_hour(theVariable + "::switchhour");

  TextGenPosixTime start(round_up(theTime));

  TextGenPosixTime end(start);
  end.ChangeByDays(days);
  end.SetHour(endhour);
  if (switchhour > 0 && start.GetHour() >= switchhour) end.ChangeByDays(1);

  WeatherPeriod period(start, end);
  return period;
}

// ----------------------------------------------------------------------
/*!
 * \brief Create period from given start time to given end time
 *
 * Required variables are
 *
 *  - \c variable::startday
 *  - \c variable::starthour
 *  - \c variable::switchhour
 *  - \c variable::days
 *  - \c variable::endhour
 *
 * The start time is formed by rounding up the given time to the
 * next full hour. Then \c startday is added to the days. If the
 * hour is atleast \c switchhour, another day is added. Finally
 * the hour is set to \c starthour.
 *
 * The end time is formed by adding \c days to the start time,
 * and then setting the hour to \c endhour.
 *
 * For example, if startday=1, starthour=18, switchhour=12,
 *                 days=2, endhour=6 we get
 *
 * - On Monday 11:00 the period Tueday 18:00 - Thursday 06:00
 * - On Monday 13:00 the period Wednesday 18:00 - Friday 06:00
 *
 * \param theTime The local reference time
 * \param theVariable The variable containing optional extra settings
 * \return The period
 */
// ----------------------------------------------------------------------

WeatherPeriod period_from_until(const TextGenPosixTime& theTime, const string& theVariable)
{
  const int startday = Settings::require_days(theVariable + "::startday");
  const int starthour = Settings::require_hour(theVariable + "::starthour");
  const int switchhour = Settings::require_hour(theVariable + "::switchhour");
  const int days = Settings::require_days(theVariable + "::days");
  const int endhour = Settings::require_hour(theVariable + "::endhour");

  TextGenPosixTime start(round_up(theTime));
  start.ChangeByDays(startday);
  if (switchhour > 0 && start.GetHour() >= switchhour) start.ChangeByDays(1);
  start.SetHour(starthour);

  TextGenPosixTime end(start);
  end.ChangeByDays(days);
  end.SetHour(endhour);

  WeatherPeriod period(start, end);
  return period;
}

}  // namespace anonymous

namespace TextGen
{
namespace WeatherPeriodFactory
{
// ----------------------------------------------------------------------
/*!
 * \brief Create a weather period of desired type
 *
 * The period information is parsed from global settings
 * in the given variable. For example, given variable name
 * \c textgen::period the factory expects to find
 * variable \c textgen::period::type and all other subvariables
 * associated with the period type.
 *
 * Throws if the type is unknown.
 *
 * \see page_aikavalit
 *
 * \param theTime The reference time
 * \param theVariable The name of the variable describing the period
 * \return The desired weather period
 */
// ----------------------------------------------------------------------

WeatherPeriod create(const TextGenPosixTime& theTime, const std::string& theVariable)
{
  const string var = theVariable + "::type";
  const string type = Settings::require_string(var);
  if (type == "now") return period_now(theTime, theVariable);
  if (type == "until") return period_until(theTime, theVariable);
  if (type == "from_until") return period_from_until(theTime, theVariable);

  throw TextGenError("WeatherPeriodFactory does not recognize period name " + type);
}
}
}  // namespace TextGen
