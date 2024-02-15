#include "MapDrawFunctions.h"
#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"
#ifndef DISABLE_CPPRESTSDK
#include "WmsSupport.h"
#endif // DISABLE_CPPRESTSDK
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiToolBox.h"
#include "CombinedMapHandlerInterface.h"
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
#ifndef DISABLE_CPPRESTSDK
        auto mapAreaIndex = docInterface->SelectedMapIndex(theDescTopIndex);
        auto mapPtr = docInterface->GetMapHandlerInterface(theDescTopIndex)->Area();
        auto holder = docInterface->GetWmsSupport()->getBackground(theDescTopIndex, mapAreaIndex, *mapPtr, int(bitmapSize.X()), int(bitmapSize.Y()));
        if(holder)
        {
            NFmiRect srcRect(0, 0, int(bitmapSize.X()), int(bitmapSize.Y()));
            CtrlView::DrawBitmapToDC_4(theUsedCDC, *holder->mImage, srcRect, destRect, true);
        }
#endif // DISABLE_CPPRESTSDK
    }

    void wmsOverlayDraw(CtrlViewDocumentInterface *docInterface, int theDescTopIndex,
        CDC *theUsedCDC, Gdiplus::RectF& destRect, const NFmiPoint& bitmapSize)
    {
#ifndef DISABLE_CPPRESTSDK
        auto mapAreaIndex = docInterface->SelectedMapIndex(theDescTopIndex);
        auto mapPtr = docInterface->GetMapHandlerInterface(theDescTopIndex)->Area();
        try
        {
            auto holder = docInterface->GetWmsSupport()->getOverlay(theDescTopIndex, mapAreaIndex, *mapPtr, int(bitmapSize.X()), int(bitmapSize.Y()));
            if(holder)
            {
                NFmiRect srcRect(0, 0, int(bitmapSize.X()), int(bitmapSize.Y()));
                CtrlView::DrawBitmapToDC_4(theUsedCDC, *holder->mImage, srcRect, destRect, true);
            }
        }
        catch(...)
        {
        }
#endif // DISABLE_CPPRESTSDK
    }

    void cleanMap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex)
    {
        docInterface->GetMapHandlerInterface(theDescTopIndex)->ClearMakeNewBackgroundBitmap();
    }

    void removeBitmap(CBitmap *bitmap)
    {
        if(bitmap)
            bitmap->DeleteObject();
    }

    void CreateBitmapUsedForMapDrawing(CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC, const NFmiPoint& bitmapSize)
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
        return docInterface->GetCombinedMapHandlerInterface().isOverlayMapDrawnForThisDescTop(theDescTopIndex, wantedDrawOverMapMode);
    }

    void drawOverlayMap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, int wantedDrawOverMapMode, CDC *theUsedCDC, Gdiplus::RectF& destRect, const NFmiPoint& bitmapSize, Gdiplus::Graphics* gdiplusGraphics)
    {
        if(drawOverlay(docInterface, theDescTopIndex, wantedDrawOverMapMode))
        {
            if(docInterface->GetCombinedMapHandlerInterface().useWmsOverlayMapDrawForThisDescTop(theDescTopIndex))
            {
                wmsOverlayDraw(docInterface, theDescTopIndex, theUsedCDC, destRect, bitmapSize);
            }
            else
            {
                auto namesBitmap = getOverlayBitmap(docInterface, theDescTopIndex);
                if(namesBitmap)
                {
                    auto bitmapNamesRect = getOverlaySourceRect(docInterface, theDescTopIndex);
                    CtrlView::DrawBitmapToDC_4(theUsedCDC, *namesBitmap, bitmapNamesRect, destRect, true, NFmiImageAttributes(Gdiplus::Color(Gdiplus::Color::White)), gdiplusGraphics);
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

    bool GenerateMapBitmap(CtrlViewDocumentInterface *docInterface, int theDescTopIndex, CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC, Gdiplus::Graphics* gdiplusGraphics)
    {
        if(mapIsNotDirty(docInterface, theDescTopIndex))
        {
            theUsedCDC->SelectObject(theUsedBitmap);
            return true;
        }
        cleanMap(docInterface, theDescTopIndex);
        removeBitmap(theUsedBitmap);

        auto bitmapSize = getBitmapSize(docInterface, theDescTopIndex);
        auto mfcRect = getMfcRect(bitmapSize);
        auto destRect = getDestRect(mfcRect);
        auto sourceRect = getSourceRect(docInterface, theDescTopIndex);

        CreateBitmapUsedForMapDrawing(theUsedBitmap, theUsedCDC, theCompatibilityCDC, bitmapSize);
        theUsedCDC->SelectObject(theUsedBitmap);

        if(docInterface->GetCombinedMapHandlerInterface().useWmsMapDrawForThisDescTop(theDescTopIndex))
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
            CtrlView::DrawBitmapToDC_4(theUsedCDC, *aBitmap, sourceRect, destRect, true);
        }

        int wantedDrawOverMapMode = 0; // means overlay is drawn right over background map before the dynamic data is drawn
        drawOverlayMap(docInterface, theDescTopIndex, 0, theUsedCDC, destRect, bitmapSize, gdiplusGraphics);
        return true;
    }

    // Kun karttanäyttö ruudun korkeus on tarpeeksi pieni, ohjelma kaatuu jos sitä
    // yritetään piirtää. Lisäksi turha piirtää mitään koska mitään ei kuitenkaan näy.
    // Lisäksi en saanut selville mikä kaataa ohjelman.  Kun ei piirretä
    // projektio viivoja, ohjelma kaatuu dcMem.SelectObject(itsMemoryBitmap)
    // kohtaan, mutta en voinut debugata MFC:n sisälle.
    // Kun projektio viivojen piirto on päällä, ohjelma kaatuu jotenkin
    // oudosti projektio viivojen tuhoamiseen.
    bool stopDrawingTooSmallMapview(CWnd *mapView, int theDescTopIndex)
    {
        if(!mapView)
            return true;

        CRect clientRect;
        mapView->GetClientRect(&clientRect);
        if(clientRect.Height() < 4)
        {
            std::string warningMessage = "Map view ";
            warningMessage += std::to_string(theDescTopIndex);
            warningMessage += " was too small to be drawn at all";
            CatLog::logMessage(warningMessage, CatLog::Severity::Warning, CatLog::Category::Visualization, true);
            return true; 
        }

        return false;
    }

}
