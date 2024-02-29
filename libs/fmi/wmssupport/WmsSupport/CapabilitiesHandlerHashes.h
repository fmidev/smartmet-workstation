#pragma once

#include "wmssupport/ChangedLayers.h"

#include <map>

namespace Wms
{
    class CapabilitiesHandlerHashes
    {
    private:
        std::map<long, std::map<long, LayerInfo>> hashes_;
    public:
        CapabilitiesHandlerHashes();
        CapabilitiesHandlerHashes(const CapabilitiesHandlerHashes &other);

        LayerInfo getLayerInfo(const NFmiDataIdent &dataIdent) const;
        const std::map<long, std::map<long, LayerInfo>>& peekHashes() const { return hashes_; }
        std::map<long, std::map<long, LayerInfo>>& getHashes() { return hashes_; }
    };
}
