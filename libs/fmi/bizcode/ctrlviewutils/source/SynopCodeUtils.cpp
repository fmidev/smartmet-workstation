#include "SynopCodeUtils.h"

namespace CtrlViewUtils
{
    std::string GetSynopCodeAsSynopFontText(float value)
    {
        if(value != kFloatMissing && value > 3)
        {
            char ch = static_cast<char>(157 + value);
            if(value == 99)
                ch = 48; // synop fontin viimeinen arvo 255 osuu synop arvolle 98, onneksi myˆs 99 lˆytyy fontista, mutta sen sijainti on vain 48
            return std::string(1, ch);
        }
        return std::string();
    }

    NFmiColor GetSynopCodeSymbolColor(float value)
    {
        switch(int(value))
        {
        case 4:
        case 25:
        case 26:
        case 27:
        case 80:
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
        case 88:
        case 89:
        case 90:
            return NFmiColor(0.5f, 0, 1); // violetit s‰‰t
        case 10:
        case 11:
        case 12:
            return NFmiColor(0.7f, 0.7f, 0.7f); // harmaat s‰‰t
        case 28:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
            return NFmiColor(0.85f, 0.75f, 0.02f); // keltaiset s‰‰t
        case 14:
        case 15:
        case 16:
        case 20:
        case 21:
        case 22:
        case 23:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 58:
        case 59:
        case 60:
        case 68:
        case 69:
        case 70:
        case 76:
        case 77:
        case 78:
            return NFmiColor(0, 0.6f, 0.3f); // vihre‰t s‰‰t
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
            return NFmiColor(0, 0.5f, 0); // tumman vihre‰t s‰‰t
        case 8:
        case 13:
        case 17:
        case 24:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 56:
        case 57:
        case 66:
        case 67:
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
        case 98:
        case 99:
            return NFmiColor(1, 0, 0); // punaiset s‰‰t
        default:
            return NFmiColor(0, 0, 0); // mustat s‰‰t
        }
    }

}