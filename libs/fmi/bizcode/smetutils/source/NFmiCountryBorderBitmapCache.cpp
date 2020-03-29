#include "stdafx.h"
#include "NFmiCountryBorderBitmapCache.h"

#include <gdiplus.h>


// ************* NFmiCountryBorderBitmapCache ***********************

NFmiCountryBorderBitmapCache::NFmiCountryBorderBitmapCache() = default;

NFmiCountryBorderBitmapCache::NFmiCountryBorderBitmapCache(const NFmiCountryBorderBitmapCache& other)
{
    *this = other;
}

NFmiCountryBorderBitmapCache& NFmiCountryBorderBitmapCache::operator=(const NFmiCountryBorderBitmapCache& other)
{
    if(this != &other)
    {
        // cachea ei saa kopioida, se pit‰‰ vain tyhjent‰‰
        bitmapCacheMap_.clear();
    }
    return *this;
}

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
