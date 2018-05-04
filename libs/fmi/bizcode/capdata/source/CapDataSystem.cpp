#include "CapDataSystem.h"

using namespace std;

namespace Warnings
{

    CapDataSystem::CapDataSystem()
        :configurationData_()
    {
        
    }

    CapDataSystem::~CapDataSystem() = default;

    bool CapDataSystem::init()
    {
        configurationData_ = CapDataConfiguration("WOML");
        if(!configurationData_.completeUrl().empty() || configurationData_.useFile())
        {
            initialized_ = true;
        }
        return initialized_;
    }

    CapData& CapDataSystem::capdata(unsigned long producerId, unsigned long parameterId)
    {
        if(data_.warnings().empty() && configurationData_.useFile() == 0) {
            data_.load(configurationData_.completeUrl(), configurationData_.useFile()); //Modify based on the data user wants to see
        }
        else if(data_.warnings().empty() && configurationData_.useFile() == 1) {
            data_.load(configurationData_.file(), configurationData_.useFile());
        }
        return data_;
    }

    bool CapDataSystem::checkIfWarningsAreUpdated(std::string& id)
    {
        std::string newId = data_.getOnlineCapdataPublicationId(configurationData_.completeUrl());
        if(id.compare(newId) != 0)
        {
            return true;
        }   
        return false;
    }

    //Checks if new data is available and updates it if necessary
    void CapDataSystem::refreshCapData()
    {
        //If there are already saved warnings and we are not using file, then try to check whether new warnings are available
        if(!data_.warnings().empty() && configurationData_.useFile() == 0)
        {
            std::string id = data_.getFirstCapdataPublicationId();
            if(checkIfWarningsAreUpdated(id))
            {
                data_ = {};
                data_.load(configurationData_.completeUrl(), configurationData_.useFile());
            }
        }
        else //try to get new warnings
        {
            data_ = {};
            data_.load(configurationData_.completeUrl(), configurationData_.useFile());
        }
    }

}
