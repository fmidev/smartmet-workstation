#pragma once

#include "webclient/Client.h"

#include "cpprest/http_client.h"
#include "cpprest/http_msg.h"
#undef U // This fixes cpprest's U -macro clash with boost library move code (really dangerous to give macro name like U !!!!)

#include <cppback/background-manager.h>

#include <future>
#include <string>
#include <stdexcept>

namespace Web
{
    class CppRestClient : public Client
    {
        std::shared_ptr<cppback::BackgroundManager> bManager_;
        Concurrency::cancellation_token_source tokenSource_;
        std::string proxyUrl_;
    public:
        CppRestClient(std::shared_ptr<cppback::BackgroundManager> bManager, std::string proxyUrl = "");
        ~CppRestClient();
        std::future<std::string> queryFor(const std::string& domain, const std::string& query, int timeoutInSeconds) const final;
        void cancel() const final;
    private:
       web::http::http_response tmp(const std::string& domain, const std::string& request, int timeoutInSeconds) const;
       web::http::client::http_client_config createConfig(bool useProxy, int timeoutInSeconds) const;
    };
}