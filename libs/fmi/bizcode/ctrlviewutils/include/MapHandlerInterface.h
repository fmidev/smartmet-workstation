#pragma once

#include "NFmiRect.h"
#include "boost/shared_ptr.hpp"

class NFmiArea;
#ifndef UNIX
namespace Gdiplus
{
    class Bitmap;
}
#endif // UNIX
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

class MapHandlerInterface
{
public:
	virtual ~MapHandlerInterface();

    virtual boost::shared_ptr<NFmiArea> Area() = 0;
#ifndef UNIX
    virtual Gdiplus::Bitmap* GetBitmap() = 0;
#endif // UNIX
    virtual NFmiRect TotalAbsolutRect() = 0;
    virtual boost::shared_ptr<NFmiArea> TotalArea() = 0;
    virtual bool SetMaxArea() = 0;
    virtual void SetMakeNewBackgroundBitmap(bool newState) = 0;
    virtual bool MakeNewBackgroundBitmap() = 0;
    virtual void ClearMakeNewBackgroundBitmap() = 0;
    virtual bool UpdateMapViewDrawingLayers() = 0;
    virtual const NFmiRect& Position() = 0;
    virtual int UsedMapIndex() = 0;
    virtual bool MapReallyChanged() = 0;
    virtual void MapReallyChanged(bool newState) = 0;
    virtual bool ShowOverMap(void) = 0;
#ifndef UNIX
    virtual Gdiplus::Bitmap* GetOverMapBitmap(void) = 0;
#endif // UNIX
    virtual NFmiRect ZoomedAbsolutRectOverMap(void) = 0;
    virtual NFmiRect ZoomedAbsolutRect(void) = 0;
};
