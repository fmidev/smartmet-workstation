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
        , leafNode_(false)
        , level_(nullptr)
        , treeDepth_(0)
        , displayName_()
        , origTime_()
    {}

    SingleRowItem::SingleRowItem(RowType rowType, const std::string &itemName, unsigned long itemId, 
        bool dialogTreeNodeCollapsed, const std::string& uniqueDataId, NFmiInfoData::Type dataType, 
        unsigned long parentItemId, const std::string &parentItemName, const bool leafNode, 
        const std::shared_ptr<NFmiLevel> &level, const int treeDepth, std::string &displayName
        , std::string &origTime)
        : rowType_(rowType)
        , itemName_(itemName)
        , itemId_(itemId)
        , dialogTreeNodeCollapsed_(dialogTreeNodeCollapsed)
        , uniqueDataId_(uniqueDataId)
        , dataType_(dataType)
        , parentItemId_(parentItemId)
        , parentItemName_(parentItemName)
        , leafNode_(leafNode)
        , level_(level)
        , treeDepth_(treeDepth)
        , displayName_(displayName)
        , origTime_(origTime)
    {
        if(treeDepth_ == 0) { treeDepth_ = getTreeDepth(rowType); }
        if(displayName_.empty()) { displayName_ = itemName; }
    }

    SingleRowItem::~SingleRowItem() = default;

    int SingleRowItem::getTreeDepth(AddParams::RowType rowType)
    {
        switch(rowType)
        {
        case AddParams::kCategoryType:
            return 1;
        case AddParams::kProducerType:
            return 2;
        case AddParams::kDataType:
            return 3;
        case AddParams::kParamType:
            return 4;
        case AddParams::kSubParamType:
        case AddParams::kLevelType:
            return 5;
        case AddParams::kSubParamLevelType:
            return 6;
        default:
            throw std::runtime_error("Error in getDialogTreeDepth function: Illegal row-item type");
        }
    }

}
