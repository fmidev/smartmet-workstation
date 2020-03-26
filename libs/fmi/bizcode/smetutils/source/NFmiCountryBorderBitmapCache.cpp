#include "stdafx.h"
#include "NFmiCountryBorderBitmapCache.h"
#include "NFmiColor.h"

#include "boost/math/special_functions.hpp"

#include <gdiplus.h>

#ifdef max
#undef min // pit‰‰ undeffata typer‰t win32 min/max macrot
#undef max
#endif

namespace
{
} // nameless namespace ends
 
NFmiCountryBorderBitmapCache::NFmiCountryBorderBitmapCache() = default;

NFmiCountryBorderBitmapCache::~NFmiCountryBorderBitmapCache() = default;

Gdiplus::Bitmap* NFmiCountryBorderBitmapCache::getCacheBitmap(const std::string& keyString) const
{
    auto foundIter = bitmapCacheMap_.find(keyString);
    if(foundIter != bitmapCacheMap_.end())
        return foundIter->second.get();
    else
        return nullptr;
}

void NFmiCountryBorderBitmapCache::insertCacheBitmap(const std::string& keyString, std::unique_ptr<Gdiplus::Bitmap>&& cacheBitmap)
{
    // Joko lis‰‰ uudella avaimella bitmapin tai sitten korvaa jo talletetulla avaimella olevan bitmapin t‰ll‰.
    bitmapCacheMap_[keyString] = std::move(cacheBitmap);
}

void NFmiCountryBorderBitmapCache::clearCache()
{
    bitmapCacheMap_.clear();
}
