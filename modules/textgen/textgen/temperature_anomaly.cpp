// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::TemperatureStory::anomaly
 */
// ======================================================================

#include "TemperatureStory.h"

#include "ClimatologyTools.h"
#include "GridClimatology.h"

#include <calculator/DefaultAcceptor.h>
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "Integer.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "TemperatureStoryTools.h"
#include "AreaTools.h"
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherPeriodTools.h>
#include "SeasonTools.h"
#include <calculator/WeatherResult.h>
#include "PeriodPhraseFactory.h"
#include <calculator/RangeAcceptor.h>

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
#include "WeatherForecast.h"
#include <map>

#include <boost/lexical_cast.hpp>
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>  // includes all needed Boost.Filesystem declarations
namespace boostfs = boost::filesystem;

namespace TextGen
{
namespace TemperatureAnomaly
{
using NFmiStringTools::Convert;
using namespace TextGen;
using namespace TemperatureStoryTools;
using namespace boost;
using namespace std;
using namespace Settings;
using namespace SeasonTools;
using namespace AreaTools;
using MathTools::to_precision;
using Settings::optional_int;
using Settings::optional_bool;
using Settings::optional_string;

#define MILD_TEMPERATURE_LOWER_LIMIT -3.0
#define MILD_TEMPERATURE_UPPER_LIMIT +5.0
#define HOT_WEATHER_LIMIT +25.0
#define GETTING_COOLER_NOTIFICATION_LIMIT +20.0
#define ZERO_DEGREES 0.0
#define VERY_COLD_TEMPERATURE_UPPER_LIMIT -10.0
#define SMALL_CHANGE_LOWER_LIMIT 2.0
#define SMALL_CHANGE_UPPER_LIMIT 3.0
#define MODERATE_CHANGE_LOWER_LIMIT 3.0
#define MODERATE_CHANGE_UPPER_LIMIT 5.0
#define SIGNIFIGANT_CHANGE_LOWER_LIMIT 5.0
#define NOTABLE_TEMPERATURE_CHANGE_LIMIT 2.5

#define SAA_LAUHTUU_PHRASE "saa lauhtuu"
#define SAA_WORD "saa"
#define ON_WORD "on"
#define POIKKEUKSELLISEN_WORD "poikkeuksellisen"
#define KYLMAA_WORD "kylmaa"
#define KOLEAA_WORD "koleaa"
#define HYVIN_WORD "hyvin"
#define VAHAN_WORD "vahan"
#define HUOMATTAVASTI_WORD "huomattavasti"
#define HARVINAISEN_WORD "harvinaisen"
#define LAMMINTA_WORD "lamminta"
#define LEUTOA_WORD "leutoa"

#define SAA_ON_EDELLEEN_LAUHAA_PHRASE "saa on edelleen lauhaa"
#define SAA_LAUHTUU_PHRASE "saa lauhtuu"
#define PAKKANEN_HEIKKENEE_PHRASE "pakkanen heikkenee"
#define KIREA_PAKKANEN_HEIKKENEE_PHRASE "kirea pakkanen heikkenee"
#define PAKKANEN_HELLITTAA_PHRASE "pakkanen hellittaa"
#define KIREA_PAKKANEN_JATKUU_PHRASE "kirea pakkanen jatkuu"
#define PAKKANEN_KIRISTYY_PHRASE "pakkanen kiristyy"
#define HELTEINEN_SAA_JATKUU_PHRASE "helteinen saa jatkuu"
#define VIILEA_SAA_JATKUU_PHRASE "viilea saa jatkuu"
#define KOLEA_SAA_JATKUU_PHRASE "kolea saa jatkuu"
#define SAA_MUUTTUU_HELTEISEKSI_PHRASE "saa muuttuu helteiseksi"
#define SAA_ON_HELTEISTA_PHRASE "saa on helteista"
#define SAA_LAMPENEE_PHRASE "saa lampenee"
#define SAA_LAMPENEE_VAHAN_PHRASE "saa lampenee vahan"
#define SAA_LAMPENEE_HUOMATTAVASTI_PHRASE "saa lampenee huomattavasti"
#define SAA_VIILENEE_PHRASE "saa viilenee"
#define SAA_VIILENEE_VAHAN_PHRASE "saa viilenee vahan"
#define SAA_VIILENEE_HUOMATTAVASTI_PHRASE "saa viilenee huomattavasti"

#define SAA_ON_AJANKOHTAAN_NAHDEN_TAVANOMAISTA_LAMPIMAMPAA_PHRASE \
  "saa on ajankohtaan nahden tavanomaista lampimampaa"
#define ALUEELLA_SAA_ON_AJANKOHTAAN_NAHDEN_TAVANOMAISTA_LAMPIMAMPAA_PHRASE \
  "[alueella] saa on ajankohtaan nahden tavanomaista lampimampaa"
#define MAANANTAINA_SAA_ON_AJANKOHTAAN_NAHDEN_TAVANOMAISTA_LAMPIMAMPAA_PHRASE \
  "[1-na] saa on ajankohtaan nahden tavanomaista lampimampaa"
#define MAANANTAINA_ALUEELLA_SAA_ON_AJANKOHTAAN_NAHDEN_TAVANOMAISTA_LAMPIMAMPAA_PHRASE \
  "[1-na] [alueella] saa on ajankohtaan nahden tavanomaista lampimampaa"

#define SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE "saa on [poikkeuksellisen] [kylmaa]"
#define ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE \
  "[alueella] saa on [poikkeuksellisen] [kylmaa]"
#define MAANANTAINA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE \
  "[1-na] saa on [poikkeuksellisen] [kylmaa]"
#define MAANANTAINA_ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE \
  "[1-na] [alueella] saa on [poikkeuksellisen] [kylmaa]"

#define MAANANTAINA_SAA_ON_EDELLEEN_LAUHAA_COMPOSITE_PHRASE "[1-na] saa on edelleen lauhaa"
#define MAANANTAINA_SAA_LAUHTUU_COMPOSITE_PHRASE "[1-na] saa lauhtuu"
#define MAANANTAINA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE "[1-na] pakkanen heikkenee"
#define MAANANTAINA_KIREA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE "[1-na] kirea pakkanen heikkenee"
#define MAANANTAINA_PAKKANEN_HELLITTAA_COMPOSITE_PHRASE "[1-na] pakkanen hellittaa"
#define MAANANTAINA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE "[1-na] kirea pakkanen jatkuu"
#define MAANANTAINA_PAKKANEN_KIRISTYY_COMPOSITE_PHRASE "[1-na] pakkanen kiristyy"
#define MAANANTAINA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE "[1-na] helteinen saa jatkuu"
#define MAANANTAINA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE "[1-na] viilea saa jatkuu"
#define MAANANTAINA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE "[1-na] kolea saa jatkuu"
#define MAANANTAINA_SAA_MUUTTUU_HELTEISEKSI_COMPOSITE_PHRASE "[1-na] saa muuttuu helteiseksi"
#define MAANANTAINA_SAA_ON_HELTEISTA_COMPOSITE_PHRASE "[1-na] saa on helteista"
#define MAANANTAINA_SAA_LAMPENEE_COMPOSITE_PHRASE "[1-na] saa lampenee"
#define MAANANTAINA_SAA_LAMPENEE_VAHAN_COMPOSITE_PHRASE "[1-na] saa lampenee vahan"
#define MAANANTAINA_SAA_LAMPENEE_HUOMATTAVASTI_COMPOSITE_PHRASE "[1-na] saa lampenee huomattavasti"
#define MAANANTAINA_SAA_VIILENEE_COMPOSITE_PHRASE "[1-na] saa viilenee"
#define MAANANTAINA_SAA_VIILENEE_VAHAN_COMPOSITE_PHRASE "[1-na] saa viilenee vahan"
#define MAANANTAINA_SAA_VIILENEE_HUOMATTAVASTI_COMPOSITE_PHRASE "[1-na] saa viilenee huomattavasti"

#define ALUEELLA_SAA_ON_EDELLEEN_LAUHAA_COMPOSITE_PHRASE "[alueella] saa on edelleen lauhaa"
#define ALUEELLA_SAA_LAUHTUU_COMPOSITE_PHRASE "[alueella] saa lauhtuu"
#define ALUEELLA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE "[alueella] pakkanen heikkenee"
#define ALUEELLA_KIREA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE "[alueella] kirea pakkanen heikkenee"
#define ALUEELLA_PAKKANEN_HELLITTAA_COMPOSITE_PHRASE "[alueella] pakkanen hellittaa"
#define ALUEELLA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE "[alueella] kirea pakkanen jatkuu"
#define ALUEELLA_PAKKANEN_KIRISTYY_COMPOSITE_PHRASE "[alueella] pakkanen kiristyy"
#define ALUEELLA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE "[alueella] helteinen saa jatkuu"
#define ALUEELLA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE "[alueella] viilea saa jatkuu"
#define ALUEELLA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE "[alueella] kolea saa jatkuu"
#define ALUEELLA_SAA_MUUTTUU_HELTEISEKSI_COMPOSITE_PHRASE "[alueella] saa muuttuu helteiseksi"
#define ALUEELLA_SAA_ON_HELTEISTA_COMPOSITE_PHRASE "[alueella] saa on helteista"
#define ALUEELLA_SAA_LAMPENEE_COMPOSITE_PHRASE "[alueella] saa lampenee"
#define ALUEELLA_SAA_LAMPENEE_VAHAN_COMPOSITE_PHRASE "[alueella] saa lampenee vahan"
#define ALUEELLA_SAA_LAMPENEE_HUOMATTAVASTI_COMPOSITE_PHRASE "[alueella] saa lampenee huomattavasti"
#define ALUEELLA_SAA_VIILENEE_COMPOSITE_PHRASE "[alueella] saa viilenee"
#define ALUEELLA_SAA_VIILENEE_VAHAN_COMPOSITE_PHRASE "[alueella] saa viilenee vahan"
#define ALUEELLA_SAA_VIILENEE_HUOMATTAVASTI_COMPOSITE_PHRASE "[alueella] saa viilenee huomattavasti"

#define MAANANTAINA_ALUEELLA_SAA_ON_EDELLEEN_LAUHAA_COMPOSITE_PHRASE \
  "[1-na] [alueella] saa on edelleen lauhaa"
#define MAANANTAINA_ALUEELLA_SAA_LAUHTUU_COMPOSITE_PHRASE "[1-na] [alueella] saa lauhtuu"
#define MAANANTAINA_ALUEELLA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE \
  "[1-na] [alueella] pakkanen heikkenee"
#define MAANANTAINA_ALUEELLA_KIREA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE \
  "[1-na] [alueella] kirea pakkanen heikkenee"
#define MAANANTAINA_ALUEELLA_PAKKANEN_HELLITTAA_COMPOSITE_PHRASE \
  "[1-na] [alueella] pakkanen hellittaa"
#define MAANANTAINA_ALUEELLA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE \
  "[1-na] [alueella] kirea pakkanen jatkuu"
#define MAANANTAINA_ALUEELLA_PAKKANEN_KIRISTYY_COMPOSITE_PHRASE \
  "[1-na] [alueella] pakkanen kiristyy"
#define MAANANTAINA_ALUEELLA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE \
  "[1-na] [alueella] helteinen saa jatkuu"
#define MAANANTAINA_ALUEELLA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE \
  "[1-na] [alueella] viilea saa jatkuu"
#define MAANANTAINA_ALUEELLA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE "[1-na] [alueella] kolea saa jatkuu"
#define MAANANTAINA_ALUEELLA_SAA_MUUTTUU_HELTEISEKSI_COMPOSITE_PHRASE \
  "[1-na] [alueella] saa muuttuu helteiseksi"
#define MAANANTAINA_ALUEELLA_SAA_ON_HELTEISTA_COMPOSITE_PHRASE "[1-na] [alueella] saa on helteista"
#define MAANANTAINA_ALUEELLA_SAA_LAMPENEE_COMPOSITE_PHRASE "[1-na] [alueella] saa lampenee"
#define MAANANTAINA_ALUEELLA_SAA_LAMPENEE_VAHAN_COMPOSITE_PHRASE \
  "[1-na] [alueella] saa lampenee vahan"
#define MAANANTAINA_ALUEELLA_SAA_LAMPENEE_HUOMATTAVASTI_COMPOSITE_PHRASE \
  "[1-na] [alueella] saa lampenee huomattavasti"
#define MAANANTAINA_ALUEELLA_SAA_VIILENEE_COMPOSITE_PHRASE "[1-na] [alueella] saa viilenee"
#define MAANANTAINA_ALUEELLA_SAA_VIILENEE_VAHAN_COMPOSITE_PHRASE \
  "[1-na] [alueella] saa viilenee vahan"
#define MAANANTAINA_ALUEELLA_SAA_VIILENEE_HUOMATTAVASTI_COMPOSITE_PHRASE \
  "[1-na] [alueella] saa viilenee huomattavasti"

enum anomaly_phrase_id
{
  SAA_ON_POIKKEUKSELLISEN_KOLEAA,
  SAA_ON_POIKKEUKSELLISEN_KYLMAA,
  SAA_ON_KOLEAA,
  SAA_ON_HYVIN_KYLMAA,
  SAA_ON_HARVINAISEN_LAMMINTA,
  SAA_ON_HYVIN_LEUTOA,
  SAA_ON_POIKKEUKSLLISEN_LAMMINTA,
  SAA_ON_POIKKEUKSLLISEN_LEUTOA,
  UNDEFINED_ANOMALY_PHRASE_ID
};

enum shortrun_trend_id
{
  SAA_ON_EDELLEEN_LAUHAA,
  SAA_LAUHTUU,
  KIREA_PAKKANEN_HEIKKENEE,
  KIREA_PAKKANEN_HELLITTAA,
  PAKKANEN_HEIKKENEE,
  PAKKANEN_HELLITTAA,
  KIREA_PAKKANEN_JATKUU,
  PAKKANEN_KIRISTYY,
  HELTEINEN_SAA_JATKUU,
  KOLEA_SAA_JATKUU,
  VIILEA_SAA_JATKUU,
  SAA_MUUTTUU_HELTEISEKSI,
  SAA_ON_HELTEISTA,
  SAA_LAMPENEE_HUOMATTAVASTI,
  SAA_LAMPENEE,
  SAA_LAMPENEE_VAHAN,
  SAA_VIILENEE_HUOMATTAVASTI,
  SAA_VIILENEE,
  SAA_VIILENEE_VAHAN,
  UNDEFINED_SHORTRUN_TREND_ID
};

struct temperature_anomaly_params
{
  temperature_anomaly_params(const string& variable,
                             MessageLogger& log,
                             const AnalysisSources& sources,
                             const WeatherArea& area,
                             const WeatherPeriod& period,
                             const WeatherPeriod& dayBeforeDay1Period,
                             const WeatherPeriod& day1Period,
                             const WeatherPeriod& day2Period,
                             const WeatherPeriod& dayAfterDay2Period,
                             const forecast_season_id& season,
                             const TextGenPosixTime& forecastTime,
                             const short& periodLength)
      : theVariable(variable),
        theLog(log),
        theSources(sources),
        theArea(area),
        thePeriod(period),
        theDayBeforeDay1Period(dayBeforeDay1Period),
        theDay1Period(day1Period),
        theDay2Period(day2Period),
        theDayAfterDay2Period(dayAfterDay2Period),
        theSeason(season),
        theForecastTime(forecastTime),
        thePeriodLength(periodLength),
        theAnomalyPhrase(UNDEFINED_ANOMALY_PHRASE_ID),
        theShortrunTrend(UNDEFINED_SHORTRUN_TREND_ID),
        theFakeVariable(""),
        theGrowingSeasonUnderway(false),
        theDayBeforeDay1TemperatureAreaAfternoonMinimum(kFloatMissing, 0),
        theDayBeforeDay1TemperatureAreaAfternoonMean(kFloatMissing, 0),
        theDayBeforeDay1TemperatureAreaAfternoonMaximum(kFloatMissing, 0),
        theDay1TemperatureAreaAfternoonMinimum(kFloatMissing, 0),
        theDay1TemperatureAreaAfternoonMean(kFloatMissing, 0),
        theDay1TemperatureAreaAfternoonMaximum(kFloatMissing, 0),
        theDay2TemperatureAreaAfternoonMinimum(kFloatMissing, 0),
        theDay2TemperatureAreaAfternoonMean(kFloatMissing, 0),
        theDay2TemperatureAreaAfternoonMaximum(kFloatMissing, 0),
        theDayAfterDay2TemperatureAreaAfternoonMinimum(kFloatMissing, 0),
        theDayAfterDay2TemperatureAreaAfternoonMean(kFloatMissing, 0),
        theDayAfterDay2TemperatureAreaAfternoonMaximum(kFloatMissing, 0)
  {
  }

  const string& theVariable;
  MessageLogger& theLog;
  const AnalysisSources& theSources;
  const WeatherArea& theArea;
  const WeatherPeriod& thePeriod;
  const WeatherPeriod& theDayBeforeDay1Period;
  const WeatherPeriod& theDay1Period;
  const WeatherPeriod& theDay2Period;
  const WeatherPeriod& theDayAfterDay2Period;
  const forecast_season_id& theSeason;
  const TextGenPosixTime& theForecastTime;
  const short& thePeriodLength;
  anomaly_phrase_id theAnomalyPhrase;
  shortrun_trend_id theShortrunTrend;
  string theFakeVariable;
  bool theGrowingSeasonUnderway;
  WeatherResult theDayBeforeDay1TemperatureAreaAfternoonMinimum;
  WeatherResult theDayBeforeDay1TemperatureAreaAfternoonMean;
  WeatherResult theDayBeforeDay1TemperatureAreaAfternoonMaximum;
  WeatherResult theDay1TemperatureAreaAfternoonMinimum;
  WeatherResult theDay1TemperatureAreaAfternoonMean;
  WeatherResult theDay1TemperatureAreaAfternoonMaximum;
  WeatherResult theDay2TemperatureAreaAfternoonMinimum;
  WeatherResult theDay2TemperatureAreaAfternoonMean;
  WeatherResult theDay2TemperatureAreaAfternoonMaximum;
  WeatherResult theDayAfterDay2TemperatureAreaAfternoonMinimum;
  WeatherResult theDayAfterDay2TemperatureAreaAfternoonMean;
  WeatherResult theDayAfterDay2TemperatureAreaAfternoonMaximum;
};

void log_data(const temperature_anomaly_params& theParameters)
{
  theParameters.theLog << "theDayBeforeDay1TemperatureAreaAfternoonMinimum: "
                       << theParameters.theDayBeforeDay1TemperatureAreaAfternoonMinimum << endl;
  theParameters.theLog << "theDayBeforeDay1TemperatureAreaAfternoonMean: "
                       << theParameters.theDayBeforeDay1TemperatureAreaAfternoonMean << endl;
  theParameters.theLog << "theDayBeforeDay1TemperatureAreaAfternoonMaximum: "
                       << theParameters.theDayBeforeDay1TemperatureAreaAfternoonMaximum << endl;
  theParameters.theLog << "theDay1TemperatureAreaAfternoonMinimum: "
                       << theParameters.theDay1TemperatureAreaAfternoonMinimum << endl;
  theParameters.theLog << "theDay1TemperatureAreaAfternoonMean: "
                       << theParameters.theDay1TemperatureAreaAfternoonMean << endl;
  theParameters.theLog << "theDay1TemperatureAreaAfternoonMaximum: "
                       << theParameters.theDay1TemperatureAreaAfternoonMaximum << endl;

  theParameters.theLog << "theDay2TemperatureAreaAfternoonMinimum: "
                       << theParameters.theDay2TemperatureAreaAfternoonMinimum << endl;
  theParameters.theLog << "theDay2TemperatureAreaAfternoonMean: "
                       << theParameters.theDay2TemperatureAreaAfternoonMean << endl;
  theParameters.theLog << "theDay2TemperatureAreaAfternoonMaximum: "
                       << theParameters.theDay2TemperatureAreaAfternoonMaximum << endl;
  theParameters.theLog << "theDayAfterDay2TemperatureAreaAfternoonMinimum: "
                       << theParameters.theDayAfterDay2TemperatureAreaAfternoonMinimum << endl;
  theParameters.theLog << "theDayAfterDay2TemperatureAreaAfternoonMean: "
                       << theParameters.theDayAfterDay2TemperatureAreaAfternoonMean << endl;
  theParameters.theLog << "theDayAfterDay2TemperatureAreaAfternoonMaximum: "
                       << theParameters.theDayAfterDay2TemperatureAreaAfternoonMaximum << endl;
}

void log_start_time_and_end_time(MessageLogger& theLog,
                                 const std::string& theLogMessage,
                                 const WeatherPeriod& thePeriod)
{
  theLog << NFmiStringTools::Convert(theLogMessage) << thePeriod.localStartTime() << " ... "
         << thePeriod.localEndTime() << endl;
}

std::string period2string(const WeatherPeriod& period)
{
  std::stringstream ss;

  ss << period.localStartTime() << "..." << period.localEndTime();

  return ss.str();
}

void log_daily_factiles_for_period(MessageLogger& theLog,
                                   const string& theVariable,
                                   const AnalysisSources& theSources,
                                   const WeatherArea& theArea,
                                   const std::string& theLogMessage,
                                   const WeatherPeriod& thePeriod,
                                   const fractile_type_id& theFractileType)
{
  TextGenPosixTime startTime(thePeriod.localStartTime());
  TextGenPosixTime endTime(thePeriod.localStartTime());

  startTime.SetHour(6);
  startTime.SetMin(0);
  startTime.SetSec(0);
  endTime.SetHour(18);
  endTime.SetMin(0);
  endTime.SetSec(0);

  theLog << theLogMessage;

  while (startTime.GetJulianDay() < thePeriod.localEndTime().GetJulianDay())
  {
    WeatherPeriod fractilePeriod(startTime, endTime);

    WeatherResult fractile02Temperature = get_fractile_temperature(
        theVariable, FRACTILE_02, theSources, theArea, fractilePeriod, theFractileType);
    WeatherResult fractile12Temperature = get_fractile_temperature(
        theVariable, FRACTILE_12, theSources, theArea, fractilePeriod, theFractileType);
    WeatherResult fractile37Temperature = get_fractile_temperature(
        theVariable, FRACTILE_37, theSources, theArea, fractilePeriod, theFractileType);
    WeatherResult fractile50Temperature = get_fractile_temperature(
        theVariable, FRACTILE_50, theSources, theArea, fractilePeriod, theFractileType);
    WeatherResult fractile63Temperature = get_fractile_temperature(
        theVariable, FRACTILE_63, theSources, theArea, fractilePeriod, theFractileType);
    WeatherResult fractile88Temperature = get_fractile_temperature(
        theVariable, FRACTILE_88, theSources, theArea, fractilePeriod, theFractileType);
    WeatherResult fractile98Temperature = get_fractile_temperature(
        theVariable, FRACTILE_98, theSources, theArea, fractilePeriod, theFractileType);

    theLog << "date = " << startTime << "..." << endTime << endl;
    theLog << "F02 = " << fractile02Temperature << endl;
    theLog << "F12 = " << fractile12Temperature << endl;
    theLog << "F37 = " << fractile37Temperature << endl;
    theLog << "F50 = " << fractile50Temperature << endl;
    theLog << "F63 = " << fractile63Temperature << endl;
    theLog << "F88 = " << fractile88Temperature << endl;
    theLog << "F98 = " << fractile98Temperature << endl;

    startTime.ChangeByDays(1);
    endTime.ChangeByDays(1);
  }
}

const Sentence temperature_anomaly_sentence(temperature_anomaly_params& theParameters,
                                            const float& fractile02Share,
                                            const float& fractile12Share,
                                            const float& fractile88Share,
                                            const float& fractile98Share,
                                            const WeatherPeriod& thePeriod)
{
  Sentence sentence;

  Sentence theSpecifiedDay;

  if (theParameters.thePeriodLength > 24)
  {
    theSpecifiedDay << PeriodPhraseFactory::create("today",
                                                   theParameters.theVariable,
                                                   theParameters.theForecastTime,
                                                   thePeriod,
                                                   theParameters.theArea);
  }

  Sentence theAreaPhrase;

  if (theParameters.theArea.type() == WeatherArea::Northern)
    theAreaPhrase << ALUEEN_POHJOISOSASSA_PHRASE;
  else if (theParameters.theArea.type() == WeatherArea::Southern)
    theAreaPhrase << ALUEEN_ETELAOSASSA_PHRASE;
  else if (theParameters.theArea.type() == WeatherArea::Eastern)
    theAreaPhrase << ALUEEN_ITAOSASSA_PHRASE;
  else if (theParameters.theArea.type() == WeatherArea::Western)
    theAreaPhrase << ALUEEN_LANSIOSASSA_PHRASE;

  float adequateShare(80.0);

  if (fractile02Share >= adequateShare)
  {
    if (theSpecifiedDay.size() == 0)
    {
      if (theAreaPhrase.size() > 0)
      {
        sentence << ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << theAreaPhrase
                 << POIKKEUKSELLISEN_WORD
                 << (theParameters.theSeason == SUMMER_SEASON ? KOLEAA_WORD : KYLMAA_WORD);
      }
      else
      {
        sentence << SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << POIKKEUKSELLISEN_WORD
                 << (theParameters.theSeason == SUMMER_SEASON ? KOLEAA_WORD : KYLMAA_WORD);
      }
    }
    else
    {
      if (theAreaPhrase.size() > 0)
      {
        sentence << MAANANTAINA_ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE
                 << theSpecifiedDay << theAreaPhrase << POIKKEUKSELLISEN_WORD
                 << (theParameters.theSeason == SUMMER_SEASON ? KOLEAA_WORD : KYLMAA_WORD);
      }
      else
      {
        sentence << MAANANTAINA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << theSpecifiedDay
                 << POIKKEUKSELLISEN_WORD
                 << (theParameters.theSeason == SUMMER_SEASON ? KOLEAA_WORD : KYLMAA_WORD);
      }
    }

    theParameters.theAnomalyPhrase =
        (theParameters.theSeason == SUMMER_SEASON ? SAA_ON_POIKKEUKSELLISEN_KOLEAA
                                                  : SAA_ON_POIKKEUKSELLISEN_KYLMAA);
  }
  else if (fractile12Share >= adequateShare)
  {
    if (theSpecifiedDay.size() == 0)
    {
      if (theAreaPhrase.size() > 0)
      {
        sentence << ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << theAreaPhrase
                 << EMPTY_STRING
                 << (theParameters.theSeason == SUMMER_SEASON ? KOLEAA_WORD : KYLMAA_WORD);
      }
      else
      {
        sentence << SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << EMPTY_STRING
                 << (theParameters.theSeason == SUMMER_SEASON ? KOLEAA_WORD : KYLMAA_WORD);
      }
    }
    else
    {
      if (theAreaPhrase.size() > 0)
      {
        sentence << MAANANTAINA_ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE
                 << theSpecifiedDay << theAreaPhrase << EMPTY_STRING
                 << (theParameters.theSeason == SUMMER_SEASON ? KOLEAA_WORD : KYLMAA_WORD);
      }
      else
      {
        sentence << MAANANTAINA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << theSpecifiedDay
                 << EMPTY_STRING
                 << (theParameters.theSeason == SUMMER_SEASON ? KOLEAA_WORD : KYLMAA_WORD);
      }
    }

    theParameters.theAnomalyPhrase =
        (theParameters.theSeason == SUMMER_SEASON ? SAA_ON_KOLEAA : SAA_ON_HYVIN_KYLMAA);
  }
  else if (fractile98Share >= adequateShare)
  {
    if (theSpecifiedDay.size() == 0)
    {
      if (theAreaPhrase.size() > 0)
      {
        sentence << ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << theAreaPhrase
                 << POIKKEUKSELLISEN_WORD << ((theParameters.theSeason == SUMMER_SEASON ||
                                               theParameters.theGrowingSeasonUnderway)
                                                  ? LAMMINTA_WORD
                                                  : LEUTOA_WORD);
      }
      else
      {
        sentence << SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << POIKKEUKSELLISEN_WORD
                 << ((theParameters.theSeason == SUMMER_SEASON ||
                      theParameters.theGrowingSeasonUnderway)
                         ? LAMMINTA_WORD
                         : LEUTOA_WORD);
      }
    }
    else
    {
      if (theAreaPhrase.size() > 0)
      {
        sentence << MAANANTAINA_ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE
                 << theSpecifiedDay << theAreaPhrase << POIKKEUKSELLISEN_WORD
                 << ((theParameters.theSeason == SUMMER_SEASON ||
                      theParameters.theGrowingSeasonUnderway)
                         ? LAMMINTA_WORD
                         : LEUTOA_WORD);
      }
      else
      {
        sentence << MAANANTAINA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << theSpecifiedDay
                 << POIKKEUKSELLISEN_WORD << ((theParameters.theSeason == SUMMER_SEASON ||
                                               theParameters.theGrowingSeasonUnderway)
                                                  ? LAMMINTA_WORD
                                                  : LEUTOA_WORD);
      }
    }

    theParameters.theAnomalyPhrase =
        ((theParameters.theSeason == SUMMER_SEASON || theParameters.theGrowingSeasonUnderway)
             ? SAA_ON_POIKKEUKSLLISEN_LAMMINTA
             : SAA_ON_POIKKEUKSLLISEN_LEUTOA);
  }
  else if (fractile88Share >= adequateShare)
  {
    if (theSpecifiedDay.size() == 0)
    {
      if (theParameters.theSeason == SUMMER_SEASON || theParameters.theGrowingSeasonUnderway)
      {
        if (theAreaPhrase.size() > 0)
          sentence << ALUEELLA_SAA_ON_AJANKOHTAAN_NAHDEN_TAVANOMAISTA_LAMPIMAMPAA_PHRASE
                   << theAreaPhrase;
        else
          sentence << SAA_ON_AJANKOHTAAN_NAHDEN_TAVANOMAISTA_LAMPIMAMPAA_PHRASE;
      }
      else
      {
        if (theAreaPhrase.size() > 0)
        {
          sentence << ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << theAreaPhrase
                   << HYVIN_WORD << LEUTOA_WORD;
        }
        else
        {
          sentence << SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << HYVIN_WORD << LEUTOA_WORD;
        }
      }
    }
    else
    {
      if (theParameters.theSeason == SUMMER_SEASON || theParameters.theGrowingSeasonUnderway)
      {
        if (theAreaPhrase.size() > 0)
        {
          sentence << MAANANTAINA_ALUEELLA_SAA_ON_AJANKOHTAAN_NAHDEN_TAVANOMAISTA_LAMPIMAMPAA_PHRASE
                   << theSpecifiedDay << theAreaPhrase;
        }
        else
        {
          sentence << MAANANTAINA_SAA_ON_AJANKOHTAAN_NAHDEN_TAVANOMAISTA_LAMPIMAMPAA_PHRASE
                   << theSpecifiedDay;
        }
      }
      else
      {
        if (theAreaPhrase.size() > 0)
        {
          sentence << MAANANTAINA_ALUEELLA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE
                   << theSpecifiedDay << theAreaPhrase << HYVIN_WORD << LEUTOA_WORD;
        }
        else
        {
          sentence << MAANANTAINA_SAA_ON_POIKKEUKSELLISEN_KYLMAA_COMPOSITE_PHRASE << theSpecifiedDay
                   << HYVIN_WORD << LEUTOA_WORD;
        }
      }
    }

    theParameters.theAnomalyPhrase =
        (theParameters.theSeason == SUMMER_SEASON ? SAA_ON_HARVINAISEN_LAMMINTA
                                                  : SAA_ON_HYVIN_LEUTOA);
  }

  return sentence;
}

const Sentence get_shortruntrend_sentence(const std::string& theDayAndAreaIncludedCompositePhrase,
                                          const std::string& theDayIncludedCompositePhrase,
                                          const std::string& theAreaIncludedCompositePhrase,
                                          const std::string& theTemperatureSentence,
                                          const Sentence& theSpecifiedDay,
                                          const Sentence& theAreaPhrase)
{
  Sentence sentence;

  if (theSpecifiedDay.size() > 0 && theAreaPhrase.size() > 0)
  {
    sentence << theDayAndAreaIncludedCompositePhrase << theSpecifiedDay << theAreaPhrase
             << theTemperatureSentence;
  }
  else if (theSpecifiedDay.size() > 0)
  {
    sentence << theDayIncludedCompositePhrase << theSpecifiedDay << theTemperatureSentence;
  }
  else if (theAreaPhrase.size() > 0)
  {
    sentence << theAreaIncludedCompositePhrase << theAreaPhrase << theTemperatureSentence;
  }
  else
  {
    sentence << theTemperatureSentence;
  }

  return sentence;
}

const Sentence temperature_shortruntrend_sentence(temperature_anomaly_params& theParameters,
                                                  fractile_type_id theFractileType)
{
  Sentence sentence;

  // 20160803: in wintertime use mean temperatures in summertime max temperatures
  double dayBeforeDay1Temperature =
      (theParameters.theSeason == WINTER_SEASON
           ? theParameters.theDayBeforeDay1TemperatureAreaAfternoonMean.value()
           : theParameters.theDayBeforeDay1TemperatureAreaAfternoonMaximum.value());
  double day1Temperature = (theParameters.theSeason == WINTER_SEASON
                                ? theParameters.theDay1TemperatureAreaAfternoonMean.value()
                                : theParameters.theDay1TemperatureAreaAfternoonMaximum.value());
  double day2Temperature = (theParameters.theSeason == WINTER_SEASON
                                ? theParameters.theDay2TemperatureAreaAfternoonMean.value()
                                : theParameters.theDay2TemperatureAreaAfternoonMaximum.value());
  double dayAfterDay2Temperature =
      (theParameters.theSeason == WINTER_SEASON
           ? theParameters.theDayAfterDay2TemperatureAreaAfternoonMean.value()
           : theParameters.theDayAfterDay2TemperatureAreaAfternoonMaximum.value());

  float temperatureDifferenceDay1Day2 = abs(day2Temperature - day1Temperature);
  float temperatureDifferenceDay1DayAfterDay2 = abs(dayAfterDay2Temperature - day1Temperature);
  float temperatureDifferenceDayBeforeDay1Day2 = abs(day2Temperature - dayBeforeDay1Temperature);
  float temperatureDifferenceDayBeforeDay1DayAfterDay2 =
      abs(dayAfterDay2Temperature - dayBeforeDay1Temperature);

  bool smallChange = temperatureDifferenceDay1Day2 >= SMALL_CHANGE_LOWER_LIMIT &&
                     temperatureDifferenceDay1Day2 < SMALL_CHANGE_UPPER_LIMIT &&
                     temperatureDifferenceDay1DayAfterDay2 >= SMALL_CHANGE_LOWER_LIMIT &&
                     temperatureDifferenceDay1DayAfterDay2 < SMALL_CHANGE_UPPER_LIMIT &&
                     temperatureDifferenceDayBeforeDay1Day2 >= SMALL_CHANGE_LOWER_LIMIT &&
                     temperatureDifferenceDayBeforeDay1Day2 < SMALL_CHANGE_UPPER_LIMIT &&
                     temperatureDifferenceDayBeforeDay1DayAfterDay2 >= SMALL_CHANGE_LOWER_LIMIT &&
                     temperatureDifferenceDayBeforeDay1DayAfterDay2 < SMALL_CHANGE_UPPER_LIMIT;
  bool moderateChange =
      temperatureDifferenceDay1Day2 >= MODERATE_CHANGE_LOWER_LIMIT &&
      temperatureDifferenceDay1Day2 < MODERATE_CHANGE_UPPER_LIMIT &&
      temperatureDifferenceDay1DayAfterDay2 >= MODERATE_CHANGE_LOWER_LIMIT &&
      temperatureDifferenceDay1DayAfterDay2 < MODERATE_CHANGE_UPPER_LIMIT &&
      temperatureDifferenceDayBeforeDay1Day2 >= MODERATE_CHANGE_LOWER_LIMIT &&
      temperatureDifferenceDayBeforeDay1Day2 < MODERATE_CHANGE_UPPER_LIMIT &&
      temperatureDifferenceDayBeforeDay1DayAfterDay2 >= MODERATE_CHANGE_LOWER_LIMIT &&
      temperatureDifferenceDayBeforeDay1DayAfterDay2 < MODERATE_CHANGE_UPPER_LIMIT;
  bool signifigantChange =
      temperatureDifferenceDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
      temperatureDifferenceDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
      temperatureDifferenceDayBeforeDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
      temperatureDifferenceDayBeforeDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT;
  bool temperatureGettingLower = day2Temperature < day1Temperature &&
                                 dayAfterDay2Temperature < day1Temperature &&
                                 day2Temperature < dayBeforeDay1Temperature &&
                                 dayAfterDay2Temperature < dayBeforeDay1Temperature;

  // kova pakkanen: F12,5 fractile on 1. Feb 12:00
  TextGenPosixTime veryColdRefTime(theParameters.theForecastTime.GetYear(), 2, 1, 12, 0, 0);
  WeatherPeriod veryColdWeatherPeriod(veryColdRefTime, veryColdRefTime);

  WeatherResult wr = get_fractile_temperature(theParameters.theVariable,
                                              FRACTILE_12,
                                              theParameters.theSources,
                                              theParameters.theArea,
                                              veryColdWeatherPeriod,
                                              theFractileType);

  float veryColdTemperature = wr.value() < VERY_COLD_TEMPERATURE_UPPER_LIMIT
                                  ? wr.value()
                                  : VERY_COLD_TEMPERATURE_UPPER_LIMIT;

  WeatherPeriod fractileTemperatureDay1Period(get_afternoon_period(
      theParameters.theVariable, theParameters.theDay1Period.localStartTime()));
  WeatherPeriod fractileTemperatureDayBeforeDay1Period(get_afternoon_period(
      theParameters.theVariable, theParameters.theDayBeforeDay1Period.localStartTime()));
  WeatherPeriod fractileTemperatureDay2Period(get_afternoon_period(
      theParameters.theVariable, theParameters.theDay2Period.localStartTime()));
  WeatherPeriod fractileTemperatureDayAfterDay2Period(get_afternoon_period(
      theParameters.theVariable, theParameters.theDayAfterDay2Period.localStartTime()));

  WeatherResult fractile37TemperatureDay1 = get_fractile_temperature(theParameters.theVariable,
                                                                     FRACTILE_37,
                                                                     theParameters.theSources,
                                                                     theParameters.theArea,
                                                                     fractileTemperatureDay1Period,
                                                                     theFractileType);

  WeatherResult fractile12TemperatureDay1 = get_fractile_temperature(theParameters.theVariable,
                                                                     FRACTILE_12,
                                                                     theParameters.theSources,
                                                                     theParameters.theArea,
                                                                     fractileTemperatureDay1Period,
                                                                     theFractileType);

  WeatherResult fractile63TemperatureDay1 = get_fractile_temperature(theParameters.theVariable,
                                                                     FRACTILE_63,
                                                                     theParameters.theSources,
                                                                     theParameters.theArea,
                                                                     fractileTemperatureDay1Period,
                                                                     theFractileType);
  WeatherResult fractile37TemperatureDayBeforeDay1 =
      get_fractile_temperature(theParameters.theVariable,
                               FRACTILE_37,
                               theParameters.theSources,
                               theParameters.theArea,
                               fractileTemperatureDayBeforeDay1Period,
                               theFractileType);

  WeatherResult fractile12TemperatureDayBeforeDay1 =
      get_fractile_temperature(theParameters.theVariable,
                               FRACTILE_12,
                               theParameters.theSources,
                               theParameters.theArea,
                               fractileTemperatureDayBeforeDay1Period,
                               theFractileType);

  WeatherResult fractile63TemperatureDayBeforeDay1 =
      get_fractile_temperature(theParameters.theVariable,
                               FRACTILE_63,
                               theParameters.theSources,
                               theParameters.theArea,
                               fractileTemperatureDayBeforeDay1Period,
                               theFractileType);

  WeatherResult fractile37TemperatureDay2 = get_fractile_temperature(theParameters.theVariable,
                                                                     FRACTILE_37,
                                                                     theParameters.theSources,
                                                                     theParameters.theArea,
                                                                     fractileTemperatureDay2Period,
                                                                     theFractileType);

  WeatherResult fractile12TemperatureDay2 = get_fractile_temperature(theParameters.theVariable,
                                                                     FRACTILE_12,
                                                                     theParameters.theSources,
                                                                     theParameters.theArea,
                                                                     fractileTemperatureDay2Period,
                                                                     theFractileType);

  WeatherResult fractile63TemperatureDay2 = get_fractile_temperature(theParameters.theVariable,
                                                                     FRACTILE_63,
                                                                     theParameters.theSources,
                                                                     theParameters.theArea,
                                                                     fractileTemperatureDay2Period,
                                                                     theFractileType);

  WeatherResult fractile37TemperatureDayAfterDay2 =
      get_fractile_temperature(theParameters.theVariable,
                               FRACTILE_37,
                               theParameters.theSources,
                               theParameters.theArea,
                               fractileTemperatureDayAfterDay2Period,
                               theFractileType);

  WeatherResult fractile12TemperatureDayAfterDay2 =
      get_fractile_temperature(theParameters.theVariable,
                               FRACTILE_12,
                               theParameters.theSources,
                               theParameters.theArea,
                               fractileTemperatureDayAfterDay2Period,
                               theFractileType);

  WeatherResult fractile63TemperatureDayAfterDay2 =
      get_fractile_temperature(theParameters.theVariable,
                               FRACTILE_63,
                               theParameters.theSources,
                               theParameters.theArea,
                               fractileTemperatureDayAfterDay2Period,
                               theFractileType);

  theParameters.theLog << "thePeriod: " << period2string(theParameters.thePeriod) << std::endl;
  theParameters.theLog << "fractileTemperatureDayBeforeDay1Period: "
                       << period2string(fractileTemperatureDayBeforeDay1Period) << std::endl;
  theParameters.theLog << "fractileTemperatureDay1Period: "
                       << period2string(fractileTemperatureDay1Period) << std::endl;
  theParameters.theLog << "fractileTemperatureDay2Period: "
                       << period2string(fractileTemperatureDay2Period) << std::endl;
  theParameters.theLog << "fractileTemperatureDayAfterDay2Period: "
                       << period2string(fractileTemperatureDayAfterDay2Period) << std::endl;

  theParameters.theLog << "F12 temperature for period "
                       << period2string(fractileTemperatureDayBeforeDay1Period) << ": "
                       << fractile12TemperatureDayBeforeDay1.value() << endl;
  theParameters.theLog << "F37 temperature for period "
                       << period2string(fractileTemperatureDayBeforeDay1Period) << ": "
                       << fractile37TemperatureDayBeforeDay1.value() << endl;
  theParameters.theLog << "F63 temperature for period "
                       << period2string(fractileTemperatureDayBeforeDay1Period) << ": "
                       << fractile63TemperatureDayBeforeDay1.value() << endl;
  theParameters.theLog << "F12 temperature for period "
                       << period2string(fractileTemperatureDay1Period) << ": "
                       << fractile12TemperatureDay1.value() << endl;
  theParameters.theLog << "F37 temperature for period "
                       << period2string(fractileTemperatureDay1Period) << ": "
                       << fractile37TemperatureDay1.value() << endl;
  theParameters.theLog << "F63 temperature for period "
                       << period2string(fractileTemperatureDay1Period) << ": "
                       << fractile63TemperatureDay1.value() << endl;
  theParameters.theLog << "F12 temperature for period "
                       << period2string(fractileTemperatureDay2Period) << ": "
                       << fractile12TemperatureDay2.value() << endl;
  theParameters.theLog << "F37 temperature for period "
                       << period2string(fractileTemperatureDay2Period) << ": "
                       << fractile37TemperatureDay2.value() << endl;
  theParameters.theLog << "F63 temperature for period "
                       << period2string(fractileTemperatureDay2Period) << ": "
                       << fractile63TemperatureDay2.value() << endl;
  theParameters.theLog << "F12 temperature for period "
                       << period2string(fractileTemperatureDayAfterDay2Period) << ": "
                       << fractile12TemperatureDayAfterDay2.value() << endl;
  theParameters.theLog << "F37 temperature for period "
                       << period2string(fractileTemperatureDayAfterDay2Period) << ": "
                       << fractile37TemperatureDayAfterDay2.value() << endl;
  theParameters.theLog << "F63 temperature for period "
                       << period2string(fractileTemperatureDayAfterDay2Period) << ": "
                       << fractile63TemperatureDayAfterDay2.value() << endl;

  Sentence theSpecifiedDay;

  if (theParameters.thePeriodLength > 24)
  {
    theSpecifiedDay << PeriodPhraseFactory::create("today",
                                                   theParameters.theVariable,
                                                   theParameters.theForecastTime,
                                                   theParameters.theDay2Period,
                                                   theParameters.theArea);
  }

  Sentence theAreaPhrase;

  if (theParameters.theArea.type() == WeatherArea::Northern)
    theAreaPhrase << ALUEEN_POHJOISOSASSA_PHRASE;
  else if (theParameters.theArea.type() == WeatherArea::Southern)
    theAreaPhrase << ALUEEN_ETELAOSASSA_PHRASE;
  else if (theParameters.theArea.type() == WeatherArea::Eastern)
    theAreaPhrase << ALUEEN_ITAOSASSA_PHRASE;
  else if (theParameters.theArea.type() == WeatherArea::Western)
    theAreaPhrase << ALUEEN_LANSIOSASSA_PHRASE;

  if (theParameters.theSeason == WINTER_SEASON)
  {
    // saa on edelleen lauhaa
    // saa lauhtuu
    // kirea pakkanen heikkenee
    // kirea pakkanen hellittaa*
    // pakkanen heikkenee
    // pakkanen hellittaa*
    // kirea pakkanen jatkuu
    // pakkanen kiristyy

    if (temperatureGettingLower == false)  // day2Temperature >= day1Temperature)
    {
      if (day1Temperature > MILD_TEMPERATURE_LOWER_LIMIT &&
          day1Temperature < MILD_TEMPERATURE_UPPER_LIMIT &&
          dayBeforeDay1Temperature > MILD_TEMPERATURE_LOWER_LIMIT &&
          dayBeforeDay1Temperature < MILD_TEMPERATURE_UPPER_LIMIT &&
          day2Temperature > MILD_TEMPERATURE_LOWER_LIMIT &&
          day2Temperature < MILD_TEMPERATURE_UPPER_LIMIT &&
          dayAfterDay2Temperature > MILD_TEMPERATURE_LOWER_LIMIT &&
          dayAfterDay2Temperature < MILD_TEMPERATURE_UPPER_LIMIT &&
          day2Temperature > fractile63TemperatureDay2.value() &&
          dayAfterDay2Temperature > fractile63TemperatureDayAfterDay2.value() &&
          !theParameters.theGrowingSeasonUnderway)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_SAA_ON_EDELLEEN_LAUHAA_COMPOSITE_PHRASE,
            MAANANTAINA_SAA_ON_EDELLEEN_LAUHAA_COMPOSITE_PHRASE,
            ALUEELLA_SAA_ON_EDELLEEN_LAUHAA_COMPOSITE_PHRASE,
            SAA_ON_EDELLEEN_LAUHAA_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = SAA_ON_EDELLEEN_LAUHAA;
      }
      else if (temperatureDifferenceDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDayBeforeDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDayBeforeDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               dayBeforeDay1Temperature < MILD_TEMPERATURE_LOWER_LIMIT &&
               day1Temperature < MILD_TEMPERATURE_LOWER_LIMIT &&
               day2Temperature >= MILD_TEMPERATURE_LOWER_LIMIT &&
               day2Temperature < MILD_TEMPERATURE_UPPER_LIMIT &&
               dayAfterDay2Temperature >= MILD_TEMPERATURE_LOWER_LIMIT &&
               dayAfterDay2Temperature < MILD_TEMPERATURE_UPPER_LIMIT &&
               !theParameters.theGrowingSeasonUnderway)
      {
        sentence << get_shortruntrend_sentence(MAANANTAINA_ALUEELLA_SAA_LAUHTUU_COMPOSITE_PHRASE,
                                               MAANANTAINA_SAA_LAUHTUU_COMPOSITE_PHRASE,
                                               ALUEELLA_SAA_LAUHTUU_COMPOSITE_PHRASE,
                                               SAA_LAUHTUU_PHRASE,
                                               theSpecifiedDay,
                                               theAreaPhrase);
        theParameters.theShortrunTrend = SAA_LAUHTUU;
      }
      else if (temperatureDifferenceDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDayBeforeDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDayBeforeDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               day1Temperature <= veryColdTemperature &&
               dayBeforeDay1Temperature <= veryColdTemperature &&
               day2Temperature <= MILD_TEMPERATURE_LOWER_LIMIT &&
               dayAfterDay2Temperature <= MILD_TEMPERATURE_LOWER_LIMIT)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_KIREA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE,
            MAANANTAINA_KIREA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE,
            ALUEELLA_KIREA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE,
            KIREA_PAKKANEN_HEIKKENEE_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = KIREA_PAKKANEN_HEIKKENEE;
      }
      else if (temperatureDifferenceDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDayBeforeDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDayBeforeDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               day1Temperature > veryColdTemperature &&
               dayBeforeDay1Temperature > veryColdTemperature &&
               day2Temperature < MILD_TEMPERATURE_LOWER_LIMIT &&
               dayAfterDay2Temperature < MILD_TEMPERATURE_LOWER_LIMIT)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE,
            MAANANTAINA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE,
            ALUEELLA_PAKKANEN_HEIKKENEE_COMPOSITE_PHRASE,
            PAKKANEN_HEIKKENEE_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = PAKKANEN_HEIKKENEE;
      }
      else if (temperatureDifferenceDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDayBeforeDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               temperatureDifferenceDayBeforeDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
               dayBeforeDay1Temperature > veryColdTemperature &&
               dayBeforeDay1Temperature < MILD_TEMPERATURE_LOWER_LIMIT &&
               day1Temperature > veryColdTemperature &&
               day1Temperature < MILD_TEMPERATURE_LOWER_LIMIT && day2Temperature < ZERO_DEGREES &&
               day2Temperature >= MILD_TEMPERATURE_LOWER_LIMIT && day2Temperature < ZERO_DEGREES &&
               day2Temperature >= MILD_TEMPERATURE_LOWER_LIMIT)
      {
        // redundant: this will never happen, because "saa lauhtuu" is tested before
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_PAKKANEN_HELLITTAA_COMPOSITE_PHRASE,
            MAANANTAINA_PAKKANEN_HELLITTAA_COMPOSITE_PHRASE,
            ALUEELLA_PAKKANEN_HELLITTAA_COMPOSITE_PHRASE,
            PAKKANEN_HELLITTAA_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = PAKKANEN_HELLITTAA;
      }
      else if (dayBeforeDay1Temperature < veryColdTemperature &&
               day1Temperature < veryColdTemperature && day2Temperature < veryColdTemperature &&
               dayAfterDay2Temperature < veryColdTemperature)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE,
            MAANANTAINA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE,
            ALUEELLA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE,
            KIREA_PAKKANEN_JATKUU_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = KIREA_PAKKANEN_JATKUU;
      }
    }
    else if (temperatureGettingLower == true)  // day2Temperature <= day1Temperature)
    {
      if (temperatureDifferenceDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
          temperatureDifferenceDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
          temperatureDifferenceDayBeforeDay1Day2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
          temperatureDifferenceDayBeforeDay1DayAfterDay2 >= SIGNIFIGANT_CHANGE_LOWER_LIMIT &&
          dayBeforeDay1Temperature < MILD_TEMPERATURE_LOWER_LIMIT &&
          day1Temperature < MILD_TEMPERATURE_LOWER_LIMIT &&
          day2Temperature <= veryColdTemperature && dayAfterDay2Temperature <= veryColdTemperature)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_PAKKANEN_KIRISTYY_COMPOSITE_PHRASE,
            MAANANTAINA_PAKKANEN_KIRISTYY_COMPOSITE_PHRASE,
            ALUEELLA_PAKKANEN_KIRISTYY_COMPOSITE_PHRASE,
            PAKKANEN_KIRISTYY_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = PAKKANEN_KIRISTYY;
      }
      else if (dayBeforeDay1Temperature < veryColdTemperature &&
               day1Temperature < veryColdTemperature && day2Temperature < veryColdTemperature &&
               dayAfterDay2Temperature < veryColdTemperature)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE,
            MAANANTAINA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE,
            ALUEELLA_KIREA_PAKKANEN_JATKUU_COMPOSITE_PHRASE,
            KIREA_PAKKANEN_JATKUU_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = KIREA_PAKKANEN_JATKUU;
      }
    }
  }
  else  // summer season
  {
    // helleraja
    float hot_weather_limit = Settings::optional_double(
        theParameters.theVariable + "::hot_weather_limit", HOT_WEATHER_LIMIT);

    // helteinen saa jatkuu
    // viilea saa jatkuu
    // kolea saa jatkuu
    // saa muuttuu helteiseksi
    // saa on helteista
    // saa lampenee vahan
    // saa lampenee
    // saa lampenee huomattavasti
    // saa viilenee vahan
    // saa viilenee
    // saa viilenee huomattavasti

    if (temperatureGettingLower == false)  // day2Temperature >= day1Temperature)
    {
      if (dayBeforeDay1Temperature >= hot_weather_limit && day1Temperature >= hot_weather_limit &&
          day2Temperature >= hot_weather_limit && dayAfterDay2Temperature >= hot_weather_limit)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE,
            MAANANTAINA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE,
            ALUEELLA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE,
            HELTEINEN_SAA_JATKUU_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = HELTEINEN_SAA_JATKUU;
      }
      else if (dayBeforeDay1Temperature < fractile12TemperatureDayBeforeDay1.value() &&
               day1Temperature < fractile12TemperatureDay1.value() &&
               day2Temperature < fractile12TemperatureDay2.value() &&
               dayAfterDay2Temperature < fractile12TemperatureDayAfterDay2.value())
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE,
            MAANANTAINA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE,
            ALUEELLA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE,
            KOLEA_SAA_JATKUU_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = KOLEA_SAA_JATKUU;
      }
      else if (dayBeforeDay1Temperature < fractile37TemperatureDayBeforeDay1.value() &&
               day1Temperature < fractile37TemperatureDay1.value() &&
               day2Temperature < fractile37TemperatureDay2.value() &&
               dayAfterDay2Temperature < fractile37TemperatureDayAfterDay2.value())
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE,
            MAANANTAINA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE,
            ALUEELLA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE,
            VIILEA_SAA_JATKUU_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = VIILEA_SAA_JATKUU;
      }
      else if (dayBeforeDay1Temperature < hot_weather_limit &&
               day1Temperature < hot_weather_limit && day2Temperature >= hot_weather_limit &&
               dayAfterDay2Temperature >= hot_weather_limit)
      {
        if (temperatureDifferenceDay1Day2 >= NOTABLE_TEMPERATURE_CHANGE_LIMIT &&
            temperatureDifferenceDay1DayAfterDay2 >= NOTABLE_TEMPERATURE_CHANGE_LIMIT &&
            temperatureDifferenceDayBeforeDay1Day2 >= NOTABLE_TEMPERATURE_CHANGE_LIMIT &&
            temperatureDifferenceDayBeforeDay1DayAfterDay2 >= NOTABLE_TEMPERATURE_CHANGE_LIMIT)
        {
          sentence << get_shortruntrend_sentence(
              MAANANTAINA_ALUEELLA_SAA_MUUTTUU_HELTEISEKSI_COMPOSITE_PHRASE,
              MAANANTAINA_SAA_MUUTTUU_HELTEISEKSI_COMPOSITE_PHRASE,
              ALUEELLA_SAA_MUUTTUU_HELTEISEKSI_COMPOSITE_PHRASE,
              SAA_MUUTTUU_HELTEISEKSI_PHRASE,
              theSpecifiedDay,
              theAreaPhrase);
          theParameters.theShortrunTrend = SAA_MUUTTUU_HELTEISEKSI;
        }
        else
        {
          sentence << get_shortruntrend_sentence(
              MAANANTAINA_ALUEELLA_SAA_ON_HELTEISTA_COMPOSITE_PHRASE,
              MAANANTAINA_SAA_ON_HELTEISTA_COMPOSITE_PHRASE,
              ALUEELLA_SAA_ON_HELTEISTA_COMPOSITE_PHRASE,
              SAA_ON_HELTEISTA_PHRASE,
              theSpecifiedDay,
              theAreaPhrase);
          theParameters.theShortrunTrend = SAA_ON_HELTEISTA;
        }
      }
      else if (signifigantChange && day2Temperature < 25.0)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_SAA_LAMPENEE_HUOMATTAVASTI_COMPOSITE_PHRASE,
            MAANANTAINA_SAA_LAMPENEE_HUOMATTAVASTI_COMPOSITE_PHRASE,
            ALUEELLA_SAA_LAMPENEE_HUOMATTAVASTI_COMPOSITE_PHRASE,
            SAA_LAMPENEE_HUOMATTAVASTI_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = SAA_LAMPENEE_HUOMATTAVASTI;
      }
      else if (moderateChange && day2Temperature < 25.0)
      {
        sentence << get_shortruntrend_sentence(MAANANTAINA_ALUEELLA_SAA_LAMPENEE_COMPOSITE_PHRASE,
                                               MAANANTAINA_SAA_LAMPENEE_COMPOSITE_PHRASE,
                                               ALUEELLA_SAA_LAMPENEE_COMPOSITE_PHRASE,
                                               SAA_LAMPENEE_PHRASE,
                                               theSpecifiedDay,
                                               theAreaPhrase);
        theParameters.theShortrunTrend = SAA_LAMPENEE;
      }
      else if (smallChange && day2Temperature < 20.0)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_SAA_LAMPENEE_VAHAN_COMPOSITE_PHRASE,
            MAANANTAINA_SAA_LAMPENEE_VAHAN_COMPOSITE_PHRASE,
            ALUEELLA_SAA_LAMPENEE_VAHAN_COMPOSITE_PHRASE,
            SAA_LAMPENEE_VAHAN_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = SAA_LAMPENEE_VAHAN;
      }
    }
    else if (temperatureGettingLower == true &&
             day2Temperature > ZERO_DEGREES)  // day2Temperature <= day1Temperature &&
                                              // day2Temperature > ZERO_DEGREES)
    {
      if (dayBeforeDay1Temperature >= hot_weather_limit && day1Temperature >= hot_weather_limit &&
          day2Temperature >= hot_weather_limit && dayAfterDay2Temperature >= hot_weather_limit)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE,
            MAANANTAINA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE,
            ALUEELLA_HELTEINEN_SAA_JATKUU_COMPOSITE_PHRASE,
            HELTEINEN_SAA_JATKUU_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = HELTEINEN_SAA_JATKUU;
      }
      else if (dayBeforeDay1Temperature < fractile12TemperatureDayBeforeDay1.value() &&
               day1Temperature < fractile12TemperatureDay1.value() &&
               day2Temperature < fractile12TemperatureDay2.value() &&
               dayAfterDay2Temperature < fractile12TemperatureDayAfterDay2.value())
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE,
            MAANANTAINA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE,
            ALUEELLA_KOLEA_SAA_JATKUU_COMPOSITE_PHRASE,
            KOLEA_SAA_JATKUU_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = KOLEA_SAA_JATKUU;
      }
      else if (day1Temperature < fractile37TemperatureDay1.value() &&
               dayBeforeDay1Temperature < fractile37TemperatureDayBeforeDay1.value() &&
               day2Temperature < fractile37TemperatureDay2.value() &&
               dayAfterDay2Temperature < fractile37TemperatureDayAfterDay2.value())
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE,
            MAANANTAINA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE,
            ALUEELLA_VIILEA_SAA_JATKUU_COMPOSITE_PHRASE,
            VIILEA_SAA_JATKUU_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = VIILEA_SAA_JATKUU;
      }
      else if (signifigantChange && day2Temperature < 25.0)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_SAA_VIILENEE_HUOMATTAVASTI_COMPOSITE_PHRASE,
            MAANANTAINA_SAA_VIILENEE_HUOMATTAVASTI_COMPOSITE_PHRASE,
            ALUEELLA_SAA_VIILENEE_HUOMATTAVASTI_COMPOSITE_PHRASE,
            SAA_VIILENEE_HUOMATTAVASTI_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = SAA_VIILENEE_HUOMATTAVASTI;
      }
      else if (moderateChange && day2Temperature < 25.0)
      {
        sentence << get_shortruntrend_sentence(MAANANTAINA_ALUEELLA_SAA_VIILENEE_COMPOSITE_PHRASE,
                                               MAANANTAINA_SAA_VIILENEE_COMPOSITE_PHRASE,
                                               ALUEELLA_SAA_VIILENEE_COMPOSITE_PHRASE,
                                               SAA_VIILENEE_PHRASE,
                                               theSpecifiedDay,
                                               theAreaPhrase);
        theParameters.theShortrunTrend = SAA_VIILENEE;
      }
      else if (smallChange && day2Temperature < 20.0)
      {
        sentence << get_shortruntrend_sentence(
            MAANANTAINA_ALUEELLA_SAA_VIILENEE_VAHAN_COMPOSITE_PHRASE,
            MAANANTAINA_SAA_VIILENEE_VAHAN_COMPOSITE_PHRASE,
            ALUEELLA_SAA_VIILENEE_VAHAN_COMPOSITE_PHRASE,
            SAA_VIILENEE_VAHAN_PHRASE,
            theSpecifiedDay,
            theAreaPhrase);
        theParameters.theShortrunTrend = SAA_VIILENEE_VAHAN;
      }
    }
  }

  return sentence;
}

Sentence handle_anomaly_and_shortrun_trend_sentences(
    const temperature_anomaly_params& theParameters,
    const Sentence& anomalySentence,
    const Sentence& shortrunTrendSentence)
{
  Sentence sentence;

  if (theParameters.theAnomalyPhrase == SAA_ON_POIKKEUKSELLISEN_KOLEAA ||
      theParameters.theAnomalyPhrase == SAA_ON_HARVINAISEN_LAMMINTA ||
      theParameters.theAnomalyPhrase == SAA_ON_POIKKEUKSLLISEN_LAMMINTA ||
      theParameters.theAnomalyPhrase == SAA_ON_HYVIN_LEUTOA ||
      theParameters.theAnomalyPhrase == SAA_ON_POIKKEUKSLLISEN_LAMMINTA)
  {
    sentence << anomalySentence;
  }
  else if (theParameters.theAnomalyPhrase == SAA_ON_KOLEAA)
  {
    switch (theParameters.theShortrunTrend)
    {
      case SAA_LAMPENEE_HUOMATTAVASTI:
      case SAA_VIILENEE_HUOMATTAVASTI:
        sentence << shortrunTrendSentence;
        break;
      default:
        sentence << anomalySentence;
    }
  }
  else if (theParameters.theAnomalyPhrase == SAA_ON_POIKKEUKSELLISEN_KYLMAA ||
           theParameters.theAnomalyPhrase == SAA_ON_HYVIN_KYLMAA)
  {
    switch (theParameters.theShortrunTrend)
    {
      case KIREA_PAKKANEN_HEIKKENEE:
      case PAKKANEN_HEIKKENEE:
      case SAA_LAUHTUU:
        sentence << shortrunTrendSentence;
        break;
      default:
        sentence << anomalySentence;
    }
  }
  else
  {
    sentence << shortrunTrendSentence;
  }

  return sentence;
}

const Paragraph anomaly(const TextGen::WeatherArea& itsArea,
                        const TextGen::WeatherPeriod& itsPeriod,
                        const TextGen::AnalysisSources& itsSources,
                        const TextGenPosixTime& itsForecastTime,
                        const std::string& itsVar,
                        const std::string& theAreaName,
                        MessageLogger& theLog)
{
  using namespace TemperatureAnomaly;

  Paragraph paragraph;

  log_start_time_and_end_time(theLog, "the original period: ", itsPeriod);

  forecast_season_id theSeasonId =
      isSummerHalf(itsPeriod.localStartTime(), itsVar) ? SUMMER_SEASON : WINTER_SEASON;

  // the anomaly sentence relates always to the upcoming day,
  // so the period is defined to start in the previous day

  TextGenPosixTime periodStartTime(itsPeriod.localStartTime());
  TextGenPosixTime periodEndTime(itsPeriod.localEndTime());
  int periodLength = periodEndTime.DifferenceInHours(periodStartTime);

  int ndays = HourPeriodGenerator(itsPeriod, itsVar + "::day").size();

  theLog << "Period " << itsPeriod.localStartTime() << "..." << itsPeriod.localEndTime()
         << " covers " << ndays << " days" << endl;

  if (ndays <= 0)
  {
    theLog << paragraph;
    return paragraph;
  }
  else if (ndays == 1)
  {
    periodStartTime.ChangeByDays(-1);
  }

  WeatherPeriod theExtendedPeriod(periodStartTime, periodEndTime);

  HourPeriodGenerator generator(theExtendedPeriod, itsVar + "::day");
  for (unsigned int i = 0; i < generator.size(); i++)
  {
    log_start_time_and_end_time(theLog, "The extended period: ", generator.period(i + 1));
  }

  WeatherPeriod day1Period(generator.period(1));
  WeatherPeriod day2Period(generator.period(2));

  TextGenPosixTime periodDayBeforeDay1StartTime;
  TextGenPosixTime periodDayBeforeDay1EndTime;
  TextGenPosixTime periodDayAfterDay2StartTime;
  TextGenPosixTime periodDayAfterDay2EndTime;

  periodDayBeforeDay1StartTime = day1Period.localStartTime();
  periodDayBeforeDay1StartTime.ChangeByHours(-24);
  periodDayBeforeDay1EndTime = day1Period.localEndTime();
  periodDayBeforeDay1EndTime.ChangeByHours(-24);

  periodDayAfterDay2StartTime = day2Period.localStartTime();
  periodDayAfterDay2StartTime.ChangeByHours(24);
  periodDayAfterDay2EndTime = day2Period.localEndTime();
  periodDayAfterDay2EndTime.ChangeByHours(24);

  WeatherPeriod dayBeforeDay1Period(periodDayBeforeDay1StartTime, periodDayBeforeDay1EndTime);
  WeatherPeriod dayAfterDay2Period(periodDayAfterDay2StartTime, periodDayAfterDay2EndTime);

  log_start_time_and_end_time(theLog, "day before day1: ", dayBeforeDay1Period);
  log_start_time_and_end_time(theLog, "day1: ", day1Period);
  log_start_time_and_end_time(theLog, "day2: ", day2Period);
  log_start_time_and_end_time(theLog, "day after day2: ", dayAfterDay2Period);

  bool report_day2_anomaly = true;

  temperature_anomaly_params parameters(itsVar,
                                        theLog,
                                        itsSources,
                                        itsArea,
                                        theExtendedPeriod,
                                        dayBeforeDay1Period,
                                        day1Period,
                                        day2Period,
                                        dayAfterDay2Period,
                                        theSeasonId,
                                        itsForecastTime,
                                        periodLength);

  afternoon_temperature(itsVar + "::fake::temperature::day_before_day1::afternoon::area",
                        itsSources,
                        itsArea,
                        dayBeforeDay1Period,
                        parameters.theDayBeforeDay1TemperatureAreaAfternoonMinimum,
                        parameters.theDayBeforeDay1TemperatureAreaAfternoonMaximum,
                        parameters.theDayBeforeDay1TemperatureAreaAfternoonMean);

  afternoon_temperature(itsVar + "::fake::temperature::day1::afternoon::area",
                        itsSources,
                        itsArea,
                        day1Period,
                        parameters.theDay1TemperatureAreaAfternoonMinimum,
                        parameters.theDay1TemperatureAreaAfternoonMaximum,
                        parameters.theDay1TemperatureAreaAfternoonMean);

  afternoon_temperature(itsVar + "::fake::temperature::day2::afternoon::area",
                        itsSources,
                        itsArea,
                        day2Period,
                        parameters.theDay2TemperatureAreaAfternoonMinimum,
                        parameters.theDay2TemperatureAreaAfternoonMaximum,
                        parameters.theDay2TemperatureAreaAfternoonMean);

  afternoon_temperature(itsVar + "::fake::temperature::day_after_day2::afternoon::area",
                        itsSources,
                        itsArea,
                        dayAfterDay2Period,
                        parameters.theDayAfterDay2TemperatureAreaAfternoonMinimum,
                        parameters.theDayAfterDay2TemperatureAreaAfternoonMaximum,
                        parameters.theDayAfterDay2TemperatureAreaAfternoonMean);

  fractile_type_id fractileType(MAX_FRACTILE);

  WeatherPeriod fractileTemperatureDay1Period(
      get_afternoon_period(parameters.theVariable, parameters.theDay1Period.localStartTime()));
  WeatherPeriod fractileTemperatureDay2Period(
      get_afternoon_period(parameters.theVariable, parameters.theDay2Period.localStartTime()));

  fractile_id theFractileDay1 =
      get_fractile(itsVar,
                   parameters.theDay1TemperatureAreaAfternoonMaximum.value(),
                   itsSources,
                   itsArea,
                   fractileTemperatureDay1Period,
                   fractileType);

  fractile_id theFractileDay2 =
      get_fractile(itsVar,
                   parameters.theDay2TemperatureAreaAfternoonMaximum.value(),
                   itsSources,
                   itsArea,
                   fractileTemperatureDay2Period,
                   fractileType);

  WeatherResult fractileTemperatureDay1 = get_fractile_temperature(
      itsVar, theFractileDay1, itsSources, itsArea, fractileTemperatureDay1Period, fractileType);

  WeatherResult fractileTemperatureDay2 = get_fractile_temperature(
      itsVar, theFractileDay2, itsSources, itsArea, fractileTemperatureDay2Period, fractileType);

  WeatherPeriod fractileTemperaturePeriod(report_day2_anomaly ? fractileTemperatureDay2Period
                                                              : fractileTemperatureDay1Period);

  theLog << "Day1 maximum temperature " << parameters.theDay1TemperatureAreaAfternoonMaximum;
  if (theFractileDay1 == FRACTILE_100)
  {
    WeatherResult f98Temperature = get_fractile_temperature(
        itsVar, FRACTILE_98, itsSources, itsArea, fractileTemperatureDay1Period, fractileType);

    theLog << "is higher than F98 temperature " << f98Temperature << endl;
  }
  else
  {
    theLog << " falls into fractile " << fractile_name(theFractileDay1) << fractileTemperatureDay1
           << endl;
  }

  theLog << "Day2 maximum temperature " << parameters.theDay2TemperatureAreaAfternoonMaximum;
  if (theFractileDay2 == FRACTILE_100)
  {
    WeatherResult f98Temperature = get_fractile_temperature(
        itsVar, FRACTILE_98, itsSources, itsArea, fractileTemperatureDay2Period, fractileType);
    theLog << "is higher than F98 temperature " << f98Temperature << endl;
  }
  else
  {
    theLog << " falls into fractile " << fractile_name(theFractileDay2) << fractileTemperatureDay2
           << endl;
  }

  if (parameters.theDay1TemperatureAreaAfternoonMaximum.value() == kFloatMissing ||
      parameters.theDay2TemperatureAreaAfternoonMaximum.value() == kFloatMissing)
  {
    theLog << "Maximum temperature value(s) missing, no story generated!" << endl;
    theLog << paragraph;
    return paragraph;
  }

  WeatherResult fractile02Temperature = get_fractile_temperature(
      itsVar, FRACTILE_02, itsSources, itsArea, fractileTemperaturePeriod, fractileType);

  WeatherResult fractile12Temperature = get_fractile_temperature(
      itsVar, FRACTILE_12, itsSources, itsArea, fractileTemperaturePeriod, fractileType);

  WeatherResult fractile88Temperature = get_fractile_temperature(
      itsVar, FRACTILE_88, itsSources, itsArea, fractileTemperaturePeriod, fractileType);

  WeatherResult fractile98Temperature = get_fractile_temperature(
      itsVar, FRACTILE_98, itsSources, itsArea, fractileTemperaturePeriod, fractileType);

  GridForecaster theForecaster;
  RangeAcceptor upperLimitF02Acceptor;
  upperLimitF02Acceptor.upperLimit(fractile02Temperature.value());
  RangeAcceptor upperLimitF12Acceptor;
  upperLimitF12Acceptor.upperLimit(fractile12Temperature.value());
  RangeAcceptor lowerLimitF88Acceptor;
  lowerLimitF88Acceptor.lowerLimit(fractile88Temperature.value());
  RangeAcceptor lowerLimitF98Acceptor;
  lowerLimitF98Acceptor.lowerLimit(fractile98Temperature.value());

  WeatherResult fractile02Share = theForecaster.analyze(itsVar + "::fake::fractile::share::F02",
                                                        itsSources,
                                                        Temperature,
                                                        Percentage,
                                                        Maximum,
                                                        itsArea,
                                                        fractileTemperaturePeriod,
                                                        DefaultAcceptor(),
                                                        DefaultAcceptor(),
                                                        upperLimitF02Acceptor);

  WeatherResult fractile12Share = theForecaster.analyze(itsVar + "::fake::fractile::share::F12",
                                                        itsSources,
                                                        Temperature,
                                                        Percentage,
                                                        Maximum,
                                                        itsArea,
                                                        fractileTemperaturePeriod,
                                                        DefaultAcceptor(),
                                                        DefaultAcceptor(),
                                                        upperLimitF12Acceptor);

  WeatherResult fractile88Share = theForecaster.analyze(itsVar + "::fake::fractile::share::F88",
                                                        itsSources,
                                                        Temperature,
                                                        Percentage,
                                                        Maximum,
                                                        itsArea,
                                                        fractileTemperaturePeriod,
                                                        DefaultAcceptor(),
                                                        DefaultAcceptor(),
                                                        lowerLimitF88Acceptor);

  WeatherResult fractile98Share = theForecaster.analyze(itsVar + "::fake::fractile::share::F98",
                                                        itsSources,
                                                        Temperature,
                                                        Percentage,
                                                        Maximum,
                                                        itsArea,
                                                        fractileTemperaturePeriod,
                                                        DefaultAcceptor(),
                                                        DefaultAcceptor(),
                                                        lowerLimitF98Acceptor);

  theLog << "Fractiles for period: " << period2string(fractileTemperaturePeriod) << std::endl;
  theLog << "Fractile 02 temperature and share (" << theAreaName << "): " << fractile02Temperature
         << "; " << fractile02Share << endl;
  theLog << "Fractile 12 temperature and share (" << theAreaName << "): " << fractile12Temperature
         << "; " << fractile12Share << endl;
  theLog << "Fractile 88 temperature and share (" << theAreaName << "): " << fractile88Temperature
         << "; " << fractile88Share << endl;
  theLog << "Fractile 98 temperature and share (" << theAreaName << "): " << fractile98Temperature
         << "; " << fractile98Share << endl;

  if (fractile02Temperature.value() == kFloatMissing ||
      fractile12Temperature.value() == kFloatMissing ||
      fractile88Temperature.value() == kFloatMissing ||
      fractile98Temperature.value() == kFloatMissing)
  {
    theLog << "Fractile temperature value(s) missing, no story generated!" << endl;
    theLog << paragraph;
    return paragraph;
  }

  if (fractile02Share.value() == kFloatMissing || fractile12Share.value() == kFloatMissing ||
      fractile88Share.value() == kFloatMissing || fractile98Share.value() == kFloatMissing)
  {
    theLog << "Fractile share value(s) missing, no story generated!" << endl;
    theLog << paragraph;
    return paragraph;
  }

  const WeatherPeriod& thePeriod =
      (report_day2_anomaly ? parameters.theDay2Period : parameters.theDay1Period);

  parameters.theGrowingSeasonUnderway =
      growing_season_going_on(itsArea, itsSources, thePeriod, itsVar);
  Sentence temperatureAnomalySentence;
  Sentence shortrunTrendSentence;

  temperatureAnomalySentence << temperature_anomaly_sentence(
      parameters,
      fractile02Share.value(),
      fractile12Share.value(),
      fractile88Share.value(),
      fractile98Share.value(),
      report_day2_anomaly ? parameters.theDay2Period : parameters.theDay1Period);

  shortrunTrendSentence << temperature_shortruntrend_sentence(parameters, fractileType);

  log_data(parameters);

  paragraph << handle_anomaly_and_shortrun_trend_sentences(
      parameters, temperatureAnomalySentence, shortrunTrendSentence);

  theLog << paragraph;

  return paragraph;
}

}  // namespace TemperatureAnomaly

// ----------------------------------------------------------------------
/*!
 * \brief Generate temperature anomaly story
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

const Paragraph TemperatureStory::anomaly() const
{
  using namespace TemperatureAnomaly;

  Paragraph paragraph;
  MessageLogger log("TemperatureStory::anomaly");

  std::string areaName("");

  if (itsArea.isNamed())
  {
    areaName = itsArea.name();
    log << areaName << endl;
  }

  boostfs::path climatologyFile(Settings::optional_string("textgen::tmax_climatology", ""));

  if (!boostfs::exists(climatologyFile))
  {
    log << "The climatology file " << climatologyFile << " does not exist!" << endl;
    return paragraph;
  }

  WeatherArea areaOne(itsArea);
  WeatherArea areaTwo(itsArea);
  split_method splitMethod =
      check_area_splitting(itsVar, itsArea, itsPeriod, itsSources, areaOne, areaTwo, log);

  if (NO_SPLITTING != splitMethod)
  {
    Paragraph paragraphAreaOne;
    Paragraph paragraphAreaTwo;

    std::string areaId =
        (areaName + (splitMethod == HORIZONTAL ? " - southern part" : " - western part"));
    log << areaId << endl;

    paragraphAreaOne << TemperatureAnomaly::anomaly(
        areaOne, itsPeriod, itsSources, itsForecastTime, itsVar, areaId, log);

    areaId = (areaName + (splitMethod == HORIZONTAL ? " - northern part" : " - eastern part"));
    log << areaId << endl;

    paragraphAreaTwo << TemperatureAnomaly::anomaly(
        areaTwo, itsPeriod, itsSources, itsForecastTime, itsVar, areaId, log);
    paragraph << paragraphAreaOne << paragraphAreaTwo;
  }
  else
  {
    paragraph << TemperatureAnomaly::anomaly(
        itsArea, itsPeriod, itsSources, itsForecastTime, itsVar, areaName, log);
  }

  return paragraph;
}

}  // namespace TextGen

// ======================================================================
