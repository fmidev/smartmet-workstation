#pragma once

#include <string>

namespace HakeMessage
{
    class HakeMsg;

    class XmlHakeParser
    {
    public:
        static HakeMsg parse(const std::string &xmlString);
    };
}

