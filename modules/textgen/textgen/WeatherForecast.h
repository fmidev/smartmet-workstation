#ifndef TEXTGEN_WEATHER_FORECAST_H
#define TEXTGEN_WEATHER_FORECAST_H

#include "AreaTools.h"
#include <calculator/WeatherPeriod.h>
#include "MessageLogger.h"

namespace TextGen
{
class PrecipitationForecast;
class CloudinessForecast;
class FogForecast;
class ThunderForecast;
class Sentence;

#define EMPTY_STRING "empty_string"
#define SPACE_STRING " "
#define COMMA_PUNCTUATION_MARK ","
#define COLON_PUNCTUATION_MARK ":"
#define INTENSITY_PARAMETER "intensity"
#define PRECIPITATION_PARAMETER "precipitation"
#define IN_PLACES_PARAMETER "paikoin"
#define PLAIN_PRECIPITATION_PHRASE "precipitation phrase"

#define INLAND_PHRASE "sisamaassa"
#define COAST_PHRASE "rannikolla"
#define SADE_ALKAA_PHRASE "sade alkaa"
#define AAMULLA_WORD "aamulla"
#define AAMUPAIVALLA_WORD "aamupaivalla"
#define ILTAPAIVALLA_WORD "iltapaivalla"
#define PAIVALLA_WORD "paivalla"
#define ILLALLA_WORD "illalla"
#define ILTAYOLLA_WORD "iltayolla"
#define KESKIYOLLA_WORD "keskiyolla"
#define AAMUYOLLA_WORD "aamuyolla"
#define YOLLA_WORD "yolla"
#define TANAAN_WORD "tanaan"
#define HUOMENNA_WORD "huomenna"
#define ALUKSI_WORD "aluksi"
#define MYOHEMMIN_WORD "myohemmin"

#define SAA_ON_ENIMMAKSEEN_POUTAISTA_PHRASE "saa on enimmakseen poutaista"
#define SAA_ON_POUTAINEN_PHRASE "saa on poutainen"
#define HEIKKOJA_WORD "heikkoja"
#define SAKEITA_WORD "sakeita"
#define RUNSAITA_WORD "runsaita"
#define ENIMMAKSEEN_WORD "enimmakseen"
#define SAA_WORD "saa"
#define ON_WORD "on"
#define SAADAAN_WORD "saadaan"
#define HEIKKOA_WORD "heikkoa"
#define SAKEAA_WORD "sakeaa"
#define KOVAA_WORD "kovaa"
#define SADETTA_WORD "sadetta"
#define RUNSASTA_VESISADETTA_PHRASE "runsasta vesisadetta"
#define RUNSASTA_SADETTA_PHRASE "runsasta sadetta"
#define VAIHTELEE_PUOLIPILVISESTA_PILVISEEN_PHRASE "vaihtelee puolipilvisesta pilviseen"
#define VERRATTAIN_PILVINEN_PHRASE "verrattain pilvinen"
#define SELKEA_WORD "selkea"
#define PILVINEN_WORD "pilvinen"
#define SADETTA_WORD "sadetta"
#define MELKO_SELKEA_PHRASE "melko selkea"
#define PUOLIPILVINEN_WORD "puolipilvinen"
#define HEIKKOA_SADETTA_PHRASE "heikkoa sadetta"
#define HEIKKOA_VESISADETTA_PHRASE "heikkoa vesisadetta"
#define HEIKKOJA_VESIKUUROJA_PHRASE "heikkoja vesikuuroja"
#define VOIMAKKAITA_VESIKUUROJA_PHRASE "voimakkaita vesikuuroja"
#define PAIKOIN_WORD "paikoin"
#define MONIN_PAIKOIN_WORD "monin paikoin"
#define PILVISTYVAA_WORD "pilvistyvaa"
#define SELKENEVAA_WORD "selkenevaa"
#define SAA_POUTAANTUU_PHRASE "saa poutaantuu"
#define PAIKOIN_VOI_MYOS_UKKOSTAA_PHRASE "paikoin voi myos ukkostaa"
#define PAIKOIN_MYOS_UKKOSTAA_PHRASE "paikoin myos ukkostaa"
#define MAHDOLLISESTI_MYOS_UKKOSTAA_PHRASE "mahdollisesti myos ukkostaa"
#define MYOS_UKKOSTA_ESIINTYY_PHRASE "myos ukkosta esiintyy"
#define TODENNAKOISESTI_MYOS_UKKOSTAA_PHRASE "todennakoisesti myos ukkostaa"

#define HEIKKOJA_SADEKUUROJA_PHRASE "heikkoja sadekuuroja"
#define VOIMAKKAITA_SADEKUUROJA_PHRASE "voimakkaita sadekuuroja"
#define SADEKUUROJA_WORD "sadekuuroja"
#define LUMIKUUROJA_WORD "lumikuuroja"
#define SADEKUUROJA_WORD "sadekuuroja"
#define RANTAKUUROJA_WORD "rantakuuroja"
#define VESIKUUROJA_WORD "vesikuuroja"
#define RANTASADETTA_WORD "rantasadetta"
#define VESISADETTA_WORD "vesisadetta"
#define LUMISADETTA_WORD "lumisadetta"
#define TIHKUSADETTA_WORD "tihkusadetta"

#define RANTA_TAI_LUMISADETTA_PHRASE "ranta- tai lumisadetta"
#define LUMI_TAI_RANTASADETTA_PHRASE "lumi- tai rantasadetta"
#define LUMI_TAI_VESISADETTA_PHRASE "lumi- tai vesisadetta"
#define VESI_TAI_LUMISADETTA_PHRASE "vesi- tai lumisadetta"
#define RANTA_TAI_VESISADETTA_PHRASE "ranta- tai vesisadetta"
#define VESI_TAI_RANTASADETTA_PHRASE "vesi- tai rantasadetta"
#define RANTA_TAI_VESIKUUROJA_PHRASE "ranta- tai vesikuuroja"
#define VESI_TAI_RANTAKUUROJA_PHRASE "vesi- tai rantakuuroja"
#define VESI_TAI_LUMIKUUROJA_PHRASE "vesi- tai lumikuuroja"
#define LUMI_TAI_VESIKUUROJA_PHRASE "lumi- tai vesikuuroja"

#define JA_WORD "ja"
#define JAATAVAA_VESISADETTA_PHRASE "jaatavaa vesisadetta"
#define JOKA_VOI_OLLA_JAATAVAA_PHRASE "joka voi olla jaatavaa"
#define JOTKA_VOIVAT_OLLA_JAATAVIA_PHRASE "jotka voivat olla jaatavia"
#define YKSITTAISET_SADEKUUROT_MAHDOLLISIA_PHRASE "yksittaiset sadekuurot mahdollisia"
#define YKSITTAISET_VESIKUUROT_MAHDOLLISIA_PHRASE "yksittaiset vesikuurot mahdollisia"
#define YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_PHRASE "yksittaiset rantakuurot mahdollisia"
#define YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_PHRASE "yksittaiset lumikuurot mahdollisia"
#define YKSITTAISET_VESI_RANTA_KUUROT_MAHDOLLISIA_PHRASE \
  "yksittaiset vesi- tai rantakuurot mahdollisia"
#define YKSITTAISET_RANTA_VESI_KUUROT_MAHDOLLISIA_PHRASE \
  "yksittaiset ranta- tai vesikuurot mahdollisia"
#define YKSITTAISET_LUMI_RANTA_KUUROT_MAHDOLLISIA_PHRASE \
  "yksittaiset lumi- tai rantakuurot mahdollisia"
#define YKSITTAISET_VESI_LUMI_KUUROT_MAHDOLLISIA_PHRASE \
  "yksittaiset vesi- tai lumikuurot mahdollisia"
#define JOKA_MUUTTUU_LUMISATEEKSI_PHRASE "joka muuttuu lumisateeksi"
#define JOKA_MUUTTUU_VESISATEEKSI_PHRASE "joka muuttuu vesisateeksi"
#define JOKA_MUUTTUU_TIHKUSATEEKSI_PHRASE "joka muuttuu tihkusateeksi"
#define JOKA_MUUTTUU_RANTASATEEKSI_PHRASE "joka muuttuu rantasateeksi"

#define PIKKUPAKKASTA_PHRASE "pikkupakkasta"
#define SUMUA_WORD "sumua"
#define JOKA_VOI_OLLA_SAKEAA_PHRASE "joka voi olla sakeaa"

#define ALUEEN_POHJOISOSASSA_PHRASE "alueen pohjoisosassa"
#define ALUEEN_ETELAOSASSA_PHRASE "alueen etelaosassa"
#define ALUEEN_ITAOSASSA_PHRASE "alueen itaosassa"
#define ALUEEN_LANSIOSASSA_PHRASE "alueen lansiosassa"
#define ALUEEN_KOILLISOSASSA_PHRASE "alueen koillisosassa"
#define ALUEEN_KAAKKOISOSASSA_PHRASE "alueen kaakkoisosassa"
#define ALUEEN_LOUNAISOSASSA_PHRASE "alueen lounaisosassa"
#define ALUEEN_LUOTEISOSASSA_PHRASE "alueen luoteisosassa"

#define AAMUSTA_ALKAEN_PHRASE "aamusta alkaen"
#define AAMUPAIVASTA_ALKAEN_PHRASE "aamupaivasta alkaen"
#define ILTAPAIVASTA_ALKAEN_PHRASE "iltapaivasta alkaen"
#define ILLASTA_ALKAEN_PHRASE "illasta alkaen"
#define ILTAYOSTA_ALKAEN_PHRASE "iltayosta alkaen"
#define KESKIYOSTA_ALKAEN_PHRASE "keskiyosta alkaen"
#define AAMUYOSTA_ALKAEN_PHRASE "aamuyosta alkaen"
#define AAMUUN_ASTI_PHRASE "aamuun asti"
#define AAMUPAIVAAN_ASTI_PHRASE "aamupaivaan asti"
#define ILTAPAIVAAN_ASTI_PHRASE "iltapaivaan asti"
#define ILTAAN_ASTI_PHRASE "iltaan asti"
#define ILTAYOHON_ASTI_PHRASE "iltayohon asti"
#define KESKIYOHON_ASTI_PHRASE "keskiyohon asti"
#define AAMUYOHON_ASTI_PHRASE "aamuyohon asti"

#define AAMULLA_JA_AAMUPAIVALLA_PHRASE "aamulla ja aamupaivalla"
#define ILTAPAIVALLA_JA_ILLALLA_PHRASE "iltapaivalla ja illalla"
#define ILLALLA_JA_ILTAYOLLA_PHRASE "illalla ja iltayolla"
#define ILTAYOLLA_JA_KESKIYOLLA_PHRASE "iltayolla ja keskiyolla"
#define KESKIYOLLA_JA_AAMUYOLLA_PHRASE "keskiyolla ja aamuyolla"
#define AAMUYOLLA_JA_AAMULLA_PHRASE "aamuyolla ja aamulla"

#define POHJOISESTA_ALKAEN_PHRASE "pohjoisesta alkaen"
#define ETELASTA_ALKAEN_PHRASE "etelasta alkaen"
#define IDASTA_ALKAEN_PHRASE "idasta alkaen"
#define LANNESTA_ALKAEN_PHRASE "lannesta alkaen"
#define KOILLISESTA_ALKAEN_PHRASE "koillisesta alkaen"
#define KAAKOSTA_ALKAEN_PHRASE "kaakosta alkaen"
#define LOUNAASTA_ALKAEN_PHRASE "lounaasta alkaen"
#define LUOTEESTA_ALKAEN_PHRASE "luoteesta alkaen"

#define SADEALUE_WORD "sadealue"
#define SAAPUU_WORD "saapuu"
#define POHJOISESTA_WORD "pohjoisesta"
#define ETELASTA_WORD "etelasta"
#define IDASTA_WORD "idasta"
#define LANNESTA_WORD "lannesta"
#define KOILLISESTA_WORD "koillisesta"
#define KAAKOSTA_WORD "kaakosta"
#define LOUNAASTA_WORD "lounaasta"
#define LUOTEESTA_WORD "luoteesta"

#define HORIZONTAL_SPLIT_KEY "horizontal"
#define VERTICAL_SPLIT_KEY "vertical"

#define SELKEA_UPPER_LIMIT 9.9
#define MELKEIN_SELKEA_LOWER_LIMIT 9.9
#define MELKEIN_SELKEA_UPPER_LIMIT 35.0
#define PUOLIPILVISTA_LOWER_LIMIT 35.0
#define PUOLIPILVISTA_UPPER_LIMIT 65.0
#define VERRATTAIN_PILVISTA_LOWER_LIMIT 65.0
#define VERRATTAIN_PILVISTA_UPPER_LIMIT 85.0
#define PILVISTA_LOWER_LIMIT 85.0
#define PILVISTYVAA_LOWER_LIMIT 20.0
#define PILVISTYVAA_UPPER_LIMIT 65.0

#define TAYSIN_PILVETON_LIMIT 5.0
#define HYVIN_PILVINEN_LIMIT 70.0

#define DRY_WEATHER_LIMIT_WATER 0.04
#define DRY_WEATHER_LIMIT_DRIZZLE 0.02
#define DRY_WEATHER_LIMIT_SLEET 0.02
#define DRY_WEATHER_LIMIT_SNOW 0.02
#define WEAK_PRECIPITATION_LIMIT_WATER 0.4
#define WEAK_PRECIPITATION_LIMIT_SLEET 0.4
#define WEAK_PRECIPITATION_LIMIT_SNOW 0.4
#define HEAVY_PRECIPITATION_LIMIT_WATER 2.0
#define HEAVY_PRECIPITATION_LIMIT_SLEET 1.7
#define HEAVY_PRECIPITATION_LIMIT_SNOW 1.5
#define RAINSTORM_LIMIT 7.0
#define MOSTLY_DRY_WEATHER_LIMIT 10.0
#define IN_SOME_PLACES_LOWER_LIMIT_FOG 20.0
#define IN_SOME_PLACES_LOWER_LIMIT 10.0
#define IN_SOME_PLACES_UPPER_LIMIT 50.0
#define IN_MANY_PLACES_LOWER_LIMIT 50.0
#define IN_MANY_PLACES_UPPER_LIMIT 90.0
#define FREEZING_RAIN_LIMIT 10
#define PRECIPITATION_FORM_REPORTING_LIMIT 10.0  // at least 10% must be of the type to be mentioned

#define SMALL_PROBABILITY_FOR_THUNDER_LOWER_LIMIT 0.1
#define SMALL_PROBABILITY_FOR_THUNDER_UPPER_LIMIT 15.0
#define POSSIBLY_THUNDER_LOWER_LIMIT 15.0
#define POSSIBLY_THUNDER_UPPER_LIMIT 35.0
#define OCCASIONALLY_THUNDER_LOWER_LIMIT 35.0
#define OCCASIONALLY_THUNDER_UPPER_LIMIT 110.0
#define MAJORITY_LIMIT 50.0
#define SEPARATE_COASTAL_AREA_PERCENTAGE 5.0

#define TREND_CHANGE_COEFFICIENT_TRESHOLD 0.65  // pearson coefficient
#define PEARSON_CO_FORM_TRANSFORM 0.65

#define KESKIYO_START 0
#define KESKIYO_END 3
#define AAMUYO_START 2
#define AAMUYO_END 6
#define AAMU_START 6
#define AAMU_END 9
#define AAMUPAIVA_START 9
#define AAMUPAIVA_END 12
#define PAIVA_START 10
#define PAIVA_END 18
#define ILTAPAIVA_START 12
#define ILTAPAIVA_END 18
#define ILTA_START 17
#define ILTA_END 22
#define ILTAYO_START 21
#define ILTAYO_END 23
#define YO_START 22
#define YO_END 6

enum weather_result_data_id
{
  CLOUDINESS_DATA = 0x1,
  CLOUDINESS_NORTHEAST_SHARE_DATA,
  CLOUDINESS_SOUTHEAST_SHARE_DATA,
  CLOUDINESS_SOUTHWEST_SHARE_DATA,
  CLOUDINESS_NORTHWEST_SHARE_DATA,
  PRECIPITATION_MAX_DATA,
  PRECIPITATION_MEAN_DATA,
  PRECIPITATION_EXTENT_DATA,
  PRECIPITATION_TYPE_DATA,
  PRECIPITATION_FORM_WATER_DATA,
  PRECIPITATION_FORM_DRIZZLE_DATA,
  PRECIPITATION_FORM_SLEET_DATA,
  PRECIPITATION_FORM_SNOW_DATA,
  PRECIPITATION_FORM_FREEZING_DATA,
  PRECIPITATION_NORTHEAST_SHARE_DATA,
  PRECIPITATION_SOUTHEAST_SHARE_DATA,
  PRECIPITATION_SOUTHWEST_SHARE_DATA,
  PRECIPITATION_NORTHWEST_SHARE_DATA,
  PRECIPITATION_POINT_DATA,
  THUNDER_PROBABILITY_DATA,
  THUNDER_EXTENT_DATA,
  THUNDER_NORTHEAST_SHARE_DATA,
  THUNDER_SOUTHEAST_SHARE_DATA,
  THUNDER_SOUTHWEST_SHARE_DATA,
  THUNDER_NORTHWEST_SHARE_DATA,
  FOG_INTENSITY_MODERATE_DATA,
  FOG_INTENSITY_DENSE_DATA,
  FOG_NORTHEAST_SHARE_DATA,
  FOG_SOUTHEAST_SHARE_DATA,
  FOG_SOUTHWEST_SHARE_DATA,
  FOG_NORTHWEST_SHARE_DATA,
  UNDEFINED_DATA_ID
};

enum precipitation_form_id
{
  WATER_FORM = 0x1,
  DRIZZLE_FORM = 0x2,
  SLEET_FORM = 0x4,
  SNOW_FORM = 0x8,
  FREEZING_FORM = 0x10,
  WATER_DRIZZLE_FORM = 0x3,
  WATER_SLEET_FORM = 0x5,
  WATER_SNOW_FORM = 0x9,
  WATER_FREEZING_FORM = 0x11,
  WATER_DRIZZLE_SLEET_FORM = 0x7,
  WATER_DRIZZLE_SNOW_FORM = 0xB,
  WATER_DRIZZLE_FREEZING_FORM = 0x13,
  WATER_SLEET_SNOW_FORM = 0xD,
  WATER_SLEET_FREEZING_FORM = 0x15,
  WATER_SNOW_FREEZING_FORM = 0x19,
  DRIZZLE_SLEET_FORM = 0x6,
  DRIZZLE_SNOW_FORM = 0xA,
  DRIZZLE_FREEZING_FORM = 0x12,
  DRIZZLE_SLEET_SNOW_FORM = 0xE,
  DRIZZLE_SLEET_FREEZING_FORM = 0x16,
  DRIZZLE_SNOW_FREEZING_FORM = 0x1A,
  SLEET_SNOW_FORM = 0xC,
  SLEET_FREEZING_FORM = 0x14,
  SLEET_SNOW_FREEZING_FORM = 0x1C,
  SNOW_FREEZING_FORM = 0x18,
  MISSING_PRECIPITATION_FORM = 0x0
};

enum precipitation_form_transformation_id
{
  WATER_TO_SNOW,
  WATER_TO_DRIZZLE,
  WATER_TO_SLEET,
  SNOW_TO_WATER,
  SNOW_TO_DRIZZLE,
  SNOW_TO_SLEET,
  DRIZZLE_TO_WATER,
  DRIZZLE_TO_SNOW,
  DRIZZLE_TO_SLEET,
  SLEET_TO_WATER,
  SLEET_TO_DRIZZLE,
  SLEET_TO_SNOW,
  NO_FORM_TRANSFORMATION
};

enum precipitation_intesity_id
{
  DRY_WEATHER,
  WEAK_PRECIPITATION,
  MODERATE_PRECIPITATION,
  HEAVY_PRECIPITATION,
  MISSING_INTENSITY_ID
};

enum precipitation_category_id
{
  WEAK_WATER_PRECIPITATION,
  MODERATE_WATER_PRECIPITATION,
  HEAVY_WATER_PRECIPITATION,
  WEAK_SLEET_PRECIPITATION,
  MODERATE_SLEET_PRECIPITATION,
  HEAVY_SLEET_PRECIPITATION,
  WEAK_SNOW_PRECIPITATION,
  MODERATE_SNOW_PRECIPITATION,
  HEAVY_SNOW_PRECIPITATION,
  DRY_WEATHER_CATEGORY
};

enum precipitation_traverse_id
{
  FROM_SOUTH_TO_NORTH,
  FROM_NORTH_TO_SOUTH,
  FROM_EAST_TO_WEST,
  FROM_WEST_TO_EAST,
  FROM_NORTHEAST_TO_SOUTHWEST,
  FROM_SOUTHWEST_TO_NORTHEAST,
  FROM_NORTHWEST_TO_SOUTHEAST,
  FROM_SOUTHEAST_TO_NORTHWEST,
  MISSING_TRAVERSE_ID
};

enum cloudiness_id
{
  SELKEA,
  MELKO_SELKEA,
  PUOLIPILVINEN,
  VERRATTAIN_PILVINEN,
  PILVINEN,
  PUOLIPILVINEN_JA_PILVINEN,
  MISSING_CLOUDINESS_ID
};

enum thunder_probability_id
{
  SMALL_PROBABILITY_FOR_THUNDER,
  POSSIBLY_THUNDER,
  OCCASIONALLY_THUNDER,
  NO_THUNDER
};

enum part_of_the_day_id
{
  AAMU,               // 06-09
  AAMUPAIVA,          // 09-12
  ILTAPAIVA,          // 12-18
  ILTA,               // 17-22
  ILTAYO,             // 21-24
  KESKIYO,            // 00-03
  AAMUYO,             // 02-06
  PAIVA,              // 10-18
  YO,                 // 22-06
  AAMU_JA_AAMUPAIVA,  // 06-12
  ILTAPAIVA_JA_ILTA,  // 13-21
  ILTA_JA_ILTAYO,     // 18-00
  ILTAYO_JA_KESKIYO,  // 22-03
  KESKIYO_JA_AAMUYO,  // 00-06
  AAMUYO_JA_AAMU,     // 03-09
  MISSING_PART_OF_THE_DAY_ID
};

enum weather_event_id
{
  PILVISTYY,
  SELKENEE,
  POUTAANTUU,                    // >= 6h
  POUTAANTUU_WHEN_EXTENT_SMALL,  // >= 6h, but extent is not large
  SADE_ALKAA,                    // >= 6h
  // TYPE_CHANGES,
  MISSING_WEATHER_EVENT
};

enum story_part_id
{
  PRECIPITATION_STORY_PART = 0x1,
  CLOUDINESS_STORY_PART = 0x2,
  GETTING_CLOUDY_STORY_PART = 0x4,
  CLEARING_UP_STORY_PART = 0x8,
  PRECIPITATION_TYPE_CHANGE_STORY_PART = 0x10,
  MISSING_STORY_PART = 0x0
};

enum stat_func_id
{
  MIN,
  MAX,
  MEAN
};

enum area_specific_sentence_id
{
  ALUEEN_POHJOISOSASSA,
  ALUEEN_ETELAOSASSA,
  ALUEEN_ITAOSASSA,
  ALUEEN_LANSIOSASSA,
  ENIMMAKSEEN_ALUEEN_POHJOISOSASSA,
  ENIMMAKSEEN_ALUEEN_ETELAOSASSA,
  ENIMMAKSEEN_ALUEEN_ITAOSASSA,
  ENIMMAKSEEN_ALUEEN_LANSIOSASSA,
  MISSING_AREA_SPECIFIC_SENTENCE_ID
};

enum precipitation_type
{
  CONTINUOUS = 1,
  SHOWERS = 2,
  MISSING_PRECIPITATION_TYPE
};

enum split_method
{
  HORIZONTAL,
  VERTICAL,
  NO_SPLITTING
};

enum time_phrase_format
{
  FROM_FORMAT,
  TILL_FORMAT,
  AT_FORMAT
};

class WeatherResultDataItem;
class PrecipitationDataItemData;
class FogIntensityDataItem;
class PrecipitationDataItem;
class CloudinessDataItem;
class ThunderDataItem;

typedef std::vector<WeatherResultDataItem*> weather_result_data_item_vector;
typedef std::pair<TextGenPosixTime, weather_event_id> timestamp_weather_event_id_pair;
typedef std::vector<timestamp_weather_event_id_pair> weather_event_id_vector;
typedef std::vector<PrecipitationDataItemData*> precipitation_data_vector;
typedef std::vector<FogIntensityDataItem*> fog_data_vector;
typedef std::map<int, PrecipitationDataItem*> precipitation_data_item_container;
typedef std::vector<precipitation_data_item_container*> precipitation_data_container;
typedef std::map<int, weather_result_data_item_vector*> weather_forecast_result_container;
typedef std::map<int, weather_forecast_result_container*> weather_forecast_data_container;
typedef std::map<int, CloudinessDataItem*> cloudiness_data_item_container;
typedef std::vector<cloudiness_data_item_container*> cloudiness_data_container;
typedef std::map<int, ThunderDataItem*> thunder_data_item_container;
typedef std::vector<thunder_data_item_container*> thunder_data_container;
typedef std::map<int, FogIntensityDataItem*> fog_data_item_container;
typedef std::vector<fog_data_item_container*> fog_data_container;
typedef std::vector<NFmiPoint*> location_coordinate_vector;
typedef std::pair<WeatherPeriod, unsigned int> weather_period_story_part_id_pair;
typedef std::vector<weather_period_story_part_id_pair> story_part_vector;

struct wf_story_params
{
  wf_story_params(const std::string& variable,
                  const WeatherArea& weatherArea,
                  const WeatherPeriod& dataPeriod,
                  const WeatherPeriod& forecastPeriod,
                  const TextGenPosixTime& forecastTime,
                  const AnalysisSources& analysisSources,
                  MessageLogger& log)
      : theVariable(variable),
        theArea(weatherArea),
        theDataPeriod(dataPeriod),
        theForecastPeriod(forecastPeriod),
        theForecastTime(forecastTime),
        theSources(analysisSources),
        theLog(log),
        theCoastalAndInlandTogetherFlag(false),
        thePrecipitationForecast(0),
        theCloudinessForecast(0),
        theFogForecast(0),
        theThunderForecast(0),
        theHourPeriodCount(0),
        theOriginalPeriodCount(0),
        theForecastArea(TextGen::AreaTools::NO_AREA)
  {
  }

  const std::string& theVariable;
  const WeatherArea& theArea;
  const WeatherPeriod theDataPeriod;
  const WeatherPeriod theForecastPeriod;
  const TextGenPosixTime theForecastTime;
  const AnalysisSources& theSources;
  MessageLogger& theLog;
  bool theCoastalAndInlandTogetherFlag;
  PrecipitationForecast* thePrecipitationForecast;
  CloudinessForecast* theCloudinessForecast;
  FogForecast* theFogForecast;
  ThunderForecast* theThunderForecast;
  unsigned int theHourPeriodCount;
  unsigned int theOriginalPeriodCount;
  unsigned short theForecastArea;
  float theDryWeatherLimitWater;
  float theDryWeatherLimitDrizzle;
  float theDryWeatherLimitSleet;
  float theDryWeatherLimitSnow;
  float theWeakPrecipitationLimitWater;
  float theWeakPrecipitationLimitSleet;
  float theWeakPrecipitationLimitSnow;
  float theHeavyPrecipitationLimitWater;
  float theHeavyPrecipitationLimitSleet;
  float theHeavyPrecipitationLimitSnow;
  float thePrecipitationStormLimit;
  float theMostlyDryWeatherLimit;
  float theInSomePlacesLowerLimit;
  float theInSomePlacesUpperLimit;
  float theInManyPlacesLowerLimit;
  float theInManyPlacesUpperLimit;
  float theFreezingPrecipitationLimit;
  float theClearSkyUpperLimit;
  float theAlmostClearSkyUpperLimit;
  float thePartlyCloudySkyUpperLimit;
  float theMostlyCloudySkyUpperLimit;
  weather_forecast_data_container theCompleteData;
  cloudiness_data_container theCloudinessData;
  precipitation_data_container thePrecipitationData;
  thunder_data_container theThunderData;
  fog_data_container theFogData;
};

Sentence get_large_time_phrase(const WeatherPeriod& theWeatherPeriod,
                               bool theSpecifyDayFlag,
                               std::string& thePhraseString,
                               part_of_the_day_id& thePartOfTheDay);
std::string get_narrow_time_phrase(const WeatherPeriod& theWeatherPeriod,
                                   const std::string& theVar,
                                   part_of_the_day_id& thePartOfTheDay,
                                   bool theAlkaenPhrase = false);
void get_precipitation_limit_value(const wf_story_params& theParameters,
                                   unsigned int thePrecipitationForm,
                                   precipitation_intesity_id theIntensityId,
                                   float& theLowerLimit,
                                   float& theUpperLimit);
void get_dry_and_weak_precipitation_limit(const wf_story_params& theParameters,
                                          unsigned int thePrecipitationForm,
                                          float& theDryWeatherLimit,
                                          float& theWeakPrecipitationLimit);
const char* weather_event_string(weather_event_id theWeatherEventId);
const char* precipitation_form_string(precipitation_form_id thePrecipitationForm);
const char* precipitation_type_string(precipitation_type thePrecipitationType);
const char* precipitation_traverse_string(precipitation_traverse_id thePrecipitationTraverseId);
const char* part_of_the_day_string(part_of_the_day_id thePartOfTheDayId);
const char* story_part_id_string(story_part_id theStoryPartId);

bool is_same_part_of_day(const WeatherPeriod& thePeriod1, const WeatherPeriod& thePeriod2);
void get_part_of_the_day(part_of_the_day_id thePartOfTheDayId, int& theStartHour, int& theEndHour);
part_of_the_day_id get_part_of_the_day_id_narrow(const WeatherPeriod& thePeriod,
                                                 bool ignoreKeskiyo = false);
part_of_the_day_id get_part_of_the_day_id_large(const WeatherPeriod& thePeriod);
part_of_the_day_id get_adjusted_part_of_the_day_id(const WeatherPeriod& theWeatherPeriod,
                                                   bool theAlkaenPhrase = false);
part_of_the_day_id get_part_of_the_day_id(const TextGenPosixTime& theTimestamp,
                                          bool ignoreKeskiyo = false);
bool is_inside(const WeatherPeriod& theWeatherPeriod1, const WeatherPeriod& theWeatherPeriod2);
bool is_inside(const WeatherPeriod& theWeatherPeriod, part_of_the_day_id thePartOfTheDayId);
bool is_inside(const TextGenPosixTime& theTimeStamp, part_of_the_day_id thePartOfTheDayId);
bool is_inside(const TextGenPosixTime& theTimeStamp, const WeatherPeriod& theWeatherPeriod);
bool same_period(const WeatherPeriod& theWeatherPeriod1, const WeatherPeriod& theWeatherPeriod2);
WeatherPeriod intersecting_period(const WeatherPeriod& theWeatherPeriod1,
                                  const WeatherPeriod& theWeatherPeriod2);

Sentence get_direction_phrase(const AreaTools::direction_id& theDirectionId,
                              bool theAlkaenPhrase = false);
Sentence get_time_phrase_large(const WeatherPeriod& theWeatherPeriod,
                               bool theSpecifyDayFlag,
                               const std::string& theVar,
                               std::string& thePhraseString,
                               bool theAlkaenPhrase,
                               part_of_the_day_id& thePartOfTheDay);
std::string get_time_phrase(const TextGenPosixTime& theTimestamp,
                            const std::string& theVar,
                            bool theAlkaenPhrase = false);
std::string get_time_phrase_from_id(part_of_the_day_id thePartOfTheDayId,
                                    const std::string& theVar,
                                    bool theAlkaenPhrase = false);
Sentence get_today_phrase(const TextGenPosixTime& theEventTimestamp,
                          const std::string& theVariable,
                          const WeatherArea& theArea,
                          const WeatherPeriod thePeriod,
                          const TextGenPosixTime& theForecastTime);
std::string parse_time_phrase(short theWeekday,
                              bool theSpecifyDayFlag,
                              const std::string& theTimePhrase);
precipitation_form_id get_complete_precipitation_form(const std::string& theVariable,
                                                      float thePrecipitationFormWater,
                                                      float thePrecipitationFormDrizzle,
                                                      float thePrecipitationFormSleet,
                                                      float thePrecipitationFormSnow,
                                                      float thePrecipitationFormFreezing);
void get_sub_time_series(const WeatherPeriod& thePeriod,
                         const weather_result_data_item_vector& theSourceVector,
                         weather_result_data_item_vector& theDestinationVector);
void get_sub_time_series(part_of_the_day_id thePartOfTheDay,
                         const weather_result_data_item_vector& theSourceVector,
                         weather_result_data_item_vector& theDestinationVector);
float get_mean(const weather_result_data_item_vector& theTimeSeries,
               int theStartIndex = 0,
               int theEndIndex = 0);
float get_standard_deviation(const weather_result_data_item_vector& theTimeSeries);
void get_min_max(const weather_result_data_item_vector& theTimeSeries,
                 float& theMin,
                 float& theMax);
double get_pearson_coefficient(const weather_result_data_item_vector& theTimeSeries,
                               unsigned int theStartIndex,
                               unsigned int theEndIndex,
                               bool theUseErrorValueFlag = false);
void print_out_weather_event_vector(std::ostream& theOutput,
                                    const weather_event_id_vector& theWeatherEventVector);
Sentence area_specific_sentence(float north,
                                float south,
                                float east,
                                float west,
                                float northEast,
                                float southEast,
                                float southWest,
                                float northWest,
                                bool mostlyFlag = true);
int get_today_vector(const std::string& theVariable,
                     const WeatherArea& theArea,
                     const WeatherPeriod& thePeriod,
                     const TextGenPosixTime& theForecastTime,
                     std::vector<Sentence*>& theTodayVector);
area_specific_sentence_id get_area_specific_sentence_id(float north,
                                                        float south,
                                                        float east,
                                                        float west,
                                                        float northEast,
                                                        float southEast,
                                                        float southWest,
                                                        float northWest,
                                                        bool mostlyFlag = true);
int get_period_length(const WeatherPeriod& thePeriod);
float get_area_percentage(const std::string& theVar,
                          const WeatherArea& theArea,
                          const TextGen::WeatherArea::Type& theType,
                          const AnalysisSources& theSources,
                          const WeatherPeriod& thePeriod);
std::string parse_weekday_phrase(short weekday, const std::string& part_of_the_day);
Sentence parse_weekday_phrase(short weekday, const Sentence& part_of_the_day);
WeatherPeriod get_intersection_period(const WeatherPeriod& thePeriod1,
                                      const WeatherPeriod& thePeriod2,
                                      bool& theIntersectionPeriodFound);

split_method split_the_area(const std::string& theVar,
                            const TextGen::WeatherArea& theArea,
                            const TextGen::WeatherPeriod& thePeriod,
                            const TextGen::AnalysisSources& theSources,
                            double& theDivisionLine,
                            MessageLogger& theLog);

bool test_temperature_split_criterion(const std::string& theVar,
                                      bool morningTemperature,
                                      const TextGen::WeatherArea& theAreaOne,
                                      const TextGen::WeatherArea& theAreaTwo,
                                      const TextGen::WeatherPeriod& thePeriod,
                                      const TextGen::AnalysisSources& theSources,
                                      MessageLogger& theLog);
split_method check_area_splitting(const std::string& theVar,
                                  const TextGen::WeatherArea& theArea,
                                  const TextGen::WeatherPeriod& thePeriod,
                                  const TextGen::AnalysisSources& theSources,
                                  TextGen::WeatherArea& theFirstArea,
                                  TextGen::WeatherArea& theSecondArea,
                                  MessageLogger& theLog);
bool fit_into_narrow_day_part(const WeatherPeriod& thePeriod);
bool fit_into_large_day_part(const WeatherPeriod& thePeriod);

struct WeatherResultDataItem
{
  WeatherResultDataItem(const WeatherPeriod& period,
                        const WeatherResult& result,
                        part_of_the_day_id partOfTheDay)
      : thePeriod(period), theResult(result), thePartOfTheDay(partOfTheDay)
  {
  }

  WeatherPeriod thePeriod;
  WeatherResult theResult;
  part_of_the_day_id thePartOfTheDay;
};

struct CloudinessDataItemData
{
  CloudinessDataItemData(cloudiness_id id,
                         float min,
                         float mean,
                         float max,
                         float standardDeviation,
                         weather_event_id weatherEventId,
                         float pearsonCoefficient)
      : theId(id),
        theMin(min),
        theMean(mean),
        theMax(max),
        theStandardDeviation(standardDeviation),
        theWeatherEventId(weatherEventId),
        thePearsonCoefficient(pearsonCoefficient)
  {
  }

  cloudiness_id theId;
  float theMin;
  float theMean;
  float theMax;
  float theStandardDeviation;
  weather_event_id theWeatherEventId;
  float thePearsonCoefficient;
};

struct CloudinessDataItem
{
  CloudinessDataItem(const CloudinessDataItemData* coastalData,
                     const CloudinessDataItemData* inlandData,
                     const CloudinessDataItemData* fullData)
      : theCoastalData(coastalData), theInlandData(inlandData), theFullData(fullData)
  {
  }

  const CloudinessDataItemData* theCoastalData;
  const CloudinessDataItemData* theInlandData;
  const CloudinessDataItemData* theFullData;

  ~CloudinessDataItem()
  {
    if (theCoastalData) delete theCoastalData;
    if (theInlandData) delete theInlandData;
    if (theFullData) delete theFullData;
  }
};

struct ThunderDataItem
{
  ThunderDataItem(float minProbability,
                  float meanProbability,
                  float maxProbability,
                  float standardDeviationProbability)
      : theMinProbability(minProbability),
        theMeanProbability(meanProbability),
        theMaxProbability(maxProbability),
        theStandardDeviationProbability(standardDeviationProbability)
  {
  }

  float theMinProbability;
  float theMeanProbability;
  float theMaxProbability;
  float theStandardDeviationProbability;
};

struct FogIntensityDataItem
{
  FogIntensityDataItem(float moderateFogExtent, float denseFogExtent)
      : theModerateFogExtent(moderateFogExtent), theDenseFogExtent(denseFogExtent)
  {
  }

  float theModerateFogExtent;
  float theDenseFogExtent;
};

struct PrecipitationDataItemData
{
  PrecipitationDataItemData(const wf_story_params& theParameters,
                            precipitation_form_id precipitationForm,
                            float precipitationIntensity,
                            float precipitationMaxIntensity,
                            float precipitationExtent,
                            float precipitationFormWater,
                            float precipitationFormDrizzle,
                            float precipitationFormSleet,
                            float precipitationFormSnow,
                            float precipitationFormFreezing,
                            float precipitationTypeShowers,
                            weather_event_id weatherEventId,
                            float pearsonCoefficient,
                            const TextGenPosixTime& observationTime)
      : thePrecipitationForm(precipitationForm),
        thePrecipitationIntensity(precipitationIntensity),
        thePrecipitationMaxIntensity(precipitationMaxIntensity),
        thePrecipitationExtent(precipitationExtent),
        thePrecipitationFormWater(precipitationFormWater),
        thePrecipitationFormDrizzle(precipitationFormDrizzle),
        thePrecipitationFormSleet(precipitationFormSleet),
        thePrecipitationFormSnow(precipitationFormSnow),
        thePrecipitationFormFreezing(precipitationFormFreezing),
        thePrecipitationTypeShowers(precipitationTypeShowers),
        theWeatherEventId(weatherEventId),
        thePearsonCoefficient(pearsonCoefficient),
        theObservationTime(observationTime),
        thePrecipitationPercentageNorthEast(0.0),
        thePrecipitationPercentageSouthEast(0.0),
        thePrecipitationPercentageSouthWest(0.0),
        thePrecipitationPercentageNorthWest(0.0),
        thePrecipitationPoint(0.0, 0.0),
        thePrecipitationType(MISSING_PRECIPITATION_TYPE),
        thePrecipitationIntensityId(MISSING_INTENSITY_ID)
  {
    if (precipitationTypeShowers != kFloatMissing)
      thePrecipitationType = precipitationTypeShowers >= MAJORITY_LIMIT ? SHOWERS : CONTINUOUS;

    if (precipitationIntensity == kFloatMissing)
    {
      thePrecipitationIntensityId = DRY_WEATHER;
    }
    else
    {
      float lowerLimit(0.0);
      float dryUpperLimit(0.0);
      float weakUpperLimit(0.0);
      float moderateUpperLimit(0.0);
      get_precipitation_limit_value(
          theParameters, precipitationForm, DRY_WEATHER, lowerLimit, dryUpperLimit);
      get_precipitation_limit_value(
          theParameters, precipitationForm, WEAK_PRECIPITATION, lowerLimit, weakUpperLimit);
      get_precipitation_limit_value(
          theParameters, precipitationForm, MODERATE_PRECIPITATION, lowerLimit, moderateUpperLimit);

      if (precipitationIntensity < dryUpperLimit)
        thePrecipitationIntensityId = DRY_WEATHER;
      else if (precipitationIntensity < weakUpperLimit)
        thePrecipitationIntensityId = WEAK_PRECIPITATION;
      else if (precipitationIntensity < moderateUpperLimit)
        thePrecipitationIntensityId = MODERATE_PRECIPITATION;
      else
        thePrecipitationIntensityId = HEAVY_PRECIPITATION;
    }
  }

  precipitation_form_id thePrecipitationForm;
  float thePrecipitationIntensity;
  float thePrecipitationMaxIntensity;
  float thePrecipitationExtent;
  float thePrecipitationFormWater;
  float thePrecipitationFormDrizzle;
  float thePrecipitationFormSleet;
  float thePrecipitationFormSnow;
  float thePrecipitationFormFreezing;
  float thePrecipitationTypeShowers;
  weather_event_id theWeatherEventId;
  float thePearsonCoefficient;
  TextGenPosixTime theObservationTime;
  float thePrecipitationPercentageNorthEast;
  float thePrecipitationPercentageSouthEast;
  float thePrecipitationPercentageSouthWest;
  float thePrecipitationPercentageNorthWest;
  weather_result_data_item_vector thePrecipitationShareNorthEast;
  weather_result_data_item_vector thePrecipitationShareSouthEast;
  weather_result_data_item_vector thePrecipitationShareSouthWest;
  weather_result_data_item_vector thePrecipitationShareNorthWest;
  NFmiPoint thePrecipitationPoint;
  precipitation_type thePrecipitationType;
  precipitation_intesity_id thePrecipitationIntensityId;

  float precipitationPercentageNorth() const
  {
    return thePrecipitationPercentageNorthEast + thePrecipitationPercentageNorthWest;
  }
  float precipitationPercentageSouth() const
  {
    return thePrecipitationPercentageSouthEast + thePrecipitationPercentageSouthWest;
  }
  float precipitationPercentageEast() const
  {
    return thePrecipitationPercentageNorthEast + thePrecipitationPercentageSouthEast;
  }
  float precipitationPercentageWest() const
  {
    return thePrecipitationPercentageNorthWest + thePrecipitationPercentageSouthWest;
  }

  bool operator==(const PrecipitationDataItemData& theItem)
  {
    return (thePrecipitationForm == theItem.thePrecipitationForm &&
            thePrecipitationIntensity == theItem.thePrecipitationIntensity &&
            thePrecipitationExtent == theItem.thePrecipitationExtent &&
            thePrecipitationFormWater == theItem.thePrecipitationFormWater &&
            thePrecipitationFormDrizzle == theItem.thePrecipitationFormDrizzle &&
            thePrecipitationFormSleet == theItem.thePrecipitationFormSleet &&
            thePrecipitationFormSnow == theItem.thePrecipitationFormSnow &&
            thePrecipitationFormFreezing == theItem.thePrecipitationFormFreezing &&
            thePrecipitationTypeShowers == theItem.thePrecipitationTypeShowers &&
            theWeatherEventId == theItem.theWeatherEventId &&
            thePearsonCoefficient == theItem.thePearsonCoefficient &&
            thePrecipitationShareNorthEast == theItem.thePrecipitationShareNorthEast &&
            thePrecipitationShareSouthEast == theItem.thePrecipitationShareSouthEast &&
            thePrecipitationShareSouthWest == theItem.thePrecipitationShareSouthWest &&
            thePrecipitationShareNorthWest == theItem.thePrecipitationShareNorthWest);
  }
};

struct PrecipitationDataItem
{
  PrecipitationDataItem(const PrecipitationDataItemData* coastalData,
                        const PrecipitationDataItemData* inlandData,
                        const PrecipitationDataItemData* fullData)
      : theCoastalData(coastalData), theInlandData(inlandData), theFullData(fullData)
  {
  }

  const PrecipitationDataItemData* theCoastalData;
  const PrecipitationDataItemData* theInlandData;
  const PrecipitationDataItemData* theFullData;

  ~PrecipitationDataItem()
  {
    if (theCoastalData) delete theCoastalData;
    if (theInlandData) delete theInlandData;
    if (theFullData) delete theFullData;
  }
};
}

#endif  // TEXTGEN_WEATHER_FORECAST_H
