// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::WeatherPeriod
 */
// ======================================================================
/*!
 * \class TextGen::WeatherPeriod
 *
 * \brief Representation of an analysis time period
 *
 * A WeatherPeriod stores the start and end times of analysis.
 * The start and end times may be equal, but is rarely used.
 *
 * The class is intended for storage only, the provided accessors
 * are to be used externally in actual analysis code.
 *
 * Any additional functions, if any are needed, are to be placed
 * into a new WeatherPeriodTools namespace. It is possible that
 * a function for testing inclusion will be added to such a namespace
 * later on. The intention is to avoid cluttering this small class
 * with a lot of methods, since it is forseeable that all kinds
 * of interval splitting algorithms will also be needed.
 *
 */
// ======================================================================

#include "WeatherPeriod.h"
#include "TimeTools.h"
#include "TextGenError.h"
#include <cassert>
#include <iostream>  // std::cout
#include <sstream>   // std::stringstream

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * Construction is possible only by explicitly stating the
 * start and end times, or by copy constructing. The void
 * constructor is intentionally disabled.
 *
 * It is assumed that the proper timezone has been set
 * prior to constructing the object. This can be done
 * for example by using
 * \code
 * putenv("TZ=Europe/Helsinki");
 * tzset();
 * \endcode
 * See the man-pages for \c tzset for more information.
 * In Linux the available timezones are listed in
 * directory \c /usr/share/zoneinfo
 *
 * \param theLocalStartTime The start time of the period
 * \param theLocalEndTime The end time of the period
 */
// ----------------------------------------------------------------------

WeatherPeriod::WeatherPeriod(const TextGenPosixTime& theLocalStartTime,
                             const TextGenPosixTime& theLocalEndTime)
    : itsLocalStartTime(theLocalStartTime),
      itsLocalEndTime(theLocalEndTime),
      itsUtcStartTime(TextGenPosixTime::UtcTime(theLocalStartTime)),
      itsUtcEndTime(TextGenPosixTime::UtcTime(theLocalEndTime))
{
  if (theLocalEndTime.IsLessThan(theLocalStartTime))
    throw TextGenError("WeatherPeriod: end time must be after start time");
}

// ----------------------------------------------------------------------
/*!
 * \brief Start time accessor
 *
 * \return The start time
 */
// ----------------------------------------------------------------------

const TextGenPosixTime& WeatherPeriod::localStartTime() const { return itsLocalStartTime; }
// ----------------------------------------------------------------------
/*!
 * \brief End time accessor
 *
 * \return The end time
 */
// ----------------------------------------------------------------------

const TextGenPosixTime& WeatherPeriod::localEndTime() const { return itsLocalEndTime; }
// ----------------------------------------------------------------------
/*!
 * \brief Start time accessor
 *
 * \return The start time
 */
// ----------------------------------------------------------------------

const TextGenPosixTime& WeatherPeriod::utcStartTime() const { return itsUtcStartTime; }
// ----------------------------------------------------------------------
/*!
 * \brief End time accessor
 *
 * \return The end time
 */
// ----------------------------------------------------------------------

const TextGenPosixTime& WeatherPeriod::utcEndTime() const { return itsUtcEndTime; }
// ----------------------------------------------------------------------
/*!
 * \brief Less-than comparison for TextGen::WeatherPeriod
 *
 * We define < to mean the lexicographic ordering based on the
 * start time and then the end time.
 *
 * \param theRhs The right hand side
 * \return True if the theLhs < theRhs
 */
// ----------------------------------------------------------------------

bool WeatherPeriod::operator<(const TextGen::WeatherPeriod& theRhs) const
{
  if (utcStartTime() != theRhs.utcStartTime()) return (utcStartTime() < theRhs.utcStartTime());
  return (utcEndTime() < theRhs.utcEndTime());
}

}  // namespace TextGen

// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison for TextGen::WeatherPeriod
 *
 * \param theLhs The left hand side
 * \param theRhs The right hand side
 * \return True if the periods are equal
 */
// ----------------------------------------------------------------------

bool operator==(const TextGen::WeatherPeriod& theLhs, const TextGen::WeatherPeriod& theRhs)
{
  return (theLhs.utcStartTime() == theRhs.utcStartTime() &&
          theLhs.utcEndTime() == theRhs.utcEndTime());
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison for TextGen::WeatherPeriod
 *
 * \param theLhs The left hand side
 * \param theRhs The right hand side
 * \return True if the periods are not equal
 */
// ----------------------------------------------------------------------

bool operator!=(const TextGen::WeatherPeriod& theLhs, const TextGen::WeatherPeriod& theRhs)
{
  return !(theLhs == theRhs);
}

// ======================================================================
