#pragma once

#include <cpprest/asyncrt_utils.h>
#include <cpprest/http_msg.h>

#include <string>
#include <unordered_map>

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

    inline std::string toRequest(const WmsQuery& query)
    {
        web::http::uri_builder builder;
        builder.append_path(utility::conversions::to_string_t(query.path));
        for(decltype(auto) param : query.query)
        {
            builder.append_query(
                utility::conversions::to_string_t(param.first),
                utility::conversions::to_string_t(param.second)
            );
        }
        return utility::conversions::to_utf8string(builder.to_string());
    }

    inline std::string toBaseUri(const WmsQuery& query)
    {
        return utility::conversions::to_utf8string(web::http::uri_builder()
            .set_scheme(utility::conversions::to_string_t(query.scheme))
            .set_host(utility::conversions::to_string_t(query.host))
            .to_string());
    }
}

