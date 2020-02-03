

#pragma once

#include <string>
#include "boost/shared_ptr.hpp"

class NFmiIsoLineData;
class NFmiRect;
class NFmiPoint;
class NFmiDrawParam;

const int s_rgbDefCount = 4;

#ifndef DISABLE_UNIRAS_TOOLMASTER
// Here are normal function declarations if Uniras ToolMaster is supported
int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &theGrid2PixelRatio, int theCrossSectionIsoLineDrawIndex);
void CreateClassesAndColorTableAndColorShade(float min, float max, int classCount, int defaultTableColorIndices[s_rgbDefCount], int shadingScaleIndex, int colorTableIndex, float colorWidths[s_rgbDefCount], bool fDoIsolines);
float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor);
bool IsolineDataDownSizingNeeded(const NFmiIsoLineData& theOrigIsoLineData, const NFmiPoint& theGrid2PixelRatio, NFmiPoint& theDownSizeFactorOut, const boost::shared_ptr<NFmiDrawParam> &thePossibleDrawParam = nullptr);
#else
// Here are dymmy versions of functions with dummy definitions, if Uniras ToolMaster isn't supported (must be inline because VC++ linker...)
inline int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &theGrid2PixelRatio, int theCrossSectionIsoLineDrawIndex)
{return 0;}
inline void CreateClassesAndColorTableAndColorShade(float min, float max, int classCount, int defaultTableColorIndices[s_rgbDefCount], int shadingScaleIndex, int colorTableIndex, float colorWidths[s_rgbDefCount], bool fDoIsolines)
{}
inline float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor)
{return 0;}
#endif // DISABLE_UNIRAS_TOOLMASTER
