#include "wmssupport/BackgroundFetcher.h"

namespace Wms
{
    BackgroundFetcher::BackgroundFetcher(std::shared_ptr<cppback::BackgroundManager> bManager, int howManyBackward, int howManyForward)
        :bManager_{ bManager }
        , forwardQueries_(howManyForward)
        , backwardQueries_(howManyBackward)
    {
    }

    void BackgroundFetcher::fetch(WmsClient& client, QueryBuilder qb, const NFmiMetTime& time, int editorTimeStepInMinutes)
    {
        if(shouldNotDoBackgroundFetching(client, qb))
        {
            return;
        }
        update(qb, time, editorTimeStepInMinutes);
        fetchQueriesInTheBackground(client, forwardQueries_);
        fetchQueriesInTheBackground(client, backwardQueries_);
    }

    void BackgroundFetcher::fetchQueriesInTheBackground(WmsClient& client, std::vector<WmsQuery> queries)
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

    void BackgroundFetcher::update(const QueryBuilder& qb, const NFmiMetTime& time, int timeStepInMinutes)
    {
        createQueries(qb, time, timeStepInMinutes, forwardQueries_);
        createQueries(qb, time, -timeStepInMinutes, backwardQueries_);
    }

    void BackgroundFetcher::createQueries(QueryBuilder qb, NFmiMetTime time, int timeStepInMinutes, std::vector<WmsQuery>& queries)
    {
        for(auto& query : queries)
        {
            time.ChangeByMinutes(timeStepInMinutes);
            query = qb.setTime(time).build();
        }
    }

    bool BackgroundFetcher::shouldNotDoBackgroundFetching(const WmsClient& client, const QueryBuilder& qb) const
    {
        return running_ > 0 || client.isCached(qb.build());
    }
}