#pragma once

// Compatibility: Several NFmiInfoData helper functions were removed from
// the system smartmet-library-newbase. Provide them here for the Linux port.
// On Windows the local newbase module still has these functions.

#include "NFmiInfoData.h"

#ifdef UNIX
namespace NFmiInfoData
{

inline bool IsModelRunBasedData(NFmiInfoData::Type dataType)
{
    switch(dataType)
    {
    case NFmiInfoData::kEditable:
    case NFmiInfoData::kViewable:
    case NFmiInfoData::kCopyOfEdited:
    case NFmiInfoData::kKepaData:
    case NFmiInfoData::kClimatologyData:
    case NFmiInfoData::kHybridData:
    case NFmiInfoData::kFuzzyData:
    case NFmiInfoData::kVerificationData:
    case NFmiInfoData::kModelHelpData:
    case NFmiInfoData::kEditingHelpData:
        return true;
    default:
        return false;
    }
}

inline bool IsLatestOnlyBasedData(NFmiInfoData::Type dataType)
{
    switch(dataType)
    {
    case NFmiInfoData::kObservations:
    case NFmiInfoData::kAnalyzeData:
    case NFmiInfoData::kFlashData:
    case NFmiInfoData::kTrajectoryHistoryData:
    case NFmiInfoData::kSingleStationRadarData:
    case NFmiInfoData::kStationary:
        return true;
    default:
        return false;
    }
}

} // namespace NFmiInfoData
#endif
