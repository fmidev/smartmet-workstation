#ifndef TEXTGEN_THUNDER_FORECAST_H
#define TEXTGEN_THUNDER_FORECAST_H

#include "WeatherForecast.h"
#include "PrecipitationForecast.h"

namespace TextGen
{
class ThunderForecast
{
 public:
  ThunderForecast(wf_story_params& parameters);

  Sentence thunderSentence(const WeatherPeriod& thePeriod,
                           const AreaTools::forecast_area_id& theForecastAreaId,
                           const std::string& theVariable) const;

 private:
  Sentence areaSpecificSentence(const WeatherPeriod& thePeriod) const;
  float getMaxValue(const WeatherPeriod& theWeatherPeriod,
                    const weather_result_data_item_vector& theDataVector) const;

  wf_story_params& theParameters;
};
}  // namespace TextGen

#endif  // TEXTGEN_THUNDER_FORECAST_H
