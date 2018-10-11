#include "NFmiLimitChecker.h"
#include "NFmiGlobals.h"
#include <math.h>

NFmiLimitChecker::NFmiLimitChecker(float theMin, float theMax, FmiParameterName theParam)
    :itsMin(theMin)
    , itsMax(theMax)
    , fModularFixNeeded(IsModularParam(theParam))
{}

bool NFmiLimitChecker::IsModularParam(FmiParameterName theParam)
{
    return (theParam == kFmiWindDirection) || (theParam == kFmiWaveDirection);
}

float NFmiLimitChecker::CheckValue(float theCheckedValue) const
{
    if(theCheckedValue == kFloatMissing)
        return kFloatMissing;

    if(fModularFixNeeded)
    {
        const float directionModuloValue = 360.f;
        theCheckedValue = std::fmodf(theCheckedValue, directionModuloValue);
        if(theCheckedValue < 0)
            theCheckedValue += directionModuloValue;
    }
    else
    {
        if(theCheckedValue > itsMax)
            theCheckedValue = itsMax;
        else if(theCheckedValue < itsMin)
            theCheckedValue = itsMin;
    }
    return theCheckedValue;
}
