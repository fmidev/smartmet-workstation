#pragma once

#include "NFmiColor.h"
#include "NFmiPoint.h"
#include <string>

namespace SettingsFunctions
{
   NFmiColor GetColorFromSettings(const std::string &theSettingKey, const NFmiColor *theOptionalColor = 0);
   void SetColorToSettings(const std::string &theSettingKey, const NFmiColor &theColor);
   NFmiPoint GetPointFromSettings(const std::string &theSettingKey);
   NFmiPoint GetCommaSeaparatedPointFromSettings(const std::string &theKey);
   void SetCommaSeaparatedPointToSettings(const std::string &theKey, const NFmiPoint &theGridSize);
   void SetPointToSettings(const std::string &theSettingKey, const NFmiPoint &thePoint);
   std::string GetUrlFromSettings(const std::string &theSettingKey, bool fDoOptional = false, const std::string &theOptionalValue = "");
   void SetUrlToSettings(const std::string &theSettingKey, const std::string &theUrlStr);

}
