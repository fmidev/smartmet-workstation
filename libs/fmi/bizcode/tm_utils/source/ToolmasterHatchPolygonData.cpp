#include "ToolmasterHatchPolygonData.h"
#include "CtrlViewFunctions.h"
#include "NFmiDataModifierMin.h"
#include "PolygonRelationsToBottomGridRowCalculator.h"

#ifndef DISABLE_UNIRAS_TOOLMASTER

#include <agX/agxfont.h>
#include <agX/agx.h>


float ToolmasterHatchPolygonData::normallyUsedCoordinateEpsilon_ = std::numeric_limits<float>::epsilon() * 4;
float ToolmasterHatchPolygonData::toolmasterRelatedBigEpsilonFactor_ = 1.f;
int ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex1_ = 10;
int ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex2_ = 12;

ToolmasterHatchPolygonData::ToolmasterHatchPolygonData(NFmiIsoLineData& theIsoLineData, const NFmiHatchingSettings& theHatchSettings)
    :hatchSettings_(theHatchSettings),
    hatchClassValues_(2, 0)
{
    usedToolmasterRelatedBigEpsilon_ = calculateUsedToolmasterEpsilon(static_cast<float>(theIsoLineData.itsSingleSubMapViewHeightInMillimeters));
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

CoordinateYStatus ToolmasterHatchPolygonData::calculateCoordinateYStatus(float value, float bottomRowCoordinateY)
{
    if(CtrlViewUtils::IsEqualEnough(value, bottomRowCoordinateY, normallyUsedCoordinateEpsilon_))
        return CoordinateYStatus::BottomRowValue;
    if(CtrlViewUtils::IsEqualEnough(value, bottomRowCoordinateY, usedToolmasterRelatedBigEpsilon_))
        return CoordinateYStatus::BottomRowInToolmasterMarginCase;

    return CoordinateYStatus::NotBottomRowValue;
}

std::vector<CoordinateYStatus> ToolmasterHatchPolygonData::calculateCoordinateYStatusVector(const std::vector<float>& polygonsCoordinatesY, float bottomRowCoordinateY)
{
    std::vector<CoordinateYStatus> statusVector;
    for(auto coordinateY : polygonsCoordinatesY)
    {
        statusVector.push_back(calculateCoordinateYStatus(coordinateY, bottomRowCoordinateY));
    }
    return statusVector;
}

bool ToolmasterHatchPolygonData::areTwoPointsExcatlySame(size_t pointIndex1, size_t pointIndex2, const std::vector<float>& polygonsCoordinatesX, const std::vector<float>& polygonsCoordinatesY)
{
    auto xCoordinatesAreSame = polygonsCoordinatesX[pointIndex1] == polygonsCoordinatesX[pointIndex2];
    auto yCoordinatesAreSame = polygonsCoordinatesY[pointIndex1] == polygonsCoordinatesY[pointIndex2];
    return xCoordinatesAreSame && yCoordinatesAreSame;
}

// Toolmasterin laskemissa polygoneissa on joskus pieni vertikaalisuunnassa tapahtuva vika. 
// Tämä funktio tekee seuraavaa:
// 1. Tutkii jokaisen polygonin pisteen ja pohjarivin läheisyyden (normi epsilon tai isompi erikseen laskettu Toolmaster epsilon)
// 2. Jos on tälläisiä pisteitä, asetetaan niiden y-koordinaatti suoraan pohjarivin tasolle.
// Funktio palauttaa vektorin jossa on tehty korjaukset, jos on tehty mitään korjauksia, muuten palautetaan vain tyhjä vektori.
std::vector<float> ToolmasterHatchPolygonData::doYPointCoordinateFixes(const std::vector<float>& polygonsCoordinatesY, const std::vector<float>& polygonsCoordinatesX, float bottomRowCoordinateY)
{
    auto yCoordinateStatusVector = calculateCoordinateYStatusVector(polygonsCoordinatesY, bottomRowCoordinateY);
    auto correctedCoordinates = polygonsCoordinatesY;
    bool hasAnyCorrectionsBeenMade = false;
    for(size_t coordinateIndex = 0; coordinateIndex < yCoordinateStatusVector.size(); coordinateIndex++)
    {
        auto currentStatus = yCoordinateStatusVector[coordinateIndex];
        if(currentStatus == CoordinateYStatus::BottomRowValue || currentStatus == CoordinateYStatus::BottomRowInToolmasterMarginCase)
        {
            // Asetetaan ainakin pohjariviin liitetty y-koordinaatti tarkalleen pohjarivin
            if(correctedCoordinates[coordinateIndex] != bottomRowCoordinateY)
            {
                hasAnyCorrectionsBeenMade = true;
                correctedCoordinates[coordinateIndex] = bottomRowCoordinateY;
            }
        }
    }
    if(hasAnyCorrectionsBeenMade)
        return correctedCoordinates;
    else
        return std::vector<float>();
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
        size_t polygonsToolmasterGridRowIndex = polygonDataIntArray_[currentIntDataCounter + 1]; // +1:llä saadaan rivi koordinaatti
        float polygonsBottomCoordinateY = gridRowCoordinateY_[polygonsToolmasterGridRowIndex - 1]; // Toolmaster rivit alkavat 1:stä, joten hakuindeksiä pitää vähentää 1:llä.

        auto startIterX = polygonCoordinateX_.begin() + currentPolygonCoordinateCounter;
        std::vector<float> currentPolygonsCoordinatesX(startIterX, startIterX + polygonCoordinateSize);
        auto startIterY = polygonCoordinateY_.begin() + currentPolygonCoordinateCounter;
        std::vector<float> currentPolygonsCoordinatesY(startIterY, startIterY + polygonCoordinateSize);
        auto correctedCoordinateYVector = doYPointCoordinateFixes(currentPolygonsCoordinatesY, currentPolygonsCoordinatesX, polygonsBottomCoordinateY);
        if(!correctedCoordinateYVector.empty())
        {
            std::copy(correctedCoordinateYVector.begin(), correctedCoordinateYVector.end(), startIterY);
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
// 1. karttaruudun korkeus 23.8 cm => toolmaster-epsilon = 0.002
// 2. karttaruudun korkeus 13.5 cm => toolmaster-epsilon = 0.005
// 3. karttaruudun korkeus 7.9 cm => toolmaster-epsilon = 0.007
// Näiden pisteiden avulla laskettu 2. asteen yhtälö, jonka avulla voidaan laskea sopiva kerroin jokaiselle 
// karttanäytön korkeudelle.
// Laskentaan lisätty vielä kerroin ~1, jolla voidaan manipuloida käytettyä kerrointa pinemmäksi tai isommaksi,
// kyseistä kerrointa voidaan säätää SmartMetin Settings dialogista.
float ToolmasterHatchPolygonData::calculateUsedToolmasterEpsilon(float singleMapSubViewHeightInMillimeters)
{
    // Kaava on siis laskettu cm korkeuksien kautta, joten parametrina satu korkeus (kaavassa helppouden vuoksi x) pitää muuttaa mm -> cm
    float x = singleMapSubViewHeightInMillimeters / 10.f;
    float usedEpsilon = 0.0102633266165965f - 0.000445812507087464f * x + 0.000004143441586197f * x * x;
    return usedEpsilon * toolmasterRelatedBigEpsilonFactor_;
}

#endif // DISABLE_UNIRAS_TOOLMASTER
