#pragma once


class CtrlViewDocumentInterface;
class CDC;
class NFmiPoint;
class CRect;
class CBitmap;
class NFmiRect;
class NFmiToolBox;
class CWnd;

namespace Gdiplus
{
    class RectF;
}

namespace MapDraw
{
    // Helper functions for drawing background maps or overlays from old file based system or with newer Wms based system
    CRect getMfcRect(const NFmiPoint& bitmapSize);
    CRect getMfcRect(const NFmiRect& relativeRect, NFmiToolBox *toolbox);
    bool drawOverlay(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, int wantedDrawOverMapMode);
    void drawOverlayMap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, int wantedDrawOverMapMode, CDC *theUsedCDC, Gdiplus::RectF& destRect, const NFmiPoint& bitmapSize);
    NFmiPoint getBitmapSize(CtrlViewDocumentInterface *docInterface, int theDescTopIndex);
    Gdiplus::RectF getDestRect(const CRect& mfcRect);
    bool GenerateMapBitmap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC);
    bool stopDrawingTooSmallMapview(CWnd *mapView, int theDescTopIndex);

    template<typename DocumentInterface>
    bool mapIsNotDirty(DocumentInterface* docInterface, int theDescTopIndex)
    {
        return !(docInterface && docInterface->GetMapHandlerInterface(theDescTopIndex)->MakeNewBackgroundBitmap());
    }
}
