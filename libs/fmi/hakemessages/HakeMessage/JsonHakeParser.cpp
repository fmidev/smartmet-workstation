#include "HakeMessage\JsonHakeParser.h"
#include "HakeMessage\HakeMsg.h"

#include "NFmiTime.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "NFmiStringTools.h"

#include "json.hpp"
#include <cppext/split.h>

#include <unordered_map>
#include <utility>

namespace HakeMessage
{
    namespace
    {
        using Json = nlohmann::json;

        NFmiMetTime convertJsonTimeToMetTime(const std::string& timeStr)
        {
            auto subStrings = cppext::split(timeStr, 'T');
            if(subStrings.size() != 2)
            {
                return NFmiMetTime::gMissingTime;
            }

            auto dateSubStrings = cppext::split(subStrings[0], '-');
            if(dateSubStrings.size() != 3)
            {
                return NFmiMetTime::gMissingTime;
            }
            auto year = std::stoi(dateSubStrings[0]);
            auto month = std::stoi(dateSubStrings[1]);
            auto day = std::stoi(dateSubStrings[2]);

            auto timeSubStrings = cppext::split(subStrings[1], '.');
            timeSubStrings = cppext::split(timeSubStrings[0], ':');
            if(timeSubStrings.size() != 3)
            {
                return NFmiMetTime::gMissingTime;
            }
            auto hour = std::stoi(timeSubStrings[0]);
            auto minute = std::stoi(timeSubStrings[1]);

            return NFmiMetTime(NFmiTime(year, month, day, hour, minute, 0).UTCTime(), 1);
        }

        std::string lastWordOfDescription(const std::string& jsonString)
        {
            auto pos = jsonString.find_last_of(' ');
            return jsonString.substr(pos + 1);
        }

        int getLevelCorrespondingToChar(char firstChar)
        {
            static std::unordered_map<char, int> levels = {
                { 's', 3 },
                { 'k', 2 },
                { 'p', 1 }
            };
            return levels[firstChar];
        }

        int parseMessageLevelFromDescription(const std::string& jsonString)
        {
            auto level = lastWordOfDescription(jsonString);
            if(level.empty())
            {
                level = "pieni";
            }
            return getLevelCorrespondingToChar(level.front());
        }

        void ifMissingStartTimeTryToSubstituteWithSendingTime(HakeMsg& jsonMessage)
        {
            if(jsonMessage.StartTime() == NFmiMetTime::gMissingTime)
            {
                if(jsonMessage.SendingTime() != NFmiMetTime::gMissingTime)
                {
                    jsonMessage.StartTime(jsonMessage.SendingTime());
                }
            }
        }
    }


    HakeMsg JsonHakeParser::parseHake(const std::string &jsonString)
    {
        auto hakeMsg = HakeMsg{};
        auto json = Json::parse(jsonString);

        if(json.is_array() && !json.empty())
        {
            json = json.front();
        }
        auto id = json.find("id");
        if(id != json.cend())
        {
            hakeMsg.Number(*id);
        }
        auto startTime = json.find("creationTime");
        if(startTime != json.cend())
        {
            hakeMsg.StartTime(convertJsonTimeToMetTime(*startTime));
        }

        auto notices = json.find("notices");
        if(notices != json.cend())
        {
            if(notices->is_array() && !notices->empty())
            {
                *notices = notices->front();
            }
            auto contacts = notices->find("contacts");
            if(contacts != notices->cend())
            {
                if(contacts->is_array() && !contacts->empty())
                {
                    *contacts = contacts->front();
                }
                auto handler = contacts->find("handlerSites");
                if(handler != contacts->cend())
                {
                    if(handler->is_array() && !handler->empty())
                    {
                        *handler = handler->front();
                    }
                    auto name = handler->find("name");
                    if(name != handler->cend())
                    {
                        if(name->is_array() && !name->empty())
                        {
                            *name = name->front();
                        }
                        hakeMsg.MessageCenterId(*name);
                    }
                }
            }
        }
        auto tasks = json.find("tasks");
        if(tasks != json.cend())
        {
            if(tasks->is_array() && !tasks->empty())
            {
                *tasks = tasks->front();
            }

            auto sendingTime = tasks->find("creationTime");
            if(sendingTime != tasks->cend())
            {
                hakeMsg.SendingTime(convertJsonTimeToMetTime(*sendingTime));
            }

            auto taskClassification = tasks->find("taskClassification");
            if(taskClassification != tasks->cend())
            {
                auto description = taskClassification->find("description");
                if(description != taskClassification->cend())
                {
                    hakeMsg.TypeStr(*description);
                    hakeMsg.MessageStr(*description);
                    hakeMsg.MessageLevel(parseMessageLevelFromDescription(*description));
                }
                auto code = taskClassification->find("code");
                if(code != taskClassification->cend())
                {
                    hakeMsg.Category(std::stoi(code->get<std::string>()));
                }
            }
        }

        auto location = json.find("location");
        if(location != json.cend())
        {
            auto countyStr = location->find("municipality");
            if(countyStr != location->cend())
            {
                hakeMsg.CountyStr(*countyStr);
            }
            auto address = location->find("streetAddress");
            if(address != location->cend())
            {
                hakeMsg.Address(*address);
            }
            auto area = location->find("area");
            if(area != location->cend())
            {
                auto coordinates = area->find("coordinates");
                if(coordinates != area->cend())
                {
                    hakeMsg.LatlonPoint(NFmiPoint((*coordinates)[0], (*coordinates)[1]));
                }
            }
        }
        hakeMsg.TotalMessageStr(Json::parse(jsonString).dump(4));

        ifMissingStartTimeTryToSubstituteWithSendingTime(hakeMsg);
        return hakeMsg;
    }
}