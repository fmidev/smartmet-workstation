#pragma once

#include "NFmiDataParamModifier.h"
#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"
#include "NFmiLimitChecker.h"
#include "boost/shared_ptr.hpp"

class NFmiThreadCallBacks;
class NFmiFastQueryInfo;
class NFmiDrawParam;
class NFmiEditorControlPointManager;
class NFmiAreaMaskList;
class NFmiTimeDescriptor;

class NFmiControlPointObservationBlender : public NFmiDataParamControlPointModifier
{
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> itsObservationInfos;
    NFmiMetTime itsActualFirstTime;
 public:

     struct BlendingDataHelper
     {
         BlendingDataHelper();

         NFmiDataMatrix<float> changeField;
         unsigned long blendingTimeSize = 0;
         unsigned long blendingTimeIndex = 0;
         NFmiLimitChecker limitChecker;
     };

     NFmiControlPointObservationBlender(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
         unsigned long theAreaMask, boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, const NFmiRect &theCPGridCropRect,
         bool theUseGridCrop, const NFmiPoint &theCropMarginSize, std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, const NFmiMetTime &actualFirstTime, const NFmiGriddingProperties &griddingProperties);

     bool ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks);
     static bool GetObservationsToChangeValueFields(boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, const NFmiTimeDescriptor &allowedTimeRange);
     static bool SeekClosestObsBlenderData(const NFmiLocation &cpLocation, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, const NFmiTimeDescriptor &allowedTimeRange, float &zValueOut, boost::shared_ptr<NFmiFastQueryInfo> &closestObsBlenderInfoOut);
     static NFmiTimeDescriptor CalcAllowedObsBlenderTimes(const NFmiMetTime &actualFirstTime, const NFmiMetTime &firstEditedTime, long expirationTimeInMinutes);

protected:
    void FillZeroChangeValuesForMissingCpPoints(std::vector<float> &zValues);
    NFmiDataMatrix<float> CalcChangeField(const NFmiDataMatrix<float> &analysisField);
    bool MakeBlendingOperation(NFmiTimeDescriptor &blendingTimes);
    bool DoBlendingDataGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues);
    void DoCroppedPointCalculations(const NFmiDataMatrix<float> &usedData, size_t xIndex, size_t yIndex, float maskFactor) override;
    void DoNormalPointCalculations(const NFmiDataMatrix<float> &usedData, unsigned long locationIndex, float maskFactor) override;
    void MakeAnalysisModificationToStartOfEditedData(unsigned long firstModifiedTimeIndex);

    BlendingDataHelper itsBlendingDataHelper;
};
