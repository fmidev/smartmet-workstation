#include "wmssupport/BackgroundFetcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    BackgroundFetcher::BackgroundFetcher(const std::shared_ptr<cppback::BackgroundManager> &bManager, int howManyBackward, int howManyForward)
        :bManager_{ bManager }
        , forwardQueries_(howManyForward)
        , backwardQueries_(howManyBackward)
    {
    }

    void BackgroundFetcher::fetch(WmsClient& client, const QueryBuilder &qb, const NFmiMetTime& time, int editorTimeStepInMinutes)
    {
        if(shouldNotDoBackgroundFetching(client, qb))
        {
            return;
        }
        update(qb, time, editorTimeStepInMinutes, true);
        fetchQueriesInTheBackground(client, forwardQueries_);
        fetchQueriesInTheBackground(client, backwardQueries_);
    }

    void BackgroundFetcher::fetchQueriesInTheBackground(WmsClient& client, const std::vector<WmsQuery> &queries)
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

    void BackgroundFetcher::update(const QueryBuilder& qb, const NFmiMetTime& time, int timeStepInMinutes, bool hasTimeDimension)
    {
        if(hasTimeDimension)
        {
            createQueries(qb, time, timeStepInMinutes, forwardQueries_);
            createQueries(qb, time, -timeStepInMinutes, backwardQueries_);
        }
    }

    void BackgroundFetcher::createQueries(const QueryBuilder &qb, const NFmiMetTime &time, int timeStepInMinutes, std::vector<WmsQuery>& queries)
    {
        auto usedTime = time;
        auto usedQueryBuilder = qb;
        for(auto& query : queries)
        {
            usedTime.ChangeByMinutes(timeStepInMinutes);
            query = usedQueryBuilder.setTime(usedTime, true).build();
        }
    }

    bool BackgroundFetcher::shouldNotDoBackgroundFetching(const WmsClient& client, const QueryBuilder& qb) const
    {
        return running_ > 0 || client.isCached(qb.build());
    }
}