// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::WeatherPeriod
 */
// ======================================================================

#ifndef TEXTGEN_WEATHERPERIOD_H
#define TEXTGEN_WEATHERPERIOD_H

#include "TextGenPosixTime.h"

namespace TextGen
{
class WeatherPeriod
{
 public:
#ifdef NO_COMPILER_GENERATED
  ~WeatherPeriod();
  WeatherPeriod(const WeatherPeriod& thePeriod);
  WeatherPeriod& operator=(const WeatherPeriod& thePeriod);
#endif

  WeatherPeriod(const TextGenPosixTime& theLocalStartTime, const TextGenPosixTime& theLocalEndTime);

  const TextGenPosixTime& localStartTime() const;
  const TextGenPosixTime& localEndTime() const;

  const TextGenPosixTime& utcStartTime() const;
  const TextGenPosixTime& utcEndTime() const;

  bool operator<(const WeatherPeriod& theRhs) const;

 private:
  WeatherPeriod();

  TextGenPosixTime itsLocalStartTime;
  TextGenPosixTime itsLocalEndTime;

  TextGenPosixTime itsUtcStartTime;
  TextGenPosixTime itsUtcEndTime;
};  // class WeatherPeriod

}  // namespace TextGen

// Free functions

bool operator==(const TextGen::WeatherPeriod& theLhs, const TextGen::WeatherPeriod& theRhs);

bool operator!=(const TextGen::WeatherPeriod& theLhs, const TextGen::WeatherPeriod& theRhs);

#endif  // TEXTGEN_WEATHERPERIOD_H

// ======================================================================
