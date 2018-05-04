// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::WeatherPeriodTools
 */
// ======================================================================

#ifndef TEXTGEN_WEATHERPERIODTOOLS_H
#define TEXTGEN_WEATHERPERIODTOOLS_H

namespace TextGen
{
class WeatherPeriod;

namespace WeatherPeriodTools
{
int hours(const WeatherPeriod& thePeriod);

int countPeriods(const WeatherPeriod& thePeriod, int theStartHour, int theEndHour);

int countPeriods(const WeatherPeriod& thePeriod,
                 int theStartHour,
                 int theEndHour,
                 int theMaxStartHour,
                 int theMinEndHour);

WeatherPeriod getPeriod(const WeatherPeriod& thePeriod,
                        int theNumber,
                        int theStartHour,
                        int theEndHour);

WeatherPeriod getPeriod(const WeatherPeriod& thePeriod,
                        int theNumber,
                        int theStartHour,
                        int theEndHour,
                        int theMaxStartHour,
                        int theMinEndHour);

}  // namespace WeatherPeriodTools
}  // namespace TextGen

#endif  // TEXTGEN_WEATHERPERIODTOOLS_H

// ======================================================================
