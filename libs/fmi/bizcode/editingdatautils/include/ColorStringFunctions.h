#pragma once

#include <string>
#include <iomanip>

class NFmiColor;

namespace ColorString
{
   std::string ColorFloat2HexStr(float value);
   std::string Color2HtmlColorStr(const NFmiColor &theColor);

   template<class T>
   std::string ToHex(const T& value, int minWidth = 0, char paddingchar = ' ')
   {
       std::ostringstream oss;
       if(!(oss << std::setw(minWidth) << std::setfill(paddingchar) << std::hex << value))
           throw std::exception("ToHex - Invalid argument");
       return oss.str();
   }
}
