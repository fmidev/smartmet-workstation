#pragma once
#include "Setup.h"

#include "NFmiSettings.h"

#include <cppext/split.h>

#include <string>
#include <utility>
#include <chrono>


namespace Wms
{
    class SetupParser
    {
    public:
        static Setup parse();
    };
}

