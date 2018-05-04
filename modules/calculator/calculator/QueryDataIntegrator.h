// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::QueryDataIntegrator
 */
// ======================================================================

#ifndef TEXTGEN_QUERYDATAINTEGRATOR_H
#define TEXTGEN_QUERYDATAINTEGRATOR_H

class NFmiFastQueryInfo;
class NFmiIndexMask;
class NFmiIndexMaskSource;
class NFmiPoint;
class TextGenPosixTime;

namespace TextGen
{
class Calculator;
class WeatherPeriodGenerator;

namespace QueryDataIntegrator
{
// Integrate over time with current location & param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator);

// Integrate over time with current param & level and given latlon

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiPoint& theLatLon,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator);

// Integrate over time with subinterval generator w/ current location/param/level

float Integrate(NFmiFastQueryInfo& theQI,
                const WeatherPeriodGenerator& thePeriods,
                Calculator& theSubTimeCalculator,
                Calculator& theMainTimeCalculator);

// Integrate over time with subinterval generator w/ current param/level for given latlon

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiPoint& theLatLon,
                const WeatherPeriodGenerator& thePeriods,
                Calculator& theSubTimeCalculator,
                Calculator& theMainTimeCalculator);

// Integrate over grid with current param & time & level

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMask& theIndexMask,
                Calculator& theSpaceCalculator);

// Integrate over grid and time with current param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMask& theIndexMask,
                Calculator& theSpaceCalculator,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator);

// Integrate over time and grid with current param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator,
                const NFmiIndexMask& theIndexMask,
                Calculator& theSpaceCalculator);

// Integrate over grid and time with time dependend mask, current P & L

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMaskSource& theMaskSource,
                Calculator& theSpaceCalculator,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator);

// Integrate over time, sub time and grid with current param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const WeatherPeriodGenerator& thePeriods,
                Calculator& theSubTimeCalculator,
                Calculator& theMainTimeCalculator,
                const NFmiIndexMask& theIndexMask,
                Calculator& theSpaceCalculator);

}  // namespace QueryDataIntegrator
}  // namespace TextGen

#endif  // TEXTGEN_QUERYDATAINTEGRATOR_H

// ======================================================================
