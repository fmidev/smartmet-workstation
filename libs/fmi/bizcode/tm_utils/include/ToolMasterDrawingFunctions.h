

#pragma once

#include <string>

class NFmiIsoLineData;
class NFmiRect;
class NFmiPoint;

const int s_rgbDefCount = 4;

#ifndef DISABLE_UNIRAS_TOOLMASTER
// Here are normal function declarations if Uniras ToolMaster is supported
int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &theGrid2PixelRatio, int theCrossSectionIsoLineDrawIndex);
void CreateClassesAndColorTableAndColorShade(float min, float max, int classCount, int defaultTableColorIndices[s_rgbDefCount], int shadingScaleIndex, int colorTableIndex, float colorWidths[s_rgbDefCount], bool fDoIsolines);
float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor);
#else
// Here are dymmy versions of functions with dummy definitions, if Uniras ToolMaster isn't supported (must be inline because VC++ linker...)
inline int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &theGrid2PixelRatio, int theCrossSectionIsoLineDrawIndex)
{return 0;}
inline void CreateClassesAndColorTableAndColorShade(float min, float max, int classCount, int defaultTableColorIndices[s_rgbDefCount], int shadingScaleIndex, int colorTableIndex, float colorWidths[s_rgbDefCount], bool fDoIsolines)
{}
inline float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor)
{return 0;}
#endif // DISABLE_UNIRAS_TOOLMASTER
