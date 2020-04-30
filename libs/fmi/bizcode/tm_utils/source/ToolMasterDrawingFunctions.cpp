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
    if(hatchSettings.itsHatchPattern == -1) // jos hatch-pattern on -1, tehd‰‰nkin t‰ysin peitt‰v‰ sivelline ilman hatchi‰
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

// T‰m‰n testi funktion toteutus on tiedoston lopussa.
static void DrawShadedPolygonsTest(CDC *pDC, checkedVector<int> & thePolyNumbers, checkedVector<float> &thePolyPointsX, checkedVector<float> &thePolyPointsY);

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
        //if(gDrawTest)
        //    DrawShadedPolygonsTest(pDC, polyNumbers, polyPointsX, polyPointsY);
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

//static int gDesignatedPolygon = -1; // Laita t‰h‰n 0 tai positiivinen luku jos haluat jonkun tietyn polygonin n‰kyv‰n (kun gDrawTest on true), laita t‰h‰n arvo -1, jos haluat juoksuttaa korostettua polygonia.
//
//static void DrawShadedPolygonsTest(CDC *pDC, checkedVector<int> & thePolyNumbers, checkedVector<float> &thePolyPointsX, checkedVector<float> &thePolyPointsY)
//{
//    if(thePolyNumbers.size())
//    {
//        static int polygonCounter = 0;
//
//        std::vector<COLORREF> colorVec;
//        colorVec.push_back(RGB(255, 0, 0));
//        colorVec.push_back(RGB(0, 255, 0));
//        colorVec.push_back(RGB(0, 0, 255));
//        colorVec.push_back(RGB(128, 128, 0));
//        colorVec.push_back(RGB(0, 128, 128));
//
//        // Luo joukko erilaisia siveltimi‰
//        std::vector<CBrush*> brushVec;
//        for(size_t i = 0; i < colorVec.size(); i++)
//        {
//            brushVec.push_back(new CBrush());
//            brushVec[i]->CreateHatchBrush(static_cast<int>(i), colorVec[i]);
//        }
//
//        int penStyle = PS_SOLID;
//        int penWidth = 1;
//        int penWidth2 = 2;
//        COLORREF penColor1 = 0x00000000;
//        COLORREF penColor2 = 0x000000FF;
//        CPen myPen1(penStyle, penWidth, penColor1);
//        CPen myPen2(penStyle, penWidth2, penColor2);
//
//        int oldBkMode = pDC->SetBkMode(TRANSPARENT);
//        int oldTextAlign = pDC->SetTextAlign(TA_CENTER);
//        CBrush *oldBrush = pDC->SelectObject(brushVec[0]);
//        CPen *oldPen = pDC->SelectObject(&myPen1);
//
//        int minRowYPixelValue = gIgnoreMinMaxPixelHandling; // t‰m‰ arvo on puuttuva arvo, jolloin testi piirrossa n‰iden k‰sittely ignoorataan
//        int maxRowYPixelValue = gIgnoreMinMaxPixelHandling;
//        checkedVector<int> polyPointsXInPixels, polyPointsYInPixels;
//        checkedVector<CPoint> polygonCPoints;
//        int polygonPointTotalCount = 0;
//        MakeTotalPolygonPointConversion(thePolyPointsX, thePolyPointsY, polyPointsXInPixels, polyPointsYInPixels);
//        for(size_t i = 0; i < thePolyNumbers.size(); i++)
//        {
//            bool drawHelperPolygon = (gDesignatedPolygon >= 0 && gDesignatedPolygon == static_cast<int>(i)) || (gDesignatedPolygon < 0 && (i == polygonCounter));
//            pDC->SelectObject(&myPen1);
//            pDC->SelectObject(brushVec[i % 5]);
//            int polPoinsCount = thePolyNumbers[i];
//            FillHatchedPolygonData(polyPointsXInPixels, polyPointsYInPixels, polygonPointTotalCount, polPoinsCount, polygonCPoints, minRowYPixelValue, maxRowYPixelValue);
//            //			pDC->Polygon(&polygonCPoints[0], polPoinsCount);
//            int circleSize = 3;
//            for(size_t j = 0; j < polygonCPoints.size(); j++)
//            { // piirret‰‰n polygonin pisteet n‰kyviin
//                if(drawHelperPolygon)
//                {
//                    if(j < 3 || j >= polygonCPoints.size() - 3)
//                    {
//                        pDC->SelectObject(&myPen2); // ensimm‰inen ja viiimeinen ympyr‰ piirret‰‰n eriv‰rill‰
//                        circleSize = 5;
//                    }
//                    else
//                    {
//                        pDC->SelectObject(&myPen1); // kaksi ensimm‰ist‰ ympyr‰‰ piirret‰‰n eriv‰rill‰
//                        circleSize = 3;
//                    }
//
//                    pDC->Ellipse(polygonCPoints[j].x - circleSize, polygonCPoints[j].y - circleSize, polygonCPoints[j].x + circleSize, polygonCPoints[j].y + circleSize);
//                }
//            }
//            pDC->SelectObject(&myPen1);
//            circleSize = 3;
//            // piirret‰‰n polygonin keskipisteeseen juokseva indeksi
//            if(drawHelperPolygon)
//            {
//                CPoint centerPoint = ::CalcPolygonCenter(polygonCPoints);
//                CString indexStrU_;
//                int tmpCounter = static_cast<int>(i);
//                indexStrU_.Format(_TEXT("%d"), tmpCounter);
//                pDC->TextOut(centerPoint.x, centerPoint.y, indexStrU_);
//                pDC->Ellipse(centerPoint.x - circleSize, centerPoint.y - circleSize, centerPoint.x + circleSize, centerPoint.y + circleSize);
//            }
//            polygonPointTotalCount += polPoinsCount;
//        }
//        polygonCounter++;
//        if(polygonCounter >= thePolyNumbers.size())
//            polygonCounter = 0;
//
//        pDC->SelectObject(oldPen);
//        pDC->SelectObject(oldBrush);
//        pDC->SetTextAlign(oldTextAlign);
//        pDC->SetBkMode(oldBkMode);
//
//        // Tuhoa siveltimet
//        for(size_t i = 0; i < brushVec.size(); i++)
//        {
//            brushVec[i]->DeleteObject();
//            delete brushVec[i];
//        }
//    }
//}

#endif // DISABLE_UNIRAS_TOOLMASTER
