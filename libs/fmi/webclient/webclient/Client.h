#pragma once

#include <future>

namespace Web
{
    class Client
    {
    public:
        virtual std::future<std::string> queryFor(const std::string& domain, const std::string& query, int timeoutInSeconds) const = 0;
        virtual void cancel() const = 0;
    };
}