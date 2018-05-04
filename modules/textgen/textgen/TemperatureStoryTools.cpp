// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::TemperatureStoryTools
 */
// ======================================================================
/*!
 * \namespace TextGen::TemperatureStoryTools
 *
 * \brief Common utilities for TemperatureStory methods
 *
 */
// ======================================================================

#include "TemperatureStoryTools.h"
#include "Integer.h"
#include "TemperatureRange.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "UnitFactory.h"
#include "ClimatologyTools.h"
#include "GridClimatology.h"
#include <calculator/WeatherPeriodGenerator.h>
#include <calculator/HourPeriodGenerator.h>
#include "PositiveValueAcceptor.h"

#include <calculator/GridForecaster.h>
#include <calculator/WeatherResult.h>
#include <calculator/WeatherPeriod.h>
#include "SeasonTools.h"
#include <calculator/WeatherArea.h>
#include "TextFormatter.h"
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <string>

using namespace Settings;
using namespace TextGen;
using namespace std;

namespace TextGen
{
namespace TemperatureStoryTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Return temperature comparison phrase
 *
 * \param theMean1 The first mean
 * \param theMean2 The second mean
 * \param theVariable The variable containing the limits
 */
// ----------------------------------------------------------------------

const char* temperature_comparison_phrase(int theMean1, int theMean2, const string& theVariable)
{
  using namespace Settings;

  const int significantly_higher =
      require_percentage(theVariable + "::comparison::significantly_higher");
  const int significantly_lower =
      require_percentage(theVariable + "::comparison::significantly_lower");
  const int higher = require_percentage(theVariable + "::comparison::higher");
  const int lower = require_percentage(theVariable + "::comparison::lower");
  const int somewhat_higher = require_percentage(theVariable + "::comparison::somewhat_higher");
  const int somewhat_lower = require_percentage(theVariable + "::comparison::somewhat_lower");

  if (theMean2 - theMean1 >= significantly_higher) return "huomattavasti korkeampi";
  if (theMean2 - theMean1 >= higher) return "korkeampi";
  if (theMean2 - theMean1 >= somewhat_higher) return "hieman korkeampi";
  if (theMean1 - theMean2 >= significantly_lower) return "huomattavasti alempi";
  if (theMean1 - theMean2 >= lower) return "alempi";
  if (theMean1 - theMean2 >= somewhat_lower) return "hieman alempi";
  return "suunnilleen sama";
}

// ----------------------------------------------------------------------
/*!
 * \brief Return temperature sentence
 *
 * Possible sentences are
 *
 *  - "noin x astetta"
 *  - "x...y astetta";
 *
 * \param theMinimum The minimum temperature
 * \param theMean The mean temperature
 * \param theMaximum The maximum temperature
 * \param theMinInterval The minimum interval limit
 * \param theZeroFlag True if zero is always intervalled
 * \param theRangeSeparator String separating the numbers
 * \return The sentence
 */
// ----------------------------------------------------------------------

TextGen::Sentence temperature_sentence(int theMinimum,
                                       int theMean,
                                       int theMaximum,
                                       int theMinInterval,
                                       bool theZeroFlag,
                                       const std::string& theRangeSeparator)
{
  Sentence sentence;

  bool range(false);

  // theMaximum can contain lower value than the theMinimum
  int diff = abs(theMaximum - theMinimum);

  if (theMinimum == theMaximum)
    range = false;
  else if (diff >= theMinInterval)
    range = true;
  else if (theMinimum <= 0 && theMaximum >= 0)
    range = true;
  else
    range = false;

  if (range)
  {
    int intervalStartValue(theMinimum);
    int intervalEndValue(theMaximum);
    temperature_range(theMinimum, theMaximum, intervalStartValue, intervalEndValue);

    sentence << TemperatureRange(intervalStartValue, intervalEndValue, theRangeSeparator)
             << *UnitFactory::create_unit(DegreesCelsius, intervalEndValue, true);
  }
  else
  {
    sentence << "noin" << Integer(theMean) << *UnitFactory::create_unit(DegreesCelsius, theMean);
  }

  return sentence;
}

// if temperature is lower than -15 degrees, we can round 2 degrees otherwise 1 degrees
// to the nearest number that is divisible by five
int round_temperature(const int& theTemperatureToRound)
{
  int theRoundingLimit = theTemperatureToRound < -15 ? 2 : 1;
  int theRoundedValue = theTemperatureToRound;
  int theModuloOfValue = theTemperatureToRound % 5;

  if (theModuloOfValue != 0)
  {
    if (theModuloOfValue < 0) theModuloOfValue += 5;

    if (theModuloOfValue <= theRoundingLimit)
      theRoundedValue -= theModuloOfValue;
    else if (theModuloOfValue >= (5 - theRoundingLimit))
      theRoundedValue += (5 - theModuloOfValue);
  }

  return theRoundedValue;
}

// changed 6.10.2010
// Lea Saukkonen:
// 1. Lukemat positiivisi: pienempi lukema ensin esim L-Admpvtila on viidestd kymmeneen
// (5…10) astetta
// 2. Lukemat nollan molemmin puolin: kylmempi ensin esim Ldmpvtila on miinus kolmen ja plus
// kahden (-3…+2)asteen vdlilld
// 3. Lukemista toinen on nolla: nolla ensin esim Ldmpvtila on nollan ja miinus viiden
// 0…-5) asteen vdlilld toinen esimerkki Ldmpvtila on nollan ja plus viiden
// (0…+5) asteen vdlilld
// 4. Lukemat negatiivisia: ldmpimdmpi emsin esim Ldmpvtila on miinus viidestd miinus
// kymmeneen (-5…-10) asteeseen tai Pakkasta on viidestd kymmeneen asteeseen.
void temperature_range(const int& theTemperature1,
                       const int& theTemperature2,
                       int& intervalStart,
                       int& intervalEnd)
{
  int theMinimum = theTemperature1 < theTemperature2 ? theTemperature1 : theTemperature2;
  int theMaximum = theTemperature1 > theTemperature2 ? theTemperature1 : theTemperature2;
  intervalStart = theMinimum;
  intervalEnd = theMaximum;

  if (theMinimum < 0 && theMaximum <= 0)
  {
    intervalStart = theMaximum;
    intervalEnd = theMinimum;
  }
}

TextGen::Sentence temperature_range(const int& theTemperature1,
                                    const int& theTemperature2,
                                    const std::string& theRangeSeparator,
                                    int& intervalStart,
                                    int& intervalEnd)
{
  Sentence sentence;

  temperature_range(theTemperature1, theTemperature2, intervalStart, intervalEnd);

  sentence << TemperatureRange(intervalStart, intervalEnd, theRangeSeparator);

  return sentence;
}

bool sort_out_temperature_interval(int theMinimum,
                                   int theMean,
                                   int theMaximum,
                                   int theMinInterval,
                                   bool theZeroFlag,
                                   int& intervalStart,
                                   int& intervalEnd,
                                   const bool& theRoundTheNumber)
{
  bool interval_used(true);

  bool range = false;

  // in winter theMaximum contains the lower value than the theMinimum
  int diff = abs(theMaximum - theMinimum);

  if (theMinimum == theMaximum)
    range = false;
  else if (diff >= theMinInterval)
    range = true;
  else if (theMinimum <= 0 && theMaximum >= 0)
    range = true;

  if (range)
  {
    int theRoundedMinimum = theMinimum;
    int theRoundedMaximum = theMaximum;

    if (theMinimum <= -15 && theMaximum <= -15 && theRoundTheNumber)
    {
      theRoundedMinimum = round_temperature(theMinimum);
      theRoundedMaximum = round_temperature(theMaximum);
    }

    if (theRoundedMinimum == theRoundedMaximum)
    {
      interval_used = false;
      intervalStart = theRoundedMinimum;
      intervalEnd = intervalStart;
    }
    else
    {
      interval_used = true;
      temperature_range(theRoundedMinimum, theRoundedMaximum, intervalStart, intervalEnd);
    }
  }
  else
  {
    interval_used = false;
    intervalStart = (theRoundTheNumber ? round_temperature(theMean) : theMean);
    intervalEnd = intervalStart;
  }

  return interval_used;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return temperature sentence
 *
 * Possible sentences are
 *
 *  - "noin x astetta"
 *  - "x...y astetta";
 *
 * \param theMinimum The minimum temperature
 * \param theMean The mean temperature
 * \param theMaximum The maximum temperature
 * \param theMinInterval The minimum interval limit
 * \param theZeroFlag True if zero is always intervalled
 * \param theRangeSeparator String separating the numbers
 * \return The sentence
 */
// ----------------------------------------------------------------------

TextGen::Sentence temperature_sentence2(int theMinimum,
                                        int theMean,
                                        int theMaximum,
                                        int theMinInterval,
                                        bool theZeroFlag,
                                        bool& interval,
                                        int& intervalStart,
                                        int& intervalEnd,
                                        const std::string& theRangeSeparator,
                                        const bool& theRoundTheNumber)
{
  Sentence sentence;

  interval = sort_out_temperature_interval(theMinimum,
                                           theMean,
                                           theMaximum,
                                           theMinInterval,
                                           theZeroFlag,
                                           intervalStart,
                                           intervalEnd,
                                           theRoundTheNumber);

  if (interval)
  {
    sentence << TemperatureRange(intervalStart, intervalEnd, theRangeSeparator);
    sentence << *UnitFactory::create_unit(DegreesCelsius, intervalEnd, true);
    interval = true;
  }
  else
  {
    sentence << "noin" << Integer(intervalStart)
             << *UnitFactory::create_unit(DegreesCelsius, intervalStart);
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate Minimum, Mean and Maximum temperatures of the area's Maximum temperatures
 *
 * \param theVar The control variable prefix
 * \param theSources The analysis sources
 * \param theArea The waether area
 * \param theMinimum The varible where the  minimum temperature is stored
 * \param theMean The varible where the  mean temperature is stored
 * \param theMaximum The varible where the  maximum temperature is stored
 */
// ----------------------------------------------------------------------

void min_max_mean_temperature(const string& theVar,
                              const AnalysisSources& theSources,
                              const WeatherArea& theArea,
                              const WeatherPeriod& thePeriod,
                              const bool& theIsWinterHalf,
                              WeatherResult& theMin,
                              WeatherResult& theMax,
                              WeatherResult& theMean)
{
  GridForecaster theForecaster;

  theMin = theForecaster.analyze(theVar + "::min",
                                 theSources,
                                 Temperature,
                                 Minimum,
                                 theIsWinterHalf ? Mean : Maximum,
                                 theArea,
                                 thePeriod);

  theMax = theForecaster.analyze(theVar + "::max",
                                 theSources,
                                 Temperature,
                                 Maximum,
                                 theIsWinterHalf ? Mean : Maximum,
                                 theArea,
                                 thePeriod);

  theMean = theForecaster.analyze(theVar + "::mean",
                                  theSources,
                                  Temperature,
                                  Mean,
                                  theIsWinterHalf ? Mean : Maximum,
                                  theArea,
                                  thePeriod);
}

void min_max_mean_temperature(const string& theVar,
                              const AnalysisSources& theSources,
                              const WeatherArea& theArea,
                              const WeatherPeriodGenerator& thePeriods,
                              const bool& theIsWinterHalf,
                              WeatherResult& theMin,
                              WeatherResult& theMax,
                              WeatherResult& theMean)
{
  GridForecaster theForecaster;

  theMin = theForecaster.analyze(theVar + "::min",
                                 theSources,
                                 Temperature,
                                 Minimum,
                                 theIsWinterHalf ? Mean : Maximum,
                                 Maximum,
                                 theArea,
                                 thePeriods);

  theMax = theForecaster.analyze(theVar + "::max",
                                 theSources,
                                 Temperature,
                                 Maximum,
                                 theIsWinterHalf ? Mean : Maximum,
                                 Maximum,
                                 theArea,
                                 thePeriods);

  theMean = theForecaster.analyze(theVar + "::mean",
                                  theSources,
                                  Temperature,
                                  Mean,
                                  theIsWinterHalf ? Mean : Maximum,
                                  Maximum,
                                  theArea,
                                  thePeriods);
}

// ----------------------------------------------------------------------
/*!
 * \brief calculate morning temperature
 *
 * \param theVar The control variable prefix
 * \param theSources The analysis sources
 * \param theArea The waether area
 * \param thePeriod The main period
 * \param theMinimum The varible where the  minimum temperature is stored
 * \param theMean The varible where the  mean temperature is stored
 * \param theMaximum The varible where the  maximum temperature is stored
 */
// ----------------------------------------------------------------------

void morning_temperature(const string& theVar,
                         const AnalysisSources& theSources,
                         const WeatherArea& theArea,
                         const WeatherPeriod& thePeriod,
                         WeatherResult& theMin,
                         WeatherResult& theMax,
                         WeatherResult& theMean)

{
  int year = thePeriod.localStartTime().GetYear();
  int month = thePeriod.localStartTime().GetMonth();
  int day = thePeriod.localStartTime().GetDay();

  int default_starthour = 8;
  int default_endhour = 8;

  int fakeStrPos = theVar.find("::fake");
  std::string thePlainVar(fakeStrPos == -1 ? theVar : theVar.substr(0, fakeStrPos));
  bool is_winter = (SeasonTools::isWinterHalf(thePeriod.localStartTime(), thePlainVar));

  std::string season(is_winter ? "::wintertime" : "::summertime");

  int morning_starthour =
      optional_hour(thePlainVar + season + "::morning_temperature::starthour", default_starthour);
  int morning_endhour =
      optional_hour(thePlainVar + season + "::morning_temperature::endhour", default_endhour);

  TextGenPosixTime time1(year, month, day, morning_starthour, 0, 0);
  TextGenPosixTime time2(year, month, day, morning_endhour, 0, 0);

  WeatherPeriod morningPeriod(time1, time2);

  min_max_mean_temperature(
      theVar, theSources, theArea, morningPeriod, is_winter, theMin, theMax, theMean);
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate afternoon period.
 *
 * \param theVar The control variable prefix
 * \param theTime The timestamp
 */
// ----------------------------------------------------------------------
WeatherPeriod get_afternoon_period(const string& theVar, const TextGenPosixTime& theTime)
{
  int fakeStrPos = theVar.find("::fake");
  std::string thePlainVar(fakeStrPos == -1 ? theVar : theVar.substr(0, fakeStrPos));

  bool is_winter = SeasonTools::isWinterHalf(theTime, thePlainVar);
  int timezone = TextGenPosixTime::GetZoneDifferenceHour(theTime, false);
  std::string season(is_winter ? "::wintertime" : "::summertime");

  // in wintertime afternoon temperature is 12:00 UTC
  int default_starthour = (is_winter ? 12 + timezone : 13);
  int default_endhour = (is_winter ? 12 + timezone : 17);

  int afternoon_starthour =
      optional_hour(thePlainVar + season + "::day_temperature::starthour", default_starthour);
  int afternoon_endhour =
      optional_hour(thePlainVar + season + "::day_temperature::endhour", default_endhour);

  int year = theTime.GetYear();
  int month = theTime.GetMonth();
  int day = theTime.GetDay();

  TextGenPosixTime time1(year, month, day, afternoon_starthour, 0, 0);
  TextGenPosixTime time2(year, month, day, afternoon_endhour, 0, 0);

  return WeatherPeriod(time1, time2);
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate afternoon temperature.
 *
 * \param theVar The control variable prefix
 * \param theSources The analysis sources
 * \param theArea The waether area
 * \param thePeriod The main period
 * \param theMinimum The varible where the minimum temperature is stored
 * \param theMean The varible where the mean temperature is stored
 * \param theMaximum The varible where the maximum temperature is stored
 */
// ----------------------------------------------------------------------

void afternoon_temperature(const string& theVar,
                           const AnalysisSources& theSources,
                           const WeatherArea& theArea,
                           const WeatherPeriod& thePeriod,
                           WeatherResult& theMin,
                           WeatherResult& theMax,
                           WeatherResult& theMean)
{
  int fakeStrPos = theVar.find("::fake");
  std::string thePlainVar(fakeStrPos == -1 ? theVar : theVar.substr(0, fakeStrPos));
  bool is_winter = SeasonTools::isWinterHalf(thePeriod.localStartTime(), thePlainVar);

  min_max_mean_temperature(theVar,
                           theSources,
                           theArea,
                           get_afternoon_period(theVar, thePeriod.localStartTime()),
                           is_winter,
                           theMin,
                           theMax,
                           theMean);
}

void afternoon_temperature(const string& theVar,
                           const AnalysisSources& theSources,
                           const WeatherArea& theArea,
                           const WeatherPeriodGenerator& thePeriods,
                           WeatherResult& theMin,
                           WeatherResult& theMax,
                           WeatherResult& theMean)
{
  int fakeStrPos = theVar.find("::fake");
  std::string thePlainVar(fakeStrPos == -1 ? theVar : theVar.substr(0, fakeStrPos));

  bool is_winter = SeasonTools::isWinterHalf(thePeriods.period(1).localStartTime(), thePlainVar);

  min_max_mean_temperature(
      theVar, theSources, theArea, thePeriods, is_winter, theMin, theMax, theMean);
}
// ----------------------------------------------------------------------
/*!
 * \brief Clamp the temperature range if it is bigger than the defined maximum interval
 *
 * \param theVar The control variable prefix
 * \param isWinter This parameter tells whether it is wintertime (true) or summertime (false)
 * \param isDay This parameter tells whether it is daytime (true) or nighttime (false)
 * \param theMinimum The varible that contains the original minimum temperature and where the new
 * minimum temperature is stored
 * \param theMaximum The varible that contains the original maximum temperature  and where the new
 * maximum temperature is stored
 */
// ----------------------------------------------------------------------

void clamp_temperature(
    const string& theVar, const bool& isWinter, const bool& isDay, int& theMinimum, int& theMaximum)
{
  int fakeStrPos = theVar.find("::fake");
  std::string season(isWinter ? "::wintertime" : "::summertime");
  std::string period(isDay ? "::day" : "::night");
  std::string thePlainVar(fakeStrPos == -1 ? theVar : theVar.substr(0, fakeStrPos));
  int temperature_max_interval =
      optional_int(thePlainVar + season + period + "::temperature_max_interval", 5);

  // if minimum is below zero and maximum above, we allow bigger ranges and do no clamping
  if ((theMinimum <= 0 && theMaximum > 0) || (theMinimum < 0 && theMaximum >= 0)) return;

  // if both both minimum and maximum are below -15 degrees, dont' clamp
  if ((theMinimum <= -15 && theMaximum <= -15) && temperature_max_interval <= 10)
    temperature_max_interval = 10;
  // SOL-4856 Annakaisa (20170208): Jos minimi on kylmenpi kuin -20 ja
  // minimin ja maksimin ero on yli 10 astetta ndytetddn koko haarukka
  else if (theMinimum <= -20 && abs(theMaximum - theMinimum) > 10)
    return;

  bool clamp_down = optional_bool(thePlainVar + season + period + "::temperature_clamp_down",
                                  isWinter ? false : true);

  if (theMaximum - theMinimum > temperature_max_interval)
  {
    if (clamp_down)
      theMinimum = theMaximum - temperature_max_interval;
    else
      theMaximum = theMinimum + temperature_max_interval;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief determines the fractile of the given temperature
 *
 * \param theVar The control variable prefix
 * \param theSources The analysis sources
 * \param theArea The waether area
 * \param thePeriod The main period
 * \return The fractile id for the given temperature
 *
 */
// ----------------------------------------------------------------------
fractile_id get_fractile(const std::string& theVar,
                         const float& theTemperature,
                         const AnalysisSources& theSources,
                         const WeatherArea& theArea,
                         const WeatherPeriod& thePeriod,
                         const fractile_type_id& theFractileType)
{
  if (theTemperature == kFloatMissing) return FRACTILE_UNDEFINED;

  string seasonStr =
      SeasonTools::isSummerHalf(thePeriod.localStartTime(), theVar) ? "summer" : "winter";

  // fake variables are just for rough testing purposes
  if (Settings::isset(theVar + "::fake::fractile::" + seasonStr + "::F02"))
  {
    float f02 = require_double(theVar + "::fake::fractile::" + seasonStr + "::F02");
    float f12 = require_double(theVar + "::fake::fractile::" + seasonStr + "::F12");
    float f37 = require_double(theVar + "::fake::fractile::" + seasonStr + "::F37");
    float f50 = require_double(theVar + "::fake::fractile::" + seasonStr + "::F50");
    float f63 = require_double(theVar + "::fake::fractile::" + seasonStr + "::F63");
    float f88 = require_double(theVar + "::fake::fractile::" + seasonStr + "::F88");
    float f98 = require_double(theVar + "::fake::fractile::" + seasonStr + "::F98");
    if (theTemperature <= f02)
      return FRACTILE_02;
    else if (theTemperature <= f12)
      return FRACTILE_12;
    else if (theTemperature <= f37)
      return FRACTILE_37;
    else if (theTemperature <= f50)
      return FRACTILE_50;
    else if (theTemperature <= f63)
      return FRACTILE_63;
    else if (theTemperature <= f88)
      return FRACTILE_88;
    else if (theTemperature <= f98)
      return FRACTILE_98;
    else
      return FRACTILE_UNDEFINED;
  }

  std::string dataName("textgen::fractiles");

  WeatherPeriod climatePeriod =
      ClimatologyTools::getClimatologyPeriod(thePeriod, dataName, theSources);

  GridClimatology gc;

  WeatherResult result(kFloatMissing, 0.0);

  result = gc.analyze(
      theVar,
      theSources,
      (theFractileType == MIN_FRACTILE ? NormalMinTemperatureF02 : (theFractileType == MEAN_FRACTILE
                                                                        ? NormalMeanTemperatureF02
                                                                        : NormalMaxTemperatureF02)),
      Mean,
      Mean,
      theArea,
      climatePeriod);

  if (result.value() != kFloatMissing && theTemperature <= result.value()) return FRACTILE_02;

  result = gc.analyze(
      theVar,
      theSources,
      (theFractileType == MIN_FRACTILE ? NormalMinTemperatureF12 : (theFractileType == MEAN_FRACTILE
                                                                        ? NormalMeanTemperatureF12
                                                                        : NormalMaxTemperatureF12)),
      Mean,
      Mean,
      theArea,
      climatePeriod);

  if (result.value() != kFloatMissing && theTemperature <= result.value()) return FRACTILE_12;

  result = gc.analyze(
      theVar,
      theSources,
      (theFractileType == MIN_FRACTILE ? NormalMinTemperatureF37 : (theFractileType == MEAN_FRACTILE
                                                                        ? NormalMeanTemperatureF37
                                                                        : NormalMaxTemperatureF37)),
      Mean,
      Mean,
      theArea,
      climatePeriod);

  if (result.value() != kFloatMissing && theTemperature <= result.value()) return FRACTILE_37;

  result = gc.analyze(
      theVar,
      theSources,
      (theFractileType == MIN_FRACTILE ? NormalMinTemperatureF50 : (theFractileType == MEAN_FRACTILE
                                                                        ? NormalMeanTemperatureF50
                                                                        : NormalMaxTemperatureF50)),
      Mean,
      Mean,
      theArea,
      climatePeriod);

  if (result.value() != kFloatMissing && theTemperature <= result.value()) return FRACTILE_50;

  result = gc.analyze(
      theVar,
      theSources,
      (theFractileType == MIN_FRACTILE ? NormalMinTemperatureF63 : (theFractileType == MEAN_FRACTILE
                                                                        ? NormalMeanTemperatureF63
                                                                        : NormalMaxTemperatureF63)),
      Mean,
      Mean,
      theArea,
      climatePeriod);

  if (result.value() != kFloatMissing && theTemperature <= result.value()) return FRACTILE_63;

  result = gc.analyze(
      theVar,
      theSources,
      (theFractileType == MIN_FRACTILE ? NormalMinTemperatureF88 : (theFractileType == MEAN_FRACTILE
                                                                        ? NormalMeanTemperatureF88
                                                                        : NormalMaxTemperatureF88)),
      Mean,
      Mean,
      theArea,
      climatePeriod);

  if (result.value() != kFloatMissing && theTemperature <= result.value()) return FRACTILE_88;

  result = gc.analyze(
      theVar,
      theSources,
      (theFractileType == MIN_FRACTILE ? NormalMinTemperatureF98 : (theFractileType == MEAN_FRACTILE
                                                                        ? NormalMeanTemperatureF98
                                                                        : NormalMaxTemperatureF98)),
      Mean,
      Mean,
      theArea,
      climatePeriod);

  if (result.value() != kFloatMissing && theTemperature <= result.value())
    return FRACTILE_98;
  else if (result.value() != kFloatMissing)
    return FRACTILE_100;
  else
    return FRACTILE_UNDEFINED;
}

// ----------------------------------------------------------------------
/*!
 * \brief determines the temperature for the given fractile
 *
 * \param theVar The control variable prefix
 * \param theFractileId The fractile id
 * \param theSources The analysis sources
 * \param theArea The waether area
 * \param thePeriod The main period
 * \return The temperature for the given fractile id
 *
 */
// ----------------------------------------------------------------------
WeatherResult get_fractile_temperature(const std::string& theVar,
                                       const fractile_id& theFractileId,
                                       const AnalysisSources& theSources,
                                       const WeatherArea& theArea,
                                       const WeatherPeriod& thePeriod,
                                       const fractile_type_id& theFractileType)
{
  string seasonStr =
      SeasonTools::isSummerHalf(thePeriod.localStartTime(), theVar) ? "summer" : "winter";

  // fake variables are just for rough testing purposes
  if (Settings::isset(theVar + "::fake::fractile::" + seasonStr + "::F02"))
  {
    if (theFractileId == FRACTILE_02)
      return WeatherResult(require_double(theVar + "::fake::fractile::" + seasonStr + "::F02"), 0);
    else if (theFractileId == FRACTILE_12)
      return WeatherResult(require_double(theVar + "::fake::fractile::" + seasonStr + "::F12"), 0);
    else if (theFractileId == FRACTILE_37)
      return WeatherResult(require_double(theVar + "::fake::fractile::" + seasonStr + "::F37"), 0);
    else if (theFractileId == FRACTILE_50)
      return WeatherResult(require_double(theVar + "::fake::fractile::" + seasonStr + "::F50"), 0);
    else if (theFractileId == FRACTILE_63)
      return WeatherResult(require_double(theVar + "::fake::fractile::" + seasonStr + "::F63"), 0);
    else if (theFractileId == FRACTILE_88)
      return WeatherResult(require_double(theVar + "::fake::fractile::" + seasonStr + "::F88"), 0);
    else if (theFractileId == FRACTILE_98)
      return WeatherResult(require_double(theVar + "::fake::fractile::" + seasonStr + "::F98"), 0);
    else
      return WeatherResult(kFloatMissing, 0);
  }

  std::string dataName("textgen::fractiles");

  WeatherPeriod climatePeriod =
      ClimatologyTools::getClimatologyPeriod(thePeriod, dataName, theSources);

  GridClimatology gc;

  WeatherResult result(kFloatMissing, 0.0);

  // Initialize with a value (Fog) that can not be returned here
  WeatherParameter theWeatherParameter = Fog;

  switch (theFractileId)
  {
    case FRACTILE_02:
      theWeatherParameter = (theFractileType == MIN_FRACTILE
                                 ? NormalMinTemperatureF02
                                 : (theFractileType == MEAN_FRACTILE ? NormalMeanTemperatureF02
                                                                     : NormalMaxTemperatureF02));
      break;
    case FRACTILE_12:
      theWeatherParameter = (theFractileType == MIN_FRACTILE
                                 ? NormalMinTemperatureF12
                                 : (theFractileType == MEAN_FRACTILE ? NormalMeanTemperatureF12
                                                                     : NormalMaxTemperatureF12));
      break;
    case FRACTILE_37:
      theWeatherParameter = (theFractileType == MIN_FRACTILE
                                 ? NormalMinTemperatureF37
                                 : (theFractileType == MEAN_FRACTILE ? NormalMeanTemperatureF37
                                                                     : NormalMaxTemperatureF37));
      break;
    case FRACTILE_50:
      theWeatherParameter = (theFractileType == MIN_FRACTILE
                                 ? NormalMinTemperatureF50
                                 : (theFractileType == MEAN_FRACTILE ? NormalMeanTemperatureF50
                                                                     : NormalMaxTemperatureF50));
      break;
    case FRACTILE_63:
      theWeatherParameter = (theFractileType == MIN_FRACTILE
                                 ? NormalMinTemperatureF63
                                 : (theFractileType == MEAN_FRACTILE ? NormalMeanTemperatureF63
                                                                     : NormalMaxTemperatureF63));
      break;
    case FRACTILE_88:
      theWeatherParameter = (theFractileType == MIN_FRACTILE
                                 ? NormalMinTemperatureF88
                                 : (theFractileType == MEAN_FRACTILE ? NormalMeanTemperatureF88
                                                                     : NormalMaxTemperatureF88));
      break;
    case FRACTILE_98:
      theWeatherParameter = (theFractileType == MIN_FRACTILE
                                 ? NormalMinTemperatureF98
                                 : (theFractileType == MEAN_FRACTILE ? NormalMeanTemperatureF98
                                                                     : NormalMaxTemperatureF98));
      break;
    default:
      break;
  }

  if (theWeatherParameter != Fog)
    result =
        gc.analyze(theVar, theSources, theWeatherParameter, Mean, Mean, theArea, climatePeriod);

  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief returns fractile as a readable string
 *
 * \return Text describing the fractile id
 */
// ----------------------------------------------------------------------

const char* fractile_range(const fractile_id& id)
{
  const char* retval = "No value";

  switch (id)
  {
    case FRACTILE_02:
      retval = "F0,0-F2,5";
      break;
    case FRACTILE_12:
      retval = "F2,5-F12,5";
      break;
    case FRACTILE_37:
      retval = "F12,5-37,5";
      break;
    case FRACTILE_50:
      retval = "F37,5-F50,0";
      break;
    case FRACTILE_63:
      retval = "F50,0-F62,5";
      break;
    case FRACTILE_88:
      retval = "F62,5-F87,5";
      break;
    case FRACTILE_98:
      retval = "F87,5-F97,5";
      break;
    case FRACTILE_100:
      retval = "F97,5-F100,0";
      break;
    case FRACTILE_UNDEFINED:
      break;
  }

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * \brief returns fractile as a readable string
 *
 * \return Text describing the fractile id
 */
// ----------------------------------------------------------------------

const char* fractile_name(const fractile_id& id)
{
  const char* retval = "No value";

  switch (id)
  {
    case FRACTILE_02:
      retval = "F02";
      break;
    case FRACTILE_12:
      retval = "F12";
      break;
    case FRACTILE_37:
      retval = "F37";
      break;
    case FRACTILE_50:
      retval = "F50";
      break;
    case FRACTILE_63:
      retval = "F63";
      break;
    case FRACTILE_88:
      retval = "F88";
      break;
    case FRACTILE_98:
      retval = "F98";
      break;
    case FRACTILE_100:
      retval = "F100";
      break;
    case FRACTILE_UNDEFINED:
      break;
  }

  return retval;
}

}  // namespace TemperatureStoryTools
}  // namespace TextGen

// ======================================================================
