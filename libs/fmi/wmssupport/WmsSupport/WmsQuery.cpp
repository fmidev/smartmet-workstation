#include "wmssupport/WmsQuery.h"

#include <cpprest/asyncrt_utils.h>
#include <cpprest/http_msg.h>
#undef U // This fixes cpprest's U -macro clash with boost library move code (really dangerous to give macro name like U !!!!)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    std::string toRequest(const WmsQuery& query)
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

    std::string toBaseUri(const WmsQuery& query)
    {
        return utility::conversions::to_utf8string(web::http::uri_builder()
            .set_scheme(utility::conversions::to_string_t(query.scheme))
            .set_host(utility::conversions::to_string_t(query.host))
            .to_string());
    }
}

