#pragma once

#include "NFmiPoint.h"
#include "CombinedMapHandlerInterface.h"

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
    std::list<std::vector<NFmiPoint>> drawBorderPolyLineListGdiplus_;
    // Sama selitys kuin fDrawBorderPolyLineListSet:in kanssa edell‰, mutta koskien itsDrawBorderPolyLineListGdiplus -listan likaisuutta.
    bool drawBorderPolyLineListGdiplusDirty_ = true;
public:
    NFmiCountryBorderPolylineCache();
    NFmiCountryBorderPolylineCache(const NFmiCountryBorderPolylineCache& other);
    NFmiCountryBorderPolylineCache& operator=(const NFmiCountryBorderPolylineCache &other);
    ~NFmiCountryBorderPolylineCache();

    void clearCache();
    void drawBorderPolyLineList(std::list<NFmiPolyline*>& newPolyLineList);
    void drawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>>& newPolyLineList);
    void drawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>>&& newPolyLineList);
    void setBorderDrawDirtyState(CountryBorderDrawDirtyState newState);
    bool borderDrawPolylinesDirty() const;
    bool borderDrawPolylinesGdiplusDirty() const;
    std::list<NFmiPolyline*>& drawBorderPolyLineList();
    const NFmiPoint& relativeDrawingOffset() const { return relativeDrawingOffset_; }
    const std::list<std::vector<NFmiPoint>>& drawBorderPolyLineListGdiplus();

private:
    void clearDrawBorderPolyLineList();
    void clearDrawBorderPolyLineListGdiplus();
};
