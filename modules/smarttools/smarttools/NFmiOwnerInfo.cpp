
#include "NFmiOwnerInfo.h"
#include <newbase/NFmiQueryData.h>

NFmiOwnerInfo::NFmiOwnerInfo(void)
    : NFmiFastQueryInfo(), itsDataPtr(), itsDataFileName(), itsDataFilePattern()
{
}

NFmiOwnerInfo::NFmiOwnerInfo(NFmiQueryData *theOwnedData,
                             NFmiInfoData::Type theDataType,
                             const std::string &theDataFileName,
                             const std::string &theDataFilePattern)
    : NFmiFastQueryInfo(theOwnedData),
      itsDataPtr(theOwnedData),
      itsDataFileName(theDataFileName),
      itsDataFilePattern(theDataFilePattern)
{
  if (theOwnedData == 0)
    throw std::runtime_error(
        "Error in NFmiOwnerInfo konstructor, given queryData was NULL pointer.");
  DataType(theDataType);
}

NFmiOwnerInfo::NFmiOwnerInfo(const NFmiOwnerInfo &theInfo)
    : NFmiFastQueryInfo(theInfo),
      itsDataPtr(theInfo.itsDataPtr),
      itsDataFileName(theInfo.itsDataFileName),
      itsDataFilePattern(theInfo.itsDataFilePattern)
{
}

NFmiOwnerInfo::~NFmiOwnerInfo(void)
{
}

NFmiOwnerInfo &NFmiOwnerInfo::operator=(const NFmiOwnerInfo &theInfo)
{
  NFmiFastQueryInfo::operator=(theInfo);
  itsDataPtr = theInfo.itsDataPtr;
  itsDataFileName = theInfo.itsDataFileName;
  itsDataFilePattern = theInfo.itsDataFilePattern;

  return *this;
}

NFmiOwnerInfo *NFmiOwnerInfo::Clone(void) const
{
  NFmiQueryData *cloneData = itsDataPtr.get()->Clone();
  return new NFmiOwnerInfo(cloneData, DataType(), itsDataFileName, itsDataFilePattern);
}
