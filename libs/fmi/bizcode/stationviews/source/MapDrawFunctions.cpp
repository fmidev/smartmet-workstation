#include "MapDrawFunctions.h"
#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"
#include "WmsSupport.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiToolBox.h"
#include "catlog/catlog.h"

namespace
{
    void fillMapWithWhite(CDC *theUsedCDC, const CRect& mfcRect)
    {
        CBrush whiteBrush(RGB(255, 255, 255));
        CBrush *oldBrush = theUsedCDC->SelectObject(&whiteBrush);
        theUsedCDC->Rectangle(mfcRect);
        theUsedCDC->SelectObject(oldBrush);
    }

    Gdiplus::Bitmap * getOverlayBitmap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex)
    {
        return docInterface->GetMapHandlerInterface(theDescTopIndex)->GetOverMapBitmap();
    }

    NFmiRect getOverlaySourceRect(CtrlViewDocumentInterface *docInterface, int theDescTopIndex)
    {
        return docInterface->GetMapHandlerInterface(theDescTopIndex)->ZoomedAbsolutRectOverMap();
    }

    void wmsDraw(CtrlViewDocumentInterface *docInterface, int theDescTopIndex,
        CDC *theUsedCDC, Gdiplus::RectF& destRect, const NFmiPoint& bitmapSize)
    {
        auto mapPtr = docInterface->GetMapHandlerInterface(theDescTopIndex)->Area();
        auto holder = docInterface->WmsSupport().getBackground(*mapPtr, int(bitmapSize.X()), int(bitmapSize.Y()));
        if(holder)
        {
            NFmiRect srcRect(0, 0, int(bitmapSize.X()), int(bitmapSize.Y()));
            CtrlView::DrawBitmapToDC(theUsedCDC, *holder->mImage, srcRect, destRect, true);
        }
    }

    void wmsOverlayDraw(CtrlViewDocumentInterface *docInterface, int theDescTopIndex,
        CDC *theUsedCDC, Gdiplus::RectF& destRect, const NFmiPoint& bitmapSize)
    {
        auto mapPtr = docInterface->GetMapHandlerInterface(theDescTopIndex)->Area();
        try
        {
            auto holder = docInterface->WmsSupport().getOverlay(*mapPtr, int(bitmapSize.X()), int(bitmapSize.Y()));
            if(holder)
            {
                NFmiRect srcRect(0, 0, int(bitmapSize.X()), int(bitmapSize.Y()));
                CtrlView::DrawBitmapToDC(theUsedCDC, *holder->mImage, srcRect, destRect, true);
            }
        }
        catch(...)
        {
        }
    }

    bool mapIsNotDirty(CtrlViewDocumentInterface *docInterface, int theDescTopIndex)
    {
        return !(docInterface && docInterface->GetMapHandlerInterface(theDescTopIndex)->IsMapDirty());
    }

    void cleanMap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex)
    {
        docInterface->GetMapHandlerInterface(theDescTopIndex)->MapDirty(false);
    }

    void removeBitmap(CBitmap *bitmap)
    {
        if(bitmap)
            bitmap->DeleteObject();
    }

    void askMarkoWhatThisDoes(CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC, const NFmiPoint& bitmapSize)
    {
        theUsedBitmap->CreateCompatibleBitmap(theCompatibilityCDC, int(bitmapSize.X()),
            int(bitmapSize.Y()));
        ASSERT(theUsedBitmap->m_hObject != 0);
    }

    NFmiRect getSourceRect(CtrlViewDocumentInterface *docInterface, int theDescTopIndex)
    {
        return docInterface->GetMapHandlerInterface(theDescTopIndex)->ZoomedAbsolutRect();
    }

    Gdiplus::Bitmap * getBitmap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex)
    {
        Gdiplus::Bitmap *aBitmap = nullptr;
        try
        {
            aBitmap = docInterface->GetMapHandlerInterface(theDescTopIndex)->GetBitmap();
        }
        catch(std::exception &e)
        { // raportoidaan virheestä, mutta jatketaan
            docInterface->LogAndWarnUser(e.what(), "", CatLog::Severity::Error, CatLog::Category::Visualization, true);
        }
        return aBitmap;
    }
}


namespace MapDraw
{
    CRect getMfcRect(const NFmiRect& relativeRect, NFmiToolBox *toolbox)
    {
        CRect absoluteRect;
        toolbox->ConvertRect(relativeRect, absoluteRect);
        return absoluteRect;
    }

    CRect getMfcRect(const NFmiPoint& bitmapSize)
    {
        return CRect(0, 0, int(bitmapSize.X()), int(bitmapSize.Y()));
    }

    bool drawOverlay(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, int wantedDrawOverMapMode)
    {
        if(docInterface->UseWmsMaps())
            return (docInterface->DrawOverMapMode(theDescTopIndex) == wantedDrawOverMapMode);
        else
            return docInterface->GetMapHandlerInterface(theDescTopIndex)->ShowOverMap()
            && docInterface->DrawOverMapMode(theDescTopIndex) == wantedDrawOverMapMode;
    }

    void drawOverlayMap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, int wantedDrawOverMapMode, CDC *theUsedCDC, Gdiplus::RectF& destRect, const NFmiPoint& bitmapSize)
    {
        if(drawOverlay(docInterface, theDescTopIndex, wantedDrawOverMapMode))
        {
            if(docInterface->UseWmsMaps())
            {
                wmsOverlayDraw(docInterface, theDescTopIndex, theUsedCDC, destRect, bitmapSize);
            }
            else
            {
                auto namesBitmap = getOverlayBitmap(docInterface, theDescTopIndex);
                if(namesBitmap)
                {
                    auto bitmapNamesRect = getOverlaySourceRect(docInterface, theDescTopIndex);
                    CtrlView::DrawBitmapToDC(theUsedCDC, *namesBitmap, bitmapNamesRect, destRect,
                        Gdiplus::Color(Gdiplus::Color::White), true);
                }
            }
        }
    }

    NFmiPoint getBitmapSize(CtrlViewDocumentInterface *docInterface, int theDescTopIndex)
    {
        return docInterface->ActualMapBitmapSizeInPixels(theDescTopIndex);
    }

    Gdiplus::RectF getDestRect(const CRect& mfcRect)
    {
        return Gdiplus::RectF(static_cast<Gdiplus::REAL>(mfcRect.left),
            static_cast<Gdiplus::REAL>(mfcRect.top), static_cast<Gdiplus::REAL>(mfcRect.Width()), static_cast<Gdiplus::REAL>(mfcRect.Height()));
    }

    bool GenerateMapBitmap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC, CBitmap *theOldBitmap)
    {
        if(mapIsNotDirty(docInterface, theDescTopIndex))
        {
            theOldBitmap = theUsedCDC->SelectObject(theUsedBitmap);
            return true;
        }
        cleanMap(docInterface, theDescTopIndex);
        removeBitmap(theUsedBitmap);

        auto bitmapSize = getBitmapSize(docInterface, theDescTopIndex);
        auto mfcRect = getMfcRect(bitmapSize);
        auto destRect = getDestRect(mfcRect);
        auto sourceRect = getSourceRect(docInterface, theDescTopIndex);

        askMarkoWhatThisDoes(theUsedBitmap, theUsedCDC, theCompatibilityCDC, bitmapSize);
        theOldBitmap = theUsedCDC->SelectObject(theUsedBitmap);

        if(docInterface->UseWmsMaps())
        {
            wmsDraw(docInterface, theDescTopIndex, theUsedCDC, destRect, bitmapSize);
        }
        else
        {
            auto aBitmap = getBitmap(docInterface, theDescTopIndex);
            if(!aBitmap)
            {
                return true;
            }
            fillMapWithWhite(theUsedCDC, mfcRect);
            CtrlView::DrawBitmapToDC(theUsedCDC, *aBitmap, sourceRect, destRect, true);
        }

        int wantedDrawOverMapMode = 0; // means overlay is drawn right over background map before the dynamic data is drawn
        drawOverlayMap(docInterface, theDescTopIndex, 0, theUsedCDC, destRect, bitmapSize);
        return true;
    }
}
