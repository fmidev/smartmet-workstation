#pragma once
#include "wmssupport/WmsClient.h"
#include "wmssupport/QueryBuilder.h"

#include "NFmiMetTime.h"

#include <cppback/background-manager.h>

#include <atomic>

namespace Wms
{
    class BackgroundFetcher
    {
        std::shared_ptr<cppback::BackgroundManager> bManager_;

        std::vector<WmsQuery> forwardQueries_;
        std::vector<WmsQuery> backwardQueries_;;

        std::atomic_int running_ = 0;
    public:
        BackgroundFetcher(const std::shared_ptr<cppback::BackgroundManager> &bManager, int howManyBackward, int howManyForward);

        void fetch(WmsClient& client, const QueryBuilder &qb, const NFmiMetTime& time, int editorTimeStepInMinutes);

    private:
        void fetchQueriesInTheBackground(WmsClient& client, const std::vector<WmsQuery> &queries);
        void update(const QueryBuilder& qb, const NFmiMetTime& time, int timeStepInMinutes, bool hasTimeDimension);
        void createQueries(const QueryBuilder &qb, const NFmiMetTime &time, int timeStepInMinutes, std::vector<WmsQuery>& queries);
        bool shouldNotDoBackgroundFetching(const WmsClient& client, const QueryBuilder& qb) const;
    };
}
