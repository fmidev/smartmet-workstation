// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::WindChillParameterAnalyzer
 */
// ======================================================================

#ifndef WEATHERANALYZER_WINDCHILLPARAMETERANALYZER_H
#define WEATHERANALYZER_WINDCHILLPARAMETERANALYZER_H

#include "ParameterAnalyzer.h"
#include <string>

namespace TextGen
{
class AnalyzerSources;
class WeatherArea;
class WeatherPeriodGenerator;
class WeatherResult;

class WindChillParameterAnalyzer : public ParameterAnalyzer
{
 public:
  WindChillParameterAnalyzer(const std::string& theVariable, const std::string& theParameter);

  virtual WeatherResult analyze(const AnalysisSources& theSources,
                                const WeatherDataType& theDataType,
                                const WeatherFunction& theAreaFunction,
                                const WeatherFunction& theTimeFunction,
                                const WeatherFunction& theSubTimeFunction,
                                const WeatherArea& theArea,
                                const WeatherPeriodGenerator& thePeriods,
                                const Acceptor& theAreaAcceptor,
                                const Acceptor& theTimeAcceptor,
                                const Acceptor& theTester = NullAcceptor()) const;

 private:
  WindChillParameterAnalyzer();

  const std::string itsVariable;
  const std::string itsParameter;
};
}

#endif  // TEXTGEN_WINDCHILLPARAMETERANALYZER_H

// ======================================================================
