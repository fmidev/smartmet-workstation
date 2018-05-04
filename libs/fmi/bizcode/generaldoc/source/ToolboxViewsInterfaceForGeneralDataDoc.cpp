#include "ToolboxViewsInterfaceForGeneralDataDoc.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiMapViewDescTop.h"

ToolboxViewsInterfaceForGeneralDataDoc::ToolboxViewsInterfaceForGeneralDataDoc(NFmiEditMapGeneralDataDoc *theDoc)
    :itsDoc(theDoc)
{

}

ToolboxViewsInterfaceForGeneralDataDoc::~ToolboxViewsInterfaceForGeneralDataDoc() = default;

float ToolboxViewsInterfaceForGeneralDataDoc::TimeControlTimeStepInHours(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->TimeControlTimeStep();
}

double ToolboxViewsInterfaceForGeneralDataDoc::ContextPixelsPerMM_x(int theMapViewDescTopIndex)
{
    return itsDoc->GetGraphicalInfo(theMapViewDescTopIndex).itsPixelsPerMM_x;
}

const NFmiPoint& ToolboxViewsInterfaceForGeneralDataDoc::MapViewGridSize(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->ViewGridSize();
}

CtrlViewUtils::MapViewMode ToolboxViewsInterfaceForGeneralDataDoc::MapViewDisplayMode(int theMapViewDescTopIndex)
{
    return itsDoc->MapViewDescTop(theMapViewDescTopIndex)->MapViewDisplayMode();
}

const NFmiMetTime& ToolboxViewsInterfaceForGeneralDataDoc::ActiveMapTime()
{
    return itsDoc->ActiveMapTime();
}

const NFmiMetTime& ToolboxViewsInterfaceForGeneralDataDoc::CurrentMapTime(int theMapViewDescTopIndex)
{
    return itsDoc->CurrentTime(theMapViewDescTopIndex);
}

FmiLanguage ToolboxViewsInterfaceForGeneralDataDoc::Language()
{
    return itsDoc->Language();
}

