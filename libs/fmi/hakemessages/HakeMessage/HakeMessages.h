#pragma once
#include "HakeMessage/HakeMsg.h"

#include <set>
#include <map>

namespace HakeMessage
{
    class HakeMessages
    {
    public:
        using SetOfHakeMsgs = std::multiset<HakeMsg>;
    private:
        SetOfHakeMsgs messages_;
        std::map<std::string, NFmiMetTime> idMap_;
    public:
        void addMessage(const HakeMsg &messagePtr);
        const SetOfHakeMsgs& peekMessages() const;
        size_t getSize() const;
        void clearMessages();
    };
}
