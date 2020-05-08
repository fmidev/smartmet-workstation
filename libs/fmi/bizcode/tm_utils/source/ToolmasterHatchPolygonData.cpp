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
int ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex1_ = 1970;
int ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex2_ = 0;

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
    // onko tämä turha????
    XuShadingColorIndices(theIsoLineData.itsCustomColorContoursColorIndexies.data(), 3);
    float lineWidth = 0.f;
    XuIsolineWidths(&lineWidth, 1); // ei piirretä isoviivoja sheidauksen yhteydessä
    XuMapDrawOptions(XuPROBE); // piirretään polygonit erikseen toisella ohjelmalla, että saadaan hatch kuviot mukaan
    XuContourDraw(theIsoLineData.itsVectorFloatGridData.data(), theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);

    int mapComponent = Xu2D_CONTOUR;
    XuMapPolygonsNumberSize(mapComponent, &polygonCount_, &polygonElementCount_);

    if(polygonCount_ <= 0 || polygonElementCount_ <= 0)
        return; // ei jatketa, jos ei polygoneja löytynyt

    polygonSizeNumbers_.resize(polygonCount_, 0);
    polygonCoordinateX_.resize(polygonElementCount_, 0);
    polygonCoordinateY_.resize(polygonElementCount_, 0);
    XuMapPolygonsQuery(mapComponent, polygonSizeNumbers_.data(), polygonCoordinateX_.data(), polygonCoordinateY_.data());
    int polygonCount2 = 0; // tämä polygon koko kysytään toistamiseen, pitäisi olla sama luku molemmissa
    XuMapPolygonsDataSize(mapComponent, &polygonCount2, &polygonIntDataCount_, &polygonFloatDataCount_);

    if(polygonCount2 == 0)
        return; // ei jatketa, jos ei polygoneja löytynyt

    // Kuinka monta arvoa kuhunkin polygonin float dataan kuuluu
    polygonDataFloatNumberArray_.resize(polygonCount2, 0);
    // Polygonin float data kertoo kunkin polygonin pisteen arvon visualisoitavassa datassa, 
    // sen avulla voidaan päätellä pitääkö hatch piirtää tähän polygoniin vai ei.
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
    // 1. Tee lista ToolMasterin polygoniin liittyvistä pohjarivin pisteistä int-datasta
    auto toolMasterBottomRowPoints = getToolMasterBottomRowPoints(currentPolygonIndex, currentPolygonIntDataTotalIndex);
    // 2. Tee konversio Toolmaster pisteistä relatiiviseen maailmaan
    auto relativeBottomRowPoints = toolmasterPointsToRelative(toolMasterBottomRowPoints);
    // 3. Tutki onko toolmaster pisteet polygonin pohjarivillä 'fyysisesti' vai ei
    auto bottomRowPointsInsidePolygon = areBottomRowPointsInsidePolygon(relativeBottomRowPoints, currentPolygonIndex, currentCoordinateDataTotalIndex);
    // 4. Mikä on kunkin pisteen arvo, eli onko se hatch rajojen sisällä vai ei
    auto bottomRowPointValuesInsideHatchLimits = areBottomRowPointValuesInsideHatchLimits(currentPolygonIndex, currentPolygonFloatDataTotalIndex);
    // 5. Tee päätelmä, pitääkö polygonin hatch piirtää vai ei
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

// Toolmaster pisteet menevät 1:stä oikean datahilakoon n-1
// Eli jos hila koko on 5x6, menee toolmaster pisteet 1,1 - 4,5
// Lisäksi pisteet alkavat oikean datan 0,0 pisteestä ja menevät n-1,m-1 pisteeseen,
// eli ylärivi ja oikea sarake jäävät puuttumaan kokonaan.
FloatPoint ToolmasterHatchPolygonData::toolmasterPointToRelative(const IntPoint& toolMasterPoint)
{
    auto toolMasterX = static_cast<float>(toolMasterPoint.x);
    auto x = worldLimits_.x_min + (toolMasterX - 1.f) * (worldLimits_.width() / (dataGridSizeX_ - 1.f));
    auto toolMasterY = static_cast<float>(toolMasterPoint.y);
    auto y = worldLimits_.y_min + (toolMasterY - 1.f) * (worldLimits_.height() / (dataGridSizeY_ - 1.f));
    return FloatPoint(x, y);
}

// Onko ToolMaster pohjarivin pisteet (intDatasta muutettuina suhteelliseen avaruuteen) polygonin sisällä vai ei.
// Oikeasti haetaan bool arvoa, mutta palautetaan int muodossa, jotta paluu datan debuggaus on mahdollista.
std::vector<int> ToolmasterHatchPolygonData::areBottomRowPointsInsidePolygon(const std::vector<FloatPoint>& relativeBottomRowPoints, int currentPolygonIndex, int currentCoordinateDataTotalIndex)
{
    if(relativeBottomRowPoints.size())
    {
        // Riittää että tutkitaan, onko kukin piste kahden peräkkäisen pohjarivillä olevan pisteen välissä
        auto botttomRowXRanges = getBottomRowXRanges(currentPolygonIndex, currentCoordinateDataTotalIndex, relativeBottomRowPoints.front().y);
        if(botttomRowXRanges.size())
        {
            return checkIfBottomRowPointsAreInsideXRanges(relativeBottomRowPoints, botttomRowXRanges);
        }
        // yhtään rangea ei löytynyt, joten palautetaan oikean kokoinen false vektori
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

std::pair<float, float> ToolmasterHatchPolygonData::calculateTotalValueRange(const std::vector<float>& polygonsCoordinates)
{
    auto minMaxIters = std::minmax_element(polygonsCoordinates.begin(), polygonsCoordinates.end());
    return std::make_pair(*minMaxIters.first, *minMaxIters.second);
}

// Tutkii onko annetun coordinateIndex:in kohdalla oleva y-koordinaatti kiinni bottomRowCoordinateY 
// osoittamassa pohjarivissä (tiukan epsilonin rajoissa).
// Mutta niin että kumpikaan naapuri indeksi ei ole (tiukan epsilonin rajoissa).
// Mutta niin että toinen naapuri on (joka on lähempänä pohjariviä), on tarpeeksi lähellä isomman epsilonin rajoissa.
// Jos totta, palauttaa true:n ja sen korjattavan koordinaatin indeksin std::pair:issa.
std::pair<bool, size_t> ToolmasterHatchPolygonData::isSingleBottomRowTouchingCase(size_t coordinateIndex, const std::vector<float>& polygonsCoordinatesY, float bottomRowCoordinateY)
{
    auto currentValue = polygonsCoordinatesY[coordinateIndex];
    if(CtrlViewUtils::IsEqualEnough(currentValue, bottomRowCoordinateY, normallyUsedCoordinateEpsilon_))
    {
        auto previousValuePair = getPreviousValue(coordinateIndex, polygonsCoordinatesY);
        auto nextValuePair = getNextValue(coordinateIndex, polygonsCoordinatesY);
        auto diffToPrevious = std::abs(currentValue - previousValuePair.first);
        auto diffToNext = std::abs(currentValue - nextValuePair.first);
        if(diffToPrevious < diffToNext)
        {

        }
        else
        {

        }
    }
    return std::make_pair(false, 0); // false tapauksessa indeksillä ei ole väliä, joten laitetaan se vain 0:ksi
}

CoordinateYStatus ToolmasterHatchPolygonData::calculateCoordinateYStatus(float value, float bottomRowCoordinateY, float topRowCoordinateY)
{
    CoordinateYStatus coordinateStatus = CoordinateYStatus::NoValue;
    // Tehdään ensin bottom-row tapaus
    if(CtrlViewUtils::IsEqualEnough(value, bottomRowCoordinateY, normallyUsedCoordinateEpsilon_))
        coordinateStatus = CoordinateYStatus::BottomRowValue;
    else if(CtrlViewUtils::IsEqualEnough(value, bottomRowCoordinateY, usedToolmasterRelatedBigEpsilon_))
        coordinateStatus = CoordinateYStatus::BottomRowInToolmasterMarginCase;

    // Jos ollaan selkeästi jo alarajalla, ei tehdä enää mitään
    if(coordinateStatus != CoordinateYStatus::BottomRowValue)
    {
        if(CtrlViewUtils::IsEqualEnough(value, topRowCoordinateY, normallyUsedCoordinateEpsilon_))
            coordinateStatus = CoordinateYStatus::TopRowValue;
        else if(CtrlViewUtils::IsEqualEnough(value, topRowCoordinateY, usedToolmasterRelatedBigEpsilon_))
        {
            // Tarkistetaan onko piste virhemarginaalien sisällä tarpeeksi lähellä molempia rivejä vaiko vain top-row:ta
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

bool ToolmasterHatchPolygonData::areTwoPointsExcatlySame(size_t pointIndex1, size_t pointIndex2, const std::vector<float>& polygonsCoordinatesX, const std::vector<float>& polygonsCoordinatesY)
{
    auto xCoordinatesAreSame = polygonsCoordinatesX[pointIndex1] == polygonsCoordinatesX[pointIndex2];
    auto yCoordinatesAreSame = polygonsCoordinatesY[pointIndex1] == polygonsCoordinatesY[pointIndex2];
    return xCoordinatesAreSame && yCoordinatesAreSame;
}

static bool DoYCoordinateFix(size_t currentCoordinateIndex, const std::vector<CoordinateYStatus> &yCoordinateStatusVector, float wantedBottomRowCoordinateY, float wantedTopRowCoordinateY, std::vector<float>& polygonsCoordinatesY_inOut)
{
    auto &modifiedCoordinateY = polygonsCoordinatesY_inOut[currentCoordinateIndex];
    auto relativeOffsetLimit = std::abs(wantedTopRowCoordinateY - wantedBottomRowCoordinateY) / 3.f;
    auto currentStatus = yCoordinateStatusVector[currentCoordinateIndex];
    if(wantedBottomRowCoordinateY != kFloatMissing && (currentStatus == CoordinateYStatus::BottomRowValue || currentStatus == CoordinateYStatus::BottomRowInToolmasterMarginCase))
    {
        if(std::abs(modifiedCoordinateY - wantedBottomRowCoordinateY) <= relativeOffsetLimit)
        {
            // Asetetaan pohjariviin liitetty y-koordinaatti tarkalleen pohjarivin
            if(polygonsCoordinatesY_inOut[currentCoordinateIndex] != wantedBottomRowCoordinateY)
            {
                polygonsCoordinatesY_inOut[currentCoordinateIndex] = wantedBottomRowCoordinateY;
                return true;
            }
        }
    }
    else if(wantedTopRowCoordinateY != kFloatMissing && (currentStatus == CoordinateYStatus::TopRowValue || currentStatus == CoordinateYStatus::TopRowInToolmasterMarginCase))
    {
        if(std::abs(modifiedCoordinateY - wantedTopRowCoordinateY) <= relativeOffsetLimit)
        {
            // Asetetaan pohjariviin liitetty y-koordinaatti tarkalleen pohjarivin
            if(polygonsCoordinatesY_inOut[currentCoordinateIndex] != wantedTopRowCoordinateY)
            {
                polygonsCoordinatesY_inOut[currentCoordinateIndex] = wantedTopRowCoordinateY;
                return true;
            }
        }
    }
    return false;
}

static bool DoYCoordinateFix_ver2(size_t currentCoordinateIndex, const std::vector<PolygonPointFixStatus>& polygonPointFixStatusVector, float wantedBottomRowCoordinateY, float wantedTopRowCoordinateY, std::vector<float>& polygonsCoordinatesY_inOut)
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
// 2. Laske kunkin pisteen ja siitä seuraavaan pisteeseen olevan edgen pituus ja suhteuta se kokonais pituuteen.
static std::vector<float> calculatePolygonEdgeRelativeWidths(const std::vector<float>& polygonsCoordinatesX)
{
    std::vector<float> edgeWidths(polygonsCoordinatesX.size(), 0);
    auto polygonWidth = ::calculatePolygonsWidth(polygonsCoordinatesX);
    for(auto coordinateIndex = 0ull; coordinateIndex < polygonsCoordinatesX.size(); coordinateIndex++)
    {
        auto currentCoordinate = polygonsCoordinatesX[coordinateIndex];
        // Seuraava koordinaatti on joko coordinateIndex+1 -kohdasta tai jos ollaan jo viimeisessa pisteessä, vektorin 1. piste
        auto nextCoordinate = (coordinateIndex < (polygonsCoordinatesX.size() - 1)) ? polygonsCoordinatesX[coordinateIndex+1] : polygonsCoordinatesX[0];
        edgeWidths[coordinateIndex] = std::abs(currentCoordinate - nextCoordinate) / polygonWidth;
    }
    return edgeWidths;
}

// edgeWidth on etaisyys siihen viereiseen pisteeseen, joka on kauempana.
static PolygonPointFixStatus calculateMarginCaseFixStatus(float edgeWidth, CoordinateYStatus currentCoordinateYStatus, CoordinateYStatus neighborCoordinateYStatus)
{
    const float minimumEdgeWidthLength = 0.35f;
    if(currentCoordinateYStatus == CoordinateYStatus::BottomRowInToolmasterMarginCase)
    {
        if(neighborCoordinateYStatus == CoordinateYStatus::BottomRowValue && edgeWidth >= minimumEdgeWidthLength)
            return PolygonPointFixStatus::BottomFixNeeded;
    }
    else if(currentCoordinateYStatus == CoordinateYStatus::TopRowInToolmasterMarginCase)
    {
        if(neighborCoordinateYStatus == CoordinateYStatus::TopRowValue && edgeWidth >= minimumEdgeWidthLength)
            return PolygonPointFixStatus::TopFixNeeded;
    }

    return PolygonPointFixStatus::NoFixNeeded;
}

// Kutsutaan ainoastaan tapauksissa, missä tarkastelupisteen CoordinateYStatus on 
// BottomRowInToolmasterMarginCase tai TopRowInToolmasterMarginCase
static PolygonPointFixStatus calculateMarginCaseFixStatus(size_t pointIndex, const std::vector<CoordinateYStatus>& coordinateYStatusVector, const std::vector<float>& polygonEdgeRelativeWidths)
{
    auto currentCoordinateYStatus = coordinateYStatusVector[pointIndex];
    if(currentCoordinateYStatus == CoordinateYStatus::BottomRowInToolmasterMarginCase || currentCoordinateYStatus == CoordinateYStatus::TopRowInToolmasterMarginCase)
    {
        // Pituus nykypisteesta seuraavaan pisteeseen
        auto currentEdgeWidth = polygonEdgeRelativeWidths[pointIndex];
        // Pituus edellispisteesta nykypisteeseen
        auto previousEdgeWidth = ToolmasterHatchPolygonData::getPreviousValue(pointIndex, polygonEdgeRelativeWidths).first;
        auto nextCoordinateYStatus = ToolmasterHatchPolygonData::getNextValue(pointIndex, coordinateYStatusVector).first;
        auto previousCoordinateYStatus = ToolmasterHatchPolygonData::getPreviousValue(pointIndex, coordinateYStatusVector).first;
        if(currentEdgeWidth > previousEdgeWidth)
            return calculateMarginCaseFixStatus(currentEdgeWidth, currentCoordinateYStatus, nextCoordinateYStatus);
        else
            return calculateMarginCaseFixStatus(previousEdgeWidth, currentCoordinateYStatus, previousCoordinateYStatus);
    }

    return PolygonPointFixStatus::NoFixNeeded;
}

// Oletuksia:
// 1. On ainakin yksi BothFixesPossible tapaus.
// 2. Vain ala/ylariviin loytyy suoraan pisteita, joten tarkastelut tehdaan vain siihen yhteen riviin.
// Ehto: Jotta suurempia koordinaatti korjauksia tehtaisiin, pitaa olla seka tarpeeksi pitka edge ja 
// korjauksen vieressa pitaa olla tarkalleen yla/alariville osuva piste.
static void doFinalFixVectorForOneRowOnly(bool bottomRowCase, std::vector<PolygonPointFixStatus>& fixStatusVector_inOut, std::vector<CoordinateYStatus>& coordinateYStatusVector_inOut, const std::vector<float>& polygonEdgeRelativeWidths)
{
    for(auto pointIndex = 0ull; pointIndex < fixStatusVector_inOut.size(); pointIndex++)
    {
        if(fixStatusVector_inOut[pointIndex] == PolygonPointFixStatus::BothFixesPossible)
        {
            // Asetetaan coordinateYStatus osoittamaan siihen riviin, mihin se on enaa mahdollista tehda
            coordinateYStatusVector_inOut[pointIndex] = bottomRowCase ? CoordinateYStatus::BottomRowInToolmasterMarginCase : CoordinateYStatus::TopRowInToolmasterMarginCase;
            auto newFixStatus = ::calculateMarginCaseFixStatus(pointIndex, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths);
            fixStatusVector_inOut[pointIndex] = newFixStatus;
        }
    }
}

static void doFinalFixVectorForGeneralCase(std::vector<PolygonPointFixStatus>& fixStatusVector_inOut, std::vector<CoordinateYStatus>& coordinateYStatusVector_inOut, const std::vector<float>& polygonEdgeRelativeWidths)
{
    for(auto pointIndex = 0ull; pointIndex < fixStatusVector_inOut.size(); pointIndex++)
    {
        if(fixStatusVector_inOut[pointIndex] == PolygonPointFixStatus::BothFixesPossible)
        {
            // Pituus nykypisteesta seuraavaan pisteeseen
            auto currentEdgeWidth = polygonEdgeRelativeWidths[pointIndex];
            // Pituus edellispisteesta nykypisteeseen
            auto previousEdgeWidth = ToolmasterHatchPolygonData::getPreviousValue(pointIndex, polygonEdgeRelativeWidths).first;
            if(currentEdgeWidth > previousEdgeWidth)
            {
                auto nextCoordinateYStatus = ToolmasterHatchPolygonData::getNextValue(pointIndex, coordinateYStatusVector_inOut).first;
                if(nextCoordinateYStatus == CoordinateYStatus::BottomRowValue)
                {
                    coordinateYStatusVector_inOut[pointIndex] = CoordinateYStatus::BottomRowInToolmasterMarginCase;
                }
                else if(nextCoordinateYStatus == CoordinateYStatus::TopRowValue)
                {
                    coordinateYStatusVector_inOut[pointIndex] = CoordinateYStatus::TopRowInToolmasterMarginCase;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                auto previousCoordinateYStatus = ToolmasterHatchPolygonData::getPreviousValue(pointIndex, coordinateYStatusVector_inOut).first;
                if(previousCoordinateYStatus == CoordinateYStatus::BottomRowValue)
                {
                    coordinateYStatusVector_inOut[pointIndex] = CoordinateYStatus::BottomRowInToolmasterMarginCase;
                }
                else if(previousCoordinateYStatus == CoordinateYStatus::TopRowValue)
                {
                    coordinateYStatusVector_inOut[pointIndex] = CoordinateYStatus::TopRowInToolmasterMarginCase;
                }
                else
                {
                    continue;
                }
            }
            auto newFixStatus = ::calculateMarginCaseFixStatus(pointIndex, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths);
            fixStatusVector_inOut[pointIndex] = newFixStatus;
        }
    }
}

// Jos fixStatusVector_inOut:issa on BothFixesPossible arvoja, pitaa ne viela tarkastella erikseen:
// 1. Tarkista yhtään BothFixesPossible arvoa.
// 2. Jos löytyy laske kuinka monta uniikkia tarkkaa bottom-row pistetta on
// 3. Laske myös vastaava top-row pisteiden lukema
// 4. Jos jompi kumpi lukema on 0, tee tarkastelu toiseen riviin (pitaa tehda viela mm. edgen pituus tarkasteluja)
// 5. Jos jompi kumpi lukema on 1 ja toisessa on enemman kuin 1, tee tarkastelut sen rivin kanssa, jossa lukema oli 1
// 6. Jos molempien rivien lukemia enemman kuin 1, tee tarkastelut lahempaan reunaan
static void doFinalFixVectorChecks(std::vector<PolygonPointFixStatus>& fixStatusVector_inOut, std::vector<CoordinateYStatus>& coordinateYStatusVector_inOut, const std::vector<float>& polygonEdgeRelativeWidths, size_t samePointIndex)
{
    auto bothFixesPossibleCount = std::count(fixStatusVector_inOut.begin(), fixStatusVector_inOut.end(), PolygonPointFixStatus::BothFixesPossible);
    if(bothFixesPossibleCount)
    {
        auto samePointCoordinateYStatus = coordinateYStatusVector_inOut[samePointIndex];
        auto bottomRowCount = std::count(coordinateYStatusVector_inOut.begin(), coordinateYStatusVector_inOut.end(), CoordinateYStatus::BottomRowValue);
        if(samePointCoordinateYStatus == CoordinateYStatus::BottomRowValue)
            bottomRowCount--;
        auto topRowCount = std::count(coordinateYStatusVector_inOut.begin(), coordinateYStatusVector_inOut.end(), CoordinateYStatus::TopRowValue);
        if(samePointCoordinateYStatus == CoordinateYStatus::TopRowValue)
            topRowCount--;
        if(bottomRowCount == 0 && topRowCount == 0)
            return;
        if(bottomRowCount == 0)
            doFinalFixVectorForOneRowOnly(false, fixStatusVector_inOut, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths);
        else if(topRowCount == 0)
            doFinalFixVectorForOneRowOnly(true, fixStatusVector_inOut, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths);
        else
        {
            if(bottomRowCount == 1 && topRowCount > 1)
                doFinalFixVectorForOneRowOnly(true, fixStatusVector_inOut, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths);
            else if(topRowCount == 1 && bottomRowCount > 1)
                doFinalFixVectorForOneRowOnly(false, fixStatusVector_inOut, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths);
            else
            {
                doFinalFixVectorForGeneralCase(fixStatusVector_inOut, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths);
            }
        }
    }
}

static std::vector<PolygonPointFixStatus> calculatePolygonPointFixStatusVector(std::vector<CoordinateYStatus> & coordinateYStatusVector_inOut, const std::vector<float>& polygonEdgeRelativeWidths, size_t samePointIndex)
{
    std::vector<PolygonPointFixStatus> fixStatusVector;
    for(auto pointIndex = 0ull; pointIndex < coordinateYStatusVector_inOut.size(); pointIndex++)
    {
        auto coordinateYStatus = coordinateYStatusVector_inOut[pointIndex];
        auto edgeRelativeWidth = polygonEdgeRelativeWidths[pointIndex];
        switch(coordinateYStatus)
        {
        case CoordinateYStatus::BottomRowValue:
            fixStatusVector.push_back(PolygonPointFixStatus::AccuracyBottomFixNeeded);
            break;
        case CoordinateYStatus::TopRowValue:
            fixStatusVector.push_back(PolygonPointFixStatus::AccuracyTopFixNeeded);
            break;
        case CoordinateYStatus::BothRowsInToolmasterMarginCase:
            fixStatusVector.push_back(PolygonPointFixStatus::BothFixesPossible);
            break;
        case CoordinateYStatus::ClearMiddleValue:
            fixStatusVector.push_back(PolygonPointFixStatus::NoFixNeeded);
            break;
        case CoordinateYStatus::BottomRowInToolmasterMarginCase:
        case CoordinateYStatus::TopRowInToolmasterMarginCase:
            fixStatusVector.push_back(::calculateMarginCaseFixStatus(pointIndex, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths));
            break;
        default:
            fixStatusVector.push_back(PolygonPointFixStatus::NoFixNeeded);
            break;
        }
    }

    ::doFinalFixVectorChecks(fixStatusVector, coordinateYStatusVector_inOut, polygonEdgeRelativeWidths, samePointIndex);

    return fixStatusVector;
}

static size_t calculateSamePointIndex(const std::vector<float>& polygonsCoordinatesX, const std::vector<float>& polygonsCoordinatesY)
{
    for(auto pointIndex = 0ull; pointIndex < polygonsCoordinatesX.size(); pointIndex++)
    {
        if(pointIndex < polygonsCoordinatesX.size() - 1)
        {
            if(ToolmasterHatchPolygonData::areTwoPointsExcatlySame(pointIndex, pointIndex + 1, polygonsCoordinatesX, polygonsCoordinatesY))
                return pointIndex;
        }
        else
        {
            // kurkataan lopun yli alkuun eli 0-indeksiin
            if(ToolmasterHatchPolygonData::areTwoPointsExcatlySame(pointIndex, 0, polygonsCoordinatesX, polygonsCoordinatesY))
                return 0;
        }
    }
    return 0; // en tieda mita pitaisi palauttaa jos ei loydy samaa, aina pitaisi olla kaksi samaa pistetta
}

bool ToolmasterHatchPolygonData::doYPointCoordinateFixes(std::vector<float>& polygonsCoordinatesY_inOut, const std::vector<float>& polygonsCoordinatesX, float bottomRowCoordinateY, float topRowCoordinateY)
{
    bool hasAnyCorrectionsBeenMade = false;

    auto yCoordinateStatusVector = calculateCoordinateYStatusVector(polygonsCoordinatesY_inOut, bottomRowCoordinateY, topRowCoordinateY);
    auto edgeRelativeWidths = ::calculatePolygonEdgeRelativeWidths(polygonsCoordinatesX);
    auto samePointIndex = ::calculateSamePointIndex(polygonsCoordinatesX, polygonsCoordinatesY_inOut);
    auto polygonPointFixStatusVector = ::calculatePolygonPointFixStatusVector(yCoordinateStatusVector, edgeRelativeWidths, samePointIndex);
    for(size_t coordinateIndex = 0; coordinateIndex < yCoordinateStatusVector.size(); coordinateIndex++)
    {
        hasAnyCorrectionsBeenMade |= ::DoYCoordinateFix_ver2(coordinateIndex, polygonPointFixStatusVector, bottomRowCoordinateY, topRowCoordinateY, polygonsCoordinatesY_inOut);
    }

    return hasAnyCorrectionsBeenMade;
}


// Toolmasterin laskemissa polygoneissa on joskus pieni vertikaalisuunnassa tapahtuva vika. 
// Tämä funktio tekee seuraavaa:
// 1. Tutkii jokaisen polygonin pisteen ja pohjarivin läheisyyden (normi epsilon tai isompi erikseen laskettu Toolmaster epsilon)
// 2. Jos on tälläisiä pisteitä, asetetaan niiden y-koordinaatti suoraan pohjarivin tasolle.
// 3. Sama toiminto myös yläriville eli sen lähellä olevat arvot pakotetaan väkisin ylärivin koordinaatteihin.
// Funktio palauttaa true:n jossa on tehty yhtään korjauksia, muuten palautetaan vain false.
//bool ToolmasterHatchPolygonData::doYPointCoordinateFixes(std::vector<float>& polygonsCoordinatesY_inOut, const std::vector<float>& polygonsCoordinatesX, float bottomRowCoordinateY, float topRowCoordinateY)
//{
//    bool hasAnyCorrectionsBeenMade = false;
//
//    auto yCoordinateStatusVector = calculateCoordinateYStatusVector(polygonsCoordinatesY_inOut, bottomRowCoordinateY, topRowCoordinateY);
//    for(size_t coordinateIndex = 0; coordinateIndex < yCoordinateStatusVector.size(); coordinateIndex++)
//    {
//        hasAnyCorrectionsBeenMade |= ::DoYCoordinateFix(coordinateIndex, yCoordinateStatusVector, bottomRowCoordinateY, topRowCoordinateY, polygonsCoordinatesY_inOut);
//    }
//
//    return hasAnyCorrectionsBeenMade;
//}

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
                // Varmistetaan että luvut menevat rangeen nousevassa järjestyksessä
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
// yritetään korjata kaikkien polygonien y-koordinaatteja tarpeen mukaan.
void ToolmasterHatchPolygonData::doCoordinateYFixes()
{
    size_t currentPolygonCoordinateCounter = 0;
    size_t currentIntDataCounter = 0;
    for(size_t polygonIndex = 0; polygonIndex < polygonSizeNumbers_.size(); polygonIndex++)
    {
        int polygonCoordinateSize = polygonSizeNumbers_[polygonIndex];
        int intDataSize = polygonDataIntNumberArray_[polygonIndex];
        // +1:llä saadaan rivi koordinaatti
        size_t polygonsToolmasterGridRowIndex = polygonDataIntArray_[currentIntDataCounter + 1];
        // Toolmaster rivit alkavat 1:stä, joten hakuindeksiä pitää vähentää 1:llä.
        float polygonsBottomCoordinateY = gridRowCoordinateY_[polygonsToolmasterGridRowIndex - 1];
        // Toolmaster rivit alkavat 1:stä, joten toprivin hakuindeksiä bottomRowIndex + 1,
        // mutta jätetään viimeinen rivi väliin
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
        // Otetaan tähän polygoniin liittyvä Toolmaster rivinumero (int-array sisältää polygonin hilapisteen sarake-rivi -pareja).
        // Polygonin int-datan jokainen hilapiste viittaa aina vain yhteen ja ainoaan hilariviin (polygonin alla olevaan riviin).
        int polygonsToolmasterGridRowIndex = polygonDataIntArray_[currentIntDataCounter + 1]; // +1:llä saadaan rivi koordinaatti
        if(currentToolmasterGridRowIndex != polygonsToolmasterGridRowIndex)
        {
            if(currentToolmasterGridRowIndex >= 0)
            {
                // Otetaan käsitelty rivin y-koordinaatti talteen
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
    // Otetaan vielä viimeisinkin käsitelty rivi (y-koordinaatti) talteen
    gridRowCoordinateY_.push_back(static_cast<float>(minCoodinateY.FloatValue()));
}

// Tutki jokaisen polygonin suhde alariviin, onko kosketus ala- vai yläosaan, jos kosketus molempiin => kosketus alaosaan = true.
// Tehdään myös toinen työ eli tutkitaan miten polygoni liittyy pohjariviin 'fyysisesti'.
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
        size_t polygonsToolmasterGridRowIndex = polygonDataIntArray_[currentIntDataCounter + 1]; // +1:llä saadaan rivi koordinaatti
        float polygonsBottomCoordinateY = gridRowCoordinateY_[polygonsToolmasterGridRowIndex - 1]; // Toolmaster rivit alkavat 1:stä, joten hakuindeksiä pitää vähentää 1:llä.
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

// Kokeellisesti haettu kolme pistettä, jossa hatch laskut toimivat hyvin kolmelle eri kokoiselle karttaruudulle.
// 1. karttaruudun korkeus 300 mm => toolmaster-epsilon = 0.00135
// 2. karttaruudun korkeus 166 mm => toolmaster-epsilon = 0.00215
// 3. karttaruudun korkeus  83 mm => toolmaster-epsilon = 0.00385
// Näiden pisteiden avulla laskettu 2. asteen yhtälö, jonka avulla voidaan laskea sopiva kerroin jokaiselle 
// karttanäytön korkeudelle.
// Koska en ole voinut testata 300 mm isommilla näytöillä, ja oletettavasti kerroin isommilla koilla pysyy pienenä tai samana,
// niin laitetaan ehto että jos x >= 300, niin käytetään usedEpsilon:ina aina 0.00135.
// Laskentaan lisätty vielä kerroin ~1, jolla voidaan manipuloida käytettyä kerrointa pinemmäksi tai isommaksi,
// kyseistä kerrointa voidaan säätää SmartMetin Settings dialogista.
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
