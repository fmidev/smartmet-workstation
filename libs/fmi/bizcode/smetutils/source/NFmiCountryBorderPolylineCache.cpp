#include "stdafx.h"
#include "NFmiCountryBorderPolylineCache.h"
#include "NFmiPolyline.h"

namespace
{
    //template<typename T>
    struct PointerDestroyer
    {
        template<typename T>
        void operator()(T* thePtr)
        {
            delete thePtr;
        }
    };

} // nameless namespace ends

// ************* NFmiCountryBorderPolylineCache ***********************

NFmiCountryBorderPolylineCache::NFmiCountryBorderPolylineCache() = default;

NFmiCountryBorderPolylineCache::NFmiCountryBorderPolylineCache(const NFmiCountryBorderPolylineCache& other)
{
    *this = other;
}

NFmiCountryBorderPolylineCache& NFmiCountryBorderPolylineCache::operator=(const NFmiCountryBorderPolylineCache& other)
{
    if(this != &other)
    {
        // When copying, the caches just need to be reset.
        clearCache();
        // There is also no point in copying or resetting the offset points
    }
    return *this;
}

NFmiCountryBorderPolylineCache::~NFmiCountryBorderPolylineCache()
{
    clearDrawBorderPolyLineList();
}

void NFmiCountryBorderPolylineCache::clearCache()
{
    clearDrawBorderPolyLineList();
    clearDrawBorderPolyLineListGdiplus();
}

void NFmiCountryBorderPolylineCache::clearDrawBorderPolyLineList()
{
    std::for_each(drawBorderPolyLineList_.begin(), drawBorderPolyLineList_.end(), PointerDestroyer());
    drawBorderPolyLineList_.clear();
    drawBorderPolyLineListDirty_ = true;
}

void NFmiCountryBorderPolylineCache::drawBorderPolyLineList(std::list<NFmiPolyline*>& newPolyLineList)
{
    // First destroy the content of the old list
    std::for_each(drawBorderPolyLineList_.begin(), drawBorderPolyLineList_.end(), PointerDestroyer());
    drawBorderPolyLineList_.clear();
    // Take ownership of the new list's content with a swap
    drawBorderPolyLineList_.swap(newPolyLineList);
    // Mark the polyline list as 'clean' again, i.e. ready for use
    drawBorderPolyLineListDirty_ = false;
}

void NFmiCountryBorderPolylineCache::drawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>>& newPolyLineList)
{
    drawBorderPolyLineListGdiplus_ = newPolyLineList;
    drawBorderPolyLineListGdiplusDirty_ = false;
}

void NFmiCountryBorderPolylineCache::drawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>>&& newPolyLineList)
{
    drawBorderPolyLineListGdiplus_ = std::move(newPolyLineList);
    drawBorderPolyLineListGdiplusDirty_ = false;
}

// The new border-draw-dirty system does not set flags on, but clears the necessary caches
// so that on the next drawing round work has to be done.
void NFmiCountryBorderPolylineCache::setBorderDrawDirtyState(CountryBorderDrawDirtyState newState)
{
    if(newState == CountryBorderDrawDirtyState::Geometry)
    {
        // Cosmetic changes do not rebuild the polylines, but all geometric changes do
        clearDrawBorderPolyLineList();
        clearDrawBorderPolyLineListGdiplus();
    }
}

void NFmiCountryBorderPolylineCache::clearDrawBorderPolyLineListGdiplus()
{
    drawBorderPolyLineListGdiplus_.clear();
    drawBorderPolyLineListGdiplusDirty_ = true;
}

bool NFmiCountryBorderPolylineCache::borderDrawPolylinesDirty() const
{
    return drawBorderPolyLineListDirty_;
}

bool NFmiCountryBorderPolylineCache::borderDrawPolylinesGdiplusDirty() const
{
    return drawBorderPolyLineListGdiplusDirty_;
}

std::list<NFmiPolyline*>& NFmiCountryBorderPolylineCache::drawBorderPolyLineList()
{ 
    return drawBorderPolyLineList_;
}

const std::list<std::vector<NFmiPoint>>& NFmiCountryBorderPolylineCache::drawBorderPolyLineListGdiplus()
{ 
    return drawBorderPolyLineListGdiplus_;
}
