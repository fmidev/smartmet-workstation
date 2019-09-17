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

namespace Gdiplus
{
    class Graphics;
}

namespace CtrlView
{
    void DrawNormalColorContourLegend(const NFmiColorContourLegendSettings& colorContourLegendSettings, const NFmiColorContourLegendValues& colorContourLegendValues, NFmiPoint& lastLegendRelativeBottomRightCornerInOut, NFmiToolBox* toolbox, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics, float sizeFactor);
    NFmiPoint CalcProjectedPointInRectsXyArea(const NFmiRect& xyArea, const NFmiPoint& xyPoint);
}
