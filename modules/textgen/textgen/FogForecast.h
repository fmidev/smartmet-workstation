#ifndef TEXTGEN_FOG_FORECAST_H
#define TEXTGEN_FOG_FORECAST_H

#include "WeatherForecast.h"
#include "PrecipitationForecast.h"

namespace TextGen
{
enum fog_type_id
{
  FOG,
  FOG_POSSIBLY_DENSE,
  FOG_IN_SOME_PLACES,
  FOG_IN_SOME_PLACES_POSSIBLY_DENSE,
  FOG_IN_MANY_PLACES,
  FOG_IN_MANY_PLACES_POSSIBLY_DENSE,
  NO_FOG
};

typedef std::pair<WeatherPeriod, FogIntensityDataItem> weather_period_fog_intensity_pair;
typedef std::pair<WeatherPeriod, fog_type_id> weather_period_fog_type_intensity_pair;
typedef std::vector<weather_period_fog_intensity_pair> fog_period_vector;
typedef std::vector<weather_period_fog_type_intensity_pair> fog_type_period_vector;

class FogForecast
{
 public:
  FogForecast(wf_story_params& parameters);

  ~FogForecast() {}
  Sentence fogSentence(const WeatherPeriod& thePeriod) const;

  void printOutFogPeriods(std::ostream& theOutput) const;
  void printOutFogTypePeriods(std::ostream& theOutput) const;
  void printOutFogData(std::ostream& theOutput) const;

 private:
  void findOutFogTypePeriods(const fog_period_vector& theFogPeriods,
                             fog_type_period_vector& theFogTypePeriods);
  void findOutFogPeriods(const weather_result_data_item_vector* theModerateFogData,
                         const weather_result_data_item_vector* theDenseFogData,
                         fog_period_vector& theFogPeriods);
  void printOutFogPeriods(std::ostream& theOutput, const fog_period_vector& theFogPeriods) const;
  void printOutFogTypePeriods(std::ostream& theOutput,
                              const fog_type_period_vector& theFogTypePeriods) const;
  void printOutFogData(std::ostream& theOutput,
                       const std::string& theLinePrefix,
                       const weather_result_data_item_vector& theFogData) const;
  Sentence getFogPhrase(const fog_type_id& theFogTypeId);
  void findOutFogPeriods();
  void findOutFogTypePeriods();
  Sentence fogSentence(const WeatherPeriod& thePeriod,
                       const fog_type_period_vector& theFogTypePeriods,
                       const std::string& theAreaString) const;

  Sentence getFogPhrase(const fog_type_id& theFogTypeId) const;
  Sentence areaSpecificSentence(const WeatherPeriod& thePeriod) const;
  float getMean(const fog_period_vector& theFogPeriods,
                const WeatherPeriod& theWeatherPeriod) const;
  WeatherPeriod getActualFogPeriod(const WeatherPeriod& theForecastPeriod,
                                   const WeatherPeriod& theFogPeriod,
                                   bool& theFogPeriodOkFlag) const;
  bool getFogPeriodAndId(const WeatherPeriod& theForecastPeriod,
                         const fog_type_period_vector& theFogTypePeriods,
                         WeatherPeriod& theResultPeriod,
                         fog_type_id& theFogTypeId) const;
  Sentence constructFogSentence(const std::string& theDayPhasePhrase,
                                const std::string& theAreaString,
                                const std::string& theInPlacesString,
                                const bool& thePossiblyDenseFlag) const;

  wf_story_params& theParameters;

  const weather_result_data_item_vector* theCoastalModerateFogData;
  const weather_result_data_item_vector* theInlandModerateFogData;
  const weather_result_data_item_vector* theFullAreaModerateFogData;
  const weather_result_data_item_vector* theCoastalDenseFogData;
  const weather_result_data_item_vector* theInlandDenseFogData;
  const weather_result_data_item_vector* theFullAreaDenseFogData;

  fog_period_vector theCoastalFog;
  fog_period_vector theInlandFog;
  fog_period_vector theFullAreaFog;

  fog_type_period_vector theCoastalFogType;
  fog_type_period_vector theInlandFogType;
  fog_type_period_vector theFullAreaFogType;
};
}  // namespace TextGen

#endif  // TEXTGEN_FOG_FORECAST_H
