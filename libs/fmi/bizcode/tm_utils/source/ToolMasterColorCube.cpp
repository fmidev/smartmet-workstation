#include "stdafx.h"

#ifndef DISABLE_UNIRAS_TOOLMASTER

#include "ToolMasterColorCube.h"

#include <agx\agx.h>

namespace
{
    const int gColorTableRedSize = 8;
    const int gColorTableGreenSize = 8;
    const int gColorTableBlueSize = 8;

    // t‰m‰ toolmasterin colortable on varattu editorin vakio k‰yttˆˆn johon on varattu 8x8x8 v‰ri kuutio
    const int gUsedDefaultToolMasterColorTableIndex = 2;
    // eli 3. v‰ri on l‰pin‰kyv‰ v‰ri kuutiossa on v‰ri COLORREF muodossa ja sen vastaava indeksi toolmaster tauluun
    //static std::pair<int, COLORREF> gUsedColorsCube[gColorTableRedSize][gColorTableGreenSize][gColorTableBlueSize];
    const int gUsedDefaultToolMasterColorTableHollowColorIndex = 2;
    // Tehd‰‰n alkuun kahden siirtym‰ fore- ja background colorien takia ja + 1 hollow color
    const int gUsedDefaultToolMasterColorTableIndexStart = gUsedDefaultToolMasterColorTableHollowColorIndex + 1;

    Matrix3D<std::pair<int, COLORREF> > gUsedColorsCube(gColorTableRedSize, gColorTableGreenSize, gColorTableBlueSize);
    bool fColorTableInitialized = false;

}


namespace ToolMasterColorCube 
{
    Matrix3D<std::pair<int, COLORREF> >* UsedColorsCube(void)
    {
        return &gUsedColorsCube;
    }

    int UsedDefaultColorTableIndex()
    {
        return gUsedDefaultToolMasterColorTableIndex;
    }

    int UsedHollowColorIndex()
    {
        return gUsedDefaultToolMasterColorTableHollowColorIndex;
    }

    int ColorCubeColorChannelSize()
    {
        return gColorTableRedSize;
    }

    int SpecialColorCountInColorTableStart()
    {
        return gUsedDefaultToolMasterColorTableIndexStart;
    }

    void SetupSpecialColorsForActiveColorTable()
    {
        COLORREF f;
        float rgb[3], dummy[5];
        // Copy Windows' window color to Toolmasters background
        int backgroundColorIndex = 0;
        f = GetSysColor(COLOR_WINDOW);
        rgb[0] = GetRValue(f);
        rgb[1] = GetGValue(f);
        rgb[2] = GetBValue(f);
        XuColor(XuCOLOR, backgroundColorIndex, rgb, dummy);
        XuColorType(backgroundColorIndex, XuBACKGROUND);
        XuColorDeviceLoad(backgroundColorIndex);

        // Copy Windows' text color to Toolmaster's foreground
        int foregroundColorIndex = 1;
        f = GetSysColor(COLOR_WINDOWTEXT);
        rgb[0] = GetRValue(f);
        rgb[1] = GetGValue(f);
        rgb[2] = GetBValue(f);
        XuColor(XuCOLOR, foregroundColorIndex, rgb, dummy);
        XuColorType(foregroundColorIndex, XuANTIBACKGROUND);
        XuColorDeviceLoad(foregroundColorIndex);

        SetupTransparentColor();
    }

    void SetupTransparentColor(bool setup1, bool setup2, bool setup3)
    {
        // m‰‰ritet‰‰n 3. v‰ri l‰pin‰kyv‰ksi (hollow)
        int hollowColorIndex = gUsedDefaultToolMasterColorTableHollowColorIndex;
        float rgb[3], dummy[5];
        rgb[0] = 255;
        rgb[1] = 0;
        rgb[2] = 0;
        if(setup1)
            XuColor(XuCOLOR, hollowColorIndex, rgb, dummy);
        if(setup2)
            XuUndefined(kFloatMissing, hollowColorIndex);
        if(setup3)
            XuColorType(hollowColorIndex, XuHOLLOW_COLOR);
    }

    void InitDefaultColorTable(bool fToolMasterAvailable)
    {
        if(!fColorTableInitialized)
        {
            fColorTableInitialized = true;

            if(fToolMasterAvailable)
            {
                COLORREF f;
                float rgb[3], dummy[5];

                // 2+ on tilaa fore- ja background coloreille
                int colorTableSize = SpecialColorCountInColorTableStart() + gColorTableRedSize * gColorTableGreenSize * gColorTableBlueSize;

                XuColorTableCreate(gUsedDefaultToolMasterColorTableIndex, colorTableSize, XuLOOKUP, XuRGB, 255);
                XuColorTableActivate(gUsedDefaultToolMasterColorTableIndex);

                SetupSpecialColorsForActiveColorTable();

                int index = SpecialColorCountInColorTableStart();
                for(int k = 0; k < gColorTableRedSize; k++)
                {
                    for(int j = 0; j < gColorTableGreenSize; j++)
                    {
                        for(int i = 0; i < gColorTableBlueSize; i++)
                        {
                            rgb[0] = std::roundf(::round(255.f / (gColorTableRedSize - 1) * k));
                            rgb[1] = std::roundf(::round(255.f / (gColorTableGreenSize - 1) * j));
                            rgb[2] = std::roundf(::round(255.f / (gColorTableBlueSize - 1) * i));
                            XuColor(XuCOLOR, index, rgb, dummy);
                            f = RGB(rgb[0], rgb[1], rgb[2]);
                            gUsedColorsCube[k][j][i] = std::make_pair(index, f);
                            index++;
                        }
                    }
                }
            }
            else // toolmasterista riippumaton alustus
            {
                COLORREF f;
                float rgb[3];

                // 2+ on tilaa fore- ja background coloreille
                int index = SpecialColorCountInColorTableStart();
                for(int k = 0; k < gColorTableRedSize; k++)
                {
                    for(int j = 0; j < gColorTableGreenSize; j++)
                    {
                        for(int i = 0; i < gColorTableBlueSize; i++)
                        {
                            rgb[0] = static_cast<float>(int(255. / (gColorTableRedSize - 1) * k));
                            rgb[1] = static_cast<float>(int(255. / (gColorTableGreenSize - 1) * j));
                            rgb[2] = static_cast<float>(int(255. / (gColorTableBlueSize - 1) * i));
                            f = RGB(rgb[0], rgb[1], rgb[2]);
                            gUsedColorsCube[k][j][i] = std::make_pair(index, f);
                            index++;
                        }
                    }
                }
            }
        }
    }

    // yhdist‰ n‰m‰ RgbToColorIndex funktiot k‰ytt‰m‰‰n samaa runkoa konversioiden avulla

    int RgbToColorIndex(float RGBcolors[3])
    {
        int k = boost::math::iround(RGBcolors[0] / 255.*(gColorTableRedSize - 1));
        int j = boost::math::iround(RGBcolors[1] / 255.*(gColorTableGreenSize - 1));
        int i = boost::math::iround(RGBcolors[2] / 255.*(gColorTableBlueSize - 1));

        return gUsedColorsCube[k][j][i].first;
    }

    int RgbToColorIndex(COLORREF color)
    {
        int k = GetRValue(color) / gColorTableRedSize;
        int j = GetGValue(color) / gColorTableGreenSize;
        int i = GetBValue(color) / gColorTableBlueSize;

        return gUsedColorsCube[k][j][i].first;
    }

    int RgbToColorIndex(const NFmiColor& color)
    {
        if(!IsColorFullyOpaque(color))
            return UsedHollowColorIndex();

        int k = boost::math::iround(color.Red()*(gColorTableRedSize - 1));
        int j = boost::math::iround(color.Green()*(gColorTableGreenSize - 1));
        int i = boost::math::iround(color.Blue()*(gColorTableBlueSize - 1));

        return gUsedColorsCube[k][j][i].first;
    }

    NFmiColor ColorIndexToRgb(int theColorIndex)
    {
        if(theColorIndex < gUsedDefaultToolMasterColorTableIndexStart)
            return NFmiColor(0, 0, 0, 1); // en jaksa palauttaa foreground, background ja hollow v‰rej‰

        theColorIndex -= gUsedDefaultToolMasterColorTableIndexStart;
        float blue = (theColorIndex % gColorTableBlueSize) / (gColorTableBlueSize - 1.f);
        theColorIndex = theColorIndex / gColorTableBlueSize;
        float green = (theColorIndex % gColorTableGreenSize) / (gColorTableGreenSize - 1.f);
        theColorIndex = theColorIndex / gColorTableGreenSize;
        float red = (theColorIndex % gColorTableRedSize) / (gColorTableRedSize - 1.f);

        return NFmiColor(red, green, blue);
    }

    NFmiColor ColorToActualCubeColor(const NFmiColor& color)
    {
        return ColorIndexToRgb(RgbToColorIndex(color));
    }

    bool IsColorFullyOpaque(const NFmiColor& color)
    {
        return color.Alpha() <= 0.f;
    }

    bool IsColorFullyTransparent(const NFmiColor& color)
    {
        return color.Alpha() >= 1.f;
    }

}

#endif // DISABLE_UNIRAS_TOOLMASTER
