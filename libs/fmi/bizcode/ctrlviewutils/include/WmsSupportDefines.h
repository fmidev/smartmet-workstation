#pragma once

#include "NFmiProducer.h"
#include "NFmiDataIdent.h"

#include <string>
#include <set>

#ifndef DISABLE_CPPRESTSDK

#include <cppext/tree.h>

namespace Wms
{
    struct Capability
    {
        NFmiProducer producer;
        long paramId;
        std::string name;
        bool hasTimeDimension = false;
    };

    using CapabilityLeaf = cppext::Leaf<Capability>;
    using CapabilityNode = cppext::Node<Capability>;
    using CapabilityTree = cppext::Tree<Capability>;

    struct CustomIdentCompare 
    {
        bool operator() (const NFmiDataIdent& el1, const NFmiDataIdent& el2) const;
    };

    using LegendIdentSet = std::set<NFmiDataIdent, CustomIdentCompare>;
}
#endif // DISABLE_CPPRESTSDK
