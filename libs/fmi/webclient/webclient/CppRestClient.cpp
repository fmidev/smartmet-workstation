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
        if(timeoutInSeconds <= 0)
            throw std::invalid_argument("HTTP request timeout must be positive");
        ::makeTraceLevelRequestLogging(domain, request);
        return bManager_->addTask([this, domain, request, timeoutInSeconds]()
        {
            return tmp(domain, request, timeoutInSeconds);
        });
    }

    std::string CppRestClient::tmp(const std::string& domain, const std::string& request, int timeoutInSeconds) const
    {
        auto attempt = [&](bool useProxy)
        {
            // Each attempt has its own cancellation source; timing out must not
            // permanently cancel this client or any concurrent image requests.
            Concurrency::cancellation_token_source requestCancellation;
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutInSeconds);
            auto waitFor = [&](auto task)
            {
                while(!task.is_done())
                {
                    const bool stopping = tokenSource_.get_token().is_canceled() || bManager_->isKillSignalSet();
                    if(stopping || std::chrono::steady_clock::now() >= deadline)
                    {
                        requestCancellation.cancel();
                        // Observe eventual completion without holding the caller hostage
                        // if the transport takes longer to finish cancellation.
                        task.then([](decltype(task) completed)
                        {
                            try { completed.get(); }
                            catch(...) {}
                        });
                        if(stopping)
                            throw pplx::task_canceled();
                        throw web::http::http_exception("HTTP request deadline exceeded for " + domain);
                    }
                    bManager_->isKillSignalSet(std::chrono::milliseconds(50));
                }
                return task.get();
            };

            if(tokenSource_.get_token().is_canceled() || bManager_->isKillSignalSet())
                throw pplx::task_canceled();
            auto client = web::http::client::http_client(to_string_t(domain), createConfig(useProxy, timeoutInSeconds));
            auto response = waitFor(client.request(web::http::methods::GET, to_string_t(request), requestCancellation.get_token()));
            if(response.status_code() < 200 || response.status_code() >= 300)
            {
                requestCancellation.cancel();
                throw std::runtime_error("HTTP " + std::to_string(response.status_code()) + " from " + domain);
            }

            // Keep the destination alive until the body task finishes, including
            // when waitFor exits early after cancellation or its deadline.
            auto buffer = std::make_shared<Concurrency::streams::container_buffer<std::string>>();
            auto bodyTask = response.body().read_to_end(*buffer).then([buffer](pplx::task<size_t> completed)
            {
                completed.get();
                return buffer->collection();
            });
            return waitFor(bodyTask);
        };

        try
        {
            return attempt(false);
        }
        catch(const web::http::http_exception& e)
        {
            if(proxyUrl_.empty() || tokenSource_.get_token().is_canceled() || bManager_->isKillSignalSet())
                throw;
            CatLog::logMessage("Request to " + domain + " failed on the default route: " + e.what()
                + "; retrying once with the configured proxy (timeout " + std::to_string(timeoutInSeconds) + "s)",
                CatLog::Severity::Warning, CatLog::Category::NetRequest);
            return attempt(true);
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

