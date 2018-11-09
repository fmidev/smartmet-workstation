#pragma once

#include "NFmiGlobals.h"
#include "NFmiRect.h"

class NFmiToolBox;

// Luokka asettaa konstruktorissa palauttaa tiettyjä asetuksia, ottaen vanhat
// asetukset talteen ja sitten destruktorissa palauttaa vanhat asetukset takaisin.
class ToolBoxStateRestorer
{
    FmiDirection oldAligment_ = kNoDirection;
    bool oldUseClipping_ = false;
    NFmiRect oldClippingRect_;
    bool restoreClippingRect_ = false;
    NFmiToolBox &toolBox_;
public:
    ToolBoxStateRestorer(NFmiToolBox &toolBox, FmiDirection newAligment, bool useClipping, const NFmiRect *possibleNewClippingRect = nullptr);
    ~ToolBoxStateRestorer();
};
