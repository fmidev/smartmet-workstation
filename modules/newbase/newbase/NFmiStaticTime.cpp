// ======================================================================
/*!
 * \file NFmiStaticTime.cpp
 * \brief Implementation of class NFmiStaticTime.cpp
 */
// ======================================================================
/*!
 * \class NFmiStaticTime
 *
 * Undocumented
 */
// ======================================================================

#include "NFmiStaticTime.h"
#include "NFmiValueString.h"
#include <ctime>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiStaticTime::NFmiStaticTime()
    : NFmiSortable(), fYear(), fMonth(), fDay(), fHour(), fMin(), fSec()
{
  _setCurrent();
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param from The other object being copied
 *
 * \todo Should use initialization list instead of regular code
 */
// ----------------------------------------------------------------------

NFmiStaticTime::NFmiStaticTime(const NFmiStaticTime &from)
    : NFmiSortable(), fYear(), fMonth(), fDay(), fHour(), fMin(), fSec()
{
  fYear = from.fYear;
  fMonth = from.fMonth;
  fDay = from.fDay;
  fHour = from.fHour;
  fMin = from.fMin;
  fSec = from.fSec;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param year Undocumented
 * \param month Undocumented
 * \param day Undocumented
 */
// ----------------------------------------------------------------------

NFmiStaticTime::NFmiStaticTime(const short year, const short month, const short day)
    : NFmiSortable(), fYear(), fMonth(), fDay(), fHour(), fMin(), fSec()
{
  SetDate(year, month, day);
  SetTime(0, 0, 0);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param year Undocumented
 * \param month Undocumented
 * \param day Undocumented
 * \param hour Undocumented
 * \param minute Undocumented
 * \param sec Undocumented
 * \todo This can be combined with the previous constructor
 *
 */
// ----------------------------------------------------------------------

NFmiStaticTime::NFmiStaticTime(const short year,
                               const short month,
                               const short day,
                               const short hour,
                               const short minute,
                               const short sec)
    : NFmiSortable(), fYear(), fMonth(), fDay(), fHour(), fMin(), fSec()
{
  SetDate(year, month, day);
  SetTime(hour, minute, sec);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theTime Undocumented
 * \param fMakeLocal Undocumented
 */
// ----------------------------------------------------------------------

NFmiStaticTime::NFmiStaticTime(time_t theTime, bool fMakeLocal)
    : NFmiSortable(), fYear(), fMonth(), fDay(), fHour(), fMin(), fSec()
{
  if (fMakeLocal) _set2CurrentLocalTime(theTime);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::_setCurrent()
{
  struct tm xTime = GetSystemTime();

  SetYear(static_cast<short>(xTime.tm_year + 1900));
  SetMonth(static_cast<short>(xTime.tm_mon + 1));
  SetDay(static_cast<short>(xTime.tm_mday));
  SetHour(static_cast<short>(xTime.tm_hour));
  SetMin(static_cast<short>(xTime.tm_min));
  SetSec(static_cast<short>(xTime.tm_sec));
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::_set2CurrentLocalTime(time_t theTime)
{
  struct tm xTime;

#ifdef _MSC_VER
  // OBS! There are no thread safe localtime(_r) or gmtime(_r) functions in MSVC++ 2008 (or before).
  // Closest things available are some what safer (but not thread safe) and with almost same
  // function
  // definitions are the localtime_s and gmtime_s -functions. Parameters are ordered otherway round
  // and their return value is success status, not struct tm pointer.

  ::localtime_s(&xTime, &theTime);

#else
  localtime_r(&theTime, &xTime);
#endif

  SetYear(static_cast<short>(xTime.tm_year + 1900));
  SetMonth(static_cast<short>(xTime.tm_mon + 1));
  SetDay(static_cast<short>(xTime.tm_mday));
  SetHour(static_cast<short>(xTime.tm_hour));
  SetMin(static_cast<short>(xTime.tm_min));
  SetSec(static_cast<short>(xTime.tm_sec));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

struct tm NFmiStaticTime::GetSystemTime()
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

  ::localtime_s(&ret, &t);

#else
  localtime_r(&t, &ret);
#endif
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param from The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiStaticTime &NFmiStaticTime::operator=(const NFmiStaticTime &from)
{
  fYear = from.fYear;
  fMonth = from.fMonth;
  fDay = from.fDay;
  fHour = from.fHour;
  fMin = from.fMin;
  fSec = from.fSec;
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param aFmiTest Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStaticTime::IsEqual(const NFmiSortable &aFmiTest) const
{
  if (GetYear() != (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetYear()) return false;

  if (GetMonth() != (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetMonth()) return false;

  if (GetDay() != (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetDay()) return false;

  if (GetHour() != (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetHour()) return false;

  if (GetMin() != (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetMin()) return false;

  if (GetSec() != (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetSec()) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param aFmiTest Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStaticTime::IsLessThan(const NFmiSortable &aFmiTest) const
{
  if (GetYear() > (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetYear())
    return false;
  else if (GetYear() < (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetYear())
    return true;

  if (GetMonth() > (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetMonth())
    return false;
  else if (GetMonth() < (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetMonth())
    return true;

  if (GetDay() > (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetDay())
    return false;
  else if (GetDay() < (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetDay())
    return true;

  if (GetHour() > (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetHour())
    return false;
  else if (GetHour() < (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetHour())
    return true;

  if (GetMin() > (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetMin())
    return false;
  else if (GetMin() < (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetMin())
    return true;

  if (GetSec() < (static_cast<const NFmiStaticTime *>(&aFmiTest))->GetSec()) return true;

  return false;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetMissing() { fYear = 0; }
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool NFmiStaticTime::IsMissing() { return fYear == 0; }
// ----------------------------------------------------------------------
/*!
 * \param year Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetYear(const short year)
{
  fYear = year;
  if (fYear < 100) fYear += 1900;
}

// ----------------------------------------------------------------------
/*!
 * \param month Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetMonth(const short month)
{
  if (month >= 1 && month <= 12)
    fMonth = month;
  else
    fMonth = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param day Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetDay(const short day)
{
  if (day >= 1 && day <= 31)
    fDay = day;
  else
    fDay = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param hour Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetHour(const short hour)
{
  if (hour >= 0 && hour <= 23)
    fHour = hour;
  else
    fHour = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param minute Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetMin(const short minute)
{
  if (minute >= 0 && minute <= 59)
    fMin = minute;
  else
    fMin = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param sec Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetSec(const short sec)
{
  if (sec >= 0 && sec <= 59)
    fSec = sec;
  else
    fSec = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param year Undocumented
 * \param month Undocumented
 * \param day Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetDate(const short year, const short month, const short day)
{
  SetYear(year);
  SetMonth(month);
  SetDay(day);
}

// ----------------------------------------------------------------------
/*!
 * \param year Undocumented
 * \param month Undocumented
 * \param day Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::GetDate(short &year, short &month, short &day) const
{
  year = GetYear();
  month = GetMonth();
  day = GetDay();
}

// ----------------------------------------------------------------------
/*!
 * \param hour Undocumented
 * \param minute Undocumented
 * \param sec Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::SetTime(const short hour, const short minute, const short sec)
{
  SetHour(hour);
  SetMin(minute);
  SetSec(sec);
}

// ----------------------------------------------------------------------
/*!
 * \param hour Undocumented
 * \param minute Undocumented
 * \param sec Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::GetTime(short &hour, short &minute, short &sec) const
{
  hour = GetHour();
  minute = GetMin();
  sec = GetSec();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::ReadDateTime()  // testausfunktio!!!!
{
  short y, m, d, h;
  std::cout << " Input date 'yy mm dd hh': ";
  std::cin >> y >> m >> d >> h;
  SetDate(y, m, d);
  SetTime(h, 0, 0);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::XPrint() const  // testausfunktio!!!!
{
  Print();
}

// ----------------------------------------------------------------------
/*!
 * \param str Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::XPrint(const char *str) const  // testausfunktio!!!!
{
  std::cout << str << ": ";
  XPrint();
}

// bool FromStr(const NFmiString theTimeString);

// ----------------------------------------------------------------------
/*!
 * \param theTimeString Undocumented
 * \param theTimeMask Undocumented
 * \todo theTimeString should be a const reference
 * BUG: ei toimi esim. kDDHHMM vakion kanssa, koska siirtymää ei lasketa oikein.
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::FromStr(const NFmiString theTimeString, const unsigned long theTimeMask)
{
  unsigned short theYearStep = 0;

  if (kShortYear & theTimeMask)
  {
    theYearStep = 2;
    SetYear(static_cast<short>(NFmiValueString(theTimeString.GetChars(1, theYearStep))));
  }
  else if (kLongYear & theTimeMask)
  {
    theYearStep = 4;
    SetYear(static_cast<short>(NFmiValueString(theTimeString.GetChars(1, theYearStep))));
  }

  if (kMonth & theTimeMask)
    SetMonth(static_cast<short>(NFmiValueString(theTimeString.GetChars((theYearStep + 1), 2))));

  if (kDay & theTimeMask)
    SetDay(static_cast<short>(NFmiValueString(theTimeString.GetChars(theYearStep + 3, 2))));

  if (kHour & theTimeMask)
    SetHour(static_cast<short>(NFmiValueString(theTimeString.GetChars(theYearStep + 5, 2))));

  if (kMinute & theTimeMask)
    SetMin(static_cast<short>(NFmiValueString(theTimeString.GetChars(theYearStep + 7, 2))));

  if (kSecond & theTimeMask)
    SetSec(static_cast<short>(NFmiValueString(theTimeString.GetChars(theYearStep + 9, 2))));
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeMask Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiStaticTime::ToStr(const unsigned long theTimeMask) const
{
  NFmiValueString theString;

  if (kShortYear & theTimeMask)
    theString += NFmiValueString(GetYear(), "%02d").GetChars(3, 2);
  else if (kLongYear & theTimeMask)
    theString += NFmiValueString(GetYear(), "%04d");

  if (kMonth & theTimeMask) theString += NFmiValueString(GetMonth(), "%02d");

  if (kDay & theTimeMask) theString += NFmiValueString(GetDay(), "%02d");

  if (kHour & theTimeMask) theString += NFmiValueString(GetHour(), "%02d");

  if (kMinute & theTimeMask) theString += NFmiValueString(GetMin(), "%02d");

  if (kSecond & theTimeMask) theString += NFmiValueString(GetSec(), "%02d");

  return theString;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiStaticTime::Print() const  // testausfunktio!!!!
{
  std::cout << fDay << "." << fMonth << "." << fYear << " " << fHour << ":" << fMin << ":"
            << fSec;  //<< "\n";
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param oStream The output stream to write to
 * \result The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiStaticTime::Write(std::ostream &oStream) const
{
  oStream.width(2);
  oStream.fill('0');
  oStream << GetDay() << ".";
  oStream.width(2);
  oStream.fill('0');
  oStream << GetMonth() << "." << GetYear() << " ";
  oStream.width(2);
  oStream.fill('0');
  oStream << GetHour() << ":";
  oStream.width(2);
  oStream.fill('0');
  oStream << GetMin() << ":";
  oStream.width(2);
  oStream.fill('0');
  oStream << GetSec();
  return oStream;
}

// ----------------------------------------------------------------------
/*!
 * A local help subroutine to convert a UTC tm to UTC time_t
 *
 * The original C code is by C.A. Lademann and Richard Kettlewell.
 *
 * \param t The UTC time as a tm struct
 * \return The UTC time as a time_t
 * \bug This has not been verified to work in SGI/Windows
 */
// ----------------------------------------------------------------------

::time_t NFmiStaticTime::my_timegm(struct ::tm *t)
{
#if 0

  // This is not thread safe!!!

  return ::timegm(t);  // timegm is a GNU extension

#else

  // Note: long instead of int so that epoch calculation would not overflow
  const long MINUTE = 60;
  const long HOUR = 60 * MINUTE;
  const long DAY = 24 * HOUR;
  const long YEAR = 365 * DAY;

  const long mon[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (t->tm_year < 70) return (static_cast<time_t>(-1));

  int n = t->tm_year + 1900 - 1;
  time_t epoch = (t->tm_year - 70) * YEAR +
                 ((n / 4 - n / 100 + n / 400) - (1969 / 4 - 1969 / 100 + 1969 / 400)) * DAY;

  int y = t->tm_year + 1900;
  int m = 0;
  for (int i = 0; i < t->tm_mon; i++)
  {
    epoch += mon[m] * DAY;
    if (m == 1 && y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) epoch += DAY;
    if (++m > 11)
    {
      m = 0;
      y++;
    }
  }

  epoch += (t->tm_mday - 1) * DAY;
  epoch += t->tm_hour * HOUR;
  epoch += t->tm_min * MINUTE;
  epoch += t->tm_sec;

  return epoch;
#endif
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the time as system time_t
 */
// ----------------------------------------------------------------------

::time_t NFmiStaticTime::EpochTime() const
{
  // The UTC time
  struct ::tm utc;
  utc.tm_sec = fSec;
  utc.tm_min = fMin;
  utc.tm_hour = fHour;
  utc.tm_mday = fDay;
  utc.tm_mon = fMonth - 1;     // tm months start from 0
  utc.tm_year = fYear - 1900;  // tm years start from 1900
  utc.tm_wday = -1;
  utc.tm_yday = -1;
  utc.tm_isdst = -1;

  ::time_t epochtime = NFmiStaticTime::my_timegm(&utc);  // timegm is a GNU extension
  return epochtime;
}

// ======================================================================
