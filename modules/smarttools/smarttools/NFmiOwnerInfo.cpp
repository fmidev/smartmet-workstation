
#include "NFmiOwnerInfo.h"
#include <newbase/NFmiQueryData.h>

NFmiOwnerInfo::NFmiOwnerInfo()
    : NFmiFastQueryInfo(), itsDataPtr(), itsDataFileName(), itsDataFilePattern()
{
  SetupDataLoadedTimer(true);  // true = 'vanha' data
}

NFmiOwnerInfo::NFmiOwnerInfo(NFmiQueryData *theOwnedData,
                             NFmiInfoData::Type theDataType,
                             const std::string &theDataFileName,
                             const std::string &theDataFilePattern,
                             bool IsConsideredOldData)
    : NFmiFastQueryInfo(theOwnedData),
      itsDataPtr(theOwnedData),
      itsDataFileName(theDataFileName),
      itsDataFilePattern(theDataFilePattern)
{
  if (theOwnedData == 0)
    throw std::runtime_error(
        "Error in NFmiOwnerInfo konstructor, given queryData was NULL pointer.");
  DataType(theDataType);
  SetupDataLoadedTimer(IsConsideredOldData);
}

NFmiOwnerInfo::NFmiOwnerInfo(const NFmiOwnerInfo &theInfo)
    : NFmiFastQueryInfo(theInfo),
      itsDataPtr(theInfo.itsDataPtr),
      itsDataFileName(theInfo.itsDataFileName),
      itsDataFilePattern(theInfo.itsDataFilePattern),
      itsDataLoadedTimer(theInfo.itsDataLoadedTimer)
{
}

NFmiOwnerInfo::~NFmiOwnerInfo() {}

NFmiOwnerInfo &NFmiOwnerInfo::operator=(const NFmiOwnerInfo &theInfo)
{
  NFmiFastQueryInfo::operator=(theInfo);
  itsDataPtr = theInfo.itsDataPtr;
  itsDataFileName = theInfo.itsDataFileName;
  itsDataFilePattern = theInfo.itsDataFilePattern;
  itsDataLoadedTimer = theInfo.itsDataLoadedTimer;

  return *this;
}

NFmiOwnerInfo *NFmiOwnerInfo::Clone() const
{
  NFmiQueryData *cloneData = itsDataPtr.get()->Clone();
  auto ownerInfo =
      new NFmiOwnerInfo(cloneData, DataType(), itsDataFileName, itsDataFilePattern, false);
  ownerInfo->itsDataLoadedTimer = itsDataLoadedTimer;
  return ownerInfo;
}

double NFmiOwnerInfo::ElapsedTimeFromLoadInSeconds() const
{
  return itsDataLoadedTimer.elapsedTimeInSeconds();
}

// IsConsideredOldData tarkoittaa ett‰ datasta halutaan tehd‰ 'vanhaa' ja timerin aikaa
// siirret‰‰n keinotekoisesti taaksep‰in. Muuten timer aloittaa nykyhetkest‰ ajanmittauksen.
void NFmiOwnerInfo::SetupDataLoadedTimer(bool IsConsideredOldData)
{
  if (IsConsideredOldData)
  {
    // Siirret‰‰n vanhan datan alkua vaikka 4 minuuttia taaksep‰in, jolloin ne ovat 'uusia' vain 1
    // minuutin.
    const int oldDataTimeChangeInMS = -1000 * 60 * 4;
    itsDataLoadedTimer = NFmiNanoSecondTimer(oldDataTimeChangeInMS);
  }
  else
    itsDataLoadedTimer = NFmiNanoSecondTimer();
}
