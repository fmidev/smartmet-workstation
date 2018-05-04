#include "CapDataConfiguration.h"
#include "NFmiSettings.h"

namespace Warnings
{

    CapDataConfiguration::CapDataConfiguration()
    {
    }

    CapDataConfiguration::CapDataConfiguration(const std::string &name) //, const std::string &url)
        :name_(name)
        ,url_()
        ,key_(), path_()
        ,maxFeatures_()
    {
        initFromConfigurations();
    }

    CapDataConfiguration::~CapDataConfiguration() = default;


    void CapDataConfiguration::initFromConfigurations()
    {
        useCapData_ = NFmiSettings::Optional<bool>("SmartMet::Warnings::Use", false);
        if(useCapData_)
        {
            url_ = NFmiSettings::Optional<std::string>("SmartMet::Warnings::Url", "");
            key_ = NFmiSettings::Optional<std::string>("SmartMet::Warnings::Key", "");
            path_ = NFmiSettings::Optional<std::string>("SmartMet::Warnings::Path", "");
            maxFeatures_ = NFmiSettings::Optional<int>("SmartMet::Warnings::MaxFeatures", 1000);
            file_ = NFmiSettings::Optional<std::string>("SmartMet::Warnings::File", "");
            useFile_ = NFmiSettings::Optional<int>("SmartMet::Warnings::UseFile", 0);
            completeUrl_ = "https://" + url_ + key_ + "/" + path_ + "&maxFeatures=" + std::to_string(maxFeatures_);
        }
    }

}
