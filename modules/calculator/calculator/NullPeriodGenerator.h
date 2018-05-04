// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::NullPeriodGenerator
 */
// ======================================================================

#ifndef TEXTGEN_NULLPERIODGENERATOR_H
#define TEXTGEN_NULLPERIODGENERATOR_H

#include "WeatherPeriodGenerator.h"
#include "WeatherPeriod.h"

#include <string>

namespace TextGen
{
class WeatherPeriod;

class NullPeriodGenerator : public WeatherPeriodGenerator
{
 public:
  typedef WeatherPeriodGenerator::size_type size_type;

  NullPeriodGenerator(const WeatherPeriod& theMainPeriod);

  virtual ~NullPeriodGenerator() {}
  virtual bool undivided() const;
  virtual size_type size() const;
  virtual WeatherPeriod period() const;
  virtual WeatherPeriod period(size_type thePeriod) const;

 private:
  NullPeriodGenerator(void);

  const WeatherPeriod itsMainPeriod;

};  // class NullPeriodGenerator

}  // namespace TextGen

#endif  // TEXTGEN_NULLPERIODGENERATOR_H

// ======================================================================
