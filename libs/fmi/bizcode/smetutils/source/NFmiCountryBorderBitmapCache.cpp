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

// S‰‰dett‰v‰ cachen likaus funktio: 
// 1. Jos newState = Geometry, kaikki cachet menee sile‰ksi.
// 2. Jos cosmetic, pit‰‰ keyString pointterissa olla jokin arvo, ja sill‰ avaimella oleva kuva pyyhit‰‰n pois.
void NFmiCountryBorderBitmapCache::setBorderDrawDirtyState(CountryBorderDrawDirtyState newState, const std::string& keyString)
{
    if(newState == CountryBorderDrawDirtyState::Geometry)
        clearCache();
    else if(newState == CountryBorderDrawDirtyState::Cosmetic && !keyString.empty())
    {
        bitmapCacheMap_.erase(keyString);
    }
}
