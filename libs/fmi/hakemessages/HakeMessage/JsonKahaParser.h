#pragma once
#include "HakeMessage/KahaMsg.h"

#include <string>

namespace HakeMessage
{

    class JsonKahaParser
    {
    public:
        static KahaMsg parseKaha(const std::string &jsonString);
    };
}