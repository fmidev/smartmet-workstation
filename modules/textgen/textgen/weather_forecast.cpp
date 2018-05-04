// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::WeatherStory::overview
 */
// ======================================================================

#include "WeatherStory.h"
#include "CloudinessStory.h"
#include "CloudinessStoryTools.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "NightAndDayPeriodGenerator.h"
#include "PeriodPhraseFactory.h"
#include "PrecipitationPeriodTools.h"
#include "PrecipitationStoryTools.h"
#include <calculator/RangeAcceptor.h>
#include "ValueAcceptor.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include <calculator/WeatherResult.h>
#include "WeekdayTools.h"
#include <calculator/NullPeriodGenerator.h>
#include <calculator/WeatherPeriodTools.h>
#include "AreaTools.h"
#include <calculator/MathTools.h>
#include "SeasonTools.h"
#include "SubMaskExtractor.h"
#include "WeatherForecast.h"
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "PlainTextFormatter.h"
#include "PrecipitationForecast.h"
#include "CloudinessForecast.h"
#include "FogForecast.h"
#include "ThunderForecast.h"
#include <calculator/WeatherHistory.h>
#include "WeatherForecastStory.h"
#include <calculator/ParameterAnalyzer.h>
#include "DebugTextFormatter.h"
#include "Phrase.h"

#include <newbase/NFmiCombinedParam.h>

#include <typeinfo>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <map>

namespace TextGen
{
using namespace Settings;
using namespace TextGen;
using namespace AreaTools;
using namespace boost;
using namespace std;

void print_out_weather_event_vector(std::ostream& theOutput,
                                    const weather_event_id_vector& theWeatherEventVector)
{
  for (unsigned int i = 0; i < theWeatherEventVector.size(); i++)
  {
    weather_event_id trid(theWeatherEventVector.at(i).second);

    theOutput << theWeatherEventVector.at(i).first << ": " << weather_event_string(trid) << endl;
  }
}

std::ostream& operator<<(std::ostream& theOutput,
                         const WeatherResultDataItem& theWeatherResultDataItem)
{
  const WeatherResult theResult(theWeatherResultDataItem.theResult);

  theOutput << theWeatherResultDataItem.thePeriod.localStartTime() << " ... "
            << theWeatherResultDataItem.thePeriod.localEndTime() << ": " << '('
            << theWeatherResultDataItem.theResult.value() << ','
            << theWeatherResultDataItem.theResult.error() << ')' << endl;

  return theOutput;
}

bool same_content(const Sentence& sentence1, const Sentence& sentence2)
{
  boost::shared_ptr<TextGen::Dictionary> dict;
  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  TextGen::PlainTextFormatter formatter;
  formatter.dictionary(dict);
  std::string string1(sentence1.realize(formatter));
  std::string string2(sentence2.realize(formatter));

  return (string1.compare(string2) == 0);
}

void get_part_of_the_day_time_series(wf_story_params& theParameters,
                                     const WeatherPeriod& theWeatherPeriod,
                                     const part_of_the_day_id& thePartOfTheDayId,
                                     const weather_result_data_id& theDataId,
                                     weather_result_data_item_vector& theResultVector)
{
}

void log_start_time_and_end_time(MessageLogger& theLog,
                                 const std::string& theLogMessage,
                                 const WeatherPeriod& thePeriod)
{
  theLog << NFmiStringTools::Convert(theLogMessage) << thePeriod.localStartTime() << " ... "
         << thePeriod.localEndTime() << endl;
}

void log_weather_result_time_series(MessageLogger& theLog,
                                    const std::string& theLogMessage,
                                    const weather_result_data_item_vector& theTimeSeries)
{
  theLog << NFmiStringTools::Convert(theLogMessage) << endl;

  for (unsigned int i = 0; i < theTimeSeries.size(); i++)
  {
    //		const WeatherResultDataItem& theWeatherResultDataItem = *theTimeSeries[i];
    // log_weather_result_data_item(theLog, *theTimeSeries[i]);
    theLog << *theTimeSeries[i];
  }
}

void log_weather_result_data(MessageLogger& theLog,
                             weather_forecast_result_container& theDataContainer)
{
  std::string theLogMessage;
  weather_result_data_item_vector* timeSeries = 0;
  for (unsigned int i = CLOUDINESS_DATA; i < UNDEFINED_DATA_ID; i++)
  {
    timeSeries = theDataContainer[i];
    if (i == PRECIPITATION_MAX_DATA)
      theLogMessage = "*** precipitation maximum ****";
    else if (i == PRECIPITATION_MEAN_DATA)
      theLogMessage = "*** precipitation mean ****";
    else if (i == PRECIPITATION_EXTENT_DATA)
      theLogMessage = "*** precipitation extent ****";
    else if (i == PRECIPITATION_TYPE_DATA)
      theLogMessage = "*** precipitation type ****";
    else if (i == CLOUDINESS_DATA)
      theLogMessage = "*** cloudiness ****";
    else if (i == PRECIPITATION_FORM_WATER_DATA)
      theLogMessage = "*** precipitation form water ****";
    else if (i == PRECIPITATION_FORM_DRIZZLE_DATA)
      theLogMessage = "*** precipitation form drizzle ****";
    else if (i == PRECIPITATION_FORM_SLEET_DATA)
      theLogMessage = "*** precipitation form sleet ****";
    else if (i == PRECIPITATION_FORM_SNOW_DATA)
      theLogMessage = "*** precipitation form snow ****";
    else if (i == PRECIPITATION_FORM_FREEZING_DATA)
      theLogMessage = "*** precipitation form freezing ****";
    else if (i == THUNDER_PROBABILITY_DATA)
      theLogMessage = "*** thunder probability ****";
    else if (i == THUNDER_EXTENT_DATA)
      theLogMessage = "*** thunder extent ****";
    else if (i == FOG_INTENSITY_MODERATE_DATA)
      theLogMessage = "*** fog intensity moderate ****";
    else if (i == FOG_INTENSITY_DENSE_DATA)
      theLogMessage = "*** fog intensity dense ****";
    else if (i == PRECIPITATION_NORTHEAST_SHARE_DATA)
      theLogMessage = "*** precipitation share northeast ****";
    else if (i == PRECIPITATION_SOUTHEAST_SHARE_DATA)
      theLogMessage = "*** precipitation share southeast ****";
    else if (i == PRECIPITATION_SOUTHWEST_SHARE_DATA)
      theLogMessage = "*** precipitation share southwest ****";
    else if (i == PRECIPITATION_NORTHWEST_SHARE_DATA)
      theLogMessage = "*** precipitation share northwest ****";
    else if (i == PRECIPITATION_POINT_DATA)
      theLogMessage = "*** precipitation point ****";
    else if (i == CLOUDINESS_NORTHEAST_SHARE_DATA)
      theLogMessage = "*** cloudiness share northeast ****";
    else if (i == CLOUDINESS_SOUTHEAST_SHARE_DATA)
      theLogMessage = "*** cloudiness share southeast ****";
    else if (i == CLOUDINESS_SOUTHWEST_SHARE_DATA)
      theLogMessage = "*** cloudiness share southwest ****";
    else if (i == CLOUDINESS_NORTHWEST_SHARE_DATA)
      theLogMessage = "*** cloudiness share northwest ****";
    else if (i == THUNDER_NORTHEAST_SHARE_DATA)
      theLogMessage = "*** thunder share northeast ****";
    else if (i == THUNDER_SOUTHEAST_SHARE_DATA)
      theLogMessage = "*** thunder share southeast ****";
    else if (i == THUNDER_SOUTHWEST_SHARE_DATA)
      theLogMessage = "*** thunder share southwest ****";
    else if (i == THUNDER_NORTHWEST_SHARE_DATA)
      theLogMessage = "*** thunder share northwest ****";
    else if (i == FOG_NORTHEAST_SHARE_DATA)
      theLogMessage = "*** fog share northeast ****";
    else if (i == FOG_SOUTHEAST_SHARE_DATA)
      theLogMessage = "*** fog share southeast ****";
    else if (i == FOG_SOUTHWEST_SHARE_DATA)
      theLogMessage = "*** fog share southwest ****";
    else if (i == FOG_NORTHWEST_SHARE_DATA)
      theLogMessage = "*** fog share northwest ****";
    else
      theLogMessage = "*** unknown data ****";

    log_weather_result_time_series(theLog, theLogMessage, *timeSeries);
  }
}

void log_weather_result_data(wf_story_params& theParameters)
{
  if (theParameters.theForecastArea & INLAND_AREA)
  {
    theParameters.theLog << "**  INLAND AREA DATA   **" << endl;
    log_weather_result_data(theParameters.theLog, *theParameters.theCompleteData[INLAND_AREA]);
  }
  if (theParameters.theForecastArea & COASTAL_AREA)
  {
    theParameters.theLog << "**  COASTAL AREA DATA   **" << endl;
    log_weather_result_data(theParameters.theLog, *theParameters.theCompleteData[COASTAL_AREA]);
  }
  if (theParameters.theForecastArea & FULL_AREA)
  {
    theParameters.theLog << "**  FULL AREA DATA   **" << endl;
    log_weather_result_data(theParameters.theLog, *theParameters.theCompleteData[FULL_AREA]);
  }
}

void log_subperiods(wf_story_params& theParameters)
{
  NightAndDayPeriodGenerator generator(theParameters.theForecastPeriod, theParameters.theVariable);

  for (unsigned int i = 1; i <= generator.size(); i++)
  {
    weather_result_data_item_vector resultVector;
    if (generator.isday(i))
    {
      get_part_of_the_day_time_series(
          theParameters, generator.period(i), AAMU, PRECIPITATION_MAX_DATA, resultVector);
      log_weather_result_time_series(theParameters.theLog, "AAMUPERIODI MAX", resultVector);
      resultVector.clear();

      get_part_of_the_day_time_series(
          theParameters, generator.period(i), AAMUPAIVA, PRECIPITATION_MAX_DATA, resultVector);
      log_weather_result_time_series(theParameters.theLog, "AAMUPÄIVÄPERIODI MAX", resultVector);
      resultVector.clear();
      get_part_of_the_day_time_series(
          theParameters, generator.period(i), ILTAPAIVA, PRECIPITATION_MAX_DATA, resultVector);
      log_weather_result_time_series(theParameters.theLog, "ILTAPÄIVÄPERIODI MAX", resultVector);
      resultVector.clear();
      get_part_of_the_day_time_series(
          theParameters, generator.period(i), ILTA, PRECIPITATION_MAX_DATA, resultVector);
      log_weather_result_time_series(theParameters.theLog, "ILTAPERIODI MAX", resultVector);
      resultVector.clear();
    }
    else
    {
      get_part_of_the_day_time_series(
          theParameters, generator.period(i), ILTA, PRECIPITATION_MAX_DATA, resultVector);
      log_weather_result_time_series(theParameters.theLog, "ILTAPERIODI MAX", resultVector);
      resultVector.clear();
      get_part_of_the_day_time_series(
          theParameters, generator.period(i), ILTAYO, PRECIPITATION_MAX_DATA, resultVector);
      log_weather_result_time_series(theParameters.theLog, "ILTAYÖPERIODI MAX", resultVector);
      resultVector.clear();
      get_part_of_the_day_time_series(
          theParameters, generator.period(i), YO, PRECIPITATION_MAX_DATA, resultVector);
      log_weather_result_time_series(theParameters.theLog, "YÖPERIODI MAX", resultVector);
      resultVector.clear();
      get_part_of_the_day_time_series(
          theParameters, generator.period(i), AAMUYO, PRECIPITATION_MAX_DATA, resultVector);
      log_weather_result_time_series(theParameters.theLog, "AAMUYÖPERIODI MAX", resultVector);
      resultVector.clear();
    }
  }
}

void populate_precipitation_time_series(const string& theVariable,
                                        const AnalysisSources& theSources,
                                        const WeatherArea& theArea,
                                        weather_forecast_result_container& theHourlyDataContainer)
{
  GridForecaster theForecaster;

  weather_result_data_item_vector* precipitationMaxHourly =
      theHourlyDataContainer[PRECIPITATION_MAX_DATA];
  weather_result_data_item_vector* precipitationMeanHourly =
      theHourlyDataContainer[PRECIPITATION_MEAN_DATA];
  weather_result_data_item_vector* precipitationExtentHourly =
      theHourlyDataContainer[PRECIPITATION_EXTENT_DATA];
  weather_result_data_item_vector* precipitationFormWaterHourly =
      theHourlyDataContainer[PRECIPITATION_FORM_WATER_DATA];
  weather_result_data_item_vector* precipitationFormDrizzleHourly =
      theHourlyDataContainer[PRECIPITATION_FORM_DRIZZLE_DATA];
  weather_result_data_item_vector* precipitationFormSleetHourly =
      theHourlyDataContainer[PRECIPITATION_FORM_SLEET_DATA];
  weather_result_data_item_vector* precipitationFormSnowHourly =
      theHourlyDataContainer[PRECIPITATION_FORM_SNOW_DATA];
  weather_result_data_item_vector* precipitationFormFreezingHourly =
      theHourlyDataContainer[PRECIPITATION_FORM_FREEZING_DATA];
  weather_result_data_item_vector* precipitationTypeHourly =
      theHourlyDataContainer[PRECIPITATION_TYPE_DATA];
  weather_result_data_item_vector* precipitationShareNorthEastHourly =
      theHourlyDataContainer[PRECIPITATION_NORTHEAST_SHARE_DATA];
  weather_result_data_item_vector* precipitationShareSouthEastHourly =
      theHourlyDataContainer[PRECIPITATION_SOUTHEAST_SHARE_DATA];
  weather_result_data_item_vector* precipitationShareSouthWestHourly =
      theHourlyDataContainer[PRECIPITATION_SOUTHWEST_SHARE_DATA];
  weather_result_data_item_vector* precipitationShareNorthWestHourly =
      theHourlyDataContainer[PRECIPITATION_NORTHWEST_SHARE_DATA];
  weather_result_data_item_vector* precipitationPointHourly =
      theHourlyDataContainer[PRECIPITATION_POINT_DATA];

  RangeAcceptor precipitationlimits;
  precipitationlimits.lowerLimit(DRY_WEATHER_LIMIT_DRIZZLE);
  ValueAcceptor waterfilter;
  waterfilter.value(kTRain);  // 1 = water
  // RangeAcceptor percentagelimits;
  // percentagelimits.lowerLimit(maxprecipitationlimit);
  ValueAcceptor drizzlefilter;
  drizzlefilter.value(kTDrizzle);  // 0 = drizzle
  ValueAcceptor sleetfilter;
  sleetfilter.value(kTSleet);  // 2 = sleet
  ValueAcceptor snowfilter;
  snowfilter.value(kTSnow);  // 3 = snow
  ValueAcceptor freezingfilter;
  freezingfilter.value(kTFreezingDrizzle);  // 4 = freezing
  ValueAcceptor showerfilter;
  showerfilter.value(kTConvectivePrecipitation);  // 1=large scale, 2=showers

  for (unsigned int i = 0; i < precipitationMaxHourly->size(); i++)
  {
    (*precipitationMaxHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              Precipitation,
                              Maximum,
                              Sum,
                              theArea,
                              (*precipitationMaxHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              precipitationlimits);

    (*precipitationMeanHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              Precipitation,
                              Mean,
                              Sum,
                              theArea,
                              (*precipitationMeanHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              precipitationlimits);

    (*precipitationExtentHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              Precipitation,
                              Percentage,
                              Sum,
                              theArea,
                              (*precipitationExtentHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              precipitationlimits);

    (*precipitationFormWaterHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              PrecipitationForm,
                              Mean,
                              Percentage,
                              theArea,
                              (*precipitationMaxHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              waterfilter);

    (*precipitationFormDrizzleHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              PrecipitationForm,
                              Mean,
                              Percentage,
                              theArea,
                              (*precipitationMaxHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              drizzlefilter);

    (*precipitationFormSleetHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              PrecipitationForm,
                              Mean,
                              Percentage,
                              theArea,
                              (*precipitationMaxHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              sleetfilter);

    (*precipitationFormSnowHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              PrecipitationForm,
                              Mean,
                              Percentage,
                              theArea,
                              (*precipitationMaxHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              snowfilter);

    (*precipitationFormFreezingHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              PrecipitationForm,
                              Mean,
                              Percentage,
                              theArea,
                              (*precipitationMaxHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              freezingfilter);

    (*precipitationTypeHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              PrecipitationType,
                              Mean,
                              Percentage,
                              theArea,
                              (*precipitationMaxHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              showerfilter);

    NFmiPoint precipitationPoint =
        AreaTools::getArealDistribution(theSources,
                                        Precipitation,
                                        theArea,
                                        (*precipitationShareNorthEastHourly)[i]->thePeriod,
                                        precipitationlimits,
                                        (*precipitationShareNorthEastHourly)[i]->theResult,
                                        (*precipitationShareSouthEastHourly)[i]->theResult,
                                        (*precipitationShareSouthWestHourly)[i]->theResult,
                                        (*precipitationShareNorthWestHourly)[i]->theResult);

    // lets store lat/lon pair into
    WeatherResult precipitationPointResult(precipitationPoint.X(), precipitationPoint.Y());
    (*precipitationPointHourly)[i]->theResult = precipitationPointResult;
  }
}

void populate_precipitation_time_series(wf_story_params& theParameters)
{
  if (theParameters.theForecastArea & INLAND_AREA)
  {
    WeatherArea inlandArea = theParameters.theArea;
    inlandArea.type(WeatherArea::Inland);
    populate_precipitation_time_series(theParameters.theVariable,
                                       theParameters.theSources,
                                       inlandArea,
                                       *theParameters.theCompleteData[INLAND_AREA]);
  }
  if (theParameters.theForecastArea & COASTAL_AREA)
  {
    WeatherArea coastalArea = theParameters.theArea;
    coastalArea.type(WeatherArea::Coast);
    populate_precipitation_time_series(theParameters.theVariable,
                                       theParameters.theSources,
                                       coastalArea,
                                       *theParameters.theCompleteData[COASTAL_AREA]);
  }
  if (theParameters.theForecastArea & FULL_AREA)
  {
    populate_precipitation_time_series(theParameters.theVariable,
                                       theParameters.theSources,
                                       theParameters.theArea,
                                       *theParameters.theCompleteData[FULL_AREA]);
  }
}

void populate_thunderprobability_time_series(
    const string& theVariable,
    const AnalysisSources& theSources,
    const WeatherArea& theArea,
    weather_forecast_result_container& theHourlyDataContainer)
{
  weather_result_data_item_vector& thunderProbabilityHourly =
      *(theHourlyDataContainer[THUNDER_PROBABILITY_DATA]);
  weather_result_data_item_vector* thunderExtentHourly =
      theHourlyDataContainer[THUNDER_EXTENT_DATA];
  weather_result_data_item_vector* thunderNorthEastHourly =
      theHourlyDataContainer[THUNDER_NORTHEAST_SHARE_DATA];
  weather_result_data_item_vector* thunderSouthEastHourly =
      theHourlyDataContainer[THUNDER_SOUTHEAST_SHARE_DATA];
  weather_result_data_item_vector* thunderSouthWestHourly =
      theHourlyDataContainer[THUNDER_SOUTHWEST_SHARE_DATA];
  weather_result_data_item_vector* thunderNorthWestHourly =
      theHourlyDataContainer[THUNDER_NORTHWEST_SHARE_DATA];

  RangeAcceptor thunderlimits;
  thunderlimits.lowerLimit(5.0);  // 5 % propability is the minimum

  GridForecaster theForecaster;

  for (unsigned int i = 0; i < thunderProbabilityHourly.size(); i++)
  {
    thunderProbabilityHourly[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              Thunder,
                              Maximum,
                              Maximum,
                              theArea,
                              thunderProbabilityHourly[i]->thePeriod);

    (*thunderExtentHourly)[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              Thunder,
                              Percentage,
                              Maximum,
                              theArea,
                              (*thunderExtentHourly)[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              thunderlimits);

    RangeAcceptor thunderlimits;
    thunderlimits.lowerLimit(SMALL_PROBABILITY_FOR_THUNDER_LOWER_LIMIT);
    AreaTools::getArealDistribution(theSources,
                                    Thunder,
                                    theArea,
                                    (*thunderNorthEastHourly)[i]->thePeriod,
                                    thunderlimits,
                                    (*thunderNorthEastHourly)[i]->theResult,
                                    (*thunderSouthEastHourly)[i]->theResult,
                                    (*thunderSouthWestHourly)[i]->theResult,
                                    (*thunderNorthWestHourly)[i]->theResult);
  }
}

void populate_thunderprobability_time_series(wf_story_params& theParameters)
{
  if (theParameters.theForecastArea & INLAND_AREA)
  {
    WeatherArea inlandArea = theParameters.theArea;
    inlandArea.type(WeatherArea::Inland);
    populate_thunderprobability_time_series(theParameters.theVariable,
                                            theParameters.theSources,
                                            inlandArea,
                                            *theParameters.theCompleteData[INLAND_AREA]);
  }

  if (theParameters.theForecastArea & COASTAL_AREA)
  {
    WeatherArea coastalArea = theParameters.theArea;
    coastalArea.type(WeatherArea::Coast);
    populate_thunderprobability_time_series(theParameters.theVariable,
                                            theParameters.theSources,
                                            coastalArea,
                                            *theParameters.theCompleteData[COASTAL_AREA]);
  }

  if (theParameters.theForecastArea & FULL_AREA)
  {
    populate_thunderprobability_time_series(theParameters.theVariable,
                                            theParameters.theSources,
                                            theParameters.theArea,
                                            *theParameters.theCompleteData[FULL_AREA]);
  }
}

void populate_fogintensity_time_series(const string& theVariable,
                                       const AnalysisSources& theSources,
                                       const WeatherArea& theArea,
                                       weather_forecast_result_container& theHourlyDataContainer)
{
  weather_result_data_item_vector& fogIntensityModerateHourly =
      *(theHourlyDataContainer[FOG_INTENSITY_MODERATE_DATA]);
  weather_result_data_item_vector& fogIntensityDenseHourly =
      *(theHourlyDataContainer[FOG_INTENSITY_DENSE_DATA]);
  weather_result_data_item_vector* fogNorthEastHourly =
      theHourlyDataContainer[FOG_NORTHEAST_SHARE_DATA];
  weather_result_data_item_vector* fogSouthEastHourly =
      theHourlyDataContainer[FOG_SOUTHEAST_SHARE_DATA];
  weather_result_data_item_vector* fogSouthWestHourly =
      theHourlyDataContainer[FOG_SOUTHWEST_SHARE_DATA];
  weather_result_data_item_vector* fogNorthWestHourly =
      theHourlyDataContainer[FOG_NORTHWEST_SHARE_DATA];

  GridForecaster theForecaster;
  ValueAcceptor moderateFogFilter;
  moderateFogFilter.value(kTModerateFog);
  ValueAcceptor denseFogFilter;
  denseFogFilter.value(kTDenseFog);

  for (unsigned int i = 0; i < fogIntensityModerateHourly.size(); i++)
  {
    fogIntensityModerateHourly[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              Fog,
                              Mean,
                              Percentage,
                              theArea,
                              fogIntensityModerateHourly[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              moderateFogFilter);

    fogIntensityDenseHourly[i]->theResult =
        theForecaster.analyze(theVariable,
                              theSources,
                              Fog,
                              Mean,
                              Percentage,
                              theArea,
                              fogIntensityDenseHourly[i]->thePeriod,
                              DefaultAcceptor(),
                              DefaultAcceptor(),
                              denseFogFilter);

    RangeAcceptor foglimits;
    foglimits.lowerLimit(kTModerateFog);
    AreaTools::getArealDistribution(theSources,
                                    Fog,
                                    theArea,
                                    (*fogNorthEastHourly)[i]->thePeriod,
                                    foglimits,
                                    (*fogNorthEastHourly)[i]->theResult,
                                    (*fogSouthEastHourly)[i]->theResult,
                                    (*fogSouthWestHourly)[i]->theResult,
                                    (*fogNorthWestHourly)[i]->theResult);
  }
}

void populate_fogintensity_time_series(wf_story_params& theParameters)
{
  if (theParameters.theForecastArea & INLAND_AREA)
  {
    WeatherArea inlandArea = theParameters.theArea;
    inlandArea.type(WeatherArea::Inland);
    populate_fogintensity_time_series(theParameters.theVariable,
                                      theParameters.theSources,
                                      inlandArea,
                                      *theParameters.theCompleteData[INLAND_AREA]);
  }

  if (theParameters.theForecastArea & COASTAL_AREA)
  {
    WeatherArea coastalArea = theParameters.theArea;
    coastalArea.type(WeatherArea::Coast);
    populate_fogintensity_time_series(theParameters.theVariable,
                                      theParameters.theSources,
                                      coastalArea,
                                      *theParameters.theCompleteData[COASTAL_AREA]);
  }

  if (theParameters.theForecastArea & FULL_AREA)
  {
    populate_fogintensity_time_series(theParameters.theVariable,
                                      theParameters.theSources,
                                      theParameters.theArea,
                                      *theParameters.theCompleteData[FULL_AREA]);
  }
}

void populate_cloudiness_time_series(const string& theVariable,
                                     const AnalysisSources& theSources,
                                     const WeatherArea& theArea,
                                     weather_forecast_result_container& theHourlyDataContainer)
{
  weather_result_data_item_vector& cloudinessHourly = *(theHourlyDataContainer[CLOUDINESS_DATA]);
  weather_result_data_item_vector* cloudinessNorthEastHourly =
      theHourlyDataContainer[CLOUDINESS_NORTHEAST_SHARE_DATA];
  weather_result_data_item_vector* cloudinessSouthEastHourly =
      theHourlyDataContainer[CLOUDINESS_SOUTHEAST_SHARE_DATA];
  weather_result_data_item_vector* cloudinessSouthWestHourly =
      theHourlyDataContainer[CLOUDINESS_SOUTHWEST_SHARE_DATA];
  weather_result_data_item_vector* cloudinessNorthWestHourly =
      theHourlyDataContainer[CLOUDINESS_NORTHWEST_SHARE_DATA];

  GridForecaster theForecaster;
  for (unsigned int i = 0; i < cloudinessHourly.size(); i++)
  {
    // areal function Maximum changed to Mean (after consulting with Kaisa 25.11.2010)
    cloudinessHourly[i]->theResult = theForecaster.analyze(
        theVariable, theSources, Cloudiness, Mean, Mean, theArea, cloudinessHourly[i]->thePeriod);

    RangeAcceptor cloudinesslimits;
    cloudinesslimits.lowerLimit(VERRATTAIN_PILVISTA_LOWER_LIMIT);
    AreaTools::getArealDistribution(theSources,
                                    Cloudiness,
                                    theArea,
                                    (*cloudinessNorthEastHourly)[i]->thePeriod,
                                    cloudinesslimits,
                                    (*cloudinessNorthEastHourly)[i]->theResult,
                                    (*cloudinessSouthEastHourly)[i]->theResult,
                                    (*cloudinessSouthWestHourly)[i]->theResult,
                                    (*cloudinessNorthWestHourly)[i]->theResult);
  }
}

void populate_cloudiness_time_series(wf_story_params& theParameters)
{
  // theParameters.theVariable+"::fake::"+hourstr+"::cloudiness",
  if (theParameters.theForecastArea & INLAND_AREA)
  {
    WeatherArea inlandArea = theParameters.theArea;
    inlandArea.type(WeatherArea::Inland);
    populate_cloudiness_time_series(theParameters.theVariable,
                                    theParameters.theSources,
                                    inlandArea,
                                    *theParameters.theCompleteData[INLAND_AREA]);
  }

  if (theParameters.theForecastArea & COASTAL_AREA)
  {
    WeatherArea coastalArea = theParameters.theArea;
    coastalArea.type(WeatherArea::Coast);
    populate_cloudiness_time_series(theParameters.theVariable,
                                    theParameters.theSources,
                                    coastalArea,
                                    *theParameters.theCompleteData[COASTAL_AREA]);
  }

  if (theParameters.theForecastArea & FULL_AREA)
  {
    populate_cloudiness_time_series(theParameters.theVariable,
                                    theParameters.theSources,
                                    theParameters.theArea,
                                    *theParameters.theCompleteData[FULL_AREA]);
  }
}

void allocate_data_structures(wf_story_params& theParameters, const forecast_area_id& theAreaId)
{
  weather_result_data_item_vector* hourlyMaxPrecipitation = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyMeanPrecipitation = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationExtent =
      new weather_result_data_item_vector();

  weather_result_data_item_vector* hourlyPrecipitationType = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationFormWater =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationFormDrizzle =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationFormSleet =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationFormSnow =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationFormFreezing =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyCloudiness = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyThunderProbability = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyThunderExtent = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyFogIntensityModerate =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyFogIntensityDense = new weather_result_data_item_vector();

  weather_result_data_item_vector* hourlyPrecipitationShareNortEast =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationShareSouthEast =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationShareSouthWest =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationShareNorthWest =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyPrecipitationPoint = new weather_result_data_item_vector();

  weather_result_data_item_vector* hourlyCloudinessShareNortEast =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyCloudinessShareSouthEast =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyCloudinessShareSouthWest =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyCloudinessShareNorthWest =
      new weather_result_data_item_vector();

  weather_result_data_item_vector* hourlyThunderProbabilityShareNortEast =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyThunderProbabilityShareSouthEast =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyThunderProbabilityShareSouthWest =
      new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyThunderProbabilityShareNorthWest =
      new weather_result_data_item_vector();

  weather_result_data_item_vector* hourlyFogShareNortEast = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyFogShareSouthEast = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyFogShareSouthWest = new weather_result_data_item_vector();
  weather_result_data_item_vector* hourlyFogShareNorthWest = new weather_result_data_item_vector();

  // first split the whole period to one-hour subperiods
  TextGenPosixTime periodStartTime = theParameters.theDataPeriod.localStartTime();

  theParameters.theHourPeriodCount = 0;
  while (periodStartTime.IsLessThan(theParameters.theDataPeriod.localEndTime()))
  {
    TextGenPosixTime periodEndTime = periodStartTime;
    periodEndTime.ChangeByHours(1);
    WeatherPeriod theWeatherPeriod(periodEndTime, periodEndTime);
    WeatherResult theWeatherResult(kFloatMissing, kFloatMissing);

    part_of_the_day_id partOfTheDayId = get_part_of_the_day_id_large(theWeatherPeriod);

    hourlyMaxPrecipitation->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyMeanPrecipitation->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationExtent->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationType->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationFormWater->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationFormDrizzle->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationFormSleet->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationFormSnow->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationFormFreezing->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationPoint->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyCloudiness->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyThunderProbability->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyThunderExtent->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyFogIntensityModerate->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyFogIntensityDense->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));

    hourlyPrecipitationShareNortEast->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationShareSouthEast->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationShareSouthWest->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyPrecipitationShareNorthWest->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));

    hourlyCloudinessShareNortEast->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyCloudinessShareSouthEast->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyCloudinessShareSouthWest->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyCloudinessShareNorthWest->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));

    hourlyThunderProbabilityShareNortEast->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyThunderProbabilityShareSouthEast->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyThunderProbabilityShareSouthWest->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyThunderProbabilityShareNorthWest->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));

    hourlyFogShareNortEast->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyFogShareSouthEast->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyFogShareSouthWest->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));
    hourlyFogShareNorthWest->push_back(
        new WeatherResultDataItem(theWeatherPeriod, theWeatherResult, partOfTheDayId));

    periodStartTime.ChangeByHours(1);
    theParameters.theHourPeriodCount++;
  }

  weather_forecast_result_container* resultContainer = new weather_forecast_result_container();

  resultContainer->insert(make_pair(PRECIPITATION_MAX_DATA, hourlyMaxPrecipitation));
  resultContainer->insert(make_pair(PRECIPITATION_MEAN_DATA, hourlyMeanPrecipitation));
  resultContainer->insert(make_pair(PRECIPITATION_EXTENT_DATA, hourlyPrecipitationExtent));
  resultContainer->insert(make_pair(PRECIPITATION_TYPE_DATA, hourlyPrecipitationType));
  resultContainer->insert(make_pair(PRECIPITATION_FORM_WATER_DATA, hourlyPrecipitationFormWater));
  resultContainer->insert(
      make_pair(PRECIPITATION_FORM_DRIZZLE_DATA, hourlyPrecipitationFormDrizzle));
  resultContainer->insert(make_pair(PRECIPITATION_FORM_SLEET_DATA, hourlyPrecipitationFormSleet));
  resultContainer->insert(make_pair(PRECIPITATION_FORM_SNOW_DATA, hourlyPrecipitationFormSnow));
  resultContainer->insert(
      make_pair(PRECIPITATION_FORM_FREEZING_DATA, hourlyPrecipitationFormFreezing));
  resultContainer->insert(make_pair(CLOUDINESS_DATA, hourlyCloudiness));
  resultContainer->insert(make_pair(THUNDER_PROBABILITY_DATA, hourlyThunderProbability));
  resultContainer->insert(make_pair(THUNDER_EXTENT_DATA, hourlyThunderExtent));
  resultContainer->insert(make_pair(FOG_INTENSITY_MODERATE_DATA, hourlyFogIntensityModerate));
  resultContainer->insert(make_pair(FOG_INTENSITY_DENSE_DATA, hourlyFogIntensityDense));
  resultContainer->insert(
      make_pair(PRECIPITATION_NORTHEAST_SHARE_DATA, hourlyPrecipitationShareNortEast));
  resultContainer->insert(
      make_pair(PRECIPITATION_SOUTHEAST_SHARE_DATA, hourlyPrecipitationShareSouthEast));
  resultContainer->insert(
      make_pair(PRECIPITATION_SOUTHWEST_SHARE_DATA, hourlyPrecipitationShareSouthWest));
  resultContainer->insert(
      make_pair(PRECIPITATION_NORTHWEST_SHARE_DATA, hourlyPrecipitationShareNorthWest));
  resultContainer->insert(make_pair(PRECIPITATION_POINT_DATA, hourlyPrecipitationPoint));
  resultContainer->insert(
      make_pair(CLOUDINESS_NORTHEAST_SHARE_DATA, hourlyCloudinessShareNortEast));
  resultContainer->insert(
      make_pair(CLOUDINESS_SOUTHEAST_SHARE_DATA, hourlyCloudinessShareSouthEast));
  resultContainer->insert(
      make_pair(CLOUDINESS_SOUTHWEST_SHARE_DATA, hourlyCloudinessShareSouthWest));
  resultContainer->insert(
      make_pair(CLOUDINESS_NORTHWEST_SHARE_DATA, hourlyCloudinessShareNorthWest));
  resultContainer->insert(
      make_pair(THUNDER_NORTHEAST_SHARE_DATA, hourlyThunderProbabilityShareNortEast));
  resultContainer->insert(
      make_pair(THUNDER_SOUTHEAST_SHARE_DATA, hourlyThunderProbabilityShareSouthEast));
  resultContainer->insert(
      make_pair(THUNDER_SOUTHWEST_SHARE_DATA, hourlyThunderProbabilityShareSouthWest));
  resultContainer->insert(
      make_pair(THUNDER_NORTHWEST_SHARE_DATA, hourlyThunderProbabilityShareNorthWest));
  resultContainer->insert(make_pair(FOG_NORTHEAST_SHARE_DATA, hourlyFogShareNortEast));
  resultContainer->insert(make_pair(FOG_SOUTHEAST_SHARE_DATA, hourlyFogShareSouthEast));
  resultContainer->insert(make_pair(FOG_SOUTHWEST_SHARE_DATA, hourlyFogShareSouthWest));
  resultContainer->insert(make_pair(FOG_NORTHWEST_SHARE_DATA, hourlyFogShareNorthWest));

  theParameters.theCompleteData.insert(make_pair(theAreaId, resultContainer));
}

void create_data_structures(wf_story_params& theParameters)
{
  // first check which areas exists and create the data structures accordinly
  WeatherArea fullArea = theParameters.theArea;
  WeatherArea inlandArea = theParameters.theArea;
  inlandArea.type(WeatherArea::Inland);
  WeatherArea coastalArea = theParameters.theArea;
  coastalArea.type(WeatherArea::Coast);
  bool inlandExists = false;
  bool coastExists = false;
  GridForecaster theForecaster;
  WeatherResult result = theForecaster.analyze(theParameters.theVariable,
                                               theParameters.theSources,
                                               Temperature,
                                               Mean,
                                               Maximum,
                                               inlandArea,
                                               theParameters.theForecastPeriod);
  if (result.value() != kFloatMissing)
  {
    inlandExists = true;
  }

  result = theForecaster.analyze(theParameters.theVariable,
                                 theParameters.theSources,
                                 Temperature,
                                 Mean,
                                 Maximum,
                                 coastalArea,
                                 theParameters.theForecastPeriod);
  if (result.value() != kFloatMissing)
  {
    coastExists = true;
  }

  if (inlandExists && coastExists)
  {
    // create data structures for inland, coast and full areas
    allocate_data_structures(theParameters, INLAND_AREA);
    allocate_data_structures(theParameters, COASTAL_AREA);
    allocate_data_structures(theParameters, FULL_AREA);
    theParameters.theForecastArea |= INLAND_AREA;
    theParameters.theForecastArea |= COASTAL_AREA;
    theParameters.theForecastArea |= FULL_AREA;

    theParameters.theLog << "inland area included" << endl;
    theParameters.theLog << "coastal area included" << endl;
    theParameters.theLog << "full area included" << endl;
  }
  else if (inlandExists)
  {
    // create data structures for inland only
    allocate_data_structures(theParameters, INLAND_AREA);
    theParameters.theForecastArea |= INLAND_AREA;
    theParameters.theLog << "inland area included" << endl;
  }
  else if (coastExists)
  {
    // create data structures for coast only
    allocate_data_structures(theParameters, COASTAL_AREA);
    theParameters.theForecastArea |= COASTAL_AREA;
    theParameters.theLog << "coastal area included" << endl;
  }
  else
  {
    // error not inland, nor coastal area included
  }
}

void deallocate_data_structure(unsigned int& thePeriodCount,
                               weather_forecast_result_container& theResultContainer)
{
  for (unsigned int i = 0; i < thePeriodCount; i++)
  {
    delete (*theResultContainer[CLOUDINESS_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_MAX_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_MEAN_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_EXTENT_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_TYPE_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_FORM_WATER_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_FORM_DRIZZLE_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_FORM_SLEET_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_FORM_SNOW_DATA])[i];
    delete (*theResultContainer[PRECIPITATION_FORM_FREEZING_DATA])[i];
    delete (*theResultContainer[THUNDER_PROBABILITY_DATA])[i];
    delete (*theResultContainer[THUNDER_EXTENT_DATA])[i];
    delete (*theResultContainer[FOG_INTENSITY_MODERATE_DATA])[i];
  }

  theResultContainer[PRECIPITATION_MAX_DATA]->clear();
  theResultContainer[PRECIPITATION_MEAN_DATA]->clear();
  theResultContainer[PRECIPITATION_EXTENT_DATA]->clear();
  theResultContainer[PRECIPITATION_TYPE_DATA]->clear();
  theResultContainer[PRECIPITATION_FORM_WATER_DATA]->clear();
  theResultContainer[PRECIPITATION_FORM_DRIZZLE_DATA]->clear();
  theResultContainer[PRECIPITATION_FORM_SLEET_DATA]->clear();
  theResultContainer[PRECIPITATION_FORM_SNOW_DATA]->clear();
  theResultContainer[PRECIPITATION_FORM_FREEZING_DATA]->clear();
  theResultContainer[THUNDER_PROBABILITY_DATA]->clear();
  theResultContainer[THUNDER_EXTENT_DATA]->clear();
  theResultContainer[FOG_INTENSITY_MODERATE_DATA]->clear();
  theResultContainer[FOG_INTENSITY_DENSE_DATA]->clear();

  delete theResultContainer[CLOUDINESS_DATA];
  delete theResultContainer[CLOUDINESS_NORTHEAST_SHARE_DATA];
  delete theResultContainer[CLOUDINESS_SOUTHEAST_SHARE_DATA];
  delete theResultContainer[CLOUDINESS_SOUTHWEST_SHARE_DATA];
  delete theResultContainer[CLOUDINESS_NORTHWEST_SHARE_DATA];
  delete theResultContainer[PRECIPITATION_MAX_DATA];
  delete theResultContainer[PRECIPITATION_MEAN_DATA];
  delete theResultContainer[PRECIPITATION_EXTENT_DATA];
  delete theResultContainer[PRECIPITATION_TYPE_DATA];
  delete theResultContainer[PRECIPITATION_FORM_WATER_DATA];
  delete theResultContainer[PRECIPITATION_FORM_DRIZZLE_DATA];
  delete theResultContainer[PRECIPITATION_FORM_SLEET_DATA];
  delete theResultContainer[PRECIPITATION_FORM_SNOW_DATA];
  delete theResultContainer[PRECIPITATION_FORM_FREEZING_DATA];
  delete theResultContainer[PRECIPITATION_NORTHEAST_SHARE_DATA];
  delete theResultContainer[PRECIPITATION_SOUTHEAST_SHARE_DATA];
  delete theResultContainer[PRECIPITATION_SOUTHWEST_SHARE_DATA];
  delete theResultContainer[PRECIPITATION_NORTHWEST_SHARE_DATA];
  delete theResultContainer[PRECIPITATION_POINT_DATA];
  delete theResultContainer[THUNDER_PROBABILITY_DATA];
  delete theResultContainer[THUNDER_EXTENT_DATA];
  delete theResultContainer[THUNDER_NORTHEAST_SHARE_DATA];
  delete theResultContainer[THUNDER_SOUTHEAST_SHARE_DATA];
  delete theResultContainer[THUNDER_SOUTHWEST_SHARE_DATA];
  delete theResultContainer[THUNDER_NORTHWEST_SHARE_DATA];
  delete theResultContainer[FOG_INTENSITY_MODERATE_DATA];
  delete theResultContainer[FOG_INTENSITY_DENSE_DATA];
  delete theResultContainer[FOG_NORTHEAST_SHARE_DATA];
  delete theResultContainer[FOG_SOUTHEAST_SHARE_DATA];
  delete theResultContainer[FOG_SOUTHWEST_SHARE_DATA];
  delete theResultContainer[FOG_NORTHWEST_SHARE_DATA];

  theResultContainer.clear();
}

void delete_data_structures(wf_story_params& theParameters)
{
  if (theParameters.theForecastArea & INLAND_AREA)
  {
    deallocate_data_structure(theParameters.theHourPeriodCount,
                              *theParameters.theCompleteData[INLAND_AREA]);
  }
  if (theParameters.theForecastArea & COASTAL_AREA)
  {
    deallocate_data_structure(theParameters.theHourPeriodCount,
                              *theParameters.theCompleteData[COASTAL_AREA]);
  }
  if (theParameters.theForecastArea & FULL_AREA)
  {
    deallocate_data_structure(theParameters.theHourPeriodCount,
                              *theParameters.theCompleteData[FULL_AREA]);
  }

  for (unsigned int i = 0; i < theParameters.theCloudinessData.size(); i++)
  {
    cloudiness_data_item_container* cloudinessDataItemContainer =
        theParameters.theCloudinessData[i];
    for (unsigned int k = 0; k < cloudinessDataItemContainer->size(); k++)
      delete (*cloudinessDataItemContainer)[k];
    cloudinessDataItemContainer->clear();
  }
  theParameters.theCloudinessData.clear();
  for (unsigned int i = 0; i < theParameters.thePrecipitationData.size(); i++)
  {
    precipitation_data_item_container* precipitationDataItemContainer =
        theParameters.thePrecipitationData[i];
    for (unsigned int k = 0; k < precipitationDataItemContainer->size(); k++)
      delete (*precipitationDataItemContainer)[k];
    precipitationDataItemContainer->clear();
  }
  theParameters.thePrecipitationData.clear();

  for (unsigned int i = 0; i < theParameters.theThunderData.size(); i++)
  {
    thunder_data_item_container* thunderDataItemContainer = theParameters.theThunderData[i];
    for (unsigned int k = 0; k < thunderDataItemContainer->size(); k++)
      delete (*thunderDataItemContainer)[k];
    thunderDataItemContainer->clear();
  }
  theParameters.theFogData.clear();
  for (unsigned int i = 0; i < theParameters.theFogData.size(); i++)
  {
    fog_data_item_container* fogIntensityDataItemContainer = theParameters.theFogData[i];
    for (unsigned int k = 0; k < fogIntensityDataItemContainer->size(); k++)
      delete (*fogIntensityDataItemContainer)[k];
    fogIntensityDataItemContainer->clear();
  }
  theParameters.theFogData.clear();
}

void init_parameters(wf_story_params& theParameters)
{
  create_data_structures(theParameters);

  theParameters.theDryWeatherLimitWater = Settings::optional_double(
      theParameters.theVariable + "::dry_weather_limit_water", DRY_WEATHER_LIMIT_WATER);
  theParameters.theDryWeatherLimitDrizzle = Settings::optional_double(
      theParameters.theVariable + "::dry_weather_limit_drizzle", DRY_WEATHER_LIMIT_DRIZZLE);
  theParameters.theDryWeatherLimitSleet = Settings::optional_double(
      theParameters.theVariable + "::dry_weather_limit_sleet", DRY_WEATHER_LIMIT_SLEET);
  theParameters.theDryWeatherLimitSnow = Settings::optional_double(
      theParameters.theVariable + "::dry_weather_limit_snow", DRY_WEATHER_LIMIT_SNOW);
  theParameters.theWeakPrecipitationLimitWater =
      Settings::optional_double(theParameters.theVariable + "::weak_precipitation_limit_water",
                                WEAK_PRECIPITATION_LIMIT_WATER);
  theParameters.theWeakPrecipitationLimitSleet =
      Settings::optional_double(theParameters.theVariable + "::weak_precipitation_limit_sleet",
                                WEAK_PRECIPITATION_LIMIT_SLEET);
  theParameters.theWeakPrecipitationLimitSnow = Settings::optional_double(
      theParameters.theVariable + "::weak_precipitation_limit_snow", WEAK_PRECIPITATION_LIMIT_SNOW);
  theParameters.theHeavyPrecipitationLimitWater =
      Settings::optional_double(theParameters.theVariable + "::heavy_precipitation_limit_water",
                                HEAVY_PRECIPITATION_LIMIT_WATER);
  theParameters.theHeavyPrecipitationLimitSleet =
      Settings::optional_double(theParameters.theVariable + "::heavy_precipitation_limit_sleet",
                                HEAVY_PRECIPITATION_LIMIT_SLEET);
  theParameters.theHeavyPrecipitationLimitSnow =
      Settings::optional_double(theParameters.theVariable + "::heavy_precipitation_limit_snow",
                                HEAVY_PRECIPITATION_LIMIT_SNOW);
  theParameters.thePrecipitationStormLimit =
      Settings::optional_double(theParameters.theVariable + "::rainstorm_limit", RAINSTORM_LIMIT);
  theParameters.theMostlyDryWeatherLimit = Settings::optional_double(
      theParameters.theVariable + "::mostly_dry_weather_limit", MOSTLY_DRY_WEATHER_LIMIT);
  theParameters.theInSomePlacesLowerLimit = Settings::optional_double(
      theParameters.theVariable + "::in_some_places_lower_limit", IN_SOME_PLACES_LOWER_LIMIT);
  theParameters.theInSomePlacesUpperLimit = Settings::optional_double(
      theParameters.theVariable + "::in_some_places_upper_limit", IN_SOME_PLACES_UPPER_LIMIT);
  theParameters.theInManyPlacesLowerLimit = Settings::optional_double(
      theParameters.theVariable + "::in_many_places_lower_limit", IN_MANY_PLACES_LOWER_LIMIT);
  theParameters.theInManyPlacesUpperLimit = Settings::optional_double(
      theParameters.theVariable + "::in_many_places_upper_limit", IN_MANY_PLACES_UPPER_LIMIT);
  theParameters.theFreezingPrecipitationLimit = Settings::optional_double(
      theParameters.theVariable + "::freezing_rain_limit", FREEZING_RAIN_LIMIT);
  theParameters.theClearSkyUpperLimit = Settings::optional_double(
      theParameters.theVariable + "::clear_sky_limit", SELKEA_UPPER_LIMIT);
  theParameters.theAlmostClearSkyUpperLimit = Settings::optional_double(
      theParameters.theVariable + "::almost_clear_sky_limit", MELKEIN_SELKEA_UPPER_LIMIT);
  theParameters.thePartlyCloudySkyUpperLimit = Settings::optional_double(
      theParameters.theVariable + "::partly_cloudy_sky_limit", PUOLIPILVISTA_UPPER_LIMIT);
  theParameters.theMostlyCloudySkyUpperLimit = Settings::optional_double(
      theParameters.theVariable + "::mostly_cloudy_sky_limit", VERRATTAIN_PILVISTA_UPPER_LIMIT);

  float coastalPercentage =
      get_area_percentage(theParameters.theVariable + "::fake::area_percentage",
                          theParameters.theArea,
                          WeatherArea::Coast,
                          theParameters.theSources,
                          theParameters.theForecastPeriod);

  float separate_coastal_area_percentage =
      Settings::optional_double(theParameters.theVariable + "::separate_coastal_area_percentage",
                                SEPARATE_COASTAL_AREA_PERCENTAGE);

  theParameters.theCoastalAndInlandTogetherFlag =
      (theParameters.theArea.isMarine() || theParameters.theArea.isIsland() ||
       (coastalPercentage > 0 && coastalPercentage < separate_coastal_area_percentage));

  if (theParameters.theCoastalAndInlandTogetherFlag)
  {
    if (theParameters.theArea.isMarine() == false && theParameters.theArea.isIsland() == false)
      theParameters.theLog << "Inland and coastal area(" << coastalPercentage << ") not separated!"
                           << endl;
  }
}

void log_weather_forecast_story(MessageLogger& theLog,
                                WeatherForecastStory& theWeatherForecastStory)
{
  theLog << "WEATHER FORECAST STORY FOR PERIOD ";
  log_start_time_and_end_time(theLog, "", theWeatherForecastStory.getStoryPeriod());

  const vector<WeatherForecastStoryItem*> storyItemVector(
      theWeatherForecastStory.getStoryItemVector());

  for (unsigned int i = 0; i < storyItemVector.size(); i++)
  {
    WeatherForecastStoryItem& storyItem = *(storyItemVector[i]);

    if (!storyItem.isIncluded()) continue;

    if (storyItem.getStoryPartId() == PRECIPITATION_STORY_PART)
      theLog << "PRECIPITATION STORY PART";
    else if (storyItem.getStoryPartId() == CLOUDINESS_STORY_PART)
      theLog << "CLOUDINESS STORY PART";
    else if (storyItem.getStoryPartId() == MISSING_STORY_PART)
      theLog << "MISSING STORY PART";
    theLog << " ";
    log_start_time_and_end_time(theLog, "", storyItem.getStoryItemPeriod());
    theLog << storyItem.getStoryItemSentence();
  }

  theLog << "THE COMPLETE SENTENCE BEGIN" << endl;
  theLog << theWeatherForecastStory.getWeatherForecastStory();
  theLog << "THE COMPLETE SENTENCE END" << endl;
}

const Paragraph weather_forecast(const TextGen::WeatherArea& itsArea,
                                 const TextGen::WeatherPeriod& itsPeriod,
                                 const TextGen::AnalysisSources& itsSources,
                                 const TextGenPosixTime& itsForecastTime,
                                 const std::string itsVar,
                                 MessageLogger& theLog)
{
  using namespace PrecipitationPeriodTools;

  Paragraph paragraph;

  // Period generator
  const HourPeriodGenerator periodgenerator(itsPeriod, itsVar + "::day");

  theLog << "Period " << itsPeriod.localStartTime() << "..." << itsPeriod.localEndTime();

  TextGenPosixTime dataPeriodStartTime(itsPeriod.localStartTime());
  TextGenPosixTime dataPeriodEndTime(itsPeriod.localEndTime());

  dataPeriodStartTime.ChangeByHours(-12);
  dataPeriodEndTime.ChangeByHours(12);

  log_start_time_and_end_time(theLog, "the forecast period: ", itsPeriod);

  log_start_time_and_end_time(
      theLog, "the data gathering period: ", WeatherPeriod(dataPeriodStartTime, dataPeriodEndTime));

  if (itsArea.isNamed())
  {
    std::string name(itsArea.name());
    theLog << "** " << name << " **" << endl;
  }

  WeatherPeriod theDataGatheringPeriod(dataPeriodStartTime, dataPeriodEndTime);

  wf_story_params theParameters(
      itsVar, itsArea, theDataGatheringPeriod, itsPeriod, itsForecastTime, itsSources, theLog);

  init_parameters(theParameters);

  if (theParameters.theForecastArea == NO_AREA) return paragraph;

  populate_precipitation_time_series(theParameters);
  populate_cloudiness_time_series(theParameters);
  populate_fogintensity_time_series(theParameters);
  populate_thunderprobability_time_series(theParameters);

  CloudinessForecast cloudinessForecast(theParameters);
  PrecipitationForecast precipitationForecast(theParameters);
  FogForecast fogForecast(theParameters);
  ThunderForecast thunderForecast(theParameters);

  theParameters.thePrecipitationForecast = &precipitationForecast;
  theParameters.theCloudinessForecast = &cloudinessForecast;
  theParameters.theFogForecast = &fogForecast;
  theParameters.theThunderForecast = &thunderForecast;

  precipitationForecast.printOutPrecipitationData(theLog);
  precipitationForecast.printOutPrecipitationPeriods(theLog, itsArea.isPoint());
  precipitationForecast.printOutPrecipitationWeatherEvents(theLog);
  //	cloudinessForecast.printOutCloudinessData(theLog);
  cloudinessForecast.printOutCloudinessWeatherEvents(theLog);
  fogForecast.printOutFogData(theLog);
  fogForecast.printOutFogPeriods(theLog);
  fogForecast.printOutFogTypePeriods(theLog);

  // log_weather_result_data(theParameters);

  WeatherForecastStory wfs(itsVar,
                           itsPeriod,
                           itsArea,
                           theParameters.theForecastArea,
                           itsForecastTime,
                           precipitationForecast,
                           cloudinessForecast,
                           fogForecast,
                           thunderForecast,
                           theParameters.theLog);

  wfs.logTheStoryItems();

  const_cast<WeatherHistory&>(itsArea.history())
      .updateTimePhrase("", "", TextGenPosixTime(1970, 1, 1));

  paragraph << wfs.getWeatherForecastStory();

  log_weather_forecast_story(theLog, wfs);

  delete_data_structures(theParameters);

  theLog << paragraph;

  return paragraph;
}

bool is_same_part_of_the_day(const std::string& phrase1, const std::string& phrase2)
{
  if ((phrase1 == AAMULLA_WORD && (phrase2 == AAMULLA_WORD || phrase2 == AAMUSTA_ALKAEN_PHRASE)) ||
      (phrase1 == AAMUPAIVALLA_WORD &&
       (phrase2 == AAMUPAIVALLA_WORD || phrase2 == AAMUPAIVASTA_ALKAEN_PHRASE)) ||
      (phrase1 == ILTAPAIVALLA_WORD &&
       (phrase2 == ILTAPAIVALLA_WORD || phrase2 == ILTAPAIVASTA_ALKAEN_PHRASE)) ||
      (phrase1 == ILLALLA_WORD && (phrase2 == ILLALLA_WORD || phrase2 == ILLASTA_ALKAEN_PHRASE)) ||
      (phrase1 == ILTAYOLLA_WORD &&
       (phrase2 == ILTAYOLLA_WORD || phrase2 == ILTAYOSTA_ALKAEN_PHRASE)) ||
      (phrase1 == KESKIYOLLA_WORD &&
       (phrase2 == KESKIYOLLA_WORD || phrase2 == KESKIYOSTA_ALKAEN_PHRASE)) ||
      (phrase1 == AAMUYOLLA_WORD &&
       (phrase2 == AAMUYOLLA_WORD || phrase2 == AAMUYOSTA_ALKAEN_PHRASE)) ||
      (phrase1 == YOLLA_WORD && phrase2 == YOLLA_WORD) ||
      (phrase1 == PAIVALLA_WORD && phrase2 == PAIVALLA_WORD))
    return true;

  return false;
}

// if the successive sentences contains the same time phrase, insert
// aluksi, myohemmin to the beginning of the sentence
void check_sentences(boost::shared_ptr<Glyph>& theSentence1, boost::shared_ptr<Glyph>& theSentence2)
{
  GlyphContainer& gc1 = static_cast<GlyphContainer&>(*theSentence1);
  GlyphContainer& gc2 = static_cast<GlyphContainer&>(*theSentence2);

  DebugTextFormatter dtf;

  if (gc1.size() < 2 || gc2.size() < 2) return;

  GlyphContainer::iterator iter1(gc1.begin());
  GlyphContainer::iterator iter2(gc2.begin());
  iter1++;
  iter2++;

  stringstream ss1;
  stringstream ss2;
  ss1 << dtf.format(**iter1);
  ss2 << dtf.format(**iter2);
  if (is_same_part_of_the_day(ss1.str(), ss2.str()))
  {
    gc1.push_front(Phrase("aluksi"));
    gc2.push_front(Phrase("myohemmin"));
  }
}

void get_sentences(const Glyph& glyphi, vector<boost::shared_ptr<Glyph> >& sentences)
{
  if (typeid(glyphi) == typeid(Sentence))
  {
    const GlyphContainer& gc = static_cast<const GlyphContainer&>(glyphi);
    if (gc.size() > 0) sentences.push_back(glyphi.clone());
  }
  else
  {
    const GlyphContainer& containeri = static_cast<const GlyphContainer&>(glyphi);
    for (GlyphContainer::const_iterator iter = containeri.begin(); iter != containeri.end(); ++iter)
      get_sentences(**iter, sentences);
  }
}

void analyze_sentences(Paragraph& paragraph)
{
  Paragraph paragraph_tmp;
  paragraph_tmp << paragraph;
  paragraph.clear();

  vector<boost::shared_ptr<Glyph> > sentences;
  get_sentences(paragraph_tmp, sentences);

  for (unsigned int i = 0; i < sentences.size(); i++)
  {
    if (i < sentences.size() - 1) check_sentences(sentences[i], sentences[i + 1]);
    Sentence& sen = static_cast<Sentence&>(*(sentences[i]));
    paragraph << sen;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate overview on weather
 *
 * \return The story
 *
 * \see page_weather_forecast
 *
 * \todo Much missing
 */
// ----------------------------------------------------------------------

Paragraph WeatherStory::forecast() const
{
  MessageLogger log("WeatherStory::forecast");

  using namespace PrecipitationPeriodTools;

  Paragraph paragraph;

  std::string areaName("");

  if (itsArea.isNamed())
  {
    areaName = itsArea.name();
  }

  WeatherArea areaOne(itsArea);
  WeatherArea areaTwo(itsArea);
  split_method splitMethod =
      check_area_splitting(itsVar, itsArea, itsPeriod, itsSources, areaOne, areaTwo, log);

  if (NO_SPLITTING != splitMethod)
  {
    Paragraph paragraphAreaOne;
    Paragraph paragraphAreaTwo;

    Sentence onAreaOneSentence;
    Sentence onAreaTwoSentence;
    onAreaOneSentence << ALUEEN_ETELAOSASSA_PHRASE << Delimiter(":");
    onAreaTwoSentence << ALUEEN_POHJOISOSASSA_PHRASE << Delimiter(":");

    paragraphAreaOne << onAreaOneSentence;
    paragraphAreaTwo << onAreaTwoSentence;

    log << areaName + (splitMethod == HORIZONTAL ? " - southern part" : " - western part") << endl;

    paragraphAreaOne << weather_forecast(
        areaOne, itsPeriod, itsSources, itsForecastTime, itsVar, log);

    log << areaName + (splitMethod == HORIZONTAL ? " - northern part" : " - eastern part") << endl;

    paragraphAreaTwo << weather_forecast(
        areaTwo, itsPeriod, itsSources, itsForecastTime, itsVar, log);

    paragraph << paragraphAreaOne << paragraphAreaTwo;
  }
  else
  {
    log << areaName << endl;

    paragraph << weather_forecast(itsArea, itsPeriod, itsSources, itsForecastTime, itsVar, log);
  }

  analyze_sentences(paragraph);

  return paragraph;
}

}  // namespace TextGen

// ======================================================================
