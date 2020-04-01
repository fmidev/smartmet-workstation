#include "NFmiAreaMaskHelperStructures.h"

NFmiMacroParamValue::NFmiMacroParamValue() = default;


NFmiCalculationParams::NFmiCalculationParams() = default;

NFmiCalculationParams::NFmiCalculationParams(const NFmiPoint &theLatlon,
                                             unsigned long theLocationIndex,
                                             const NFmiMetTime &theTime,
                                             unsigned long theTimeIndex,
                                             bool crossSectionCase,
                                             float thePressureHeight)
    : itsLatlon(theLatlon),
      itsLocationIndex(theLocationIndex),
      itsTime(theTime),
      itsTimeIndex(theTimeIndex),
      fCrossSectionCase(crossSectionCase),
      itsPressureHeight(thePressureHeight)
{
}
