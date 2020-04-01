#include "ToolboxViewsInterfaceForGeneralDataDoc.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiMapViewDescTop.h"
#include "CombinedMapHandlerInterface.h"

ToolboxViewsInterfaceForGeneralDataDoc::ToolboxViewsInterfaceForGeneralDataDoc(NFmiEditMapGeneralDataDoc *theDoc)
    :itsDoc(theDoc)
{

}

ToolboxViewsInterfaceForGeneralDataDoc::~ToolboxViewsInterfaceForGeneralDataDoc() = default;

float ToolboxViewsInterfaceForGeneralDataDoc::TimeControlTimeStepInHours(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->TimeControlTimeStep();
}

double ToolboxViewsInterfaceForGeneralDataDoc::ContextPixelsPerMM_x(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getGraphicalInfo(theMapViewDescTopIndex).itsPixelsPerMM_x;
}

const NFmiPoint& ToolboxViewsInterfaceForGeneralDataDoc::MapViewGridSize(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->ViewGridSize();
}

CtrlViewUtils::MapViewMode ToolboxViewsInterfaceForGeneralDataDoc::MapViewDisplayMode(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->MapViewDisplayMode();
}

const NFmiMetTime& ToolboxViewsInterfaceForGeneralDataDoc::ActiveMapTime()
{
    return itsDoc->GetCombinedMapHandler()->activeMapTime();
}

const NFmiMetTime& ToolboxViewsInterfaceForGeneralDataDoc::CurrentMapTime(int theMapViewDescTopIndex)
{
    return itsDoc->GetCombinedMapHandler()->currentTime(theMapViewDescTopIndex);
}

FmiLanguage ToolboxViewsInterfaceForGeneralDataDoc::Language()
{
    return itsDoc->Language();
}

