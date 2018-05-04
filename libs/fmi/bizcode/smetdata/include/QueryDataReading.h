#pragma once

#include <string>
#include <memory>

class NFmiQueryData;

// SmartMet's queryData reading and error handlin code is moved to own separate functions.
// This way we can separate this code (used by other classes) from the NFmiEditMapGeneralDataDoc class completely.
namespace QueryDataReading
{
    std::unique_ptr<NFmiQueryData> ReadLatestDataWithFileFilterAfterTimeStamp(const std::string& theFileFilter, time_t theLimitingTimeStamp, std::string& theFileNameOut, time_t& theTimeStampOut);
    std::unique_ptr<NFmiQueryData> ReadDataFromFile(const std::string& fileName, bool useMemoryMapping);

}

