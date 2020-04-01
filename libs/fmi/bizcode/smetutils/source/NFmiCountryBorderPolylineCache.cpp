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
        // Kopioinnissa pitää vain nollata cachet.
        clearCache();
        // offset pointeja on myös turha kopioida tai nollata
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
    // tuhotaan ensin vanhan listan sisältö
    std::for_each(drawBorderPolyLineList_.begin(), drawBorderPolyLineList_.end(), PointerDestroyer());
    drawBorderPolyLineList_.clear();
    // otetaan uuden listan sisältö omistukseen swap:illa
    drawBorderPolyLineList_.swap(newPolyLineList);
    // Merkitään polyline lista taas 'puhtaaksi' eli käyttövalmiiksi
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

// Uusi border-draw-dirty systeemi ei laita lippuja päälle, vaan tyhjentää tarvittavat cachet
// jotta seuraavalla piirto kerralla on pakko tehdä töitä.
void NFmiCountryBorderPolylineCache::setBorderDrawDirtyState(CountryBorderDrawDirtyState newState)
{
    if(newState == CountryBorderDrawDirtyState::Geometry)
    {
        // Kosmeettiset muutokset eivät laita polylineja uusiksi, mutta kaikki geometriset muutokset kyllä
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
