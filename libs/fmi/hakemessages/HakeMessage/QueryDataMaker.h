#pragma once

#include "HakeMessage/HakeMessages.h"

#include <cppback/background-manager.h>

#include <memory>

class NFmiQueryData;
class NFmiProducer;

namespace HakeMessage
{
    class QueryDataMaker
    {
        std::shared_ptr<cppback::BackgroundManager> bManager_;
    public:
        QueryDataMaker(std::shared_ptr<cppback::BackgroundManager> bManager);
        std::unique_ptr<NFmiQueryData> createQueryDataFrom(const std::multiset<HakeMsg>& messages, const NFmiProducer &producer) const;
    private:
        NFmiTimeDescriptor createTimeDescriptor(const HakeMessages::SetOfHakeMsgs& messages) const;
        std::unique_ptr<NFmiQueryData> createSkeletonForQueryData(const HakeMessages::SetOfHakeMsgs& messages, const NFmiProducer &producer) const;
        std::unique_ptr<NFmiQueryData> fillQueryData(std::unique_ptr<NFmiQueryData> queryData, const HakeMessages::SetOfHakeMsgs& messages) const;
    };
}

