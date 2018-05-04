#ifndef TEXTGEN_POSIX_TIME_H
#define TEXTGEN_POSIX_TIME_H

#include <newbase/NFmiStaticTime.h>

#include <string>                                     // std::string
#include <boost/date_time/posix_time/posix_time.hpp>  //include all types plus i/o
#include <boost/date_time/time_zone_base.hpp>
#include <boost/date_time/local_time/local_time.hpp>

class TextGenPosixTime
{
 public:
  TextGenPosixTime(void);
  TextGenPosixTime(time_t theTime);
  TextGenPosixTime(const boost::posix_time::ptime& theTime);
  TextGenPosixTime(const TextGenPosixTime& theTime);
  TextGenPosixTime(const NFmiStaticTime& theTime);

  TextGenPosixTime(short year, short month, short day);
  TextGenPosixTime(short year, short month, short day, short hour, short minute = 0, short sec = 0);

  void ChangeBySeconds(long sec);
  void ChangeByMinutes(long min);
  void ChangeByHours(long hour);
  void ChangeByDays(long day);

  long DifferenceInMinutes(const TextGenPosixTime& anotherTime) const;
  long DifferenceInHours(const TextGenPosixTime& anotherTime) const;
  long DifferenceInDays(const TextGenPosixTime& anotherTime) const;

  bool IsEqual(const TextGenPosixTime& anotherTime) const;
  bool IsLessThan(const TextGenPosixTime& anotherTime) const;
  bool operator<(const TextGenPosixTime& anotherTime) const;
  bool operator>(const TextGenPosixTime& anotherTime) const;
  bool operator>=(const TextGenPosixTime& anotherTime) const;
  bool operator<=(const TextGenPosixTime& anotherTime) const;
  bool operator==(const TextGenPosixTime& anotherTime) const;
  bool operator!=(const TextGenPosixTime& anotherTime) const;

  short GetYear(void) const;
  short GetMonth(void) const;
  short GetDay(void) const;
  short GetHour(void) const;
  short GetMin(void) const;
  short GetSec(void) const;

  void SetDate(const short year, const short month, const short day);
  void SetYear(short year);
  void SetMonth(short month);
  void SetDay(short day);
  void SetHour(short hour);
  void SetMin(short minute);
  void SetSec(short sec);

  std::string ToStr(const unsigned long theTimeMask) const;
  time_t EpochTime() const;
  short GetJulianDay(void) const;
  short GetWeekday(void) const;  // mon=1, tue=2,..., sat=6,  sun=7
  const boost::posix_time::ptime& PosixTime() { return istPosixTime; }
  static TextGenPosixTime UtcTime(const TextGenPosixTime& localTime);
  static TextGenPosixTime LocalTime(const TextGenPosixTime& localTime);
  // parameter isUtc tells whether theTime is in UTC or localtime:
  // it must be known when the theTime is in the proximity of daylight saving time change
  static short GetZoneDifferenceHour(const TextGenPosixTime& theTime, bool isUtc);

  static short DaysInYear(short aYear);
  static short DaysInMonth(short aMonth, short aYear);

  static void SetThreadTimeZone(const std::string& theTimeZoneId = "");
  static void ResetThreadTimeZone();

 private:
  boost::posix_time::ptime istPosixTime;

};  // class TextGenPosixTime

std::ostream& operator<<(std::ostream& os, const TextGenPosixTime& tgTime);

bool operator<=(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime);
bool operator>=(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime);
bool operator==(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime);
bool operator<(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime);
bool operator>(const NFmiStaticTime& nfmiStaticTime, const TextGenPosixTime& tgTime);

#endif  // TEXTGEN_POSIX_TIME_H
