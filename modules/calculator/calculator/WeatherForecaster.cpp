// ======================================================================
/*!
 * \file
 * \brief Interface of abstract class TextGen::WeatherForecaster
 */
// ======================================================================
/*!
 * \class TextGen::WeatherForecaster
 *
 * \brief Provides forecast analysis services to clients
 *
 * The WeatherForecaster class provides an uniform interface for
 * weather forecast analysis services. The only thing required by the
 * API is the ability to calculate a single result either for
 * an area or for a single point. All the remaining work is in
 * initializing the class implementing the abstract interface
 * with query data, map information and so on.
 *
 * The WeatherForecaster class specializes in analyzing forecast
 * data.
 *
 * \see WeatherObserver, WeatherStatistician
 */
// ======================================================================

#include "WeatherForecaster.h"
