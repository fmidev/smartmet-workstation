
#include "WarningSymbolInfo.h"
#include "NFmiDataStoringHelpers.h"
#include "NFmiStringTools.h"

#include <fstream>

namespace HakeLegacySupport
{

    WarningSymbolInfo::WarningSymbolInfo(void)
        :itsColor()
        , itsSymbolID(kEllipse)
        , itsSymbolSizeInPixels(10, 10)
        , itsMessageCategory(0)
        , itsMinLevelToBeShown(0)
        , fUse(false)
    {
    }

    WarningSymbolInfo::~WarningSymbolInfo(void)
    {
    }

    // HUOM!! T�m� laittaa kommentteja mukaan!
    void WarningSymbolInfo::Write(std::ostream& os) const
    {
        os << "// WarningSymbolInfo::Write..." << std::endl;

        os << "// SymbolID + MessageCategory + MinLevelToBeShown + Use" << std::endl;
        os << itsSymbolID << " " << itsMessageCategory << " " << itsMinLevelToBeShown << " " << fUse << std::endl;

        os << "// Color" << std::endl;
        os << itsColor << std::endl;

        os << "// SymbolSizeInPixels" << std::endl;
        os << itsSymbolSizeInPixels;

        NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel� mahdollinen extra data
                                                                // Kun tulee uusia muuttujia, tee t�h�n extradatan t�ytt��, jotta se saadaan talteen tiedopstoon siten ett�
                                                                // edelliset versiot eiv�t mene solmuun vaikka on tullut uutta dataa.
        os << "// possible extra data" << std::endl;
        os << extraData;

        if(os.fail())
            throw std::runtime_error("NFmiWarningSymbolInfo::Write failed");
    }

    // HUOM!! ennen kuin t�m� luokka luetaan sis��n tiedostosta, poista kommentit
    // NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
    // sekaan. Eli �l� k�yt� suoraan t�t� metodia, vaan Init(filename)-metodia!!!!
    void WarningSymbolInfo::Read(std::istream& is)
    {
        int tmpValue = 0;
        is >> tmpValue >> itsMessageCategory >> itsMinLevelToBeShown >> fUse;
        itsSymbolID = static_cast<WarningSymbolInfo::SymbolID>(tmpValue);

        is >> itsColor;

        is >> itsSymbolSizeInPixels;

        if(is.fail())
            throw std::runtime_error("NFmiWarningSymbolInfo::Read failed");

        NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel� mahdollinen extra data
        is >> extraData;
        // T�ss� sitten otetaaan extradatasta talteen uudet muuttujat, mit� on mahdollisesti tullut
        // eli jos uusia muutujia tai arvoja, k�sittele t�ss�.

        if(is.fail())
            throw std::runtime_error("NFmiWarningSymbolInfo::Read failed");
    }

    // purkaa conffista saadun symbolinfo stringin NFmiWarningSymbolInfo-olioksi
    // 128,0,255:2:12,14:413:1:2
    // jossa :-merkill� erotettu osat v�ri(R,G,B):symbolID:koko n�yt�ll�(x,y):kategoria:k�yt�-flagi:minimi level joka n�ytet��n
    WarningSymbolInfo WarningSymbolInfo::GetWarningSymbolInfo(const std::string &theSymbolInfoStr)
    {
        std::string tmpStr(theSymbolInfoStr);
        NFmiStringTools::Trim(tmpStr);
        std::vector<std::string> infoParts = NFmiStringTools::Split(tmpStr, ":");
        if(infoParts.size() != 6)
            throw std::runtime_error(std::string("GetWarningSymbolInfo - WarningSymbolInfo-string invalid, there should be 6 sections divided with ':' in string:\n") + theSymbolInfoStr);

        std::vector<std::string> rgbParts = NFmiStringTools::Split(infoParts[0], ",");
        if(rgbParts.size() != 3)
            throw std::runtime_error(std::string("GetWarningSymbolInfo - WarningSymbolInfo-string invalid, RGB part was invalid, needs three values r,g,b:\n") + infoParts[0]);

        int red = NFmiStringTools::Convert<int>(rgbParts[0]);
        int green = NFmiStringTools::Convert<int>(rgbParts[1]);
        int blue = NFmiStringTools::Convert<int>(rgbParts[2]);
        NFmiColor color(red / 255.f, green / 255.f, blue / 255.f);
        SymbolID sId = static_cast<SymbolID>(NFmiStringTools::Convert<int>(infoParts[1]));

        std::vector<std::string> sizeParts = NFmiStringTools::Split(infoParts[2], ",");
        if(sizeParts.size() != 2)
            throw std::runtime_error(std::string("GetWarningSymbolInfo - WarningSymbolInfo-string invalid, size part was invalid, needs two values x,y:\n") + infoParts[2]);
        int x = NFmiStringTools::Convert<int>(sizeParts[0]);
        int y = NFmiStringTools::Convert<int>(sizeParts[1]);
        NFmiPoint ssize(x, y);

        int category = NFmiStringTools::Convert<int>(infoParts[3]);
        int use = NFmiStringTools::Convert<int>(infoParts[4]);
        int minLevel = NFmiStringTools::Convert<int>(infoParts[5]);

        WarningSymbolInfo sInfo;
        sInfo.Color(color);
        sInfo.SymbolId(sId);
        sInfo.SymbolSizeInPixels(ssize);
        sInfo.MessageCategory(category);
        sInfo.Use(use != 0);
        sInfo.MinLevelToBeShown(minLevel);

        return sInfo;
    }

    std::string WarningSymbolInfo::MakeWarningSymbolInfoStr(const WarningSymbolInfo &theSymbolInfo)
    {
        std::string returnStr;
        returnStr += NFmiStringTools::Convert(static_cast<int>(theSymbolInfo.Color().Red() * 255));
        returnStr += ",";
        returnStr += NFmiStringTools::Convert(static_cast<int>(theSymbolInfo.Color().Green() * 255));
        returnStr += ",";
        returnStr += NFmiStringTools::Convert(static_cast<int>(theSymbolInfo.Color().Blue() * 255));
        returnStr += ":";

        returnStr += NFmiStringTools::Convert(theSymbolInfo.SymbolId());
        returnStr += ":";

        returnStr += NFmiStringTools::Convert(static_cast<int>(theSymbolInfo.SymbolSizeInPixels().X()));
        returnStr += ",";
        returnStr += NFmiStringTools::Convert(static_cast<int>(theSymbolInfo.SymbolSizeInPixels().Y()));
        returnStr += ":";

        returnStr += NFmiStringTools::Convert(theSymbolInfo.MessageCategory());
        returnStr += ":";

        returnStr += NFmiStringTools::Convert(static_cast<int>(theSymbolInfo.Use()));
        returnStr += ":";

        returnStr += NFmiStringTools::Convert(theSymbolInfo.MinLevelToBeShown());

        return returnStr;
    }

} // namespace HakeLegacySupport
