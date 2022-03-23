

#pragma once

#include <string>
#include "boost/shared_ptr.hpp"

class NFmiIsoLineData;
class NFmiRect;
class NFmiPoint;
class NFmiDrawParam;
class NFmiVisualizationSpaceoutSettings;

const int s_rgbDefCount = 4;

#ifndef DISABLE_UNIRAS_TOOLMASTER
// Here are normal function declarations if Uniras ToolMaster is supported
int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &thePixelToGridPointRatio, int theCrossSectionIsoLineDrawIndex, const NFmiVisualizationSpaceoutSettings & visualizationSettings);
float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor);
#else
// Here are dummy versions of functions with dummy definitions, if Uniras ToolMaster isn't supported (must be inline because VC++ linker...)
inline int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &thePixelToGridPointRatio, int theCrossSectionIsoLineDrawIndex)
{return 0;}
inline float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor)
{return 0;}
#endif // DISABLE_UNIRAS_TOOLMASTER
