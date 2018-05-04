#pragma once

#include "NFmiColor.h"
#include "matrix3d.h"

namespace ToolMasterColorCube 
{
    Matrix3D<std::pair<int, COLORREF> >* UsedColorsCube();
    int UsedColorTableIndex();
    int UsedHollowColorIndex();
    void InitDefaultColorTable(bool fToolMasterAvailable);
    int RgbToColorIndex(float RGBcolors[3]);
    int RgbToColorIndex(COLORREF color);
    int RgbToColorIndex(const NFmiColor& color);
    NFmiColor ColorIndexToRgb(int theColorIndex);
}