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

     bool ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks);
     static float BlendData(float editedDataValue, float changeValue, unsigned long timeSize, unsigned long timeIndex, const NFmiDataParamModifier::LimitChecker &limitChecker);

protected:
    bool GetObservationsToChangeValueFields(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, const NFmiTimeDescriptor &allowedTimeRange, double maxAllowedDistanceToStationInKm);
    void FillZeroChangeValuesForMissingCpPoints(std::vector<float> &zValues);
    NFmiDataMatrix<float> CalcChangeField(const NFmiDataMatrix<float> &analysisField);
    bool MakeBlendingOperation(const NFmiDataMatrix<float> &changeField, NFmiTimeDescriptor &blendingTimes);
};
