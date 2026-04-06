

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
#ifndef UNIX
// Windows with ToolMaster: real implementation using MFC CDC
int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &thePixelToGridPointRatio, int theCrossSectionIsoLineDrawIndex, const NFmiVisualizationSpaceoutSettings & visualizationSettings);
float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor);
#endif // UNIX
#else
// Stub (used on Linux and when ToolMaster is disabled on Windows)
#ifndef UNIX
inline int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &thePixelToGridPointRatio, int theCrossSectionIsoLineDrawIndex, const NFmiVisualizationSpaceoutSettings & visualizationSettings)
{return 0;}
#else
// Linux: CDC is not available; callers are guarded by #ifndef UNIX
inline int ToolMasterDraw(void* /*pDC*/, NFmiIsoLineData* /*theIsoLineData*/, const NFmiRect& /*theRelViewRect*/, const NFmiRect& /*theZoomedViewRect*/, const NFmiPoint& /*thePixelToGridPointRatio*/, int /*theCrossSectionIsoLineDrawIndex*/, const NFmiVisualizationSpaceoutSettings& /*visualizationSettings*/)
{return 0;}
#endif // UNIX
inline float CalcMMSizeFactor(float /*theViewHeightInMM*/, float /*theMaxFactor*/)
{return 0;}
#endif // DISABLE_UNIRAS_TOOLMASTER
