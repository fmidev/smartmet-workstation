#pragma once

#include "NFmiPoint.h"
#include "CombinedMapHandlerInterface.h"
#include <gdiplus.h>

#include <list>

class NFmiPolyline;

// Luokka pit‰‰ sis‰ll‰‰n maiden rajaviivan piirtoon liittyvi‰ polkuja ja hanskaa niiden cachetusta.
class NFmiCountryBorderPolylineCache
{
    // Optimointia: kun stationviewhandler on tehnyt piirtolistan valmiiksi, se talletetaan dokumenttiin ja sen piirto on nopeaa kun kaikki konversiot on valmikksi tehty	
    std::list<NFmiPolyline*> drawBorderPolyLineList_;
    // Miss‰ kohtaa suhteellista n‰yttˆ‰ 'originaali' drawBorderPolyLineList_ on piirretty,
    // t‰m‰n avulla piirto voidaan kohdentaa tulos bitmappiin, oli se miss‰ kohtaa tahansa karttan‰ytˆll‰.
    NFmiPoint relativeDrawingOffset_;
    // Kun kysyt‰‰n ett‰ BorderDrawPolylinesDirty(), tarkistetaan onko itsDrawBorderPolyLineList tyhj‰ vai ei.
    // Mutta jos laskettu polyline onkin tyhj‰, pit‰‰ olla mekanismi, jolla voidaan tarkistaa ett‰ vaikka polyline-lista onkin tyhj‰,
    // mutta silti listaa ei tarvitse laskea uudestaan t‰m‰ lipun avulla.
    bool drawBorderPolyLineListDirty_ = true;
    std::list<std::vector<Gdiplus::PointF>> polyLineListGdiplusInPixelCoordinates_;
    // Sama selitys kuin drawBorderPolyLineListDirty_:in kanssa edell‰, mutta koskien polyLineListGdiplusInPixelCoordinates_ -listan likaisuutta.
    bool polyLineListGdiplusInPixelCoordinatesDirty_ = true;
public:
    NFmiCountryBorderPolylineCache();
    NFmiCountryBorderPolylineCache(const NFmiCountryBorderPolylineCache& other);
    NFmiCountryBorderPolylineCache& operator=(const NFmiCountryBorderPolylineCache &other);
    ~NFmiCountryBorderPolylineCache();

    void clearCache();
    void drawBorderPolyLineList(std::list<NFmiPolyline*>& newPolyLineList);
    void polyLineListGdiplusInPixelCoordinates(const std::list<std::vector<Gdiplus::PointF>> &polyLineList);
    void polyLineListGdiplusInPixelCoordinates(const std::list<std::vector<Gdiplus::PointF>> &&polyLineList);
    void setBorderDrawDirtyState(CountryBorderDrawDirtyState newState);
    bool borderDrawPolylinesDirty() const;
    bool polyLineListGdiplusInPixelCoordinatesDirty() const;
    std::list<NFmiPolyline*>& drawBorderPolyLineList();
    const NFmiPoint& relativeDrawingOffset() const { return relativeDrawingOffset_; }
    const std::list<std::vector<Gdiplus::PointF>>& polyLineListGdiplusInPixelCoordinates();

private:
    void clearDrawBorderPolyLineList();
    void clearPolyLineListGdiplusInPixelCoordinates();
};
