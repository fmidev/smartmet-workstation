// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::IntervalPeriodGenerator
 */
// ======================================================================

#ifndef TEXTGEN_INTERVALPERIODGENERATOR_H
#define TEXTGEN_INTERVALPERIODGENERATOR_H

#include "WeatherPeriodGenerator.h"
#include "WeatherPeriod.h"

#include <string>
#include <vector>

namespace TextGen
{
class WeatherPeriod;

class IntervalPeriodGenerator : public WeatherPeriodGenerator
{
 public:
  typedef WeatherPeriodGenerator::size_type size_type;

  IntervalPeriodGenerator(const WeatherPeriod& theMainPeriod,
                          int theStartHour,
                          int theInterval,
                          int theMinimumInterval);

  IntervalPeriodGenerator(const WeatherPeriod& theMainPeriod, const std::string& theVariable);

  virtual ~IntervalPeriodGenerator() {}
  virtual bool undivided() const;
  virtual size_type size() const;
  virtual WeatherPeriod period() const;
  virtual WeatherPeriod period(size_type thePeriod) const;

 private:
  IntervalPeriodGenerator(void);
  void init();

  const WeatherPeriod itsMainPeriod;
  const int itsStartHour;
  const int itsInterval;
  const int itsMinimumInterval;

  std::vector<WeatherPeriod> itsPeriods;

};  // class IntervalPeriodGenerator

}  // namespace TextGen

#endif  // TEXTGEN_INTERVALPERIODGENERATOR_H

// ======================================================================
