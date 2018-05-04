// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::TimePeriod
 */
// ======================================================================

#ifndef TEXTGEN_TIME_PERIOD_H
#define TEXTGEN_TIME_PERIOD_H

#include "Glyph.h"
#include <calculator/WeatherPeriod.h>
#include <string>

namespace TextGen
{
class Dictionary;

class TimePeriod : public Glyph
{
 public:
  virtual ~TimePeriod();
  TimePeriod(const WeatherPeriod& thePeriod);
#ifdef NO_COMPILER_GENERATED
  TimePeriod(const TimePeriod& thePeriod);
  TimePeriod& operator=(const WeatherPeriod& thePeriod);
#endif
  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;
  virtual bool isDelimiter() const;

  inline const TextGenPosixTime& localStartTime() const { return itsPeriod.localStartTime(); }
  inline const TextGenPosixTime& localEndTime() const { return itsPeriod.localEndTime(); }
  inline const WeatherPeriod& weatherPeriod() const { return itsPeriod; }
 private:
  TimePeriod();
  WeatherPeriod itsPeriod;

};  // class TimePeriod

}  // namespace TextGen

#endif  // TEXTGEN_TIME_PERIOD_H

// ======================================================================
