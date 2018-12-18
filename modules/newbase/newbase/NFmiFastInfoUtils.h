#pragma once

#include "NFmiMetTime.h"
#include "NFmiDataMatrix.h"
#include <boost/shared_ptr.hpp>

class NFmiFastQueryInfo;
class NFmiLevel;
class NFmiPoint;

namespace NFmiFastInfoUtils
{
    // Apu luokka tekem‰‰n ik‰v‰n fastInfon parametrin valinta tilan palautuksen (indeksi + aliparametri juttu).
    // Tila otetaan konstruktorissa ja palautetaan destruktorissa.
    class FastInfoParamStateRestorer
    {
        NFmiFastQueryInfo &info_;
        unsigned long paramIndex_;
        bool subParamUsed_;
    public:
        FastInfoParamStateRestorer(NFmiFastQueryInfo &info);
        ~FastInfoParamStateRestorer();
    };

    class MetaWindParamUsage
    {
        // If Totalwind combine parameter is present, no need for any meta parameters...
        bool fHasTotalWind = false;
        // Data has kFmiWindVectorMS parameter, so no need to make meta parameter of it.
        bool fHasWindVectorParam = false;
        // Data has wind speed and wind direction (that can be used to calculate wind vector)
        bool fHasWsAndWd = false;
        // data has u- and v-components of the wind (that can be used to calculate wind vector)
        bool fHasWindComponents = false;
    public:
        bool NoWindMetaParamsNeeded() const;
        bool MakeMetaWindVectorParam() const;
        bool MakeMetaWsAndWdParams() const;
        bool MakeMetaWindComponents() const;
        bool HasWsAndWd() const { return fHasWsAndWd; }
        bool HasWindComponents() const { return fHasWindComponents; }


        friend MetaWindParamUsage CheckMetaWindParamUsage(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
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
MetaWindParamUsage CheckMetaWindParamUsage(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const MetaWindParamUsage &metaWindParamUsage);
float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const MetaWindParamUsage &metaWindParamUsage);
float GetMetaWindVectorValue(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const NFmiPoint& theLatlon, const MetaWindParamUsage &metaWindParamUsage);
std::pair<float, float> GetMetaWsWdValues(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const MetaWindParamUsage &metaWindParamUsage);
std::pair<float, float> GetMetaWsWdValues(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const MetaWindParamUsage &metaWindParamUsage);
std::pair<float, float> GetMetaWsWdValues(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const NFmiPoint& theLatlon, const MetaWindParamUsage &metaWindParamUsage);
std::pair<float, float> GetMetaWindComponentsValues(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const MetaWindParamUsage &metaWindParamUsage);
std::pair<float, float> GetMetaWindComponentsValues(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const MetaWindParamUsage &metaWindParamUsage);
std::pair<float, float> GetMetaWindComponentsValues(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime, const NFmiPoint& theLatlon, const MetaWindParamUsage &metaWindParamUsage);
void CalcWindSpeedAndDirectionFromComponents(const std::deque<float> &u, const std::deque<float> &v, std::deque<float> &wsOut, std::deque<float> &wdOut);
void CalcWindComponentsFromSpeedAndDirection(const std::deque<float> &ws, const std::deque<float> &wd, std::deque<float> &uOut, std::deque<float> &vOut);
void CalcWindComponentsFromSpeedAndDirection(const NFmiDataMatrix<float> &ws, const NFmiDataMatrix<float> &wd, NFmiDataMatrix<float> &uOut, NFmiDataMatrix<float> &vOut);
void CalcWindVectorFromSpeedAndDirection(const std::deque<float> &ws, const std::deque<float> &wd, std::deque<float> &windVectorOut);
void CalcWindVectorFromSpeedAndDirection(const NFmiDataMatrix<float> &ws, const NFmiDataMatrix<float> &wd, NFmiDataMatrix<float> &windVectorOut, unsigned int theStartColumnIndex = 0);
void CalcWindVectorFromWindComponents(const NFmiDataMatrix<float> &u, const NFmiDataMatrix<float> &v, NFmiDataMatrix<float> &windVectorOut, unsigned int theStartColumnIndex = 0);
void CalcWsFromWindComponents(const NFmiDataMatrix<float> &u, const NFmiDataMatrix<float> &v, NFmiDataMatrix<float> &wsOut, unsigned int theStartColumnIndex = 0);
void CalcWdFromWindComponents(const NFmiDataMatrix<float> &u, const NFmiDataMatrix<float> &v, NFmiDataMatrix<float> &wdOut, unsigned int theStartColumnIndex = 0);
void CalcUcomponentFromSpeedAndDirection(const NFmiDataMatrix<float> &ws, const NFmiDataMatrix<float> &wd, NFmiDataMatrix<float> &uOut, unsigned int theStartColumnIndex = 0);
void CalcVcomponentFromSpeedAndDirection(const NFmiDataMatrix<float> &ws, const NFmiDataMatrix<float> &wd, NFmiDataMatrix<float> &vOut, unsigned int theStartColumnIndex = 0);
bool SetInfoToGridPoint(boost::shared_ptr<NFmiFastQueryInfo> &info, unsigned long gridPointX, unsigned long gridPointY);
}
