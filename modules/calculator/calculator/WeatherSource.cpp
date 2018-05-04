// ======================================================================
/*!
 * \file
 * \brief Implementation of abstract class TextGen::WeatherSource
 */
// ======================================================================
/*!
 * \class TextGen::WeatherSource
 *
 * \brief Weather data source
 *
 * The WeatherSource class manages shared instances of weather data.
 * This class is abstract so that we may define different kinds
 * of weather sources. For example, a server would use one that
 * would automatically check that any requested data is the latest
 * possible (LatestWeatherSource), while regression tests would use
 * one that allows one to specify the data in more detail.
 *
 */
// ======================================================================

#include "WeatherSource.h"
