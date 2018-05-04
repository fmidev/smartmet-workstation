// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::QueryDataIntegrator
 */
// ======================================================================
/*!
 * \namespace TextGen::QueryDataIntegrator
 *
 * \brief Functions for integrating query data
 *
 * The QueryDataIntegrator namespace defines several methods
 * for performing data integration in space and time.
 *
 */
// ======================================================================

#include "QueryDataIntegrator.h"
#include "Calculator.h"
#include "QueryDataTools.h"
#include "WeatherPeriod.h"
#include "WeatherPeriodGenerator.h"

#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiIndexMask.h>
#include <newbase/NFmiIndexMaskSource.h>
#include <newbase/NFmiPoint.h>
#include "TextGenPosixTime.h"
#include <newbase/NFmiMetMath.h>

namespace TextGen
{
namespace QueryDataIntegrator
{
// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time
 *
 * Integrate over time with current location, parameter and level.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeCalculator The calculator for accumulating the result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator)
{
  theTimeCalculator.reset();

  if (!QueryDataTools::firstTime(theQI, theStartTime, theEndTime)) return kFloatMissing;

  do
  {
    const float tmp = theQI.FloatValue();
    theTimeCalculator(tmp);
  } while (theQI.NextTime() && theQI.Time() <= theEndTime);

  return theTimeCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time
 *
 * Integrate over time with current parameter and level and given latlon
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theLatLon The coordinate
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeCalculator The calculator for accumulating the result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiPoint& theLatLon,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator)
{
  theTimeCalculator.reset();

  if (!QueryDataTools::firstTime(theQI, theStartTime, theEndTime)) return kFloatMissing;

  do
  {
    const float tmp = theQI.InterpolatedValue(theLatLon);
    theTimeCalculator(tmp);
  } while (theQI.NextTime() && theQI.Time() <= theEndTime);

  return theTimeCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time with subinterval generator
 *
 * Integrate over time with current location, parameter and level.
 * The time interval is divided into subinterval by a given
 * generator and the subintervals are integrated using a separate
 * calculator.
 *
 * Note that the generated periods are inclusive, both the start
 * and end time are considered to belong into the period.
 *
 * If no subintervals can be created, a missing value is returned.
 *
 * \param theQI The query info
 * \param thePeriods The subperiod generator
 * \param theSubTimeCalculator The calculator for accumulating the subresult
 * \param theMainTimeCalculator The calculator for subresults
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const WeatherPeriodGenerator& thePeriods,
                Calculator& theSubTimeCalculator,
                Calculator& theMainTimeCalculator)
{
  if (thePeriods.undivided())
  {
    return Integrate(theQI,
                     thePeriods.period(1).utcStartTime(),
                     thePeriods.period(1).utcEndTime(),
                     theMainTimeCalculator);
  }

  // Safety against bad loop
  if (thePeriods.size() <= 0) return kFloatMissing;

  theMainTimeCalculator.reset();

  for (unsigned int i = 1; i < thePeriods.size(); i++)
  {
    WeatherPeriod period = thePeriods.period(i);

    if (!QueryDataTools::firstTime(theQI, period.utcStartTime(), period.utcEndTime()))
      return kFloatMissing;

    theSubTimeCalculator.reset();

    do
    {
      const float tmp = theQI.FloatValue();
      theSubTimeCalculator(tmp);
    } while (theQI.NextTime() && theQI.Time() <= period.utcEndTime());

    const float subresult = theSubTimeCalculator();
    theMainTimeCalculator(subresult);
  }

  return theMainTimeCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time with subinterval generator
 *
 * Integrate over time with current parameter and level and given latlon
 * The time interval is divided into subinterval by a given
 * generator and the subintervals are integrated using a separate
 * calculator.
 *
 * Note that the generated periods are inclusive, both the start
 * and end time are considered to belong into the period.
 *
 * If no subintervals can be created, a missing value is returned.
 *
 * \param theQI The query info
 * \param thePeriods The subperiod generator
 * \param theSubTimeCalculator The calculator for accumulating the subresult
 * \param theMainTimeCalculator The calculator for subresults
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiPoint& theLatLon,
                const WeatherPeriodGenerator& thePeriods,
                Calculator& theSubTimeCalculator,
                Calculator& theMainTimeCalculator)
{
  if (thePeriods.undivided())
  {
    return Integrate(theQI,
                     theLatLon,
                     thePeriods.period(1).utcStartTime(),
                     thePeriods.period(1).utcEndTime(),
                     theMainTimeCalculator);
  }

  // Safety against bad loop
  if (thePeriods.size() <= 0) return kFloatMissing;

  theMainTimeCalculator.reset();

  for (unsigned int i = 1; i < thePeriods.size(); i++)
  {
    WeatherPeriod period = thePeriods.period(i);

    if (!QueryDataTools::firstTime(theQI, period.utcStartTime(), period.utcEndTime()))
      return kFloatMissing;

    theSubTimeCalculator.reset();

    do
    {
      const float tmp = theQI.InterpolatedValue(theLatLon);
      theSubTimeCalculator(tmp);
    } while (theQI.NextTime() && theQI.Time() <= period.utcEndTime());

    const float subresult = theSubTimeCalculator();
    theMainTimeCalculator(subresult);
  }

  return theMainTimeCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over space
 *
 * Integrate over space with current time, parameter and level.
 *
 * It is assumed that the index mask was generated from a grid
 * equivalent to the one in the query data.
 *
 * \param theQI The query info
 * \param theIndexMask The mask containing the space indices
 * \param theSpaceCalculator The modifier for accumulating the result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMask& theIndexMask,
                Calculator& theSpaceCalculator)
{
  if (theIndexMask.empty()) return kFloatMissing;

  theSpaceCalculator.reset();

  for (NFmiIndexMask::const_iterator it = theIndexMask.begin(); it != theIndexMask.end(); ++it)
  {
    // possible -1 is handled by IndexFloatValue
    const unsigned long idx =
        theQI.Index(theQI.ParamIndex(), *it, theQI.LevelIndex(), theQI.TimeIndex());
    const float tmp = theQI.GetFloatValue(idx);
    theSpaceCalculator(tmp);
  }

  return theSpaceCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over space and time
 *
 * Integrate over space and time with current parameter and level.
 * Note that the integration order is different from the Integrate
 * command where the order of time and space arguments are reversed.
 * Here space integration is performed first, then time integration.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theIndexMask The  mask containing the space indices
 * \param theSpaceCalculator  The calculator for accumulating space result
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeCalculator The calculator for accumulating the time result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMask& theIndexMask,
                Calculator& theSpaceCalculator,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator)
{
  theTimeCalculator.reset();

  if (!QueryDataTools::firstTime(theQI, theStartTime, theEndTime)) return kFloatMissing;
  if (theIndexMask.empty()) return kFloatMissing;

  do
  {
    const float tmp = Integrate(theQI, theIndexMask, theSpaceCalculator);
    theTimeCalculator(tmp);
  } while (theQI.NextTime() && theQI.Time() <= theEndTime);

  return theTimeCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time and space
 *
 * Integrate over time and space with current parameter and level.
 * Note that the integration order is different from the Integrate
 * command where the order of time and space arguments are reversed.
 * Here time integration is performed first, then space integration.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeCalculator The calculator for accumulating the time result
 * \param theIndexMask The mask containing the space indices
 * \param theSpaceCalculator The calculator for accumulating the space result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator,
                const NFmiIndexMask& theIndexMask,
                Calculator& theSpaceCalculator)
{
  theSpaceCalculator.reset();

  if (theIndexMask.empty()) return kFloatMissing;

  unsigned long startindex, endindex;

  if (!QueryDataTools::findIndices(theQI, theStartTime, theEndTime, startindex, endindex))
    return kFloatMissing;

  for (NFmiIndexMask::const_iterator it = theIndexMask.begin(); it != theIndexMask.end(); ++it)
  {
    theTimeCalculator.reset();

    theQI.TimeIndex(startindex);

    do
    {
      // possible -1 is handled by IndexFloatValue
      const unsigned long idx =
          theQI.Index(theQI.ParamIndex(), *it, theQI.LevelIndex(), theQI.TimeIndex());
      const float tmp = theQI.GetFloatValue(idx);

      theTimeCalculator(tmp);
    } while (theQI.NextTime() && theQI.TimeIndex() < endindex);

    const float timeresult = theTimeCalculator();
    theSpaceCalculator(timeresult);
  }

  return theSpaceCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over space and time with time dependent masks
 *
 * Integrate over space and time with current parameter and level.
 * The space mask is time dependent and is obtained from a
 * NFmiIndexMaskSource.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theMaskSource The masks for any specific time
 * \param theSpaceCalculator The calculator for accumulating the space result
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeCalculator The calculator for accumulating the time result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMaskSource& theMaskSource,
                Calculator& theSpaceCalculator,
                const TextGenPosixTime& theStartTime,
                const TextGenPosixTime& theEndTime,
                Calculator& theTimeCalculator)
{
  theTimeCalculator.reset();

  if (!QueryDataTools::firstTime(theQI, theStartTime, theEndTime)) return kFloatMissing;

  do
  {
    const NFmiIndexMask& mask = theMaskSource.Find(theQI.Time());
    const float tmp = Integrate(theQI, mask, theSpaceCalculator);
    theTimeCalculator(tmp);
  } while (theQI.NextTime() && theQI.Time() <= theEndTime);

  return theTimeCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time, subtime, time and space
 *
 * Integrate over time and space with current parameter and level.
 * Note that the integration order is different from the Integrate
 * command where the order of time and space arguments are reversed.
 * Here time integration is performed first, then space integration.
 * Also, the time integration is split into two parts with separate
 * modifiers. The parts are generated by the given WeatherPeriodGenerator.
 *
 * \param theQI The query info
 * \param thePeriods The weather period generator
 * \param theSubTimeCalculator The calculator for accumulating sub periods
 * \param theMainTimeCalculator The calculator for accumulating sub period results
 * \param theIndexMask The mask containing the space indices
 * \param theSpaceCalculator The calculator for accumulating the space result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const WeatherPeriodGenerator& thePeriods,
                Calculator& theSubTimeCalculator,
                Calculator& theMainTimeCalculator,
                const NFmiIndexMask& theIndexMask,
                Calculator& theSpaceCalculator)
{
  if (thePeriods.undivided())
  {
    return Integrate(theQI,
                     thePeriods.period(1).utcStartTime(),
                     thePeriods.period(1).utcEndTime(),
                     theMainTimeCalculator,
                     theIndexMask,
                     theSpaceCalculator);
  }

  // Safety against bad loop
  if (thePeriods.size() <= 0) return kFloatMissing;

  theSpaceCalculator.reset();

  if (theIndexMask.empty()) return kFloatMissing;

  for (NFmiIndexMask::const_iterator it = theIndexMask.begin(); it != theIndexMask.end(); ++it)
  {
    theMainTimeCalculator.reset();

    for (unsigned int i = 1; i < thePeriods.size(); i++)
    {
      WeatherPeriod period = thePeriods.period(i);

      if (!QueryDataTools::firstTime(theQI, period.utcStartTime(), period.utcEndTime()))
        return kFloatMissing;

      theSubTimeCalculator.reset();

      do
      {
        // possible -1 is handled by IndexFloatValue
        const unsigned long idx =
            theQI.Index(theQI.ParamIndex(), *it, theQI.LevelIndex(), theQI.TimeIndex());
        const float tmp = theQI.GetFloatValue(idx);

        theSubTimeCalculator(tmp);
      } while (theQI.NextTime() && theQI.Time() <= period.utcEndTime());

      const float subtimeresult = theSubTimeCalculator();
      theMainTimeCalculator(subtimeresult);
    }

    const float timeresult = theMainTimeCalculator();
    theSpaceCalculator(timeresult);
  }

  return theSpaceCalculator();
}

}  // namespace QueryDataIntegrator
}  // namespace TextGen

// ======================================================================
