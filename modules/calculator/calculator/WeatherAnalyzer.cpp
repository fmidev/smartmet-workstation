// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::WeatherAnalyzer
 */
// ======================================================================
/*!
 * \class TextGen::WeatherAnalyzer
 *
 * \brief Provides analysis services to clients
 *
 * The WeatherAnalyzer class provides an uniform interface for
 * weather analysis services. The only thing required by the
 * API is the ability to calculate a single result either for
 * an area or for a single point. All the remaining work is in
 * initializing the class implementing the abstract interface
 * with query data, map information and so on.
 *
 */
// ======================================================================

#include "WeatherAnalyzer.h"
#include "Acceptor.h"
#include "AnalysisSources.h"
#include "NullPeriodGenerator.h"
#include "Settings.h"
#include "WeatherArea.h"
#include "WeatherPeriod.h"
#include "WeatherPeriodGenerator.h"
#include "WeatherResult.h"

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Analyze weather data for area
 *
 * \param theSources Analysis sources
 * \param theParameter The weather phenomenon to analyze
 * \param theAreaFunction The area function
 * \param theTimeFunction The time function
 * \param thePeriod The time period
 * \param theArea The name of the area to analyze
 * \param theAreaAcceptor The weather data acceptor in area integration
 * \param theTimeAcceptor The weather data acceptor in time integration
 * \param theTester The acceptor for Percentage calculations
 * \return The result of the analysis
 */
// ----------------------------------------------------------------------

WeatherResult WeatherAnalyzer::analyze(const AnalysisSources& theSources,
                                       const WeatherParameter& theParameter,
                                       const WeatherFunction& theAreaFunction,
                                       const WeatherFunction& theTimeFunction,
                                       const WeatherArea& theArea,
                                       const WeatherPeriod& thePeriod,
                                       const Acceptor& theAreaAcceptor,
                                       const Acceptor& theTimeAcceptor,
                                       const Acceptor& theTester) const
{
  return analyze(theSources,
                 theParameter,
                 theAreaFunction,
                 theTimeFunction,
                 NullFunction,
                 theArea,
                 NullPeriodGenerator(thePeriod),
                 theAreaAcceptor,
                 theTimeAcceptor,
                 theTester);
}

// ----------------------------------------------------------------------
/*!
 * \brief Analyze weather data for area
 *
 * \param theFakeVariable The Settings variable containing the optional
 *                        result. Useful for debugging and regression
 *                        testing.
 * \param theSources Analysis sources
 * \param theParameter The weather phenomenon to analyze
 * \param theAreaFunction The area function
 * \param theTimeFunction The time function
 * \param theSubTimeFunction The time function for subperiods
 * \param thePeriods The time periods to analyze
 * \param theArea The name of the area to analyze
 * \param theAreaAcceptor The weather data acceptor in area integration
 * \param theTimeAcceptor The weather data acceptor in time integration
 * \param theTester The acceptor for Percentage calculations
 * \return The result of the analysis
 */
// ----------------------------------------------------------------------

WeatherResult WeatherAnalyzer::analyze(const std::string& theFakeVariable,
                                       const AnalysisSources& theSources,
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
  if (Settings::isset(theFakeVariable)) return Settings::require_result(theFakeVariable);

  return analyze(theSources,
                 theParameter,
                 theAreaFunction,
                 theTimeFunction,
                 theSubTimeFunction,
                 theArea,
                 thePeriods,
                 theAreaAcceptor,
                 theTimeAcceptor,
                 theTester);
}

// ----------------------------------------------------------------------
/*!
 * \brief Analyze weather data for area
 *
 * \param theFakeVariable The Settings variable containing the optional
 *                        result. Useful for debugging and regression
 *                        testing.
 * \param theSources Analysis sources
 * \param theParameter The weather phenomenon to analyze
 * \param theAreaFunction The area function
 * \param theTimeFunction The time function
 * \param thePeriod The time period to analyze
 * \param theArea The name of the area to analyze
 * \param theAreaAcceptor The weather data acceptor in area integration
 * \param theTimeAcceptor The weather data acceptor in time integration
 * \param theTester The acceptor for Percentage calculations
 * \return The result of the analysis
 */
// ----------------------------------------------------------------------

WeatherResult WeatherAnalyzer::analyze(const std::string& theFakeVariable,
                                       const AnalysisSources& theSources,
                                       const WeatherParameter& theParameter,
                                       const WeatherFunction& theAreaFunction,
                                       const WeatherFunction& theTimeFunction,
                                       const WeatherArea& theArea,
                                       const WeatherPeriod& thePeriod,
                                       const Acceptor& theAreaAcceptor,
                                       const Acceptor& theTimeAcceptor,
                                       const Acceptor& theTester) const
{
  if (Settings::isset(theFakeVariable)) return Settings::require_result(theFakeVariable);

  return analyze(theSources,
                 theParameter,
                 theAreaFunction,
                 theTimeFunction,
                 NullFunction,
                 theArea,
                 NullPeriodGenerator(thePeriod),
                 theAreaAcceptor,
                 theTimeAcceptor,
                 theTester);
}

}  // namespace TextGen

// ======================================================================
