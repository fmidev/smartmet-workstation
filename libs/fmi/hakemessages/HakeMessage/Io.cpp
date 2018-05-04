#include "HakeMessage/Io.h"

#include <cppback/loop-signal-checker.h>

#include "NFmiFileSystem.h"

#include <regex>
#include <experimental/filesystem>

namespace HakeMessage
{
    namespace
    {
        std::string readFile(const std::string& path)
        {
            std::string contents;
            auto res = NFmiFileSystem::ReadFile2String(path, contents);
            if(!res)
            {
                throw FileNotFoundException("File with path: " + path + " could not be found.");
            }
            return contents;
        }
    }

    Io::Io(std::shared_ptr<cppback::BackgroundManager> bManager)
        :bManager_{bManager}
    {
    }

    std::vector<std::string> Io::readFromFolderWithFilter(const std::string& folder, const std::string& filter, unsigned maxNumberOfMessagesToRead)
    {
        auto checker = cppback::LoopSignalChecker(maxNumberOfMessagesToRead / 10, "HakeMessages::Io::readFromFolderWithFilter");

        std::experimental::filesystem::path folderPath(folder);
        const std::regex filterRegex(filter);
        std::vector<std::string> msgs;

        for(decltype(auto) entry : std::experimental::filesystem::directory_iterator(folderPath))
        {
            auto fileName = entry.path().string();
            if(
                !std::experimental::filesystem::is_regular_file(entry)
                || !std::regex_match(fileName, filterRegex)
                || readFiles_.find(fileName) != readFiles_.cend()
                )
            {
                continue;
            }
            checker.check(*bManager_);
            msgs.push_back(readFile(fileName));
            readFiles_.insert(fileName);
            if(msgs.size() >= maxNumberOfMessagesToRead)
            {
                break;
            }
        }
        return msgs;
    }
}