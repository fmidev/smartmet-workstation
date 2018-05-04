#pragma once

#include "stdafx.h"
#include "NFmiColor.h"
#include <gdiplus.h>

class GdiPlusLineInfo
{
    float itsThickness; // in pixels
    float itsTension;
    NFmiColor itsColor;
    int itsLineType; // 0=Solid,1=Dash,2=Dot,3=DashDot,4=DashDotDot,5=Custom
    Gdiplus::SmoothingMode itsSmoothingMode;
    Gdiplus::LineJoin itsLineJoin;
    Gdiplus::LineCap itsLineCap;
    bool fUseOutLine;
public:
    GdiPlusLineInfo(float theThickness, const NFmiColor &theColor, int theLineType);

    float Thickness(void) const { return itsThickness; }
    void Thickness(float newValue) { itsThickness = newValue; }
    const NFmiColor& Color(void) const { return itsColor; }
    void Color(const NFmiColor &newColor) { itsColor = newColor; }
    int LineType(void) const { return itsLineType; }
    void LineType(int newValue) { itsLineType = newValue; }
    float Tension(void) const { return itsTension; }
    void Tension(float newValue) { itsTension = newValue; }
    Gdiplus::SmoothingMode GetSmoothingMode(void) const { return itsSmoothingMode; }
    void SetSmoothingMode(Gdiplus::SmoothingMode newValue) { itsSmoothingMode = newValue; }
    Gdiplus::LineJoin GetLineJoin(void) const { return itsLineJoin; }
    void SetLineJoin(Gdiplus::LineJoin newValue) { itsLineJoin = newValue; }
    Gdiplus::LineCap GetLineCap(void) const { return itsLineCap; }
    void SetLineCap(Gdiplus::LineCap newValue) { itsLineCap = newValue; }
    bool UseOutLine(void) const { return fUseOutLine; }
    void UseOutLine(bool newValue) { fUseOutLine = newValue; }
};
