#include "NFmiSatelliteImageCacheHelpers.h"

NFmiImageData::NFmiImageData()
:mErrorMessage()
#ifndef UNIX
,mImage()
#endif
,mState(kUninitialized)
,mImageTime(NFmiMetTime::gMissingTime)
{
}

NFmiImageData::NFmiImageData(const NFmiMetTime &time)
:mErrorMessage()
#ifndef UNIX
,mImage()
#endif
,mState(kUninitialized)
, mImageTime(time)
{
}

bool NFmiImageData::IsImageUsable() const
{
    // T�m� on ehk� v�h�n vainoharhaista tutkia kaikkia kolmea dataosaa,
    // periaatteessa jos mImage:ssa on jotain, se riitt�isi.
#ifndef UNIX
    if(mState == kOk && mErrorMessage.empty() && mImage != nullptr)
#else
    if(mState == kOk && mErrorMessage.empty())
#endif
        return true;
    else
        return false;
}
