#pragma once

#include "NFmiStationView.h"
#include "NFmiParameterName.h"
#include "xmlliteutils/XMLite.h"
#include "CapData.h"
#include "WarningMember.h"
#include "NFmiImageMap.h"


class NFmiToolBox;
class NFmiArea;

namespace Gdiplus
{
    class PointF;
    class Bitmap;
    class GraphicsPath;
    class Font;
}

class NFmiCapView : public NFmiStationView
{
    Warnings::CapData capData_;

public:
    NFmiCapView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox *theToolBox
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamId
        , int theRowIndex
        , int theColumnIndex);

    ~NFmiCapView(void) = default;
    static void InitCapSymbolMap(const std::string &theWomlDirectory);
    void Draw(NFmiToolBox * theGTB) override;
    std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;

protected:
    int itsRowIndex;
    double itsScreenPixelSizeInMM;
    static NFmiImageMap itsCapSymbolMap;

private:
    NFmiColor getWarningColor(const std::shared_ptr<Warnings::WarningMember>& warning);
    boolean insideArea(const std::vector<NFmiPoint> &latlonVec, const NFmiPoint &theLatlon);
    void drawSymbol(const std::shared_ptr<Warnings::WarningMember>& warning, double wantedSymbolSizeInMM);
    double distance(NFmiPoint& a, NFmiPoint& b);
    void drawArrow(const std::shared_ptr<Warnings::WarningMember>& warning, double wantedSymbolSizeInMM);
};

