// ======================================================================
/*!
 * \file
 * \brief Implementation of abstract class TextGen::WeatherObserver
 */
// ======================================================================
/*!
 * \class TextGen::WeatherObserver
 *
 * \brief Provides observation analysis services to clients
 *
 * The WeatherObserver class provides an uniform interface for
 * weather observation analysis services. The only thing required by the
 * API is the ability to calculate a single result either for
 * an area or for a single point. All the remaining work is in
 * initializing the class implementing the abstract interface
 * with query data, map information and so on.
 *
 * The WeatherObserver class specializes in analyzing observation
 * data.
 *
 * \see WeatherForecaster, WeatherStatistician
 */
// ======================================================================

#include "WeatherObserver.h"
