#pragma once

#include "SingleRowItem.h"
#include <vector>

namespace AddParams
{
    const SingleRowItem* findDataRowItem(const std::string& uniqueRowItemId, const std::vector<SingleRowItem>& dialogRowData);
    const SingleRowItem* findParameterDataRowItem(const std::string &uniqueRowItemId, unsigned long parameterId, const std::vector<SingleRowItem> &dialogRowData);
}
