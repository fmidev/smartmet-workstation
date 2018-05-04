#pragma once

#include "HakeMessage/HakeMsg.h"
#include "HakeMessage/KahaMsg.h"
#include "HakeMessage/json.hpp"

namespace HakeMessage {
    class MsgParser
    {
    public:
        static HakeMsg parseXmlToHakeMessage(const std::string &xmlString);
        static HakeMsg parseJsonToHakeMessage(const std::string &jsonString);
        static KahaMsg parseJsonToKahaMessage(const std::string &jsonString);
    };

    class HakeMsgIsMissingRequiredFields : public std::runtime_error
    {
    public:
        HakeMsgIsMissingRequiredFields(const std::string& errMessage) : std::runtime_error(errMessage) {}
    };
}