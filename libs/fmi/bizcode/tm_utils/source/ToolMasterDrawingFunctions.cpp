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

#include <fstream>
#include "boost/math/special_functions/round.hpp"

class FloatPoint
{
public:
    FloatPoint(void)
        :x(0)
        , y(0)
    {
    }

    float x;
    float y;
};

class IntPoint
{
public:
    IntPoint(void)
        :x(0)
        , y(0)
    {
    }

    int x;
    int y;
};

class BoundingBox
{
public:
    BoundingBox(void)
        :top(0)
        , bottom(0)
        , left(0)
        , right(0)
    {
    }

    float top;
    float bottom;
    float left;
    float right;
};

class TMWorldLimits
{
public:
    TMWorldLimits(void)
        :x_min(0)
        , x_max(1)
        , y_min(0)
        , y_max(1)
        , z_min(0)
        , z_max(0)
    {
    }

    float x_min;
    float x_max;
    float y_min;
    float y_max;
    float z_min;
    float z_max;
};

class PolygonLineInfo
{
public:
    PolygonLineInfo(void)
        :itsSideStatus(kNoDirection)
        , itsSecondPointIndex(0)
        , itsLineLength(0)
    {
    }

    FmiDirection itsSideStatus;
    size_t itsSecondPointIndex; // indeksi polygon taulukkoon, 1. piste on siis (itsSecondPointIndex - 1)
    float itsLineLength;
};

bool IsLineLonger(const PolygonLineInfo &pi1, const PolygonLineInfo &pi2)
{
    return pi1.itsLineLength > pi2.itsLineLength;
}

static float CalcLineLen(const FloatPoint &p1, const FloatPoint &p2)
{
    return ::sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

// kerroin 1 = ruudun koko m‰‰r‰‰ suoraan symbolien jne. koot
// kerroin <1 loiventaa muutosta eli pieniin ruutuihin tulee hieman isommat symbolit (kuin kertoimella 1)
// ja isoihin ruutuihin hieman pienemm‰t symbolit (kuin kertoimella 1)
//static float gDefaultNormalViewHeightInMMFactor = 0.7;

float CalcMMSizeFactor(float theViewHeightInMM, float theMaxFactor)
{
    float factor = 1.f - (0.65f * (180.f - theViewHeightInMM) / 180.f);
    factor = FmiMin(factor, theMaxFactor);
    return factor;
}

using namespace std;

// Huom! Oletus on ett‰ missing arvot on tarkastettu ennen t‰t‰.
// Vai pit‰isikˆ missing tapuksessa heitt‰‰ poikkeus?
static bool IsInsideLimits(float low, float high, float value)
{
    return (value >= low) && (value <= high);
}

// muista theColorIndexies-vektorin koko ei ole k‰ytett‰viss‰ t‰ss‰, koska se on asetettu joksikin maksimi kooksi
static bool IsTransparencyColorUsed(const checkedVector<int>& theColorIndexies, int theRealColorIndexCount, int theTransparencyColorIndex)
{
    for(int i = 0; i < theRealColorIndexCount; i++)
        if(theColorIndexies[i] == theTransparencyColorIndex)
            return true;
    return false;
}

static const NFmiColor gTransparentColor(1.f, 1.f, 1.f, 1.f);  // valkoinen ja 100% l‰pin‰kyv‰ on l‰pin‰kyvyyden merkki

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

static void AddColorValuesToTables(checkedVector<NFmiColor> &colors, checkedVector<float> &classes, checkedVector<int> &colInds, const NFmiColor &aColor, float classValue, int colInd, bool lastValue)
{
    colors.push_back(aColor);
    if(lastValue == false) // viimeisell‰ rajalla ei laiteta luokkaa sis‰‰n, luokkia PITƒƒ olla yksi v‰hemm‰n kuin v‰rej‰ color contourauksessa
        classes.push_back(classValue);
    if(aColor == gTransparentColor)
        colInds.push_back(2);
    else
        colInds.push_back(colInd);
}

// tehd‰‰n ja aktivoidaan ToolMaster ColorTable, jossa on myˆs mukana transparency v‰ri (index 2)
// T‰t‰ kutsuttaessa on jo tiedosssa ett‰ mukana on transparentti v‰ri/v‰rej‰.
static void InitCustomColorTableWithTransparentColor(NFmiIsoLineData &theIsoLineData, int theColorTableIndex, int theTransparencyColorIndex)
{
    size_t origColorIndexTableSize = theIsoLineData.itsColorIndexCount;
    int realOrigClassesSize = theIsoLineData.itsTrueIsoLineCount; // t‰ss‰ on tallessa originaali luokkien oikea lukum‰‰r‰ kun steppi contourit ja l‰pin‰kyvi‰ v‰rej‰ mukana
    float step = theIsoLineData.itsColorContoursStep;
    if(origColorIndexTableSize <= 2 || realOrigClassesSize <= 2 || step == 0)
        return;

    float colorRGB[3], dummy[5];

    checkedVector<NFmiColor> usedColorVector;
    for(size_t i = 0; i < origColorIndexTableSize; i++)
    {
        if(theIsoLineData.itsCustomColorContoursColorIndexies[i] == theTransparencyColorIndex)
            usedColorVector.push_back(gTransparentColor);
        else
        {
            XuColorQuery(theIsoLineData.itsCustomColorContoursColorIndexies[i], colorRGB, dummy);
            usedColorVector.push_back(NFmiColor(colorRGB[0] / 255.f, colorRGB[1] / 255.f, colorRGB[2] / 255.f));
        }
    }

    // theIsoLineData->itsTrueColorContoursCount // t‰ss‰ on laskettu oikea m‰‰r‰ lopullisia rajoja luokille
    checkedVector<float> &origContourClasses = theIsoLineData.itsCustomColorContours; // t‰ss‰ on tallessa k‰ytetyt luokat

    float aMin = theIsoLineData.itsClassMinValue;
    float aMax = theIsoLineData.itsClassMaxValue;
    //int classCount = theIsoLineData->itsTrueColorContoursCount;

    checkedVector<NFmiColor> finalColorTable;
    checkedVector<float> finalClassesTable;
    checkedVector<int> finalColorIndexTable; // t‰m‰ ratkaisee muidenkin taulukoiden koon, ei talleteta duplikaatteja tauluun

    // t‰ytet‰‰n 1. data osio eli v‰rit ennen 1. rajaa
    float firstOrigClass = origContourClasses[0];
    int runningColorIndex = 3;
    if(aMin < firstOrigClass)
    {
        ::AddColorValuesToTables(finalColorTable, finalClassesTable, finalColorIndexTable, usedColorVector[0], firstOrigClass, runningColorIndex++, false);
    }

    // t‰ytet‰‰n v‰li v‰rit 1. ja viimeisen rajan v‰lill‰
    NFmiColor col1 = usedColorVector[0];
    NFmiColor col2;
    bool colorSlideJustMade = false;
    int usedColorIndex = 0;
    bool firstTime = true;
    bool fixStartslideColor = false; // jos t‰m‰ on true, aloita v‰ri liuutus niin ett‰ 1. kohdassa on puhdas col1 (t‰m‰ siis heti transparentin v‰rin j‰lkeen)
    for(int i = 0; i < realOrigClassesSize - 1; i++)
    {
        float currentOrigClass = origContourClasses[i];
        float nextOrigClass = origContourClasses[i + 1];
        if(usedColorIndex >= static_cast<int>(origColorIndexTableSize - 1))
            break; // muuten menee v‰ri taulun yli
        col2 = usedColorVector[usedColorIndex + 1];
        if(col1 != gTransparentColor && col2 == gTransparentColor)
        {
            if(firstTime == false && colorSlideJustMade == false)
            {
                ::AddColorValuesToTables(finalColorTable, finalClassesTable, finalColorIndexTable, col1, nextOrigClass, runningColorIndex++, false);
                i++;
                if(i + 1 >= realOrigClassesSize)
                    break;
                nextOrigClass = origContourClasses[i + 1];
            }
            ::AddColorValuesToTables(finalColorTable, finalClassesTable, finalColorIndexTable, col2, nextOrigClass, runningColorIndex++, false);
            colorSlideJustMade = false;
            fixStartslideColor = false;
        }
        else if(col1 == gTransparentColor && col2 == gTransparentColor)
        {
            ::AddColorValuesToTables(finalColorTable, finalClassesTable, finalColorIndexTable, col2, nextOrigClass, runningColorIndex++, false);
            colorSlideJustMade = false;
            fixStartslideColor = false;
        }
        else if(col1 != gTransparentColor && col2 != gTransparentColor)
        { // nyt molemmat v‰rit ovat ei transparenttia, joten nyt liuutetaan v‰rej‰ luokasta toiseen steppien mukaan
            // menn‰‰n l‰pi luvut alaraja -> yl‰raja - step
            float colorRatioFixer = 0;
            if(fixStartslideColor)
                colorRatioFixer = step;
            for(float currentValue = currentOrigClass + step; currentValue <= nextOrigClass; currentValue += step)
            {
                NFmiColor mixCol(col1);
                float mixRatio = (currentValue - currentOrigClass - colorRatioFixer) / (nextOrigClass - currentOrigClass - colorRatioFixer);
                mixCol.Mix(col2, mixRatio);
                ::AddColorValuesToTables(finalColorTable, finalClassesTable, finalColorIndexTable, mixCol, currentValue, runningColorIndex++, false);
            }
            colorSlideJustMade = true;
            fixStartslideColor = false;
        }
        else
        {
            // jos edellinen oli transparent, mutta seuraava ei ole, siirryt‰‰n v‰reiss‰ yksi askel eteenp‰in
            i--; // perus juoksutus indeksi‰ pit‰‰ v‰henbt‰‰, koska haluamme vain edist‰‰ v‰rien juoksutusta
            colorSlideJustMade = false;
            fixStartslideColor = true;
        }
        usedColorIndex++;
        col1 = col2;
        firstTime = false;

    } // end of for

    // t‰ytet‰‰n viimeinen data osio eli v‰rit viimeisen rajan j‰lkeen
    float lastOrigClass = origContourClasses[realOrigClassesSize - 1];
    if(lastOrigClass <= aMax)
    {
        NFmiColor lastCol1 = usedColorVector[usedColorIndex];
        int lastColorIndex = usedColorIndex + 1;
        if(origColorIndexTableSize <= static_cast<size_t>(lastColorIndex))
            lastColorIndex = static_cast<int>(origColorIndexTableSize - 1);
        NFmiColor lastCol2 = usedColorVector[lastColorIndex];
        if(lastCol1 == gTransparentColor || lastCol2 == gTransparentColor)
            ::AddColorValuesToTables(finalColorTable, finalClassesTable, finalColorIndexTable, lastCol2, lastOrigClass, runningColorIndex++, true);
        else
            ::AddColorValuesToTables(finalColorTable, finalClassesTable, finalColorIndexTable, lastCol1, lastOrigClass, runningColorIndex++, true);
    }


    // 3+ on tilaa fore- ja background coloreille ja transparentille v‰rille (index 2)
    int colorTableSize = static_cast<int>(finalColorIndexTable.size() + 3);

    XuColorTableCreate(theColorTableIndex, colorTableSize, XuLOOKUP, XuRGB, 255);
    XuColorTableActivate(theColorTableIndex);
    XuClasses(&finalClassesTable[0], static_cast<int>(finalClassesTable.size()));
    XuShadingColorIndices(&finalColorIndexTable[0], static_cast<int>(finalColorIndexTable.size()));

    // Copy Windows' text color to Toolmaster's foreground
    COLORREF f = GetSysColor(COLOR_WINDOWTEXT);
    colorRGB[0] = GetRValue(f);
    colorRGB[1] = GetGValue(f);
    colorRGB[2] = GetBValue(f);
    XuColor(XuCOLOR, 1, colorRGB, dummy);
    XuColorDeviceLoad(1);

    // Copy Windows' window color to Toolmasters background
    f = GetSysColor(COLOR_WINDOW);
    colorRGB[0] = GetRValue(f);
    colorRGB[1] = GetGValue(f);
    colorRGB[2] = GetBValue(f);
    XuColor(XuCOLOR, 0, colorRGB, dummy);
    XuColorDeviceLoad(0);

    // m‰‰ritet‰‰n 3. v‰ri l‰pin‰kyv‰ksi (hollow)
    int hollowColorIndex = 2;
    int colorType = 0;
    XuColorTypeQuery(hollowColorIndex, &colorType);
    if(colorType != XuOFF)
        XuColorType(hollowColorIndex, XuOFF);
    XuUndefined(kFloatMissing, hollowColorIndex);
    XuColorType(hollowColorIndex, XuHOLLOW_COLOR);

    int index = hollowColorIndex + 1; // aletaan rakentaan v‰ri taulukkoa l‰pin‰kyv‰n v‰rin j‰lkeen
    for(size_t i = 0; i < finalColorTable.size(); i++)
    {
        if(finalColorTable[i] != gTransparentColor)
        {
            colorRGB[0] = static_cast<float>(int(255 * finalColorTable[i].GetRed()));
            colorRGB[1] = static_cast<float>(int(255 * finalColorTable[i].GetGreen()));
            colorRGB[2] = static_cast<float>(int(255 * finalColorTable[i].GetBlue()));
            XuColor(XuCOLOR, index, colorRGB, dummy);
        }
        index++;
    }
}

// paluttaa annettua v‰ri‰ l‰himm‰n v‰rin indeksin annetusta 'v‰rikuutiosta'
static int RgbToColorIndex(Matrix3D<std::pair<int, COLORREF> > &theColorsCube, const NFmiColor& color)
{
    int k = static_cast<int>(color.Red()*(theColorsCube.Rows() - 1));
    int j = static_cast<int>(color.Green()*(theColorsCube.Columns() - 1));
    int i = static_cast<int>(color.Blue()*(theColorsCube.zDIM() - 1));

    return theColorsCube[k][j][i].first;
}

// t‰ytt‰‰ gridnode datan (kutsutaan kun zoomataan)
// oletus alue on yksikkˆ laatikon sis‰ll‰ (0,0, 1,1)
static void FillGridNodeData(NFmiIsoLineData &theIsoLineData, checkedVector<float>& theGridNodesX, checkedVector<float>& theGridNodesY, const NFmiRect &theGridArea)
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
void SetupViewWorld(NFmiIsoLineData &theIsoLineData, const NFmiRect& theRelViewRect, const NFmiRect& theZoomedViewRect, const NFmiRect &theGridArea, checkedVector<float>& theGridNodesX, checkedVector<float>& theGridNodesY, CRect* theMfcClipRect, NFmiPoint &theTotViewSizeOut)
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

// t‰t‰ k‰ytet‰‰n kun lasketaan custom color contour jossa k‰ytetty askellusta ja ei transparentteja v‰rej‰
static checkedVector<float> CalcCustomColorWidths(NFmiIsoLineData &theIsoLineData)
{
    int origClassCount = theIsoLineData.itsColorIndexCount;
    checkedVector<float> widths(origClassCount, 1.f);
    if(origClassCount > 1)
    {
        for(int i = 0; i < origClassCount; i++)
        {
            float width = 1;
            if(i == origClassCount - 1) // viimeinen on speciaali tapaus
                width = (theIsoLineData.itsCustomColorContours[origClassCount - 1] - theIsoLineData.itsCustomColorContours[origClassCount - 2]); // lasketaan viimeisen v‰lin erotus (uudestaan)
            else // muuten lasketaan t‰m‰n ja seuraava v‰linen erotus
                width = (theIsoLineData.itsCustomColorContours[i + 1] - theIsoLineData.itsCustomColorContours[i]);

            widths[i] = width;
        }
    }
    return widths;
}

static void CreateClassesAndColorTableAndColorShadeForCustomContourWithSteps(float aMin, float aMax, int classCount, checkedVector<int> defaultTableColorIndices, int shadingScaleIndex, int colorTableIndex, checkedVector<float> colorWidths, NFmiIsoLineData &theIsolineData)
{
    float colorRGB[3], hatch[5];

    size_t colorTablesize = defaultTableColorIndices.size();
    theIsolineData.ResizeDefRGBArray(colorTablesize);
    colorTablesize = theIsolineData.itsDefRGBRowSize; // t‰ll‰ on maksimi koko, mik‰ pit‰‰ tarkistaa t‰ss‰
    for(size_t i = 0; i < colorTablesize; i++)
    {
        theIsolineData.itsDefRGB[i][0] = colorWidths[i];
        XuColorQuery(defaultTableColorIndices[i], colorRGB, hatch);
        theIsolineData.itsDefRGB[i][1] = colorRGB[0];
        theIsolineData.itsDefRGB[i][2] = colorRGB[1];
        theIsolineData.itsDefRGB[i][3] = colorRGB[2];
    }

    const int max_level = 255; // RGB tapauksessa avo on 0-255
    XuClassesMinMax(aMin, aMax, classCount);
    XuShadingScaleChange(shadingScaleIndex, theIsolineData.itsDefRGB, static_cast<int>(theIsolineData.itsDefRGBRowSize), XuRGB, max_level);

    XuColorTableCreate(colorTableIndex, classCount, XuLOOKUP, XuRGB, max_level);
    XuColorTableActivate(colorTableIndex);
    XuShadingScaleLoad(shadingScaleIndex, colorTableIndex, 0, classCount + 0);
    checkedVector<int> colorIndices(classCount);
    for(int i = 0; i < classCount; i++)
        colorIndices[i] = i;

    if(colorIndices.size())
        XuShadingColorIndices(&colorIndices[0], classCount);
    XuShadingScale(shadingScaleIndex);
}

static void DoContourUserDraw(NFmiIsoLineData& theIsoLineData)
{
    XuContourUserDraw(theIsoLineData.itsContourUserDrawData.xCoordinates.data(), theIsoLineData.itsContourUserDrawData.yCoordinates.data(), theIsoLineData.itsContourUserDrawData.itsUserDrawValues.data(), theIsoLineData.itsContourUserDrawData.itsYNumber, theIsoLineData.itsContourUserDrawData.itsXNumber);
}

static void DrawCustomColorContours(NFmiIsoLineData &theIsoLineData)
{
    int oldColorTable;
    XuColorTableActiveQuery(&oldColorTable);

    float isolineWidth = 0.f;
    if(theIsoLineData.fUseSeparatorLinesBetweenColorContourClasses)
        isolineWidth = 0.1f;
    XuIsolineWidths(&isolineWidth, 1);
    int isolineColorArr[1] = { 3 }; // 3 = musta
    XuIsolineColors(isolineColorArr, 1);

    XuIsolineSplineSmoothing(0); // ei kannata pyˆrit‰‰ isoviivoja koska contourit eiv‰t kuitenkaan pyˆristy
    if(theIsoLineData.fUseIsoLineGabWithCustomContours == false)
    {
        XuClasses(&theIsoLineData.itsCustomColorContours[0], theIsoLineData.itsTrueColorContoursCount);
        XuShadingColorIndices(&theIsoLineData.itsCustomColorContoursColorIndexies[0], theIsoLineData.itsTrueColorContoursCount + 1);
    }
    else
    {
        if(::IsTransparencyColorUsed(theIsoLineData.itsCustomColorContoursColorIndexies, theIsoLineData.itsColorIndexCount, 2))
            ::InitCustomColorTableWithTransparentColor(theIsoLineData, 6, 2);
        else
        {
            float aMin = theIsoLineData.itsClassMinValue;
            float aMax = theIsoLineData.itsClassMaxValue;
            int classCount = theIsoLineData.itsTrueColorContoursCount;
            checkedVector<int> defaultTableColorIndices(theIsoLineData.itsCustomColorContoursColorIndexies.begin(), theIsoLineData.itsCustomColorContoursColorIndexies.begin() + theIsoLineData.itsColorIndexCount);
            checkedVector<float> colorWidths = ::CalcCustomColorWidths(theIsoLineData);
            ::CreateClassesAndColorTableAndColorShadeForCustomContourWithSteps(aMin, aMax, classCount, defaultTableColorIndices, 5, 3, colorWidths, theIsoLineData);
        }
    }

    if(theIsoLineData.UseContourUserDraw())
        ::DoContourUserDraw(theIsoLineData);
    else if(theIsoLineData.fUseColorContours == 2) // 2=quickcontours
        XuContourQuickDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
    else
        XuContourDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
    XuColorTableActivate(oldColorTable);
}

static void DrawSimpleColorContours(NFmiIsoLineData &theIsoLineData)
{
    int oldColorTable;
    XuColorTableActiveQuery(&oldColorTable);
    float aMin = theIsoLineData.itsClassMinValue;
    float aMax = theIsoLineData.itsClassMaxValue;
    int classCount = theIsoLineData.itsTrueColorContoursCount;
    int colorIndices[s_rgbDefCount] = { theIsoLineData.itsCustomColorContoursColorIndexies[0]
                                        ,theIsoLineData.itsCustomColorContoursColorIndexies[1]
                                        ,theIsoLineData.itsCustomColorContoursColorIndexies[2]
                                        ,theIsoLineData.itsCustomColorContoursColorIndexies[3] };
    float width1 = theIsoLineData.itsCustomColorContours[1] - theIsoLineData.itsCustomColorContours[0];
    float width2 = theIsoLineData.itsCustomColorContours[2] - theIsoLineData.itsCustomColorContours[1];
    float width3 = theIsoLineData.itsCustomColorContours[3] - theIsoLineData.itsCustomColorContours[2];
    float colorWidths[s_rgbDefCount] = { width1, width2, width3, width3 };
    CreateClassesAndColorTableAndColorShade(aMin, aMax, classCount, colorIndices, 7, 3, colorWidths, false);
    float isolineWidth = 0.f;
    if(theIsoLineData.fUseSeparatorLinesBetweenColorContourClasses)
        isolineWidth = 0.1f;
    int isolineColorArr[1] = { 3 }; // 3 = musta
    XuIsolineSplineSmoothing(0); // ei kannata pyˆrist‰‰ isoviivoja koska contourit eiv‰t kuitenkaan pyˆristy
    XuIsolineWidths(&isolineWidth, 1);
    XuIsolineColors(isolineColorArr, 1);

    if(theIsoLineData.UseContourUserDraw())
        ::DoContourUserDraw(theIsoLineData);
    else if(theIsoLineData.fUseColorContours == 2) // 2=quickcontours
        XuContourQuickDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
    else
        XuContourDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
    XuColorTableActivate(oldColorTable);
}

static void FixWorldLimitsWithViewPortSettings(TMWorldLimits &theWorldLimits)
{
    theWorldLimits.x_min = 0;
    theWorldLimits.x_max = 1;
    theWorldLimits.y_min = 0;
    theWorldLimits.y_max = 1;
}

static void MakeTotalPolygonPointConversion(checkedVector<float>& thePolyPointsX, checkedVector<float>& thePolyPointsY, checkedVector<int>& thePolyPointsXInPixels, checkedVector<int>& thePolyPointsYInPixels)
{
    size_t totalPolyPointSize = thePolyPointsX.size();
    if(thePolyPointsXInPixels.size() < totalPolyPointSize)
    {
        thePolyPointsXInPixels.resize(totalPolyPointSize);
        thePolyPointsYInPixels.resize(totalPolyPointSize);
    }
    XuViewWorldToPixel(&thePolyPointsX[0], &thePolyPointsY[0], static_cast<int>(totalPolyPointSize), &thePolyPointsXInPixels[0], &thePolyPointsYInPixels[0]);
}

static BoundingBox GetRowBoundingBox(int theRowNumber, int theGridYSize, const TMWorldLimits &theWorldLimits)
{
    float yDiff = (theWorldLimits.y_max - theWorldLimits.y_min) / static_cast<float>(theGridYSize - 1);
    BoundingBox bBox;
    bBox.bottom = yDiff * (theRowNumber - 1) + theWorldLimits.y_min;
    bBox.top = yDiff + bBox.bottom;
    bBox.left = theWorldLimits.x_min;
    bBox.right = theWorldLimits.x_max;

    return bBox;
}

static bool IsEqual(float value1, float value2, float epsilon)
{
    return (::fabs(value1 - value2) <= epsilon);
}

// Annettua pistett‰ verrataan annettuun bounding-boxiin. Jos joku piste osuu johonkin kulmaan, annetaan sit‰ vastaava arvo.
// Jos piste ei ole kulmassa, palautetaan kNoDirection.
// En viel‰ tied‰ tarvitaanko tarkastelussa float tarkkuudesta johtuvaa virheraja tarkastelua.
static FmiDirection IsCornerPoint(const BoundingBox &theBoundings, const FloatPoint &thePoint, float fUsedEpsilon)
{
    if(::IsEqual(thePoint.x, theBoundings.left, fUsedEpsilon) && ::IsEqual(thePoint.y, theBoundings.top, fUsedEpsilon))
        return kTopLeft;
    if(::IsEqual(thePoint.x, theBoundings.left, fUsedEpsilon) && ::IsEqual(thePoint.y, theBoundings.bottom, fUsedEpsilon))
        return kBottomLeft;
    if(::IsEqual(thePoint.x, theBoundings.right, fUsedEpsilon) && ::IsEqual(thePoint.y, theBoundings.top, fUsedEpsilon))
        return kTopRight;
    if(::IsEqual(thePoint.x, theBoundings.right, fUsedEpsilon) && ::IsEqual(thePoint.y, theBoundings.bottom, fUsedEpsilon))
        return kBottomRight;

    return kNoDirection;
}

static float gHatchLimitCheckEpsilon = 0;

class PointOnHatchLimitException
{
public:
    PointOnHatchLimitException(void)
    {
    }
};

static bool IsIndexedVaueInHatchLimits(int theXIndex, int theYIndex, NFmiIsoLineData &theIsoLineData, const NFmiHatchingSettings &theHatchSettings)
{
    if(theXIndex >= 0 && theYIndex >= 0 && theXIndex < theIsoLineData.itsXNumber && theYIndex < theIsoLineData.itsYNumber)
    {
        float value = theIsoLineData.itsIsolineData[theXIndex][theYIndex];
        if(::IsEqual(value, theHatchSettings.itsHatchLowerLimit, gHatchLimitCheckEpsilon) || ::IsEqual(value, theHatchSettings.itsHatchUpperLimit, gHatchLimitCheckEpsilon) || value == kFloatMissing)
            throw PointOnHatchLimitException();
        return ::IsInsideLimits(theHatchSettings.itsHatchLowerLimit, theHatchSettings.itsHatchUpperLimit, value);
    }
    else
        return false;
}

static const float gUsedEpsilon = 0.00002f;

// Tutkii onko annetut pisteet bounding-boxin yl‰ tai ala reunalla. Jos on, palautetaan vastaava arvo (kTop tai kBottom).
// Jos ei palautetaan kNoDirection.
// Jossain tapauksissa etsint‰ haarukkaa pit‰‰ laventaa yhden pikselin verran (toolmaster feature), sit‰ varten
// parametri theOnePixelOffset, johon tavallisesti annetaan arvo 0.
// HUOM! Jos pisteet ovat samalla janalla, pit‰‰ niiden toinen dimension erota tarpeeksi paljon, t‰t‰ kontrolloidaan theSecondDimensionMinDiff -parametrilla.
static FmiDirection ArePointsOnSameSide(const BoundingBox &theBoundingBox, const FloatPoint &thePoint1, const FloatPoint &thePoint2, float theOnePixelOffset, float theSecondDimensionMinDiff)
{
    float usedEpsilon = gUsedEpsilon + theOnePixelOffset;

    if(::IsEqual(thePoint1.x, theBoundingBox.left, usedEpsilon) && ::IsEqual(thePoint2.x, theBoundingBox.left, usedEpsilon))
    {
        if(::IsEqual(thePoint1.y, thePoint2.y, theSecondDimensionMinDiff) == false) // huom! pisteet eiv‰t saa olla samoja tai liian l‰hell‰ toisiaan toisessa suunnassa (4 merkitt‰v‰n pisteen joukossa on luultavasti 1. ja viimeinen piste samoja)
            return kLeft;
    }
    if(::IsEqual(thePoint1.x, theBoundingBox.right, usedEpsilon) && ::IsEqual(thePoint2.x, theBoundingBox.right, usedEpsilon))
    {
        if(::IsEqual(thePoint1.y, thePoint2.y, theSecondDimensionMinDiff) == false) // huom! pisteet eiv‰t saa olla samoja tai liian l‰hell‰ toisiaan toisessa suunnassa (4 merkitt‰v‰n pisteen joukossa on luultavasti 1. ja viimeinen piste samoja)
            return kRight;
    }

    if(::IsEqual(thePoint1.y, theBoundingBox.top, usedEpsilon) && ::IsEqual(thePoint2.y, theBoundingBox.top, usedEpsilon))
    {
        if(::IsEqual(thePoint1.x, thePoint2.x, theSecondDimensionMinDiff) == false) // huom! pisteet eiv‰t saa olla samoja tai liian l‰hell‰ toisiaan toisessa suunnassa (4 merkitt‰v‰n pisteen joukossa on luultavasti 1. ja viimeinen piste samoja)
            return kTop;
    }
    if(::IsEqual(thePoint1.y, theBoundingBox.bottom, usedEpsilon) && ::IsEqual(thePoint2.y, theBoundingBox.bottom, usedEpsilon))
    {
        if(::IsEqual(thePoint1.x, thePoint2.x, theSecondDimensionMinDiff) == false) // huom! pisteet eiv‰t saa olla samoja tai liian l‰hell‰ toisiaan toisessa suunnassa (4 merkitt‰v‰n pisteen joukossa on luultavasti 1. ja viimeinen piste samoja)
            return kBottom;
    }

    return kNoDirection;
}

static bool MajorityCheck(const std::vector<NFmiPoint> &thePoints, NFmiIsoLineData &theIsoLineData, const NFmiHatchingSettings &theHatchSettings, const NFmiRect &theRelativeRect)
{
    size_t trueCounter = 0;
    size_t missingCounter = 0;
    for(size_t i = 0; i < thePoints.size(); i++)
    {
        float value = theIsoLineData.itsIsolineData.InterpolatedValue(thePoints[i], theRelativeRect, static_cast<FmiParameterName>(theIsoLineData.itsParam.GetParamIdent()), true); // true lopussa tarkoittaa ett‰ y-akselia ei k‰‰nnet‰ (en ymm‰rr‰ miksi muuallla t‰ytyy mutta t‰ss‰ ei?!?!?)
        if(::IsEqual(value, theHatchSettings.itsHatchLowerLimit, gHatchLimitCheckEpsilon) || ::IsEqual(value, theHatchSettings.itsHatchUpperLimit, gHatchLimitCheckEpsilon) || value == kFloatMissing)
            missingCounter++;
        else if(::IsInsideLimits(theHatchSettings.itsHatchLowerLimit, theHatchSettings.itsHatchUpperLimit, value))
            trueCounter++;
    }
    if(missingCounter == 0)
        return (static_cast<double>(trueCounter) / thePoints.size()) > 0.5;
    else if(missingCounter < thePoints.size())
        return (static_cast<double>(trueCounter) / (thePoints.size() - missingCounter)) > 0.5;
    else
        return false;
}

// Etsit‰‰n tarkasteluna olleen polygonin theMinXCoordinate:sta seuraava hila indeksi ja katsotaan onko se hatch-rajojen sis‰ll‰.
static bool IsSidePolygonDrawn(FmiDirection theSideStatus, NFmiIsoLineData &theIsoLineData, const NFmiHatchingSettings &theHatchSettings, const FloatPoint &thePoint1, const FloatPoint &thePoint2, const TMWorldLimits &theWorldLimits, float theOnePixelOffset)
{
    bool verticalLine = (theSideStatus == kLeft) || (theSideStatus == kRight);

    NFmiRect relativeRect(theWorldLimits.x_min, theWorldLimits.y_min, theWorldLimits.x_max, theWorldLimits.y_max);
    float gWorldLimitEpsilon = theOnePixelOffset / 300.f; // maailman rajoja pit‰‰ v‰h‰n laajentaa, koska muuten interpolointi reuna pisteille saattaa palauttaa missing arvon, johtuen laskennan ep‰tarkkuudesta.
    if(theSideStatus == kTop && ::IsEqual(thePoint1.y, theWorldLimits.y_max, gWorldLimitEpsilon * 2))
        relativeRect = NFmiRect(theWorldLimits.x_min - gWorldLimitEpsilon, theWorldLimits.y_min - gWorldLimitEpsilon, theWorldLimits.x_max + gWorldLimitEpsilon, theWorldLimits.y_max + gWorldLimitEpsilon);

    // etsit‰‰n kolmen pisteen avulla enemmistˆ p‰‰tˆs piirrolle
    std::vector<NFmiPoint> points;
    if(verticalLine)
    {
        NFmiPoint point(thePoint1.x, (thePoint1.y + thePoint2.y) / 2.);
        points.push_back(point);
        double yDiff = (thePoint1.y - thePoint2.y) / 3.;
        point.Y(point.Y() - yDiff); // t‰m‰ 1/3 polygonin sis‰lt‰
        points.push_back(point);
        point.Y(point.Y() + 2 * yDiff); // t‰m‰ on 2/3 polygonin sis‰lt‰
        points.push_back(point);
    }
    else
    {
        NFmiPoint point((thePoint1.x + thePoint2.x) / 2., thePoint1.y);
        points.push_back(point);
        double xDiff = (thePoint1.x - thePoint2.x) / 3.;
        point.X(point.X() - xDiff); // t‰m‰ 1/3 polygonin sis‰lt‰
        points.push_back(point);
        point.X(point.X() + 2 * xDiff); // t‰m‰ on 2/3 polygonin sis‰lt‰
        points.push_back(point);
    }
    return ::MajorityCheck(points, theIsoLineData, theHatchSettings, relativeRect);
}

// Etsii annetun pistejoukon l‰pi ja tutkii sit‰ annettua bounding boxia vasten.
// Tiedot kulmapisteist‰ talletetaan vektoriin jossa tieto kunkin pisteen asemasta tai ett‰ kyse ei ole kulmapisteest‰.
// Palauttaa true, jos lˆytyi yksikin kulmapiste ja false jos ei lˆytynyt yht‰‰n.
static bool FindCornerPoints(const BoundingBox &theBoundings, int thePolygonPointTotalOffset, int thePolygonPointSize, checkedVector<float> &thePolyPointsX, checkedVector<float> &thePolyPointsY, checkedVector<FmiDirection> &thePointInfoOut, float fUsedEpsilon)
{
    thePointInfoOut.resize(thePolygonPointSize); // varmistetaan ett‰ tulos vektorissa on sama koko
    bool status = false;
    for(size_t i = 0; i < thePolygonPointSize; i++)
    {
        FloatPoint pt;
        pt.x = thePolyPointsX[thePolygonPointTotalOffset + i];
        pt.y = thePolyPointsY[thePolygonPointTotalOffset + i];
        FmiDirection result = ::IsCornerPoint(theBoundings, pt, fUsedEpsilon);
        thePointInfoOut[i] = result;
        if(result != kNoDirection)
            status = true;
    }

    return status;
}

// Jos lˆytyi kulma piste, voimme p‰‰tell‰ sen indeksin hilassa ja vertailla kyseisess‰ kulmapisteess‰ olevaa arvoa
// suhteessa hatch-rajoihin. Jos se on rajojen sis‰ll‰, polygoni piirret‰‰n.
static bool IsCornerPolygonDrawn(int theTmRowNumber, NFmiIsoLineData &theIsoLineData, const NFmiHatchingSettings &theHatchSettings, const checkedVector<FmiDirection> &thePointsInfo)
{
    for(size_t i = 0; i < thePointsInfo.size(); i++)
    {
        FmiDirection cornerStatus = thePointsInfo[i];
        if(cornerStatus != kNoDirection)
        {
            int xIndex = -1;
            int yIndex = -1;
            if(cornerStatus == kTopLeft)
            {
                xIndex = 0;
                yIndex = theTmRowNumber;
            }
            else if(cornerStatus == kBottomLeft)
            {
                xIndex = 0;
                yIndex = theTmRowNumber - 1;
            }
            else if(cornerStatus == kTopRight)
            {
                xIndex = theIsoLineData.itsXNumber - 1;
                yIndex = theTmRowNumber;
            }
            else if(cornerStatus == kBottomRight)
            {
                xIndex = theIsoLineData.itsXNumber - 1;
                yIndex = theTmRowNumber - 1;
            }

            try
            {
                return ::IsIndexedVaueInHatchLimits(xIndex, yIndex, theIsoLineData, theHatchSettings);
            }
            catch(PointOnHatchLimitException & /* excep */)
            {
            }
        }
    }
    throw PointOnHatchLimitException();
}

static const float gCornerSeekEpsilonFactor = 15000.f;

static bool IsPolygonDrawnSlowCheck(NFmiIsoLineData &theIsoLineData, int theRowNumber, int thePolygonPointTotalOffset, int thePolygonPointSize, checkedVector<float> &thePolyPointsX, checkedVector<float> &thePolyPointsY, const NFmiHatchingSettings &theHatchSettings, const TMWorldLimits &theWorldLimits, float theOnePixelOffset, const BoundingBox &theBoundingBox)
{
    // K‰yd‰‰n pisteet l‰pi ja etsit‰‰n ensin edustavia kulmapisteit‰
    try
    {
        float cornerSeekEpsilon = theOnePixelOffset / gCornerSeekEpsilonFactor;
        checkedVector<FmiDirection> pointInfo;
        if(::FindCornerPoints(theBoundingBox, thePolygonPointTotalOffset, thePolygonPointSize, thePolyPointsX, thePolyPointsY, pointInfo, cornerSeekEpsilon))
            return ::IsCornerPolygonDrawn(theRowNumber, theIsoLineData, theHatchSettings, pointInfo);
    }
    catch(PointOnHatchLimitException & /* excep */)
    {
    }

    // k‰yd‰‰n l‰pi pisteet ja etsit‰‰n edustavia reuna janoja otetaan tietoja talteen (1. pisteen indeksi, reunatieto, pituus)
    // Jos jana on tarpeeksi pitk‰, sit‰ testataan heti. Lopuksi janat laitetaan pituus j‰rjestykseen ja kokeillaan siin‰ j‰rjestyksess‰.
    std::vector<PolygonLineInfo> polygonLineInfos;
    FloatPoint p1;
    p1.x = thePolyPointsX[thePolygonPointTotalOffset];
    p1.y = thePolyPointsY[thePolygonPointTotalOffset];
    FloatPoint p2;
    float minSecondDimensionDiff = theOnePixelOffset * 4; // t‰ss‰ minimi pituus, mink‰ viivan pit‰‰ olla
    float criticalLineLen = minSecondDimensionDiff * 8;
    for(size_t i = thePolygonPointTotalOffset + 1; i < thePolygonPointTotalOffset + thePolygonPointSize; i++)
    {
        p2.x = thePolyPointsX[i];
        p2.y = thePolyPointsY[i];
        try
        {
            // sitten tarkastellaan onko pisteet samalla reuna janalla pienell‰-toleranssilla
            FmiDirection sideStatus = ::ArePointsOnSameSide(theBoundingBox, p1, p2, 0, minSecondDimensionDiff);
            float lineLen = ::CalcLineLen(p1, p2);
            if(lineLen >= criticalLineLen && sideStatus == kNoDirection) // tarpeeksi pitkille piste v‰leille kokeillaan heti isompaa toleranssia, jos viivaa ei tulkittu janaksi
                sideStatus = ::ArePointsOnSameSide(theBoundingBox, p1, p2, theOnePixelOffset, minSecondDimensionDiff);

            PolygonLineInfo lineInfo;
            lineInfo.itsSideStatus = sideStatus;
            lineInfo.itsSecondPointIndex = i;
            lineInfo.itsLineLength = lineLen;
            polygonLineInfos.push_back(lineInfo);
        }
        catch(PointOnHatchLimitException & /* excep */)
        {
        }
        p1 = p2;
    }

    // nyt on kriittisen pituuden omaavat viivat tarkasteltu, lopuksi tutkitaan loputkin viivat pituus j‰rjestyksess‰
    std::sort(polygonLineInfos.begin(), polygonLineInfos.end(), IsLineLonger);
    for(size_t j = 0; j < polygonLineInfos.size(); j++)
    {
        p1.x = thePolyPointsX[polygonLineInfos[j].itsSecondPointIndex - 1];
        p1.y = thePolyPointsY[polygonLineInfos[j].itsSecondPointIndex - 1];
        p2.x = thePolyPointsX[polygonLineInfos[j].itsSecondPointIndex];
        p2.y = thePolyPointsY[polygonLineInfos[j].itsSecondPointIndex];
        try
        {
            if(polygonLineInfos[j].itsSideStatus == kNoDirection)
            {
                FmiDirection sideStatus = ::ArePointsOnSameSide(theBoundingBox, p1, p2, theOnePixelOffset, minSecondDimensionDiff); // jos ei oltu tulkittu janaksi, tarkastellaan viel‰ pienell‰ toleranssilla
                if(sideStatus != kNoDirection)
                {
                    return ::IsSidePolygonDrawn(sideStatus, theIsoLineData, theHatchSettings, p1, p2, theWorldLimits, theOnePixelOffset);
                }
            }
            else
            {
                return ::IsSidePolygonDrawn(polygonLineInfos[j].itsSideStatus, theIsoLineData, theHatchSettings, p1, p2, theWorldLimits, theOnePixelOffset);
            }
        }
        catch(PointOnHatchLimitException & /* excep */)
        {
        }
    }

    return false; // koko polygoni k‰ytiin l‰pi eik‰ ratkaisua lˆytynyt, ei piirret‰ TODO viel‰ lˆytyy tarkasteluja
}

static bool IsPolygonDrawn(NFmiIsoLineData &theIsoLineData, int theRowNumber, int thePolygonPointTotalOffset, int thePolygonPointSize, checkedVector<float> &thePolyPointsX, checkedVector<float> &thePolyPointsY, const NFmiHatchingSettings &theHatchSettings, const TMWorldLimits &theWorldLimits, float theOnePixelOffset)
{
    BoundingBox boundingBox = ::GetRowBoundingBox(theRowNumber, theIsoLineData.itsYNumber, theWorldLimits);
    // Nyt k‰yd‰‰n koko polygoni l‰pi piste kerrallaan ja katsotaan piirret‰‰nkˆ se.
    return ::IsPolygonDrawnSlowCheck(theIsoLineData, theRowNumber, thePolygonPointTotalOffset, thePolygonPointSize, thePolyPointsX, thePolyPointsY, theHatchSettings, theWorldLimits, theOnePixelOffset, boundingBox);
}

#ifdef min // win32 makrot s‰hl‰‰v‰t std-min ja max:ien k‰ytˆn, ne pit‰‰ 'undefinoida'
#undef min
#endif
#ifdef max // win32 makrot s‰hl‰‰v‰t std-min ja max:ien k‰ytˆn, ne pit‰‰ 'undefinoida'
#undef max
#endif

// jos annetut ovat -99999 arvoisia, ignoorataan min ja max y-pixel arvojen k‰sittely
static const int gIgnoreMinMaxPixelHandling = -99999;

static void FillHatchedPolygonData(checkedVector<int>& thePolyPointsXInPixels, checkedVector<int>& thePolyPointsYInPixels, int theTotalPolyPointCounter, int thePolyPointCount, checkedVector<CPoint> & thePolygonCPoints, int &theMinRowYPixelValue, int &theMaxRowYPixelValue)
{
    bool doPolygonNarrowFix = (theMinRowYPixelValue != gIgnoreMinMaxPixelHandling) && (theMaxRowYPixelValue != gIgnoreMinMaxPixelHandling);

    if(doPolygonNarrowFix)
    {
        checkedVector<int>::iterator startIt = (thePolyPointsYInPixels.begin() + theTotalPolyPointCounter);
        checkedVector<int>::iterator endIt = (startIt + thePolyPointCount);
        checkedVector<int>::iterator itMin(std::min_element(startIt, endIt));
        int minYpolygonValue = *itMin;
        theMinRowYPixelValue = std::min(theMinRowYPixelValue, minYpolygonValue);
        checkedVector<int>::iterator itMax(std::max_element(startIt, endIt));
        int maxYpolygonValue = *itMax;
        theMaxRowYPixelValue = std::max(theMaxRowYPixelValue, maxYpolygonValue);
    }
    bool doPolygonWideFix = (theMaxRowYPixelValue - theMinRowYPixelValue) >= 3;
    doPolygonNarrowFix = (theMaxRowYPixelValue - theMinRowYPixelValue) >= 2;
    thePolygonCPoints.resize(thePolyPointCount);
    int suggestedFixPointIndex = -1;
    int maxXPixelLen = -9999; // t‰h‰n haetaan l‰hell‰ pohjareunaa olevien pisteiden et‰isyytt‰ eteen tai taaksep‰in
    for(int i = 0; i < thePolyPointCount; i++)
    {
        thePolygonCPoints[i].x = thePolyPointsXInPixels[theTotalPolyPointCounter + i];
        int yValue = thePolyPointsYInPixels[theTotalPolyPointCounter + i];
        thePolygonCPoints[i].y = yValue;

        if(doPolygonNarrowFix)
        {
            if(yValue == theMinRowYPixelValue + 1) // Toolmasterin piirre, joskus polygonien alaosa ei ole samalla tasolla ja j‰‰ hatchiin rako, ei jos ollaan boundingboxin alaosassa, lis‰t‰‰n polygonin siihen reunaan, joke on yhden pikselin koholla yksi pikseli, ett‰ saataisiin mahd. 1 pikselin raot umpeen
            {
                if(i < thePolyPointCount - 1)
                { // tarkastellaan x-et‰isyys seuraavaan pisteeseen
                    int currentXLen = ::abs(thePolygonCPoints[i].x - thePolyPointsXInPixels[theTotalPolyPointCounter + i + 1]);
                    int nextPointY = thePolyPointsYInPixels[theTotalPolyPointCounter + i + 1];
                    if(currentXLen > maxXPixelLen && nextPointY == theMinRowYPixelValue)
                    {
                        maxXPixelLen = currentXLen;
                        suggestedFixPointIndex = i;
                    }
                }
                if(i > 0)
                { // tarkastellaan x-et‰isyys edelliseen pisteeseen
                    int currentXLen = ::abs(thePolygonCPoints[i].x - thePolyPointsXInPixels[theTotalPolyPointCounter + i - 1]);
                    int prevPointY = thePolyPointsYInPixels[theTotalPolyPointCounter + i - 1];
                    if(currentXLen > maxXPixelLen && prevPointY == theMinRowYPixelValue)
                    {
                        maxXPixelLen = currentXLen;
                        suggestedFixPointIndex = i;
                    }
                }
            }
        }

        if(doPolygonWideFix)
        {
            if(yValue == theMinRowYPixelValue + 2) // Toolmasterin piirre, joskus polygonien alaosa ei ole samalla tasolla ja j‰‰ hatchiin rako, ei jos ollaan boundingboxin alaosassa, lis‰t‰‰n polygonin siihen reunaan, joke on yhden pikselin koholla yksi pikseli, ett‰ saataisiin mahd. 1 pikselin raot umpeen
            {
                if(i < thePolyPointCount - 1)
                { // tarkastellaan x-et‰isyys seuraavaan pisteeseen
                    int currentXLen = ::abs(thePolygonCPoints[i].x - thePolyPointsXInPixels[theTotalPolyPointCounter + i + 1]);
                    int nextPointY = thePolyPointsYInPixels[theTotalPolyPointCounter + i + 1];
                    if(currentXLen > maxXPixelLen && nextPointY == theMinRowYPixelValue)
                    {
                        maxXPixelLen = currentXLen;
                        suggestedFixPointIndex = i;
                    }
                }
                if(i > 0)
                { // tarkastellaan x-et‰isyys edelliseen pisteeseen
                    int currentXLen = ::abs(thePolygonCPoints[i].x - thePolyPointsXInPixels[theTotalPolyPointCounter + i - 1]);
                    int prevPointY = thePolyPointsYInPixels[theTotalPolyPointCounter + i - 1];
                    if(currentXLen > maxXPixelLen && prevPointY == theMinRowYPixelValue)
                    {
                        maxXPixelLen = currentXLen;
                        suggestedFixPointIndex = i;
                    }
                }
            }
        }
    }
    if(suggestedFixPointIndex >= 0)
        thePolygonCPoints[suggestedFixPointIndex].y = theMinRowYPixelValue; // siirret‰‰n pistett‰, joka on ollut kahden pikselin p‰‰ss‰ pohjasta, jos pohja pisteit‰ ei ole lˆytynyt kuin yksi
}

static void DrawShadedPolygons4(NFmiIsoLineData &theIsoLineData, const NFmiHatchingSettings &theHatchSettings, CDC *pDC, checkedVector<int> &thePolyNumbers, checkedVector<float> &thePolyPointsX, checkedVector<float> &thePolyPointsY, checkedVector<int>& thePolyNumIntValues, checkedVector<int>& thePolyIntValues, const CRect& theMfcClipRect)
{
    if(thePolyNumbers.size())
    {
        checkedVector<int> polyPointsXInPixels, polyPointsYInPixels;
        checkedVector<CPoint> polygonCPoints;

        TMWorldLimits worldLimits;
        XuViewWorldLimitsQuery(&worldLimits.x_min, &worldLimits.x_max, &worldLimits.y_min, &worldLimits.y_max, &worldLimits.z_min, &worldLimits.z_max);
        float xoff, yoff, xsize, ysize;
        XuViewportQuery(&xoff, &yoff, &xsize, &ysize);
        ::FixWorldLimitsWithViewPortSettings(worldLimits);
        float onePixelOffsetY = 1.5f * (worldLimits.y_max - worldLimits.y_min) / theMfcClipRect.Height(); // t‰t‰ kerroint‰ k‰ytet‰‰n mm. erilaisten virhe-epsilonien laskuissa

        MakeTotalPolygonPointConversion(thePolyPointsX, thePolyPointsY, polyPointsXInPixels, polyPointsYInPixels);
        int polygonPointTotalCount = 0;
        int polygonRefIntTotalCount = 0;

        int minRowYPixelValue = 99999999; // t‰h‰n haetaan aina joka rivilt‰ minimi y-arvo, ett‰ voidaan hakea yhden tai kahden pikselin heittoja pois, mitk‰ toolmaster pistee joskus laskemiinsa polygoneihin
        int maxRowYPixelValue = -99999999;
        int lastRowNumber = -999;

        for(size_t i = 0; i < thePolyNumbers.size(); i++)
        {
            int polPoinsCount = thePolyNumbers[i];
            int refIntValuesCount = thePolyNumIntValues[i];

            int currentRowNumber = thePolyIntValues[polygonRefIntTotalCount + 1]; // otetaan toolmasterin nime‰m‰ hila-rivinumero talteen (alkaa 1:st‰ ja esim. 1:n kertoo ett‰ ollaan kosketuksissa riveihin 0 ja 1, jolloin 0 on ala- ja 1 on yl‰reuna)
            if(currentRowNumber != lastRowNumber)
            {
                if(lastRowNumber >= 0) // 1. kerralla (kun lastRow on -999) ei anneta max-arvoa min-arvolle
                {
                    minRowYPixelValue = maxRowYPixelValue; // kun polygoni rivi vaihtuu, pit‰‰ vaihtaa min-pixel y arvo vanhaan max arvoon, vanha x-arvo pysyy samana
                    maxRowYPixelValue = -99999999; // laitetaan max aina 'resettiin' kun rivi vaihtuu
                }
            }

            if(::IsPolygonDrawn(theIsoLineData, currentRowNumber, polygonPointTotalCount, polPoinsCount, thePolyPointsX, thePolyPointsY, theHatchSettings, worldLimits, onePixelOffsetY))
            {
                FillHatchedPolygonData(polyPointsXInPixels, polyPointsYInPixels, polygonPointTotalCount, polPoinsCount, polygonCPoints, minRowYPixelValue, maxRowYPixelValue);
                pDC->Polygon(&polygonCPoints[0], polPoinsCount);
            }

            polygonPointTotalCount += polPoinsCount;
            polygonRefIntTotalCount += refIntValuesCount;
            lastRowNumber = currentRowNumber;
        }
    }
}

static void SetupMFCAndDrawShadedPolygons3(NFmiIsoLineData &theIsoLineData, const NFmiHatchingSettings& theHatchSettings, CDC* pDC, checkedVector<int>& thePolyNumbers, checkedVector<float>& thePolyPointsX, checkedVector<float>& thePolyPointsY, checkedVector<int>& thePolyNumIntValues, checkedVector<int>& thePolyIntValues, const CRect& theMfcClipRect)
{
    int oldBkMode = pDC->SetBkMode(TRANSPARENT);
    CBrush brush;
    COLORREF crColor = theHatchSettings.itsHatchColorRef;
    if(theHatchSettings.itsHatchPattern == -1) // jos hatch-pattern on -1, tehd‰‰nkin t‰ysin peitt‰v‰ sivelline ilman hatchi‰
        brush.CreateSolidBrush(crColor);
    else
        brush.CreateHatchBrush(theHatchSettings.itsHatchPattern, crColor);
    CBrush *oldBrush = pDC->SelectObject(&brush);

    int penStyle = PS_NULL;
    int penWidth = theHatchSettings.fDrawHatchBorders ? 1 : 0;
    COLORREF penColor1 = 0x00000000;
    if(penWidth > 0)
        penColor1 = crColor;
    CPen myPen(penStyle, penWidth, penColor1);
    CPen *oldPen = pDC->SelectObject(&myPen);

    ::DrawShadedPolygons4(theIsoLineData, theHatchSettings, pDC, thePolyNumbers, thePolyPointsX, thePolyPointsY, thePolyNumIntValues, thePolyIntValues, theMfcClipRect);

    pDC->SelectObject(oldBrush);
    brush.DeleteObject();
    pDC->SetBkMode(oldBkMode);

    pDC->SelectObject(oldPen);
}

// T‰m‰n testi funktion toteutus on tiedoston lopussa.
static void DrawShadedPolygonsTest(CDC *pDC, checkedVector<int> & thePolyNumbers, checkedVector<float> &thePolyPointsX, checkedVector<float> &thePolyPointsY);

bool gDrawTest = false; // Laita t‰m‰n arvoksi true, jos haluat n‰hd‰ tietyn piirrett‰n polygonin high-lightauksen.

static void GetReadyAndDoTheHatch(NFmiIsoLineData &theIsoLineData, const NFmiHatchingSettings& theHatchSettings, CDC* pDC, const CRect& theMfcClipRect, int hatchIndex)
{
    static checkedVector<int> polyNumbers;
    static checkedVector<float> polyPointsX;
    static checkedVector<float> polyPointsY;
    static checkedVector<int> polyNumIntValues;
    static checkedVector<int> polyIntValues;
    static checkedVector<float> hatchClassValues(2);

    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, std::string("Drawing hatch ") + std::to_string(hatchIndex) + " for " + ::MakeDataIdentString(theIsoLineData.itsParam));
    XuViewWorld();

    XuIsolineSplineSmoothing(1);

    hatchClassValues[0] = theHatchSettings.itsHatchLowerLimit;
    hatchClassValues[1] = theHatchSettings.itsHatchUpperLimit;
    XuClasses(&hatchClassValues[0], 2);

    // onko t‰m‰ turha????
    XuShadingColorIndices(&theIsoLineData.itsCustomColorContoursColorIndexies[0], 3);

    float lineWidth = 0.f;
    XuIsolineWidths(&lineWidth, 1); // ei piirret‰ isoviivoja sheidauksen yhteydess‰
    XuMapDrawOptions(XuPROBE); // piirret‰‰n polygonit erikseen toisella ohjelmalla, ett‰ saadaan hatch kuviot mukaan
    XuContourDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);

    int mapComponent = Xu2D_CONTOUR;//Xu2D_ISOLINE;//CONTOUR;
    int polNumber = 0;
    int polSize = 0;
    XuMapPolygonsNumberSize(mapComponent, &polNumber, &polSize);

    if(polNumber == 0 || polSize == 0)
        return; // ei jatketa, jos ei polygoneja lˆytynyt

    polyNumbers.resize(polNumber);
    polyPointsX.resize(polSize);
    polyPointsY.resize(polSize);

    XuMapPolygonsQuery(mapComponent, &polyNumbers[0], &polyPointsX[0], &polyPointsY[0]);

    int polNumber2 = 0;
    int polIntegerDataSize = 0;
    int polFloatDataSize = 0;
    XuMapPolygonsDataSize(mapComponent, &polNumber2, &polIntegerDataSize, &polFloatDataSize);

    if(polNumber2 == 0)
        return; // ei jatketa, jos ei polygoneja lˆytynyt

    polyNumIntValues.resize(polNumber2);
    polyIntValues.resize(polIntegerDataSize);
    XuMapPolygonsDataIntQuery(mapComponent, &polyNumIntValues[0], &polyIntValues[0]);

    CRgn rgn;
    CRect rectClip(theMfcClipRect);
    rgn.CreateRectRgn(rectClip.left, rectClip.top,
        rectClip.right, rectClip.bottom);
    pDC->SelectClipRgn(&rgn);

    try
    {
        gHatchLimitCheckEpsilon = ::fabs((theHatchSettings.itsHatchLowerLimit + theHatchSettings.itsHatchUpperLimit) / (2.f * 100000.f));
        ::SetupMFCAndDrawShadedPolygons3(theIsoLineData, theHatchSettings, pDC, polyNumbers, polyPointsX, polyPointsY, polyNumIntValues, polyIntValues, theMfcClipRect);
        if(gDrawTest)
            DrawShadedPolygonsTest(pDC, polyNumbers, polyPointsX, polyPointsY);
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

void DrawCustomIsoLines(NFmiIsoLineData &theIsoLineData, float theViewHeight, int theCrossSectionIsoLineDrawIndex, float scaleFactor)
{
    int oldColorTable;
    XuColorTableActiveQuery(&oldColorTable);

    // HUOM! ToolMaster bugi, kun piirret‰‰n contour+isoviiva (aiemmin tehty XuClassesMinMax-kutsu jyr‰‰ nyt teht‰v‰n XuClasses-kutsun)
//	int classes1 = 0;
//	XuClassesNumberQuery(&classes1);
//	XuClassesMinMax(0, 1, 1); // yritet‰‰n nollata classes asetuksia, koska ne voivat j‰‰d‰ p‰‰lle contour+isoline piirrossa
//	XuClassesStartStep(1000, 5, 2);
//	int classes2 = 0;
//	XuClassesNumberQuery(&classes2);
    XuClasses(&theIsoLineData.itsCustomIsoLineClasses[0], theIsoLineData.itsTrueIsoLineCount);
    //	int classes3 = 0;
    //	XuClassesNumberQuery(&classes3);

    int i = 0;
    for(i = 0; i < theIsoLineData.itsTrueIsoLineCount; i++)
        theIsoLineData.itsIsoLineWidth[i] *= scaleFactor;
    XuIsolineWidths(&theIsoLineData.itsIsoLineWidth[0], theIsoLineData.itsTrueIsoLineCount);
    XuIsolineColors(&theIsoLineData.itsIsoLineColor[0], theIsoLineData.itsTrueIsoLineCount);
    XuIsolineSplineSmoothing(theIsoLineData.itsIsoLineSplineSmoothingFactor);

    double usedGraceFactor = ::CalcUsedGraceFactor(theCrossSectionIsoLineDrawIndex);
    // isoline label strategioita (XuOFF, XuSIMPLE, XuCURVATURE_CHECK, XuOVERLAP_CHECK, XuF_IGNORE)
    XuIsolineLabel(3, theIsoLineData.itsIsoLineLabelDecimalsCount, theViewHeight * usedGraceFactor, XuOVERLAP_CHECK);
    XuIsolineLabelAngle(55, -55);

    // 2=hollowcolorIndex
    float labelBoxWidth = 0;
    if(theIsoLineData.fUseLabelBox)
        labelBoxWidth = 0.15f;
    XuIsolineLabelBoxAttr(theIsoLineData.itsIsoLineBoxFillColorIndex, theIsoLineData.itsIsoLineLabelColor[0], 0, labelBoxWidth);

    for(i = 0; i < theIsoLineData.itsTrueIsoLineCount; i++)
        theIsoLineData.itsIsoLineAnnonationHeight[i] *= scaleFactor;
    XuIsolineLabelHeight(&theIsoLineData.itsIsoLineAnnonationHeight[0], theIsoLineData.itsTrueIsoLineCount);
    XuIsolineStyles(&theIsoLineData.itsIsoLineStyle[0], theIsoLineData.itsTrueIsoLineCount);

    if(theIsoLineData.fUseIsoLineFeathering)
        XuIsolineFeathering(0.8 * scaleFactor, 2 * scaleFactor); // 1.= min dist. in mm 2.= min displ. feath. isoline in mm
    else
        XuIsolineFeathering(0, 0); // 1.= min dist. in mm 2.= min displ. feath. isoline in mm

    // en tied‰ muuta tapaa muuttaa label-tekstin v‰ri‰
    XuTextColor(theIsoLineData.itsIsoLineLabelColor[0], theIsoLineData.itsIsoLineLabelColor[0]);
    XuIsolineDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
}

void DrawSimpleIsoLines(NFmiIsoLineData &theIsoLineData, float theViewHeight, int theCrossSectionIsoLineDrawIndex, float scaleFactor)
{
    XuIsolineSplineSmoothing(theIsoLineData.itsIsoLineSplineSmoothingFactor);
    XuClassesStartStep(theIsoLineData.itsIsoLineStartClassValue, theIsoLineData.itsIsoLineStep, theIsoLineData.itsTrueIsoLineCount);

    theIsoLineData.itsIsoLineWidth[0] *= scaleFactor;
    // Ongelma Arome datan kanssa t‰st‰ lent‰‰ poikkeus ja toolmaster piirto ei en‰‰ toimi
    float isolineWidth = theIsoLineData.itsIsoLineWidth[0];
    XuIsolineWidths(&isolineWidth, 1);
    if(theIsoLineData.fUseSingleColorsWithSimpleIsoLines)
        XuIsolineColors(&theIsoLineData.itsIsoLineColor[0], 1);
    else
        XuIsolineColors(&theIsoLineData.itsIsoLineColor[0], theIsoLineData.itsTrueIsoLineCount);

    XuIsolineStyles(&theIsoLineData.itsIsoLineStyle[0], 1);
    // 2=hollowcolorIndex
    float labelBoxWidth = 0;
    if(theIsoLineData.fUseLabelBox)
        labelBoxWidth = 0.15f;
    XuIsolineLabelBoxAttr(theIsoLineData.itsIsoLineBoxFillColorIndex, theIsoLineData.itsIsoLineLabelColor[0], 0, labelBoxWidth);

    // tee label height strategiasta jotenkin riippuvainen ruudun kokoon millimetreiss‰!!
    float labelHeight = theIsoLineData.itsIsoLineAnnonationHeight[0] * scaleFactor;

    double usedGraceFactor = ::CalcUsedGraceFactor(theCrossSectionIsoLineDrawIndex);
    // isoline label strategioita (XuOFF, XuSIMPLE, XuCURVATURE_CHECK, XuOVERLAP_CHECK, XuF_IGNORE)
    XuIsolineLabel(labelHeight, theIsoLineData.itsIsoLineLabelDecimalsCount, theViewHeight * usedGraceFactor, XuOVERLAP_CHECK);
    XuIsolineLabelAngle(55, -55);
    // label height pit‰‰ kuitenkin s‰‰t‰‰ t‰ss‰ 1. viivalle
    XuIsolineLabelHeight(&labelHeight, 1);

    if(theIsoLineData.fUseIsoLineFeathering)
        XuIsolineFeathering(0.8 * scaleFactor, 2 * scaleFactor); // 1.= min dist. in mm 2.= min displ. feath. isoline in mm
    else
        XuIsolineFeathering(0, 0); // 1.= min dist. in mm 2.= min displ. feath. isoline in mm

    // en tied‰ muuta tapaa muuttaa label-tekstin v‰ri‰
    XuTextColor(theIsoLineData.itsIsoLineLabelColor[0], theIsoLineData.itsIsoLineLabelColor[0]);
    XuIsolineDraw(&theIsoLineData.itsVectorFloatGridData[0], theIsoLineData.itsYNumber, theIsoLineData.itsXNumber);
}

// T‰ytet‰‰n osittainen hila data originaali datasta annettujen alkuindeksien ja tulos hilan koon avulla.
static void FillPartialGridData(NFmiIsoLineData &theOrigIsoLineData, NFmiIsoLineData &thePartialIsoLineDataOut, const IntPoint &theStartIndex)
{
    // 1. t‰ytet‰‰n NFmiDataMatrix-osio
    thePartialIsoLineDataOut.itsIsolineData.Resize(thePartialIsoLineDataOut.itsXNumber, thePartialIsoLineDataOut.itsYNumber, kFloatMissing);
    int nx = static_cast<int>(thePartialIsoLineDataOut.itsIsolineData.NX());
    int ny = static_cast<int>(thePartialIsoLineDataOut.itsIsolineData.NY());
    for(int j = 0; j < ny; j++)
    {
        for(int i = 0; i < nx; i++)
        {
            thePartialIsoLineDataOut.itsIsolineData[i][j] = theOrigIsoLineData.itsIsolineData[theStartIndex.x + i][theStartIndex.y + j];
        }
    }

    // 2. t‰ytet‰‰n sitten toolmasterin k‰ytt‰m‰ float-vektori -osio
    for(int j = 0; j < ny; j++)
    {
        for(int i = 0; i < nx; i++)
        {
            thePartialIsoLineDataOut.itsVectorFloatGridData[j * nx + i] = thePartialIsoLineDataOut.itsIsolineData[i][j];
        }
    }
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
    static checkedVector<float> gridNodesX;
    static checkedVector<float> gridNodesY;

    CRect mfcClipRect;
    NFmiPoint totalViewSize;

    SetupViewWorld(theIsoLineData, theRelViewRect, theZoomedViewRect, theGridArea, gridNodesX, gridNodesY, &mfcClipRect, totalViewSize);
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

static IntPoint CalcNeededSubGridSize(const NFmiIsoLineData &theOrigIsoLineData)
{
    const double criticalXSize = 220;
    const double criticalYSize = 220;

    IntPoint neededSize;
    neededSize.x = static_cast<int>(std::ceil(theOrigIsoLineData.itsXNumber / criticalXSize));
    neededSize.y = static_cast<int>(std::ceil(theOrigIsoLineData.itsYNumber / criticalYSize));
    return neededSize;
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

static void CalcDownSizedMatrix(const NFmiDataMatrix<float> &theOrigData, NFmiDataMatrix<float> &theDownSizedData, FmiParameterName theParamId)
{
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
            theDownSizedData[i][j] = theOrigData.InterpolatedValue(pt, theParamId, true);
        }
    }
}

static void BuildDownSizedData(NFmiIsoLineData &theOrigIsoLineData, NFmiIsoLineData &theDownSizedIsoLineData, const NFmiPoint &theDownSizeFactor)
{
    int newSizeX = boost::math::iround(theOrigIsoLineData.itsXNumber / theDownSizeFactor.X());
    int newSizeY = boost::math::iround(theOrigIsoLineData.itsYNumber / theDownSizeFactor.Y());
    // T‰ytet‰‰n uuden isolineDatan hila-arvot halutuille osaalueilleen.
    NFmiDataMatrix<float> downSizedGridData(newSizeX, newSizeY, kFloatMissing);
    ::CalcDownSizedMatrix(theOrigIsoLineData.itsIsolineData, downSizedGridData, static_cast<FmiParameterName>(theOrigIsoLineData.itsParam.GetParamIdent()));
    theDownSizedIsoLineData.Init(downSizedGridData, theOrigIsoLineData.itsMaxAllowedIsoLineCount);
    // Alustetaan uuden isoline datan piirtoasetukset originaalista
    theDownSizedIsoLineData.InitDrawOptions(theOrigIsoLineData);
}

static void DoTraceLogging(const std::string message)
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

// luokat asetetaan ja luodaan uusi colortable ja asetetaan se aktiiviseksi ja tehd‰‰n haluttu
// color shade taulu
// HUOM!! ota talteen vanha colortable indeksi ennen kutsua!!
void CreateClassesAndColorTableAndColorShade(float aMin, float aMax, int classCount, int defaultTableColorIndices[s_rgbDefCount], int shadingScaleIndex, int colorTableIndex, float colorWidths[s_rgbDefCount], bool fDoIsolines)
{
    //								et‰isyys,  R, G, B
    float defRGB[s_rgbDefCount][4] = { colorWidths[0],	   0, 0, 0, // 1. arvo kertoo v‰rien et‰isyyden
                                        colorWidths[1],	   0, 0, 0,
                                        colorWidths[2],	   0, 0, 0,
                                        colorWidths[3],	   0, 0, 0 };
    float colorRGB[3], hatch[5];
    // v‰ri indeksit talletettu 3 ensimm‰iseen customcolorcontourindex tauluun!!!!
// **************** low value color *****************
    XuColorQuery(defaultTableColorIndices[0], colorRGB, hatch);
    defRGB[0][1] = colorRGB[0];
    defRGB[0][2] = colorRGB[1];
    defRGB[0][3] = colorRGB[2];

    // **************** mid value color *****************
    XuColorQuery(defaultTableColorIndices[1], colorRGB, hatch);
    defRGB[1][1] = colorRGB[0];
    defRGB[1][2] = colorRGB[1];
    defRGB[1][3] = colorRGB[2];

    // **************** high value color *****************
    XuColorQuery(defaultTableColorIndices[2], colorRGB, hatch);
    defRGB[2][1] = colorRGB[0];
    defRGB[2][2] = colorRGB[1];
    defRGB[2][3] = colorRGB[2];

    // **************** high2 value color *****************
    if(fDoIsolines) // isoviivoja ei tehd‰ viel‰ kuin kolmella v‰rill‰
        XuColorQuery(defaultTableColorIndices[2], colorRGB, hatch);
    else
        XuColorQuery(defaultTableColorIndices[3], colorRGB, hatch);
    defRGB[3][1] = colorRGB[0];
    defRGB[3][2] = colorRGB[1];
    defRGB[3][3] = colorRGB[2];

    const int max_level = 255; // RGB tapauksessa avo on 0-255
    XuClassesMinMax(aMin, aMax, classCount);
    XuShadingScaleChange(shadingScaleIndex, defRGB, s_rgbDefCount, XuRGB, max_level);

    // t‰t‰ ei tarvitsisi varmaan aina luoda!!! optimoi!!!
//	int colorIndexStart = 2; // 2 ensimm‰ist‰ paikkaa varattu backgroud ja foreground colorille + hollow?
    XuColorTableCreate(colorTableIndex, classCount, XuLOOKUP, XuRGB, max_level);
    XuColorTableActivate(colorTableIndex);
    XuShadingScaleLoad(shadingScaleIndex, colorTableIndex, 0, classCount + 1);
    checkedVector<int> colorIndices(classCount);
    for(int i = 0; i < classCount; i++)
        colorIndices[i] = i;
    XuShadingColorIndices(&colorIndices[0], classCount);
    XuShadingScale(shadingScaleIndex);
}


// ***************************************************************************************************************
// **** T‰ss‰ on hieman testaus koodia, jos joskus tulee ongelmia hatchin tai muun polygoni piirron kanssa *******
// **** Globaali muuttuja gDrawTest pit‰‰ muuttaa arvoon true, ja halutun polygonin high-lighttauksen saa  *******
// **** p‰‰lle kun laittaa gDesignatedPolygon globaali muuttujan arvoksi halutun polygonin indeksin. Jos   *******
// **** sen arvo on -1, piirt‰‰ se high-lightin aina juoksu j‰rjestyksess‰ seuraava polygonin kohdallle.   *******
// **** T‰llˆin SMartMetilla voidaan etsi‰ ongelma polygoni refreshaamalla n‰yttˆ‰ F5:lla.                 *******
// ***************************************************************************************************************

const int gMissingValue = -9999;
static const CPoint gMissingCPoint = CPoint(gMissingValue, gMissingValue);

static CPoint CalcPolygonCenter(const checkedVector<CPoint> &thePolygonCPoints)
{
    if(thePolygonCPoints.size())
    {
        double sumX = 0;
        double sumY = 0;
        for(size_t i = 0; i < thePolygonCPoints.size(); i++)
        {
            sumX += thePolygonCPoints[i].x;
            sumY += thePolygonCPoints[i].y;
        }
        CPoint centerPoint(boost::math::iround(sumX / thePolygonCPoints.size()), boost::math::iround(sumY / thePolygonCPoints.size()));
        return centerPoint;
    }
    else
        return gMissingCPoint;
}

static int gDesignatedPolygon = -1; // Laita t‰h‰n 0 tai positiivinen luku jos haluat jonkun tietyn polygonin n‰kyv‰n (kun gDrawTest on true), laita t‰h‰n arvo -1, jos haluat juoksuttaa korostettua polygonia.

static void DrawShadedPolygonsTest(CDC *pDC, checkedVector<int> & thePolyNumbers, checkedVector<float> &thePolyPointsX, checkedVector<float> &thePolyPointsY)
{
    if(thePolyNumbers.size())
    {
        static int polygonCounter = 0;

        std::vector<COLORREF> colorVec;
        colorVec.push_back(RGB(255, 0, 0));
        colorVec.push_back(RGB(0, 255, 0));
        colorVec.push_back(RGB(0, 0, 255));
        colorVec.push_back(RGB(128, 128, 0));
        colorVec.push_back(RGB(0, 128, 128));

        // Luo joukko erilaisia siveltimi‰
        std::vector<CBrush*> brushVec;
        for(size_t i = 0; i < colorVec.size(); i++)
        {
            brushVec.push_back(new CBrush());
            brushVec[i]->CreateHatchBrush(static_cast<int>(i), colorVec[i]);
        }

        int penStyle = PS_SOLID;
        int penWidth = 1;
        int penWidth2 = 2;
        COLORREF penColor1 = 0x00000000;
        COLORREF penColor2 = 0x000000FF;
        CPen myPen1(penStyle, penWidth, penColor1);
        CPen myPen2(penStyle, penWidth2, penColor2);

        int oldBkMode = pDC->SetBkMode(TRANSPARENT);
        int oldTextAlign = pDC->SetTextAlign(TA_CENTER);
        CBrush *oldBrush = pDC->SelectObject(brushVec[0]);
        CPen *oldPen = pDC->SelectObject(&myPen1);

        int minRowYPixelValue = gIgnoreMinMaxPixelHandling; // t‰m‰ arvo on puuttuva arvo, jolloin testi piirrossa n‰iden k‰sittely ignoorataan
        int maxRowYPixelValue = gIgnoreMinMaxPixelHandling;
        checkedVector<int> polyPointsXInPixels, polyPointsYInPixels;
        checkedVector<CPoint> polygonCPoints;
        int polygonPointTotalCount = 0;
        MakeTotalPolygonPointConversion(thePolyPointsX, thePolyPointsY, polyPointsXInPixels, polyPointsYInPixels);
        for(size_t i = 0; i < thePolyNumbers.size(); i++)
        {
            bool drawHelperPolygon = (gDesignatedPolygon >= 0 && gDesignatedPolygon == static_cast<int>(i)) || (gDesignatedPolygon < 0 && (i == polygonCounter));
            pDC->SelectObject(&myPen1);
            pDC->SelectObject(brushVec[i % 5]);
            int polPoinsCount = thePolyNumbers[i];
            FillHatchedPolygonData(polyPointsXInPixels, polyPointsYInPixels, polygonPointTotalCount, polPoinsCount, polygonCPoints, minRowYPixelValue, maxRowYPixelValue);
            //			pDC->Polygon(&polygonCPoints[0], polPoinsCount);
            int circleSize = 3;
            for(size_t j = 0; j < polygonCPoints.size(); j++)
            { // piirret‰‰n polygonin pisteet n‰kyviin
                if(drawHelperPolygon)
                {
                    if(j < 3 || j >= polygonCPoints.size() - 3)
                    {
                        pDC->SelectObject(&myPen2); // ensimm‰inen ja viiimeinen ympyr‰ piirret‰‰n eriv‰rill‰
                        circleSize = 5;
                    }
                    else
                    {
                        pDC->SelectObject(&myPen1); // kaksi ensimm‰ist‰ ympyr‰‰ piirret‰‰n eriv‰rill‰
                        circleSize = 3;
                    }

                    pDC->Ellipse(polygonCPoints[j].x - circleSize, polygonCPoints[j].y - circleSize, polygonCPoints[j].x + circleSize, polygonCPoints[j].y + circleSize);
                }
            }
            pDC->SelectObject(&myPen1);
            circleSize = 3;
            // piirret‰‰n polygonin keskipisteeseen juokseva indeksi
            if(drawHelperPolygon)
            {
                CPoint centerPoint = ::CalcPolygonCenter(polygonCPoints);
                CString indexStrU_;
                int tmpCounter = static_cast<int>(i);
                indexStrU_.Format(_TEXT("%d"), tmpCounter);
                pDC->TextOut(centerPoint.x, centerPoint.y, indexStrU_);
                pDC->Ellipse(centerPoint.x - circleSize, centerPoint.y - circleSize, centerPoint.x + circleSize, centerPoint.y + circleSize);
            }
            polygonPointTotalCount += polPoinsCount;
        }
        polygonCounter++;
        if(polygonCounter >= thePolyNumbers.size())
            polygonCounter = 0;

        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
        pDC->SetTextAlign(oldTextAlign);
        pDC->SetBkMode(oldBkMode);

        // Tuhoa siveltimet
        for(size_t i = 0; i < brushVec.size(); i++)
        {
            brushVec[i]->DeleteObject();
            delete brushVec[i];
        }
    }
}

#endif // DISABLE_UNIRAS_TOOLMASTER
