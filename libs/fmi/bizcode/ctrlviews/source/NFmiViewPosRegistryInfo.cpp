#ifndef UNIX
#include "stdafx.h"
#endif // UNIX
#include "NFmiViewPosRegistryInfo.h"

#ifndef UNIX
NFmiViewPosRegistryInfo::NFmiViewPosRegistryInfo(const CRect &rectDefault, const std::string &winRegistryKeyStr)
:mRectDefault(rectDefault)
,mWinRegistryKeyStr(winRegistryKeyStr)
{
}
#endif // UNIX

