#include "SingleRowItem.h"
#include "NFmiLevel.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
        , totalFilePath_()
        , searchWords_()
		, crossSectionLeafNode_(false)
    {}

    SingleRowItem::SingleRowItem(RowType rowType, const std::string &itemName, unsigned long itemId, 
        bool dialogTreeNodeCollapsed, const std::string& uniqueDataId, NFmiInfoData::Type dataType, 
        unsigned long parentItemId, const std::string &parentItemName, const bool leafNode, 
        const std::shared_ptr<NFmiLevel> &level, const int treeDepth, const std::string &displayName
        , const std::string &origTime, const std::string &totalFilePath)
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
        , totalFilePath_(totalFilePath)
		, crossSectionLeafNode_(false)
    {
        if(treeDepth_ == 0) { treeDepth_ = getTreeDepth(rowType); }
        if(displayName_.empty()) { displayName_ = itemName; }
        createSearchWords();
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

    void SingleRowItem::createSearchWords()
    {
        searchWords_ += ((!itemName_.empty()) ? itemName_ + " " : "");
        searchWords_ += ((!displayName_.empty()) ? displayName_ + " " : "");
        searchWords_ += ((!std::to_string(itemId_).empty()) ? std::to_string(itemId_) + " " : "");
        searchWords_ += (level_ ? std::string(level_->GetName()) + " " : "");
        searchWords_ += ((!parentItemName_.empty()) ? parentItemName_ + " " : "");
        searchWords_ += ((!totalFilePath_.empty()) ? totalFilePath_ + " " : "");
        searchWords_ += ((parentItemId_ != 0) ? std::to_string(parentItemId_) + " " : "");
    }

}
