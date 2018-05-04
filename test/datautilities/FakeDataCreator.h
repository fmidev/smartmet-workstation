#pragma once

#include "NFmiQueryData.h"
#include "NFmiProducerName.h"
#include "NFmiTimeList.h"
#include "NFmiParamDescriptor.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiAreaFactory.h"

#include <memory>

namespace FakeDataCreator
{
    namespace
    {
        NFmiHPlaceDescriptor createPlaceDescriptor(const std::string &areaString, unsigned long x, unsigned long y);
        NFmiParamDescriptor createParamsDescriptor();
        NFmiTimeDescriptor createTimeDescriptor();
        decltype(auto) createQueryDataFrom(NFmiQueryInfo metaData);

        NFmiHPlaceDescriptor createPlaceDescriptor(const std::string &areaString, unsigned long x, unsigned long y)
        {
            auto createdArea = NFmiAreaFactory::Create(areaString);
            NFmiGrid grid(createdArea.get(), x, y);
            return NFmiHPlaceDescriptor(grid);
        }

        NFmiParamDescriptor createParamsDescriptor()
        {
            NFmiProducer dataProducer(kFmiHakeMessages, "DATA");
            NFmiParamBag params;
            params.Add(NFmiDataIdent(NFmiParam(kFmiHakeMessageType, "Param Name"), dataProducer));

            return NFmiParamDescriptor(params);
        }

        NFmiTimeDescriptor createTimeDescriptor()
        {
            NFmiMetTime origTime = NFmiMetTime::now();
            NFmiTimeList times;

            times.Add(new NFmiMetTime(origTime), true, true);
            return NFmiTimeDescriptor(origTime, times);
        }

        decltype(auto) createQueryDataFrom(NFmiQueryInfo metaData)
        {
            auto data = std::make_unique<NFmiQueryData>(metaData);
            data->Init();
            return data;
        }

        decltype(auto) createFakeData(const std::string &areaString, unsigned long x, unsigned long y)
        {
            NFmiVPlaceDescriptor vPlaceDesc;
            NFmiHPlaceDescriptor hPlaceDesc = createPlaceDescriptor(areaString, x, y);
            NFmiParamDescriptor paramDesc = createParamsDescriptor();
            NFmiTimeDescriptor timeDesc = createTimeDescriptor();

            NFmiQueryInfo metaData(paramDesc, timeDesc, hPlaceDesc, vPlaceDesc);
            return createQueryDataFrom(metaData);
        }
    }

    inline decltype(auto) createFastQueryInfoFrom(const std::string &areaString, unsigned long x, unsigned long y)
    {
        return NFmiFastQueryInfo(createFakeData(areaString, x, y).get());
    }
}