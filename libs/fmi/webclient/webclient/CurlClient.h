#pragma once
// Linux HTTP client using libcurl, implementing the same Web::Client
// interface as CppRestClient (which depends on cpprestsdk/Windows).

#include "webclient/Client.h"
#include <cppback/background-manager.h>

#include <atomic>
#include <future>
#include <memory>
#include <set>
#include <string>

namespace Web
{
    class CurlClient : public Client
    {
        std::shared_ptr<cppback::BackgroundManager> bManager_;
        std::string proxyUrl_;
        mutable std::atomic_bool cancelled_{false};
        mutable std::set<std::string> useProxy_;
    public:
        CurlClient(std::shared_ptr<cppback::BackgroundManager> bManager, std::string proxyUrl = "");
        ~CurlClient() override;
        std::future<std::string> queryFor(const std::string& domain, const std::string& query, int timeoutInSeconds) const final;
        void cancel() const final;
    private:
        std::string doRequest(const std::string& url, int timeoutInSeconds, bool withProxy) const;
        bool needsProxy(const std::string& domain) const;
    };
}
