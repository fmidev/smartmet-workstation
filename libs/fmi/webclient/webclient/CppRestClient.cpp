#include "webclient/CppRestClient.h"
#include "catlog/catlog.h"

using namespace utility::conversions;

namespace
{
    void makeTraceLevelRequestLogging(const std::string& domain, const std::string& request)
    {
        if(CatLog::doTraceLevelLogging())
        {
            auto loggingMessage = domain;
            loggingMessage += request;
            CatLog::logMessage(loggingMessage, CatLog::Severity::Trace, CatLog::Category::NetRequest, true);
        }
    }

}

namespace Web
{
    CppRestClient::CppRestClient(std::shared_ptr<cppback::BackgroundManager> bManager, std::string proxyUrl)
        :bManager_{bManager}
        , proxyUrl_{proxyUrl}
    {
    }

    std::future<std::string> CppRestClient::queryFor(const std::string& domain, const std::string& request) const
    {
        ::makeTraceLevelRequestLogging(domain, request);
            return bManager_->addTask(
                [&,
                domain = domain,
                request = request]
                    {
                            auto response = tmp(domain, request);
                            auto bodyStream = response.body();
                            Concurrency::streams::container_buffer<std::string> inStringBuffer;
                            bodyStream.read_to_end(inStringBuffer).wait();
                            return inStringBuffer.collection();
                    });
    }

    web::http::http_response CppRestClient::tmp(const std::string& domain, const std::string& request) const
    {
        auto client = web::http::client::http_client(to_string_t(domain), createConfig(domain));
        auto responseFut = client.request(web::http::methods::GET, to_string_t(request), tokenSource_.get_token());
        try
        {
            return responseFut.get();
        }
        catch(const std::exception& e)
        {
            if(!needsProxy(domain))
            {
                useProxy_.insert(domain);
                return tmp(domain, request);
            }
            else
            {
                throw e;
            }
        }
    }

    void CppRestClient::cancel() const
    {
        tokenSource_.cancel();
    }

    bool CppRestClient::needsProxy(const std::string& domain) const
    {
        return useProxy_.find(domain) != useProxy_.cend();
    }

    web::http::client::http_client_config CppRestClient::createConfig(const std::string& domain) const
    {
        auto client_config = web::http::client::http_client_config{};
        if(needsProxy(domain))
        {
            client_config.set_proxy(web::web_proxy(to_string_t(proxyUrl_)));
        }
        return client_config;
    }
}

