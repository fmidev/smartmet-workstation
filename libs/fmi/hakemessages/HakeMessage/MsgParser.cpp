#include "HakeMessage/MsgParser.h"
#include "HakeMessage/HakeMsg.h"
#include "HakeMessage/XmlHakeParser.h"
#include "HakeMessage/JsonHakeParser.h"
#include "HakeMessage/JsonKahaParser.h"

#include "xmlliteutils/UtfConverter.h"

namespace HakeMessage
{
    namespace
    {
        void throwExceptionIfMissingRequiredFields(const HakeMsg& message)
        {
            auto point = message.LatlonPoint();
            auto time = message.StartTime();
            auto category = message.Category();

            if(point == NFmiPoint::gMissingLatlon || time == NFmiMetTime::gMissingTime || category == 0)
            {
                throw HakeMsgIsMissingRequiredFields("Message is missing required fields.");
            }
        }
    }

    HakeMsg MsgParser::parseXmlToHakeMessage(const std::string &xmlString)
    {
        auto converted = UtfConverter::ConvertUtf_8ToString(xmlString);
        auto xmlMessage = XmlHakeParser::parse(converted);
        throwExceptionIfMissingRequiredFields(xmlMessage);
        return xmlMessage;
    }

    HakeMsg MsgParser::parseJsonToHakeMessage(const std::string &jsonString)
    {
        auto jsonMessage = JsonHakeParser::parseHake(UtfConverter::ConvertUtf_8ToString(jsonString));
        throwExceptionIfMissingRequiredFields(jsonMessage);
        return jsonMessage;
    }

   HakeMsg MsgParser::parseJsonToKahaMessage(const std::string &jsonString)
    {
        auto jsonMessage = JsonKahaParser::parseKaha(UtfConverter::ConvertUtf_8ToString(jsonString));
        throwExceptionIfMissingRequiredFields(jsonMessage);
        return jsonMessage;
    }
}