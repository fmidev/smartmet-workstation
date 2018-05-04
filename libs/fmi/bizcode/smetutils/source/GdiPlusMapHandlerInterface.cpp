#include "GdiPlusMapHandlerInterface.h"
#include "NFmiGdiPlusImageMapHandler.h"

GdiPlusMapHandlerInterface::GdiPlusMapHandlerInterface(NFmiGdiPlusImageMapHandler *theMapHandler)
    :itsMapHandler(theMapHandler)
{}

boost::shared_ptr<NFmiArea> GdiPlusMapHandlerInterface::Area()
{
    return itsMapHandler->Area();
}

Gdiplus::Bitmap* GdiPlusMapHandlerInterface::GetBitmap()
{
    return itsMapHandler->GetBitmap();
}

NFmiRect GdiPlusMapHandlerInterface::TotalAbsolutRect()
{
    return itsMapHandler->TotalAbsolutRect();
}

boost::shared_ptr<NFmiArea> GdiPlusMapHandlerInterface::TotalArea()
{
    return itsMapHandler->TotalArea();
}

bool GdiPlusMapHandlerInterface::SetMaxArea()
{
    return itsMapHandler->SetMaxArea();
}

void GdiPlusMapHandlerInterface::MapDirty(bool newState)
{
    itsMapHandler->MapDirty(newState);
}

bool GdiPlusMapHandlerInterface::IsMapDirty(void)
{
    return itsMapHandler->IsMapDirty();
}

const NFmiRect& GdiPlusMapHandlerInterface::Position()
{
    return itsMapHandler->Position();
}

int GdiPlusMapHandlerInterface::UsedMapIndex()
{
    return itsMapHandler->UsedMapIndex();
}

bool GdiPlusMapHandlerInterface::MapReallyChanged()
{
    return itsMapHandler->MapReallyChanged();
}

void GdiPlusMapHandlerInterface::MapReallyChanged(bool newState)
{
    itsMapHandler->MapReallyChanged(newState);
}

bool GdiPlusMapHandlerInterface::ShowOverMap(void)
{
    return itsMapHandler->ShowOverMap();
}

Gdiplus::Bitmap* GdiPlusMapHandlerInterface::GetOverMapBitmap(void)
{
    return itsMapHandler->GetOverMapBitmap();
}

NFmiRect GdiPlusMapHandlerInterface::ZoomedAbsolutRectOverMap(void)
{
    return itsMapHandler->ZoomedAbsolutRectOverMap();
}

NFmiRect GdiPlusMapHandlerInterface::ZoomedAbsolutRect(void)
{
    return itsMapHandler->ZoomedAbsolutRect();
}

