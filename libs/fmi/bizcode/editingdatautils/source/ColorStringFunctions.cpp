#include "ColorStringFunctions.h"
#include "NFmiColor.h"

namespace ColorString
{
    std::string ColorFloat2HexStr(float value)
    {
        int iValue = static_cast<unsigned char>(value * 255.f);
        return ToHex(iValue, 2, '0');
    }

    std::string Color2HtmlColorStr(const NFmiColor& theColor)
    {
        std::string str("#");
        str += ColorFloat2HexStr(theColor.GetRed());
        str += ColorFloat2HexStr(theColor.GetGreen());
        str += ColorFloat2HexStr(theColor.GetBlue());
        return str;
    }
}
