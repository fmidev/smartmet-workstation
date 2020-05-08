#include "NFmiIsoLineData.h"
#include "ToolmasterHatchingUtils.h"

#ifndef DISABLE_UNIRAS_TOOLMASTER

class ToolmasterHatchPolygonData
{
public:
    ToolmasterHatchPolygonData(NFmiIsoLineData& theIsoLineData, const NFmiHatchingSettings& theHatchSettings);

    void setWorldLimits(const TMWorldLimits& worldLimits);
    bool isHatchPolygonDrawn(int currentPolygonIndex, int currentPolygonFloatDataTotalIndex, int currentPolygonIntDataTotalIndex, int currentCoordinateDataTotalIndex);
    bool isInsideHatchLimits(float value);

    NFmiHatchingSettings hatchSettings_;
    std::vector<float> hatchClassValues_;
    int polygonCount_ = 0;
    int polygonElementCount_ = 0;
    int polygonIntDataCount_ = 0;
    int polygonFloatDataCount_ = 0;
    std::vector<int> polygonSizeNumbers_;
    std::vector<float> polygonCoordinateX_;
    std::vector<float> polygonCoordinateY_;
    // Kuinka monta arvoa kuhunkin polygonin float dataan kuuluu
    std::vector<int> polygonDataFloatNumberArray_;
    // Polygonin float data kertoo kunkin polygonin pisteen arvon visualisoitavassa datassa, 
    // sen avulla voidaan p‰‰tell‰ pit‰‰kˆ hatch piirt‰‰ t‰h‰n polygoniin vai ei.
    std::vector<float> polygonDataFloatArray_;
    // Kuinka monta arvoa kunkin polygonin int dataan kuuluu
    std::vector<int> polygonDataIntNumberArray_;
    // Polygonin int data kertoo kunkin polygonin pisteen x- ja y- hilapisteen arvot (indeksit alkavat 1:st‰), 
    // sen avulla voidaan p‰‰tell‰ mill‰ rivill‰ polygon on ja ett‰ pit‰‰kˆ hatch piirt‰‰ t‰h‰n polygoniin vai ei.
    std::vector<int> polygonDataIntArray_;
    // T‰m‰ asetetaan true:ksi, jos polygonien laskut menev‰t oikein
    bool continueHatchDraw_ = false;
    // T‰h‰n talletetaan jokaisen visualisoinneissa k‰ytetyn hilarivin y-koordinaatti.
    // Dataosassa polygonDataIntArray_ on ToolMasterin polygoniin kohdistama rivi numero,
    // joka alkaa 1:st‰ ylˆsp‰in. N‰ytˆn alaosassa oleva hilarivi on tuo 1. rivi.
    std::vector<float> gridRowCoordinateY_;
    // Jos arvo on 0, ei polygoni ole yhteydess‰ alahilariviin ja muuten on.
    // K‰ytet‰‰n boolean tyypin sijasta char:ia, jotta debuggaaminen on mahdollista
    std::vector<char> polygonConnectedToBottomRow_;
    std::vector<PolygonsBottomEdgeRelation> polygonsFloatDataBottomEdgeRelations_;
    // Miten polygoni on kiinni pohjahilariviss‰ (kokonaan, nousevasti, laskevasti, ei ollenkaan)
    std::vector<PolygonsBottomEdgeTouching> polygonsBottomEdgeTouchings_;
    int dataGridSizeX_ = 0;
    int dataGridSizeY_ = 0;
    TMWorldLimits worldLimits_;
    float usedToolmasterRelatedBigEpsilon_ = 0.002f;

    static float normallyUsedCoordinateEpsilon_;
    // T‰ll‰ kertoimella muokataan karttaruudun korkeuteen liittyv‰‰ epsilon laskentaa.
    // Arvo on siis normaalisti 1, mutta laskettua kertointa voidaan kasvattaa tai pienent‰‰ haluttaessa t‰ll‰.
    static float toolmasterRelatedBigEpsilonFactor_;
    // T‰t‰ indeksi‰ k‰ytet‰‰n kun mets‰stet‰‰n ongelma polygoneja debuggerilla
    static int debugHelperWantedPolygonIndex1_;
    static int debugHelperWantedPolygonIndex2_;


    template<typename T>
    static std::pair<T, size_t> getPreviousValue(size_t index, const std::vector<T>& values)
    {
        size_t previousIndex = index - 1;
        // Jos ollaan alussa, kurkataan arvo reunan yli lopusta
        if(index == 0)
            previousIndex = values.size() - 1;

        return std::make_pair(values[previousIndex], previousIndex);
    }

    template<typename T>
    static std::pair<T, size_t> getNextValue(size_t index, const std::vector<T>& values)
    {
        size_t nextIndex = index + 1;
        // Jos ollaan lopussa, kurkataan arvo reunan yli alusta
        if(index == values.size() - 1)
            nextIndex = 0;

        return std::make_pair(values[nextIndex], nextIndex);
    }

    static bool areTwoPointsExcatlySame(size_t pointIndex1, size_t pointIndex2, const std::vector<float>& polygonsCoordinatesX, const std::vector<float>& polygonsCoordinatesY);

private:

    bool isHatchPolygonDrawn(const std::vector<int>& bottomRowPointsInsidePolygon, const std::vector<int>& bottomRowPointValuesInsideHatchLimits);
    std::vector<int> areBottomRowPointValuesInsideHatchLimits(int currentPolygonIndex, int currentPolygonFloatDataTotalIndex);
    std::vector<IntPoint> getToolMasterBottomRowPoints(int currentPolygonIndex, int currentPolygonIntDataTotalIndex);
    std::vector<FloatPoint> toolmasterPointsToRelative(const std::vector<IntPoint>& toolMasterBottomRowPoints);
    FloatPoint toolmasterPointToRelative(const IntPoint& toolMasterPoint);
    std::vector<int> areBottomRowPointsInsidePolygon(const std::vector<FloatPoint>& relativeBottomRowPoints, int currentPolygonIndex, int currentCoordinateDataTotalIndex);
    std::vector<int> checkIfBottomRowPointsAreInsideXRanges(const std::vector<FloatPoint>& relativeBottomRowPoints, const std::vector<std::pair<float, float>>& botttomRowXRanges);
    bool isPointInsideXRanges(const FloatPoint& point, const std::vector<std::pair<float, float>>& botttomRowXRanges);
    bool isValueInsideRange(float value, const std::pair<float, float>& range);
    std::pair<float, float> calculateTotalValueRange(const std::vector<float>& polygonsCoordinates);
    std::pair<bool, size_t> isSingleBottomRowTouchingCase(size_t coordinateIndex, const std::vector<float>& polygonsCoordinatesY, float bottomRowCoordinateY);
    CoordinateYStatus calculateCoordinateYStatus(float value, float bottomRowCoordinateY, float topRowCoordinateY);
    std::vector<CoordinateYStatus> calculateCoordinateYStatusVector(const std::vector<float>& polygonsCoordinatesY, float bottomRowCoordinateY, float topRowCoordinateY);
    bool doYPointCoordinateFixes(std::vector<float>& polygonsCoordinatesY_inOut, const std::vector<float>& polygonsCoordinatesX, float bottomRowCoordinateY, float topRowCoordinateY);
    std::vector<std::pair<float, float>> getBottomRowXRanges(int currentPolygonIndex, int currentCoordinateDataTotalIndex, float bottomRowCoordinateY);
    void initializeRowInformation();
    void doCoordinateYFixes();
    void initializeGridRowCoordinateYValues();
    void initializePolygonConnectionToBottomGridRow();
    void calculatePolygonBottomEdgeTouchings(size_t currentPolygonCoordinateCounter, size_t polygonCoordinateSize, float polygonsBottomCoordinateY);
    void initializePolygonBottomEdgeRelations();
    PolygonsBottomEdgeRelation calcPolygonsBottomEdgeRelation(bool startsInsideLimits, bool changesFromStart);
    float calculateUsedToolmasterEpsilon(float singleMapSubViewHeightInMillimeters, float dataGridToViewHeightRatio);
};

#endif // DISABLE_UNIRAS_TOOLMASTER
