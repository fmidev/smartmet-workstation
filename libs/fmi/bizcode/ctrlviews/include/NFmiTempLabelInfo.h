#pragma once

#include "NFmiColor.h"
#include "NFmiPoint.h"

class NFmiTempLabelInfo
{ // huom! labelilla on sitten sama v‰ri kuin viivalla!! joten t‰‰ll‰ ei ole omaa v‰ri m‰‰rityst‰
public:
    NFmiTempLabelInfo(void)
        :itsStartPointPixelOffSet(0, 0)
        , itsTextAlignment(kLeft)
        , itsFontSize(16)
        , fDrawLabelText(true)
        , fClipWithDataRect(false)
    {}
    NFmiTempLabelInfo(const NFmiPoint &theStartPointPixelOffSet, FmiDirection theTextAlignment, int theFontSize, bool drawLabelText, bool clipWithDataRect)
        :itsStartPointPixelOffSet(theStartPointPixelOffSet)
        , itsTextAlignment(theTextAlignment)
        , itsFontSize(theFontSize)
        , fDrawLabelText(drawLabelText)
        , fClipWithDataRect(clipWithDataRect)
    {}

    const NFmiPoint& StartPointPixelOffSet(void) const { return itsStartPointPixelOffSet; }
    void StartPointPixelOffSet(const NFmiPoint &newValue) { itsStartPointPixelOffSet = newValue; }
    FmiDirection TextAlignment(void) const { return itsTextAlignment; }
    void TextAlignment(FmiDirection newValue) { itsTextAlignment = newValue; }
    int FontSize(void) const { return itsFontSize; }
    void FontSize(int newValue) { itsFontSize = newValue; }
    bool DrawLabelText(void) const { return fDrawLabelText; }
    void DrawLabelText(bool newValue) { fDrawLabelText = newValue; }
    bool ClipWithDataRect(void) const { return fClipWithDataRect; }
    void ClipWithDataRect(bool newValue) { fClipWithDataRect = newValue; }

    void Write(std::ostream& os) const;
    void Read(std::istream& is);
private:
    NFmiPoint itsStartPointPixelOffSet; // siit‰ mist‰ apuviivaa aletaan piirt‰‰, mink‰lainen pikseli m‰‰r‰inen siirtym‰ laitetaan label tekstille
    FmiDirection itsTextAlignment;
    int itsFontSize; // fontin koko pikselein‰
    bool fDrawLabelText; // piirret‰‰nkˆ kyseiset arvot labeleina vai ei
    bool fClipWithDataRect; // cliparaanko fontit niin ett‰ niit‰ ei piirret‰ data-alueen ulkopuolelle
};

inline std::ostream& operator<<(std::ostream& os, const NFmiTempLabelInfo& item) { item.Write(os); return os; }
inline std::istream& operator >> (std::istream& is, NFmiTempLabelInfo& item) { item.Read(is); return is; }
