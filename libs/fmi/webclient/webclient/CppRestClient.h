#pragma once

#include "webclient/Client.h"

#include "cpprest/http_client.h"
#include "cpprest/http_msg.h"
#undef U // This fixes cpprest's U -macro clash with boost library move code (really dangerous to give macro name like U !!!!)

#include <cppback/background-manager.h>

#include <future>
#include <string>
#include <set>

namespace Web
{
    class CppRestClient : public Client
    {
        std::shared_ptr<cppback::BackgroundManager> bManager_;
        Concurrency::cancellation_token_source tokenSource_;
        web::http::client::http_client_config client_config_;
        std::string proxyUrl_;
        mutable std::set<std::string> useProxy_;
    public:
        CppRestClient(std::shared_ptr<cppback::BackgroundManager> bManager, std::string proxyUrl = "");
        std::future<std::string> queryFor(const std::string& domain, const std::string& query) const final;
        void cancel() const final;
    private:
       web::http::http_response tmp(const std::string& domain, const std::string& request) const;
       web::http::client::http_client_config createConfig(const std::string& domain) const;
       bool needsProxy(const std::string& domain) const;
    };
}