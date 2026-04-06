#include "GdiPlusLineInfo.h"

#ifndef UNIX
GdiPlusLineInfo::GdiPlusLineInfo(float theThickness, const NFmiColor &theColor, int theLineType)
    :itsThickness(theThickness)
    , itsColor(theColor)
    , itsLineType(theLineType)
    , itsTension(0.f)
    , itsSmoothingMode(Gdiplus::SmoothingModeAntiAlias)
    , itsLineJoin(Gdiplus::LineJoinRound)
    , itsLineCap(Gdiplus::LineCapFlat)
    , fUseOutLine(false)
{
}
#endif // UNIX
