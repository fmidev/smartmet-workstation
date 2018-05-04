#pragma once

#include "NFmiGlobals.h"

class NFmiCtrlView;
class NFmiRect;
class NFmiToolBox;

namespace StationViews
{
    void PlaceBoxIntoFrame(NFmiRect &theObjectBoxAbsolute, const NFmiRect &theViewFrameRelative, NFmiToolBox *theToolbox, FmiDirection theWantedLocation);
    void DrawBetaProductParamBox(NFmiCtrlView *view, bool fCrossSectionInfoWanted);

}
