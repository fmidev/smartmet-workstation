// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::GridClimatology
 */
// ======================================================================

#ifndef TEXTGEN_GRIDCLIMATOLOGY_H
#define TEXTGEN_GRIDCLIMATOLOGY_H

#include <calculator/WeatherForecaster.h>

namespace TextGen
{
class Acceptor;
class AnalysisSources;
class WeatherArea;
class WeatherPeriodGenerator;
class WeatherResult;

class GridClimatology : public WeatherForecaster
{
 public:
  using WeatherAnalyzer::analyze;

  virtual WeatherResult analyze(const AnalysisSources& theSources,
                                const WeatherParameter& theParameter,
                                const WeatherFunction& theAreaFunction,
                                const WeatherFunction& theTimeFunction,
                                const WeatherFunction& theSubTimeFunction,
                                const WeatherArea& theArea,
                                const WeatherPeriodGenerator& thePeriods,
                                const Acceptor& theAreaAcceptor = DefaultAcceptor(),
                                const Acceptor& theTimeAcceptor = DefaultAcceptor(),
                                const Acceptor& theTester = NullAcceptor()) const;

};  // class GridClimatology

}  // namespace TextGen

#endif  // TEXTGEN_GRIDCLIMATOLOGY_H

// ======================================================================
