#include "NFmiStationViewHandlerDebugging.h"
#include "NFmiArea.h"
#include "NFmiStationViewHandler.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewDocumentInterface.h"

#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;

namespace NFmiStationViewHandlerDebugging
{
    namespace
    {
        typedef vector<PointF> PointFVec;
        typedef pair<PointFVec, PointFVec> PointFVecPair;

        PointF toGdiPoint(Params& params, const NFmiPoint& latLon)
        {
            return CtrlView::ConvertLatlonToGdiPlusPoint(&params.stationViewHandler, latLon);
        }

        void storeInGivenPointFVecPair(const PointF gdiPoint1, const PointF gdiPoint2, PointFVecPair& fPoints)
        {
            fPoints.first.push_back(gdiPoint1);
            fPoints.second.push_back(gdiPoint2);
        }

        void convertToPointFAndStoreInGivenPointFVecPair(Params& p, const NFmiArea& area,const NFmiPoint& relative1, const NFmiPoint& relative2, PointFVecPair& fPoints)
        {
            NFmiPoint latLon1 = area.ToLatLon(relative1);
            NFmiPoint latLon2 = area.ToLatLon(relative2);
            storeInGivenPointFVecPair(toGdiPoint(p, latLon1), toGdiPoint(p, latLon2), fPoints);
        }

        PointFVecPair leftAndRightBorderGdiPoints(Params& p, const NFmiArea& area, size_t resolution)
        {
            PointFVecPair leftAndRight;
            double increment = (area.Bottom() - area.Top()) / resolution;
            double iterator = area.Top();
            NFmiPoint leftRelative;
            NFmiPoint rightRelative;
            for(int i = 0; i < resolution - 1; ++i, iterator += increment)
            {
                leftRelative = NFmiPoint(area.Left(), iterator);
                rightRelative = NFmiPoint(area.Right(), iterator);
                convertToPointFAndStoreInGivenPointFVecPair(p, area, leftRelative, rightRelative, leftAndRight);
            }
            leftRelative = NFmiPoint(area.Left(), area.Bottom());
            rightRelative = NFmiPoint(area.Right(), area.Bottom());
            convertToPointFAndStoreInGivenPointFVecPair(p, area, leftRelative, rightRelative, leftAndRight);
            return leftAndRight;
        }

        PointFVecPair topAndBottomBorderGdiPoints(Params& p, const NFmiArea& area, size_t resolution)
        {
            PointFVecPair topAndBottom;
            double increment = (area.Right() - area.Left()) / resolution;
            double iterator = area.Left();
            NFmiPoint topRelative;
            NFmiPoint bottomRelative;
            for(int i = 0; i < resolution - 1; ++i, iterator += increment)
            {
                topRelative = NFmiPoint(iterator, area.Top());
                bottomRelative = NFmiPoint(iterator, area.Bottom());
                convertToPointFAndStoreInGivenPointFVecPair(p, area, topRelative, bottomRelative, topAndBottom);
            }
            topRelative = NFmiPoint(area.Right(), area.Top());
            bottomRelative = NFmiPoint(area.Right(), area.Bottom());
            convertToPointFAndStoreInGivenPointFVecPair(p, area, topRelative, bottomRelative, topAndBottom);
            return topAndBottom;
        }

        void drawBorders(Params& p, const GdiPlusLineInfo& lineType, PointFVecPair& leftRight, PointFVecPair& topBottom)
        {
            CtrlView::DrawGdiplusCurve(p.gdiPlusGraphics, leftRight.first, lineType, false, 0, p.doc.Printing());
            CtrlView::DrawGdiplusCurve(p.gdiPlusGraphics, leftRight.second, lineType, false, 0, p.doc.Printing());
            CtrlView::DrawGdiplusCurve(p.gdiPlusGraphics, topBottom.first, lineType, false, 0, p.doc.Printing());
            CtrlView::DrawGdiplusCurve(p.gdiPlusGraphics, topBottom.second, lineType, false, 0, p.doc.Printing());
        }
    }

    void drawAreasBordersWithGivenLineInfo(Params& params, const NFmiArea& areaToDraw, const GdiPlusLineInfo& lineType, size_t resolution /*= 100*/)
    {
        PointFVecPair leftRight = leftAndRightBorderGdiPoints(params, areaToDraw, resolution);
        PointFVecPair topBottom = topAndBottomBorderGdiPoints(params, areaToDraw, resolution);
        drawBorders(params, lineType, leftRight, topBottom);
    }
}
