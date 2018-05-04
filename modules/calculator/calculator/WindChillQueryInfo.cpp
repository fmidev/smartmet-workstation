#include "WindChillQueryInfo.h"
#include <newbase/NFmiMetMath.h>

WindChillQueryInfo::WindChillQueryInfo(const NFmiFastQueryInfo& theInfo)
    : NFmiFastQueryInfo(theInfo)
{
  First();

  // Parameters must be defined in this order, because
  // WindSpeedMS is a composite parameter and must be
  // handled first in GetFloatValue function

  Param(kFmiTemperature);

  long idx1 = Index();

  Param(kFmiWindSpeedMS);

  long idx2 = Index();

  itsParameterOffset = idx2 - idx1;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
/*!
 * Get the wind chill float value. The value is composed of
 * wind speed and temperature components. Offset between wind speed
 * and temperature parameters has been saved in itsParameterOffset in constructor.
 *
 * \return The value
 */
// ----------------------------------------------------------------------
float WindChillQueryInfo::GetFloatValue(unsigned long theIndex) const
{
  // GetFloatValue decodes the composite value
  float wspd = NFmiFastQueryInfo::GetFloatValue(theIndex);

  // IndexFloatValue does not decode the composite value
  float t2m = NFmiFastQueryInfo::IndexFloatValue(theIndex - itsParameterOffset);
  return FmiWindChill(wspd, t2m);
}
