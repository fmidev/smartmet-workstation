#include "NFmiCountryBorderDrawUtils.h"
#include "NFmiCtrlView.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "catlog/catlog.h"
#include "NFmiPolyline.h"
#include "NFmiToolBox.h"
#include "NFmiVoidPtrList.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiPath.h"
#include "NFmiIsoLineView.h"
#include "ToolBoxStateRestorer.h"
#include "ApplicationInterface.h"
#include "CtrlViewWin32Functions.h"
#include "CombinedMapHandlerInterface.h"
#include "NFmiDrawParam.h"
#include "OGR.h"
#include "Box.h"
#include "ogr_geometry.h"

#include <list>

namespace
{
    const COLORREF g_transparentColorRGB = RGB(213, 156, 68);
    const Gdiplus::Color g_transparentColorGdiplus(213, 156, 68);

    void traceLogLandBorderLineCounts(size_t polyLineCount, size_t totalLinePointCount, NFmiCtrlView* ctrlView)
    {
        if(CatLog::doTraceLevelLogging())
        {
            std::string traceMessage = "Polyline count = ";
            traceMessage += std::to_string(polyLineCount);
            traceMessage += ", total line point count = ";
            traceMessage += std::to_string(totalLinePointCount);
            CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(traceMessage, ctrlView);
        }
    }

    CPoint convertGdiplusPolypointToToolbox(const Gdiplus::PointF& gdiplusPolypoint)
    {
        return CPoint(static_cast<int>(gdiplusPolypoint.X), static_cast<int>(gdiplusPolypoint.Y));
    }

    std::vector<CPoint> convertGdiplusPolylineToToolbox(const std::vector<Gdiplus::PointF>& gdiplusPolyline)
    {
        std::vector<CPoint> toolboxPolyline(gdiplusPolyline.size());
        for(size_t index = 0; index < toolboxPolyline.size(); index++)
        {
            toolboxPolyline[index] = ::convertGdiplusPolypointToToolbox(gdiplusPolyline[index]);
        }
        return toolboxPolyline;
    }

    std::list<std::vector<CPoint>> convertGdiplusPolylineListToToolbox(const std::list<std::vector<Gdiplus::PointF>>& polyLineListGdiplusInPixelCoordinates, size_t* totalLinePointCount)
    {
        std::list<std::vector<CPoint>> toolboxPolylines;
        for(const auto& gdiplusPolyline : polyLineListGdiplusInPixelCoordinates)
        {
            toolboxPolylines.push_back(::convertGdiplusPolylineToToolbox(gdiplusPolyline));
        }
        return toolboxPolylines;
    }

    void drawPixelPolyLineListWithToolbox(NFmiToolBox* theGTB, const std::list<std::vector<Gdiplus::PointF>>& polyLineListGdiplusInPixelCoordinates, NFmiCtrlView* ctrlView, NFmiDrawingEnvironment& drawingEnvironment)
    {
        size_t totalLinePointCount = 0;
        auto toolBoxPolyLineListInPixelCoordinates = ::convertGdiplusPolylineListToToolbox(polyLineListGdiplusInPixelCoordinates, &totalLinePointCount);
        theGTB->DrawPolylineListInPixelCoordinates(toolBoxPolyLineListInPixelCoordinates, drawingEnvironment);
        ::traceLogLandBorderLineCounts(toolBoxPolyLineListInPixelCoordinates.size(), totalLinePointCount, ctrlView);
    }

    void drawPolyLineListGdiplus(Gdiplus::Graphics& gdiplusGraphics, NFmiToolBox* toolbox, const std::list<std::vector<Gdiplus::PointF>>& polyLineListGdiplusInPixelCoordinates, NFmiCtrlView* ctrlView, const NFmiColor& lineColor, int lineThickness)
    {
        size_t totalLinePointCount = CtrlView::DrawGdiplusSimpleMultiPolyLineInPixelCoordinates(gdiplusGraphics, toolbox, polyLineListGdiplusInPixelCoordinates, lineColor, lineThickness);
        ::traceLogLandBorderLineCounts(polyLineListGdiplusInPixelCoordinates.size(), totalLinePointCount, ctrlView);
    }

    std::vector<Gdiplus::PointF> convertLineStringGeometryToGdiplusPolygon(OGRLineString* lineStringGeometry, const Fmi::Box& box)
    {
        if(lineStringGeometry)
        {
            const int numberOfPoints = lineStringGeometry->getNumPoints();
            std::vector<Gdiplus::PointF> gdiplusPolygon(numberOfPoints);

            for(int i = 0; i < numberOfPoints; ++i)
            {
                double x = lineStringGeometry->getX(i);
                double y = lineStringGeometry->getY(i);

                box.transform(x, y);
                gdiplusPolygon[i] = Gdiplus::PointF(static_cast<Gdiplus::REAL>(x), static_cast<Gdiplus::REAL>(y));
            }
            return gdiplusPolygon;
        }
        return {};
    }

    std::list<std::vector<Gdiplus::PointF>> convertMultiLineStringGeometryToGdiplusMultiPolygon(OGRMultiLineString* multiLineStringGeometry, const Fmi::Box& box)
    {
        std::list<std::vector<Gdiplus::PointF>> gdiplusMultiPolygon;
        if(multiLineStringGeometry)
        {
            for(int i = 0, n = multiLineStringGeometry->getNumGeometries(); i < n; ++i)
            {
                gdiplusMultiPolygon.emplace_back(::convertLineStringGeometryToGdiplusPolygon(dynamic_cast<OGRLineString*>(multiLineStringGeometry->getGeometryRef(i)), box));
                if(gdiplusMultiPolygon.back().empty())
                    gdiplusMultiPolygon.pop_back();
            }
        }
        return gdiplusMultiPolygon;
    }

    std::list<std::vector<Gdiplus::PointF>> convertGeometryToGdiplusMultiPolygon(OGRGeometry* geometry, const Fmi::Box& box)
    {
        OGRwkbGeometryType id = geometry->getGeometryType();
        switch(id)
        {
        case wkbMultiLineString:
            return ::convertMultiLineStringGeometryToGdiplusMultiPolygon(dynamic_cast<OGRMultiLineString*>(geometry), box);
        default:
            return {};
        }
    }

    Fmi::Box calculateZoomedAreaWorldXyClipRect(CtrlViewDocumentInterface *ctrlViewDocumentInterface, int mapViewDescTopIndex, const boost::shared_ptr<NFmiArea>& mapArea)
    {
        auto singleMapViewSizeInPixels = ctrlViewDocumentInterface->ActualMapBitmapSizeInPixels(mapViewDescTopIndex);
        auto worldXyRect = mapArea->WorldRect();
        return Fmi::Box(worldXyRect.Left(), worldXyRect.Top(), worldXyRect.Right(), worldXyRect.Bottom(), boost::math::iround(singleMapViewSizeInPixels.X()), boost::math::iround(singleMapViewSizeInPixels.Y()));
    }

    void recalculateCountryBorderGeometry(NFmiCtrlView* mapView, const boost::shared_ptr<NFmiArea> & mapArea)
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(mapView, std::string(__FUNCTION__) + " borders were 'dirty', recalculating them");
        auto ctrlViewDocumentInterface = mapView->GetCtrlViewDocumentInterface();
        int mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        auto baseAreaCountryBorderGeometry = ctrlViewDocumentInterface->CountryBorderGeometry(mapViewDescTopIndex);
        if(baseAreaCountryBorderGeometry)
        {
            Fmi::Box zoomedClipBox = ::calculateZoomedAreaWorldXyClipRect(ctrlViewDocumentInterface, mapViewDescTopIndex, mapArea);
            auto zoomedAreaGeometryInPixelCoordinates = Fmi::OGR::lineclip(*baseAreaCountryBorderGeometry, zoomedClipBox);
            // laitetaan piirtovalmis polylinelista talteen dokumenttiin, t‰m‰ myˆs asettaa listaan liittyv‰n likaisuus lipun pois eli k‰yttˆvalmiiksi
            ctrlViewDocumentInterface->PolyLineListGdiplusInPixelCoordinates(mapViewDescTopIndex, ::convertGeometryToGdiplusMultiPolygon(zoomedAreaGeometryInPixelCoordinates, zoomedClipBox));
        }
        else
            CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging("Didn't find any borders to draw", mapView);
    }

    void drawLandBordersWithWantedSystem(NFmiCtrlView* mapView, NFmiToolBox* toolbox, NFmiDrawParam *separateBorderLayerDrawOptions, bool doGdiplusDraw)
    {
        auto ctrlViewDocumentInterface = mapView->GetCtrlViewDocumentInterface();
        int mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        auto mapArea = mapView->GetArea();
        if(ctrlViewDocumentInterface->DrawLandBorders(mapViewDescTopIndex, separateBorderLayerDrawOptions))
        {
            // Border viivojen piirto erilliseen cache bitmap:iin pit‰‰ tapahtua aina kiinni origossa.
            // Siksi pit‰‰ luoda uusi area-olio, jonka top-left kulma on relatiivisessa origossa (0, 0)
            if(ctrlViewDocumentInterface->PolyLineListGdiplusInPixelCoordinatesDirty(mapViewDescTopIndex))
                ::recalculateCountryBorderGeometry(mapView, mapArea);

            auto& polyLineListGdiplusInPixelCoordinates = ctrlViewDocumentInterface->PolyLineListGdiplusInPixelCoordinates(mapViewDescTopIndex);
            if(polyLineListGdiplusInPixelCoordinates.empty() == false)
            {
                CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(mapView, std::string(__FUNCTION__) + " doing final border drawing");
                int penSize = ctrlViewDocumentInterface->LandBorderPenSize(mapViewDescTopIndex, separateBorderLayerDrawOptions);
                auto lineColor = ctrlViewDocumentInterface->LandBorderColor(mapViewDescTopIndex, separateBorderLayerDrawOptions);
                if(doGdiplusDraw)
                {
                    // Pit‰‰ luoda rajaviiva piirrossa k‰ytˆss‰ olevan toolboxin avulla uusi Gdiplus-graphics olio, ei saa k‰ytt‰‰ mapView:in vastaavaa oliota!
                    std::unique_ptr<Gdiplus::Graphics> gdigraphicsPtr(NFmiCtrlView::CreateGdiplusGraphics(toolbox, &mapArea->XYArea()));
                    if(gdigraphicsPtr)
                        ::drawPolyLineListGdiplus(*gdigraphicsPtr, toolbox, polyLineListGdiplusInPixelCoordinates, mapView, lineColor, penSize);
                    else
                        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + " couldn't create gdiplus graphics object from given toolbox", mapView);
                }
                else
                {
                    NFmiDrawingEnvironment envi;
                    envi.SetFrameColor(lineColor);
                    envi.SetPenSize(NFmiPoint(penSize, penSize));
                    ToolBoxStateRestorer toolBoxStateRestorer(*toolbox, toolbox->GetTextAlignment(), true, &mapArea->XYArea());
                    ::drawPixelPolyLineListWithToolbox(toolbox, polyLineListGdiplusInPixelCoordinates, mapView, envi);
                }
            }
        }
        else
            CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + " no land border drawing here", mapView);
    }

    void drawLandBorders(NFmiCtrlView* mapView, NFmiToolBox* toolbox, NFmiDrawParam* separateBorderLayerDrawOptions)
    {
        auto ctrlViewDocumentInterface = mapView->GetCtrlViewDocumentInterface();
        int mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        auto mapArea = mapView->GetArea();

        auto penSize = ctrlViewDocumentInterface->LandBorderPenSize(mapViewDescTopIndex, separateBorderLayerDrawOptions);
        //// Vanha rajaviiva piirto on nopeaa, kun piirto tehd‰‰n 1 paksuisella viivalla.
        //// Jos piirto tehd‰‰n 2 tai 3 pikselin kyn‰ll‰ vanha piirto hidastui merkitt‰v‰sti:
        //// Alue piirrot: suomi n. 25-40x (piirtoaika), skandi n. 45-75x, euro n. 15-30 ja maailma n. 6-10x
        //// Tein uuden piirto koodin, joka on hitaampi 1 pikselin piirrolle, mutta nopeampi 2-3 pikselille:
        //// Uusi vs vanha piirtonopeus: suomi n. 3-5x (nopeampi), skandi n. 7-11x, euro n. 2.5-4.5x ja maailma n. 1.5-3x
        //// Siksi jos piirto paksuus on 1, k‰ytet‰‰n vanhaa piirtoa ja muuten k‰ytet‰‰n uutta.
        bool useGdiPlus = (penSize > 1);
        ::drawLandBordersWithWantedSystem(mapView, toolbox, separateBorderLayerDrawOptions, useGdiPlus);
    }

    void fillMapWithTransparentColor(CDC* theUsedCDC, const CRect& mfcRect)
    {
        CBrush whiteBrush(g_transparentColorRGB);
        CBrush* oldBrush = theUsedCDC->SelectObject(&whiteBrush);
        theUsedCDC->Rectangle(mfcRect);
        theUsedCDC->SelectObject(oldBrush);
    }

    void drawLandBordersToCacheBitmap(NFmiCtrlView* mapView, NFmiToolBox* toolbox, NFmiDrawParam* separateBorderLayerDrawOptions)
    {
        int mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        auto mfcMapView = ApplicationInterface::GetApplicationInterfaceImplementation()->GetView(mapViewDescTopIndex);
        if(mfcMapView)
        {
            CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(__FUNCTION__, mapView);
            auto ctrlViewDocumentInterface = mapView->GetCtrlViewDocumentInterface();
            CBitmap* landBorderMapBitmap = new CBitmap;
            auto bitmapSize = ctrlViewDocumentInterface->ActualMapBitmapSizeInPixels(mapViewDescTopIndex);
            CtrlView::MakeCombatibleBitmap(mfcMapView, &landBorderMapBitmap, static_cast<int>(bitmapSize.X()), static_cast<int>(bitmapSize.Y()));
            auto usedDc = toolbox->GetDC();
            auto oldBitmap = usedDc->SelectObject(landBorderMapBitmap);
            CRect drawRect(0, 0, static_cast<int>(bitmapSize.X()), static_cast<int>(bitmapSize.Y()));
            ::fillMapWithTransparentColor(usedDc, drawRect);
            ::drawLandBorders(mapView, toolbox, separateBorderLayerDrawOptions);
            landBorderMapBitmap = toolbox->GetDC()->SelectObject(oldBitmap);
            Gdiplus::Bitmap* gdiplusBitmap = new Gdiplus::Bitmap(HBITMAP(*landBorderMapBitmap), NULL);
            ctrlViewDocumentInterface->SetLandBorderMapBitmap(mapViewDescTopIndex, gdiplusBitmap, separateBorderLayerDrawOptions);
            //landBorderMapBitmap->DeleteObject();
            delete landBorderMapBitmap;
        }
    }

    void drawLandBordersFromCacheBitmap(NFmiCtrlView* mapView, NFmiToolBox* toolbox, NFmiDrawParam* separateBorderLayerDrawOptions)
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(__FUNCTION__, mapView);
        int mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        auto ctrlViewDocumentInterface = mapView->GetCtrlViewDocumentInterface();
        auto usedDc = toolbox->GetDC();
        auto *landBorderMapBitmap = ctrlViewDocumentInterface->LandBorderMapBitmap(mapViewDescTopIndex, separateBorderLayerDrawOptions);
        if(!landBorderMapBitmap)
            throw std::runtime_error(std::string("Error in ") + __FUNCTION__ + ": landBorderMapBitmap was empty, error in application logic?");

        NFmiRect sourcePixels(0, 0, landBorderMapBitmap->GetWidth(), landBorderMapBitmap->GetHeight());
        auto destPixels = CtrlView::Relative2GdiplusRectF(toolbox, mapView->GetArea()->XYArea());
        // Edell‰ laskettu piito alueen koko saattaa heitt‰‰ yhdell‰ pikselill‰, ja se tekee viiva piirrosta h‰m‰‰v‰n, 
        // joten fiksaan t‰ss‰ piirtoalueen originaali bitmap kokoon, vaikka se ehk‰ onkin pikkuisen v‰‰rin.
        destPixels.Width = static_cast<Gdiplus::REAL>(landBorderMapBitmap->GetWidth());
        destPixels.Height = static_cast<Gdiplus::REAL>(landBorderMapBitmap->GetHeight());
        CtrlView::DrawBitmapToDC(usedDc, *landBorderMapBitmap, sourcePixels, destPixels, g_transparentColorGdiplus, false);
    }

    void drawLandBordersWithBitmap(NFmiCtrlView* mapView, NFmiToolBox* toolbox, NFmiDrawParam* separateBorderLayerDrawOptions)
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter traceLogging(mapView, __FUNCTION__);
        int mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        auto ctrlViewDocumentInterface = mapView->GetCtrlViewDocumentInterface();
        if(ctrlViewDocumentInterface->DrawLandBorders(mapViewDescTopIndex, separateBorderLayerDrawOptions))
        {
            if(ctrlViewDocumentInterface->BorderDrawBitmapDirty(mapViewDescTopIndex, separateBorderLayerDrawOptions))
                ::drawLandBordersToCacheBitmap(mapView, toolbox, separateBorderLayerDrawOptions);
            ::drawLandBordersFromCacheBitmap(mapView, toolbox, separateBorderLayerDrawOptions);
        }
    }

} // nameless namespace ends

void NFmiCountryBorderDrawUtils::drawCountryBordersToMapView(NFmiCtrlView* mapView, NFmiToolBox* toolbox, NFmiDrawParam* separateBorderLayerDrawOptions)
{
    // Printatessa ei sitten voinutkaan k‰ytt‰‰ uutta nopeampaa bitmap optimoitua piirtoa.
    if(mapView->GetCtrlViewDocumentInterface()->Printing())
        ::drawLandBorders(mapView, toolbox, separateBorderLayerDrawOptions);
    else
        ::drawLandBordersWithBitmap(mapView, toolbox, separateBorderLayerDrawOptions);
}