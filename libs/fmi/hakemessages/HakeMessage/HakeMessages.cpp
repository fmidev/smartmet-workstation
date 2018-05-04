#include "HakeMessage/HakeMessages.h"
#include "HakeMessage/HakeMsg.h"

#include "NFmiMetTime.h"

namespace HakeMessage
{
    void HakeMessages::addMessage(const HakeMsg &message)
    {
        auto res = idMap_.find(message.Number());
        if(res != idMap_.cend())
        {
            auto foundMsgsSendingTime = res->second;
            if(foundMsgsSendingTime.IsLessThan(message.SendingTime()))
            {
                idMap_[message.Number()] = message.SendingTime();
                decltype(auto) msgToDelete = std::find_if(messages_.lower_bound(message), messages_.upper_bound(message), [&message](const auto& el)
                {
                    return el == message;
                });
                if (msgToDelete != messages_.cend())
                {
                    messages_.erase(msgToDelete);
                }
                messages_.insert(message);
            }
        }
        else
        {
            idMap_[message.Number()] = message.SendingTime();
            messages_.insert(message);
        }
    }

    const HakeMessages::SetOfHakeMsgs& HakeMessages::peekMessages() const
    {
        return messages_;
    }

    size_t HakeMessages::getSize() const
    {
        return messages_.size();
    }
}