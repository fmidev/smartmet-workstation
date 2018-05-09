#pragma once
#ifndef DISABLE_CPPRESTSDK

#include "HakeMessage/KahaMsg.h"

#include <webclient/Client.h>


#include <set>
#include <map>
#include <memory>

namespace HakeMessage
{
    class KahaMessages
    {
    public:
        using SetOfKahaMsgs = std::multiset<KahaMsg>;
    private:
        SetOfKahaMsgs messages_;
        std::map<std::string, NFmiMetTime> idMap_;
        std::shared_ptr<Web::Client> client_;
    public:
        KahaMessages(std::shared_ptr<Web::Client> client);
        void addMessage(KahaMsg message);
        const SetOfKahaMsgs& peekMessages() const;
        size_t getSize() const;
    private:
        bool shouldReplaceOlderMessage(const KahaMsg& message, const NFmiMetTime& foundMsgsSendingTime);
    };
}

#endif // DISABLE_CPPRESTSDK
