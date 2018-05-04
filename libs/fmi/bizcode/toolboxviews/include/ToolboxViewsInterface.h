#pragma once

#include "NFmiGlobals.h"
#include "MapViewMode.h"
#include <functional>

class NFmiPoint;
class NFmiMetTime;

class ToolboxViewsInterface
{
public:
    virtual ~ToolboxViewsInterface();

    virtual float TimeControlTimeStepInHours(int theMapViewDescTopIndex) = 0;
    virtual double ContextPixelsPerMM_x(int theMapViewDescTopIndex) = 0;
    virtual const NFmiPoint& MapViewGridSize(int theMapViewDescTopIndex) = 0;
    virtual CtrlViewUtils::MapViewMode MapViewDisplayMode(int theMapViewDescTopIndex) = 0;
    virtual const NFmiMetTime& ActiveMapTime() = 0;
    virtual const NFmiMetTime& CurrentMapTime(int theMapViewDescTopIndex) = 0;
    virtual FmiLanguage Language() = 0;

    using GetToolboxViewsInterfaceCallBackType = std::function<ToolboxViewsInterface*(void)>;
    // Tämä pitää asettaa johonkin konkreettiseen funktioon, jotta käyttäjä koodi saa käyttöönsä halutun interface toteutuksen
    static GetToolboxViewsInterfaceCallBackType GetToolboxViewsInterfaceCallBack;
};
