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

void ApplicationInterface::SetHatchingToolmasterEpsilonFactor(float newEpsilon)
{
    ToolmasterHatchPolygonData::toolmasterRelatedBigEpsilon_ = newEpsilon;
}
