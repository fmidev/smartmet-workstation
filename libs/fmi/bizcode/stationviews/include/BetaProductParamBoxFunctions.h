#pragma once

#include "NFmiGlobals.h"

class NFmiCtrlView;
class NFmiRect;
class NFmiToolBox;
class NFmiBetaProduct;
class CtrlViewDocumentInterface;

namespace StationViews
{
    void DrawBetaProductParamBox(NFmiCtrlView *view, bool fCrossSectionInfoWanted, const NFmiBetaProduct* optionalBetaProduct);
    const NFmiBetaProduct& GetPrintingBetaProductForParamBoxDraw(int viewDesctopIndex, CtrlViewDocumentInterface& ctrlViewDocumentInterface);
}
