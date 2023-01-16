#include "NFmiCachedDataFileInfo.h"
#include "NFmiHelpDataInfo.h"

NFmiCachedDataFileInfo::NFmiCachedDataFileInfo() = default;

bool NFmiCachedDataFileInfo::IsDataCached(const NFmiHelpDataInfo & theDataInfo)
{
    // kSatelData-tyyppi ei ole queryDataa, n�m� ignoorataan (kuvatkin kyll� on tarkoitus joskus cachettaa).
    if(theDataInfo.DataType() != NFmiInfoData::kSatelData)
        return true;
    else
        return false;
}
