// ======================================================================
/*!
 * \file
 * \brief Implementation of abstract class TextGen::WeatherPeriodGenerator
 */
// ======================================================================
/*!
 * \class TextGen::WeatherPeriodGenerator
 *
 * \brief Generates a sequence of periods
 *
 * Expected usage is something like
 * \code
 * WeatherPeriodGenerator * generator = ...;
 *
 * for(WeatherPeriodGenerator::size_type i=1; i<generator->size(); i++)
 * {
 *   WeatherPeriod period = generator->period(i);
 *   ...
 * }
 * \endcode
 */
// ----------------------------------------------------------------------

#include "WeatherPeriodGenerator.h"
