#include "NFmiCachedDataFileInfo.h"
#include "NFmiHelpDataInfo.h"

NFmiCachedDataFileInfo::NFmiCachedDataFileInfo() = default;

bool NFmiCachedDataFileInfo::IsDataCached(const NFmiHelpDataInfo & theDataInfo)
{
    // kSatelData-tyyppi ei ole queryDataa, nämä ignoorataan (kuvatkin kyllä on tarkoitus joskus cachettaa).
    if(theDataInfo.DataType() != NFmiInfoData::kSatelData)
        return true;
    else
        return false;
}
