#pragma once
#include "WmsClient.h"
#include "QueryBuilder.h"

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
        BackgroundFetcher(std::shared_ptr<cppback::BackgroundManager> bManager, int howManyBackward, int howManyForward);

        void fetch(WmsClient& client, QueryBuilder qb, const NFmiMetTime& time, int editorTimeStepInMinutes)
        {
            if(shouldNotDoBackgroundFetching(client, qb))
            {
                return;
            }
            update(qb, time, editorTimeStepInMinutes);
            fetchQueriesInTheBackground(client, forwardQueries_);
            fetchQueriesInTheBackground(client, backwardQueries_);
        }

    private:
        void fetchQueriesInTheBackground(WmsClient& client, std::vector<WmsQuery> queries)
        {
            for(const auto& query : queries)
            {
                bManager_->addTask([&client, query, &running = running_]
                {
                    ++running;
                    client.getImage(query);
                    --running;
                });
            }
        }

        void update(const QueryBuilder& qb, const NFmiMetTime& time, int timeStepInMinutes)
        {
            createQueries(qb, time, timeStepInMinutes, forwardQueries_);
            createQueries(qb, time, -timeStepInMinutes, backwardQueries_);
        }

        void createQueries(QueryBuilder qb, NFmiMetTime time, int timeStepInMinutes, std::vector<WmsQuery>& queries)
        {
            for(auto& query : queries)
            {
                time.ChangeByMinutes(timeStepInMinutes);
                query = qb.setTime(time).build();
            }
        }

        bool shouldNotDoBackgroundFetching(const WmsClient& client, const QueryBuilder& qb) const
        {
            return running_ > 0 || client.isCached(qb.build());
        }
    };
}