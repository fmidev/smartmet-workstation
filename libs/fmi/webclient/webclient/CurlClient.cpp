#include "webclient/CurlClient.h"
#include "catlog/catlog.h"

#include <curl/curl.h>
#include <stdexcept>

namespace
{
    std::string makeFinalLogMessage(const std::string& domain, const std::string& request)
    {
        auto loggingMessage = domain;
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

    // libcurl write callback: append received data to a std::string
    size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        auto* response = static_cast<std::string*>(userdata);
        size_t totalBytes = size * nmemb;
        response->append(ptr, totalBytes);
        return totalBytes;
    }

    // libcurl progress callback: check cancellation
    int progressCallback(void* clientp, curl_off_t /*dltotal*/, curl_off_t /*dlnow*/, curl_off_t /*ultotal*/, curl_off_t /*ulnow*/)
    {
        auto* cancelled = static_cast<std::atomic_bool*>(clientp);
        return cancelled->load() ? 1 : 0;  // Return non-zero to abort
    }
}

namespace Web
{
    CurlClient::CurlClient(std::shared_ptr<cppback::BackgroundManager> bManager, std::string proxyUrl)
        : bManager_{std::move(bManager)}
        , proxyUrl_{std::move(proxyUrl)}
    {
    }

    CurlClient::~CurlClient() = default;

    std::future<std::string> CurlClient::queryFor(const std::string& domain, const std::string& query, int timeoutInSeconds) const
    {
        ::makeTraceLevelRequestLogging(domain, query);
        return bManager_->addTask(
            [this, domain = domain, query = query, timeoutInSeconds]
            {
                std::string url = domain + query;
                try
                {
                    return doRequest(url, timeoutInSeconds, needsProxy(domain));
                }
                catch(const std::exception&)
                {
                    // If first attempt failed without proxy, retry with proxy
                    if(!needsProxy(domain) && !proxyUrl_.empty())
                    {
                        useProxy_.insert(domain);
                        return doRequest(url, timeoutInSeconds, true);
                    }
                    throw;
                }
            });
    }

    std::string CurlClient::doRequest(const std::string& url, int timeoutInSeconds, bool withProxy) const
    {
        CURL* curl = curl_easy_init();
        if(!curl)
            throw std::runtime_error("CurlClient: failed to initialize curl");

        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeoutInSeconds));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // Match CppRestClient's set_validate_certificates(false)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        // Set up cancellation via progress callback
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, const_cast<std::atomic_bool*>(&cancelled_));
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        if(withProxy && !proxyUrl_.empty())
            curl_easy_setopt(curl, CURLOPT_PROXY, proxyUrl_.c_str());

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK)
        {
            std::string errMsg = "CurlClient: request failed for " + url + ": " + curl_easy_strerror(res);
            CatLog::logMessage(errMsg, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
            throw std::runtime_error(errMsg);
        }

        if(httpCode >= 400)
        {
            std::string errMsg = "CurlClient: HTTP " + std::to_string(httpCode) + " for " + url;
            CatLog::logMessage(errMsg, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
            throw std::runtime_error(errMsg);
        }

        return response;
    }

    void CurlClient::cancel() const
    {
        cancelled_.store(true);
    }

    bool CurlClient::needsProxy(const std::string& domain) const
    {
        return useProxy_.find(domain) != useProxy_.cend();
    }
}
