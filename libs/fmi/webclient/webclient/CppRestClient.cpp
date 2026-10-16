#include "webclient/CppRestClient.h"
#include "catlog/catlog.h"

using namespace utility::conversions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace
{
    std::string makeFinalLogMessage(const std::string& domain, const std::string& request)
    {
        auto loggingMessage = domain;
        // Halutaan v‰ltt‰‰ tupla kenoviiva domain:in ja request:in v‰liss‰ lokiviestiin,
        // jos domain loppuu ja request alkaa kenoviivalla.
        if(!domain.empty() && !request.empty() && domain.back() == '/' && request.front() == '/')
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

    CppRestClient::~CppRestClient() = default;

    std::future<std::string> CppRestClient::queryFor(const std::string& domain, const std::string& request, int timeoutInSeconds) const
    {
        ::makeTraceLevelRequestLogging(domain, request);
            return bManager_->addTask(
                [this, domain, request, timeoutInSeconds]
                    {
                            auto response = tmp(domain, request, timeoutInSeconds);
                            if(response.status_code() < 200 || response.status_code() >= 300)
                            {
                                throw std::runtime_error("HTTP " + std::to_string(response.status_code()) + " from " + domain);
                            }
                            auto bodyStream = response.body();
                            Concurrency::streams::container_buffer<std::string> inStringBuffer;
                            bodyStream.read_to_end(inStringBuffer).wait();
                            return inStringBuffer.collection();
                    });
    }

    web::http::http_response CppRestClient::tmp(const std::string& domain, const std::string& request, int timeoutInSeconds) const
    {
        try
        {
            auto client = web::http::client::http_client(to_string_t(domain), createConfig(false, timeoutInSeconds));
            return client.request(web::http::methods::GET, to_string_t(request), tokenSource_.get_token()).get();
        }
        catch(const web::http::http_exception& e)
        {
            // A transient failure must not change routing for subsequent requests.
            if(proxyUrl_.empty() || tokenSource_.get_token().is_canceled() || bManager_->isKillSignalSet())
                throw;

            CatLog::logMessage("Request to " + domain + " failed on the default route: " + e.what()
                + "; retrying once with the configured proxy (timeout " + std::to_string(timeoutInSeconds) + "s)",
                CatLog::Severity::Warning, CatLog::Category::NetRequest);
            auto client = web::http::client::http_client(to_string_t(domain), createConfig(true, timeoutInSeconds));
            return client.request(web::http::methods::GET, to_string_t(request), tokenSource_.get_token()).get();
        }
    }

    void CppRestClient::cancel() const
    {
        tokenSource_.cancel();
    }

    web::http::client::http_client_config CppRestClient::createConfig(bool useProxy, int timeoutInSeconds) const
    {
        auto client_config = web::http::client::http_client_config{};
		client_config.set_validate_certificates(false);
        if(useProxy)
        {
            client_config.set_proxy(web::web_proxy(to_string_t(proxyUrl_)));
        }
        client_config.set_timeout(std::chrono::seconds(timeoutInSeconds));
        return client_config;
    }
}

