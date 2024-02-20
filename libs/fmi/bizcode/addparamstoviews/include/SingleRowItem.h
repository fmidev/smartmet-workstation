#pragma once
#include <string>
#include <memory>
#include <vector>

#include "NFmiInfoData.h"
#include "NFmiLevel.h"


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
	
	// This is used to populate ParameterSelection dialog's rows
    class SingleRowItem
    {
        // Data's file filter is unique id in SmartMet
        RowType rowType_;
        std::string itemName_;
        std::string displayName_;
        unsigned long itemId_;
        bool dialogTreeNodeCollapsed_;
        // If rowItem is kDataType, store here data's uniqueDataId (its file path filter).
        // We need to be able to find individual data from last dialogData to remember all the
        // collapsed and uncollapsed dialog tree nodes.
        std::string uniqueDataId_;
        NFmiInfoData::Type dataType_;
        unsigned long parentItemId_;
        std::string parentItemName_;
        bool leafNode_;
		bool crossSectionLeafNode_;
        std::shared_ptr<NFmiLevel> level_;
        int treeDepth_;
        std::string origTime_;
        std::string totalFilePath_;
        std::string searchWords_;
        FmiInterpolationMethod interpolationType_ = kNoneInterpolation;
        bool wmsLayerHasTimeDimension_ = false;

    public:
        SingleRowItem();
        SingleRowItem(RowType rowType, const std::string &itemName, unsigned long itemId,
            bool dialogTreeNodeCollapsed, const std::string& uniqueDataId, NFmiInfoData::Type dataType,
            unsigned long parentItemId = 0, const std::string &parentItemName = "", const bool leafNode = false,
            const std::shared_ptr<NFmiLevel>& level = nullptr, const int treeDepth = 0, const std::string &displayName = std::string(), 
            const std::string &origTime = std::string(), const std::string &totalFilePath = std::string());
        ~SingleRowItem();

        RowType rowType() const { return rowType_; }
        void rowType(RowType rowType) { rowType_ = rowType; }
        const std::string& itemName() const { return itemName_; }
        void itemName(const std::string &itemName) { itemName_ = itemName; }
        const std::string& displayName() const { return displayName_; }
        void displayName(const std::string &displayName) { displayName_ = displayName; }
        unsigned long itemId() const { return itemId_; }
        void itemId(unsigned long itemId) { itemId_ = itemId; }
        bool dialogTreeNodeCollapsed() const { return dialogTreeNodeCollapsed_; }
        void dialogTreeNodeCollapsed(bool newValue) { dialogTreeNodeCollapsed_ = newValue; }
        const std::string& uniqueDataId() const { return uniqueDataId_; }
        NFmiInfoData::Type dataType() const { return dataType_; }
        void dataType(NFmiInfoData::Type dataType) { dataType_ = dataType; }
        unsigned long parentItemId() const { return parentItemId_; }
        const std::string& parentItemName() const { return parentItemName_; }
        bool leafNode() const { return leafNode_; }
		bool crossSectionLeafNode() const { return crossSectionLeafNode_; }
		void crossSectionLeafNode(bool newValue) { crossSectionLeafNode_ = newValue; }
        const std::shared_ptr<NFmiLevel>& level() const { return level_; }
        void level(const std::shared_ptr<NFmiLevel>& level) {level_ = level; }
        int treeDepth() const { return treeDepth_; }
        int getTreeDepth(AddParams::RowType rowType);
        const std::string& origTime() const { return origTime_; }
        void origTime(const std::string &origTime) { origTime_ = origTime; }
        const std::string& totalFilePath() const { return totalFilePath_; }
        void totalFilePath(const std::string &totalFilePath) { totalFilePath_ = totalFilePath; }
        const std::string& searchWords() const { return searchWords_; }
        FmiInterpolationMethod interpolationType() const { return interpolationType_; }
        void interpolationType(FmiInterpolationMethod interpolationType) { interpolationType_ = interpolationType; }
        bool wmsLayerHasTimeDimension() const { return wmsLayerHasTimeDimension_; }
        void wmsLayerHasTimeDimension(bool hasTimeDimension) { wmsLayerHasTimeDimension_ = hasTimeDimension; }

    private:
        void createSearchWords();
    };
}
