#include "NFmiSatelliteImageCacheHelpers.h"

NFmiImageData::NFmiImageData()
:mErrorMessage()
,mImage()
,mState(kUninitialized)
,mImageTime(NFmiMetTime::gMissingTime)
{
}

NFmiImageData::NFmiImageData(const NFmiMetTime &time)
:mErrorMessage()
,mImage()
,mState(kUninitialized)
, mImageTime(time)
{
}

bool NFmiImageData::IsImageUsable() const
{
    // Tämä on ehkä vähän vainoharhaista tutkia kaikkia kolmea dataosaa, 
    // periaatteessa jos mImage:ssa on jotain, se riittäisi.
    if(mState == kOk && mErrorMessage.empty() && mImage != nullptr)
        return true;
    else
        return false;
}
