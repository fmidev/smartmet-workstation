#pragma once

#include <string>
#include <map>

namespace Wms
{
    class WmsQuery
    {
    public:
        std::string scheme;
        std::string host;
        std::string path;
        std::string proxy;
        std::map<std::string, std::string> query;
    };

    std::string toRequest(const WmsQuery& query);
    std::string toBaseUri(const WmsQuery& query);
}

