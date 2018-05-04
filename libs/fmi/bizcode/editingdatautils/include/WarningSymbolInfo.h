#pragma once

#include "NFmiPoint.h"
#include "NFmiColor.h"
#include <iosfwd>

namespace HakeLegacySupport
{

    class WarningSymbolInfo
    {
    public:
        enum SymbolID
        {
            kRectangle = 1,
            kEllipse = 2,
            kTriangle = 3,
            kPentagon = 4,
            kPlusSign = 5, // +
            kCross = 6,    // X
            kDiamond = 7
        };

        WarningSymbolInfo(void);
        ~WarningSymbolInfo(void);

        const NFmiColor& Color(void) const { return itsColor; }
        void Color(const NFmiColor &newValue) { itsColor = newValue; }
        SymbolID SymbolId(void) const { return itsSymbolID; }
        void SymbolId(SymbolID newValue) { itsSymbolID = newValue; }
        const NFmiPoint& SymbolSizeInPixels(void) const { return itsSymbolSizeInPixels; }
        void SymbolSizeInPixels(const NFmiPoint &newValue) { itsSymbolSizeInPixels = newValue; }
        int MessageCategory(void) const { return itsMessageCategory; }
        void MessageCategory(int newValue) { itsMessageCategory = newValue; }
        int MinLevelToBeShown(void) const { return itsMinLevelToBeShown; }
        void MinLevelToBeShown(int newValue) { itsMinLevelToBeShown = newValue; }
        bool Use(void) const { return fUse; }
        void Use(bool newValue) { fUse = newValue; }

        static WarningSymbolInfo GetWarningSymbolInfo(const std::string &theSymbolInfoStr);
        static std::string MakeWarningSymbolInfoStr(const WarningSymbolInfo &theSymbolInfo);

        // HUOM!! T‰m‰ laittaa kommentteja mukaan!
        void Write(std::ostream& os) const;
        // HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
        // NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
        // sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
        void Read(std::istream& is);
    private:
        NFmiColor itsColor;
        SymbolID itsSymbolID;
        NFmiPoint itsSymbolSizeInPixels;
        int itsMessageCategory;
        int itsMinLevelToBeShown; // messageilla on levelit 0=ei alustettu, 1=pieni, 2=normaali ja 3=suuri. T‰m‰ m‰‰r‰‰ mink‰ tasoiset minimiss‰‰n n‰ytet‰‰n.
        bool fUse; // on/off kytkin, k‰ytet‰‰nkˆ tai n‰ytet‰‰nkˆ t‰m‰ kategorian h‰lyj‰
    };

}

inline std::ostream& operator<<(std::ostream& os, const HakeLegacySupport::WarningSymbolInfo& item) { item.Write(os); return os; }
inline std::istream& operator >> (std::istream& is, HakeLegacySupport::WarningSymbolInfo& item) { item.Read(is); return is; }
