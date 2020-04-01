#include "wmssupport/LegendHandler.h"

namespace Wms
{
        LegendIdentSet LegendHandler::getLegends(int row, int col, int descTop)
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

        void LegendHandler::registerLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent)
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

        void LegendHandler::unregisterLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent)
        {
            registeredLayers_.at(descTop).at(row).at(col).erase(dataIdent);
        }

        const LegendIdentSet& LegendHandler::getRegisteredLayers(int row, int col, int descTop)
        {
            try
            {
                return registeredLayers_.at(descTop).at(row).at(col);
            }
            catch(const std::exception&)
            {
                static LegendIdentSet dummySet{};
                return dummySet;
            }
        }

        bool LegendHandler::exists(int row, int col, int descTop)
        {
            try
            {
                return registeredLayers_.at(descTop).at(row).count(col) > 0;
            }
            catch(const std::exception&)
            {
                return false;
            }
        }
}
