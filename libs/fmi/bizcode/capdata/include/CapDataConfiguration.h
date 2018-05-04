#pragma once

#include <string>

namespace Warnings
{
    class CapDataConfiguration
    {
        bool useCapData_ = false;
        std::string name_;
        std::string url_;
        std::string key_;
        std::string path_;
        int maxFeatures_;
        std::string file_;
        int useFile_;

    public:
	    CapDataConfiguration();
        CapDataConfiguration(const std::string &name);//, const std::string &url);
	    ~CapDataConfiguration();
        void initFromConfigurations();
        const std::string& name() const { return name_; }
        const std::string& url() const { return url_; }
        const std::string& completeUrl() const { return completeUrl_; }
        const int useFile() const { return useFile_; }
        const std::string& file() const { return file_; }
        bool useCapData() const { return useCapData_; }

    private:
        std::string completeUrl_;

    };

}
