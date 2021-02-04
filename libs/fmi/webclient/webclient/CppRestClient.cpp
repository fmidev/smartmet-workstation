#include "webclient/CppRestClient.h"
#include "catlog/catlog.h"

using namespace utility::conversions;

namespace
{
    std::string makeFinalLogMessage(const std::string& domain, const std::string& request)
    {
        auto loggingMessage = domain;
        // Halutaan v‰ltt‰‰ tupla kenoviiva domain:in ja request:in v‰liss‰ lokiviestiin,
        // jos domain loppuu ja request alkaa kenoviivalla.
        if(domain.back() == '/' && request.front() == '/' && request.empty() == false)
            loggingMessage += std::string(request.begin() + 1, request.end());
        else
            loggingMessage += request;
        return loggingMessage;
    }

    void makeTraceLevelRequestLogging(const std::string& domain, const std::string& request)
    {
        if(CatLog::doTraceLevelLogging())
        {
            auto loggingMessage = ::makeFinalLogMessage(domain, request);
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

    std::future<std::string> CppRestClient::queryFor(const std::string& domain, const std::string& request, int timeoutInSeconds) const
    {
        ::makeTraceLevelRequestLogging(domain, request);
            return bManager_->addTask(
                [&,
                domain = domain,
                request = request]
                    {
                            auto response = tmp(domain, request, timeoutInSeconds);
                            auto bodyStream = response.body();
                            Concurrency::streams::container_buffer<std::string> inStringBuffer;
                            bodyStream.read_to_end(inStringBuffer).wait();
                            return inStringBuffer.collection();
                    });
    }

    web::http::http_response CppRestClient::tmp(const std::string& domain, const std::string& request, int timeoutInSeconds) const
    {
        auto client = web::http::client::http_client(to_string_t(domain), createConfig(domain, timeoutInSeconds));
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
                return tmp(domain, request, timeoutInSeconds);
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

    web::http::client::http_client_config CppRestClient::createConfig(const std::string& domain, int timeoutInSeconds) const
    {
        auto client_config = web::http::client::http_client_config{};
		client_config.set_validate_certificates(false);
        if(needsProxy(domain))
        {
            client_config.set_proxy(web::web_proxy(to_string_t(proxyUrl_)));
        }
        client_config.set_timeout(std::chrono::seconds(timeoutInSeconds));
        return client_config;
    }
}

