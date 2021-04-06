#pragma once

#include "NFmiColor.h"
#include "matrix3d.h"

namespace ToolMasterColorCube 
{
#ifndef DISABLE_UNIRAS_TOOLMASTER
    // Here are normal function declarations if Uniras ToolMaster is supported
    Matrix3D<std::pair<int, COLORREF>>* UsedColorsCube();
    int UsedDefaultColorTableIndex();
    int UsedHollowColorIndex();
    int ColorCubeColorChannelSize();
    int SpecialColorCountInColorTableStart();
    void InitDefaultColorTable(bool fToolMasterAvailable);
    int RgbToColorIndex(float RGBcolors[3]);
    int RgbToColorIndex(COLORREF color);
    int RgbToColorIndex(const NFmiColor& color);
    NFmiColor ColorIndexToRgb(int theColorIndex);
    NFmiColor ColorToActualCubeColor(const NFmiColor& color);
    bool IsColorFullyOpaque(const NFmiColor& color);
#else
    // Here are dymmy versions of functions with dummy definitions, if Uniras ToolMaster isn't supported (must be inline because VC++ linker...)
    inline Matrix3D<std::pair<int, COLORREF>>* UsedColorsCube()
    {return nullptr;}
    inline int UsedColorTableIndex()
    {return 0;}
    inline int UsedHollowColorIndex()
    {return 0;}
    inline void InitDefaultColorTable(bool fToolMasterAvailable)
    {}
    inline int RgbToColorIndex(float RGBcolors[3])
    {return 0;}
    inline int RgbToColorIndex(COLORREF color)
    {return 0;}
    inline int RgbToColorIndex(const NFmiColor& color)
    {return 0;}
    inline NFmiColor ColorIndexToRgb(int theColorIndex)
    { return NFmiColor(); }
#endif // DISABLE_UNIRAS_TOOLMASTER
}