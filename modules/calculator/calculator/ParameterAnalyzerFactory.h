// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::ParameterAnalyzerFactory
 */
// ======================================================================

#ifndef TEXTGEN_PARAMETERANALYZERFACTORY_H
#define TEXTGEN_PARAMETERANALYZERFACTORY_H

#include "ParameterAnalyzer.h"
#include "WeatherParameter.h"

namespace TextGen
{
namespace ParameterAnalyzerFactory
{
ParameterAnalyzer* create(TextGen::WeatherParameter theParameter);

}  // namespace ParameterAnalyzerFactory
}  // namespace TextGen

#endif  // TEXTGEN_PARAMETERANALYZERFACTORY_H

// ======================================================================
