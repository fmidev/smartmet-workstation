#include "ToolBoxStateRestorer.h"
#include "NFmiToolBox.h"


ToolBoxStateRestorer::ToolBoxStateRestorer(NFmiToolBox &toolBox, FmiDirection newAligment, bool useClipping, const NFmiRect *possibleNewClippingRect)
:toolBox_(toolBox)
,oldClippingRect_()
{
    oldAligment_ = toolBox_.GetTextAlignment();
    toolBox_.SetTextAlignment(newAligment);
    oldUseClipping_ = toolBox_.UseClipping();
    toolBox_.UseClipping(useClipping);
    if(possibleNewClippingRect)
    {
        restoreClippingRect_ = true;
        oldClippingRect_ = toolBox_.RelativeClipRect();
        toolBox_.RelativeClipRect(*possibleNewClippingRect, useClipping);
    }
}

ToolBoxStateRestorer::~ToolBoxStateRestorer()
{
    toolBox_.SetTextAlignment(oldAligment_);
    if(restoreClippingRect_)
    {
        toolBox_.RelativeClipRect(oldClippingRect_, oldUseClipping_);
    }
    else
    {
        toolBox_.UseClipping(oldUseClipping_);
    }
}
