#include "SingleRowItem.h"

namespace AddParams
{

    SingleRowItem::SingleRowItem()
        :rowType_(kNoType)
        , itemName_()
        , itemId_(0)
        , dialogTreeNodeCollapsed_(false)
        , uniqueDataId_()
        , dataType_(NFmiInfoData::kNoDataType)
        , parentItemId_(0)
    {}

    SingleRowItem::SingleRowItem(RowType rowType, const std::string &itemName, unsigned long itemId, bool dialogTreeNodeCollapsed, const std::string& uniqueDataId, NFmiInfoData::Type dataType, unsigned long parentItemId, const std::string &parentItemName)
        : rowType_(rowType)
        , itemName_(itemName)
        , itemId_(itemId)
        , dialogTreeNodeCollapsed_(dialogTreeNodeCollapsed)
        , uniqueDataId_(uniqueDataId)
        , dataType_(dataType)
        , parentItemId_(parentItemId)
        , parentItemName_(parentItemName)
    {}

    SingleRowItem::~SingleRowItem() = default;

}
