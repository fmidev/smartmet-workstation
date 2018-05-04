

#pragma once

#include <string>

class NFmiIsoLineData;
class NFmiRect;
class NFmiPoint;

const int s_rgbDefCount = 4;

int mar002(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &theGrid2PixelRatio, int theCrossSectionIsoLineDrawIndex);
void CreateClassesAndColorTableAndColorShade(float min, float max, int classCount, int defaultTableColorIndices[s_rgbDefCount], int shadingScaleIndex, int colorTableIndex, float colorWidths[s_rgbDefCount], bool fDoIsolines);
float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor);
