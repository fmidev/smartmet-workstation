#pragma once

#include "NFmiGlobals.h"

class NFmiToolBox;

// Luokka asettaa konstruktorissa palauttaa tiettyjä asetuksia, ottaen vanhat
// asetukset talteen ja sitten destruktorissa palauttaa vanhat asetukset takaisin.
class ToolBoxStateRestorer
{
    FmiDirection oldAligment_ = kNoDirection;
    bool oldUseClipping_ = false;
    NFmiToolBox &toolBox_;
public:
    ToolBoxStateRestorer(NFmiToolBox &toolBox, FmiDirection newAligment, bool useClipping);
    ~ToolBoxStateRestorer();
};
