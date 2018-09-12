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
    int gUsedDefaultToolMasterColorTableIndex = 2;
    // tehd‰‰n alkuun kahden siirtym‰ fore- ja background colorien takia ja + 1 hollow color
    int gUsedDefaultToolMasterColorTableIndexStart = 3; 
    // eli 3. v‰ri on l‰pin‰kyv‰ v‰ri kuutiossa on v‰ri COLORREF muodossa ja sen vastaava indeksi toolmaster tauluun
    //static std::pair<int, COLORREF> gUsedColorsCube[gColorTableRedSize][gColorTableGreenSize][gColorTableBlueSize];
    int gUsedDefaultToolMasterColorTableHollowColorIndex = 2; 

    Matrix3D<std::pair<int, COLORREF> > gUsedColorsCube(gColorTableRedSize, gColorTableGreenSize, gColorTableBlueSize);
    bool fColorTableInitialized = false;

}


namespace ToolMasterColorCube 
{
    Matrix3D<std::pair<int, COLORREF> >* UsedColorsCube(void)
    {
        return &gUsedColorsCube;
    }

    int UsedColorTableIndex()
    {
        return gUsedDefaultToolMasterColorTableIndex;
    }

    int UsedHollowColorIndex()
    {
        return gUsedDefaultToolMasterColorTableHollowColorIndex;
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
                int colorTableSize = gUsedDefaultToolMasterColorTableIndexStart + gColorTableRedSize * gColorTableGreenSize * gColorTableBlueSize;

                XuColorTableCreate(gUsedDefaultToolMasterColorTableIndex, colorTableSize, XuLOOKUP, XuRGB, 255);
                XuColorTableActivate(gUsedDefaultToolMasterColorTableIndex);

                // Copy Windows' text color to Toolmaster's foreground
                f = GetSysColor(COLOR_WINDOWTEXT);
                rgb[0] = GetRValue(f);
                rgb[1] = GetGValue(f);
                rgb[2] = GetBValue(f);
                XuColor(XuCOLOR, 1, rgb, dummy);
                XuColorDeviceLoad(1);

                // Copy Windows' window color to Toolmasters background
                f = GetSysColor(COLOR_WINDOW);
                rgb[0] = GetRValue(f);
                rgb[1] = GetGValue(f);
                rgb[2] = GetBValue(f);
                XuColor(XuCOLOR, 0, rgb, dummy);
                XuColorDeviceLoad(0);

                // m‰‰ritet‰‰n 3. v‰ri l‰pin‰kyv‰ksi (hollow)
                int hollowColorIndex = gUsedDefaultToolMasterColorTableHollowColorIndex;
                int colorType = 0;
                XuColorTypeQuery(hollowColorIndex, &colorType);
                if(colorType != XuOFF)
                    XuColorType(hollowColorIndex, XuOFF);
                XuUndefined(kFloatMissing, hollowColorIndex);
                XuColorType(hollowColorIndex, XuHOLLOW_COLOR);

                int index = gUsedDefaultToolMasterColorTableIndexStart;
                for(int k = 0; k < gColorTableRedSize; k++)
                {
                    for(int j = 0; j < gColorTableGreenSize; j++)
                    {
                        for(int i = 0; i < gColorTableBlueSize; i++)
                        {
                            rgb[0] = static_cast<float>(::round(255. / (gColorTableRedSize - 1) * k));
                            rgb[1] = static_cast<float>(::round(255. / (gColorTableGreenSize - 1) * j));
                            rgb[2] = static_cast<float>(::round(255. / (gColorTableBlueSize - 1) * i));
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
                int index = gUsedDefaultToolMasterColorTableIndexStart;
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
        int k = static_cast<int>(RGBcolors[0] / 255.*(gColorTableRedSize - 1));
        int j = static_cast<int>(RGBcolors[1] / 255.*(gColorTableGreenSize - 1));
        int i = static_cast<int>(RGBcolors[2] / 255.*(gColorTableBlueSize - 1));

        return gUsedColorsCube[k][j][i].first;
    }

    int RgbToColorIndex(COLORREF color)
    {
        int k = GetRValue(color) / gColorTableRedSize
            , j = GetGValue(color) / gColorTableGreenSize
            , i = GetBValue(color) / gColorTableBlueSize;

        return gUsedColorsCube[k][j][i].first;
    }

    int RgbToColorIndex(const NFmiColor& color)
    {
        int k = static_cast<int>(color.Red()*(gColorTableRedSize - 1));
        int j = static_cast<int>(color.Green()*(gColorTableGreenSize - 1));
        int i = static_cast<int>(color.Blue()*(gColorTableBlueSize - 1));

        return gUsedColorsCube[k][j][i].first;
    }

    NFmiColor ColorIndexToRgb(int theColorIndex)
    {
        if(theColorIndex < gUsedDefaultToolMasterColorTableIndexStart)
            return NFmiColor(0, 0, 0); // en jaksa palauttaa foreground, background ja hollow v‰rej‰

        theColorIndex -= gUsedDefaultToolMasterColorTableIndexStart;
        float blue = (theColorIndex % gColorTableBlueSize) / (gColorTableBlueSize - 1.f);
        theColorIndex = theColorIndex / gColorTableBlueSize;
        float green = (theColorIndex % gColorTableGreenSize) / (gColorTableGreenSize - 1.f);
        theColorIndex = theColorIndex / gColorTableGreenSize;
        float red = (theColorIndex % gColorTableRedSize) / (gColorTableRedSize - 1.f);

        return NFmiColor(red, green, blue);
    }

}

#endif // DISABLE_UNIRAS_TOOLMASTER
