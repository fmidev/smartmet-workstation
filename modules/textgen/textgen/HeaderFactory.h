// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::HeaderFactory
 */
// ======================================================================

#ifndef TEXTGEN_HEADERFACTORY_H
#define TEXTGEN_HEADERFACTORY_H

#include <string>

namespace TextGen
{
class WeatherArea;
class WeatherPeriod;
}

namespace TextGen
{
class Header;

namespace HeaderFactory
{
Header create(const TextGen::WeatherArea& theArea,
              const TextGen::WeatherPeriod& thePeriod,
              const std::string& theVariable);

}  // namespace HeaderFactory
}  // namespace TextGen

#endif  // TEXTGEN_HEADERFACTORY_H

// ======================================================================
