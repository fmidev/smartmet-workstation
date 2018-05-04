#include "TemperatureTools.h"

#include "Story.h"

#include <calculator/GridForecaster.h>
#include <calculator/Settings.h>
#include <calculator/WeatherResult.h>
#include "SeasonTools.h"

namespace TextGen
{
namespace TemperatureTools
{
using namespace Settings;

void min_max_mean_temperature(const string& theVar,
                              const AnalysisSources& theSources,
                              const WeatherArea& theArea,
                              const WeatherPeriod& thePeriod,
                              WeatherResult& theMin,
                              WeatherResult& theMax,
                              WeatherResult& theMean)
{
  GridForecaster theForecaster;

  theMin = theForecaster.analyze(
      theVar + "::min", theSources, Temperature, Minimum, Maximum, theArea, thePeriod);

  theMax = theForecaster.analyze(
      theVar + "::max", theSources, Temperature, Maximum, Maximum, theArea, thePeriod);

  theMean = theForecaster.analyze(
      theVar + "::mean", theSources, Temperature, Mean, Maximum, theArea, thePeriod);
}

// ----------------------------------------------------------------------
/*!
 * \brief calculate morning temperature, default values follow finnish convention
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

  std::string season(
      SeasonTools::isWinterHalf(thePeriod.localStartTime(), thePlainVar) ? "::winter" : "::summer");

  int morning_starthour =
      optional_hour(thePlainVar + season + "::morning_temperature::starthour", default_starthour);
  int morning_endhour =
      optional_hour(thePlainVar + season + "::morning_temperature::endhour", default_endhour);

  TextGenPosixTime time1(year, month, day, morning_starthour, 0, 0);
  TextGenPosixTime time2(year, month, day, morning_endhour, 0, 0);

  WeatherPeriod morningPeriod(time1, time2);

  min_max_mean_temperature(theVar, theSources, theArea, morningPeriod, theMin, theMax, theMean);
}

// ----------------------------------------------------------------------
/*!
 * \brief calculate afternoon temperature, default values follow finnish convention
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
  int year = thePeriod.localStartTime().GetYear();
  int month = thePeriod.localStartTime().GetMonth();
  int day = thePeriod.localStartTime().GetDay();

  int fakeStrPos = theVar.find("::fake");
  std::string thePlainVar(fakeStrPos == -1 ? theVar : theVar.substr(0, fakeStrPos));

  bool is_winter = SeasonTools::isWinterHalf(thePeriod.localStartTime(), thePlainVar);
  int timezone = TextGenPosixTime::GetZoneDifferenceHour(thePeriod.localStartTime(), false);
  std::string season(is_winter ? "::winter" : "::summer");

  // in wintertime convert the default value to localtime
  int default_starthour = (is_winter ? 12 - timezone : 13);
  int default_endhour = (is_winter ? 12 - timezone : 17);

  int afternoon_starthour =
      optional_hour(thePlainVar + season + "::day_temperature::starthour", default_starthour);
  int afternoon_endhour =
      optional_hour(thePlainVar + season + "::day_temperature::endhour", default_endhour);

  TextGenPosixTime time1(year, month, day, afternoon_starthour, 0, 0);
  TextGenPosixTime time2(year, month, day, afternoon_endhour, 0, 0);

  WeatherPeriod dayPeriod(time1, time2);

  min_max_mean_temperature(theVar, theSources, theArea, dayPeriod, theMin, theMax, theMean);
}

}  // namespace TemperatureTools
}  // namespace TextGen
