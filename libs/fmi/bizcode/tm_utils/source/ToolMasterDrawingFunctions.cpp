#include "stdafx.h"

#ifndef DISABLE_UNIRAS_TOOLMASTER

#include <agX/agxfont.h>
#include <agX/agx.h>

#include <algorithm>
#include "ToolMasterDrawingFunctions.h"
#include "NFmiFastQueryInfo.h" // tämän pitää olla ennen NFmiIsoLineData.h includea!
#include "NFmiIsoLineData.h"
#include "NFmiRect.h"
#include "NFmiColor.h"
#include "matrix3d.h"
#include "catlog/catlog.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "NFmiDrawParam.h"
#include "NFmiDataModifierAvg.h"
#include "CtrlViewFunctions.h"
#include "ToolmasterHatchingUtils.h"
#include "ToolmasterHatchPolygonData.h"
#include "ToolMasterColorCube.h"
#include "NFmiVisualizationSpaceoutSettings.h"

#include <fstream>
#include "boost/math/special_functions/round.hpp"

// Win32 makrot sähläävät std-min ja max:ien käytön, ne pitää 'undefinoida'
#ifdef min 
#undef min
#undef max
#endif

float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor)
{
    float factor = 1.f - (0.65f * (180.f - theViewHeightInMM) / 180.f);
    factor = FmiMin(factor, theMaxFactor);
    return factor;
}

using namespace std;

static std::string MakeDataIdentString(const NFmiDataIdent &dataIdent)
{
    std::string str = "par: ";
    str += dataIdent.GetParamName();
    str += " (";
    str += std::to_string(dataIdent.GetParamIdent());
    str += ") prod: ";
    str += dataIdent.GetProducer()->GetName();
    str += " (";
    str += std::to_string(dataIdent.GetProducer()->GetIdent());
    str += ")";
    return str;
}

// täyttää gridnode datan (kutsutaan kun zoomataan)
// oletus alue on yksikkö laatikon sisällä (0,0, 1,1)
static void FillGridNodeData(NFmiIsoLineData &theIsoLineData, std::vector<float>& theGridNodesX, std::vector<float>& theGridNodesY, const NFmiRect &theGridArea)
{
    // Tämän pitää saada ottamaan huomioon myös partial hila-rect

    theGridNodesX.resize(theIsoLineData.itsXNumber);
    theGridNodesY.resize(theIsoLineData.itsYNumber);

    int i = 0;
    for(i = 0; i < theIsoLineData.itsXNumber; i++)
        theGridNodesX[i] = static_cast<float>(theGridArea.Left() + ((theGridArea.Width() * i) / (theIsoLineData.itsXNumber - 1)));
    for(i = 0; i < theIsoLineData.itsYNumber; i++)
        theGridNodesY[i] = static_cast<float>(theGridArea.Top() + ((theGridArea.Height() * i) / (theIsoLineData.itsYNumber - 1)));
}

// Asettaa Toolmasterin viewworldin ja workboxit ja muut systeemit piirtoa varten.
// Palauttaa parametreina clippaus rect:in ja näytön korkeuden.
// Asettaa toolmasterin käyttämät gridnode:t (missä kukin hilapiste on ToolMaster maailmassa)
// Parametri theRelViewRect: Näkyvän kartta-alueen koko suhteellisessa 0,0 - 1,1 maailmassa. Tähän 
// vaikuttaa mm. karttaruudukko ja alareunan aikakontrolli-ikkuna.
// Parametri theZoomedViewRect: Näkyvän kartta-alueen koko suhteessa datasta käytetyn hilan alueeseen. Käytetyn hilan alue on aina
// 0,0 - 1,1. Jos datan alue on pienempi kuin kartan, on tämä isompi kuin datan alue ja toisin päin.
// Parametri theGridArea: tämä on aina 0,0 - 1,1.
// Parametrit theGridNodesX/Y: Näihin vektoreihin lasketaan käytetyn hilan x- ja y-pisteiden paikat theGridArea:n maailmassa.
// Parametri theMfcClipRect: Tähän lasketaan piirtoalueen laatikko pikseleissä. Sitä käytetään ei ToolMaster piirroissa.
// Parametri theTotViewSizeOut: Tähän lasketaan piirtoalueen koko pikseleissä, tietoa käytetään isoviiva labeloinnin harvennukseen.
void SetupViewWorld(NFmiIsoLineData &theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiRect &theGridArea, std::vector<float>& theGridNodesX, std::vector<float>& theGridNodesY, CRect* theMfcClipRect, NFmiPoint &theTotViewSizeOut, double & dataGridToViewHeightRatioOut)
{
    float xsi, ysi; // koko CWnd ikkunan piirtoalueen koko [mm]
    int   xpic, ypic; // koko CWnd ikkunan piirtoalueen koko pikseleissä
    XuDrawingAreaQuery(&xsi, &ysi, &xpic, &ypic);
    double rl = theRelViewRect.Left();
    double rb = theRelViewRect.Bottom();
    double rw = theRelViewRect.Width();
    double rh = theRelViewRect.Height();
    double rr = theRelViewRect.Right();
    double rt = theRelViewRect.Top();

    double xsize = xsi * rw;
    double ysize = ysi * rh;
    double xoff = xsi * rl;
    double yoff = ysi * (1 - rb); // taas käännetään y-akselin suhteen!!!!

    theTotViewSizeOut.X(xsi * theRelViewRect.Size().X());
    theTotViewSizeOut.Y(ysi * theRelViewRect.Size().Y());

    // theGridArea on aina 0,0 - 1,1, eli käytetty hila on vakio paikassa ja piirtosysteemiä siirretään sen mukaan.
    ::FillGridNodeData(theIsoLineData, theGridNodesX, theGridNodesY, theGridArea);
    double zl = theZoomedViewRect.Left();
    double zr = theZoomedViewRect.Right();
    double zt = theZoomedViewRect.Top();
    double zb = theZoomedViewRect.Bottom();

    double zxmin = zl;
    double zxmax = zr;
    double zymin = 1. - zb;
    double zymax = 1. - zt;
    dataGridToViewHeightRatioOut = 1. / (zymax - zymin);
    // Asetetaan kartalla näkyvän alueen koko ja paikka suhteellisessa maailmassa
    XuViewWorldLimits(zxmin, zxmax, zymin, zymax, 0, 0);
    // Asetetaan eri akseleiden suhteita, tärkein on kai tuo x/y suhdeluku
    XuViewWorkbox(xsize / ysize, 1, 0);
    // Asetetaan näkyvän maailman koko ja paikka pikseleissa näyttöikkunassa
    XuViewport(xoff, yoff, xsize, ysize);

    int leftPix = static_cast<int>(rl * xpic);
    int topPix = static_cast<int>(rt * ypic);
    int rightPix = static_cast<int>(rr * xpic);
    int bottomPix = static_cast<int>(rb * ypic);

    // Toolmaster tekee omat clippaukset, mutta kun tehdään omia piirtojuttuja, käytetään tätä clip aluetta
    *theMfcClipRect = CRect(leftPix, topPix, rightPix, bottomPix);
}

static void DoContourUserDraw(NFmiIsoLineData& theIsoLineData)
{
    XuContourUserDraw(theIsoLineData.itsContourUserDrawData.xCoordinates.data(), theIsoLineData.itsContourUserDrawData.yCoordinates.data(), theIsoLineData.itsContourUserDrawData.itsUserDrawValues.data(), theIsoLineData.itsContourUserDrawData.itsYNumber, theIsoLineData.itsContourUserDrawData.itsXNumber);
}

static void BaseSetupColorContourDraw_new(NFmiIsoLineData& theIsoLineData)
{
    // Color-contour separation line setup:
    // ====================================
    // 1. Viivan paksuus joko 0, jos ei piirretä ja 0.1, jos piirretään.
    float isolineWidth = theIsoLineData.fUseSeparatorLinesBetweenColorContourClasses ? 0.1f : 0.f;
    XuIsolineWidths(&isolineWidth, 1);
    // 2. Viivan väri
    int isolineColorArr = 0; // 0 = windows background väri eli käytännössä musta
    XuIsolineColors(&isolineColorArr, 1);
    // 3. Viivan piirto tyyli
    int isolineStyle = theIsoLineData.itsColorContouringData.separationLineStyle();
    XuIsolineStyles(&isolineStyle, 1);

    XuIsolineSplineSmoothing(0); // ei kannata pyöristää isoviivoja koska contourit eivät kuitenkaan pyöristy
}

static std::string GetColorModeString(int color_mode)
{
    switch(color_mode)
    {
    case XuLOOKUP:
        return std::string("LOOKUP");
    case XuVIRTUAL:
        return std::string("VIRTUAL");
    case XuCOMBINED:
        return std::string("COMBINED");
    case XuDIRECT:
        return std::string("DIRECT");
    default:
        return std::string("Unknown color-mode");
    }
}

static std::string GetColorSchemeString(int color_scheme)
{
    switch(color_scheme)
    {
    case XuHLS:
        return std::string("HLS");
    case XuRGB:
        return std::string("RGB");
    case XuCMY:
        return std::string("CMY");
    case XuBWA:
        return std::string("B/W Additive");
    case XuBWS:
        return std::string("B/W Subtractive");
    default:
        return std::string("Unknown color-scheme");
    }
}

static std::string GetColorTypeString(int color_type)
{
    switch(color_type)
    {
    case XuOFF:
        return std::string("OFF");
    case XuBACKGROUND:
        return std::string("BACKGROUND");
    case XuANTIBACKGROUND:
        return std::string("ANTIBACKGROUND");
    case XuHOLLOW_COLOR:
        return std::string("HOLLOW_COLOR");
    default:
        return std::string("Unknown color-type");
    }
}

static std::string GetColorRGBString(float colorRGB[3])
{
    std::string rgbStr = std::to_string(colorRGB[0]);
    rgbStr += ",";
    rgbStr += std::to_string(colorRGB[1]);
    rgbStr += ",";
    rgbStr += std::to_string(colorRGB[2]);
    return rgbStr;
}

static void PrintColorTableEntry(std::ostream& out, int colorIndex)
{
    int color_type = 0;
    XuColorTypeQuery(colorIndex, &color_type);
    float colorRGB[3], dummy[5];
    XuColorQuery(colorIndex, colorRGB, dummy);
    out << "Color " << colorIndex << ": type = " << ::GetColorTypeString(color_type);
    out << ", RGB = " << GetColorRGBString(colorRGB) << std::endl;
}

static void PrintHollowColorInfo(std::ostream& out)
{
    float value = 0;
    int color_index = 0;
    XuUndefinedQuery(&value, &color_index);
    out << "Undefined/hollow color: value = " << value << ", index = " << color_index << std::endl;
}

static void PrintSelectedColorTable(std::ostream& out)
{
    int color_table_id = 0;
    XuColorTableActiveQuery(&color_table_id);
    out << "Active color-table: " << color_table_id << std::endl;
    int num_vir = 0;
    int color_mode = 0;
    int color_scheme = 0;
    float max_level = 0;
    XuColorTableQuery(&color_table_id, &num_vir, &color_mode, &color_scheme, &max_level);
    out << "Number of color-table entries: " << num_vir << std::endl;
    out << "Color-mode: " << color_mode << " (" << ::GetColorModeString(color_mode) << ")" << std::endl;
    out << "Color-scheme: " << color_scheme << " (" << ::GetColorSchemeString(color_scheme) << ")" << std::endl;
    out << "Max-level of color-code: " << max_level << std::endl;
    ::PrintHollowColorInfo(out);

    for(int colorIndex = 0; colorIndex < num_vir ; colorIndex++)
    {
        ::PrintColorTableEntry(out, colorIndex);
    }
}

template<typename T>
static void PrintContainerWithCommaSeparation(std::ostream& out, const T& container)
{
    for(size_t index = 0; index < container.size(); index++)
    {
        if(index > 0)
        {
            out << ", ";
        }
        out << container[index];
    }
}

static void PrintContourClassInfo(std::ostream& out, NFmiIsoLineData& theIsoLineData)
{
    int num_limits = 0;
    XuClassesNumberQuery(&num_limits);
    std::vector<float> limit_array(num_limits, 0.f);
    XuClassesQuery(limit_array.data(), &num_limits);
    out << "Limit classes count = " << num_limits << std::endl;
    out << "Limit classes values: ";
    ::PrintContainerWithCommaSeparation(out, limit_array);
    out << std::endl;

    out << "Color indexies: ";
    ::PrintContainerWithCommaSeparation(out, theIsoLineData.itsColorContouringData.finalColorIndexies());
    out << std::endl;
}

static void PrintContourGridData(std::ostream& out, NFmiIsoLineData& theIsoLineData)
{
    const auto& dataMatrix = theIsoLineData.itsIsolineData;
    auto xSize = dataMatrix.NX();
    auto ySize = dataMatrix.NY();
    out << "Data grid: xSize = " << xSize << ", ySize = " << ySize << std::endl;
    out.precision(12);
    for(size_t yIndex = 0; yIndex < ySize; yIndex++)
    {
        for(size_t xIndex = 0; xIndex < xSize; xIndex++)
        {
            if(xIndex > 0)
                out << ", ";
            out << dataMatrix[xIndex][yIndex];
        }
        out << std::endl;
    }
    out << std::endl;
}

// Laita tämän flagin arvo debuggerissa true:ksi, kun haluat tulostaa \ToolMasterContourSetups.txt 
// -tiedostoon diagnostiikka dataa.
static bool g_DoPrintToolMasterRelatedSetups = false;

static void PrintToolMasterRelatedContourSetups(NFmiIsoLineData& theIsoLineData)
{
    if(g_DoPrintToolMasterRelatedSetups)
    {
        std::ofstream out("\\ToolMasterContourSetups.txt", std::ios::binary);
        if(out)
        {
            ::PrintSelectedColorTable(out);
            ::PrintContourClassInfo(out, theIsoLineData);
            ::PrintContourGridData(out, theIsoLineData);
        }
    }
}

static void DoActualColorContourDraw_new(NFmiIsoLineData& theIsoLineData)
{
    ::PrintToolMasterRelatedContourSetups(theIsoLineData);

    if(theIsoLineData.UseContourUserDraw())
        ::DoContourUserDraw(theIsoLineData);
    else if(theIsoLineData.fUseColorContours == 2) // 2=quickcontours
        XuContourQuickDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
    else
        XuContourDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
}

static void DrawSimpleColorContours_new(NFmiIsoLineData& theIsoLineData)
{
    BaseSetupColorContourDraw_new(theIsoLineData);
    auto& colorContouringData = theIsoLineData.itsColorContouringData;

    if(colorContouringData.useDefaultColorTable())
    {
        XuColorTableActivate(ToolMasterColorCube::UsedDefaultColorTableIndex());
        auto& fixedClassLimits = colorContouringData.finalToolmasterFixedClassLimits();
        XuClasses(fixedClassLimits.data(), static_cast<int>(fixedClassLimits.size()));
        auto& colorIndexies = colorContouringData.finalColorIndexies();
        XuShadingColorIndices(colorIndexies.data(), static_cast<int>(colorIndexies.size()));
    }
    else
    {
        // Tehdään uudesta color-tablen indeksistä käytetyn default taulun indeksi + 1
        if(!colorContouringData.createNewToolMasterColorTable(ToolMasterColorCube::UsedDefaultColorTableIndex() + 1))
            return; // Ei tehdä piirtoa, jos color-tablen alustus meni jotenkin pieleen...
    }
    DoActualColorContourDraw_new(theIsoLineData);
}

static void DrawCustomColorContours(NFmiIsoLineData &theIsoLineData)
{
    DrawSimpleColorContours_new(theIsoLineData);
}

static void DrawSimpleColorContours(NFmiIsoLineData &theIsoLineData)
{
    DrawSimpleColorContours_new(theIsoLineData);
}

static void FixWorldLimitsWithViewPortSettings(TMWorldLimits &theWorldLimits)
{
    theWorldLimits.x_min = 0;
    theWorldLimits.x_max = 1;
    theWorldLimits.y_min = 0;
    theWorldLimits.y_max = 1;
}

static void MakeTotalPolygonPointConversion(ToolmasterHatchPolygonData &theToolmasterHatchPolygonData, std::vector<int>& thePolyPointsXInPixels, std::vector<int>& thePolyPointsYInPixels)
{
    size_t totalPolyPointSize = theToolmasterHatchPolygonData.polygonCoordinateX_.size();
    thePolyPointsXInPixels.resize(totalPolyPointSize);
    thePolyPointsYInPixels.resize(totalPolyPointSize);
    XuViewWorldToPixel(theToolmasterHatchPolygonData.polygonCoordinateX_.data(), theToolmasterHatchPolygonData.polygonCoordinateY_.data(), static_cast<int>(totalPolyPointSize), &thePolyPointsXInPixels[0], &thePolyPointsYInPixels[0]);
}

static void FillHatchedPolygonData(const std::vector<int> &polyPointsXInPixels, const std::vector<int>& polyPointsYInPixels, int theTotalPolyPointCounter, int thePolyPointCount, std::vector<CPoint> & thePolygonCPoints)
{
    thePolygonCPoints.resize(thePolyPointCount);
    for(int i = 0; i < thePolyPointCount; i++)
    {
        thePolygonCPoints[i].x = polyPointsXInPixels[theTotalPolyPointCounter + i];
        thePolygonCPoints[i].y = polyPointsYInPixels[theTotalPolyPointCounter + i];
    }
}

static void DrawPolygonIndexText(int polygonIndex, int wantedPolygonIndex, CDC *pDC, int xPixelCoordinate, int yPixelCoordinate)
{
    if(polygonIndex == wantedPolygonIndex)
    {
        CString polygonIndexText;
        polygonIndexText.Format(_T("Ind: %d"), wantedPolygonIndex);
        pDC->TextOut(xPixelCoordinate, yPixelCoordinate, polygonIndexText);
    }
}

static void UpdateMinCoordinateY(const std::vector<CPoint> &polygonCPoints, int &currentRowMinCoordinateY_inOut)
{
    auto minElement = std::min_element(polygonCPoints.begin(), polygonCPoints.end(),
        [](const auto& point1, const auto& point2) {return point1.y < point2.y; });
    if(currentRowMinCoordinateY_inOut > minElement->y)
        currentRowMinCoordinateY_inOut = minElement->y;
}

// Pieni overlap fiksaus polygoneihin. En tiedä johtuuko ongelma toolmasterin polygoneista vai
// GDI piirron ominaisuuksista, mutta tämä korjaa yhden pikselin satunnaiset välit hatcheistä.
// Eli lasketaan koko ajan, mikä oli edellisen polygoni rivin ylimmän pikselin arvo (monitorilla ylin
// on siis pienin arvo, kun 0,0 piste on näytön top-left). Jos läpikäytävä piste on juuri siinä, siirretään 
// se yhden pikselin verran alaspäin (overlap).
static void FixOnePixelGapFromPolygon(int lastRowMinCoordinateY, std::vector<CPoint>& polygonCPoints_inOut)
{
    for(auto& point : polygonCPoints_inOut)
    {
        if(lastRowMinCoordinateY == point.y)
            point.y++;
    }
}

static void DrawShadedPolygons4(ToolmasterHatchPolygonData& theToolmasterHatchPolygonData, CDC *pDC, const CRect& theMfcClipRect)
{
    if(theToolmasterHatchPolygonData.polygonSizeNumbers_.size())
    {
        TMWorldLimits worldLimits;
        XuViewWorldLimitsQuery(&worldLimits.x_min, &worldLimits.x_max, &worldLimits.y_min, &worldLimits.y_max, &worldLimits.z_min, &worldLimits.z_max);
        float xoff, yoff, xsize, ysize;
        XuViewportQuery(&xoff, &yoff, &xsize, &ysize);
        ::FixWorldLimitsWithViewPortSettings(worldLimits);
        theToolmasterHatchPolygonData.setWorldLimits(worldLimits);

        std::vector<int> polyPointsXInPixels, polyPointsYInPixels;
        std::vector<CPoint> polygonCPoints;
        MakeTotalPolygonPointConversion(theToolmasterHatchPolygonData, polyPointsXInPixels, polyPointsYInPixels);
        int polygonPointTotalCount = 0;
        int currentPolygonFloatDataTotalIndex = 0;
        int currentPolygonIntDataTotalIndex = 0;
        int lastRowMinCoordinateY = std::numeric_limits<int>::max();
        int currentRowMinCoordinateY = std::numeric_limits<int>::max();
        int currentRowToolmasterIndexY = -1;
        const auto& polygonSizeNumbers = theToolmasterHatchPolygonData.polygonSizeNumbers_;
        for(int polygonIndex = 0; polygonIndex < polygonSizeNumbers.size(); polygonIndex++)
        {
            int polygonPointsCount = polygonSizeNumbers[polygonIndex];
            int polygonFloatDataCount = theToolmasterHatchPolygonData.polygonDataFloatNumberArray_[polygonIndex];
            int polygonIntDataCount = theToolmasterHatchPolygonData.polygonDataIntNumberArray_[polygonIndex];
            int polygonRowToolmasterIndexY = theToolmasterHatchPolygonData.polygonDataIntArray_[currentPolygonIntDataTotalIndex + 1];

            if(currentRowToolmasterIndexY != polygonRowToolmasterIndexY)
            {
                currentRowToolmasterIndexY = polygonRowToolmasterIndexY;
                lastRowMinCoordinateY = currentRowMinCoordinateY;
            }

            // **** HUOM! Pida seuraavia 5 koodi rivia tallessa kommenteissa, niiden ***********
            // **** avulla voidaan metsastaa ongelma polygoneja hatching yhteydessa  ***********
            // ---------------------------------------------------------------------------------
            //int wantedPolygonIndex1 = theToolmasterHatchPolygonData.debugHelperWantedPolygonIndex1_;
            //int wantedPolygonIndex2 = theToolmasterHatchPolygonData.debugHelperWantedPolygonIndex2_;
            //if(polygonIndex == wantedPolygonIndex1 || polygonIndex == wantedPolygonIndex2)
            {
                //::DrawPolygonIndexText(polygonIndex, wantedPolygonIndex1, pDC, polyPointsXInPixels[polygonPointTotalCount], polyPointsYInPixels[polygonPointTotalCount]);
                //::DrawPolygonIndexText(polygonIndex, wantedPolygonIndex2, pDC, polyPointsXInPixels[polygonPointTotalCount], polyPointsYInPixels[polygonPointTotalCount]);
                if(theToolmasterHatchPolygonData.isHatchPolygonDrawn(polygonIndex, currentPolygonFloatDataTotalIndex, currentPolygonIntDataTotalIndex, polygonPointTotalCount))
                {
                    FillHatchedPolygonData(polyPointsXInPixels, polyPointsYInPixels, polygonPointTotalCount, polygonPointsCount, polygonCPoints);
                    ::UpdateMinCoordinateY(polygonCPoints, currentRowMinCoordinateY);
                    ::FixOnePixelGapFromPolygon(lastRowMinCoordinateY, polygonCPoints);

                    pDC->Polygon(polygonCPoints.data(), polygonPointsCount);
                }
            }
            polygonPointTotalCount += polygonPointsCount;
            currentPolygonFloatDataTotalIndex += polygonFloatDataCount;
            currentPolygonIntDataTotalIndex += polygonIntDataCount;
        }
    }
}

static void SetupMFCAndDrawShadedPolygons3(ToolmasterHatchPolygonData &theToolmasterHatchPolygonData, CDC* pDC, const CRect& theMfcClipRect)
{
    int oldBkMode = pDC->SetBkMode(TRANSPARENT);
    CBrush brush;
    const auto& hatchSettings = theToolmasterHatchPolygonData.hatchSettings_;
    COLORREF crColor = hatchSettings.itsHatchColorRef;
    if(hatchSettings.itsHatchPattern == -1) // jos hatch-pattern on -1, tehdäänkin täysin peittävä sivellin ilman hatchiä
        brush.CreateSolidBrush(crColor);
    else
        brush.CreateHatchBrush(hatchSettings.itsHatchPattern, crColor);
    CBrush *oldBrush = pDC->SelectObject(&brush);

    int penStyle = PS_NULL; // PS_SOLID
    int penWidth = hatchSettings.fDrawHatchBorders ? 1 : 0;
    COLORREF penColor1 = 0x00000000;
    if(penWidth > 0)
        penColor1 = crColor;
    CPen myPen(penStyle, penWidth, penColor1);
    CPen *oldPen = pDC->SelectObject(&myPen);

    ::DrawShadedPolygons4(theToolmasterHatchPolygonData, pDC, theMfcClipRect);

    pDC->SelectObject(oldBrush);
    brush.DeleteObject();
    pDC->SetBkMode(oldBkMode);

    pDC->SelectObject(oldPen);
}

bool gDrawTest = false; // Laita tämän arvoksi true, jos haluat nähdä tietyn piirrettän polygonin high-lightauksen.

static void GetReadyAndDoTheHatch(NFmiIsoLineData &theIsoLineData, const NFmiHatchingSettings& theHatchSettings, CDC* pDC, const CRect& theMfcClipRect, int hatchIndex)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, std::string("Drawing hatch ") + std::to_string(hatchIndex) + " for " + ::MakeDataIdentString(theIsoLineData.itsParam));

    ToolmasterHatchPolygonData toolmasterHatchPolygonData(theIsoLineData, theHatchSettings);
    if(!toolmasterHatchPolygonData.continueHatchDraw_)
        return;

    CRgn rgn;
    CRect rectClip(theMfcClipRect);
    rgn.CreateRectRgn(rectClip.left, rectClip.top,
        rectClip.right, rectClip.bottom);
    pDC->SelectClipRgn(&rgn);

    try
    {
        ::SetupMFCAndDrawShadedPolygons3(toolmasterHatchPolygonData, pDC, theMfcClipRect);
    }
    catch(...)
    {
    }
    pDC->SelectClipRgn(NULL);
    rgn.DeleteObject();
    XuMapDrawOptions(XuON);
}

static void GetReadyAndDoTheHatch(NFmiIsoLineData &theIsoLineData, CDC* pDC, const CRect& theMfcClipRect)
{
    if(theIsoLineData.itsHatch1.fUseHatch)
        ::GetReadyAndDoTheHatch(theIsoLineData, theIsoLineData.itsHatch1, pDC, theMfcClipRect, 1);
    if(theIsoLineData.itsHatch2.fUseHatch)
        ::GetReadyAndDoTheHatch(theIsoLineData, theIsoLineData.itsHatch2, pDC, theMfcClipRect, 2);
}

// jos index on negatiivinen, palauttaa kertoimen -0.9, jolloin iso viivaan laitetaan
// yksi. label miel. keskelle sitä.
// jos tulee isoviiva indeksi arvoja (index>0), annetaan joka toiselle arvo -0.9 ja
// joka toiselle 0.5. Tällöin joka toinen isoviiva piirretään yhdellä labelilla (keskelle)
// ja joka toiselle kahdelle 1/3 ja 2/3 kohtiin iso viivaa. Tällä yritetään välttää
// eri parametreja päällekkäin piirrettäessä piirtää labelit eri kohtiin kun ollaan
// poikkileikkausnäytössä. KArttanäytössä tämä ei ole yleensä ongelma.
static double CalcUsedGraceFactor(int theCrossSectionIsoLineDrawIndex)
{
    double usedGraceFactor = -0.9;
    if(theCrossSectionIsoLineDrawIndex > 0)
    {
        if(theCrossSectionIsoLineDrawIndex % 2 == 1)
            usedGraceFactor = -0.5;
    }
    return usedGraceFactor;
}

template<typename Container>
static Container ScaleContainer(const Container& originalData, float scalingFactor)
{
    Container scaledData = originalData;
    std::transform(originalData.begin(), originalData.end(), scaledData.begin(), 
        [&scalingFactor](auto& value) {return value * scalingFactor; });
    return scaledData;
}

static float GetUsedReferenceLabelHeight(const std::vector<float>& labelHeights, float scaleFactor)
{
    float maxHeight = *(std::max_element(labelHeights.begin(), labelHeights.end()));
    float referenceLabelHeight = maxHeight * scaleFactor;
    return referenceLabelHeight;
}

static void SetIsolineLabelBoxSettings(IsolineVizualizationData& isolineVizData, float theViewHeight, int theCrossSectionIsoLineDrawIndex, float scaleFactor)
{
    float labelBoxWidth = 0;
    if(isolineVizData.useLabelBox())
    {
        labelBoxWidth = 0.15f;
    }
    auto labelTextColorIndex = isolineVizData.labelTextColorIndex();
    XuIsolineLabelBoxAttr(isolineVizData.labelBoxFillColorIndex(), labelTextColorIndex, 0, labelBoxWidth);

    // Label teksti height on riippuvainen ruudun koosta millimetreissä.
    // Lisäksi haetaan maksimi koko isolineVizData.labelHeights:ista, koska custom tapauksissa voi olla
    // joitain 0 arvoja listassa ja pitää hakea joku sopiva peruskorkeus sieltä.
    const auto& labelHeights = isolineVizData.labelHeights();
    float usedReferenceLabelHeight = GetUsedReferenceLabelHeight(labelHeights, scaleFactor);
    double usedGraceFactor = ::CalcUsedGraceFactor(theCrossSectionIsoLineDrawIndex);
    // isoline label strategioita (XuOFF, XuSIMPLE, XuCURVATURE_CHECK, XuOVERLAP_CHECK, XuF_IGNORE)
    XuIsolineLabel(usedReferenceLabelHeight, isolineVizData.labelDecimalsCount(), theViewHeight * usedGraceFactor, XuOVERLAP_CHECK);
    XuIsolineLabelAngle(55, -55);
    // label height pitää vielä säätää erikseen
    auto scaledLabelHeights = ::ScaleContainer(isolineVizData.labelHeights(), scaleFactor);
    XuIsolineLabelHeight(&scaledLabelHeights[0], static_cast<int>(scaledLabelHeights.size()));
    // en tiedä muuta tapaa muuttaa label-tekstin väriä
    XuTextColor(labelTextColorIndex, labelTextColorIndex);
}

static void SetupIsolineFeathering(IsolineVizualizationData & isolineVizData, float scaleFactor)
{
    if(isolineVizData.useFeathering())
        XuIsolineFeathering(0.8 * scaleFactor, 2 * scaleFactor); // 1.= min dist. in mm 2.= min displ. feath. isoline in mm
    else
        XuIsolineFeathering(0, 0); // 1.= min dist. in mm 2.= min displ. feath. isoline in mm
}

void DrawCustomIsoLines(NFmiIsoLineData &theIsoLineData, float theViewHeight, int theCrossSectionIsoLineDrawIndex, float scaleFactor)
{
    XuColorTableActivate(ToolMasterColorCube::UsedDefaultColorTableIndex());

    auto& isolineVizData = theIsoLineData.itsIsolineVizualizationData;

    XuClasses(&isolineVizData.finalToolmasterFixedClassLimits()[0], isolineVizData.usedIsolineCount());

    auto scaledLineWidths = ::ScaleContainer(isolineVizData.lineWidths(), scaleFactor);
    XuIsolineWidths(&scaledLineWidths[0], isolineVizData.usedIsolineCount());
    XuIsolineColors(&isolineVizData.finalColorIndexies()[0], isolineVizData.usedIsolineCount());
    XuIsolineSplineSmoothing(isolineVizData.splineSmoothingFactor());

    ::SetIsolineLabelBoxSettings(isolineVizData, theViewHeight, theCrossSectionIsoLineDrawIndex, scaleFactor);
    ::SetupIsolineFeathering(isolineVizData, scaleFactor);

    XuIsolineStyles(&isolineVizData.lineStyles()[0], isolineVizData.usedIsolineCount());
    XuIsolineDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
}

void DrawSimpleIsoLines(NFmiIsoLineData& theIsoLineData, float theViewHeight, int theCrossSectionIsoLineDrawIndex, float scaleFactor)
{
    XuColorTableActivate(ToolMasterColorCube::UsedDefaultColorTableIndex());

    auto& isolineVizData = theIsoLineData.itsIsolineVizualizationData;
    XuClassesStartStep(isolineVizData.startLimitValue(), isolineVizData.usedStep(), isolineVizData.usedIsolineCount());

    auto usedLineWidth = isolineVizData.lineWidths().front() * scaleFactor;
    XuIsolineWidths(&usedLineWidth, 1);
    if(isolineVizData.useSingleColor())
        XuIsolineColors(&isolineVizData.finalColorIndexies().front(), 1);
    else
        XuIsolineColors(&isolineVizData.finalColorIndexies()[0], isolineVizData.usedIsolineCount());

    XuIsolineSplineSmoothing(isolineVizData.splineSmoothingFactor());
    ::SetIsolineLabelBoxSettings(isolineVizData, theViewHeight, theCrossSectionIsoLineDrawIndex, scaleFactor);
    ::SetupIsolineFeathering(isolineVizData, scaleFactor);

    XuIsolineStyles(&isolineVizData.lineStyles().front(), 1);
    XuIsolineDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
}

static void RaportVisualizationMetrics(float theViewHeight)
{
    static bool firstTime = true;
    if(firstTime)
    {
        firstTime = false;
        float scaleFactor = ::CalcMMSizeFactor(theViewHeight, 1.1f);
        std::string message = "ToolMaster visualization metrics: viewHeight = ";
        message += NFmiStringTools::Convert(theViewHeight);
        message += " [mm], scaleFactor = ";
        message += NFmiStringTools::Convert(scaleFactor);
        CatLog::logMessage(message, CatLog::Severity::Info, CatLog::Category::Visualization);
    }
}

static void SetIsolineMinLength(double currentViewSizeInMM, double usedIsolineMinLengthFactor)
{
    // Perussäädöillä n. 200 mm korkea näyttö saa pituudeksi n. 4 mm
    double viewSizeFactor = currentViewSizeInMM / 50.;
    double usedIsolineMinLengthInMM = viewSizeFactor * usedIsolineMinLengthFactor;
    XuIsolineMinLength(usedIsolineMinLengthInMM);
}

static void DrawGridData(CDC* pDC, NFmiIsoLineData &theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiRect &theGridArea, int theCrossSectionIsoLineDrawIndex)
{
    static std::vector<float> gridNodesX;
    static std::vector<float> gridNodesY;

    CRect mfcClipRect;
    NFmiPoint totalViewSize;
    auto& dataGridToViewHeightRatio = theIsoLineData.itsDataGridToViewHeightRatio;

    SetupViewWorld(theIsoLineData, theRelViewRect, theZoomedViewRect, theGridArea, gridNodesX, gridNodesY, &mfcClipRect, totalViewSize, dataGridToViewHeightRatio);
    XuTextFont(XuSWIM, XuBEST_FIT); // vaikuttaa isoviiva label tekstin fonttiin

    // Once a crash report suggested that gridNodesX and gridNodesY were empty even though theIsoLineData had specific values for x- and y-grid dimensions
    if(gridNodesX.empty() || gridNodesY.empty())
    {
        std::string errorMessage("Error in ");
        errorMessage += __FUNCTION__;
        errorMessage += ": gridNodesX/Y was empty, reason unknown, won't visualize this data at all";
        CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
        return;
    }
    XuVariableGridNodesCreate(&gridNodesX[0], theIsoLineData.itsXNumber, &gridNodesY[0], theIsoLineData.itsYNumber, NULL, 0);
    XuClip(XuON);
    try
    {
        if(theIsoLineData.fUseColorContours) // tehdään ensin color contourit, sitten hatchit ja lopuksi isolinet
        {
            if(theIsoLineData.fUseCustomColorContoursClasses)
                ::DrawCustomColorContours(theIsoLineData);
            else
                ::DrawSimpleColorContours(theIsoLineData);
        }
        if(theIsoLineData.itsHatch1.fUseHatch || theIsoLineData.itsHatch2.fUseHatch)
        {
            ::GetReadyAndDoTheHatch(theIsoLineData, pDC, mfcClipRect);
        }
        if(theIsoLineData.fUseIsoLines)
        {
            float usedLength = static_cast<float>(totalViewSize.Y());
            ::RaportVisualizationMetrics(usedLength);
            if(theCrossSectionIsoLineDrawIndex >= 0)
                usedLength = static_cast<float>(totalViewSize.X()); // jos ollaan poikkileikkaus piirrossa (eli if lause tosi), käytetään label harvennuksen laskuissa ruudun leveyttä kertoimena
            float scaleFactor = CalcMMSizeFactor(usedLength, 1.1f);
            ::SetIsolineMinLength(usedLength, theIsoLineData.itsIsolineMinLengthFactor);

            if(theIsoLineData.fUseCustomIsoLineClasses)
                ::DrawCustomIsoLines(theIsoLineData, usedLength, theCrossSectionIsoLineDrawIndex, scaleFactor);
            else
                ::DrawSimpleIsoLines(theIsoLineData, usedLength, theCrossSectionIsoLineDrawIndex, scaleFactor);
        }
    }
    catch(...)
    {
    }

    XuClip(XuOFF);
    XuVariableGridNodesOption(XuGRID_DELETE);
}

static void DoTraceLogging(const std::string &message)
{
    if(CatLog::doTraceLevelLogging())
    {
        CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization, true);
    }
}

static void DoPossibleQuickContourSetting(NFmiIsoLineData &theOrigIsoLineDataInOut, const NFmiPoint &thePixelToGridPointRatio, const NFmiVisualizationSpaceoutSettings& visualizationSettings)
{
    if(theOrigIsoLineDataInOut.fUseColorContours == 1) // jos käytössä tavallinen contouraus
    {
        const auto criticalPixelToGridPointRatio = visualizationSettings.criticalPixelToGridPointRatioLimitForContours();
        if(thePixelToGridPointRatio.X() != 0 && thePixelToGridPointRatio.Y() != 0)
        {
            if(thePixelToGridPointRatio.X() < criticalPixelToGridPointRatio || thePixelToGridPointRatio.Y() < criticalPixelToGridPointRatio)
            { // jos hila koko on tarpeeksi lähellä näytön pikseli kokoa, laitetaan quickcontour päälle
                theOrigIsoLineDataInOut.fUseColorContours = 2;
                ::DoTraceLogging(std::string("Changing to quick-contour draw with ") + ::MakeDataIdentString(theOrigIsoLineDataInOut.itsParam));
            }
        }
    }
}

int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint& thePixelToGridPointRatio, int theCrossSectionIsoLineDrawIndex, const NFmiVisualizationSpaceoutSettings& visualizationSettings)
{
    NFmiRect gridArea(0, 0, 1, 1); // tämä on normaali yksi osaisen hilan alue (0,0 - 1,1)
        // Optimointia: jos piirretään contoureja, ja datan hilaväli lähetyy pikseli kokoa, laitetaan quickcontour -optio päälle
    ::DoPossibleQuickContourSetting(*theIsoLineData, thePixelToGridPointRatio, visualizationSettings);
    ::DrawGridData(pDC, *theIsoLineData, theRelViewRect, theZoomedViewRect, gridArea, theCrossSectionIsoLineDrawIndex);

    return 0;
}

#endif // DISABLE_UNIRAS_TOOLMASTER
