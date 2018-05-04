#pragma once

#include "ToolboxViewsInterface.h"

class NFmiEditMapGeneralDataDoc;

class ToolboxViewsInterfaceForGeneralDataDoc : public ToolboxViewsInterface
{
    NFmiEditMapGeneralDataDoc *itsDoc;
public:
    ToolboxViewsInterfaceForGeneralDataDoc(NFmiEditMapGeneralDataDoc *theDoc);
    virtual ~ToolboxViewsInterfaceForGeneralDataDoc();

    float TimeControlTimeStepInHours(int theMapViewDescTopIndex) override;
    double ContextPixelsPerMM_x(int theMapViewDescTopIndex) override;
    const NFmiPoint& MapViewGridSize(int theMapViewDescTopIndex) override;
    CtrlViewUtils::MapViewMode MapViewDisplayMode(int theMapViewDescTopIndex) override;
    const NFmiMetTime& ActiveMapTime() override;
    const NFmiMetTime& CurrentMapTime(int theMapViewDescTopIndex) override;
    FmiLanguage Language() override;
};
