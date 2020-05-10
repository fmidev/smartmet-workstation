#include "ToolmasterHatchPolygonData.h"
#include "CtrlViewFunctions.h"
#include "NFmiDataModifierMin.h"
#include "PolygonRelationsToBottomGridRowCalculator.h"
#include "catlog/catlog.h"

#ifndef DISABLE_UNIRAS_TOOLMASTER

#include <agX/agxfont.h>
#include <agX/agx.h>


float ToolmasterHatchPolygonData::normallyUsedCoordinateEpsilon_ = std::numeric_limits<float>::epsilon() * 4;
float ToolmasterHatchPolygonData::toolmasterRelatedBigEpsilonFactor_ = 1.f;
int ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex1_ = 757;
int ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex2_ = 0;

// Hatchiin liittyvissa polygonien y-koordinaatti korjauksissa meita kiinnostaa vain pitkat 
// vaaka suuntaiset polygoni edget. Eli meita kiinnostaa tapaukset, missa edge on vahintain 
// 20 % koko polygonin pituudesta (Toolmaster tekee jostain syysta vika y-koordinaatteja 
// top/bottom raja pisteen viereen, jos tarkoitus on ollut piirtaa pitkaa vaakaviivaa).
const float g_minimumEdgeWidthLength = 0.2f;


ToolmasterHatchPolygonData::ToolmasterHatchPolygonData(NFmiIsoLineData& theIsoLineData, const NFmiHatchingSettings& theHatchSettings)
    :hatchSettings_(theHatchSettings),
    hatchClassValues_(2, 0)
{
    usedToolmasterRelatedBigEpsilon_ = calculateUsedToolmasterEpsilon(static_cast<float>(theIsoLineData.itsSingleSubMapViewHeightInMillimeters), static_cast<float>(theIsoLineData.itsDataGridToViewHeightRatio));
    XuViewWorld();
    XuIsolineSplineSmoothing(1);

    hatchClassValues_[0] = hatchSettings_.itsHatchLowerLimit;
    hatchClassValues_[1] = hatchSettings_.itsHatchUpperLimit;
    XuClasses(&hatchClassValues_[0], static_cast<int>(hatchClassValues_.size()));
    // onko t‰m‰ turha????
    XuShadingColorIndices(theIsoLineData.itsCustomColorContoursColorIndexies.data(), 3);
    float lineWidth = 0.f;
    XuIsolineWidths(&lineWidth, 1); // ei piirret‰ isoviivoja sheidauksen yhteydess‰
    XuMapDrawOptions(XuPROBE); // piirret‰‰n polygonit erikseen toisella ohjelmalla, ett‰ saadaan hatch kuviot mukaan
    XuContourDraw(theIsoLineData.itsVectorFloatGridData.data(), theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);

    int mapComponent = Xu2D_CONTOUR;
    XuMapPolygonsNumberSize(mapComponent, &polygonCount_, &polygonElementCount_);

    if(polygonCount_ <= 0 || polygonElementCount_ <= 0)
        return; // ei jatketa, jos ei polygoneja lˆytynyt

    polygonSizeNumbers_.resize(polygonCount_, 0);
    polygonCoordinateX_.resize(polygonElementCount_, 0);
    polygonCoordinateY_.resize(polygonElementCount_, 0);
    XuMapPolygonsQuery(mapComponent, polygonSizeNumbers_.data(), polygonCoordinateX_.data(), polygonCoordinateY_.data());
    int polygonCount2 = 0; // t‰m‰ polygon koko kysyt‰‰n toistamiseen, pit‰isi olla sama luku molemmissa
    XuMapPolygonsDataSize(mapComponent, &polygonCount2, &polygonIntDataCount_, &polygonFloatDataCount_);

    if(polygonCount2 == 0)
        return; // ei jatketa, jos ei polygoneja lˆytynyt

    // Kuinka monta arvoa kuhunkin polygonin float dataan kuuluu
    polygonDataFloatNumberArray_.resize(polygonCount2, 0);
    // Polygonin float data kertoo kunkin polygonin pisteen arvon visualisoitavassa datassa, 
    // sen avulla voidaan p‰‰tell‰ pit‰‰kˆ hatch piirt‰‰ t‰h‰n polygoniin vai ei.
    polygonDataFloatArray_.resize(polygonFloatDataCount_, 0);
    XuMapPolygonsDataFloatQuery(mapComponent, polygonDataFloatNumberArray_.data(), polygonDataFloatArray_.data());

    // Kuinka monta arvoa kunkin polygonin int dataan kuuluu
    polygonDataIntNumberArray_.resize(polygonCount2, 0);
    polygonDataIntArray_.resize(polygonIntDataCount_, 0);
    XuMapPolygonsDataIntQuery(mapComponent, polygonDataIntNumberArray_.data(), polygonDataIntArray_.data());

    dataGridSizeX_ = theIsoLineData.itsXNumber;
    dataGridSizeY_ = theIsoLineData.itsYNumber;

    initializeRowInformation();
    continueHatchDraw_ = true;
}

void ToolmasterHatchPolygonData::setWorldLimits(const TMWorldLimits& worldLimits)
{
    worldLimits_ = worldLimits;
}

bool ToolmasterHatchPolygonData::isHatchPolygonDrawn(int currentPolygonIndex, int currentPolygonFloatDataTotalIndex, int currentPolygonIntDataTotalIndex, int currentCoordinateDataTotalIndex)
{
    // 1. Tee lista ToolMasterin polygoniin liittyvist‰ pohjarivin pisteist‰ int-datasta
    auto toolMasterBottomRowPoints = getToolMasterBottomRowPoints(currentPolygonIndex, currentPolygonIntDataTotalIndex);
    // 2. Tee konversio Toolmaster pisteist‰ relatiiviseen maailmaan
    auto relativeBottomRowPoints = toolmasterPointsToRelative(toolMasterBottomRowPoints);
    // 3. Tutki onko toolmaster pisteet polygonin pohjarivill‰ 'fyysisesti' vai ei
    auto bottomRowPointsInsidePolygon = areBottomRowPointsInsidePolygon(relativeBottomRowPoints, currentPolygonIndex, currentCoordinateDataTotalIndex);
    // 4. Mik‰ on kunkin pisteen arvo, eli onko se hatch rajojen sis‰ll‰ vai ei
    auto bottomRowPointValuesInsideHatchLimits = areBottomRowPointValuesInsideHatchLimits(currentPolygonIndex, currentPolygonFloatDataTotalIndex);
    // 5. Tee p‰‰telm‰, pit‰‰kˆ polygonin hatch piirt‰‰ vai ei
    return isHatchPolygonDrawn(bottomRowPointsInsidePolygon, bottomRowPointValuesInsideHatchLimits);
}


bool ToolmasterHatchPolygonData::isInsideHatchLimits(float value)
{
    if(value == kFloatMissing)
        return false;
    else
        return (value >= hatchClassValues_[0] && value <= hatchClassValues_[1]);
}

bool ToolmasterHatchPolygonData::isHatchPolygonDrawn(const std::vector<int>& bottomRowPointsInsidePolygon, const std::vector<int>& bottomRowPointValuesInsideHatchLimits)
{
    auto pointSize = bottomRowPointsInsidePolygon.size();
    if(pointSize && (pointSize == bottomRowPointValuesInsideHatchLimits.size()))
    {
        int insidePolygonAndLimits = 0;
        int insidePolygonButOutsideLimits = 0;
        int outsidePolygonButInsideLimits = 0;
        int outsidePolygonAndLimits = 0;
        for(size_t pointIndex = 0; pointIndex < pointSize; pointIndex++)
        {
            if(bottomRowPointsInsidePolygon[pointIndex] && bottomRowPointValuesInsideHatchLimits[pointIndex])
                insidePolygonAndLimits++;
            else if(bottomRowPointsInsidePolygon[pointIndex] && !bottomRowPointValuesInsideHatchLimits[pointIndex])
                insidePolygonButOutsideLimits++;
            else if(!bottomRowPointsInsidePolygon[pointIndex] && bottomRowPointValuesInsideHatchLimits[pointIndex])
                outsidePolygonButInsideLimits++;
            else
                outsidePolygonAndLimits++;
        }

        int totallyInOrOut = insidePolygonAndLimits + outsidePolygonAndLimits;
        int insideAndOutside = insidePolygonButOutsideLimits + outsidePolygonButInsideLimits;
        if(totallyInOrOut > insideAndOutside)
            return true;
        else
            return false;
    }
    return false;
}

std::vector<int> ToolmasterHatchPolygonData::areBottomRowPointValuesInsideHatchLimits(int currentPolygonIndex, int currentPolygonFloatDataTotalIndex)
{
    auto floatDataCount = polygonDataFloatNumberArray_[currentPolygonIndex];
    std::vector<float> bottomRowPointValues;
    for(size_t floatDataIndex = 0; floatDataIndex < floatDataCount; floatDataIndex++)
        bottomRowPointValues.push_back(polygonDataFloatArray_[currentPolygonFloatDataTotalIndex + floatDataIndex]);

    std::vector<int> valuesInsideHatchLimits;
    for(auto value : bottomRowPointValues)
        valuesInsideHatchLimits.push_back(isInsideHatchLimits(value));

    return valuesInsideHatchLimits;
}

std::vector<IntPoint> ToolmasterHatchPolygonData::getToolMasterBottomRowPoints(int currentPolygonIndex, int currentPolygonIntDataTotalIndex)
{
    std::vector<IntPoint> toolMasterBottomRowPoints;
    auto intDataCount = polygonDataIntNumberArray_[currentPolygonIndex];
    for(size_t intDataIndex = 0; intDataIndex < intDataCount; intDataIndex += 2)
    {
        auto x = polygonDataIntArray_[currentPolygonIntDataTotalIndex + intDataIndex];
        auto y = polygonDataIntArray_[currentPolygonIntDataTotalIndex + intDataIndex + 1];
        toolMasterBottomRowPoints.push_back(IntPoint(x, y));
    }
    return toolMasterBottomRowPoints;
}

std::vector<FloatPoint> ToolmasterHatchPolygonData::toolmasterPointsToRelative(const std::vector<IntPoint> &toolMasterBottomRowPoints)
{
    std::vector<FloatPoint> relativePoints;
    for(const auto& toolmasterPoint : toolMasterBottomRowPoints)
    {
        relativePoints.push_back(toolmasterPointToRelative(toolmasterPoint));
    }
    return relativePoints;
}

// Toolmaster pisteet menev‰t 1:st‰ oikean datahilakoon n-1
// Eli jos hila koko on 5x6, menee toolmaster pisteet 1,1 - 4,5
// Lis‰ksi pisteet alkavat oikean datan 0,0 pisteest‰ ja menev‰t n-1,m-1 pisteeseen,
// eli yl‰rivi ja oikea sarake j‰‰v‰t puuttumaan kokonaan.
FloatPoint ToolmasterHatchPolygonData::toolmasterPointToRelative(const IntPoint& toolMasterPoint)
{
    auto toolMasterX = static_cast<float>(toolMasterPoint.x);
    auto x = worldLimits_.x_min + (toolMasterX - 1.f) * (worldLimits_.width() / (dataGridSizeX_ - 1.f));
    auto toolMasterY = static_cast<float>(toolMasterPoint.y);
    auto y = worldLimits_.y_min + (toolMasterY - 1.f) * (worldLimits_.height() / (dataGridSizeY_ - 1.f));
    return FloatPoint(x, y);
}

// Onko ToolMaster pohjarivin pisteet (intDatasta muutettuina suhteelliseen avaruuteen) polygonin sis‰ll‰ vai ei.
// Oikeasti haetaan bool arvoa, mutta palautetaan int muodossa, jotta paluu datan debuggaus on mahdollista.
std::vector<int> ToolmasterHatchPolygonData::areBottomRowPointsInsidePolygon(const std::vector<FloatPoint>& relativeBottomRowPoints, int currentPolygonIndex, int currentCoordinateDataTotalIndex)
{
    if(relativeBottomRowPoints.size())
    {
        // Riitt‰‰ ett‰ tutkitaan, onko kukin piste kahden per‰kk‰isen pohjarivill‰ olevan pisteen v‰liss‰
        auto botttomRowXRanges = getBottomRowXRanges(currentPolygonIndex, currentCoordinateDataTotalIndex, relativeBottomRowPoints.front().y);
        if(botttomRowXRanges.size())
        {
            return checkIfBottomRowPointsAreInsideXRanges(relativeBottomRowPoints, botttomRowXRanges);
        }
        // yht‰‰n rangea ei lˆytynyt, joten palautetaan oikean kokoinen false vektori
        return std::vector<int>(relativeBottomRowPoints.size(), false);
    }
    return std::vector<int>();
}

std::vector<int> ToolmasterHatchPolygonData::checkIfBottomRowPointsAreInsideXRanges(const std::vector<FloatPoint>& relativeBottomRowPoints, const std::vector<std::pair<float, float>>& botttomRowXRanges)
{
    std::vector<int> bottomRowPointsAreInside;
    for(const auto& point : relativeBottomRowPoints)
    {
        bottomRowPointsAreInside.push_back(isPointInsideXRanges(point, botttomRowXRanges));
    }
    return bottomRowPointsAreInside;
}

bool ToolmasterHatchPolygonData::isPointInsideXRanges(const FloatPoint& point, const std::vector<std::pair<float, float>>& botttomRowXRanges)
{
    for(const auto& range : botttomRowXRanges)
    {
        if(isValueInsideRange(point.x, range))
            return true;
    }
    return false;
}

bool ToolmasterHatchPolygonData::isValueInsideRange(float value, const std::pair<float, float>& range)
{
    return ((value >= range.first) && (value <= range.second));
}

CoordinateYStatus ToolmasterHatchPolygonData::calculateCoordinateYStatus(float value, float bottomRowCoordinateY, float topRowCoordinateY)
{
    CoordinateYStatus coordinateStatus = CoordinateYStatus::NoValue;
    // Tehd‰‰n ensin bottom-row tapaus
    if(CtrlViewUtils::IsEqualEnough(value, bottomRowCoordinateY, normallyUsedCoordinateEpsilon_))
        coordinateStatus = CoordinateYStatus::BottomRowValue;
    else if(CtrlViewUtils::IsEqualEnough(value, bottomRowCoordinateY, usedToolmasterRelatedBigEpsilon_))
        coordinateStatus = CoordinateYStatus::BottomRowInToolmasterMarginCase;

    // Jos ollaan selke‰sti jo alarajalla, ei tehd‰ en‰‰ mit‰‰n
    if(coordinateStatus != CoordinateYStatus::BottomRowValue)
    {
        if(CtrlViewUtils::IsEqualEnough(value, topRowCoordinateY, normallyUsedCoordinateEpsilon_))
            coordinateStatus = CoordinateYStatus::TopRowValue;
        else if(CtrlViewUtils::IsEqualEnough(value, topRowCoordinateY, usedToolmasterRelatedBigEpsilon_))
        {
            // Tarkistetaan onko piste virhemarginaalien sis‰ll‰ tarpeeksi l‰hell‰ molempia rivej‰ vaiko vain top-row:ta
            if(coordinateStatus == CoordinateYStatus::BottomRowInToolmasterMarginCase)
                coordinateStatus = CoordinateYStatus::BothRowsInToolmasterMarginCase;
            else
                coordinateStatus = CoordinateYStatus::TopRowInToolmasterMarginCase;
        }
    }

    if(coordinateStatus == CoordinateYStatus::NoValue)
        coordinateStatus = CoordinateYStatus::ClearMiddleValue;

    return coordinateStatus;
}

std::vector<CoordinateYStatus> ToolmasterHatchPolygonData::calculateCoordinateYStatusVector(const std::vector<float>& polygonsCoordinatesY, float bottomRowCoordinateY, float topRowCoordinateY)
{
    std::vector<CoordinateYStatus> statusVector;
    for(auto coordinateY : polygonsCoordinatesY)
    {
        statusVector.push_back(calculateCoordinateYStatus(coordinateY, bottomRowCoordinateY, topRowCoordinateY));
    }
    return statusVector;
}

static bool DoYCoordinateFix(size_t currentCoordinateIndex, const std::vector<PolygonPointFixStatus>& polygonPointFixStatusVector, float wantedBottomRowCoordinateY, float wantedTopRowCoordinateY, std::vector<float>& polygonsCoordinatesY_inOut)
{
    auto fixStatus = polygonPointFixStatusVector[currentCoordinateIndex];
    switch(fixStatus)
    {
    case PolygonPointFixStatus::AccuracyBottomFixNeeded:
    case PolygonPointFixStatus::BottomFixNeeded:
        polygonsCoordinatesY_inOut[currentCoordinateIndex] = wantedBottomRowCoordinateY;
        return true;
    case PolygonPointFixStatus::AccuracyTopFixNeeded:
    case PolygonPointFixStatus::TopFixNeeded:
        polygonsCoordinatesY_inOut[currentCoordinateIndex] = wantedTopRowCoordinateY;
        return true;
    default:
        return false;
    }
}


static float calculatePolygonsWidth(const std::vector<float>& polygonsCoordinatesX)
{
    auto minmaxIterPair = std::minmax_element(polygonsCoordinatesX.begin(), polygonsCoordinatesX.end());
    return (*minmaxIterPair.second) - (*minmaxIterPair.first);
}

// 1. Laske koko polygonin leveys
// 2. Laske kunkin pisteen ja siit‰ seuraavaan pisteeseen olevan edgen pituus ja suhteuta se kokonais pituuteen.
static std::vector<float> calculatePolygonEdgeRelativeWidths(const std::vector<float>& polygonsCoordinatesX)
{
    std::vector<float> edgeWidths(polygonsCoordinatesX.size(), 0);
    auto polygonWidth = ::calculatePolygonsWidth(polygonsCoordinatesX);
    for(auto coordinateIndex = 0ull; coordinateIndex < polygonsCoordinatesX.size(); coordinateIndex++)
    {
        auto currentCoordinate = polygonsCoordinatesX[coordinateIndex];
        // Seuraava koordinaatti on joko coordinateIndex+1 -kohdasta tai jos ollaan jo viimeisessa pisteess‰, vektorin 1. piste
        auto nextCoordinate = (coordinateIndex < (polygonsCoordinatesX.size() - 1)) ? polygonsCoordinatesX[coordinateIndex+1] : polygonsCoordinatesX[0];
        edgeWidths[coordinateIndex] = std::abs(currentCoordinate - nextCoordinate) / polygonWidth;
    }
    return edgeWidths;
}

class FixedPointCandidateData
{
public:
    FixedPointCandidateData() = default;
    FixedPointCandidateData(size_t fixedPointIndex, size_t fixedEdgePointIndex, CoordinateYStatus fixedEdgePointYStatus, float edgeLength)
        :fixedPointIndex_(fixedPointIndex)
        , fixedEdgePointIndex_(fixedEdgePointIndex)
        , fixedEdgePointYStatus_(fixedEdgePointYStatus)
        , edgeLength_(edgeLength)
    {}

    size_t fixedPointIndex_ = 0;
    size_t fixedEdgePointIndex_ = 0;
    CoordinateYStatus fixedEdgePointYStatus_ = CoordinateYStatus::NoValue;
    float edgeLength_ = 0;

};

static std::vector<FixedPointCandidateData> calculateFixedPointCandidates(const std::vector<CoordinateYStatus>& coordinateYStatusVector, const std::vector<float>& polygonEdgeRelativeWidths)
{
    std::vector<FixedPointCandidateData> fixedPointCandidates;
    // Ei tarkasteluja, jos polygonivektori on liian lyhyt (3:n ja 4:n pituisia on olemassa)
    if(coordinateYStatusVector.size() >= 5)
    {
        // Huom! vektorin 1. ja viimeinen piste ovat samoja, siita seuraa loopituksessa seuraavia asioita:
        // Ensimmaiset previous arvot otetaan vektorien toiseksi viimeisesta pisteesta.
        // Itse looppi mennaan 1. pisteesta toiseksi viimeisimpaan.
        // Toiseksi viimeisimman pisteen next-arvot voidaan ottaa vektorin viimeisesta pisteesta.
        auto firstPreviousIndex = polygonEdgeRelativeWidths.size() - 2;
        auto previousEdgeWidth = polygonEdgeRelativeWidths[firstPreviousIndex];
        auto previousCoordinateYStatus = coordinateYStatusVector[firstPreviousIndex];
        for(auto pointIndex = 0ull; pointIndex < polygonEdgeRelativeWidths.size() - 1; pointIndex++)
        {
            auto currentEdgeWidth = polygonEdgeRelativeWidths[pointIndex];
            auto currentCoordinateYStatus = coordinateYStatusVector[pointIndex];
            if(currentCoordinateYStatus != CoordinateYStatus::BottomRowValue && currentCoordinateYStatus != CoordinateYStatus::TopRowValue)
            {
                if(currentEdgeWidth >= g_minimumEdgeWidthLength)
                {
                    auto nextCoordinateYStatus = coordinateYStatusVector[pointIndex + 1];
                    if(nextCoordinateYStatus == CoordinateYStatus::BottomRowValue || nextCoordinateYStatus == CoordinateYStatus::TopRowValue)
                    {
                        fixedPointCandidates.push_back(FixedPointCandidateData(pointIndex, pointIndex + 1, nextCoordinateYStatus, currentEdgeWidth));
                    }
                }

                if(previousEdgeWidth >= g_minimumEdgeWidthLength)
                {
                    if(previousCoordinateYStatus == CoordinateYStatus::BottomRowValue || previousCoordinateYStatus == CoordinateYStatus::TopRowValue)
                    {
                        auto previousPointIndex = (pointIndex > 0) ? pointIndex - 1 : firstPreviousIndex;
                        fixedPointCandidates.push_back(FixedPointCandidateData(pointIndex, previousPointIndex, previousCoordinateYStatus, previousEdgeWidth));
                    }
                }
            }
            previousEdgeWidth = currentEdgeWidth;
            previousCoordinateYStatus = currentCoordinateYStatus;
        }
    }
    return fixedPointCandidates;
}

static bool isForcedYCoordinateFixNeeded(float edgeLength, float pointCoordinateY, float rowCoordinateY, float usedEpsilon)
{
    // Lopullinen epsilon laketaan polygonin reunan pituuden avulla, jos reunan pituus on maksimi eli 1, sallitaan kaksinkertainen virhe.
    // Lyhyemmille reunoille (< 0.5) kuitenkin minimi on originaali usedEpsilon arvo.
    auto finalEpsilon = usedEpsilon * (2 * edgeLength);
    if(finalEpsilon < usedEpsilon)
        finalEpsilon = usedEpsilon;
    auto yDifference = std::abs(pointCoordinateY - rowCoordinateY);
    return finalEpsilon >= yDifference;
}

static std::vector<PolygonPointFixStatus> calculatePolygonPointFixStatusVector(const std::vector<CoordinateYStatus>& coordinateYStatusVector, const std::vector<FixedPointCandidateData> & fixedPointCandidates, const std::vector<float>& polygonsCoordinatesY, float usedEpsilon, float bottomRowCoordinateY, float topRowCoordinateY)
{
    std::vector<PolygonPointFixStatus> fixStatusVector;
    for(auto pointIndex = 0ull; pointIndex < coordinateYStatusVector.size(); pointIndex++)
    {
        auto coordinateYStatus = coordinateYStatusVector[pointIndex];
        switch(coordinateYStatus)
        {
        case CoordinateYStatus::BottomRowValue:
            fixStatusVector.push_back(PolygonPointFixStatus::AccuracyBottomFixNeeded);
            break;
        case CoordinateYStatus::TopRowValue:
            fixStatusVector.push_back(PolygonPointFixStatus::AccuracyTopFixNeeded);
            break;
        default:
            fixStatusVector.push_back(PolygonPointFixStatus::NoFixNeeded);
            break;
        }
    }

    if(fixedPointCandidates.size())
    {
        for(const auto& fixedPointCandidate : fixedPointCandidates)
        {
            if(fixedPointCandidate.fixedEdgePointYStatus_ == CoordinateYStatus::BottomRowValue)
            {
                if(::isForcedYCoordinateFixNeeded(fixedPointCandidate.edgeLength_, polygonsCoordinatesY[fixedPointCandidate.fixedPointIndex_], bottomRowCoordinateY, usedEpsilon))
                {
                    fixStatusVector[fixedPointCandidate.fixedPointIndex_] = PolygonPointFixStatus::BottomFixNeeded;
                }
            }
            else if(fixedPointCandidate.fixedEdgePointYStatus_ == CoordinateYStatus::TopRowValue)
            {
                if(::isForcedYCoordinateFixNeeded(fixedPointCandidate.edgeLength_, polygonsCoordinatesY[fixedPointCandidate.fixedPointIndex_], topRowCoordinateY, usedEpsilon))
                {
                    fixStatusVector[fixedPointCandidate.fixedPointIndex_] = PolygonPointFixStatus::TopFixNeeded;
                }
            }
        }
    }

    return fixStatusVector;
}

// Uusi algoritmi polygonien korjaukseen :
// 1. Laske polygonin edge pituudet x - suunnassa
// 2. Laske pisteiden CoordinateYStatukset
// 3. Tee lista ehdokaspisteist‰, jotka pit‰‰ korjata :
//   a) Etsi edget, joiden pituus ylitt‰‰ rajan(~0.35)
//   b) Pitk‰‰n edgeen pit‰‰ kuulua bottom / top - row piste ja ei - bottom / top - row piste
//   c) Ota talteen :
//     -Korjattavan pisteen indeksi
//     - Siihen liittyv‰n top / bottom - row pisteen indeksi
//     - Siihen liittyv‰n top / bottom - row pisteen CoordinateYStatus
//     - Edgen pituus
// 4. K‰y l‰pi ehdokaslistaa :
//   a) Edgen pituus vaikuttaa k‰ytettyyn epsiloniin positiivisella korrelaatiolla
//   b) Jos boostattu epsilon ja et‰isyys haluttuun riviin riitt‰v‰t, tehd‰‰n korjaus
bool ToolmasterHatchPolygonData::doYPointCoordinateFixes(std::vector<float>& polygonsCoordinatesY_inOut, const std::vector<float>& polygonsCoordinatesX, float bottomRowCoordinateY, float topRowCoordinateY)
{
    bool hasAnyCorrectionsBeenMade = false;

    auto yCoordinateStatusVector = calculateCoordinateYStatusVector(polygonsCoordinatesY_inOut, bottomRowCoordinateY, topRowCoordinateY);
    auto edgeRelativeWidths = ::calculatePolygonEdgeRelativeWidths(polygonsCoordinatesX);
    auto fixedPointCandidates = ::calculateFixedPointCandidates(yCoordinateStatusVector, edgeRelativeWidths);
    auto polygonPointFixStatusVector = ::calculatePolygonPointFixStatusVector(yCoordinateStatusVector, fixedPointCandidates, polygonsCoordinatesY_inOut, usedToolmasterRelatedBigEpsilon_, bottomRowCoordinateY, topRowCoordinateY);
    for(size_t coordinateIndex = 0; coordinateIndex < yCoordinateStatusVector.size(); coordinateIndex++)
    {
        hasAnyCorrectionsBeenMade |= ::DoYCoordinateFix(coordinateIndex, polygonPointFixStatusVector, bottomRowCoordinateY, topRowCoordinateY, polygonsCoordinatesY_inOut);
    }

    return hasAnyCorrectionsBeenMade;
}

std::vector<std::pair<float, float>> ToolmasterHatchPolygonData::getBottomRowXRanges(int currentPolygonIndex, int currentCoordinateDataTotalIndex, float bottomRowCoordinateY)
{
    std::vector<std::pair<float, float>> bottomRowXRanges;
    auto polygonCoordinateSize = polygonSizeNumbers_[currentPolygonIndex];
    auto startIterX = polygonCoordinateX_.begin() + currentCoordinateDataTotalIndex;
    std::vector<float> thisPolygonsCoordinatesX(startIterX, startIterX + polygonCoordinateSize);
    auto startIterY = polygonCoordinateY_.begin() + currentCoordinateDataTotalIndex;
    std::vector<float> thisPolygonsCoordinatesY(startIterY, startIterY + polygonCoordinateSize);
    if(thisPolygonsCoordinatesX.size())
    {
        auto previousValueX = thisPolygonsCoordinatesX.front();
        auto previousValueY = thisPolygonsCoordinatesY.front();
        auto currentValueX = 0.f;
        auto currentValueY = 0.f;
        for(size_t coordinateIndex = 1; coordinateIndex < thisPolygonsCoordinatesX.size(); coordinateIndex++)
        {
            currentValueX = thisPolygonsCoordinatesX[coordinateIndex];
            currentValueY = thisPolygonsCoordinatesY[coordinateIndex];
            if(CtrlViewUtils::IsEqualEnough(currentValueY, bottomRowCoordinateY, normallyUsedCoordinateEpsilon_) && CtrlViewUtils::IsEqualEnough(previousValueY,  bottomRowCoordinateY, normallyUsedCoordinateEpsilon_))
            {
                // Varmistetaan ett‰ luvut menevat rangeen nousevassa j‰rjestyksess‰
                if(previousValueX < currentValueX)
                    bottomRowXRanges.push_back(std::make_pair(previousValueX, currentValueX));
                else
                    bottomRowXRanges.push_back(std::make_pair(currentValueX, previousValueX));
            }
            previousValueX = currentValueX;
            previousValueY = currentValueY;
        }
    }
    return bottomRowXRanges;
}

void ToolmasterHatchPolygonData::initializeRowInformation()
{
    initializeGridRowCoordinateYValues();
    doCoordinateYFixes();
    initializePolygonConnectionToBottomGridRow();
    initializePolygonBottomEdgeRelations();
}

// Toolmasteri laskemien polygonien hienoisista  ainakin pohjatasoon y-koordinaatti ongelmista johtuen
// yritet‰‰n korjata kaikkien polygonien y-koordinaatteja tarpeen mukaan.
void ToolmasterHatchPolygonData::doCoordinateYFixes()
{
    size_t currentPolygonCoordinateCounter = 0;
    size_t currentIntDataCounter = 0;
    for(size_t polygonIndex = 0; polygonIndex < polygonSizeNumbers_.size(); polygonIndex++)
    {
        int polygonCoordinateSize = polygonSizeNumbers_[polygonIndex];
        int intDataSize = polygonDataIntNumberArray_[polygonIndex];
        // +1:ll‰ saadaan rivi koordinaatti
        size_t polygonsToolmasterGridRowIndex = polygonDataIntArray_[currentIntDataCounter + 1];
        // Toolmaster rivit alkavat 1:st‰, joten hakuindeksi‰ pit‰‰ v‰hent‰‰ 1:ll‰.
        float polygonsBottomCoordinateY = gridRowCoordinateY_[polygonsToolmasterGridRowIndex - 1];
        // Toolmaster rivit alkavat 1:st‰, joten toprivin hakuindeksi‰ bottomRowIndex + 1,
        // mutta j‰tet‰‰n viimeinen rivi v‰liin
        float polygonsTopCoordinateY = (polygonsToolmasterGridRowIndex < gridRowCoordinateY_.size()) ? gridRowCoordinateY_[polygonsToolmasterGridRowIndex] : kFloatMissing;

        auto startIterX = polygonCoordinateX_.begin() + currentPolygonCoordinateCounter;
        std::vector<float> currentPolygonsCoordinatesX(startIterX, startIterX + polygonCoordinateSize);
        auto startIterY = polygonCoordinateY_.begin() + currentPolygonCoordinateCounter;
        std::vector<float> currentPolygonsCoordinatesY(startIterY, startIterY + polygonCoordinateSize);
        if(doYPointCoordinateFixes(currentPolygonsCoordinatesY, currentPolygonsCoordinatesX, polygonsBottomCoordinateY, polygonsTopCoordinateY))
        {
            std::copy(currentPolygonsCoordinatesY.begin(), currentPolygonsCoordinatesY.end(), startIterY);
        }

        currentPolygonCoordinateCounter += polygonCoordinateSize;
        currentIntDataCounter += intDataSize;
    }
}

// Etsi jokaisen hila rivin suhteellinen y-koordinaatti
void ToolmasterHatchPolygonData::initializeGridRowCoordinateYValues()
{
    gridRowCoordinateY_.clear();
    size_t currentPolygonCoordinateCounter = 0;
    size_t currentIntDataCounter = 0;
    int currentToolmasterGridRowIndex = -1;
    NFmiDataModifierMin minCoodinateY;
    for(size_t polygonIndex = 0; polygonIndex < polygonSizeNumbers_.size(); polygonIndex++)
    {
        int polygonCoordinateSize = polygonSizeNumbers_[polygonIndex];
        int intDataSize = polygonDataIntNumberArray_[polygonIndex];
        // Otetaan t‰h‰n polygoniin liittyv‰ Toolmaster rivinumero (int-array sis‰lt‰‰ polygonin hilapisteen sarake-rivi -pareja).
        // Polygonin int-datan jokainen hilapiste viittaa aina vain yhteen ja ainoaan hilariviin (polygonin alla olevaan riviin).
        int polygonsToolmasterGridRowIndex = polygonDataIntArray_[currentIntDataCounter + 1]; // +1:ll‰ saadaan rivi koordinaatti
        if(currentToolmasterGridRowIndex != polygonsToolmasterGridRowIndex)
        {
            if(currentToolmasterGridRowIndex >= 0)
            {
                // Otetaan k‰sitelty rivin y-koordinaatti talteen
                gridRowCoordinateY_.push_back(static_cast<float>(minCoodinateY.FloatValue()));
                // Nollataan ja asetetaan muuttujia seuraavan rivin laskentaa varten
                minCoodinateY.Clear();
            }
            currentToolmasterGridRowIndex = polygonsToolmasterGridRowIndex;
        }
        for(size_t coordinateIndex = currentPolygonCoordinateCounter; coordinateIndex < currentPolygonCoordinateCounter + polygonCoordinateSize; coordinateIndex++)
        {
            minCoodinateY.Calculate(polygonCoordinateY_[coordinateIndex]);
        }
        currentPolygonCoordinateCounter += polygonCoordinateSize;
        currentIntDataCounter += intDataSize;
    }
    // Otetaan viel‰ viimeisinkin k‰sitelty rivi (y-koordinaatti) talteen
    gridRowCoordinateY_.push_back(static_cast<float>(minCoodinateY.FloatValue()));
}

// Tutki jokaisen polygonin suhde alariviin, onko kosketus ala- vai yl‰osaan, jos kosketus molempiin => kosketus alaosaan = true.
// Tehd‰‰n myˆs toinen tyˆ eli tutkitaan miten polygoni liittyy pohjariviin 'fyysisesti'.
void ToolmasterHatchPolygonData::initializePolygonConnectionToBottomGridRow()
{
    polygonConnectedToBottomRow_.clear();
    polygonsBottomEdgeTouchings_.clear();
    size_t currentPolygonCoordinateCounter = 0;
    size_t currentIntDataCounter = 0;
    for(size_t polygonIndex = 0; polygonIndex < polygonSizeNumbers_.size(); polygonIndex++)
    {
        int polygonCoordinateSize = polygonSizeNumbers_[polygonIndex];
        int intDataSize = polygonDataIntNumberArray_[polygonIndex];
        size_t polygonsToolmasterGridRowIndex = polygonDataIntArray_[currentIntDataCounter + 1]; // +1:ll‰ saadaan rivi koordinaatti
        float polygonsBottomCoordinateY = gridRowCoordinateY_[polygonsToolmasterGridRowIndex - 1]; // Toolmaster rivit alkavat 1:st‰, joten hakuindeksi‰ pit‰‰ v‰hent‰‰ 1:ll‰.
        bool polygonIsConnectedToBottomGridRow = false;
        for(size_t coordinateIndex = currentPolygonCoordinateCounter; coordinateIndex < currentPolygonCoordinateCounter + polygonCoordinateSize; coordinateIndex++)
        {
            auto coordinateY = polygonCoordinateY_[coordinateIndex];
            if(coordinateY == polygonsBottomCoordinateY)
            {
                polygonIsConnectedToBottomGridRow = true;
                break;
            }
        }
        polygonConnectedToBottomRow_.push_back(polygonIsConnectedToBottomGridRow);
        calculatePolygonBottomEdgeTouchings(currentPolygonCoordinateCounter, polygonCoordinateSize, polygonsBottomCoordinateY);
        currentPolygonCoordinateCounter += polygonCoordinateSize;
        currentIntDataCounter += intDataSize;
    }
}

void ToolmasterHatchPolygonData::calculatePolygonBottomEdgeTouchings(size_t currentPolygonCoordinateCounter, size_t polygonCoordinateSize, float polygonsBottomCoordinateY)
{
    PolygonRelationsToBottomGridRowCalculator calculator;
    calculator.addCoordinatesX(polygonCoordinateX_, currentPolygonCoordinateCounter, polygonCoordinateSize);
    calculator.addCoordinatesY(polygonCoordinateY_, currentPolygonCoordinateCounter, polygonCoordinateSize);
    calculator.setBottomRowCoordinateY(polygonsBottomCoordinateY);
    polygonsBottomEdgeTouchings_.push_back(calculator.getResult());
}
    
void ToolmasterHatchPolygonData::initializePolygonBottomEdgeRelations()
{
    polygonsFloatDataBottomEdgeRelations_.clear();
    size_t currentFloatDataCounter = 0;
    for(size_t polygonIndex = 0; polygonIndex < polygonSizeNumbers_.size(); polygonIndex++)
    {
        int floatDataSize = polygonDataFloatNumberArray_[polygonIndex];
        auto startsInsideLimits = isInsideHatchLimits(polygonDataFloatArray_[currentFloatDataCounter]);
        bool changesFromStart = false;
        for(size_t floatDataIndex = currentFloatDataCounter + 1; floatDataIndex < currentFloatDataCounter + floatDataSize; floatDataIndex++)
        {
            auto isCurrentnlyInsideLimits = isInsideHatchLimits(polygonDataFloatArray_[floatDataIndex]);
            if(startsInsideLimits != isCurrentnlyInsideLimits)
            {
                changesFromStart = true;
                break;
            }
        }
        polygonsFloatDataBottomEdgeRelations_.push_back(calcPolygonsBottomEdgeRelation(startsInsideLimits, changesFromStart));
        currentFloatDataCounter += floatDataSize;
    }
}

PolygonsBottomEdgeRelation ToolmasterHatchPolygonData::calcPolygonsBottomEdgeRelation(bool startsInsideLimits, bool changesFromStart)
{
    if(changesFromStart)
    {
        if(startsInsideLimits)
            return PolygonsBottomEdgeRelation::FirstInside;
        else
            return PolygonsBottomEdgeRelation::FirstOutside;
    }
    else
    {
        if(startsInsideLimits)
            return PolygonsBottomEdgeRelation::AlwaysInside;
        else
            return PolygonsBottomEdgeRelation::AlwaysOutside;
    }
}

// Kokeellisesti haettu kolme pistett‰, jossa hatch laskut toimivat hyvin kolmelle eri kokoiselle karttaruudulle.
// 1. karttaruudun korkeus 300 mm => toolmaster-epsilon = 0.00135
// 2. karttaruudun korkeus 166 mm => toolmaster-epsilon = 0.00215
// 3. karttaruudun korkeus  83 mm => toolmaster-epsilon = 0.00385
// N‰iden pisteiden avulla laskettu 2. asteen yht‰lˆ, jonka avulla voidaan laskea sopiva kerroin jokaiselle 
// karttan‰ytˆn korkeudelle.
// Koska en ole voinut testata 300 mm isommilla n‰ytˆill‰, ja oletettavasti kerroin isommilla koilla pysyy pienen‰ tai samana,
// niin laitetaan ehto ett‰ jos x >= 300, niin k‰ytet‰‰n usedEpsilon:ina aina 0.00135.
// Laskentaan lis‰tty viel‰ kerroin ~1, jolla voidaan manipuloida k‰ytetty‰ kerrointa pinemm‰ksi tai isommaksi,
// kyseist‰ kerrointa voidaan s‰‰t‰‰ SmartMetin Settings dialogista.
float ToolmasterHatchPolygonData::calculateUsedToolmasterEpsilon(float singleMapSubViewHeightInMillimeters, float dataGridToViewHeightRatio)
{
    const float maximumTestedViewHeight = 300;
    const float maximumTestedViewHeightEpsilon = 0.00135f;

    float x = singleMapSubViewHeightInMillimeters * dataGridToViewHeightRatio;
    float usedEpsilon = maximumTestedViewHeightEpsilon;
    if(x < maximumTestedViewHeight)
        usedEpsilon = 0.00647139762019399f - 0.0000371336919312162f * x + 6.68745551018985e-8f * x * x;

    float finalEpsilon = usedEpsilon * toolmasterRelatedBigEpsilonFactor_;
    if(CatLog::doTraceLevelLogging())
    {
        std::string logMessage = "Hatching epsilon calc: map-view height [mm] = ";
        logMessage += std::to_string(singleMapSubViewHeightInMillimeters);
        logMessage += ", epsilon from formula = ";
        logMessage += std::to_string(usedEpsilon);
        logMessage += ", final epsilon from factor = ";
        logMessage += std::to_string(finalEpsilon);
        CatLog::logMessage(logMessage, CatLog::Severity::Trace, CatLog::Category::Visualization, true);
    }

    return finalEpsilon;
}

#endif // DISABLE_UNIRAS_TOOLMASTER
