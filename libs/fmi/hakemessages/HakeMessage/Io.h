#pragma once

#include <cppback/background-manager.h>

#include <vector>
#include <set>
#include <memory>

namespace HakeMessage
{
    class Io
    {
        std::set<std::string> readFiles_;
        std::shared_ptr<cppback::BackgroundManager> bManager_;
    public:
        Io(std::shared_ptr<cppback::BackgroundManager> bManager);

        std::vector<std::string> readFromFolderWithFilter(const std::string& folder, const std::string& filter, unsigned maxNumberOfMessagesToRead);
    };

    class FileNotFoundException : public std::runtime_error
    {
    public:
        FileNotFoundException(const std::string& errMessage) : std::runtime_error(errMessage) {}
    };
}

