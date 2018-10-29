#include "ToolBoxStateRestorer.h"
#include "NFmiToolBox.h"


ToolBoxStateRestorer::ToolBoxStateRestorer(NFmiToolBox &toolBox, FmiDirection newAligment, bool useClipping)
:toolBox_(toolBox)
{
    oldAligment_ = toolBox_.GetTextAlignment();
    toolBox_.SetTextAlignment(newAligment);
    oldUseClipping_ = toolBox_.UseClipping();
    toolBox_.UseClipping(useClipping);
}

ToolBoxStateRestorer::~ToolBoxStateRestorer()
{
    toolBox_.SetTextAlignment(oldAligment_);
    toolBox_.UseClipping(oldUseClipping_);
}
