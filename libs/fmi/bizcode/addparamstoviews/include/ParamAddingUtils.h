#pragma once

#include "SingleRowItem.h"
#include <vector>

namespace AddParams
{
    const SingleRowItem* findDataRowItem(const std::string &uniqueRowItemId, const std::vector<SingleRowItem> &dialogRowData);
}
