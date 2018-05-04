#include "wmssupport/BackgroundFetcher.h"

namespace Wms
{
    BackgroundFetcher::BackgroundFetcher(std::shared_ptr<cppback::BackgroundManager> bManager, int howManyBackward, int howManyForward)
        :bManager_{ bManager }
        , forwardQueries_(howManyForward)
        , backwardQueries_(howManyBackward)
    {
    }
}