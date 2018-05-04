#pragma once

#include "CapData.h"
#include "CapDataConfiguration.h"

class NFmiMetTime;

namespace Warnings
{
    class CapDataSystem
    {
        CapDataConfiguration configurationData_;
        CapData data_;
        bool initialized_ = false;

    public:
        CapDataSystem();
        ~CapDataSystem();
        bool init();
        CapData& capdata(unsigned long producerId, unsigned long parameterId);
        void refreshCapData();   

        bool useCapData() const
        {
            return configurationData_.useCapData();
        }

    private:
        bool checkIfWarningsAreUpdated(std::string& id);
    };
   

}