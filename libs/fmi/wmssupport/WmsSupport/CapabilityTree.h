#pragma once

#include "wmssupport/WmsQuery.h"
#include "NFmiProducer.h"
#include "WmsSupportDefines.h"

#include <cppext/tree.h>

#include <string>
#include <list>

namespace Wms
{
    struct Style
    {
        std::string name;
        std::string legendDomain;
        std::string legendRequest;
    };

    inline bool operator<(const Style& style1, const Style& style2)
    {
        return style1.name < style2.name;
    }

    void insertLeaf(CapabilityNode& tree, const CapabilityLeaf& leaf, std::list<std::string>& path);

}