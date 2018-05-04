#include "HakeMessage/QueryDataMaker.h"

#include "HakeMessage/HakeMsg.h"

#include "NFmiQueryData.h"
#include "NFmiTimeList.h"
#include "NFmiParamDescriptor.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiMetTime.h"

#include <cppback/loop-signal-checker.h>

namespace HakeMessage
{
    namespace
    {
        NFmiHPlaceDescriptor createPlaceDescriptor()
        {
            auto locations = NFmiLocationBag{};
            locations.AddLocation(NFmiStation{ 1, "HAKE station" });
            return NFmiHPlaceDescriptor{ locations };
        }

        NFmiParamDescriptor createParamsDescriptor(const NFmiProducer &producer)
        {
            auto params = NFmiParamBag{};
            params.Add(NFmiDataIdent{ NFmiParam{ kFmiHakeMessageType, "Hake Message Type" }, producer });
            params.Add(NFmiDataIdent{ NFmiParam{ kFmiLongitude, "lon" }, producer });
            params.Add(NFmiDataIdent{ NFmiParam{ kFmiLatitude, "lat" }, producer });

            return NFmiParamDescriptor{ params };
        }
    }

    QueryDataMaker::QueryDataMaker(std::shared_ptr<cppback::BackgroundManager> bManager)
        :bManager_{ bManager }
    {
    }

    std::unique_ptr<NFmiQueryData> QueryDataMaker::createQueryDataFrom(const std::multiset<HakeMsg>& messages, const NFmiProducer &producer) const
    {
        return fillQueryData(createSkeletonForQueryData(messages, producer), messages);
    }

    NFmiTimeDescriptor QueryDataMaker::createTimeDescriptor(const HakeMessages::SetOfHakeMsgs& messages) const
    {
        auto origTime = NFmiMetTime::now();
        auto times = NFmiTimeList{};

        auto checker = cppback::LoopSignalChecker(messages.size() / 10, "HakeDataMaker");
        for(const auto &hakeMessage : messages)
        {
            times.Add(new NFmiMetTime(hakeMessage.StartTime()), true, true);

            checker.check(*bManager_);
        }
        return NFmiTimeDescriptor{ origTime, times };
    }

    std::unique_ptr<NFmiQueryData> QueryDataMaker::createSkeletonForQueryData(const HakeMessages::SetOfHakeMsgs& messages, const NFmiProducer &producer) const
    {
        auto metaData = NFmiQueryInfo{
            createParamsDescriptor(producer),
            createTimeDescriptor(messages),
            createPlaceDescriptor(),
            NFmiVPlaceDescriptor{}
        };

        auto hakeData = std::make_unique<NFmiQueryData>(metaData);
        hakeData->Init();
        return hakeData;
    }

    std::unique_ptr<NFmiQueryData> QueryDataMaker::fillQueryData(std::unique_ptr<NFmiQueryData> queryData, const HakeMessages::SetOfHakeMsgs& messages) const
    {
        auto checker = cppback::LoopSignalChecker(messages.size() / 10, "HakeDataMaker");

        auto hakeInfo = std::make_unique<NFmiFastQueryInfo>(queryData.get());
        for(const auto &hakeMessage : messages)
        {
            hakeInfo->Param(kFmiHakeMessageType);
            hakeInfo->FloatValue(hakeMessage.Category());
            hakeInfo->Param(kFmiLongitude);
            hakeInfo->FloatValue(hakeMessage.LatlonPoint().X());
            hakeInfo->Param(kFmiLatitude);
            hakeInfo->FloatValue(hakeMessage.LatlonPoint().Y());

            hakeInfo->NextTime();

            checker.check(*bManager_);
        }
        return std::move(queryData);
    }
}
