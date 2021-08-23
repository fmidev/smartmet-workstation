#pragma once

#include "NFmiCtrlView.h"

class NFmiSymbolBulkDrawData;
class NFmiToolBox;

namespace Gdiplus
{
	class Graphics;
}

namespace GdiplusStationBulkDraw
{
	void Draw(const NFmiSymbolBulkDrawData &sbdData, Gdiplus::Graphics* gdiPlusGraphics, NFmiToolBox& toolBox);
}
