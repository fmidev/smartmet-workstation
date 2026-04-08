#ifndef UNIX
#include "stdafx.h"
#endif // UNIX
#include "ApplicationInterface.h"
#include "ToolmasterHatchPolygonData.h"
#ifndef UNIX
#include "NFmiApplicationWinRegistry.h"
#endif // UNIX

#ifndef UNIX
CSmartMetView* ApplicationInterface::itsSmartMetView = nullptr;
CView* ApplicationInterface::itsSmartMetViewAsCView = nullptr;
#endif // UNIX
ApplicationInterface::GetApplicationInterfaceImplementationCallBackType ApplicationInterface::GetApplicationInterfaceImplementation;

#ifndef UNIX
void ApplicationInterface::SetSmartMetView(CSmartMetView *view)
{
    ApplicationInterface::itsSmartMetView = view;
}

void ApplicationInterface::SetSmartMetViewAsCView(CView *view)
{
    ApplicationInterface::itsSmartMetViewAsCView = view;
}

CSmartMetView* ApplicationInterface::GetSmartMetView()
{
    return ApplicationInterface::itsSmartMetView;
}

CView* ApplicationInterface::GetSmartMetViewAsCView()
{
    return ApplicationInterface::itsSmartMetViewAsCView;
}
#endif // UNIX

void ApplicationInterface::SetHatchingToolmasterEpsilonFactor(float newEpsilonFactor)
{
#ifndef DISABLE_UNIRAS_TOOLMASTER
    ToolmasterHatchPolygonData::toolmasterRelatedBigEpsilonFactor_ = newEpsilonFactor;
#else
    (void)newEpsilonFactor;
#endif
}

void ApplicationInterface::SetHatchingDebuggingPolygonIndex(int action)
{
#ifndef DISABLE_UNIRAS_TOOLMASTER
    switch(action)
    {
    case 1:
        ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex1_--;
        break;
    case 2:
        ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex1_++;
        break;
    case 3:
        ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex1_ = 10;
        break;
    case 4:
        ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex2_--;
        break;
    case 5:
        ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex2_++;
        break;
    case 6:
        ToolmasterHatchPolygonData::debugHelperWantedPolygonIndex2_ = 12;
        break;
    case 7:
        AddToHatchingToolmasterEpsilonFactor(-0.05f);
        break;
    case 8:
        AddToHatchingToolmasterEpsilonFactor(0.05f);
        break;
    default:
        break;
    }
#else
    (void)action;
#endif
}

void ApplicationInterface::AddToHatchingToolmasterEpsilonFactor(float addedValue)
{
#ifndef UNIX
    auto& winRegistry = ApplicationWinRegistry();
    auto factor = winRegistry.HatchingToolmasterEpsilonFactor();
    factor -= addedValue;
    winRegistry.HatchingToolmasterEpsilonFactor(factor);
    SetHatchingToolmasterEpsilonFactor(factor);
#else
    (void)addedValue;
#endif // UNIX
}

