#pragma once

#include "NFmiMetTime.h"
#include "NFmiDataMatrix.h"
#include <boost/shared_ptr.hpp>

class NFmiFastQueryInfo;
class NFmiLevel;
class NFmiPoint;

namespace NFmiFastInfoUtils
{
    struct MetaWindVectorParamUsage
    {
        // Data has no kFmiWindVectorMS parameter, so we have to make meta parameter of it.
        bool fUseMetaWindVectorParam = false;
        // Data has wind speed and wind direction (that can be used to calculate wind vector)
        bool fHasWsAndWd = false;
        // data has u- and v-components of the wind (that can be used to calculate wind vector)
        bool fHasWindComponents = false;
    };

bool IsInfoShipTypeData(NFmiFastQueryInfo &theInfo);
void SetSoundingDataLevel(const NFmiLevel &theWantedSoundingPressureLevel, NFmiFastQueryInfo &info);
std::string GetTotalDataFilePath(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
bool IsYearLongData(const boost::shared_ptr<NFmiFastQueryInfo> &info);
bool IsModelClimatologyData(const boost::shared_ptr<NFmiFastQueryInfo> &info);
NFmiMetTime GetUsedTimeIfModelClimatologyData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                              const NFmiMetTime &theTime);
bool IsMovingSoundingData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
bool FindTimeIndicesForGivenTimeRange(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                      const NFmiMetTime &theStartTime,
                                      long minuteRange,
                                      unsigned long &timeIndex1,
                                      unsigned long &timeIndex2);
bool FindMovingSoundingDataTime(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                const NFmiMetTime &theTime,
                                NFmiLocation &theLocation);
MetaWindVectorParamUsage CheckMetaWindVectorParamUsage(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const MetaWindVectorParamUsage &metaWindVectorParamUsage);
float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const MetaWindVectorParamUsage &metaWindVectorParamUsage);
float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const NFmiPoint& theLatlon, const MetaWindVectorParamUsage &metaWindVectorParamUsage);
void CalcWindSpeedAndDirectionFromComponents(const std::deque<float> &u, const std::deque<float> &v, std::deque<float> &wsOut, std::deque<float> &wdOut);
void CalcWindComponentsFromSpeedAndDirection(const std::deque<float> &ws, const std::deque<float> &wd, std::deque<float> &uOut, std::deque<float> &vOut);
void CalcWindComponentsFromSpeedAndDirection(const NFmiDataMatrix<float> &ws, const NFmiDataMatrix<float> &wd, NFmiDataMatrix<float> &uOut, NFmiDataMatrix<float> &vOut);
void CalcWindVectorFromSpeedAndDirection(const std::deque<float> &ws, const std::deque<float> &wd, std::deque<float> &windVectorOut);
void CalcWindVectorFromSpeedAndDirection(const NFmiDataMatrix<float> &ws, const NFmiDataMatrix<float> &wd, NFmiDataMatrix<float> &windVectorOut, unsigned int theStartColumnIndex = 0);
void CalcWindVectorFromWindComponents(const NFmiDataMatrix<float> &u, const NFmiDataMatrix<float> &v, NFmiDataMatrix<float> &windVectorOut, unsigned int theStartColumnIndex = 0);
bool SetInfoToGridPoint(boost::shared_ptr<NFmiFastQueryInfo> &info, unsigned long gridPointX, unsigned long gridPointY);
}
