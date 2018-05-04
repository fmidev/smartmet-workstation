#pragma once

#include "NFmiRect.h"
#include "boost/shared_ptr.hpp"

class NFmiArea;
namespace Gdiplus
{
    class Bitmap;
}
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

class MapHandlerInterface
{
public:
	virtual ~MapHandlerInterface();

    virtual boost::shared_ptr<NFmiArea> Area() = 0;
    virtual Gdiplus::Bitmap* GetBitmap() = 0;
    virtual NFmiRect TotalAbsolutRect() = 0;
    virtual boost::shared_ptr<NFmiArea> TotalArea() = 0;
    virtual bool SetMaxArea() = 0;
    virtual void MapDirty(bool newState) = 0;
    virtual bool IsMapDirty(void) = 0;
    virtual const NFmiRect& Position() = 0;
    virtual int UsedMapIndex() = 0;
    virtual bool MapReallyChanged() = 0;
    virtual void MapReallyChanged(bool newState) = 0;
    virtual bool ShowOverMap(void) = 0;
    virtual Gdiplus::Bitmap* GetOverMapBitmap(void) = 0;
    virtual NFmiRect ZoomedAbsolutRectOverMap(void) = 0;
    virtual NFmiRect ZoomedAbsolutRect(void) = 0;
};
