#pragma once

#include "webclient/Client.h"
#include "bitmaphandler/BitmapParser.h"

#include <map>
#include <set>
#include <utility>

namespace Wms
{
    struct CustomIdentCompare {
        bool operator() (const NFmiDataIdent& el1, const NFmiDataIdent& el2) const
        {
            auto producerId1 = el1.GetProducer()->GetIdent();
            auto producerId2 = el2.GetProducer()->GetIdent();

            auto paramId1 = el1.GetParamIdent();
            auto paramId2 = el2.GetParamIdent();

            if(producerId1 == producerId2)
            {
                return paramId1 < paramId2;
            }
            return producerId1 < producerId2;
        }
    };

    using LegendIdentSet = std::set<NFmiDataIdent, CustomIdentCompare>;

    class LegendHandler
    {
        std::map<int, std::map<int, std::map<int, LegendIdentSet>>> registeredLayers_;
    public:
        LegendIdentSet getLegends(int row, int col, int descTop)
        {
            if(exists(row, col, descTop))
            {
                return registeredLayers_.at(descTop).at(row).at(col);
            }
            else
            {
                return LegendIdentSet{};
            }
        }

        void registerLayer(int row, int col, int descTop, NFmiDataIdent dataIdent)
        {
            if(registeredLayers_.find(descTop) == registeredLayers_.cend())
            {
                registeredLayers_[descTop] = std::map<int, std::map<int, LegendIdentSet>>{};
            }
            if(registeredLayers_.at(descTop).find(row) == registeredLayers_.at(descTop).cend())
            {
                registeredLayers_.at(descTop)[row] = std::map<int, LegendIdentSet>{};
            }
            if(registeredLayers_.at(descTop).at(row).find(col) == registeredLayers_.at(descTop).at(row).cend())
            {
                registeredLayers_.at(descTop).at(row)[col] = LegendIdentSet{};
            }
            registeredLayers_.at(descTop).at(row).at(col).insert(dataIdent);
        }

        void unregisterLayer(int row, int col, int descTop, NFmiDataIdent dataIdent)
        {
            registeredLayers_.at(descTop).at(row).at(col).erase(dataIdent);
        }

        LegendIdentSet getRegisteredLayers(int row, int col, int descTop)
        {
            try
            {
                return registeredLayers_.at(descTop).at(row).at(col);
            }
            catch(const std::exception&)
            {
                return LegendIdentSet{};
            }
        }

    private:
        bool exists(int row, int col, int descTop)
        {
            try
            {
                registeredLayers_.at(descTop).at(row).at(col);
                return true;
            }
            catch(const std::exception&)
            {
                return false;
            }
        }
    };
}