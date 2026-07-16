#pragma once

#include "NFmiPoint.h"
#include "CombinedMapHandlerInterface.h"

#include <list>

class NFmiPolyline;

// This class holds the paths related to drawing country border lines and handles their caching.
class NFmiCountryBorderPolylineCache
{
    // Optimization: once the stationviewhandler has finished building the draw list, it is stored in the document and its drawing is fast because all conversions are already done
    std::list<NFmiPolyline*> drawBorderPolyLineList_;
    // At which position of the relative view the 'original' drawBorderPolyLineList_ has been drawn;
    // with this the drawing can be targeted onto the result bitmap, wherever it is on the map view.
    NFmiPoint relativeDrawingOffset_;
    // When BorderDrawPolylinesDirty() is queried, it checks whether itsDrawBorderPolyLineList is empty or not.
    // But if the computed polyline happens to be empty, there must be a mechanism to check that even though the polyline list is empty,
    // the list still does not need to be recomputed - this flag serves that purpose.
    bool drawBorderPolyLineListDirty_ = true;
    std::list<std::vector<NFmiPoint>> drawBorderPolyLineListGdiplus_;
    // Same explanation as with fDrawBorderPolyLineListSet above, but concerning the dirtiness of the itsDrawBorderPolyLineListGdiplus list.
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
