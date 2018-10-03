#pragma once

#include "NFmiParameterName.h"

class NFmiLimitChecker
{
    float itsMin;
    float itsMax;
    bool fModularFixNeeded; // suuntaan littyvät parametrit vaativat modulo korjauksen ja niiden arvo alue on aina [0, 360]
public:
    NFmiLimitChecker(float theMin, float theMax, FmiParameterName theParam);
    static bool IsModularParam(FmiParameterName theParam);
    float CheckValue(float theCheckedValue) const;
};
