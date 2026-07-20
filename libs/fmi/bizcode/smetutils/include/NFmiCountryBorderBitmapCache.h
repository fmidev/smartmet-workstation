#pragma once

#include "CombinedMapHandlerInterface.h"

#include <map>
#include <string>
#include <memory>

namespace Gdiplus
{
    class Bitmap;
}

// This class holds one map view's possible separate country-border drawing
// bitmaps, which are meant to be used as 'stamps' when drawing the view.
// Different view rows can have different settings, so each row potentially gets
// its own bitmap stored in the cache.
// The cache key consists of the following:
// 1. Line width in pixels (0-3, and 0 means no drawing), e.g. 1
// 2. Line color, e.g. black, expressed as RRGGBB (i.e. RGB values in hex, 00 is 0 and FF is 255); black would be 000000 and white FFFFFF
// The key as a whole e.g. 1_000000 which would be line width 1 and color black.
// Since each view has its own cache, this cache can be cleared entirely whenever e.g. the zoom changes or something is done that
// changes the base map size etc.
// Different rows can share cache images, because within one view the map area and size are always the same on every map cell.
class NFmiCountryBorderBitmapCache
{
    std::map<std::string, std::unique_ptr<Gdiplus::Bitmap>> bitmapCacheMap_;
public:
    NFmiCountryBorderBitmapCache();
    NFmiCountryBorderBitmapCache(const NFmiCountryBorderBitmapCache& other);
    NFmiCountryBorderBitmapCache& operator=(const NFmiCountryBorderBitmapCache& other);
    ~NFmiCountryBorderBitmapCache();

    Gdiplus::Bitmap* getCacheBitmap(const std::string& keyString) const;
    void insertCacheBitmap(const std::string& keyString, std::unique_ptr<Gdiplus::Bitmap> &&cacheBitmap);
    void clearCache();
    void setBorderDrawDirtyState(CountryBorderDrawDirtyState newState, const std::string& keyString = "");
};
