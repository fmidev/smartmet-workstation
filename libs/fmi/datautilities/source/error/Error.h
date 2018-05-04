#pragma once

#include <stdexcept>

namespace SmartMetDataUtilities
{
    class ToolmasterNotAvailable : public std::runtime_error
    {
    public:
        ToolmasterNotAvailable(std::string errMessage) :runtime_error(errMessage) {}
    };

    class DataNotFoundException : public std::runtime_error
    {
    public:
        DataNotFoundException(std::string errMessage) :runtime_error(errMessage) {}
    };
}