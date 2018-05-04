// ======================================================================
/*!
 * \file NFmiMetTime.cpp
 * \brief Implementation of class NFmiMetTime
 */
// ======================================================================
/*!
 * \class NFmiMetTime
 *
 * Discrete time object for meteorological observations and
 * forecasts.  Watch out for the "="-operator and constructors
 * with 'out of phase' times as the NFmiMetTime object will always
 * try to fit itself into 'the correct phase'!
 *
 * Note that NFmiMetTime is UTC time, while NFmiTime is local time.
 *
 */
// ======================================================================

#include "NFmiMetTime.h"
#include "NFmiLocation.h"
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

const NFmiMetTime NFmiMetTime::gMissingTime = NFmiMetTime(1900, 0, 0, 0, 0, 0);

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param timeStepInMinutes Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime::NFmiMetTime(const long timeStepInMinutes)
    : fTimeStepInMinutes(timeStepInMinutes), itsNegativeRange(0), itsPositiveRange(0)
{
  _setCurrent();
  SetSec(0);

  NFmiMetTime aTimeNow = *this;

  ConstructMetTime(static_cast<short>(fTimeStepInMinutes));
  if (*this > aTimeNow)
  {
    PreviousMetTime();
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from a boost posix_time
 */
// ----------------------------------------------------------------------

NFmiMetTime::NFmiMetTime(const boost::posix_time::ptime &theTime)
    : NFmiTime(theTime), fTimeStepInMinutes(), itsNegativeRange(0), itsPositiveRange(0)
{
  ConstructMetTime(1);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert to a boost posix_time
 */
// ----------------------------------------------------------------------

boost::posix_time::ptime NFmiMetTime::PosixTime() const
{
  boost::gregorian::date date(GetYear(), GetMonth(), GetDay());

  boost::posix_time::ptime utc(date,
                               boost::posix_time::hours(GetHour()) +
                                   boost::posix_time::minutes(GetMin()) +
                                   boost::posix_time::seconds(GetSec()));
  return utc;
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from a boost local_date_time
 */
// ----------------------------------------------------------------------

NFmiMetTime::NFmiMetTime(const boost::local_time::local_date_time &theLocalTime)
    : NFmiTime(theLocalTime), fTimeStepInMinutes(), itsNegativeRange(0), itsPositiveRange(0)
{
  ConstructMetTime(1);
}

NFmiMetTime NFmiMetTime::now()
{
  NFmiTime wallClock;
  NFmiMetTime timeStamp(wallClock.UTCTime());
  timeStamp.SetMin(wallClock.GetMin());
  timeStamp.SetSec(wallClock.GetSec());
  return timeStamp;
}

// ----------------------------------------------------------------------
/*!
 * \param aFmiTest Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetTime::IsEqual(const NFmiSortable &aFmiTest) const
{
  if (itsNegativeRange || itsPositiveRange)
  {
    long diff = DifferenceInMinutes(*static_cast<const NFmiTime *>(&aFmiTest));
    return diff <= itsNegativeRange && (-diff) <= itsPositiveRange;
  }
  return NFmiTime::IsEqual(aFmiTest);
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param aTime The other time being copied
 * \return The object assigned to
 * \bug NFmiStaticTime::operator= is unknown, hence self assignment
 *      should be protected.
 */
// ----------------------------------------------------------------------

NFmiMetTime &NFmiMetTime::operator=(const NFmiMetTime &aTime)
{
  if (&aTime != this)
  {
    NFmiTime::operator=(aTime);                     // retain present time step
    fTimeStepInMinutes = aTime.fTimeStepInMinutes;  // Persa korjasi Bug'in
    itsNegativeRange = aTime.itsNegativeRange;
    itsPositiveRange = aTime.itsPositiveRange;
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------
std::ostream &NFmiMetTime::Write(std::ostream &file) const
{
  file << GetYear() << " " << GetMonth() << " " << GetDay() << " " << GetHour() << " " << GetMin()
       << " " << GetSec();

  return file;
}
/*
std::ostream & NFmiMetTime::Write(std::ostream & oStream) const
{
  oStream.width( 2 );
  oStream.fill( '0' );
  oStream << GetDay() << ".";
  oStream.width( 2 );
  oStream.fill( '0' );
  oStream << GetMonth() << "." << GetYear() << " ";
  oStream.width( 2 );
  oStream.fill( '0' );
  oStream << GetHour() << ":";
  oStream.width( 2 );
  oStream.fill( '0' );
  oStream << GetMin();
  // no seconds for MetTime!

  return oStream;
}
*/

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::istream &NFmiMetTime::Read(std::istream &file)
{
  short year, month, day, hour, minutes, seconds;
  file >> year >> month >> day >> hour >> minutes >> seconds;
  SetDate(year, month, day);
  SetTime(hour, minutes, seconds);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime &NFmiMetTime::operator++()
{
  NextMetTime(GetTimeStep());
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiMetTime NFmiMetTime::operator++(int)
{
  NextMetTime(GetTimeStep());
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime &NFmiMetTime::operator--()  // prefix++
{
  PreviousMetTime(GetTimeStep());
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiMetTime NFmiMetTime::operator--(int)  // postfix++
{
  PreviousMetTime(GetTimeStep());
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::NextMetTime()
{
  if (fTimeStepInMinutes.IsDate())
  {
    NextMetTime(fTimeStepInMinutes);
  }
  else
  {
    NextMetTime(GetTimeStep());
  }
}

// ----------------------------------------------------------------------
/*!
 * \param deltaInMinutes Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::NextMetTime(const long deltaInMinutes)
{
  if (deltaInMinutes == 0) return;

  long extraMinutes = (60L * GetHour() + GetMin()) % deltaInMinutes;

  // add observation interval and delete extra minutes (if any)
  DecodeCompareValue(GetCompareValue() + deltaInMinutes - extraMinutes);
  SetSec(static_cast<short>(0));
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param thePerioid Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::NextMetTime(const NFmiTimePerioid &thePerioid)
{
  if (thePerioid.IsDate())
  {
    if ((GetMonth() + thePerioid.Month()) > 12)
    {
      SetYear(short(GetYear() + thePerioid.Year() + 1));
      SetMonth(short(GetMonth() + thePerioid.Month() - 12));
      return;
    }
    else
    {
      SetYear(short(GetYear() + thePerioid.Year()));
      SetMonth(short(GetMonth() + thePerioid.Month()));
      if (thePerioid.Day()) DecodeCompareValue(GetCompareValue() + (thePerioid.Day() * 24L * 60L));
      return;
    }
  }

  DecodeCompareValue(GetCompareValue() + (thePerioid.Hour() * 60L + thePerioid.Minute()));
}

// ----------------------------------------------------------------------
/*!
 * \param deltaInMinutes Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::PreviousMetTime(const long deltaInMinutes)
{
  if (deltaInMinutes == 0) return;

  long extraMinutes = (60L * GetHour() + GetMin()) % deltaInMinutes;

  if (extraMinutes > 0)
    // delete only extra minutes
    DecodeCompareValue(GetCompareValue() - extraMinutes);
  else
    // subtract given interval
    DecodeCompareValue(GetCompareValue() - deltaInMinutes);

  SetSec(static_cast<short>(0));
}

// ----------------------------------------------------------------------
/*!
 * \param thePerioid Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::PreviousMetTime(const NFmiTimePerioid &thePerioid)
{
  if (thePerioid.IsDate())
  {
    if ((GetMonth() - thePerioid.Month()) <= 0)
    {
      SetYear(short(GetYear() - thePerioid.Year() - 1));
      SetMonth(short(12 - GetMonth() + thePerioid.Month()));
      return;
    }
    else
    {
      SetYear(short(GetYear() - thePerioid.Year()));
      SetMonth(short(GetMonth() - thePerioid.Month()));
      if (thePerioid.Day()) DecodeCompareValue(GetCompareValue() - (thePerioid.Day() * 24L * 60L));
      return;
    }
  }

  DecodeCompareValue(GetCompareValue() - thePerioid);
}

// ----------------------------------------------------------------------
/*!
 * \param deltaInMinutes Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::NearestMetTime(const long deltaInMinutes, FmiDirection theDirect)
{
  if (deltaInMinutes == 0) return;
  long extraMinutes = (60L * GetHour() + GetMin()) % deltaInMinutes;

  if (extraMinutes == 0)  // already a meteorological time!
    return;               // add vk 940824

  if (theDirect == kForward)
    NextMetTime(deltaInMinutes);
  else if (theDirect == kBackward)
    PreviousMetTime(deltaInMinutes);
  else
  {
    if (extraMinutes < (deltaInMinutes - extraMinutes))
      PreviousMetTime(deltaInMinutes);
    else
      NextMetTime(deltaInMinutes);
  }
}

// ----------------------------------------------------------------------
/*!
 * \param timeStepInMinutes Undocumented
 * \param fSetTime Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::SetTimeStep(const long timeStepInMinutes, bool fSetTime, FmiDirection theDirect)
{
  fTimeStepInMinutes = timeStepInMinutes;
  // timestepin voi muuttaa ilman, että aikaa ruvetaan säätämään,
  // oletusarvoisesti aika säädetään kuten ennenkin
  if (fSetTime) ConstructMetTime(fTimeStepInMinutes, theDirect);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

struct tm NFmiMetTime::GetSystemTime()
{
  time_t t;
  static_cast<void>(time(&t));
  tm ret;

#ifdef _MSC_VER
  // OBS! There are no thread safe localtime(_r) or gmtime(_r) functions in MSVC++ 2008 (or before).
  // Closest things available are some what safer (but not thread safe) and with almost same
  // function
  // definitions are the localtime_s and gmtime_s -functions. Parameters are ordered otherway round
  // and their return value is success status, not struct tm pointer.

  ::gmtime_s(&ret, &t);

#else
  gmtime_r(&t, &ret);
#endif

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \param theLongitude Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::UTCTime(float theLongitude) const
{
  NFmiTime theTime(*this);
  theTime.ChangeByHours(static_cast<short>(CalcZoneDifferenceHour(theLongitude)));

  return theTime;
}

// ----------------------------------------------------------------------
/*!
 * \param theLongitude Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::LocalTime(float theLongitude) const
{
  NFmiTime theTime(*this);
  theTime.ChangeByHours(static_cast<short>(-CalcZoneDifferenceHour(theLongitude)));

  return theTime;
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::LocalTime(const NFmiLocation &theLocation) const
{
  NFmiTime theTime(*this);
  theTime.SetLocalPlace(static_cast<float>(theLocation.GetLongitude()));
  theTime.ChangeByHours(static_cast<short>(-theTime.GetZoneDifferenceHour()));

  return theTime;
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::UTCTime(const NFmiLocation &theLocation) const
{
  return UTCTime(static_cast<float>(theLocation.GetLongitude()));
}

// ----------------------------------------------------------------------
/*!
 * This method converts the UTC time represented by this into
 * a local time NFmiTime object correctly. Note that the other LocalTime
 * methods use various approximations which make them inexact.
 *
 * \return The local time
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::CorrectLocalTime() const
{
  // The UTC time
  struct ::tm utc;
  utc.tm_sec = GetSec();
  utc.tm_min = GetMin();
  utc.tm_hour = GetHour();
  utc.tm_mday = GetDay();
  utc.tm_mon = GetMonth() - 1;     // tm months start from 0
  utc.tm_year = GetYear() - 1900;  // tm years start from 1900
  utc.tm_wday = -1;
  utc.tm_yday = -1;
  utc.tm_isdst = -1;

  ::time_t epochtime = NFmiStaticTime::my_timegm(&utc);

  // As local time

  struct ::tm local;

#ifdef _MSC_VER
  // OBS! There are no thread safe localtime(_r) or gmtime(_r) functions in MSVC++ 2008 (or before).
  // Closest things available are some what safer (but not thread safe) and with almost same
  // function
  // definitions are the localtime_s and gmtime_s -functions. Parameters are ordered otherway round
  // and their return value is success status, not struct tm pointer.

  ::localtime_s(&local, &epochtime);
#else
  ::localtime_r(&epochtime, &local);
#endif

  // And build a NFmiTime from the result

  NFmiTime out(static_cast<short>(local.tm_year + 1900),
               static_cast<short>(local.tm_mon + 1),
               static_cast<short>(local.tm_mday),
               static_cast<short>(local.tm_hour),
               static_cast<short>(local.tm_min),
               static_cast<short>(local.tm_sec));

  return out;
}

// ======================================================================
