#pragma once

#include "WmsQuery.h"

#include "NFmiProducer.h"

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

    struct Capability
    {
        NFmiProducer producer;
        long paramId;
        std::string name;
    };

    using CapabilityLeaf = cppext::Leaf<Capability>;
    using CapabilityNode = cppext::Node<Capability>;
    using CapabilityTree = cppext::Tree<Capability>;

    void insertLeaf(CapabilityNode& tree, const CapabilityLeaf& leaf, std::list<std::string>& path);

}