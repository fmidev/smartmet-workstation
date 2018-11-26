#pragma once

#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiQueryInfo;
class NFmiFastQueryInfo;
class NFmiHelpDataInfo;

namespace AddParams
{
    class SingleData
    {
        // Data's filefilter is unique id in SmartMet
        std::string uniqueDataId_;
        std::string dataName_;
        std::unique_ptr<NFmiQueryInfo> latestMetaData_;
        std::string latestDataFilePath_;
        NFmiInfoData::Type dataType_;
        std::string totalLocalPath_;
        std::string totalServerPath_;

    public:
        SingleData();
        ~SingleData();

        bool updateData(const boost::shared_ptr<NFmiFastQueryInfo> &info, const NFmiHelpDataInfo *helpDataInfo = nullptr);
        const std::string& uniqueDataId() const { return uniqueDataId_; }
        const std::string& dataName() const { return dataName_; }
        const NFmiInfoData::Type& dataType() const { return dataType_; }
        unsigned long producerId() const;
        std::vector<SingleRowItem> makeDialogRowData() const;
        std::string OrigOrLastTime() const;
        const std::string& totalLocalPath() const { return totalLocalPath_; }
        const std::string& totalServerPath() const { return totalServerPath_; }
    };
}
