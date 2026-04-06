#pragma once

class NFmiToolBox;
class NFmiPoint;
class NFmiRect;
class NFmiColorContourLegendSettings;
class NFmiColorContourLegendValues;

namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

#ifndef UNIX
namespace Gdiplus
{
    class Graphics;
}
#endif

namespace CtrlView
{
#ifndef UNIX
    void DrawNormalColorContourLegend(const NFmiColorContourLegendSettings& colorContourLegendSettings, const NFmiColorContourLegendValues& colorContourLegendValues, NFmiPoint& lastLegendRelativeBottomRightCornerInOut, NFmiToolBox* toolbox, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics, float sizeFactor, const NFmiRect& relativeDataRect);
#endif
    NFmiPoint CalcProjectedPointInRectsXyArea(const NFmiRect& xyArea, const NFmiPoint& xyPoint);
}
