#ifndef TEXTGEN_CLOUDINESS_FORECAST_H
#define TEXTGEN_CLOUDINESS_FORECAST_H

#include "WeatherForecast.h"

namespace TextGen
{
typedef std::pair<WeatherPeriod, cloudiness_id> weather_period_cloudiness_id_pair;
typedef std::vector<weather_period_cloudiness_id_pair> cloudiness_period_vector;

class CloudinessForecast
{
 public:
  CloudinessForecast(wf_story_params& parameters);

  ~CloudinessForecast() {}
  void setCoastalData(const weather_result_data_item_vector* coastalData)
  {
    theCoastalData = coastalData;
    findOutCloudinessPeriods(theCoastalData, theCloudinessPeriodsCoastal);
    joinPeriods(theCoastalData, theCloudinessPeriodsCoastal, theCloudinessPeriodsCoastalJoined);
    findOutCloudinessWeatherEvents(theCoastalData, theCloudinessWeatherEventsCoastal);
  }
  void setInlandData(const weather_result_data_item_vector* inlandData)
  {
    theInlandData = inlandData;
    findOutCloudinessPeriods(theInlandData, theCloudinessPeriodsInland);
    joinPeriods(theInlandData, theCloudinessPeriodsInland, theCloudinessPeriodsInlandJoined);
    findOutCloudinessWeatherEvents(theInlandData, theCloudinessWeatherEventsInland);
  }
  void setFullData(const weather_result_data_item_vector* fullData)
  {
    theFullData = fullData;
    findOutCloudinessPeriods(theFullData, theCloudinessPeriodsFull);
    joinPeriods(theFullData, theCloudinessPeriodsFull, theCloudinessPeriodsFullJoined);
    findOutCloudinessWeatherEvents(theFullData, theCloudinessWeatherEventsFull);
  }

  const weather_result_data_item_vector* getCoastalData() const { return theCoastalData; }
  const weather_result_data_item_vector* getInlandData() const { return theInlandData; }
  const weather_result_data_item_vector* getFullData() const { return theFullData; }
  Sentence cloudinessSentence(const WeatherPeriod& thePeriod, const bool& theShortForm) const;
  Sentence cloudinessSentence(const WeatherPeriod& thePeriod,
                              const bool& thePoutainenFlag,
                              const Sentence& thePeriodPhrase,
                              const bool& theShortForm) const;
  /*
  Sentence cloudinessSentence(const WeatherPeriod& thePeriod,
                                                          const weather_result_data_item_vector&
  theCloudinessData) const;
  */
  Sentence cloudinessChangeSentence(const WeatherPeriod& thePeriod) const;

  /*
  Sentence cloudinessSentence(const unsigned int& thePeriodNumber,
                                                          const bool& theCheckCloudinessChange =
  true);
  */
  cloudiness_id getCloudinessId(const WeatherPeriod& thePeriod) const;
  cloudiness_id getCloudinessId(const WeatherPeriod& thePeriod,
                                const weather_result_data_item_vector* theCloudinessData) const;

  void getWeatherEventIdVector(weather_event_id_vector& theCloudinessWeatherEvents) const;

  void printOutCloudinessData(std::ostream& theOutput) const;
  void printOutCloudinessWeatherEvents(std::ostream& theOutput) const;
  void printOutCloudinessPeriods(std::ostream& theOutput) const;

 private:
  float getMeanCloudiness(const WeatherPeriod& theWeatherPeriod,
                          const weather_result_data_item_vector& theDataVector) const;
  bool separateCoastInlandCloudiness(const WeatherPeriod& theWeatherPeriod) const;
  // e.g. separate morning afternoon
  bool separateWeatherPeriodCloudiness(
      const WeatherPeriod& theWeatherPeriod1,
      const WeatherPeriod& theWeatherPeriod2,
      const weather_result_data_item_vector& theCloudinessData) const;

  void printOutCloudinessData(std::ostream& theOutput,
                              const weather_result_data_item_vector* theDataVector) const;
  void getWeatherPeriodCloudiness(const WeatherPeriod& thePeriod,
                                  const cloudiness_period_vector& theSourceCloudinessPeriods,
                                  cloudiness_period_vector& theWeatherPeriodCloudiness) const;

  void printOutCloudinessPeriods(std::ostream& theOutput,
                                 const cloudiness_period_vector& theCloudinessPeriods) const;

  void findOutCloudinessPeriods(const weather_result_data_item_vector* theData,
                                cloudiness_period_vector& theCloudinessPeriods);
  void findOutCloudinessPeriods();
  void findOutCloudinessWeatherEvents(const weather_result_data_item_vector* theData,
                                      weather_event_id_vector& theCloudinessWeatherEvents);
  void findOutCloudinessWeatherEvents();

  void joinPeriods(const weather_result_data_item_vector* theDataSource,
                   const cloudiness_period_vector& theCloudinessPeriodsSource,
                   cloudiness_period_vector& theCloudinessPeriodsDestination);
  void joinPeriods();
  cloudiness_id getCloudinessPeriodId(
      const TextGenPosixTime& theObservationTime,
      const cloudiness_period_vector& theCloudinessPeriodVector) const;
  Sentence areaSpecificSentence(const WeatherPeriod& thePeriod) const;
  cloudiness_id getCloudinessId(const float& theMin,
                                const float& theMean,
                                const float& theMax,
                                const float& theStandardDeviation) const;
  cloudiness_id getCloudinessId(const float& theCloudiness) const;
  void joinPuolipilvisestaPilviseen(const weather_result_data_item_vector* theData,
                                    std::vector<int>& theCloudinessPuolipilvisestaPilviseen) const;

  wf_story_params& theParameters;
  const weather_result_data_item_vector* theCoastalData;
  const weather_result_data_item_vector* theInlandData;
  const weather_result_data_item_vector* theFullData;

  cloudiness_period_vector theCloudinessPeriodsCoastal;
  cloudiness_period_vector theCloudinessPeriodsInland;
  cloudiness_period_vector theCloudinessPeriodsFull;

  cloudiness_period_vector theCloudinessPeriodsCoastalJoined;
  cloudiness_period_vector theCloudinessPeriodsInlandJoined;
  cloudiness_period_vector theCloudinessPeriodsFullJoined;

  weather_event_id_vector theCloudinessWeatherEventsCoastal;
  weather_event_id_vector theCloudinessWeatherEventsInland;
  weather_event_id_vector theCloudinessWeatherEventsFull;

  bool theSeparateCoastInlandMorning;
  bool theSeparateCoastInlandAfternoon;
  bool theSeparateMorningAfternoon;  // full area
};

const char* cloudiness_string(const cloudiness_id& theCloudinessId);
bool puolipilvisesta_pilviseen(const cloudiness_id& theCloudinessId1,
                               const cloudiness_id& theCloudinessId2);
Sentence cloudiness_sentence(const cloudiness_id& theCloudinessId);
//							   const bool& theShortForm = false);

std::ostream& operator<<(std::ostream& theOutput,
                         const CloudinessDataItemData& theCloudinessDataItemData);
std::ostream& operator<<(std::ostream& theOutput, const CloudinessDataItem& theCloudinessDataItem);
}  // namespace TextGen

#endif  // TEXTGEN_CLOUDINESS_FORECAST_H
