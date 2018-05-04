// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::ModuloParameterAnalyzer
 */
// ======================================================================

#ifndef WEATHERANALYZER_MODULOPARAMETERANALYZER_H
#define WEATHERANALYZER_MODULOPARAMETERANALYZER_H

#include "ParameterAnalyzer.h"
#include <string>

namespace TextGen
{
class AnalyzerSources;
class WeatherArea;
class WeatherPeriodGenerator;
class WeatherResult;

class ModuloParameterAnalyzer : public ParameterAnalyzer
{
 public:
  ModuloParameterAnalyzer(const std::string& theVariable,
                          const std::string& theParameter,
                          int theModulo);

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
  ModuloParameterAnalyzer();

  const std::string itsVariable;
  const std::string itsParameter;
  const int itsModulo;
};
}

#endif  // TEXTGEN_MODULOPARAMETERANALYZER_H

// ======================================================================
