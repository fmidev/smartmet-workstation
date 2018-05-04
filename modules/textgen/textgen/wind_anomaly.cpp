// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::WindStory::anomaly
 */
// ======================================================================

#include "WindStory.h"

#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "TemperatureStoryTools.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "WindStoryTools.h"
#include "AreaTools.h"
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherPeriodTools.h>
#include "SeasonTools.h"
#include <calculator/WeatherResult.h>
#include "PeriodPhraseFactory.h"
#include "Delimiter.h"
#include "WeatherForecast.h"

#include <newbase/NFmiStringTools.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiIndexMask.h>
#include <newbase/NFmiIndexMaskSource.h>
#include <newbase/NFmiIndexMaskTools.h>
#include <newbase/NFmiMetMath.h>

#include <calculator/TextGenError.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherSource.h>

#include <boost/lexical_cast.hpp>

#include <map>

#include "WeatherForecast.h"

namespace TextGen
{
namespace WindAnomaly
{
using NFmiStringTools::Convert;
using namespace TextGen;
using namespace WindStoryTools;
using namespace TemperatureStoryTools;
using namespace boost;
using namespace std;
using namespace Settings;
using namespace SeasonTools;
using namespace AreaTools;
using Settings::optional_int;
using Settings::optional_bool;
using Settings::optional_string;

#define ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE \
  "[iltapaivalla] [rannikolla] on [tuulista]"
#define ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE "[iltapaivalla] on [tuulista]"
#define RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE "[rannikolla] on [tuulista]"

#define ILTAPAIVALLA_RANNIKOLLA_PAKKANEN_ON_PUREVAA_COMPOSITE_PHRASE \
  "[iltapaivalla] [rannikolla] pakkanen on purevaa"
#define ILTAPAIVALLA_PAKKANEN_ON_PUREVAA_COMPOSITE_PHRASE "[iltapaivalla] pakkanen on purevaa"
#define RANNIKOLLA_PAKKANEN_ON_PUREVAA_COMPOSITE_PHRASE "[rannikolla] pakkanen on purevaa"
#define ILTAPAIVALLA_RANNIKOLLA_PAKKANEN_ON_ERITTAIN_PUREVAA_COMPOSITE_PHRASE \
  "[iltapaivalla] [rannikolla] pakkanen on erittain purevaa"
#define ILTAPAIVALLA_PAKKANEN_ON_ERITTAIN_PUREVAA_COMPOSITE_PHRASE \
  "[iltapaivalla] pakkanen on erittain purevaa"
#define RANNIKOLLA_PAKKANEN_ON_ERITTAIN_PUREVAA_COMPOSITE_PHRASE \
  "[rannikolla] pakkanen on erittain purevaa"

#define ILTAPAIVALLA_RANNIKOLLA_TUULI_SAA_SAAN_TUNTUMAAN_KYLMEMMALTA_COMPOSITE_PHRASE \
  "[iltapaivalla] [rannikolla] tuuli saa saan tuntumaan kylmemmalta"
#define ILTAPAIVALLA_TUULI_SAA_SAAN_TUNTUMAAN_KYLMEMMALTA_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli saa saan tuntumaan kylmemmalta"
#define RANNIKOLLA_TUULI_SAA_SAAN_TUNTUMAAN_KYLMEMMALTA_COMPOSITE_PHRASE \
  "[rannikolla] tuuli saa saan tuntumaan kylmemmalta"
#define ILTAPAIVALLA_RANNIKOLLA_TUULI_SAA_SAAN_TUNTUMAAN_VIILEAMMALTA_COMPOSITE_PHRASE \
  "[iltapaivalla] [rannikolla] tuuli saa saan tuntumaan viileammalta"
#define ILTAPAIVALLA_TUULI_SAA_SAAN_TUNTUMAAN_VIILEAMMALTA_COMPOSITE_PHRASE \
  "[rannikolla] tuuli saa saan tuntumaan viileammalta"
#define RANNIKOLLA_TUULI_SAA_SAAN_TUNTUMAAN_VIILEAMMALTA_COMPOSITE_PHRASE \
  "[rannikolla] tuuli saa saan tuntumaan viileammalta"

#define WINDY_WEATER_LIMIT 7.0
#define EXTREMELY_WINDY_WEATHER_LIMIT 10.0
#define WIND_COOLING_THE_WEATHER_LIMIT 6.0
#define EXTREME_WINDCHILL_LIMIT -35.0
#define MILD_WINDCHILL_LIMIT -25.0
#define ZERO_DEGREES 0.0
#define TEMPERATURE_AND_WINDCHILL_DIFFERENCE_LIMIT 7.0

#define TUULI_KYLMENTAA_SAATA_LOWER_LIMIT 0.0
#define TUULI_KYLMENTAA_SAATA_UPPER_LIMIT 10.0
#define TUULI_VIILENTAA_SAATA_LOWER_LIMIT 10.0
#define TUULI_VIILENTAA_SAATA_UPPER_LIMIT 15.0

#define SAA_WORD "saa"
#define ON_WORD "on"
#define HYVIN_WORD "hyvin"
#define SAA_ON_TUULINEN_PHRASE "saa on tuulinen"
#define SAA_ON_HYVIN_TUULINEN_PHRASE "saa on hyvin tuulinen"
#define HYVIN_TUULINEN_PHRASE "hyvin tuulinen"
#define HYVIN_TUULISTA_PHRASE "hyvin tuulista"
#define TUULISTA_WORD "tuulista"
#define AAMUPAIVALLA_WORD "aamupaivalla"
#define ILTAPAIVALLA_WORD "iltapaivalla"
#define SISAMAASSA_WORD "sisamaassa"
#define RANNIKOLLA_WORD "rannikolla"
#define PAKKANEN_ON_PUREVAA_PHRASE "pakkanen on purevaa"
#define PAKKANEN_ON_ERITTAIN_PUREVAA_PHRASE "pakkanen on erittain purevaa"
#define TUULI_SAA_SAAN_TUNTUMAAN_VIILEAMMALTA_PHRASE "tuuli saa saan tuntumaan viileammalta"
#define TUULI_SAA_SAAN_TUNTUMAAN_KYLMEMMALTA_PHRASE "tuuli saa saan tuntumaan kylmemmalta"
#define TANAAN_WORD "tanaan"
#define HUOMENNA_WORD "huomenna"

struct wind_anomaly_params
{
  wind_anomaly_params(const string& variable,
                      MessageLogger& log,
                      const AnalysisSources& sources,
                      const WeatherArea& area,
                      const WeatherPeriod& windAnomalyPeriod,
                      const forecast_season_id& season,
                      const TextGenPosixTime& forecastTime,
                      const short& periodLength,
                      const bool& specifyPartOfTheDayFlag)
      : theVariable(variable),
        theLog(log),
        theSources(sources),
        theArea(area),
        thePeriod(windAnomalyPeriod),
        theSeason(season),
        theForecastTime(forecastTime),
        thePeriodLength(periodLength),
        theSpecifyPartOfTheDayFlag(specifyPartOfTheDayFlag),
        theFakeVariable(""),
        theCoastalAndInlandTogetherFlag(false),
        theTemperatureAreaMorningMinimum(kFloatMissing, 0),
        theTemperatureAreaMorningMean(kFloatMissing, 0),
        theTemperatureAreaMorningMaximum(kFloatMissing, 0),
        theTemperatureAreaAfternoonMinimum(kFloatMissing, 0),
        theTemperatureAreaAfternoonMean(kFloatMissing, 0),
        theTemperatureAreaAfternoonMaximum(kFloatMissing, 0),
        theTemperatureInlandMorningMinimum(kFloatMissing, 0),
        theTemperatureInlandMorningMean(kFloatMissing, 0),
        theTemperatureInlandMorningMaximum(kFloatMissing, 0),
        theTemperatureInlandAfternoonMinimum(kFloatMissing, 0),
        theTemperatureInlandAfternoonMean(kFloatMissing, 0),
        theTemperatureInlandAfternoonMaximum(kFloatMissing, 0),
        theTemperatureCoastalMorningMinimum(kFloatMissing, 0),
        theTemperatureCoastalMorningMean(kFloatMissing, 0),
        theTemperatureCoastalMorningMaximum(kFloatMissing, 0),
        theTemperatureCoastalAfternoonMinimum(kFloatMissing, 0),
        theTemperatureCoastalAfternoonMean(kFloatMissing, 0),
        theTemperatureCoastalAfternoonMaximum(kFloatMissing, 0),
        theWindspeedInlandMorningMinimum(kFloatMissing, 0),
        theWindspeedInlandMorningMean(kFloatMissing, 0),
        theWindspeedInlandMorningMaximum(kFloatMissing, 0),
        theWindspeedInlandAfternoonMinimum(kFloatMissing, 0),
        theWindspeedInlandAfternoonMean(kFloatMissing, 0),
        theWindspeedInlandAfternoonMaximum(kFloatMissing, 0),
        theWindspeedCoastalMorningMinimum(kFloatMissing, 0),
        theWindspeedCoastalMorningMean(kFloatMissing, 0),
        theWindspeedCoastalMorningMaximum(kFloatMissing, 0),
        theWindspeedCoastalAfternoonMinimum(kFloatMissing, 0),
        theWindspeedCoastalAfternoonMean(kFloatMissing, 0),
        theWindspeedCoastalAfternoonMaximum(kFloatMissing, 0),
        theWindchillInlandMorningMinimum(kFloatMissing, 0),
        theWindchillInlandMorningMean(kFloatMissing, 0),
        theWindchillInlandMorningMaximum(kFloatMissing, 0),
        theWindchillInlandAfternoonMinimum(kFloatMissing, 0),
        theWindchillInlandAfternoonMean(kFloatMissing, 0),
        theWindchillInlandAfternoonMaximum(kFloatMissing, 0),
        theWindchillCoastalMorningMinimum(kFloatMissing, 0),
        theWindchillCoastalMorningMean(kFloatMissing, 0),
        theWindchillCoastalMorningMaximum(kFloatMissing, 0),
        theWindchillCoastalAfternoonMinimum(kFloatMissing, 0),
        theWindchillCoastalAfternoonMean(kFloatMissing, 0),
        theWindchillCoastalAfternoonMaximum(kFloatMissing, 0),
        theMorningWord(""),
        theAfternoonWord("")
  {
  }

  const string& theVariable;
  MessageLogger& theLog;
  const AnalysisSources& theSources;
  const WeatherArea& theArea;
  const WeatherPeriod& thePeriod;
  const forecast_season_id& theSeason;
  const TextGenPosixTime& theForecastTime;
  const short& thePeriodLength;
  bool theSpecifyPartOfTheDayFlag;
  string theFakeVariable;
  bool theCoastalAndInlandTogetherFlag;
  WeatherResult theTemperatureAreaMorningMinimum;
  WeatherResult theTemperatureAreaMorningMean;
  WeatherResult theTemperatureAreaMorningMaximum;
  WeatherResult theTemperatureAreaAfternoonMinimum;
  WeatherResult theTemperatureAreaAfternoonMean;
  WeatherResult theTemperatureAreaAfternoonMaximum;
  WeatherResult theTemperatureInlandMorningMinimum;
  WeatherResult theTemperatureInlandMorningMean;
  WeatherResult theTemperatureInlandMorningMaximum;
  WeatherResult theTemperatureInlandAfternoonMinimum;
  WeatherResult theTemperatureInlandAfternoonMean;
  WeatherResult theTemperatureInlandAfternoonMaximum;
  WeatherResult theTemperatureCoastalMorningMinimum;
  WeatherResult theTemperatureCoastalMorningMean;
  WeatherResult theTemperatureCoastalMorningMaximum;
  WeatherResult theTemperatureCoastalAfternoonMinimum;
  WeatherResult theTemperatureCoastalAfternoonMean;
  WeatherResult theTemperatureCoastalAfternoonMaximum;
  WeatherResult theWindspeedInlandMorningMinimum;
  WeatherResult theWindspeedInlandMorningMean;
  WeatherResult theWindspeedInlandMorningMaximum;
  WeatherResult theWindspeedInlandAfternoonMinimum;
  WeatherResult theWindspeedInlandAfternoonMean;
  WeatherResult theWindspeedInlandAfternoonMaximum;
  WeatherResult theWindspeedCoastalMorningMinimum;
  WeatherResult theWindspeedCoastalMorningMean;
  WeatherResult theWindspeedCoastalMorningMaximum;
  WeatherResult theWindspeedCoastalAfternoonMinimum;
  WeatherResult theWindspeedCoastalAfternoonMean;
  WeatherResult theWindspeedCoastalAfternoonMaximum;
  WeatherResult theWindchillInlandMorningMinimum;
  WeatherResult theWindchillInlandMorningMean;
  WeatherResult theWindchillInlandMorningMaximum;
  WeatherResult theWindchillInlandAfternoonMinimum;
  WeatherResult theWindchillInlandAfternoonMean;
  WeatherResult theWindchillInlandAfternoonMaximum;
  WeatherResult theWindchillCoastalMorningMinimum;
  WeatherResult theWindchillCoastalMorningMean;
  WeatherResult theWindchillCoastalMorningMaximum;
  WeatherResult theWindchillCoastalAfternoonMinimum;
  WeatherResult theWindchillCoastalAfternoonMean;
  WeatherResult theWindchillCoastalAfternoonMaximum;
  std::string theMorningWord;
  std::string theAfternoonWord;
};

void log_data(const wind_anomaly_params& theParameters)
{
  if (theParameters.theTemperatureAreaMorningMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theTemperatureAreaMorningMinimum: "
                         << theParameters.theTemperatureAreaMorningMinimum << endl;
    theParameters.theLog << "theTemperatureAreaMorningMean: "
                         << theParameters.theTemperatureAreaMorningMean << endl;
    theParameters.theLog << "theTemperatureAreaMorningMaximum: "
                         << theParameters.theTemperatureAreaMorningMaximum << endl;
  }

  if (theParameters.theTemperatureAreaAfternoonMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theTemperatureAreaAfternoonMinimum: "
                         << theParameters.theTemperatureAreaAfternoonMinimum << endl;
    theParameters.theLog << "theTemperatureAreaAfternoonMean: "
                         << theParameters.theTemperatureAreaAfternoonMean << endl;
    theParameters.theLog << "theTemperatureAreaAfternoonMaximum: "
                         << theParameters.theTemperatureAreaAfternoonMaximum << endl;
  }

  if (theParameters.theTemperatureInlandMorningMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theTemperatureInlandMorningMinimum: "
                         << theParameters.theTemperatureInlandMorningMinimum << endl;
    theParameters.theLog << "theTemperatureInlandMorningMean: "
                         << theParameters.theTemperatureInlandMorningMean << endl;
    theParameters.theLog << "theTemperatureInlandMorningMaximum: "
                         << theParameters.theTemperatureInlandMorningMaximum << endl;
  }

  if (theParameters.theTemperatureInlandAfternoonMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theTemperatureInlandAfternoonMinimum: "
                         << theParameters.theTemperatureInlandAfternoonMinimum << endl;
    theParameters.theLog << "theTemperatureInlandAfternoonMean: "
                         << theParameters.theTemperatureInlandAfternoonMean << endl;
    theParameters.theLog << "theTemperatureInlandAfternoonMaximum: "
                         << theParameters.theTemperatureInlandAfternoonMaximum << endl;
  }

  if (theParameters.theTemperatureCoastalMorningMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theTemperatureCoastalMorningMinimum: "
                         << theParameters.theTemperatureCoastalMorningMinimum << endl;
    theParameters.theLog << "theTemperatureCoastalMorningMean: "
                         << theParameters.theTemperatureCoastalMorningMean << endl;
    theParameters.theLog << "theTemperatureCoastalMorningMaximum: "
                         << theParameters.theTemperatureCoastalMorningMaximum << endl;
  }

  if (theParameters.theTemperatureCoastalAfternoonMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theTemperatureCoastalAfternoonMinimum: "
                         << theParameters.theTemperatureCoastalAfternoonMinimum << endl;
    theParameters.theLog << "theTemperatureCoastalAfternoonMean: "
                         << theParameters.theTemperatureCoastalAfternoonMean << endl;
    theParameters.theLog << "theTemperatureCoastalAfternoonMaximum: "
                         << theParameters.theTemperatureCoastalAfternoonMaximum << endl;
  }

  if (theParameters.theWindspeedInlandMorningMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theWindspeedInlandMorningMinimum: "
                         << theParameters.theWindspeedInlandMorningMinimum << endl;
    theParameters.theLog << "theWindspeedInlandMorningMean: "
                         << theParameters.theWindspeedInlandMorningMean << endl;
    theParameters.theLog << "theWindspeedInlandMorningMaximum: "
                         << theParameters.theWindspeedInlandMorningMaximum << endl;
  }

  if (theParameters.theWindspeedInlandAfternoonMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theWindspeedInlandAfternoonMinimum: "
                         << theParameters.theWindspeedInlandAfternoonMinimum << endl;
    theParameters.theLog << "theWindspeedInlandAfternoonMean: "
                         << theParameters.theWindspeedInlandAfternoonMean << endl;
    theParameters.theLog << "theWindspeedInlandAfternoonMaximum: "
                         << theParameters.theWindspeedInlandAfternoonMaximum << endl;
  }

  if (theParameters.theWindspeedCoastalMorningMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theWindspeedCoastalMorningMinimum: "
                         << theParameters.theWindspeedCoastalMorningMinimum << endl;
    theParameters.theLog << "theWindspeedCoastalMorningMean: "
                         << theParameters.theWindspeedCoastalMorningMean << endl;
    theParameters.theLog << "theWindspeedCoastalMorningMaximum: "
                         << theParameters.theWindspeedCoastalMorningMaximum << endl;
  }
  if (theParameters.theWindspeedCoastalAfternoonMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theWindspeedCoastalAfternoonMinimum: "
                         << theParameters.theWindspeedCoastalAfternoonMinimum << endl;
    theParameters.theLog << "theWindspeedCoastalAfternoonMean: "
                         << theParameters.theWindspeedCoastalAfternoonMean << endl;
    theParameters.theLog << "theWindspeedCoastalAfternoonMaximum: "
                         << theParameters.theWindspeedCoastalAfternoonMaximum << endl;
  }

  if (theParameters.theWindchillInlandMorningMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theWindchillInlandMorningMinimum: "
                         << theParameters.theWindchillInlandMorningMinimum << endl;
    theParameters.theLog << "theWindchillInlandMorningMean: "
                         << theParameters.theWindchillInlandMorningMean << endl;
    theParameters.theLog << "theWindchillInlandMorningMaximum: "
                         << theParameters.theWindchillInlandMorningMaximum << endl;
  }

  if (theParameters.theWindchillInlandAfternoonMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theWindchillInlandAfternoonMinimum: "
                         << theParameters.theWindchillInlandAfternoonMinimum << endl;
    theParameters.theLog << "theWindchillInlandAfternoonMean: "
                         << theParameters.theWindchillInlandAfternoonMean << endl;
    theParameters.theLog << "theWindchillInlandAfternoonMaximum: "
                         << theParameters.theWindchillInlandAfternoonMaximum << endl;
  }

  if (theParameters.theWindchillCoastalMorningMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theWindchillCoastalMorningMinimum: "
                         << theParameters.theWindchillCoastalMorningMinimum << endl;
    theParameters.theLog << "theWindchillCoastalMorningMean: "
                         << theParameters.theWindchillCoastalMorningMean << endl;
    theParameters.theLog << "theWindchillCoastalMorningMaximum: "
                         << theParameters.theWindchillCoastalMorningMaximum << endl;
  }

  if (theParameters.theWindchillCoastalAfternoonMinimum.value() != kFloatMissing)
  {
    theParameters.theLog << "theWindchillCoastalAfternoonMinimum: "
                         << theParameters.theWindchillCoastalAfternoonMinimum << endl;
    theParameters.theLog << "theWindchillCoastalAfternoonMean: "
                         << theParameters.theWindchillCoastalAfternoonMean << endl;
    theParameters.theLog << "theWindchillCoastalAfternoonMaximum: "
                         << theParameters.theWindchillCoastalAfternoonMaximum << endl;
  }
}

void calculate_windspeed_and_chill(wind_anomaly_params& theParameters,
                                   const TextGen::WeatherArea::Type& theType,
                                   const bool& theMorningPeriod,
                                   const bool& theWindspeed)
{
  GridForecaster theForecaster;

  WeatherArea theArea(theParameters.theArea);
  theArea.type(theParameters.theCoastalAndInlandTogetherFlag ? WeatherArea::Full : theType);

  std::string theFakeVariable("");
  std::string postfix_string(theWindspeed ? "::fake::windspeed" : "::fake::windchill");
  postfix_string += (theMorningPeriod ? "::morning" : "::afternoon");
  if (theType == WeatherArea::Inland)
  {
    postfix_string += "::inland";
  }
  else if (theType == WeatherArea::Coast)
  {
    postfix_string += "::coast";
  }

  theFakeVariable = theParameters.theVariable + postfix_string;

  WeatherPeriod theCompletePeriod(theParameters.thePeriod);

  int noonHour = (theMorningPeriod ? (theCompletePeriod.localEndTime().GetHour() < 12
                                          ? theCompletePeriod.localEndTime().GetHour()
                                          : 12)
                                   : (theCompletePeriod.localStartTime().GetHour() < 12
                                          ? theCompletePeriod.localStartTime().GetHour()
                                          : 12));

  TextGenPosixTime noonTime(theCompletePeriod.localStartTime().GetYear(),
                            theCompletePeriod.localStartTime().GetMonth(),
                            theCompletePeriod.localStartTime().GetDay(),
                            noonHour,
                            0,
                            0);

  TextGenPosixTime theStartTime(theMorningPeriod ? theCompletePeriod.localStartTime() : noonTime);
  TextGenPosixTime theEndTime(theMorningPeriod ? noonTime : theCompletePeriod.localEndTime());

  WeatherPeriod thePeriod(
      theStartTime > theEndTime ? theParameters.thePeriod.localStartTime() : theStartTime,
      theStartTime > theEndTime ? theParameters.thePeriod.localEndTime() : theEndTime);

  WeatherResult* theMinimum = 0;
  WeatherResult* theMean = 0;
  WeatherResult* theMaximum = 0;

  if (theType == WeatherArea::Inland)
  {
    if (theMorningPeriod)
    {
      theMinimum = &(theWindspeed ? theParameters.theWindspeedInlandMorningMinimum
                                  : theParameters.theWindchillInlandMorningMinimum);
      theMean = &(theWindspeed ? theParameters.theWindspeedInlandMorningMean
                               : theParameters.theWindchillInlandMorningMean);
      theMaximum = &(theWindspeed ? theParameters.theWindspeedInlandMorningMaximum
                                  : theParameters.theWindchillInlandMorningMaximum);
    }
    else
    {
      theMinimum = &(theWindspeed ? theParameters.theWindspeedInlandAfternoonMinimum
                                  : theParameters.theWindchillInlandAfternoonMinimum);
      theMean = &(theWindspeed ? theParameters.theWindspeedInlandAfternoonMean
                               : theParameters.theWindchillInlandAfternoonMean);
      theMaximum = &(theWindspeed ? theParameters.theWindspeedInlandAfternoonMaximum
                                  : theParameters.theWindchillInlandAfternoonMaximum);
    }
  }
  else if (theType == WeatherArea::Coast)
  {
    if (theMorningPeriod)
    {
      theMinimum = &(theWindspeed ? theParameters.theWindspeedCoastalMorningMinimum
                                  : theParameters.theWindchillCoastalMorningMinimum);
      theMean = &(theWindspeed ? theParameters.theWindspeedCoastalMorningMean
                               : theParameters.theWindchillCoastalMorningMean);
      theMaximum = &(theWindspeed ? theParameters.theWindspeedCoastalMorningMaximum
                                  : theParameters.theWindchillCoastalMorningMaximum);
    }
    else
    {
      theMinimum = &(theWindspeed ? theParameters.theWindspeedCoastalAfternoonMinimum
                                  : theParameters.theWindchillCoastalAfternoonMinimum);
      theMean = &(theWindspeed ? theParameters.theWindspeedCoastalAfternoonMean
                               : theParameters.theWindchillCoastalAfternoonMean);
      theMaximum = &(theWindspeed ? theParameters.theWindspeedCoastalAfternoonMaximum
                                  : theParameters.theWindchillCoastalAfternoonMaximum);
    }
  }

  *theMinimum = theForecaster.analyze(theFakeVariable + "::min",
                                      theParameters.theSources,
                                      theWindspeed ? WindSpeed : WindChill,
                                      Minimum,
                                      Maximum,
                                      theArea,
                                      thePeriod);

  *theMaximum = theForecaster.analyze(theFakeVariable + "::max",
                                      theParameters.theSources,
                                      theWindspeed ? WindSpeed : WindChill,
                                      Maximum,
                                      Maximum,
                                      theArea,
                                      thePeriod);

  *theMean = theForecaster.analyze(theFakeVariable + "::mean",
                                   theParameters.theSources,
                                   theWindspeed ? WindSpeed : WindChill,
                                   Mean,
                                   Maximum,
                                   theArea,
                                   thePeriod);
}

void log_start_time_and_end_time(MessageLogger& theLog,
                                 const std::string& theLogMessage,
                                 const WeatherPeriod& thePeriod)
{
  theLog << NFmiStringTools::Convert(theLogMessage) << thePeriod.localStartTime() << " ... "
         << thePeriod.localEndTime() << endl;
}

Sentence get_windiness_sentence(const Sentence& timeSpecifier,
                                const std::string& areaSpecifier,
                                const std::string& plainWindinessPhrase,
                                const std::string& windinessPhrase,
                                const bool& timeSpecifierEmpty,
                                const bool& areaSpecifierEmpty)
{
  Sentence sentence;

  if (timeSpecifierEmpty && areaSpecifierEmpty)
    sentence << plainWindinessPhrase;
  else if (!timeSpecifierEmpty && areaSpecifierEmpty)
    sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << timeSpecifier << windinessPhrase;
  else if (timeSpecifierEmpty && !areaSpecifierEmpty)
    sentence << RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << areaSpecifier << windinessPhrase;
  else
    sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << timeSpecifier
             << areaSpecifier << windinessPhrase;

  return sentence;
}

Sentence get_windiness_sentence(const std::string& timeSpecifier,
                                const std::string& areaSpecifier,
                                const std::string& plainWindinessPhrase,
                                const std::string& windinessPhrase,
                                const bool& timeSpecifierEmpty,
                                const bool& areaSpecifierEmpty)
{
  Sentence sentence;
  Sentence timeSentence;
  timeSentence << timeSpecifier;

  sentence << get_windiness_sentence(timeSentence,
                                     areaSpecifier,
                                     plainWindinessPhrase,
                                     windinessPhrase,
                                     timeSpecifierEmpty,
                                     areaSpecifierEmpty);
  return sentence;
}

const Sentence construct_windiness_sentence_for_area(const float& windspeedMorning,
                                                     const float& windspeedAfternoon,
                                                     const float& windyWeatherLimit,
                                                     const float& extremelyWindyWeatherLimit,
                                                     const Sentence& specifiedDaySentence,
                                                     const short& dayNumber,
                                                     const std::string& areaString,
                                                     const std::string& morningWord,
                                                     const std::string& afternoonWord)
{
  Sentence sentence;

  bool morningIncluded = windspeedMorning != kFloatMissing;
  bool afternoonIncluded = windspeedAfternoon != kFloatMissing;

  Sentence specifiedDay;
  if (specifiedDaySentence.size() == 0)
    specifiedDay << EMPTY_STRING;
  else
    specifiedDay << specifiedDaySentence;

  std::string weekdayMorningString(parse_weekday_phrase(dayNumber, morningWord));
  std::string weekdayAfternoonString(parse_weekday_phrase(dayNumber, afternoonWord));
  bool areaStringEmpty(areaString.compare(EMPTY_STRING) == 0);
  bool specifiedDayEmpty(specifiedDaySentence.size() == 0);
  bool weekdayMorningStringEmpty(weekdayMorningString.compare(EMPTY_STRING) == 0);
  bool weekdayAfternoonStringEmpty(weekdayAfternoonString.compare(EMPTY_STRING) == 0);

  if (morningIncluded && afternoonIncluded)
  {
    if (windspeedMorning >= extremelyWindyWeatherLimit &&
        windspeedAfternoon >= extremelyWindyWeatherLimit)
    {
      sentence << get_windiness_sentence(specifiedDay,
                                         areaString,
                                         SAA_ON_HYVIN_TUULINEN_PHRASE,
                                         HYVIN_TUULISTA_PHRASE,
                                         specifiedDayEmpty,
                                         areaStringEmpty);

      /*
if(areaStringEmpty && specifiedDayEmpty)
    sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
else if(areaStringEmpty && !specifiedDayEmpty)
    sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << specifiedDay
                     << HYVIN_TUULISTA_PHRASE;
else if(!areaStringEmpty && specifiedDayEmpty)
    sentence << RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << areaString
                     << HYVIN_TUULISTA_PHRASE;
else
    sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << specifiedDay
                     << areaString
                     << HYVIN_TUULISTA_PHRASE;
      */
    }
    else if (windspeedMorning >= extremelyWindyWeatherLimit)
    {
      sentence << get_windiness_sentence(weekdayMorningString,
                                         areaString,
                                         SAA_ON_HYVIN_TUULINEN_PHRASE,
                                         HYVIN_TUULISTA_PHRASE,
                                         weekdayMorningStringEmpty,
                                         areaStringEmpty);

      /*
      if(areaStringEmpty && weekdayMorningStringEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << weekdayMorningString
                             << areaString
                             << HYVIN_TUULISTA_PHRASE;
      */
    }
    else if (windspeedAfternoon >= extremelyWindyWeatherLimit)
    {
      sentence << get_windiness_sentence(weekdayAfternoonString,
                                         areaString,
                                         SAA_ON_HYVIN_TUULINEN_PHRASE,
                                         HYVIN_TUULISTA_PHRASE,
                                         weekdayAfternoonStringEmpty,
                                         areaStringEmpty);

      /*
      if(areaStringEmpty && weekdayAfternoonStringEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << weekdayAfternoonString
                             << areaString
                             << HYVIN_TUULISTA_PHRASE;
      */
    }
    else if (windspeedMorning >= windyWeatherLimit && windspeedAfternoon >= windyWeatherLimit)
    {
      sentence << get_windiness_sentence(specifiedDay,
                                         areaString,
                                         SAA_ON_TUULINEN_PHRASE,
                                         TUULISTA_WORD,
                                         specifiedDayEmpty,
                                         areaStringEmpty);

      /*
      if(areaStringEmpty && specifiedDayEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << specifiedDay
                             << areaString
                             << TUULISTA_WORD;
      */
    }
    else if (windspeedMorning >= windyWeatherLimit)
    {
      sentence << get_windiness_sentence(weekdayMorningString,
                                         areaString,
                                         SAA_ON_TUULINEN_PHRASE,
                                         TUULISTA_WORD,
                                         weekdayMorningStringEmpty,
                                         areaStringEmpty);

      /*
      if(areaStringEmpty && weekdayMorningStringEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << weekdayMorningString
                             << areaString
                             << TUULISTA_WORD;
      */
    }
    else if (windspeedAfternoon >= windyWeatherLimit)
    {
      sentence << get_windiness_sentence(weekdayAfternoonString,
                                         areaString,
                                         SAA_ON_TUULINEN_PHRASE,
                                         TUULISTA_WORD,
                                         weekdayAfternoonStringEmpty,
                                         areaStringEmpty);
      /*
      if(areaStringEmpty && weekdayAfternoonStringEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << weekdayAfternoonString
                             << areaString
                             << TUULISTA_WORD;
      */
    }
  }
  else if (morningIncluded)
  {
    if (windspeedMorning >= extremelyWindyWeatherLimit)
    {
      sentence << get_windiness_sentence(specifiedDay,
                                         areaString,
                                         SAA_ON_HYVIN_TUULINEN_PHRASE,
                                         HYVIN_TUULISTA_PHRASE,
                                         specifiedDayEmpty,
                                         areaStringEmpty);
      /*
      if(areaStringEmpty && specifiedDayEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << specifiedDay
                             << areaString
                             << HYVIN_TUULISTA_PHRASE;
      */
    }
    else if (windspeedMorning >= windyWeatherLimit)
    {
      sentence << get_windiness_sentence(specifiedDay,
                                         areaString,
                                         SAA_ON_TUULINEN_PHRASE,
                                         TUULISTA_WORD,
                                         specifiedDayEmpty,
                                         areaStringEmpty);
      /*
      if(areaStringEmpty && specifiedDayEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << specifiedDay
                             << areaString
                             << TUULISTA_WORD;
      */
    }
  }
  else if (afternoonIncluded)
  {
    if (windspeedAfternoon >= extremelyWindyWeatherLimit)
    {
      sentence << get_windiness_sentence(specifiedDay,
                                         areaString,
                                         SAA_ON_HYVIN_TUULINEN_PHRASE,
                                         HYVIN_TUULISTA_PHRASE,
                                         specifiedDayEmpty,
                                         areaStringEmpty);
      /*
      if(areaStringEmpty && specifiedDayEmpty)
      if(areaStringEmpty && specifiedDayEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << specifiedDay
                             << areaString
                             << HYVIN_TUULISTA_PHRASE;
      */
    }
    else if (windspeedAfternoon >= windyWeatherLimit)
    {
      sentence << get_windiness_sentence(specifiedDay,
                                         areaString,
                                         SAA_ON_TUULINEN_PHRASE,
                                         TUULISTA_WORD,
                                         specifiedDayEmpty,
                                         areaStringEmpty);
      /*
      if(areaStringEmpty && specifiedDayEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
      else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                             << specifiedDay
                             << areaString
                             << TUULISTA_WORD;
      */
    }
  }

  return sentence;
}

const Sentence construct_windiness_sentence(const wind_anomaly_params& theParameters,
                                            const Sentence& theSpecifiedDay,
                                            const short& dayNumber)
{
  Sentence sentence;

  bool inlandIncluded = theParameters.theWindspeedInlandMorningMinimum.value() != kFloatMissing ||
                        theParameters.theWindspeedInlandAfternoonMinimum.value() != kFloatMissing;
  bool coastIncluded = theParameters.theWindspeedCoastalMorningMinimum.value() != kFloatMissing ||
                       theParameters.theWindspeedCoastalAfternoonMinimum.value() != kFloatMissing;

  float windy_weather_limit = Settings::optional_double(
      theParameters.theVariable + "::windy_weather_limit", WINDY_WEATER_LIMIT);
  float extremely_windy_weather_limit = Settings::optional_double(
      theParameters.theVariable + "::extremely_windy_weather_limit", EXTREMELY_WINDY_WEATHER_LIMIT);

  std::string aamupaivalla(theParameters.theMorningWord);
  std::string iltapaivalla(theParameters.theAfternoonWord);

  int windspeedMorningInland =
      static_cast<int>(round(theParameters.theWindspeedInlandMorningMean.value()));
  int windspeedAfternoonInland =
      static_cast<int>(round(theParameters.theWindspeedInlandAfternoonMean.value()));
  int windspeedMorningCoastal =
      static_cast<int>(round(theParameters.theWindspeedCoastalMorningMean.value()));
  int windspeedAfternoonCoastal =
      static_cast<int>(round(theParameters.theWindspeedCoastalAfternoonMean.value()));

  bool specifiedDayEmpty(theSpecifiedDay.size() == 0);
  Sentence specifiedDay;
  if (theSpecifiedDay.size() == 0)
    specifiedDay << EMPTY_STRING;
  else
    specifiedDay << theSpecifiedDay;

  if (inlandIncluded && coastIncluded)
  {
    bool morningIncluded = windspeedMorningInland != kFloatMissing;
    bool afternoonIncluded = windspeedAfternoonInland != kFloatMissing;

    if (morningIncluded && afternoonIncluded)
    {
      // rannikolla aamulla tuulista
      if (windspeedMorningCoastal >= extremely_windy_weather_limit)
      {
        // rannikolla iltapaivalla hyvin tuulista
        if (windspeedAfternoonCoastal >= extremely_windy_weather_limit)
        {
          // sisamaassa aamulla tai iltapaivalla hyvin tuulista tai lahella sita
          if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0 ||
              windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << HYVIN_TUULINEN_PHRASE;
          }
          else
          {
            if (specifiedDayEmpty)
              sentence << RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << RANNIKOLLA_WORD
                       << HYVIN_TUULISTA_PHRASE;
            else
              sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << RANNIKOLLA_WORD << HYVIN_TUULISTA_PHRASE;
          }
        }
        else if (windspeedAfternoonCoastal >=
                 windy_weather_limit)  // rannikolla on iltapaivalla tuulista
        {
          // sisamaassa aamulla tai iltapaivalla hyvin tuulista tai lahella sita
          if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0 &&
              windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << HYVIN_TUULINEN_PHRASE;
          }
          else if (windspeedMorningInland >= windy_weather_limit - 0.5 ||
                   windspeedAfternoonInland >= windy_weather_limit - 0.5)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << TUULISTA_WORD;
          }
          else
          {
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, iltapaivalla) << RANNIKOLLA_WORD
                     << HYVIN_TUULISTA_PHRASE;
          }
        }
        else  // rannikolla ei tuule paljoa iltapaivalla
        {
          if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0 &&
              windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << HYVIN_TUULINEN_PHRASE;
          }
          else if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0)
          {
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, aamupaivalla) << HYVIN_TUULINEN_PHRASE;
          }
          else
          {
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, aamupaivalla) << RANNIKOLLA_WORD
                     << HYVIN_TUULISTA_PHRASE;
          }
        }
      }
      else if (windspeedMorningCoastal >= windy_weather_limit)
      {  // rannikolla on aamupaivalla tuulista
        // rannikolla on iltapaivalla erittain tuulista
        if (windspeedAfternoonCoastal >= extremely_windy_weather_limit)
        {
          // sisamaassa aamulla tai iltapaivalla hyvin tuulista tai lahella sita
          if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0 ||
              windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << HYVIN_TUULINEN_PHRASE;
          }
          else if (windspeedMorningInland >= windy_weather_limit - 0.5 ||
                   windspeedAfternoonInland >= windy_weather_limit - 0.5)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << TUULISTA_WORD;
          }
          else
          {
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, iltapaivalla) << RANNIKOLLA_WORD
                     << HYVIN_TUULISTA_PHRASE;
          }
        }
        else if (windspeedAfternoonCoastal >=
                 windy_weather_limit)  // rannikolla on iltapaivalla tuulista
        {
          // sisamaassa aamulla tai iltapaivalla hyvin tuulista tai lahella sita
          if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0 &&
              windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << HYVIN_TUULINEN_PHRASE;
          }
          else if (windspeedMorningInland >= windy_weather_limit - 0.5 ||
                   windspeedAfternoonInland >= windy_weather_limit - 0.5)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << TUULISTA_WORD;
          }
          else
          {
            if (specifiedDayEmpty)
              sentence << RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << RANNIKOLLA_WORD
                       << TUULISTA_WORD;
            else
              sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << RANNIKOLLA_WORD << TUULISTA_WORD;
          }
        }
        else  // rannikolla ei tuule paljoa iltapaivalla
        {
          if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0 &&
              windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << HYVIN_TUULINEN_PHRASE;
          }
          else if (windspeedMorningInland >= windy_weather_limit - 0.5 &&
                   windspeedAfternoonInland >= windy_weather_limit - 0.5)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << TUULISTA_WORD;
          }
          else if (windspeedMorningInland >= windy_weather_limit - 0.5)
          {
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, aamupaivalla) << TUULISTA_WORD;
          }
          else
          {
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, aamupaivalla) << RANNIKOLLA_WORD
                     << TUULISTA_WORD;
          }
        }
      }
      else
      {  // rannikolla ei juuri tuule aamupaivalla
        // rannikolla on iltapaivalla erittain tuulista
        if (windspeedAfternoonCoastal >= extremely_windy_weather_limit)
        {
          // sisamaassa aamulla tai iltapaivalla hyvin tuulista tai lahella sita
          if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0 &&
              windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << HYVIN_TUULINEN_PHRASE;
          }
          else if (windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, iltapaivalla) << HYVIN_TUULINEN_PHRASE;
          }
          else if (windspeedMorningInland >= windy_weather_limit - 0.5 &&
                   windspeedAfternoonInland >= windy_weather_limit - 0.5)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << TUULISTA_WORD;
          }
          else
          {
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, iltapaivalla) << RANNIKOLLA_WORD
                     << HYVIN_TUULISTA_PHRASE;
          }
        }
        else if (windspeedAfternoonCoastal >=
                 windy_weather_limit)  // rannikolla on iltapaivalla tuulista
        {
          // sisamaassa aamulla ja iltapaivalla hyvin tuulista tai lahella sita
          if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0 &&
              windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << HYVIN_TUULINEN_PHRASE;
          }
          else if (windspeedMorningInland >= windy_weather_limit - 0.5 &&
                   windspeedAfternoonInland >= windy_weather_limit - 0.5)
          {
            if (specifiedDayEmpty)
              sentence << SAA_ON_TUULINEN_PHRASE;
            else
              sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                       << TUULISTA_WORD;
          }
          else if (windspeedAfternoonInland >= windy_weather_limit - 0.5)
          {
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, iltapaivalla) << TUULISTA_WORD;
          }
          else
          {
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE
                     << parse_weekday_phrase(dayNumber, iltapaivalla) << RANNIKOLLA_WORD
                     << TUULISTA_WORD;
          }
        }
        else  // rannikolla ei juuri tuule iltapaivalla
        {
          sentence << construct_windiness_sentence_for_area(windspeedMorningInland,
                                                            windspeedAfternoonInland,
                                                            windy_weather_limit,
                                                            extremely_windy_weather_limit,
                                                            theSpecifiedDay,
                                                            dayNumber,
                                                            EMPTY_STRING,
                                                            aamupaivalla,
                                                            iltapaivalla);
        }
      }
    }
    else if (morningIncluded)
    {
      // rannikolla aamulla hyvin tuulista
      if (windspeedMorningCoastal >= extremely_windy_weather_limit)
      {
        // sisamaassa on aamulla hyvin tuulista
        if (windspeedMorningInland >= extremely_windy_weather_limit - 1.0)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << HYVIN_TUULINEN_PHRASE;
        }
        else
        {
          if (specifiedDayEmpty)
            sentence << RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << RANNIKOLLA_WORD
                     << HYVIN_TUULISTA_PHRASE;
          else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << RANNIKOLLA_WORD << HYVIN_TUULISTA_PHRASE;
        }
      }
      else if (windspeedMorningCoastal >= windy_weather_limit)
      {  // rannikolla aamulla tuulista
        if (windspeedMorningInland >= extremely_windy_weather_limit)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << HYVIN_TUULINEN_PHRASE;
        }
        else if (windspeedMorningInland >= windy_weather_limit - 0.5)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay << TUULISTA_WORD;
        }
        else
        {
          if (specifiedDayEmpty)
            sentence << RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << RANNIKOLLA_WORD << TUULISTA_WORD;
          else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << RANNIKOLLA_WORD << TUULISTA_WORD;
        }
      }
      else
      {  // rannikolla ei ole aamulla erityisen tuulista
        if (windspeedMorningInland >= extremely_windy_weather_limit)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << HYVIN_TUULINEN_PHRASE;
        }
        else if (windspeedMorningInland >= windy_weather_limit - 0.5)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay << TUULISTA_WORD;
        }
      }
    }
    else if (afternoonIncluded)
    {
      // rannikolla iltapaivalla hyvin tuulista
      if (windspeedAfternoonCoastal >= extremely_windy_weather_limit)
      {
        // sisamaassa on iltapaivalla hyvin tuulista
        if (windspeedAfternoonInland >= extremely_windy_weather_limit - 1.0)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << HYVIN_TUULINEN_PHRASE;
        }
        else
        {
          if (specifiedDayEmpty)
            sentence << RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << RANNIKOLLA_WORD
                     << HYVIN_TUULISTA_PHRASE;
          else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << RANNIKOLLA_WORD << HYVIN_TUULISTA_PHRASE;
        }
      }
      else if (windspeedAfternoonCoastal >= windy_weather_limit)
      {  // rannikolla iltapaivalla tuulista
        if (windspeedAfternoonInland >= extremely_windy_weather_limit)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << HYVIN_TUULINEN_PHRASE;
        }
        else if (windspeedAfternoonInland >= windy_weather_limit - 0.5)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay << TUULISTA_WORD;
        }
        else
        {
          if (specifiedDayEmpty)
            sentence << RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << RANNIKOLLA_WORD << TUULISTA_WORD;
          else
            sentence << ILTAPAIVALLA_RANNIKOLLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << RANNIKOLLA_WORD << TUULISTA_WORD;
        }
      }
      else
      {  // rannikolla ei ole iltapaivalla erityisen tuulista
        if (windspeedAfternoonInland >= extremely_windy_weather_limit)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_HYVIN_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay
                     << HYVIN_TUULINEN_PHRASE;
        }
        else if (windspeedAfternoonInland >= windy_weather_limit - 0.5)
        {
          if (specifiedDayEmpty)
            sentence << SAA_ON_TUULINEN_PHRASE;
          else
            sentence << ILTAPAIVALLA_ON_TUULISTA_COMPOSITE_PHRASE << specifiedDay << TUULISTA_WORD;
        }
      }
    }
  }
  else if (inlandIncluded)
  {
    sentence << construct_windiness_sentence_for_area(windspeedMorningInland,
                                                      windspeedAfternoonInland,
                                                      windy_weather_limit,
                                                      extremely_windy_weather_limit,
                                                      theSpecifiedDay,
                                                      dayNumber,
                                                      EMPTY_STRING,
                                                      aamupaivalla,
                                                      iltapaivalla);
  }
  else if (coastIncluded)
  {
    sentence << construct_windiness_sentence_for_area(windspeedMorningCoastal,
                                                      windspeedAfternoonCoastal,
                                                      windy_weather_limit,
                                                      extremely_windy_weather_limit,
                                                      theSpecifiedDay,
                                                      dayNumber,
                                                      EMPTY_STRING,
                                                      aamupaivalla,
                                                      iltapaivalla);
  }

  return sentence;
}

const Sentence windiness_sentence(const wind_anomaly_params& theParameters)
{
  Sentence sentence;

  bool generate_windiness_sentence =
      Settings::optional_bool(theParameters.theVariable + "::generate_windiness_sentence", true);
  bool generate_wind_cooling_sentence =
      Settings::optional_bool(theParameters.theVariable + "::generate_wind_cooling_sentence", true);

  /*
  bool inlandIncluded = theParameters.theWindspeedInlandMorningMinimum.value() != kFloatMissing ||
        theParameters.theWindspeedInlandAfternoonMinimum.value() != kFloatMissing;
  bool coastIncluded = theParameters.theWindspeedCoastalMorningMinimum.value() != kFloatMissing ||
        theParameters.theWindspeedCoastalAfternoonMinimum.value() != kFloatMissing;
  */
  std::string aamupaivalla(theParameters.theMorningWord);
  std::string iltapaivalla(theParameters.theAfternoonWord);
  std::string part_of_the_day("");
  std::string areaString("");

  Sentence theSpecifiedDay;
  short dayNumber = 0;
  if (theParameters.thePeriodLength > 24)
  {
    theSpecifiedDay << PeriodPhraseFactory::create("today",
                                                   theParameters.theVariable,
                                                   theParameters.theForecastTime,
                                                   theParameters.thePeriod,
                                                   theParameters.theArea);
    dayNumber = theParameters.thePeriod.localStartTime().GetWeekday();
  }

  float wind_cooling_the_weather_limit =
      Settings::optional_double(theParameters.theVariable + "::wind_cooling_the_weather_limit",
                                WIND_COOLING_THE_WEATHER_LIMIT);

  if (generate_windiness_sentence)
  {
    sentence << construct_windiness_sentence(theParameters, theSpecifiedDay, dayNumber);
  }

  // handle the wind cooling effect
  if (sentence.empty() && generate_wind_cooling_sentence)
  {
    areaString = EMPTY_STRING;
    float temperature = -1.0;

    bool windCoolingTheWeatherInlandMorning =
        (theParameters.theWindspeedInlandMorningMean.value() != kFloatMissing &&
         static_cast<int>(round(theParameters.theWindspeedInlandMorningMean.value())) >=
             wind_cooling_the_weather_limit);
    bool windCoolingTheWeatherCoastalMorning =
        (theParameters.theWindspeedCoastalMorningMean.value() != kFloatMissing &&
         static_cast<int>(round(theParameters.theWindspeedCoastalMorningMean.value())) >=
             wind_cooling_the_weather_limit);
    bool windCoolingTheWeatherInlandAfternoon =
        (theParameters.theWindspeedInlandAfternoonMean.value() != kFloatMissing &&
         static_cast<int>(round(theParameters.theWindspeedInlandAfternoonMean.value())) >=
             wind_cooling_the_weather_limit);
    bool windCoolingTheWeatherCoastalAfternoon =
        (theParameters.theWindspeedCoastalAfternoonMean.value() != kFloatMissing &&
         static_cast<int>(round(theParameters.theWindspeedCoastalAfternoonMean.value())) >=
             wind_cooling_the_weather_limit);

    int temperatureInlandMorning =
        static_cast<int>(round(theParameters.theTemperatureInlandMorningMean.value()));
    int temperatureInlandAfternoon =
        static_cast<int>(round(theParameters.theTemperatureInlandAfternoonMean.value()));
    int temperatureCoastalMorning =
        static_cast<int>(round(theParameters.theTemperatureCoastalMorningMean.value()));
    int temperatureCoastalAfternoon =
        static_cast<int>(round(theParameters.theTemperatureCoastalAfternoonMean.value()));

    bool inlandIncluded =
        windCoolingTheWeatherInlandMorning || windCoolingTheWeatherInlandAfternoon;
    bool coastIncluded =
        windCoolingTheWeatherCoastalMorning || windCoolingTheWeatherCoastalAfternoon;

    if (inlandIncluded && coastIncluded)
    {
      if (windCoolingTheWeatherInlandMorning && windCoolingTheWeatherCoastalMorning &&
          windCoolingTheWeatherInlandAfternoon && windCoolingTheWeatherCoastalAfternoon)
      {
        float morningWind = temperatureInlandMorning > temperatureCoastalMorning
                                ? temperatureInlandMorning
                                : temperatureCoastalMorning;
        float afternoonWind = temperatureInlandAfternoon > temperatureCoastalAfternoon
                                  ? temperatureInlandAfternoon
                                  : temperatureCoastalAfternoon;
        temperature = afternoonWind > morningWind ? afternoonWind : morningWind;
      }
      else if (!windCoolingTheWeatherInlandMorning && windCoolingTheWeatherCoastalMorning &&
               !windCoolingTheWeatherInlandAfternoon && windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureCoastalAfternoon > temperatureCoastalMorning
                          ? temperatureCoastalAfternoon
                          : temperatureCoastalMorning;
        areaString = RANNIKOLLA_WORD;
      }
      else if (!windCoolingTheWeatherInlandMorning && windCoolingTheWeatherCoastalMorning &&
               !windCoolingTheWeatherInlandAfternoon && !windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureCoastalMorning;
        part_of_the_day = aamupaivalla;
        areaString = RANNIKOLLA_WORD;
      }
      else if (!windCoolingTheWeatherInlandMorning && !windCoolingTheWeatherCoastalMorning &&
               !windCoolingTheWeatherInlandAfternoon && windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureCoastalAfternoon;
        part_of_the_day = iltapaivalla;
        areaString = RANNIKOLLA_WORD;
      }
      else if (windCoolingTheWeatherInlandMorning && !windCoolingTheWeatherCoastalMorning &&
               windCoolingTheWeatherInlandAfternoon && !windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureInlandAfternoon > temperatureInlandMorning
                          ? temperatureInlandAfternoon
                          : temperatureInlandMorning;
        areaString = SISAMAASSA_WORD;
      }
      else if (windCoolingTheWeatherInlandMorning && !windCoolingTheWeatherCoastalMorning &&
               !windCoolingTheWeatherInlandAfternoon && !windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureInlandMorning;
        part_of_the_day = aamupaivalla;
        areaString = SISAMAASSA_WORD;
      }
      else if (!windCoolingTheWeatherInlandMorning && !windCoolingTheWeatherCoastalMorning &&
               windCoolingTheWeatherInlandAfternoon && !windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureInlandAfternoon;
        part_of_the_day = iltapaivalla;
        areaString = SISAMAASSA_WORD;
      }
      else if (windCoolingTheWeatherInlandMorning && windCoolingTheWeatherCoastalMorning)
      {
        temperature = temperatureCoastalMorning > temperatureInlandMorning
                          ? temperatureCoastalMorning
                          : temperatureInlandMorning;
        part_of_the_day = aamupaivalla;
      }
      else if (windCoolingTheWeatherInlandAfternoon && windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureCoastalAfternoon > temperatureInlandAfternoon
                          ? temperatureCoastalAfternoon
                          : temperatureInlandAfternoon;
        part_of_the_day = iltapaivalla;
      }
    }
    else if (!inlandIncluded && coastIncluded)
    {
      if (windCoolingTheWeatherCoastalMorning && !windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureCoastalMorning;
        part_of_the_day = aamupaivalla;
      }
      else if (!windCoolingTheWeatherCoastalMorning && windCoolingTheWeatherCoastalAfternoon)
      {
        temperature = temperatureCoastalAfternoon;
        part_of_the_day = iltapaivalla;
      }
      else
      {
        temperature = temperatureCoastalAfternoon > temperatureCoastalMorning
                          ? temperatureCoastalAfternoon
                          : temperatureCoastalMorning;
      }
    }
    else if (inlandIncluded && !coastIncluded)
    {
      if (windCoolingTheWeatherInlandMorning && !windCoolingTheWeatherInlandAfternoon)
      {
        temperature = temperatureInlandMorning;
        part_of_the_day = aamupaivalla;
      }
      else if (!windCoolingTheWeatherInlandMorning && windCoolingTheWeatherInlandAfternoon)
      {
        temperature = temperatureInlandAfternoon;
        part_of_the_day = iltapaivalla;
      }
      else
      {
        temperature = temperatureInlandAfternoon > temperatureInlandMorning
                          ? temperatureInlandAfternoon
                          : temperatureInlandMorning;
      }
    }

    if (inlandIncluded || coastIncluded)
    {
      if (areaString.empty()) areaString = EMPTY_STRING;
      std::string timePhrase(parse_weekday_phrase(dayNumber, part_of_the_day));

      if (temperature > TUULI_KYLMENTAA_SAATA_LOWER_LIMIT &&
          temperature <= TUULI_KYLMENTAA_SAATA_UPPER_LIMIT)
      {
        if (areaString.compare(EMPTY_STRING) == 0 && timePhrase.compare(EMPTY_STRING) == 0)
          sentence << TUULI_SAA_SAAN_TUNTUMAAN_KYLMEMMALTA_PHRASE;
        else if (areaString.compare(EMPTY_STRING) != 0 && timePhrase.compare(EMPTY_STRING) == 0)
          sentence << RANNIKOLLA_TUULI_SAA_SAAN_TUNTUMAAN_KYLMEMMALTA_COMPOSITE_PHRASE
                   << areaString;
        else if (areaString.compare(EMPTY_STRING) == 0 && timePhrase.compare(EMPTY_STRING) != 0)
          sentence << ILTAPAIVALLA_TUULI_SAA_SAAN_TUNTUMAAN_KYLMEMMALTA_COMPOSITE_PHRASE
                   << timePhrase;
        else
          sentence << ILTAPAIVALLA_RANNIKOLLA_TUULI_SAA_SAAN_TUNTUMAAN_KYLMEMMALTA_COMPOSITE_PHRASE
                   << parse_weekday_phrase(dayNumber, part_of_the_day) << areaString;
      }
      else if (temperature > TUULI_VIILENTAA_SAATA_LOWER_LIMIT &&
               temperature <= TUULI_VIILENTAA_SAATA_UPPER_LIMIT)
      {
        if (areaString.compare(EMPTY_STRING) == 0 && timePhrase.compare(EMPTY_STRING) == 0)
          sentence << TUULI_SAA_SAAN_TUNTUMAAN_VIILEAMMALTA_PHRASE;
        else if (areaString.compare(EMPTY_STRING) != 0 && timePhrase.compare(EMPTY_STRING) == 0)
          sentence << RANNIKOLLA_TUULI_SAA_SAAN_TUNTUMAAN_VIILEAMMALTA_COMPOSITE_PHRASE
                   << areaString;
        else if (areaString.compare(EMPTY_STRING) == 0 && timePhrase.compare(EMPTY_STRING) != 0)
          sentence << ILTAPAIVALLA_TUULI_SAA_SAAN_TUNTUMAAN_VIILEAMMALTA_COMPOSITE_PHRASE
                   << timePhrase;
        else
          sentence << ILTAPAIVALLA_RANNIKOLLA_TUULI_SAA_SAAN_TUNTUMAAN_VIILEAMMALTA_COMPOSITE_PHRASE
                   << parse_weekday_phrase(dayNumber, part_of_the_day) << areaString;
      }
    }
  }

  return sentence;
}

const Sentence windchill_sentence(const wind_anomaly_params& theParameters)
{
  Sentence sentence;

  bool inlandIncluded = theParameters.theWindchillInlandMorningMinimum.value() != kFloatMissing ||
                        theParameters.theWindchillInlandAfternoonMinimum.value() != kFloatMissing;
  bool coastIncluded = theParameters.theWindchillCoastalMorningMinimum.value() != kFloatMissing ||
                       theParameters.theWindchillCoastalAfternoonMinimum.value() != kFloatMissing;
  std::string aamupaivalla(theParameters.theMorningWord);
  std::string iltapaivalla(theParameters.theAfternoonWord);

  WeatherResult windChillMorningMean = theParameters.theWindchillInlandMorningMinimum;
  WeatherResult windChillAfternoonMean = theParameters.theWindchillInlandMorningMinimum;
  float windChillAndTemperatureDifferenceMorning = 0.0;
  float windChillAndTemperatureDifferenceAfternoon = 0.0;

  forecast_area_id areaMorning = FULL_AREA;
  forecast_area_id areaAfternoon = FULL_AREA;
  Sentence theSpecifiedDay;
  short dayNumber = 0;
  if (theParameters.thePeriodLength > 24)
  {
    theSpecifiedDay << PeriodPhraseFactory::create("today",
                                                   theParameters.theVariable,
                                                   theParameters.theForecastTime,
                                                   theParameters.thePeriod,
                                                   theParameters.theArea);
    dayNumber = theParameters.thePeriod.localStartTime().GetWeekday();
  }

  if (inlandIncluded && coastIncluded)
  {
    if (theParameters.theWindchillInlandMorningMean.value() != kFloatMissing)
    {
      windChillMorningMean = theParameters.theWindchillInlandMorningMean;
      areaMorning = INLAND_AREA;
      windChillAndTemperatureDifferenceMorning =
          abs(theParameters.theTemperatureInlandMorningMean.value() - windChillMorningMean.value());
    }
    else
    {
      windChillMorningMean = theParameters.theWindchillCoastalMorningMean;
      areaMorning = COASTAL_AREA;
      windChillAndTemperatureDifferenceMorning = abs(
          theParameters.theTemperatureCoastalMorningMean.value() - windChillMorningMean.value());
    }

    if (theParameters.theWindchillInlandAfternoonMean.value() >
        theParameters.theWindchillCoastalAfternoonMean.value())
    {
      windChillAfternoonMean = theParameters.theWindchillInlandAfternoonMean;
      areaAfternoon = INLAND_AREA;
      windChillAndTemperatureDifferenceAfternoon = abs(
          theParameters.theTemperatureInlandAfternoonMean.value() - windChillAfternoonMean.value());
    }
    else
    {
      windChillAfternoonMean = theParameters.theWindchillCoastalAfternoonMean;
      areaAfternoon = COASTAL_AREA;
      windChillAndTemperatureDifferenceAfternoon =
          abs(theParameters.theTemperatureCoastalAfternoonMean.value() -
              windChillAfternoonMean.value());
    }
  }
  else if (inlandIncluded)
  {
    windChillMorningMean = theParameters.theWindchillInlandMorningMean;
    windChillAfternoonMean = theParameters.theWindchillInlandAfternoonMean;
    windChillAndTemperatureDifferenceMorning =
        abs(theParameters.theTemperatureInlandMorningMean.value() - windChillMorningMean.value());
    windChillAndTemperatureDifferenceAfternoon = abs(
        theParameters.theTemperatureInlandAfternoonMean.value() - windChillAfternoonMean.value());
  }
  else if (coastIncluded)
  {
    windChillMorningMean = theParameters.theWindchillCoastalMorningMean;
    windChillAfternoonMean = theParameters.theWindchillCoastalAfternoonMean;
    windChillAndTemperatureDifferenceMorning =
        abs(theParameters.theTemperatureCoastalMorningMean.value() - windChillMorningMean.value());
    windChillAndTemperatureDifferenceAfternoon = abs(
        theParameters.theTemperatureCoastalAfternoonMean.value() - windChillAfternoonMean.value());
  }

  bool morningIncluded = windChillMorningMean.value() != kFloatMissing;
  bool afternoonIncluded = windChillAfternoonMean.value() != kFloatMissing;

  float windChill = 0.0;

  if (morningIncluded && afternoonIncluded)
    windChill = windChillMorningMean.value() < windChillAfternoonMean.value()
                    ? windChillMorningMean.value()
                    : windChillAfternoonMean.value();
  else if (morningIncluded)
    windChill = windChillMorningMean.value();
  else if (afternoonIncluded)
    windChill = windChillAfternoonMean.value();

  bool windChillMorning =
      (morningIncluded && windChillMorningMean.value() >= EXTREME_WINDCHILL_LIMIT &&
       windChillMorningMean.value() <= MILD_WINDCHILL_LIMIT &&
       windChillAndTemperatureDifferenceMorning >= TEMPERATURE_AND_WINDCHILL_DIFFERENCE_LIMIT);
  bool windChillAfternoon =
      (afternoonIncluded && windChillAfternoonMean.value() >= EXTREME_WINDCHILL_LIMIT &&
       windChillAfternoonMean.value() <= MILD_WINDCHILL_LIMIT &&
       windChillAndTemperatureDifferenceAfternoon >= TEMPERATURE_AND_WINDCHILL_DIFFERENCE_LIMIT);
  bool extremelyWindChillMorning =
      (morningIncluded && windChillMorningMean.value() < EXTREME_WINDCHILL_LIMIT &&
       windChillAndTemperatureDifferenceMorning >= TEMPERATURE_AND_WINDCHILL_DIFFERENCE_LIMIT);
  bool extremelyWindChillAfternoon =
      (afternoonIncluded && windChillAfternoonMean.value() < EXTREME_WINDCHILL_LIMIT &&
       windChillAndTemperatureDifferenceAfternoon >= TEMPERATURE_AND_WINDCHILL_DIFFERENCE_LIMIT);

  if (windChill >= EXTREME_WINDCHILL_LIMIT && windChill <= MILD_WINDCHILL_LIMIT)
  {
    if (windChillMorning && windChillAfternoon)
    {
      sentence << PAKKANEN_ON_PUREVAA_PHRASE;
    }
    else if (windChillMorning && !windChillAfternoon)
    {
      std::string areaString(
          (areaMorning == INLAND_AREA
               ? SISAMAASSA_WORD
               : (areaMorning == COASTAL_AREA ? RANNIKOLLA_WORD : EMPTY_STRING)));
      if (areaString.empty())
        sentence << ILTAPAIVALLA_PAKKANEN_ON_PUREVAA_COMPOSITE_PHRASE
                 << parse_weekday_phrase(dayNumber, aamupaivalla);
      else
        sentence << ILTAPAIVALLA_RANNIKOLLA_PAKKANEN_ON_PUREVAA_COMPOSITE_PHRASE
                 << parse_weekday_phrase(dayNumber, aamupaivalla) << areaString;
    }
    else if (!windChillMorning && windChillAfternoon)
    {
      std::string areaString(
          (areaAfternoon == INLAND_AREA
               ? SISAMAASSA_WORD
               : (areaAfternoon == COASTAL_AREA ? RANNIKOLLA_WORD : EMPTY_STRING)));
      if (areaString.empty())
        sentence << ILTAPAIVALLA_PAKKANEN_ON_PUREVAA_COMPOSITE_PHRASE
                 << parse_weekday_phrase(dayNumber, iltapaivalla);
      else
        sentence << ILTAPAIVALLA_RANNIKOLLA_PAKKANEN_ON_PUREVAA_COMPOSITE_PHRASE
                 << parse_weekday_phrase(dayNumber, iltapaivalla) << areaString;
    }
  }
  else if (windChill < EXTREME_WINDCHILL_LIMIT)
  {
    if (windChillMorning && windChillAfternoon)
    {
      sentence << PAKKANEN_ON_ERITTAIN_PUREVAA_PHRASE;
    }
    if (extremelyWindChillMorning && !extremelyWindChillAfternoon)
    {
      std::string areaString(
          (areaMorning == INLAND_AREA
               ? SISAMAASSA_WORD
               : (areaMorning == COASTAL_AREA ? RANNIKOLLA_WORD : EMPTY_STRING)));

      if (areaString.empty())
        sentence << ILTAPAIVALLA_PAKKANEN_ON_ERITTAIN_PUREVAA_COMPOSITE_PHRASE
                 << parse_weekday_phrase(dayNumber, aamupaivalla);
      else
        sentence << ILTAPAIVALLA_RANNIKOLLA_PAKKANEN_ON_ERITTAIN_PUREVAA_COMPOSITE_PHRASE
                 << parse_weekday_phrase(dayNumber, aamupaivalla) << areaString;
    }
    else if (!extremelyWindChillMorning && extremelyWindChillAfternoon)
    {
      std::string areaString(
          (areaAfternoon == INLAND_AREA
               ? SISAMAASSA_WORD
               : (areaAfternoon == COASTAL_AREA ? RANNIKOLLA_WORD : EMPTY_STRING)));
      if (areaString.empty())
        sentence << ILTAPAIVALLA_PAKKANEN_ON_ERITTAIN_PUREVAA_COMPOSITE_PHRASE
                 << parse_weekday_phrase(dayNumber, iltapaivalla);
      else
        sentence << ILTAPAIVALLA_RANNIKOLLA_PAKKANEN_ON_ERITTAIN_PUREVAA_COMPOSITE_PHRASE
                 << parse_weekday_phrase(dayNumber, iltapaivalla) << areaString;
    }
  }

  return sentence;
}

void testWindiness(wind_anomaly_params& parameters, MessageLogger& log)
{
  Sentence theSpecifiedDay;
  short dayNumber = 0;
  if (parameters.thePeriodLength > 24)
  {
    theSpecifiedDay << PeriodPhraseFactory::create("today",
                                                   parameters.theVariable,
                                                   parameters.theForecastTime,
                                                   parameters.thePeriod,
                                                   parameters.theArea);
    dayNumber = parameters.thePeriod.localStartTime().GetWeekday();
  }

  parameters.theWindspeedInlandMorningMinimum = WeatherResult(1.0, 0.0);
  parameters.theWindspeedCoastalMorningMinimum = WeatherResult(1.0, 0.0);
  for (float i = 4.0; i < 13.0; i = i + 0.5)
  {
    parameters.theWindspeedInlandMorningMean = WeatherResult(i, 0.0);
    {
      for (float j = 4.0; j < 13.0; j = j + 0.5)
      {
        parameters.theWindspeedInlandAfternoonMean = WeatherResult(j, 0.0);
        for (float k = 4.0; k < 13.0; k = k + 0.5)
        {
          parameters.theWindspeedCoastalMorningMean = WeatherResult(k, 0.0);
          for (float l = 4.0; l < 13.0; l = l + 0.5)
          {
            parameters.theWindspeedCoastalAfternoonMean = WeatherResult(l, 0.0);
            Sentence sentence;
            sentence << construct_windiness_sentence(parameters, theSpecifiedDay, dayNumber);
            log << "Windspeed InlandM, InlandA, CoastalM, CoastalA: (";
            log << parameters.theWindspeedInlandMorningMean << ",";
            log << parameters.theWindspeedInlandAfternoonMean << ",";
            log << parameters.theWindspeedCoastalMorningMean << ",";
            log << parameters.theWindspeedCoastalAfternoonMean << "): " << endl;
            log << sentence;
          }
        }
      }
    }
  }
}

}  // namespace WindAnomaly

// ----------------------------------------------------------------------
/*!
 * \brief Generate story on wind anomaly
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph WindStory::anomaly() const
{
  using namespace WindAnomaly;

  Paragraph paragraph;
  MessageLogger log("WindStory::anomaly");

  if (itsArea.isNamed())
  {
    std::string nimi(itsArea.name());
    /*
      if(nimi.compare("uusimaa") == 0)
      {
      log << "TEST WINDINESS: ";
      testWindiness(parameters, log);
      }
    */

    log << nimi << endl;
  }

  forecast_season_id theSeasonId =
      isSummerHalf(itsPeriod.localStartTime(), itsVar) ? SUMMER_SEASON : WINTER_SEASON;

  // the anomaly sentence relates always to the upcoming day,
  // so the period is defined to start in the previous day
  log_start_time_and_end_time(log, "the original period: ", itsPeriod);

  TextGenPosixTime periodStartTime(itsPeriod.localStartTime());
  TextGenPosixTime periodEndTime(itsPeriod.localEndTime());
  int periodLength = periodEndTime.DifferenceInHours(periodStartTime);

  // Period generator
  const HourPeriodGenerator periodgenerator(itsPeriod, itsVar + "::day");
  const int ndays = periodgenerator.size();

  log << "Period " << itsPeriod.localStartTime() << "..." << itsPeriod.localEndTime() << " covers "
      << ndays << " days" << endl;

  if (ndays <= 0)
  {
    log << paragraph;
    return paragraph;
  }

  // the last day period
  WeatherPeriod windAnomalyPeriod(periodgenerator.period(ndays));

  log_start_time_and_end_time(log, "wind anomaly period: ", windAnomalyPeriod);

  bool specify_part_of_the_day =
      Settings::optional_bool(itsVar + "::specify_part_of_the_day", true);
  wind_anomaly_params parameters(itsVar,
                                 log,
                                 itsSources,
                                 itsArea,
                                 windAnomalyPeriod,
                                 theSeasonId,
                                 itsForecastTime,
                                 periodLength,
                                 specify_part_of_the_day);

  float coastalPercentage = get_area_percentage(
      itsVar + "::fake::area_percentage", itsArea, WeatherArea::Coast, itsSources, itsPeriod);

  float separate_coastal_area_percentage = Settings::optional_double(
      itsVar + "::separate_coastal_area_percentage", SEPARATE_COASTAL_AREA_PERCENTAGE);

  parameters.theCoastalAndInlandTogetherFlag =
      coastalPercentage > 0 && coastalPercentage < separate_coastal_area_percentage;

  if (parameters.theCoastalAndInlandTogetherFlag)
    log << "Inland and coastal area(" << coastalPercentage << ") not separated!" << endl;

  WeatherArea inlandArea = itsArea;
  inlandArea.type(!parameters.theCoastalAndInlandTogetherFlag ? WeatherArea::Inland
                                                              : WeatherArea::Full);
  WeatherArea coastalArea = itsArea;
  coastalArea.type(!parameters.theCoastalAndInlandTogetherFlag ? WeatherArea::Coast
                                                               : WeatherArea::Full);

  bool morningIncluded = windAnomalyPeriod.localStartTime().GetHour() < 10;
  bool afternoonIncluded = windAnomalyPeriod.localEndTime().GetHour() > 14;
  parameters.theMorningWord = (specify_part_of_the_day ? AAMUPAIVALLA_WORD : EMPTY_STRING);
  parameters.theAfternoonWord = (specify_part_of_the_day ? ILTAPAIVALLA_WORD : EMPTY_STRING);

  if (morningIncluded)
  {
    morning_temperature(itsVar + "::fake::temperature::morning::area",
                        itsSources,
                        itsArea,
                        windAnomalyPeriod,
                        parameters.theTemperatureAreaMorningMinimum,
                        parameters.theTemperatureAreaMorningMaximum,
                        parameters.theTemperatureAreaMorningMean);

    morning_temperature(itsVar + "::fake::temperature::morning::inland",
                        itsSources,
                        inlandArea,
                        windAnomalyPeriod,
                        parameters.theTemperatureInlandMorningMinimum,
                        parameters.theTemperatureInlandMorningMaximum,
                        parameters.theTemperatureInlandMorningMean);

    morning_temperature(itsVar + "::fake::temperature::morning::coast",
                        itsSources,
                        coastalArea,
                        windAnomalyPeriod,
                        parameters.theTemperatureCoastalMorningMinimum,
                        parameters.theTemperatureCoastalMorningMaximum,
                        parameters.theTemperatureCoastalMorningMean);

    // inland, morning, windspeed
    calculate_windspeed_and_chill(parameters, WeatherArea::Inland, true, true);
    // coastal morning, windspeed
    calculate_windspeed_and_chill(parameters, WeatherArea::Coast, true, true);

    // inland, morning, windchill
    calculate_windspeed_and_chill(parameters, WeatherArea::Inland, true, false);
    // coastal morning, windchill
    calculate_windspeed_and_chill(parameters, WeatherArea::Coast, true, false);
  }

  if (afternoonIncluded)
  {
    afternoon_temperature(itsVar + "::fake::temperature::afternoon::area",
                          itsSources,
                          itsArea,
                          windAnomalyPeriod,
                          parameters.theTemperatureAreaAfternoonMinimum,
                          parameters.theTemperatureAreaAfternoonMaximum,
                          parameters.theTemperatureAreaAfternoonMean);

    afternoon_temperature(itsVar + "::fake::temperature::afternoon::inland",
                          itsSources,
                          inlandArea,
                          windAnomalyPeriod,
                          parameters.theTemperatureInlandAfternoonMinimum,
                          parameters.theTemperatureInlandAfternoonMaximum,
                          parameters.theTemperatureInlandAfternoonMean);

    afternoon_temperature(itsVar + "::fake::temperature::afternoon::coast",
                          itsSources,
                          coastalArea,
                          windAnomalyPeriod,
                          parameters.theTemperatureCoastalAfternoonMinimum,
                          parameters.theTemperatureCoastalAfternoonMaximum,
                          parameters.theTemperatureCoastalAfternoonMean);

    // inland afternoon, windspeed
    calculate_windspeed_and_chill(parameters, WeatherArea::Inland, false, true);
    // coastal afternoon, windspeed
    calculate_windspeed_and_chill(parameters, WeatherArea::Coast, false, true);

    // inland afternoon, windchill
    calculate_windspeed_and_chill(parameters, WeatherArea::Inland, false, false);
    // coastal afternoon, windchill
    calculate_windspeed_and_chill(parameters, WeatherArea::Coast, false, false);
  }

  Paragraph paragraphDev;
  Sentence windinessSentence;
  Sentence windChillSentence;

  windinessSentence << windiness_sentence(parameters);

  windChillSentence << windchill_sentence(parameters);

  log_data(parameters);

  log << "windiness: ";
  log << windinessSentence;
  log << "wind chill: ";
  log << windChillSentence;

  paragraph << windinessSentence;
  paragraph << windChillSentence;

  log << paragraph;

  return paragraph;
}

}  // namespace TextGen

// ======================================================================
