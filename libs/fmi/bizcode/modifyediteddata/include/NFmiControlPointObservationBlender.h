#pragma once

#include "NFmiDataParamModifier.h"
#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"

class NFmiThreadCallBacks;
class NFmiFastQueryInfo;
class NFmiDrawParam;
class NFmiEditorControlPointManager;
class NFmiAreaMaskList;
class NFmiTimeDescriptor;

class NFmiControlPointObservationBlender : public NFmiDataParamControlPointModifier
{
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> itsObservationInfos;
    NFmiMetTime itsActualFirstTime;
 public:
     NFmiControlPointObservationBlender(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
         unsigned long theAreaMask, boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, const NFmiRect &theCPGridCropRect,
         bool theUseGridCrop, const NFmiPoint &theCropMarginSize, checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, const NFmiMetTime &actualFirstTime);

     void DoTimeSerialModifications(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks);
};
