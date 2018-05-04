// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::WeatherPeriodFactory
 */
// ======================================================================

#ifndef TEXTGEN_WEATHERPERIODFACTORY_H
#define TEXTGEN_WEATHERPERIODFACTORY_H

#include <string>

class TextGenPosixTime;

namespace TextGen
{
class WeatherPeriod;
}

namespace TextGen
{
namespace WeatherPeriodFactory
{
TextGen::WeatherPeriod create(const TextGenPosixTime& theTime, const std::string& theVariable);

}  // namespace WeatherPeriodFactory

}  // namespace TextGen

#endif  // TEXTGEN_WEATHERPERIODFACTORY_H

// ======================================================================
