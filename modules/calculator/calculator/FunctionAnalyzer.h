// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::FunctionAnalyzer
 */
// ======================================================================
/*!
 * \class TextGen::FunctionAnalyzer
 *
 * \brief Function analysis
 *
 */
// ======================================================================

#ifndef TEXTGEN_FUNCTIONANALYZER_H
#define TEXTGEN_FUNCTIONANALYZER_H

#include "WeatherDataType.h"
#include <string>

namespace TextGen
{
class Acceptor;
class AnalysisSources;
class Calculator;
class WeatherArea;
class WeatherPeriodGenerator;
class WeatherResult;

class FunctionAnalyzer
{
 public:
  virtual ~FunctionAnalyzer() {}
  virtual WeatherResult analyze(const AnalysisSources& theSources,
                                const WeatherDataType& theDataType,
                                const WeatherArea& theArea,
                                const WeatherPeriodGenerator& thePeriods,
                                const Acceptor& theAreaAcceptor,
                                const Acceptor& theTimeAcceptor,
                                const Acceptor& theTester,
                                const std::string& theDataName,
                                const std::string& theParameterName) const = 0;

};  // class FunctionAnalyzer

}  // namespace TextGen

#endif  // TEXTGEN_FUNCTIONANALYZER_H

// ======================================================================
