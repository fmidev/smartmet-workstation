#include "ParameterSelectionUtils.h"
#include <algorithm>

namespace AddParams
{
    const SingleRowItem* findDataRowItem(const std::string &uniqueRowItemId, const std::vector<SingleRowItem> &dialogRowData)
    {
        auto iter = std::find_if(dialogRowData.begin(), dialogRowData.end(),
            [&uniqueRowItemId]
        (const auto &singleRowItem)
        {
            return uniqueRowItemId == singleRowItem.uniqueDataId();
        });

        if(iter != dialogRowData.end())
            return &(*iter);
        else
            return nullptr;
    }
}
