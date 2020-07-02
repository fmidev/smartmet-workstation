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
        // Kopioinnissa pit�� vain nollata cachet.
        clearCache();
        // offset pointeja on my�s turha kopioida tai nollata
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
    clearPolyLineListGdiplusInPixelCoordinates();
}

void NFmiCountryBorderPolylineCache::clearDrawBorderPolyLineList()
{
    std::for_each(drawBorderPolyLineList_.begin(), drawBorderPolyLineList_.end(), PointerDestroyer());
    drawBorderPolyLineList_.clear();
    drawBorderPolyLineListDirty_ = true;
}

void NFmiCountryBorderPolylineCache::drawBorderPolyLineList(std::list<NFmiPolyline*>& newPolyLineList)
{
    // tuhotaan ensin vanhan listan sis�lt�
    std::for_each(drawBorderPolyLineList_.begin(), drawBorderPolyLineList_.end(), PointerDestroyer());
    drawBorderPolyLineList_.clear();
    // otetaan uuden listan sis�lt� omistukseen swap:illa
    drawBorderPolyLineList_.swap(newPolyLineList);
    // Merkit��n polyline lista taas 'puhtaaksi' eli k�ytt�valmiiksi
    drawBorderPolyLineListDirty_ = false;
}

void NFmiCountryBorderPolylineCache::polyLineListGdiplusInPixelCoordinates(const std::list<std::vector<Gdiplus::PointF>>& polyLineList)
{
    polyLineListGdiplusInPixelCoordinates_ = polyLineList;
    polyLineListGdiplusInPixelCoordinatesDirty_ = false;
}

void NFmiCountryBorderPolylineCache::polyLineListGdiplusInPixelCoordinates(const std::list<std::vector<Gdiplus::PointF>> && polyLineList)
{
    polyLineListGdiplusInPixelCoordinates_ = std::move(polyLineList);
    polyLineListGdiplusInPixelCoordinatesDirty_ = false;
}

// Uusi border-draw-dirty systeemi ei laita lippuja p��lle, vaan tyhjent�� tarvittavat cachet
// jotta seuraavalla piirto kerralla on pakko tehd� t�it�.
void NFmiCountryBorderPolylineCache::setBorderDrawDirtyState(CountryBorderDrawDirtyState newState)
{
    if(newState == CountryBorderDrawDirtyState::Geometry)
    {
        // Kosmeettiset muutokset eiv�t laita polylineja uusiksi, mutta kaikki geometriset muutokset kyll�
        clearDrawBorderPolyLineList();
        clearPolyLineListGdiplusInPixelCoordinates();
    }
}

void NFmiCountryBorderPolylineCache::clearPolyLineListGdiplusInPixelCoordinates()
{
    polyLineListGdiplusInPixelCoordinates_.clear();
    polyLineListGdiplusInPixelCoordinatesDirty_ = true;
}

bool NFmiCountryBorderPolylineCache::borderDrawPolylinesDirty() const
{
    return drawBorderPolyLineListDirty_;
}

bool NFmiCountryBorderPolylineCache::polyLineListGdiplusInPixelCoordinatesDirty() const
{
    return polyLineListGdiplusInPixelCoordinatesDirty_;
}

std::list<NFmiPolyline*>& NFmiCountryBorderPolylineCache::drawBorderPolyLineList()
{ 
    return drawBorderPolyLineList_;
}

const std::list<std::vector<Gdiplus::PointF>>& NFmiCountryBorderPolylineCache::polyLineListGdiplusInPixelCoordinates()
{ 
    return polyLineListGdiplusInPixelCoordinates_;
}
