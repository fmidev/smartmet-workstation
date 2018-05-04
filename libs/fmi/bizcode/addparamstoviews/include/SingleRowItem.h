#pragma once

#include "NFmiInfoData.h"
#include <string>

namespace AddParams
{
	enum RowType
	{
		kNoType = 0,
        kSubParamLevelType,
        kLevelType,
        kSubParamType,
		kParamType,
		kDataType,
		kProducerType,
		kCategoryType,
	};
	
	// This is used to populate ParamAdding dialog's rows
    class SingleRowItem
    {
        // Data's filefilter is unique id in SmartMet
        RowType rowType_;
        std::string itemName_;
        unsigned long itemId_;
        bool dialogTreeNodeCollapsed_;
        // If rowItem is kDataType, store here data's uniqueDataId (its file path filter).
        // We need to be able to find individual data from last dialogData to remember all the
        // collapsed and uncollapsed dialog tree nodes.
        std::string uniqueDataId_;
        NFmiInfoData::Type dataType_;
        unsigned long parentItemId_;
        std::string parentItemName_;
    public:
        SingleRowItem();
        SingleRowItem(RowType rowType, const std::string &itemName, unsigned long itemId, bool dialogTreeNodeCollapsed, const std::string& uniqueDataId, NFmiInfoData::Type dataType, unsigned long parentItemId = 0, const std::string &parentItemName = "");
        ~SingleRowItem();

        RowType rowType() const { return rowType_; }
        void rowType(RowType rowType) { rowType_ = rowType; }
        const std::string& itemName() const { return itemName_; }
        void itemName(const std::string &itemName) { itemName_ = itemName; }
        unsigned long itemId() const { return itemId_; }
        void itemId(unsigned long itemId) { itemId_ = itemId; }
        bool dialogTreeNodeCollapsed() const { return dialogTreeNodeCollapsed_; }
        void dialogTreeNodeCollapsed(bool newValue) { dialogTreeNodeCollapsed_ = newValue; }
        const std::string& uniqueDataId() const { return uniqueDataId_; }
        NFmiInfoData::Type dataType() const { return dataType_; }
        void dataType(NFmiInfoData::Type dataType) { dataType_ = dataType; }
        unsigned long parentItemId() const { return parentItemId_; }
        const std::string& parentItemName() const { return parentItemName_; }
    };
}
