#pragma once

class NFmiStationViewHandler;
class NFmiArea;
class GdiPlusLineInfo;
class CtrlViewDocumentInterface;
namespace Gdiplus
{
    class Graphics;
    class PointF;
}

// Does NFmiArea confuse you? Are you tired of visualizing stereographic projections in latlon world?
// Paste the snippet below to NFmiStationViewHandler::draw() method and let the SmartMet visualize the map for you!
//
//auto euroString = static_cast<string>("stereographic,10,90,60:-19.22,25,79.7,57");
//auto area1 = NFmiAreaFactory::Create(euroString);
//CFmiGdiPlusHelpers::LineInfo lineType1(1, NFmiColor(1, 0, 0), 0);
//NFmiStationViewHandlerDebugging::Params params(*this, *itsGdiPlusGraphics, *itsDoc);
//NFmiStationViewHandlerDebugging::drawAreasBordersWithGivenLineInfo(params, *area1, lineType1);

namespace NFmiStationViewHandlerDebugging
{
    struct Params
    {
        NFmiStationViewHandler& stationViewHandler;
        Gdiplus::Graphics& gdiPlusGraphics;
        CtrlViewDocumentInterface& doc;

        Params(NFmiStationViewHandler& stationViewHandler_, Gdiplus::Graphics& gdiPlusGraphics_, CtrlViewDocumentInterface& doc_)
            :stationViewHandler(stationViewHandler_)
            , gdiPlusGraphics(gdiPlusGraphics_)
            , doc(doc_)
        {
        }
    };

    void drawAreasBordersWithGivenLineInfo(Params& params, const NFmiArea& areaToDraw, const GdiPlusLineInfo& lineType, size_t resolution = 100);
}