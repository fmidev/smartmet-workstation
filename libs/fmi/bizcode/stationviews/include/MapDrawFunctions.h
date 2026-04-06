#pragma once

class CtrlViewDocumentInterface;
class NFmiPoint;
class NFmiRect;
class NFmiToolBox;

#ifndef UNIX
class CDC;
class CRect;
class CBitmap;
class CWnd;

namespace Gdiplus
{
    class RectF;
    class Graphics;
}
#endif // UNIX

namespace MapDraw
{
    // Helper functions for drawing background maps or overlays from old file based system or with newer Wms based system
#ifndef UNIX
    CRect getMfcRect(const NFmiPoint& bitmapSize);
    CRect getMfcRect(const NFmiRect& relativeRect, NFmiToolBox *toolbox);
#endif
    bool drawOverlay(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, int wantedDrawOverMapMode);
#ifndef UNIX
    void drawOverlayMap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, int wantedDrawOverMapMode, CDC *theUsedCDC, Gdiplus::RectF& destRect, const NFmiPoint& bitmapSize, Gdiplus::Graphics *gdiplusGraphics);
#endif
    NFmiPoint getBitmapSize(CtrlViewDocumentInterface *docInterface, int theDescTopIndex);
#ifndef UNIX
    Gdiplus::RectF getDestRect(const CRect& mfcRect);
    bool GenerateMapBitmap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC, Gdiplus::Graphics* gdiplusGraphics);
    bool stopDrawingTooSmallMapview(CWnd *mapView, int theDescTopIndex);
#endif

    template<typename DocumentInterface>
    bool mapIsNotDirty(DocumentInterface* docInterface, int theDescTopIndex)
    {
        return !(docInterface && docInterface->GetMapHandlerInterface(theDescTopIndex)->MakeNewBackgroundBitmap());
    }
}
