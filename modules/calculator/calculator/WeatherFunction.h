// ======================================================================
/*!
 * \file
 * \brief Enumeration of TextGen::WeatherFunction
 */
// ======================================================================

#ifndef TEXTGEN_WEATHERFUNCTION_H
#define TEXTGEN_WEATHERFUNCTION_H

namespace TextGen
{
enum WeatherFunction
{
  NullFunction,
  Mean,
  Maximum,
  Minimum,
  Median,
  Sum,
  StandardDeviation,
  Percentage,
  Count,
  Change,
  Trend

};  // enum Weatherfunction

}  // namespace TextGen

#endif  // TEXTGEN_WEATHERFUNCTION_H

// ======================================================================
