#ifndef WEATHER_FORECAST_STORY_H
#define WEATHER_FORECAST_STORY_H

//#include <vector>

class MessageLogger;

namespace TextGen
{
class Sentence;
class Paragraph;
class WeatherForecastStory;

class WeatherForecastStoryItem
{
 public:
  WeatherForecastStoryItem(WeatherForecastStory& weatherForecastStory,
                           const WeatherPeriod& period,
                           const story_part_id& storyPartId);

  virtual ~WeatherForecastStoryItem() {}
  Sentence getSentence();
  // if periods are merged this must be overwritten
  virtual WeatherPeriod getStoryItemPeriod() const;
  virtual Sentence getStoryItemSentence() { return Sentence(); }
  unsigned int getPeriodLength();
  int forecastPeriodLength() const;
  int storyItemPeriodLength() const;
  Sentence getTodayVectorSentence(const std::vector<Sentence*>& todayVector,
                                  const unsigned int& theBegIndex,
                                  const unsigned int& theEndIndex);
  Sentence getPeriodPhrase(const bool& theFromSpecifier,
                           const WeatherPeriod* thePhrasePeriod = 0,
                           const bool& theStoryUnderConstructionEmpty = true);
  std::string checkForAamuyoAndAamuPhrase(const bool& theFromSpecifier,
                                          const WeatherPeriod& thePhrasePeriod);
  Sentence checkForExtendedPeriodPhrase(const WeatherPeriod& thePhrasePeriod);

  const story_part_id& getStoryPartId() const { return theStoryPartId; }
  const WeatherPeriod& getPeriod() const { return thePeriod; }
  bool isIncluded() const { return theIncludeInTheStoryFlag; }
  unsigned int numberOfAdditionalSentences() { return theAdditionalSentences.size(); }
  Sentence getAdditionalSentence(const unsigned int& index) const;

 protected:
  std::vector<Sentence> theAdditionalSentences;
  WeatherForecastStory& theWeatherForecastStory;
  WeatherPeriod thePeriod;
  story_part_id theStoryPartId;
  // can be used to control if this item is included in the story
  bool theIncludeInTheStoryFlag;
  Sentence theSentence;
  WeatherForecastStoryItem*
      thePeriodToMergeWith;  // if periods are merged this points to the megreable period
  WeatherForecastStoryItem*
      thePeriodToMergeTo;  // if periods are merged this points to the merged period

  friend class WeatherForecastStory;

 private:
};

class PrecipitationForecastStoryItem : public WeatherForecastStoryItem
{
 public:
  PrecipitationForecastStoryItem(WeatherForecastStory& weatherForecastStory,
                                 const WeatherPeriod& period,
                                 const story_part_id& storyPartId,
                                 const float& intensity,
                                 const float& extent,
                                 const unsigned int& form,
                                 const precipitation_type& type,
                                 const bool& thunder);

  bool weakPrecipitation() const;
  Sentence getStoryItemSentence();

 private:
  float theIntensity;
  float theExtent;
  unsigned int theForm;
  precipitation_type theType;
  // this flag indicates wheather thunder exists or not during the period
  bool theThunder;
  // this flag indicates if there has been a gap between precipitation periods
  bool theSadeJatkuuFlag;
  // this flag indicates whether we tell about
  // ending precipitation period (poutaantuu)
  bool thePoutaantuuFlag;
  // if precipitation has been weak we dont report ending of it
  bool theReportPoutaantuuFlag;
  int theFullDuration;  // includes precipitation period beyond the forecast period

  friend class WeatherForecastStory;
  friend class CloudinessForecastStoryItem;
};

class CloudinessForecastStoryItem : public WeatherForecastStoryItem
{
 public:
  CloudinessForecastStoryItem(WeatherForecastStory& weatherForecastStory,
                              const WeatherPeriod& period,
                              const story_part_id& storyPartId,
                              const cloudiness_id& cloudinessId,
                              PrecipitationForecastStoryItem* previousPrecipitationStoryItem,
                              PrecipitationForecastStoryItem* nextPrecipitationStoryItem);

  Sentence getStoryItemSentence();
  Sentence cloudinessChangeSentence();

 private:
  cloudiness_id theCloudinessId;
  PrecipitationForecastStoryItem* thePreviousPrecipitationStoryItem;
  PrecipitationForecastStoryItem* theNextPrecipitationStoryItem;
  bool theReportAboutDryWeatherFlag;
  Sentence thePoutaantuuSentence;
  Sentence theShortFormSentence;
  Sentence theChangeSentence;
  TextGenPosixTime theCloudinessChangeTimestamp;

  friend class WeatherForecastStory;
};

class WeatherForecastStory
{
 public:
  WeatherForecastStory(const std::string& var,
                       const TextGen::WeatherPeriod& forecastPeriod,
                       const TextGen::WeatherArea& weatherArea,
                       const unsigned short& forecastArea,
                       const TextGenPosixTime& theForecastTime,
                       PrecipitationForecast& precipitationForecast,
                       const CloudinessForecast& cloudinessForecast,
                       const FogForecast& fogForecast,
                       const ThunderForecast& thunderForecast,
                       MessageLogger& logger);

  ~WeatherForecastStory();

  Paragraph getWeatherForecastStory();
  const WeatherPeriod& getStoryPeriod() const { return theForecastPeriod; }
  const std::vector<WeatherForecastStoryItem*>& getStoryItemVector() const
  {
    return theStoryItemVector;
  }
  void logTheStoryItems() const;

 private:
  void addPrecipitationStoryItems();
  void addCloudinessStoryItems();
  void mergePeriodsWhenFeasible();
  Sentence getTimePhrase();
  void mergePrecipitationPeriodsWhenFeasible();
  void mergeCloudinessPeriodsWhenFeasible();

  const std::string theVar;
  const WeatherPeriod& theForecastPeriod;
  const WeatherArea& theWeatherArea;
  const unsigned short& theForecastArea;
  const TextGenPosixTime& theForecastTime;
  const PrecipitationForecast& thePrecipitationForecast;
  const CloudinessForecast& theCloudinessForecast;
  const FogForecast& theFogForecast;
  const ThunderForecast& theThunderForecast;
  MessageLogger& theLogger;
  int theStorySize;
  bool theShortTimePrecipitationReportedFlag;
  bool theReportTimePhraseFlag;
  bool theCloudinessReportedFlag;

  std::vector<WeatherForecastStoryItem*> theStoryItemVector;

  friend class PrecipitationForecastStoryItem;
  friend class CloudinessForecastStoryItem;
  friend class WeatherForecastStoryItem;
};
}

#endif
