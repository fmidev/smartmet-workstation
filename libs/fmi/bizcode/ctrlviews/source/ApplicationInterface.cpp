#include "stdafx.h"
#include "ApplicationInterface.h"
#include "ToolmasterHatchPolygonData.h"

CSmartMetView* ApplicationInterface::itsSmartMetView = nullptr;
CView* ApplicationInterface::itsSmartMetViewAsCView = nullptr;
ApplicationInterface::GetApplicationInterfaceImplementationCallBackType ApplicationInterface::GetApplicationInterfaceImplementation;

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

void ApplicationInterface::SetHatchingToolmasterEpsilonFactor(float newEpsilonFactor)
{
    ToolmasterHatchPolygonData::toolmasterRelatedBigEpsilonFactor_ = newEpsilonFactor;
}

void ApplicationInterface::SetHatchingDebuggingPolygonIndex(int action)
{
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
    default:
        break;
    }
}
