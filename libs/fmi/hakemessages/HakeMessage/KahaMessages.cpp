#include "HakeMessage/KahaMessages.h"
#include "HakeMessage/HakeMsg.h"
#include "HakeMessage/json.hpp"

#include "NFmiMetTime.h"

#include "xmlliteutils/UtfConverter.h"

#include <unordered_map>

#include "boost/range/adaptors.hpp"
#include "boost/range/algorithm/copy.hpp"

using namespace boost::adaptors;

namespace HakeMessage
{
    namespace {
        std::string parseAnswers(const nlohmann::json& answers)
        {
            auto answersStringified = std::stringstream{};
            for(const auto& answer : answers)
            {
                answersStringified << "Kysymys: ";
                answersStringified << answer["question"].get<std::string>();
                answersStringified << "\n\n  ";
                answersStringified << "Vastaus: ";
                answersStringified << answer["answer"].get<std::string>();
                answersStringified << "\n\n\n   ";
            }
            return UtfConverter::ConvertUtf_8ToString(answersStringified.str());
        }
    }

    KahaMessages::KahaMessages(std::shared_ptr<Web::Client> client)
        :client_{ client }
    {
    }

    void KahaMessages::addMessage(KahaMsg message)
    {
        auto res = idMap_.find(message.Number());
        if(res == idMap_.cend() || shouldReplaceOlderMessage(message, res->second))
        {
            try
            {
                auto jsonFut = client_->queryFor("http://m.fmi.fi", "/mobile/interfaces/crowd/observation.php?id=" + message.Number());

                jsonFut.wait();

                auto json = nlohmann::json::parse(jsonFut.get());

                json = json["observation"];
                auto res = json.find("answers");
                if(res != json.cend())
                {
                    message.ReasonStr(parseAnswers(*res));
                }
                message.TotalMessageStr(UtfConverter::ConvertUtf_8ToString(json.dump(4)));
                idMap_[message.Number()] = message.SendingTime();
                messages_.insert(message);
            }
            catch(const std::exception&)
            {
            }
        }
    }

    const KahaMessages::SetOfKahaMsgs& KahaMessages::peekMessages() const
    {
        return messages_;
    }

    size_t KahaMessages::getSize() const
    {
        return messages_.size();
    }

    bool KahaMessages::shouldReplaceOlderMessage(const KahaMsg& message, const NFmiMetTime& foundMsgsSendingTime)
    {
        if(foundMsgsSendingTime.IsLessThan(message.SendingTime()))
        {
            decltype(auto) msgToDelete = std::find_if(messages_.lower_bound(message), messages_.upper_bound(message), [&message](const auto& el)
            {
                return el == message;
            });

            if(msgToDelete != messages_.cend())
            {
                messages_.erase(msgToDelete);
                return true;
            }
        }
        return false;
    }
}