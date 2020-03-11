#pragma once

#include "WmsSupportDefines.h"
#include "NFmiDataIdent.h"
#include <map>
#include <set>

namespace Wms
{
    class LegendHandler
    {
        std::map<int, std::map<int, std::map<int, LegendIdentSet>>> registeredLayers_;
    public:
        LegendIdentSet getLegends(int row, int col, int descTop);
        void registerLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent);
        void unregisterLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent);
        const LegendIdentSet& getRegisteredLayers(int row, int col, int descTop);

    private:
        bool exists(int row, int col, int descTop);
    };
}