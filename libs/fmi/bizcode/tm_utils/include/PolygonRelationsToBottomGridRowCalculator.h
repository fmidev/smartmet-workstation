#include "NFmiGlobals.h"
#include "ToolmasterHatchingUtils.h"
#include <vector>

// Luokalle syötetään seuraavia tietoja:
// 1. Polygonin kaikki x- ja y-koordinaatit
// 2. Polygoniin liittyvän hiladatan pohjarivin y-koordinaatti.
// Luokka laskee seuraavia asioita:
// 1) Polygonin pohjariviin liittyvä x-bounding-box (pari x-koordinaatteja)
// 2) Miten polygoni on 'fyysisessä' kosketuksissa pohjariviin
//    - Vastaus PolygonsBottomEdgeTouching enum arvona
class PolygonRelationsToBottomGridRowCalculator
{
    std::vector<float> coordinatesX_;
    std::vector<float> coordinatesY_;
    float bottomRowCoordinateY_ = kFloatMissing;
    std::pair<float, float> bottomRowTouchingCoordinateXRange_ = std::make_pair(kFloatMissing, kFloatMissing);
    PolygonsBottomEdgeTouching polygonsBottomEdgeTouching_ = PolygonsBottomEdgeTouching::NoValue;
    bool calculationOk_ = false;
public:
    void addCoordinatesX(const std::vector<float>& baseCoordinates, size_t startIndex, size_t polygonSize);
    void addCoordinatesY(const std::vector<float>& baseCoordinates, size_t startIndex, size_t polygonSize);
    void setBottomRowCoordinateY(float bottomRowCoordinateY);
    PolygonsBottomEdgeTouching getResult();

private:
    void doCalculations();
    PolygonsBottomEdgeTouching calcBottomTouchingIndex(float minTotalX, float maxTotalX, float minBottomRowTouchingX, float maxBottomRowTouchingX);
};
