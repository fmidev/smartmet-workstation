// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::GridForecaster
 */
// ======================================================================
/*!
 * \class TextGen::GridForecaster
 *
 * \brief Provides forecast analysis services to clients
 *
 * The GridForecaster class provides weather forecast analysis
 * services. All analysis is performed by analyzing the main
 * grid forecast. Probabilities are calculated by analyzing the
 * gridded forecasts.
 *
 */
// ======================================================================

#include "GridForecaster.h"
#include "Acceptor.h"
#include "AnalysisSources.h"
#include "ParameterAnalyzerFactory.h"
#include "WeatherArea.h"
#include "WeatherPeriod.h"
#include "WeatherPeriodGenerator.h"
#include "WeatherResult.h"

#include <boost/shared_ptr.hpp>

using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Analyze weather forecast for area
 *
 * \param theSources Analysis sources
 * \param theParameter The weather phenomenon to analyze
 * \param theAreaFunction The area function
 * \param theTimeFunction The time function
 * \param theSubTimeFunction The time function for subperiods
 * \param theArea The name of the area to analyze
 * \param thePeriods The time period to analyze
 * \param theAreaAcceptor The weather data acceptor in area integration
 * \param theTimeAcceptor The weather data acceptor in time integration
 * \param theTester The acceptor for Percentage calculations
 * \return The result of the analysis
 */
// ----------------------------------------------------------------------

WeatherResult GridForecaster::analyze(const AnalysisSources& theSources,
                                      const WeatherParameter& theParameter,
                                      const WeatherFunction& theAreaFunction,
                                      const WeatherFunction& theTimeFunction,
                                      const WeatherFunction& theSubTimeFunction,
                                      const WeatherArea& theArea,
                                      const WeatherPeriodGenerator& thePeriods,
                                      const Acceptor& theAreaAcceptor,
                                      const Acceptor& theTimeAcceptor,
                                      const Acceptor& theTester) const
{
  shared_ptr<ParameterAnalyzer> analyzer(ParameterAnalyzerFactory::create(theParameter));

  return analyzer->analyze(theSources,
                           Forecast,
                           theAreaFunction,
                           theTimeFunction,
                           theSubTimeFunction,
                           theArea,
                           thePeriods,
                           theAreaAcceptor,
                           theTimeAcceptor,
                           theTester);
}

}  // namespace TextGen

// ======================================================================
