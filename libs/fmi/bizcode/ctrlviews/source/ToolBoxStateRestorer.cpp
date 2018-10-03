#include "ToolBoxStateRestorer.h"
#include "NFmiToolBox.h"


ToolBoxStateRestorer::ToolBoxStateRestorer(NFmiToolBox &toolBox, FmiDirection newAligment)
:toolBox_(toolBox)
{
    oldAligment_ = toolBox_.GetTextAlignment();
    toolBox_.SetTextAlignment(newAligment);
}

ToolBoxStateRestorer::~ToolBoxStateRestorer()
{
    toolBox_.SetTextAlignment(oldAligment_);
}
