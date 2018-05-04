#pragma once

#include <string>

namespace HakeMessage
{

    class Logger
    {
    public:
        enum class Severity
        {
            Missing
        };
        virtual void log(const std::string& message, Severity severity) = 0;
    };
}