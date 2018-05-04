#ifndef TEXTGEN_WIND_FORECAST_STRUCTS_H
#define TEXTGEN_WIND_FORECAST_STRUCTS_H

#include "WeatherForecast.h"
#include "WindStoryTools.h"

namespace TextGen
{
using namespace WindStoryTools;

class WindDataItemUnit;
class WindDataItemsByArea;
class WindSpeedPeriodDataItem;
class WindDirectionPeriodDataItem;
class WindEventPeriodDataItem;

typedef std::vector<std::pair<float, WeatherResult> > value_distribution_data_vector;
typedef std::vector<WindDataItemsByArea*> wind_data_item_vector;
typedef std::vector<WindSpeedPeriodDataItem*> wind_speed_period_data_item_vector;
typedef std::vector<WindDirectionPeriodDataItem*> wind_direction_period_data_item_vector;
typedef std::vector<WindEventPeriodDataItem*> wind_event_period_data_item_vector;
typedef std::pair<WindEventId, WeatherPeriod> wind_event_period;
typedef std::vector<wind_event_period> wind_event_period_vector;

struct index_vectors
{
  // contains all indexes to
  std::vector<unsigned int> theOriginalWindDataIndexes;
  std::vector<unsigned int> theEqualizedWindSpeedIndexesForMaxWind;
  std::vector<unsigned int> theEqualizedWindSpeedIndexesForMedianWind;
  std::vector<unsigned int> theEqualizedWindSpeedIndexesForTopWind;
  std::vector<unsigned int> theEqualizedWindSpeedIndexesForCalcWind;
  std::vector<unsigned int> theEqualizedWindDirectionIndexes;
};

struct wo_story_params
{
  wo_story_params(const std::string& var,
                  const WeatherArea& area,
                  const WeatherPeriod& forecastPeriod,
                  const TextGenPosixTime& forecastTime,
                  const AnalysisSources& sources,
                  MessageLogger& log)
      : theVar(var),
        theArea(area),
        theForecastPeriod(forecastPeriod),
        theForecastTime(forecastTime),
        theSources(sources),
        theDataPeriod(forecastPeriod),
        theLog(log),
        theSplitMethod(NO_SPLITTING),
        theWindSpeedMaxError(2.0),
        theWindDirectionMaxError(10.0),
        theWindSpeedThreshold(4.0),
        theWindSpeedWarningThreshold(11.0),
        theWindDirectionThreshold(25.0),
        theWindDirectionMinSpeed(6.5),
        theWindCalcTopShare(80.0),
        theWindCalcTopShareWeak(80.0),
        theWindSpeedTopCoverage(98.0),
        theGustyWindTopWindDifference(5.0),
        theRangeSeparator("-"),
        theMinIntervalSize(2),
        theMaxIntervalSize(5),
        theContextualMaxIntervalSize(5),
        theMetersPerSecondFormat("SI"),
        theAlkaenPhraseUsed(false),
        theWeakTopWind(false)
  {
  }

  const std::string& theVar;
  const WeatherArea& theArea;
  const WeatherPeriod& theForecastPeriod;
  const TextGenPosixTime& theForecastTime;
  const AnalysisSources& theSources;
  // currently the same as forecast period, but could be longer in both ends
  WeatherPeriod theDataPeriod;
  MessageLogger& theLog;
  split_method theSplitMethod;

  double theWindSpeedMaxError;
  double theWindDirectionMaxError;
  double theWindSpeedThreshold;
  double theWindSpeedWarningThreshold;
  double theWindDirectionThreshold;
  double theWindDirectionMinSpeed;
  double theWindCalcTopShare;
  double theWindCalcTopShareWeak;
  double theWindSpeedTopCoverage;
  double theGustyWindTopWindDifference;
  std::string theRangeSeparator;
  int theMinIntervalSize;
  int theMaxIntervalSize;
  int theContextualMaxIntervalSize;
  std::string theMetersPerSecondFormat;
  bool theAlkaenPhraseUsed;
  bool theWeakTopWind;  // if top wind strays under 10 m/s the whole period

  // contains raw data
  wind_data_item_vector theWindDataVector;
  wind_speed_period_data_item_vector theWindSpeedVector;
  wind_direction_period_data_item_vector theWindDirectionVector;
  wind_event_period_data_item_vector theWindSpeedEventPeriodVector;

  std::map<WeatherArea::Type, index_vectors*> indexes;

  inline std::vector<unsigned int>& originalWindDataIndexes(WeatherArea::Type type)
  {
    return indexes[type]->theOriginalWindDataIndexes;
  }
  inline std::vector<unsigned int>& equalizedWSIndexesMaxWind(WeatherArea::Type type)
  {
    return indexes[type]->theEqualizedWindSpeedIndexesForMaxWind;
  }
  inline std::vector<unsigned int>& equalizedWSIndexesMedian(WeatherArea::Type type)
  {
    return indexes[type]->theEqualizedWindSpeedIndexesForMedianWind;
  }
  inline std::vector<unsigned int>& equalizedWSIndexesTopWind(WeatherArea::Type type)
  {
    return indexes[type]->theEqualizedWindSpeedIndexesForTopWind;
  }
  inline std::vector<unsigned int>& equalizedWSIndexesCalcWind(WeatherArea::Type type)
  {
    return indexes[type]->theEqualizedWindSpeedIndexesForCalcWind;
  }
  inline std::vector<unsigned int>& equalizedWDIndexes(WeatherArea::Type type)
  {
    return indexes[type]->theEqualizedWindDirectionIndexes;
  }

  // If the area is split this contains e.g. inland coast, full, eastern, western areas
  std::vector<WeatherArea> theWeatherAreas;

  std::string areaName() { return (theArea.isNamed() ? theArea.name() : ""); }
};

struct WindDataItemUnit
{
  WindDataItemUnit(const WeatherPeriod& period,
                   const WeatherResult& windSpeedMin,
                   const WeatherResult& windSpeedMax,
                   const WeatherResult& windSpeedMean,
                   const WeatherResult& windSpeedMedian,
                   const WeatherResult& windSpeedTop,
                   const WeatherResult& windDirection,
                   const WeatherResult& gustSpeed)
      : thePeriod(period),
        theWindSpeedMin(windSpeedMin),
        theWindSpeedMax(windSpeedMax),
        theWindSpeedMean(windSpeedMean),
        theWindSpeedMedian(windSpeedMedian),
        theWindSpeedTop(windSpeedTop),
        theWindSpeedCalc(WeatherResult(kFloatMissing, kFloatMissing)),
        theWindDirection(windDirection),
        theGustSpeed(gustSpeed),
        theCorrectedWindDirection(windDirection),
        theEqualizedMedianWind(windSpeedMedian),
        theEqualizedMaxWind(windSpeedMax),
        theEqualizedTopWind(windSpeedTop),
        theEqualizedCalcWind(WeatherResult(kFloatMissing, kFloatMissing)),
        theEqualizedWindDirection(theWindDirection)
  {
  }

  float getTopWindSpeedShare(float theLowerLimit, float theUpperLimit) const;
  float getWindSpeedShare(float theLowerLimit, float theUpperLimit) const;
  float getWindDirectionShare(WindStoryTools::WindDirectionId windDirectionId,
                              double theWindDirectionMinSpeed,
                              WindStoryTools::CompassType compass_type =
                                  WindStoryTools::CompassType::sixteen_directions) const;

  bool operator==(const WindDataItemUnit& dataItemUnit) const
  {
    return thePeriod == dataItemUnit.thePeriod;
  }

  WeatherPeriod thePeriod;
  WeatherResult theWindSpeedMin;
  WeatherResult theWindSpeedMax;
  WeatherResult theWindSpeedMean;
  WeatherResult theWindSpeedMedian;
  WeatherResult theWindSpeedTop;
  WeatherResult theWindSpeedCalc;
  WeatherResult theWindDirection;
  WeatherResult theGustSpeed;
  // if wind is varying and wind speed is high >= 7 m/s, we store corrected
  // wind direction here and use it in calculations
  WeatherResult theCorrectedWindDirection;
  WeatherResult theEqualizedMedianWind;
  WeatherResult theEqualizedMaxWind;
  WeatherResult theEqualizedTopWind;
  WeatherResult theEqualizedCalcWind;
  WeatherResult theEqualizedWindDirection;
  value_distribution_data_vector theWindSpeedDistribution;
  value_distribution_data_vector theWindSpeedDistributionTop;
  value_distribution_data_vector theWindDirectionDistribution16;
  value_distribution_data_vector theWindDirectionDistribution8;
};

// contains WindDataItemUnit structs for different areas (coastal, inland, full area)
struct WindDataItemsByArea
{
  WindDataItemsByArea() {}
  ~WindDataItemsByArea()
  {
    std::map<WeatherArea::Type, WindDataItemUnit*>::iterator it;
    for (it = theDataItems.begin(); it != theDataItems.end(); it++)
      delete it->second;
  }

  void addItem(const WeatherPeriod& period,
               const WeatherResult& windSpeedMin,
               const WeatherResult& windSpeedMax,
               const WeatherResult& windSpeedMean,
               const WeatherResult& windSpeedMedian,
               const WeatherResult& windSpeedTop,
               const WeatherResult& windDirection,
               const WeatherResult& gustSpeed,
               const WeatherArea::Type& type)
  {
    WindDataItemUnit* dataItem = new WindDataItemUnit(period,
                                                      windSpeedMin,
                                                      windSpeedMax,
                                                      windSpeedMean,
                                                      windSpeedMedian,
                                                      windSpeedTop,
                                                      windDirection,
                                                      gustSpeed);
    theDataItems.insert(std::make_pair(type, dataItem));
  }

  const WindDataItemUnit& operator()(const WeatherArea::Type& type = WeatherArea::Full) const
  {
    return *(theDataItems.find(type)->second);
  }

  WindDataItemUnit& getDataItem(const WeatherArea::Type& type = WeatherArea::Full) const
  {
    return *(theDataItems.find(type)->second);
  }

 private:
  std::map<WeatherArea::Type, WindDataItemUnit*> theDataItems;
};

struct WindSpeedPeriodDataItem
{
  WindSpeedPeriodDataItem(const WeatherPeriod& period,
                          const WindStoryTools::WindSpeedId& windSpeedId)
      : thePeriod(period), theWindSpeedId(windSpeedId)
  {
  }
  WeatherPeriod thePeriod;
  WindStoryTools::WindSpeedId theWindSpeedId;
};

struct WindDirectionPeriodDataItem
{
  WindDirectionPeriodDataItem(const WeatherPeriod& period,
                              WindStoryTools::WindDirectionId windDirection)
      : thePeriod(period), theWindDirection(windDirection)
  {
  }
  WeatherPeriod thePeriod;
  WindStoryTools::WindDirectionId theWindDirection;
};

struct WindEventPeriodDataItem
{
  WindEventPeriodDataItem(const WeatherPeriod& period,
                          WindEventId windEvent,
                          const WindDataItemUnit& periodBeginDataItem,
                          const WindDataItemUnit& periodEndDataItem)
      : thePeriod(period),
        theWindSpeedChangePeriod(period),
        theWindEvent(windEvent),
        thePeriodBeginDataItem(periodBeginDataItem),
        thePeriodEndDataItem(periodEndDataItem),
        theSuccessiveEventFlag(false)
  {
  }

  WeatherPeriod thePeriod;
  WeatherPeriod theWindSpeedChangePeriod;  // can be shorter than thePeriod
  WindEventId theWindEvent;
  const WindDataItemUnit& thePeriodBeginDataItem;
  const WindDataItemUnit& thePeriodEndDataItem;
  bool theSuccessiveEventFlag;  // if there is a long MISSING_WIND_EVENT period between two
  // strenghtening/weakening period, theSuccessiveEventFlag is set true to the latter
  // strenghtening/weakening period, so that we can use phrase 'voimistuu/heikkenee edelleen'
};

struct WindDirectionInfo
{
  WeatherResult direction;
  WindDirectionId id;

  WindDirectionInfo()
      : direction(WeatherResult(kFloatMissing, kFloatMissing)), id(MISSING_WIND_DIRECTION_ID)
  {
  }
  WindDirectionInfo(const WeatherResult& d, WindDirectionId i) : direction(d), id(i) {}
};

struct TimePhraseInfo
{
  TextGenPosixTime starttime;
  TextGenPosixTime endtime;
  short day_number;
  part_of_the_day_id part_of_the_day;

  TimePhraseInfo() : day_number(-1), part_of_the_day(MISSING_PART_OF_THE_DAY_ID) {}
  TimePhraseInfo(const TextGenPosixTime& st,
                 const TextGenPosixTime& et,
                 short d,
                 part_of_the_day_id pd)
      : starttime(st), endtime(et), day_number(d), part_of_the_day(pd)
  {
  }
};

struct DirectionSentenceInfo
{
  TextGenPosixTime startTime;
  TextGenPosixTime endTime;
  Sentence sentence;
  TimePhraseInfo timePhraseInfo;
  WindDirectionInfo windDirectionInfo;
};
/*
// in wind_overview.cpp
WeatherResult mean_wind_direction(const AnalysisSources& theSources,
                                const WeatherArea& theArea,
                                const WeatherPeriod& thePeriod,
                                const WeatherResult& theEqualizedWindSpeedMedian,
                                const WeatherResult& theEqualizedWindSpeedTop,
                                const std::string& theVar);
// in wind_overview.cpp
float mean_wind_direction_error(const wind_data_item_vector& theWindDataVector,
                              const WeatherArea& theArea,
                              const WeatherPeriod& thePeriod);
*/

// in WindForecast.cpp
std::string get_wind_event_string(WindEventId theWindEventId);
bool wind_speed_differ_enough(const wo_story_params& theParameter, const WeatherPeriod& thePeriod);
bool wind_direction_differ_enough(const WeatherResult theWindDirection1,
                                  const WeatherResult theWindDirection2,
                                  float theWindDirectionThreshold);

bool is_weak_period(const wo_story_params& theParameters, const WeatherPeriod& thePeriod);

unsigned int get_peak_wind(const WeatherPeriod& thePeriod, const wo_story_params& theParameters);
float get_top_wind(const WeatherPeriod& thePeriod, const wo_story_params& theParameters);

std::ostream& operator<<(std::ostream& theOutput, const WeatherPeriod& period);
std::ostream& operator<<(std::ostream& theOutput,
                         const WindEventPeriodDataItem& theWindEventPeriodDataItem);

}  // namespace TextGen

#endif  // TEXTGEN_WIND_FORECAST_STRUCTS_H
