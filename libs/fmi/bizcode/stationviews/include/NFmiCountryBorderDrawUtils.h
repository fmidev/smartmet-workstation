#pragma once

#include "NFmiPoint.h"

class NFmiCtrlView;
class NFmiToolBox;

class NFmiCountryBorderDrawUtils
{
public:
    static void drawCountryBordersToMapView(NFmiCtrlView* mapView, NFmiToolBox* toolbox);
};