#include "wmssupport/LegendHandler.h"
#include "catlog/catlog.h"

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
            try
            {
                registeredLayers_.at(descTop).at(row).at(col).erase(dataIdent);
            }
            catch(std::exception &)
            { 
                // Turha näitä on raportoida, en ymmärrä tätä koko systeemiä ja näitä unregisteroidaan jatkuvalla syötöllä ja mikään niistä ei näytä onnistuvan
                /*  
                std::string warningMessage = __FUNCTION__;
                warningMessage += " failed with map-view: ";
                warningMessage += std::to_string(descTop + 1);
                warningMessage += ", row: ";
                warningMessage += std::to_string(row);
                warningMessage += ", col: ";
                warningMessage += std::to_string(col);
                warningMessage += ", param: ";
                warningMessage += dataIdent.GetParamName();
                warningMessage += " (id = ";
                warningMessage += std::to_string(dataIdent.GetParamIdent());
                warningMessage += "), with error message: \"";
                warningMessage += e.what();
                warningMessage += "\"";
                CatLog::logMessage(warningMessage, CatLog::Severity::Warning, CatLog::Category::Visualization, true);
                */
            }
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
