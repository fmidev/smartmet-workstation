#include "TextGenPosixTime.h"
#include <newbase/NFmiStaticTime.h>
#include <macgyver/TimeZoneFactory.h>
#include <boost/thread.hpp>

#include <iostream>  // std::cout
#include <sstream>   // std::stringstream

using namespace boost::posix_time;
using namespace boost::local_time;
using namespace boost::gregorian;
using namespace std;

#define DEFAULT_TZ_ID "Europe/Helsinki"

static boost::thread_specific_ptr<std::string> tls;

void release_timezone_id()
{
  if (tls.get()) delete tls.release();
}

string& get_timezone_id()
{
  if (!tls.get()) tls.reset(new std::string());

  return *tls;
}

TextGenPosixTime::TextGenPosixTime(void) : istPosixTime(second_clock::local_time()) {}
TextGenPosixTime::TextGenPosixTime(const boost::posix_time::ptime& theTime) : istPosixTime(theTime)
{
}

TextGenPosixTime::TextGenPosixTime(time_t theTime) : istPosixTime(from_time_t(theTime)) {}
TextGenPosixTime::TextGenPosixTime(const TextGenPosixTime& theTime)
    : istPosixTime(theTime.istPosixTime)
{
}

TextGenPosixTime::TextGenPosixTime(const NFmiStaticTime& theTime)
    : istPosixTime(date(theTime.GetYear(), theTime.GetMonth(), theTime.GetDay()),
                   time_duration(theTime.GetHour(), theTime.GetMin(), theTime.GetSec()))
{
}

TextGenPosixTime::TextGenPosixTime(short year, short month, short day)
    : istPosixTime(date(year, month, day), time_duration(0, 0, 0))
{
}

TextGenPosixTime::TextGenPosixTime(
    short year, short month, short day, short hour, short minute /*=0*/, short sec /*=0*/)
{
  time_duration td(hour, minute, sec, 0);
  stringstream ss;
  ss << year << "-" << month << "-" << day;
  date d(from_simple_string(ss.str()));

  istPosixTime = ptime(d, td);
}

void TextGenPosixTime::ChangeBySeconds(long sec) { istPosixTime += seconds(sec); }
void TextGenPosixTime::ChangeByMinutes(long min) { istPosixTime += minutes(min); }
void TextGenPosixTime::ChangeByHours(long hour) { istPosixTime += hours(hour); }
void TextGenPosixTime::ChangeByDays(long day) { istPosixTime += days(day); }
long TextGenPosixTime::DifferenceInMinutes(const TextGenPosixTime& anotherTime) const
{
  time_duration td(istPosixTime - anotherTime.istPosixTime);

  return td.total_seconds() / 60;
}

long TextGenPosixTime::DifferenceInHours(const TextGenPosixTime& anotherTime) const
{
  time_duration td(istPosixTime - anotherTime.istPosixTime);

  return td.total_seconds() / 3600;
}

long TextGenPosixTime::DifferenceInDays(const TextGenPosixTime& anotherTime) const
{
  time_duration td(istPosixTime - anotherTime.istPosixTime);

  return td.total_seconds() / 86400;
}

bool TextGenPosixTime::IsEqual(const TextGenPosixTime& anotherTime) const
{
  return (istPosixTime == anotherTime.istPosixTime);
}

bool TextGenPosixTime::IsLessThan(const TextGenPosixTime& anotherTime) const
{
  return (istPosixTime < anotherTime.istPosixTime);
}

bool TextGenPosixTime::operator<(const TextGenPosixTime& anotherTime) const
{
  return (istPosixTime < anotherTime.istPosixTime);
}

bool TextGenPosixTime::operator>(const TextGenPosixTime& anotherTime) const
{
  return (istPosixTime > anotherTime.istPosixTime);
}

bool TextGenPosixTime::operator>=(const TextGenPosixTime& anotherTime) const
{
  return (istPosixTime >= anotherTime.istPosixTime);
}

bool TextGenPosixTime::operator<=(const TextGenPosixTime& anotherTime) const
{
  return (istPosixTime <= anotherTime.istPosixTime);
}

bool TextGenPosixTime::operator==(const TextGenPosixTime& anotherTime) const
{
  return (istPosixTime == anotherTime.istPosixTime);
}

bool TextGenPosixTime::operator!=(const TextGenPosixTime& anotherTime) const
{
  return (istPosixTime != anotherTime.istPosixTime);
}

void TextGenPosixTime::SetDate(const short year, const short month, const short day)
{
  date d(year, month, day);
  time_duration td(istPosixTime.time_of_day());

  istPosixTime = ptime(d, td);
}

void TextGenPosixTime::SetYear(short year)
{
  date d(istPosixTime.date());
  time_duration td(istPosixTime.time_of_day());

  istPosixTime = ptime(date(year, d.month(), d.day()), td);
}

void TextGenPosixTime::SetMonth(short month)
{
  date d(istPosixTime.date());
  time_duration td(istPosixTime.time_of_day());

  istPosixTime = ptime(date(d.year(), month, d.day()), td);
}

void TextGenPosixTime::SetDay(short day)
{
  date d(istPosixTime.date());
  time_duration td(istPosixTime.time_of_day());

  istPosixTime = ptime(date(d.year(), d.month(), day), td);
}

void TextGenPosixTime::SetHour(short hour)
{
  date d(istPosixTime.date());
  time_duration td(istPosixTime.time_of_day());

  istPosixTime = ptime(d, time_duration(hour, td.minutes(), td.seconds()));
}

void TextGenPosixTime::SetMin(short minute)
{
  date d(istPosixTime.date());
  time_duration td(istPosixTime.time_of_day());

  istPosixTime = ptime(d, time_duration(td.hours(), minute, td.seconds()));
}

void TextGenPosixTime::SetSec(short sec)
{
  date d(istPosixTime.date());
  time_duration td(istPosixTime.time_of_day());

  istPosixTime = ptime(d, time_duration(td.hours(), td.minutes(), sec));
}

short TextGenPosixTime::GetYear(void) const { return istPosixTime.date().year(); }
short TextGenPosixTime::GetMonth(void) const { return istPosixTime.date().month(); }
short TextGenPosixTime::GetDay(void) const { return istPosixTime.date().day(); }
short TextGenPosixTime::GetHour(void) const { return istPosixTime.time_of_day().hours(); }
short TextGenPosixTime::GetMin(void) const { return istPosixTime.time_of_day().minutes(); }
short TextGenPosixTime::GetSec(void) const { return istPosixTime.time_of_day().seconds(); }
std::string TextGenPosixTime::ToStr(const unsigned long theTimeMask) const
{
  stringstream ss;

  if (kShortYear & theTimeMask)
    ss << std::setw(2) << (GetYear() - (GetYear() < 2000 ? 1900 : 2000));
  else if (kLongYear & theTimeMask)
    ss << std::setw(4) << GetYear();

  if (kMonth & theTimeMask) ss << std::setw(2) << setfill('0') << GetMonth();

  if (kDay & theTimeMask) ss << std::setw(2) << setfill('0') << GetDay();

  if (kHour & theTimeMask) ss << std::setw(2) << setfill('0') << GetHour();

  if (kMinute & theTimeMask) ss << std::setw(2) << setfill('0') << GetMin();

  if (kSecond & theTimeMask) ss << std::setw(2) << setfill('0') << GetSec();

  return ss.str();
}

short TextGenPosixTime::GetZoneDifferenceHour(const TextGenPosixTime& theTime, bool isUtc)
{
  std::string timeZoneId(get_timezone_id().empty() ? DEFAULT_TZ_ID : get_timezone_id());
  const boost::local_time::time_zone_ptr timeZone =
      Fmi::TimeZoneFactory::instance().time_zone_from_string(timeZoneId);

  short dst_offset(timeZone->base_utc_offset().hours() + timeZone->dst_offset().hours());
  short normal_time_offset(timeZone->base_utc_offset().hours());
  ptime dst_local_start_time(timeZone->dst_local_start_time(theTime.GetYear()));
  ptime dst_local_end_time(timeZone->dst_local_end_time(theTime.GetYear()));
  bool dst_on(false);

  // in the southern hemisphere dst is in wintertime
  if (dst_local_start_time > dst_local_end_time)
  {
    if (isUtc)
      dst_on = (theTime.istPosixTime + hours(dst_offset) >= dst_local_start_time ||
                theTime.istPosixTime + hours(dst_offset) < dst_local_end_time);
    else
      dst_on = (theTime.istPosixTime >= dst_local_start_time ||
                theTime.istPosixTime < dst_local_end_time);
  }
  else
  {
    if (isUtc)
      dst_on = (theTime.istPosixTime + hours(dst_offset) >= dst_local_start_time &&
                theTime.istPosixTime + hours(dst_offset) < dst_local_end_time);
    else
      dst_on = (theTime.istPosixTime >= dst_local_start_time &&
                theTime.istPosixTime < dst_local_end_time);
  }

  if (dst_on)
    return dst_offset;
  else
    return normal_time_offset;
}

time_t TextGenPosixTime::EpochTime() const
{
  ptime time_t_epoch(date(1970, 1, 1));

  return (istPosixTime - time_t_epoch).total_seconds();
}

short TextGenPosixTime::GetWeekday(void) const  // mon=1, tue=2,..., sat=6,  sun=7
{
  short retval(istPosixTime.date().day_of_week());

  if (retval == 0) retval = 7;

  return retval;
}

short TextGenPosixTime::GetJulianDay(void) const { return istPosixTime.date().julian_day(); }
short TextGenPosixTime::DaysInYear(const short aYear)
{
  date d(aYear, 12, 1);

  return d.end_of_month().day_of_year();
}

short TextGenPosixTime::DaysInMonth(const short aMonth, const short aYear)
{
  date d(aYear, aMonth, 1);

  return d.end_of_month().day();
}

TextGenPosixTime TextGenPosixTime::UtcTime(const TextGenPosixTime& localTime)
{
  short zdh = TextGenPosixTime::GetZoneDifferenceHour(localTime, false);

  boost::posix_time::ptime utcptime = localTime.istPosixTime - hours(zdh);

  return TextGenPosixTime(utcptime);
}

//
TextGenPosixTime TextGenPosixTime::LocalTime(const TextGenPosixTime& utcTime)
{
  short zdh = TextGenPosixTime::GetZoneDifferenceHour(utcTime, true);

  boost::posix_time::ptime localptime = utcTime.istPosixTime + hours(zdh);

  return TextGenPosixTime(localptime);
}

void TextGenPosixTime::SetThreadTimeZone(const std::string& theTimeZoneId /*= ""*/)
{
  if (theTimeZoneId.empty())
    release_timezone_id();
  else
    get_timezone_id() = theTimeZoneId;
}

void TextGenPosixTime::ResetThreadTimeZone() { release_timezone_id(); }
std::ostream& operator<<(std::ostream& os, const TextGenPosixTime& tgTime)
{
  os << std::setw(2) << std::setfill('0') << std::right << tgTime.GetDay() << "." << std::setw(2)
     << std::setfill('0') << std::right << tgTime.GetMonth() << "." << tgTime.GetYear() << " "
     << std::setw(2) << std::setfill('0') << std::right << tgTime.GetHour() << ":" << std::setw(2)
     << std::setfill('0') << std::right << tgTime.GetMin() << ":" << std::setw(2)
     << std::setfill('0') << std::right << tgTime.GetSec();

  return os;
}

bool operator<=(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime)
{
  return TextGenPosixTime(nfmiStaticTime) <= tgTime;
}

bool operator>=(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime)
{
  return TextGenPosixTime(nfmiStaticTime) >= tgTime;
}

bool operator==(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime)
{
  return TextGenPosixTime(nfmiStaticTime) == tgTime;
}

bool operator<(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime)
{
  return TextGenPosixTime(nfmiStaticTime) < tgTime;
}

bool operator>(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime)
{
  return TextGenPosixTime(nfmiStaticTime) > tgTime;
}
