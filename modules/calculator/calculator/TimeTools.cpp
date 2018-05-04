// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TimeTools
 */
// ======================================================================
/*!
 * \namespace TextGen::TimeTools
 *
 * \brief Various TextGenPosixTime related utilities
 *
 * This namespace contains functions which depend on TextGenPosixTime but
 * do not depend on any other textgen library classes or namespaces.
 *
 * That is, miscellaneous time utilities which could have been
 * in newbase too.
 *
 */
// ======================================================================

#include "TimeTools.h"
#include "TextGenPosixTime.h"

#include <ctime>

namespace TextGen
{
namespace TimeTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Test if the dates are the same
 *
 * \param theDate1 The first date
 * \param theDate2 The second date
 * \return True if the dates are the same
 */
// ----------------------------------------------------------------------

bool isSameDay(const TextGenPosixTime& theDate1, const TextGenPosixTime& theDate2)
{
  return (theDate1.GetDay() == theDate2.GetDay() && theDate1.GetMonth() == theDate2.GetMonth() &&
          theDate1.GetYear() == theDate2.GetYear());
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the second day is the day after the first
 *
 * \param theDate1 The first date
 * \param theDate2 The second date
 * \return True if date1+1==date2
 */
// ----------------------------------------------------------------------

bool isNextDay(const TextGenPosixTime& theDate1, const TextGenPosixTime& theDate2)
{
  TextGenPosixTime date1(theDate1);
  date1.ChangeByDays(1);

  return (date1.GetDay() == theDate2.GetDay() && date1.GetMonth() == theDate2.GetMonth() &&
          date1.GetYear() == theDate2.GetYear());
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period covers several days
 *
 * Note that a period that ends at 00:00 the next day is not considered
 * to span several days, the end hour must be atleast 01:00 if the period
 * ends the day after the start date.
 *
 * \param theDate1 The first date
 * \param theDate2 The second date
 * \return True if the period covers several days
 */
// ----------------------------------------------------------------------

bool isSeveralDays(const TextGenPosixTime& theDate1, const TextGenPosixTime& theDate2)
{
  if (isSameDay(theDate1, theDate2)) return false;
  if (isNextDay(theDate1, theDate2) && theDate2.GetHour() == 0) return false;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Round the given time down to the start of the day
 *
 * Note: In all cases the date part remains the same. That is,
 * time 00:00::00 is not assumed to be midnight, but the
 * actual desired day start.
 *
 * \param theDate The time to round
 * \return The rounded date
 */
// ----------------------------------------------------------------------

TextGenPosixTime dayStart(const TextGenPosixTime& theDate)
{
  return TextGenPosixTime(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay());
}

// ----------------------------------------------------------------------
/*!
 * \brief Round the given time up to the end of the day
 *
 * Note: DD.MM.YYYY 00:00:00 remains the same, in all other
 * cases the date increases by one. That is, the time 00:00:00
 * is assumed to be the desired midnight, not the start of the
 * date.
 *
 * \param theDate The time to round
 * \return The rounded date
 */
// ----------------------------------------------------------------------

TextGenPosixTime dayEnd(const TextGenPosixTime& theDate)
{
  TextGenPosixTime tmp(dayStart(theDate));
  if (!tmp.IsEqual(theDate)) tmp.ChangeByDays(1);
  return tmp;
}

// ----------------------------------------------------------------------
/*!
 * \brief Round the given time up to the next start of day
 *
 * Note: DD.MM.YYYY 00:00:00 does NOT remain the same, the date
 * always increases by one.
 *
 * \param theDate The time to round
 * \return The rounded date
 */
// ----------------------------------------------------------------------

TextGenPosixTime nextDay(const TextGenPosixTime& theDate)
{
  TextGenPosixTime tmp(dayStart(theDate));
  tmp.ChangeByDays(1);
  return tmp;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add the given number of hours to the given time
 *
 * \param theDate The date to add to
 * \param theHours The number of hours to add
 * \return The new date
 */
// ----------------------------------------------------------------------

TextGenPosixTime addHours(const TextGenPosixTime& theDate, int theHours)
{
  TextGenPosixTime tmp(theDate);
  tmp.ChangeByHours(theHours);
  return tmp;
}

}  // namespace TimeTools
}  // namespace TextGen

// ======================================================================
