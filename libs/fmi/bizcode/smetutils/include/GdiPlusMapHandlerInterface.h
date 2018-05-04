#pragma once

#include "MapHandlerInterface.h"

class NFmiGdiPlusImageMapHandler;

class GdiPlusMapHandlerInterface : public MapHandlerInterface
{
    NFmiGdiPlusImageMapHandler *itsMapHandler; // ei omista, ei tuhoa
public:
    GdiPlusMapHandlerInterface(NFmiGdiPlusImageMapHandler *theMapHandler);
	~GdiPlusMapHandlerInterface() = default;

    boost::shared_ptr<NFmiArea> Area() override;
    Gdiplus::Bitmap* GetBitmap() override;
    NFmiRect TotalAbsolutRect() override;
    boost::shared_ptr<NFmiArea> TotalArea() override;
    bool SetMaxArea() override;
    void MapDirty(bool newState) override;
    bool IsMapDirty(void) override;
    const NFmiRect& Position() override;
    int UsedMapIndex() override;
    bool MapReallyChanged() override;
    void MapReallyChanged(bool newState) override;
    bool ShowOverMap(void) override;
    Gdiplus::Bitmap* GetOverMapBitmap(void) override;
    NFmiRect ZoomedAbsolutRectOverMap(void) override;
    NFmiRect ZoomedAbsolutRect(void) override;

};
