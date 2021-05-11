#include "stdafx.h"

#ifndef DISABLE_UNIRAS_TOOLMASTER

#include <agX/agxfont.h>
#include <agX/agx.h>

#include <algorithm>
#include "ToolMasterDrawingFunctions.h"
#include "NFmiFastQueryInfo.h" // t‰m‰n pit‰‰ olla ennen NFmiIsoLineData.h includea!
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
#include "NFmiDataMatrixUtils.h"
#include "ToolMasterColorCube.h"

#include <fstream>
#include "boost/math/special_functions/round.hpp"

// Win32 makrot s‰hl‰‰v‰t std-min ja max:ien k‰ytˆn, ne pit‰‰ 'undefinoida'
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

// t‰ytt‰‰ gridnode datan (kutsutaan kun zoomataan)
// oletus alue on yksikkˆ laatikon sis‰ll‰ (0,0, 1,1)
static void FillGridNodeData(NFmiIsoLineData &theIsoLineData, std::vector<float>& theGridNodesX, std::vector<float>& theGridNodesY, const NFmiRect &theGridArea)
{
    // T‰m‰n pit‰‰ saada ottamaan huomioon myˆs partial hila-rect

    theGridNodesX.resize(theIsoLineData.itsXNumber);
    theGridNodesY.resize(theIsoLineData.itsYNumber);

    int i = 0;
    for(i = 0; i < theIsoLineData.itsXNumber; i++)
        theGridNodesX[i] = static_cast<float>(theGridArea.Left() + ((theGridArea.Width() * i) / (theIsoLineData.itsXNumber - 1)));
    for(i = 0; i < theIsoLineData.itsYNumber; i++)
        theGridNodesY[i] = static_cast<float>(theGridArea.Top() + ((theGridArea.Height() * i) / (theIsoLineData.itsYNumber - 1)));
}

// Asettaa Toolmasterin viewworldin ja workboxit ja muut systeemit piirtoa varten.
// Palauttaa parametreina clippaus rect:in ja n‰ytˆn korkeuden.
// Asettaa toolmasterin k‰ytt‰m‰t gridnode:t (miss‰ kukin hilapiste on ToolMaster maailmassa)
// Parametri theRelViewRect: N‰kyv‰n kartta-alueen koko suhteellisessa 0,0 - 1,1 maailmassa. T‰h‰n 
// vaikuttaa mm. karttaruudukko ja alareunan aikakontrolli-ikkuna.
// Parametri theZoomedViewRect: N‰kyv‰n kartta-alueen koko suhteessa datasta k‰ytetyn hilan alueeseen. K‰ytetyn hilan alue on aina
// 0,0 - 1,1. Jos datan alue on pienempi kuin kartan, on t‰m‰ isompi kuin datan alue ja toisin p‰in.
// Parametri theGridArea: t‰m‰ on aina 0,0 - 1,1.
// Parametrit theGridNodesX/Y: N‰ihin vektoreihin lasketaan k‰ytetyn hilan x- ja y-pisteiden paikat theGridArea:n maailmassa.
// Parametri theMfcClipRect: T‰h‰n lasketaan piirtoalueen laatikko pikseleiss‰. Sit‰ k‰ytet‰‰n ei ToolMaster piirroissa.
// Parametri theTotViewSizeOut: T‰h‰n lasketaan piirtoalueen koko pikseleiss‰, tietoa k‰ytet‰‰n isoviiva labeloinnin harvennukseen.
void SetupViewWorld(NFmiIsoLineData &theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiRect &theGridArea, std::vector<float>& theGridNodesX, std::vector<float>& theGridNodesY, CRect* theMfcClipRect, NFmiPoint &theTotViewSizeOut, double & dataGridToViewHeightRatioOut)
{
    float xsi, ysi; // koko CWnd ikkunan piirtoalueen koko [mm]
    int   xpic, ypic; // koko CWnd ikkunan piirtoalueen koko pikseleiss‰
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
    double yoff = ysi * (1 - rb); // taas k‰‰nnet‰‰n y-akselin suhteen!!!!

    theTotViewSizeOut.X(xsi * theRelViewRect.Size().X());
    theTotViewSizeOut.Y(ysi * theRelViewRect.Size().Y());

    // theGridArea on aina 0,0 - 1,1, eli k‰ytetty hila on vakio paikassa ja piirtosysteemi‰ siirret‰‰n sen mukaan.
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
    // Asetetaan kartalla n‰kyv‰n alueen koko ja paikka suhteellisessa maailmassa
    XuViewWorldLimits(zxmin, zxmax, zymin, zymax, 0, 0);
    // Asetetaan eri akseleiden suhteita, t‰rkein on kai tuo x/y suhdeluku
    XuViewWorkbox(xsize / ysize, 1, 0);
    // Asetetaan n‰kyv‰n maailman koko ja paikka pikseleissa n‰yttˆikkunassa
    XuViewport(xoff, yoff, xsize, ysize);

    int leftPix = static_cast<int>(rl * xpic);
    int topPix = static_cast<int>(rt * ypic);
    int rightPix = static_cast<int>(rr * xpic);
    int bottomPix = static_cast<int>(rb * ypic);

    // Toolmaster tekee omat clippaukset, mutta kun tehd‰‰n omia piirtojuttuja, k‰ytet‰‰n t‰t‰ clip aluetta
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
    // 1. Viivan paksuus joko 0, jos ei piirret‰ ja 0.1, jos piirret‰‰n.
    float isolineWidth = theIsoLineData.fUseSeparatorLinesBetweenColorContourClasses ? 0.1f : 0.f;
    XuIsolineWidths(&isolineWidth, 1);
    // 2. Viivan v‰ri
    int isolineColorArr = 0; // 0 = windows background v‰ri eli k‰yt‰nnˆss‰ musta
    XuIsolineColors(&isolineColorArr, 1);
    // 3. Viivan piirto tyyli
    int isolineStyle = theIsoLineData.itsColorContouringData.separationLineStyle();
    XuIsolineStyles(&isolineStyle, 1);

    XuIsolineSplineSmoothing(0); // ei kannata pyˆrist‰‰ isoviivoja koska contourit eiv‰t kuitenkaan pyˆristy
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

// Laita t‰m‰n flagin arvo debuggerissa true:ksi, kun haluat tulostaa \ToolMasterContourSetups.txt 
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
        // Tehd‰‰n uudesta color-tablen indeksist‰ k‰ytetyn default taulun indeksi + 1
        if(!colorContouringData.createNewToolMasterColorTable(ToolMasterColorCube::UsedDefaultColorTableIndex() + 1))
            return; // Ei tehd‰ piirtoa, jos color-tablen alustus meni jotenkin pieleen...
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

// Pieni overlap fiksaus polygoneihin. En tied‰ johtuuko ongelma toolmasterin polygoneista vai
// GDI piirron ominaisuuksista, mutta t‰m‰ korjaa yhden pikselin satunnaiset v‰lit hatcheist‰.
// Eli lasketaan koko ajan, mik‰ oli edellisen polygoni rivin ylimm‰n pikselin arvo (monitorilla ylin
// on siis pienin arvo, kun 0,0 piste on n‰ytˆn top-left). Jos l‰pik‰yt‰v‰ piste on juuri siin‰, siirret‰‰n 
// se yhden pikselin verran alasp‰in (overlap).
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
    if(hatchSettings.itsHatchPattern == -1) // jos hatch-pattern on -1, tehd‰‰nkin t‰ysin peitt‰v‰ sivellin ilman hatchi‰
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

bool gDrawTest = false; // Laita t‰m‰n arvoksi true, jos haluat n‰hd‰ tietyn piirrett‰n polygonin high-lightauksen.

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
// yksi. label miel. keskelle sit‰.
// jos tulee isoviiva indeksi arvoja (index>0), annetaan joka toiselle arvo -0.9 ja
// joka toiselle 0.5. T‰llˆin joka toinen isoviiva piirret‰‰n yhdell‰ labelilla (keskelle)
// ja joka toiselle kahdelle 1/3 ja 2/3 kohtiin iso viivaa. T‰ll‰ yritet‰‰n v‰ltt‰‰
// eri parametreja p‰‰llekk‰in piirrett‰ess‰ piirt‰‰ labelit eri kohtiin kun ollaan
// poikkileikkausn‰ytˆss‰. KArttan‰ytˆss‰ t‰m‰ ei ole yleens‰ ongelma.
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

    // Label teksti height on riippuvainen ruudun koosta millimetreiss‰.
    // Lis‰ksi haetaan maksimi koko isolineVizData.labelHeights:ista, koska custom tapauksissa voi olla
    // joitain 0 arvoja listassa ja pit‰‰ hakea joku sopiva peruskorkeus sielt‰.
    const auto& labelHeights = isolineVizData.labelHeights();
    float usedReferenceLabelHeight = GetUsedReferenceLabelHeight(labelHeights, scaleFactor);
    double usedGraceFactor = ::CalcUsedGraceFactor(theCrossSectionIsoLineDrawIndex);
    // isoline label strategioita (XuOFF, XuSIMPLE, XuCURVATURE_CHECK, XuOVERLAP_CHECK, XuF_IGNORE)
    XuIsolineLabel(usedReferenceLabelHeight, isolineVizData.labelDecimalsCount(), theViewHeight * usedGraceFactor, XuOVERLAP_CHECK);
    XuIsolineLabelAngle(55, -55);
    // label height pit‰‰ viel‰ s‰‰t‰‰ erikseen
    auto scaledLabelHeights = ::ScaleContainer(isolineVizData.labelHeights(), scaleFactor);
    XuIsolineLabelHeight(&scaledLabelHeights[0], static_cast<int>(scaledLabelHeights.size()));
    // en tied‰ muuta tapaa muuttaa label-tekstin v‰ri‰
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
    // Peruss‰‰dˆill‰ n. 200 mm korkea n‰yttˆ saa pituudeksi n. 4 mm
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
        if(theIsoLineData.fUseColorContours) // tehd‰‰n ensin color contourit, sitten hatchit ja lopuksi isolinet
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
                usedLength = static_cast<float>(totalViewSize.X()); // jos ollaan poikkileikkaus piirrossa (eli if lause tosi), k‰ytet‰‰n label harvennuksen laskuissa ruudun leveytt‰ kertoimena
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

static bool IsIsolinesDrawn(const NFmiIsoLineData &theOrigIsoLineData)
{
    if(theOrigIsoLineData.fUseIsoLines)
        return true;
    return false;
}

double CalcFinalDownSizeRatio(double criticalRatio, double currentRatio)
{
    if(currentRatio)
    {
        if(currentRatio < criticalRatio)
            return criticalRatio / currentRatio;
    }
    return 1.;
}

static bool IsIsolinesDrawn(const boost::shared_ptr<NFmiDrawParam>& thePossibleDrawParam)
{
    auto gridDataDrawStyle = thePossibleDrawParam->GridDataPresentationStyle();
    return gridDataDrawStyle == NFmiMetEditorTypes::View::kFmiIsoLineView || gridDataDrawStyle == NFmiMetEditorTypes::View::kFmiColorContourIsoLineView;
}

bool IsDownSizingNeeded(const NFmiPoint& theGrid2PixelRatio, double criticalGrid2PixelRatio, NFmiPoint& theDownSizeFactorOut)
{
    const NFmiPoint zeroChangeFactor(1, 1);
    theDownSizeFactorOut.X(::CalcFinalDownSizeRatio(criticalGrid2PixelRatio, theGrid2PixelRatio.X()));
    theDownSizeFactorOut.Y(::CalcFinalDownSizeRatio(criticalGrid2PixelRatio, theGrid2PixelRatio.Y()));
    return theDownSizeFactorOut != zeroChangeFactor;
}

bool IsolineDataDownSizingNeeded(const NFmiIsoLineData &theOrigIsoLineData, const NFmiPoint &theGrid2PixelRatio, NFmiPoint &theDownSizeFactorOut, const boost::shared_ptr<NFmiDrawParam>& thePossibleDrawParam)
{
    if(thePossibleDrawParam)
    {
        if(!::IsIsolinesDrawn(thePossibleDrawParam))
            return false;
    }
    else if(!::IsIsolinesDrawn(theOrigIsoLineData))
        return false;

    const double criticalGrid2PixelRatio = 3.0;
    return IsDownSizingNeeded(theGrid2PixelRatio, criticalGrid2PixelRatio, theDownSizeFactorOut);
}

static void CalcDownSizedMatrix(const NFmiDataMatrix<float> &theOrigData, NFmiDataMatrix<float> &theDownSizedData, const NFmiParam &param)
{
    auto paramId = static_cast<FmiParameterName>(param.GetIdent());
    auto interpolationMethod = param.InterpolationMethod();
    auto dontInvertY = true;
    double xDiff = 1.0 / (theDownSizedData.NX() - 1.0);
    double yDiff = 1.0 / (theDownSizedData.NY() - 1.0);
    for(size_t j = 0; j < theDownSizedData.NY(); j++)
    {
        for(size_t i = 0; i < theDownSizedData.NX(); i++)
        {
            NFmiPoint pt(i * xDiff, j * yDiff);
            if(pt.X() > 1.)
                pt.X(1.); // t‰m‰ on varmistus, jos laskenta tarkkuus ongelmat vie rajan yli
            if(pt.Y() > 1.)
                pt.Y(1.); // t‰m‰ on varmistus, jos laskenta tarkkuus ongelmat vie rajan yli
            theDownSizedData[i][j] = DataMatrixUtils::InterpolatedValue(theOrigData, pt, paramId, true);
        }
    }
}

static void BuildDownSizedData(NFmiIsoLineData &theOrigIsoLineData, NFmiIsoLineData &theDownSizedIsoLineData, const NFmiPoint &theDownSizeFactor)
{
    int newSizeX = boost::math::iround(theOrigIsoLineData.itsXNumber / theDownSizeFactor.X());
    int newSizeY = boost::math::iround(theOrigIsoLineData.itsYNumber / theDownSizeFactor.Y());
    // T‰ytet‰‰n uuden isolineDatan hila-arvot halutuille osaalueilleen.
    NFmiDataMatrix<float> downSizedGridData(newSizeX, newSizeY, kFloatMissing);
    ::CalcDownSizedMatrix(theOrigIsoLineData.itsIsolineData, downSizedGridData, *theOrigIsoLineData.itsParam.GetParam());

    // Alustetaan uusi isolinedata harvennetulla matriisilla ja sen piirtoasetukset otetaan originaalista
    theDownSizedIsoLineData.InitIsoLineData(downSizedGridData, &theOrigIsoLineData);
}

static void DoTraceLogging(const std::string &message)
{
    if(CatLog::doTraceLevelLogging())
    {
        CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization, true);
    }
}

double GetCriticalGrid2PixelRatioForContour()
{
    return 1.4;
}

static void DoPossibleQuickContourSetting(NFmiIsoLineData &theOrigIsoLineDataInOut, const NFmiPoint &theGrid2PixelRatio)
{
    if(theOrigIsoLineDataInOut.fUseColorContours == 1) // jos k‰ytˆss‰ tavallinen contouraus
    {
        const double criticalGrid2PixelRatio = GetCriticalGrid2PixelRatioForContour();
        if(theGrid2PixelRatio.X() != 0 && theGrid2PixelRatio.Y() != 0)
        {
            if(theGrid2PixelRatio.X() < criticalGrid2PixelRatio || theGrid2PixelRatio.Y() < criticalGrid2PixelRatio)
            { // jos hila koko on tarpeeksi l‰hell‰ n‰ytˆn pikseli kokoa, laitetaan quickcontour p‰‰lle
                theOrigIsoLineDataInOut.fUseColorContours = 2;
                ::DoTraceLogging(std::string("Changing to quick-contour draw with ") + ::MakeDataIdentString(theOrigIsoLineDataInOut.itsParam));
            }
        }
    }
}

static std::string MakeIsoLineDataGridSizeString(NFmiIsoLineData* theIsoLineData)
{
    std::string str = std::to_string(theIsoLineData->itsXNumber);
    str += " x ";
    str += std::to_string(theIsoLineData->itsYNumber);
    return str;
}

int ToolMasterDraw(CDC* pDC, NFmiIsoLineData* theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiPoint &theGrid2PixelRatio, int theCrossSectionIsoLineDrawIndex)
{
    NFmiRect gridArea(0, 0, 1, 1); // t‰m‰ on normaali yksi osaisen hilan alue (0,0 - 1,1)
    NFmiPoint downSizeFactor;
    // Toolmaster piirto bugi ilmenee kun hila vs. pikseli suhde on n. 1 tai alle. Kierr‰n siten ongelman niin ett‰ kun t‰m‰
    // ratio on tarpeeksi pieni, lasken uuden hila koon, niin ett‰ sen suhdeluku on minimiss‰‰n 1.3 ja interpoloin t‰ll‰iseen
    // uuteen hilaan datan. T‰m‰n j‰lkeen piirto onnistuu ilman ongelmia, eik‰ asiakas huomaa juuri mit‰‰n.
    bool doDownSize = IsolineDataDownSizingNeeded(*theIsoLineData, theGrid2PixelRatio, downSizeFactor, nullptr);
    if(doDownSize)
    {
        NFmiIsoLineData downSizedIsoLineData;
        ::BuildDownSizedData(*theIsoLineData, downSizedIsoLineData, downSizeFactor);
        ::DoTraceLogging(std::string("Down-sizing isoline data from ") + ::MakeIsoLineDataGridSizeString(theIsoLineData) + " to " + ::MakeIsoLineDataGridSizeString(&downSizedIsoLineData));
        ::DrawGridData(pDC, downSizedIsoLineData, theRelViewRect, theZoomedViewRect, gridArea, theCrossSectionIsoLineDrawIndex);
    }
    else
    {
        // Optimointia: jos piirret‰‰n contoureja, ja datan hilav‰li l‰hetyy pikseli kokoa, laitetaan quickcontour -optio p‰‰lle
        ::DoPossibleQuickContourSetting(*theIsoLineData, theGrid2PixelRatio);
        ::DrawGridData(pDC, *theIsoLineData, theRelViewRect, theZoomedViewRect, gridArea, theCrossSectionIsoLineDrawIndex);
    }

    return 0;
}

#endif // DISABLE_UNIRAS_TOOLMASTER
