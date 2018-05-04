// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::ClimatologyTools
 */
// ======================================================================

#ifndef TEXTGEN_CLIMATOLOGYTOOLS_H
#define TEXTGEN_CLIMATOLOGYTOOLS_H

#include <calculator/WeatherPeriod.h>
#include <calculator/AnalysisSources.h>
#include <string>

namespace TextGen
{
namespace ClimatologyTools
{
WeatherPeriod getClimatologyPeriod(const TextGen::WeatherPeriod& thePeriod,
                                   const std::string& theDataName,
                                   const TextGen::AnalysisSources& theSources);
}  // namespace ClimatologyTools
}  // namespace TextGen

#endif  // TEXTGEN_CLIMATOLOGYTOOLS_H

// ======================================================================
