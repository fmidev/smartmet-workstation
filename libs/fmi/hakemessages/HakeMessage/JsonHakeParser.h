#pragma once
#include "HakeMessage/json.hpp"
#include "HakeMessage/HakeMsg.h"
#include "HakeMessage/KahaMsg.h"

namespace HakeMessage
{
    class JsonHakeParser
    {
    public:
        static HakeMsg parseHake(const std::string &jsonString);
    };
}

