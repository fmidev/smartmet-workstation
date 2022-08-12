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

    void drawPolyLineList(NFmiToolBox* theGTB, std::list<NFmiPolyline*>& thePolyLineList, const NFmiPoint& theOffSet, NFmiCtrlView* ctrlView, NFmiDrawingEnvironment &drawingEnvironment)
    {
        // Maiden rajat piirret‰‰n aina samalla v‰rill‰ ja viivapaksuudella ja se asetetaan t‰ss‰ ennen polylinejen piirtoa.
        theGTB->ConvertEnvironment(&drawingEnvironment);
        NFmiPoint scale;
        size_t totalLinePointCount = 0;
        for(auto polyLine : thePolyLineList)
        {
            theGTB->DrawPolyline(polyLine, theOffSet, scale);
            totalLinePointCount += polyLine->GetPoints().size();
        }
        ::traceLogLandBorderLineCounts(thePolyLineList.size(), totalLinePointCount, ctrlView);
    }

    void drawPolyLineListGdiplus(Gdiplus::Graphics& gdiplusGraphics, NFmiToolBox* toolbox, const std::list<std::vector<NFmiPoint>>& polyLineListGdiplus, const NFmiPoint& relativeOffSet, NFmiCtrlView* ctrlView, const NFmiColor& lineColor, int lineThickness)
    {
        size_t totalLinePointCount = CtrlView::DrawGdiplusSimpleMultiPolyLine(gdiplusGraphics, toolbox, polyLineListGdiplus, lineColor, lineThickness, relativeOffSet);
        ::traceLogLandBorderLineCounts(polyLineListGdiplus.size(), totalLinePointCount, ctrlView);
    }

    void pushOldPolyLineAndStartNew(std::list<std::vector<NFmiPoint>>& relativePolyLineList, std::vector<NFmiPoint>& currentPolyLine, const NFmiPoint& newPoint)
    {
        relativePolyLineList.push_back(currentPolyLine); // laitetaan edellinen polyline talteen listaan
        currentPolyLine.clear(); // nollataan polyline
        currentPolyLine.push_back(newPoint); // laitetaan uuden polylinen 1. piste
    }

    std::list<std::vector<NFmiPoint>> convertPath2RelativePolyLineListGdiplus(Imagine::NFmiPath& thePath, bool relative_moves, bool removeghostlines)
    {
        using namespace Imagine;

        std::list<std::vector<NFmiPoint>> relativePolyLineList;
        std::vector<NFmiPoint> currentPolyLine;
        float last_x, last_y;
        float last_out_x, last_out_y;
        NFmiPathOperation last_op = kFmiMoveTo;
        bool firstTime = true;

        last_x = last_y = kFloatMissing;
        last_out_x = last_out_y = kFloatMissing;

        NFmiPathData::const_iterator iter = thePath.Elements().begin();
        for(; iter != thePath.Elements().end(); ++iter)
        {
            float x = static_cast<float>((*iter).X());
            float y = static_cast<float>((*iter).Y());
            // Special code for first move

            if((*iter).Oper() == kFmiConicTo || (*iter).Oper() == kFmiCubicTo)
            {
                CatLog::logMessage(std::string(__FUNCTION__) + ": Conic and Cubic control points not supported in NFmiPath class", CatLog::Severity::Error, CatLog::Category::Visualization);
                return relativePolyLineList;
            }

            bool out_ok = true;
            if(removeghostlines && (*iter).Oper() == kFmiGhostLineTo)
            {
                out_ok = false;
            }
            else
            {
                // If ghostlines are being ignored, we must output a moveto
                // when the ghostlines end and next operation is lineto.
                if(removeghostlines && (last_op == kFmiGhostLineTo) && ((*iter).Oper() == kFmiLineTo))
                {
                    if(relative_moves)
                    {
                        if(last_out_x == kFloatMissing && last_out_y == kFloatMissing)
                        {
                            ::pushOldPolyLineAndStartNew(relativePolyLineList, currentPolyLine, NFmiPoint(last_x, last_y));
                        }
                        else
                        {
                            ::pushOldPolyLineAndStartNew(relativePolyLineList, currentPolyLine, NFmiPoint(last_x - last_out_x, last_y - last_out_y));
                        }
                    }
                    else
                    {
                        ::pushOldPolyLineAndStartNew(relativePolyLineList, currentPolyLine, NFmiPoint(last_x, last_y));
                    }
                    last_op = kFmiMoveTo;
                    last_out_x = last_x;
                    last_out_y = last_y;
                }

                if(firstTime)
                {
                    firstTime = false;
                    currentPolyLine.push_back(NFmiPoint(x, y)); // laitetaan uuden polylinen 1. piste
                }
                // Relative moves are "m dx dy" and "l dx dy"
                else if(relative_moves)
                {
                    if((*iter).Oper() == kFmiMoveTo)
                    {
                        ::pushOldPolyLineAndStartNew(relativePolyLineList, currentPolyLine, NFmiPoint(x - last_out_x, y - last_out_y));
                    }
                    else if((*iter).Oper() == kFmiLineTo)
                    {
                        currentPolyLine.push_back(NFmiPoint(x - last_out_x, y - last_out_y)); // laitetaan uuden polylinen 1. piste
                    }
                    else if(!removeghostlines && (*iter).Oper() == kFmiGhostLineTo)
                    {
                        currentPolyLine.push_back(NFmiPoint(x - last_out_x, y - last_out_y)); // laitetaan uuden polylinen 1. piste
                    }
                    else
                    {
                        currentPolyLine.push_back(NFmiPoint(x - last_out_x, y - last_out_y)); // laitetaan uuden polylinen 1. piste
                    }
                }
                // Absolute moves are "M x y" and "L x y"
                else
                {
                    if((*iter).Oper() == kFmiMoveTo)
                    {
                        ::pushOldPolyLineAndStartNew(relativePolyLineList, currentPolyLine, NFmiPoint(x, y));
                    }
                    else if((*iter).Oper() == kFmiLineTo)
                    {
                        currentPolyLine.push_back(NFmiPoint(x, y));
                    }
                    else if(!removeghostlines && (*iter).Oper() == kFmiGhostLineTo)
                    {
                        currentPolyLine.push_back(NFmiPoint(x, y));
                    }
                    else
                    {
                        currentPolyLine.push_back(NFmiPoint(x, y));
                    }
                }
            }
            last_op = (*iter).Oper();

            last_x = x;
            last_y = y;
            if(out_ok)
            {
                last_out_x = x;
                last_out_y = y;
            }
        }

        if(currentPolyLine.size() > 1)
            relativePolyLineList.push_back(currentPolyLine); // laitetaan viel‰ viimeinen polyline listaan

        return relativePolyLineList;
    }

    std::unique_ptr<NFmiArea> CreateFixedToOrigoMapArea(const boost::shared_ptr<NFmiArea>& mapArea)
    {
        std::unique_ptr<NFmiArea> fixedToOrigoMapArea(mapArea.get()->Clone());
        fixedToOrigoMapArea->Place(NFmiPoint(0, 0));
        return std::move(fixedToOrigoMapArea);
    }


    void drawLandBordersWithGdiplus(NFmiCtrlView* mapView, NFmiToolBox* toolbox, NFmiDrawParam *separateBorderLayerDrawOptions)
    {
        auto ctrlViewDocumentInterface = mapView->GetCtrlViewDocumentInterface();
        int mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        auto mapArea = mapView->GetArea();
        if(ctrlViewDocumentInterface->DrawLandBorders(mapViewDescTopIndex, separateBorderLayerDrawOptions))
        {
            // Border viivojen piirto erilliseen cache bitmap:iin pit‰‰ tapahtua aina kiinni origossa.
            // Siksi pit‰‰ luoda uusi area-olio, jonka top-left kulma on relatiivisessa origossa (0, 0)
            auto fixedToOrigoMapArea = ::CreateFixedToOrigoMapArea(mapArea);
            if(ctrlViewDocumentInterface->BorderDrawPolylinesGdiplusDirty(mapViewDescTopIndex))
            {
                CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(mapView, std::string(__FUNCTION__) + " borders were 'dirty', recalculating them");
                boost::shared_ptr<Imagine::NFmiPath> usedPath = ctrlViewDocumentInterface->LandBorderPath(mapViewDescTopIndex);
                if(usedPath)
                {
                    Imagine::NFmiPath path(*usedPath.get());
                    path.Project(fixedToOrigoMapArea.get());
                    // laitetaan piirtovalmis polylinelista talteen dokumenttiin, t‰m‰ myˆs asettaa listaan liittyv‰n likaisuus lipun pois eli k‰yttˆvalmiiksi
                    ctrlViewDocumentInterface->DrawBorderPolyLineListGdiplus(mapViewDescTopIndex, ::convertPath2RelativePolyLineListGdiplus(path, false, true));
                }
                else
                    CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging("Didn't find any borders to draw", mapView);
            }

            auto& borderPolyLineList = ctrlViewDocumentInterface->DrawBorderPolyLineListGdiplus(mapViewDescTopIndex);
            if(borderPolyLineList.empty() == false)
            {
                CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(mapView, std::string(__FUNCTION__) + " doing final border drawing");
                NFmiPoint offSet(fixedToOrigoMapArea->TopLeft());
                int penSize = ctrlViewDocumentInterface->LandBorderPenSize(mapViewDescTopIndex, separateBorderLayerDrawOptions);
                auto lineColor = ctrlViewDocumentInterface->LandBorderColor(mapViewDescTopIndex, separateBorderLayerDrawOptions);
                // Pit‰‰ luoda rajaviiva piirrossa k‰ytˆss‰ olevan toolboxin avulla uusi Gdiplus-graphics olio, ei saa k‰ytt‰‰ mapView:in vastaavaa oliota!
                std::unique_ptr<Gdiplus::Graphics> gdigraphicsPtr(NFmiCtrlView::CreateGdiplusGraphics(toolbox, &fixedToOrigoMapArea->XYArea()));
                if(gdigraphicsPtr)
                    ::drawPolyLineListGdiplus(*gdigraphicsPtr, toolbox, borderPolyLineList, offSet, mapView, lineColor, penSize);
                else
                    CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + " couldn't create gdiplus graphics object from given toolbox", mapView);
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
        //// Siksi jos piirto paksuus on 1, k‰ytet‰‰n vanhaa piirtoa j muuten k‰ytet‰‰n uutta.
        bool usedGdiPlus = (penSize > 1);
        if(usedGdiPlus)
            ::drawLandBordersWithGdiplus(mapView, toolbox, separateBorderLayerDrawOptions);
        else
        {
            if(ctrlViewDocumentInterface->DrawLandBorders(mapViewDescTopIndex, separateBorderLayerDrawOptions))
            {
                // Border viivojen piirto erilliseen cache bitmap:iin pit‰‰ tapahtua aina kiinni origossa.
                // Siksi pit‰‰ luoda uusi area-olio, jonka top-left kulma on relatiivisessa origossa (0, 0)
                auto fixedToOrigoMapArea = ::CreateFixedToOrigoMapArea(mapArea);
                if(ctrlViewDocumentInterface->BorderDrawPolylinesDirty(mapViewDescTopIndex))
                {
                    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(mapView, std::string(__FUNCTION__) + " borders were 'dirty', recalculating them");
                    boost::shared_ptr<Imagine::NFmiPath> usedPath = ctrlViewDocumentInterface->LandBorderPath(mapViewDescTopIndex);
                    if(usedPath)
                    {
                        Imagine::NFmiPath path(*usedPath.get());
                        path.Project(fixedToOrigoMapArea.get());
                        std::list<NFmiPolyline*> polyLineList;
                        // NFmiIsoLineView::ConvertPath2PolyLineList funktiolle annetaan viimeisena nullptr parametri, koska ei haluta antaa jokaiselle
                        // erilliselle polylinelle omaa piirto-ominaisuus oliota. Kun maiden rajaviivat piirret‰‰n yhteen karttaruutuun, piirret‰‰n ne 
                        // aina samalla v‰rill‰ ja viivapaksuudella, joten ei ole tarvetta laitta jokaiselle viivanp‰tk‰lle erillisi‰ asetuksia.
                        NFmiIsoLineView::ConvertPath2PolyLineList(path, polyLineList, false, true, fixedToOrigoMapArea->XYArea(), nullptr);
                        // laitetaan piirtovalmis polylinelista talteen dokumenttiin, t‰m‰ myˆs asettaa polyline-listaan liittyv‰n dirty-flagin pois p‰‰lt‰
                        ctrlViewDocumentInterface->DrawBorderPolyLineList(mapViewDescTopIndex, polyLineList);
                    }
                    else
                        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging("Didn't find any borders to draw", mapView);
                }

                auto& borderPolyLineList = ctrlViewDocumentInterface->DrawBorderPolyLineList(mapViewDescTopIndex);
                if(borderPolyLineList.empty() == false)
                {
                    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(mapView, std::string(__FUNCTION__) + " doing final border drawing");
                    // Printatessa ei ole cache bitmap piirtoa, joten se on erikoistapaus, jolloin tarvitaan originaali relatiivista kartta-aluetta oikeine offset:eineen
                    auto* usedMapAreaInDrawing = toolbox->GetDC()->IsPrinting() ? mapArea.get() : fixedToOrigoMapArea.get();
                    NFmiPoint offSet(usedMapAreaInDrawing->TopLeft());
                    NFmiDrawingEnvironment envi;
                    envi.SetFrameColor(ctrlViewDocumentInterface->LandBorderColor(mapViewDescTopIndex, separateBorderLayerDrawOptions));
                    envi.SetPenSize(NFmiPoint(penSize, penSize));
                    ToolBoxStateRestorer toolBoxStateRestorer(*toolbox, toolbox->GetTextAlignment(), true, &usedMapAreaInDrawing->XYArea());
                    ::drawPolyLineList(toolbox, borderPolyLineList, offSet, mapView, envi);
                }
            }
            else
                CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + " no land border drawing here", mapView);
        }
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
        CtrlView::DrawBitmapToDC_4(usedDc, *landBorderMapBitmap, sourcePixels, destPixels, false, NFmiImageAttributes(g_transparentColorGdiplus));
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