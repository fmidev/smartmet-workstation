#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "TemperatureStory.h"
#include "Story.h"
#include "MessageLogger.h"
#include <calculator/Settings.h>
#include <calculator/AnalysisSources.h>
#include <calculator/UserWeatherSource.h>
#include <calculator/RegularMaskSource.h>
#include "TemperatureStoryTools.h"
#include "SeasonTools.h"
#include <calculator/Settings.h>

#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiSvgPath.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiSettings.h>
#include <boost/algorithm/string.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;
using namespace TextGen::TemperatureStoryTools;

namespace TemperatureStoryTest
{
enum TemperatureTestType
{
  TEMPERATURE_MAX36_HOURS,
  TEMPERATURE_ANOMALY,
  WIND_ANOMALY
};

shared_ptr<TextGen::Dictionary> dict;
TextGen::PlainTextFormatter formatter;

void require(const TextGen::Story& theStory,
             const string& theLanguage,
             const string& theName,
             const string& theExpected)
{
  dict->init(theLanguage);
  formatter.dictionary(dict);

  TextGen::Paragraph para = theStory.makeStory(theName);
  const string value = para.realize(formatter);

  if (value != theExpected)
  {
    const string msg = value + " <> " + theExpected;
    TEST_FAILED(msg.c_str());
  }
}

string get_story(const TextGen::Story& theStory, const string& theLanguage, const string& theName)
{
  dict->init(theLanguage);
  formatter.dictionary(dict);

  TextGen::Paragraph para = theStory.makeStory(theName);
  const string value = para.realize(formatter);

  return value;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::day()
 */
// ----------------------------------------------------------------------

void temperature_day()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  // 1 day forecast

  {
    TextGenPosixTime time1(2000, 1, 1, 6);
    TextGenPosixTime time2(2000, 1, 2, 6);
    WeatherPeriod period(time1, time2);
    TemperatureStory story(time1, sources, area, period, "day");

    const string fun = "temperature_day";

    Settings::set("day::day::starthour", "6");
    Settings::set("day::day::endhour", "18");
    Settings::set("day::night::starthour", "18");
    Settings::set("day::night::endhour", "6");

    Settings::set("day::comparison::significantly_higher", "5");
    Settings::set("day::comparison::higher", "3");
    Settings::set("day::comparison::somewhat_higher", "2");
    Settings::set("day::comparison::somewhat_lower", "2");
    Settings::set("day::comparison::lower", "3");
    Settings::set("day::comparison::significantly_lower", "5");

    Settings::set("day::fake::day1::area::mean", "15,0");
    Settings::set("day::fake::day1::area::min", "15,0");
    Settings::set("day::fake::day1::area::max", "15,0");

    Settings::set("day::fake::day1::coast::mean", "15,0");
    Settings::set("day::fake::day1::coast::min", "15,0");
    Settings::set("day::fake::day1::coast::max", "15,0");

    Settings::set("day::fake::day1::inland::mean", "15,0");
    Settings::set("day::fake::day1::inland::min", "15,0");
    Settings::set("day::fake::day1::inland::max", "15,0");

    Settings::set("day::fake::night1::area::mean", "15,0");
    Settings::set("day::fake::night1::area::min", "15,0");
    Settings::set("day::fake::night1::area::max", "15,0");

    Settings::set("day::fake::night1::coast::mean", "15,0");
    Settings::set("day::fake::night1::coast::min", "15,0");
    Settings::set("day::fake::night1::coast::max", "15,0");

    Settings::set("day::fake::night1::inland::mean", "15,0");
    Settings::set("day::fake::night1::inland::min", "15,0");
    Settings::set("day::fake::night1::inland::max", "15,0");

    require(story, "fi", fun, "Lämpötila on noin 15 astetta.");
    require(story, "sv", fun, "Temperaturen är cirka 15 grader.");
    require(story, "en", fun, "Temperature is about 15 degrees.");

    Settings::set("day::fake::day1::coast::mean", "10,0");
    Settings::set("day::fake::day1::coast::min", "8,0");
    Settings::set("day::fake::day1::coast::max", "12,0");

    Settings::set("day::fake::day1::inland::mean", "16,0");
    Settings::set("day::fake::day1::inland::min", "15,0");
    Settings::set("day::fake::day1::inland::max", "14,0");

    require(story, "fi", fun, "Lämpötila on noin 16 astetta, rannikolla alempi.");
    require(story, "sv", fun, "Temperaturen är cirka 16 grader, vid kusten lägre.");
    require(story, "en", fun, "Temperature is about 16 degrees, on the coastal area lower.");

    Settings::set("day::fake::night1::area::mean", "10,0");
    Settings::set("day::fake::night1::area::min", "10,0");
    Settings::set("day::fake::night1::area::max", "10,0");

    Settings::set("day::fake::night1::coast::mean", "10,0");
    Settings::set("day::fake::night1::coast::min", "10,0");
    Settings::set("day::fake::night1::coast::max", "10,0");

    Settings::set("day::fake::night1::inland::mean", "10,0");
    Settings::set("day::fake::night1::inland::min", "10,0");
    Settings::set("day::fake::night1::inland::max", "10,0");

    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on noin 16 astetta, rannikolla huomattavasti alempi. Yön alin "
            "lämpötila on noin 10 astetta.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är cirka 16 grader, vid kusten betydligt lägre. Nattens "
            "lägsta temperatur är cirka 10 grader.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is about 16 degrees, on the coastal area significantly "
            "lower. The minimum night temperature is about 10 degrees.");

    Settings::set("day::fake::day1::coast::mean", "-10,0");
    Settings::set("day::fake::day1::coast::min", "-12,0");
    Settings::set("day::fake::day1::coast::max", "-8,0");

    Settings::set("day::fake::night1::coast::mean", "7,0");
    Settings::set("day::fake::night1::coast::min", "7,0");
    Settings::set("day::fake::night1::coast::max", "7,0");

    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on noin 16 astetta, rannikolla -8...-12 astetta. Yön alin "
            "lämpötila on noin 10 astetta, rannikolla alempi.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är cirka 16 grader, vid kusten -8...-12 grader. Nattens "
            "lägsta temperatur är cirka 10 grader, vid kusten lägre.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is about 16 degrees, on the coastal area -8...-12 "
            "degrees. The minimum night temperature is about 10 degrees, on the coastal area "
            "lower.");
  }

  // 1 1/2 days

  {
    TextGenPosixTime time1(2000, 1, 1, 6);
    TextGenPosixTime time2(2000, 1, 2, 18);
    WeatherPeriod period(time1, time2);
    TemperatureStory story(time1, sources, area, period, "day");

    const string fun = "temperature_day";

    Settings::set("day::today::phrases", "today,tomorrow,weekday");

    Settings::set("day::day::starthour", "6");
    Settings::set("day::day::endhour", "18");
    Settings::set("day::night::starthour", "18");
    Settings::set("day::night::endhour", "6");

    Settings::set("day::comparison::significantly_higher", "5");
    Settings::set("day::comparison::higher", "3");
    Settings::set("day::comparison::somewhat_higher", "2");
    Settings::set("day::comparison::somewhat_lower", "2");
    Settings::set("day::comparison::lower", "3");
    Settings::set("day::comparison::significantly_lower", "5");

    Settings::set("day::fake::day1::area::mean", "15,0");
    Settings::set("day::fake::day1::area::min", "15,0");
    Settings::set("day::fake::day1::area::max", "15,0");

    Settings::set("day::fake::day1::coast::mean", "15,0");
    Settings::set("day::fake::day1::coast::min", "15,0");
    Settings::set("day::fake::day1::coast::max", "15,0");

    Settings::set("day::fake::day1::inland::mean", "15,0");
    Settings::set("day::fake::day1::inland::min", "15,0");
    Settings::set("day::fake::day1::inland::max", "15,0");

    Settings::set("day::fake::night1::area::mean", "15,0");
    Settings::set("day::fake::night1::area::min", "15,0");
    Settings::set("day::fake::night1::area::max", "15,0");

    Settings::set("day::fake::night1::coast::mean", "15,0");
    Settings::set("day::fake::night1::coast::min", "15,0");
    Settings::set("day::fake::night1::coast::max", "15,0");

    Settings::set("day::fake::night1::inland::mean", "15,0");
    Settings::set("day::fake::night1::inland::min", "15,0");
    Settings::set("day::fake::night1::inland::max", "15,0");

    Settings::set("day::fake::day2::area::mean", "11,0");
    Settings::set("day::fake::day2::area::min", "10,0");
    Settings::set("day::fake::day2::area::max", "13,0");

    Settings::set("day::fake::day2::coast::mean", "5,0");
    Settings::set("day::fake::day2::coast::min", "0,0");
    Settings::set("day::fake::day2::coast::max", "15,0");

    Settings::set("day::fake::day2::inland::mean", "20,0");
    Settings::set("day::fake::day2::inland::min", "15,0");
    Settings::set("day::fake::day2::inland::max", "25,0");

    require(story,
            "fi",
            fun,
            "Lämpötila on tänään noin 15 astetta. Päivän ylin lämpötila on huomenna 15...25 "
            "astetta, rannikolla 0...+15 astetta.");
    require(story,
            "sv",
            fun,
            "Temperaturen är i dag cirka 15 grader. Dagens högsta temperatur är i morgon 15...25 "
            "grader, vid kusten 0...+15 grader.");
    require(story,
            "en",
            fun,
            "Temperature is today about 15 degrees. The maximum day temperature is tomorrow "
            "15...25 degrees, on the coastal area 0...+15 degrees.");

    Settings::set("day::fake::day1::coast::mean", "10,0");
    Settings::set("day::fake::day1::coast::min", "8,0");
    Settings::set("day::fake::day1::coast::max", "12,0");

    Settings::set("day::fake::day1::inland::mean", "16,0");
    Settings::set("day::fake::day1::inland::min", "15,0");
    Settings::set("day::fake::day1::inland::max", "14,0");

    require(story,
            "fi",
            fun,
            "Lämpötila on tänään noin 16 astetta, rannikolla alempi. Päivän ylin lämpötila on "
            "huomenna 15...25 astetta, rannikolla 0...+15 astetta.");
    require(story,
            "sv",
            fun,
            "Temperaturen är i dag cirka 16 grader, vid kusten lägre. Dagens högsta temperatur är "
            "i morgon 15...25 grader, vid kusten 0...+15 grader.");
    require(story,
            "en",
            fun,
            "Temperature is today about 16 degrees, on the coastal area lower. The maximum day "
            "temperature is tomorrow 15...25 degrees, on the coastal area 0...+15 degrees.");

    Settings::set("day::fake::night1::area::mean", "10,0");
    Settings::set("day::fake::night1::area::min", "10,0");
    Settings::set("day::fake::night1::area::max", "10,0");

    Settings::set("day::fake::night1::coast::mean", "10,0");
    Settings::set("day::fake::night1::coast::min", "10,0");
    Settings::set("day::fake::night1::coast::max", "10,0");

    Settings::set("day::fake::night1::inland::mean", "10,0");
    Settings::set("day::fake::night1::inland::min", "10,0");
    Settings::set("day::fake::night1::inland::max", "10,0");

    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on tänään noin 16 astetta, rannikolla huomattavasti alempi. Yön "
            "alin lämpötila on noin 10 astetta. Päivän ylin lämpötila on huomenna 15...25 astetta, "
            "rannikolla 0...+15 astetta.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är i dag cirka 16 grader, vid kusten betydligt lägre. "
            "Nattens lägsta temperatur är cirka 10 grader. Dagens högsta temperatur är i morgon "
            "15...25 grader, vid kusten 0...+15 grader.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is today about 16 degrees, on the coastal area "
            "significantly lower. The minimum night temperature is about 10 degrees. The maximum "
            "day temperature is tomorrow 15...25 degrees, on the coastal area 0...+15 degrees.");

    Settings::set("day::fake::day1::coast::mean", "-10,0");
    Settings::set("day::fake::day1::coast::min", "-12,0");
    Settings::set("day::fake::day1::coast::max", "-8,0");

    Settings::set("day::fake::night1::coast::mean", "7,0");
    Settings::set("day::fake::night1::coast::min", "7,0");
    Settings::set("day::fake::night1::coast::max", "7,0");

    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on tänään noin 16 astetta, rannikolla -8...-12 astetta. Yön "
            "alin lämpötila on noin 10 astetta, rannikolla alempi. Päivän ylin lämpötila on "
            "huomenna 15...25 astetta, rannikolla 0...+15 astetta.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är i dag cirka 16 grader, vid kusten -8...-12 grader. "
            "Nattens lägsta temperatur är cirka 10 grader, vid kusten lägre. Dagens högsta "
            "temperatur är i morgon 15...25 grader, vid kusten 0...+15 grader.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is today about 16 degrees, on the coastal area -8...-12 "
            "degrees. The minimum night temperature is about 10 degrees, on the coastal area "
            "lower. The maximum day temperature is tomorrow 15...25 degrees, on the coastal area "
            "0...+15 degrees.");
  }

  TEST_PASSED();
}
// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::mean()
 */
// ----------------------------------------------------------------------

void temperature_mean()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  TemperatureStory story(time1, sources, area, period, "mean");

  const string fun = "temperature_mean";

  Settings::set("mean::fake::mean", "0.1,0");
  require(story, "fi", fun, "Keskilämpötila 0 astetta.");
  require(story, "sv", fun, "Medeltemperaturen 0 grader.");
  require(story, "en", fun, "Mean temperature 0 degrees.");

  Settings::set("mean::fake::mean", "0.5,0");
  require(story, "fi", fun, "Keskilämpötila 1 astetta.");
  require(story, "sv", fun, "Medeltemperaturen 1 grader.");
  require(story, "en", fun, "Mean temperature 1 degrees.");

  Settings::set("mean::fake::mean", "10,0");
  require(story, "fi", fun, "Keskilämpötila 10 astetta.");
  require(story, "sv", fun, "Medeltemperaturen 10 grader.");
  require(story, "en", fun, "Mean temperature 10 degrees.");

  Settings::set("mean::fake::mean", "-10.5,0");
  require(story, "fi", fun, "Keskilämpötila -11 astetta.");
  require(story, "sv", fun, "Medeltemperaturen -11 grader.");
  require(story, "en", fun, "Mean temperature -11 degrees.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::meanmax()
 */
// ----------------------------------------------------------------------

void temperature_meanmax()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  TemperatureStory story(time1, sources, area, period, "meanmax");

  const string fun = "temperature_meanmax";

  Settings::set("meanmax::fake::mean", "0.1,0");
  require(story, "fi", fun, "Keskimääräinen ylin lämpötila 0 astetta.");
  require(story, "sv", fun, "Maximitemperaturen i medeltal 0 grader.");
  require(story, "en", fun, "Mean maximum temperature 0 degrees.");

  Settings::set("meanmax::fake::mean", "0.5,0");
  require(story, "fi", fun, "Keskimääräinen ylin lämpötila 1 astetta.");
  require(story, "sv", fun, "Maximitemperaturen i medeltal 1 grader.");
  require(story, "en", fun, "Mean maximum temperature 1 degrees.");

  Settings::set("meanmax::fake::mean", "10,0");
  require(story, "fi", fun, "Keskimääräinen ylin lämpötila 10 astetta.");
  require(story, "sv", fun, "Maximitemperaturen i medeltal 10 grader.");
  require(story, "en", fun, "Mean maximum temperature 10 degrees.");

  Settings::set("meanmax::fake::mean", "-10.5,0");
  require(story, "fi", fun, "Keskimääräinen ylin lämpötila -11 astetta.");
  require(story, "sv", fun, "Maximitemperaturen i medeltal -11 grader.");
  require(story, "en", fun, "Mean maximum temperature -11 degrees.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::meanmin()
 */
// ----------------------------------------------------------------------

void temperature_meanmin()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  TemperatureStory story(time1, sources, area, period, "meanmin");

  const string fun = "temperature_meanmin";

  Settings::set("meanmin::fake::mean", "0.1,0");
  require(story, "fi", fun, "Keskimääräinen alin lämpötila 0 astetta.");
  require(story, "sv", fun, "Minimitemperaturen i medeltal 0 grader.");
  require(story, "en", fun, "Mean minimum temperature 0 degrees.");

  Settings::set("meanmin::fake::mean", "0.5,0");
  require(story, "fi", fun, "Keskimääräinen alin lämpötila 1 astetta.");
  require(story, "sv", fun, "Minimitemperaturen i medeltal 1 grader.");
  require(story, "en", fun, "Mean minimum temperature 1 degrees.");

  Settings::set("meanmin::fake::mean", "10,0");
  require(story, "fi", fun, "Keskimääräinen alin lämpötila 10 astetta.");
  require(story, "sv", fun, "Minimitemperaturen i medeltal 10 grader.");
  require(story, "en", fun, "Mean minimum temperature 10 degrees.");

  Settings::set("meanmin::fake::mean", "-10.5,0");
  require(story, "fi", fun, "Keskimääräinen alin lämpötila -11 astetta.");
  require(story, "sv", fun, "Minimitemperaturen i medeltal -11 grader.");
  require(story, "en", fun, "Mean minimum temperature -11 degrees.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::dailymax()
 */
// ----------------------------------------------------------------------

void temperature_dailymax()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  const string fun = "temperature_dailymax";

  Settings::set("dailymax::comparison::significantly_higher", "6");
  Settings::set("dailymax::comparison::higher", "4");
  Settings::set("dailymax::comparison::somewhat_higher", "2");
  Settings::set("dailymax::comparison::somewhat_lower", "2");
  Settings::set("dailymax::comparison::lower", "4");
  Settings::set("dailymax::comparison::significantly_lower", "6");
  Settings::set("dailymax::mininterval", "2");
  Settings::set("dailymax::always_interval_zero", "true");
  Settings::set("dailymax::day::starthour", "6");
  Settings::set("dailymax::day::endhour", "18");
  Settings::set("dailymax::today::phrases", "weekday");
  Settings::set("dailymax::next_day::phrases", "followingday");

  // Test the part concerning day 1
  {
    TextGenPosixTime time1(2003, 6, 1);
    TextGenPosixTime time2(2003, 6, 2);
    WeatherPeriod period(time1, time2);
    TemperatureStory story(time1, sources, area, period, "dailymax");

    Settings::set("dailymax::fake::day1::minimum", "5,0");
    Settings::set("dailymax::fake::day1::mean", "5,0");
    Settings::set("dailymax::fake::day1::maximum", "5,0");
    require(story, "fi", fun, "Päivän ylin lämpötila on sunnuntaina noin 5 astetta.");
    require(story, "sv", fun, "Dagens högsta temperatur är på söndagen cirka 5 grader.");
    require(story, "en", fun, "The maximum day temperature is on Sunday about 5 degrees.");

    Settings::set("dailymax::fake::day1::minimum", "5,0");
    Settings::set("dailymax::fake::day1::mean", "6,0");
    Settings::set("dailymax::fake::day1::maximum", "6,0");
    require(story, "fi", fun, "Päivän ylin lämpötila on sunnuntaina noin 6 astetta.");
    require(story, "sv", fun, "Dagens högsta temperatur är på söndagen cirka 6 grader.");
    require(story, "en", fun, "The maximum day temperature is on Sunday about 6 degrees.");

    Settings::set("dailymax::fake::day1::minimum", "5,0");
    Settings::set("dailymax::fake::day1::mean", "6,0");
    Settings::set("dailymax::fake::day1::maximum", "7,0");
    require(story, "fi", fun, "Päivän ylin lämpötila on sunnuntaina 5...7 astetta.");
    require(story, "sv", fun, "Dagens högsta temperatur är på söndagen 5...7 grader.");
    require(story, "en", fun, "The maximum day temperature is on Sunday 5...7 degrees.");

    Settings::set("dailymax::fake::day1::minimum", "0,0");
    Settings::set("dailymax::fake::day1::mean", "0,0");
    Settings::set("dailymax::fake::day1::maximum", "0,0");
    require(story, "fi", fun, "Päivän ylin lämpötila on sunnuntaina noin 0 astetta.");
    require(story, "sv", fun, "Dagens högsta temperatur är på söndagen cirka 0 grader.");
    require(story, "en", fun, "The maximum day temperature is on Sunday about 0 degrees.");

    Settings::set("dailymax::fake::day1::minimum", "0,0");
    Settings::set("dailymax::fake::day1::mean", "0,0");
    Settings::set("dailymax::fake::day1::maximum", "1,0");
    require(story, "fi", fun, "Päivän ylin lämpötila on sunnuntaina 0...+1 astetta.");
    require(story, "sv", fun, "Dagens högsta temperatur är på söndagen 0...+1 grader.");
    require(story, "en", fun, "The maximum day temperature is on Sunday 0...+1 degrees.");

    Settings::set("dailymax::fake::day1::minimum", "-1,0");
    Settings::set("dailymax::fake::day1::mean", "0,0");
    Settings::set("dailymax::fake::day1::maximum", "0,0");
    require(story, "fi", fun, "Päivän ylin lämpötila on sunnuntaina 0...-1 astetta.");
    require(story, "sv", fun, "Dagens högsta temperatur är på söndagen 0...-1 grader.");
    require(story, "en", fun, "The maximum day temperature is on Sunday 0...-1 degrees.");

    Settings::set("dailymax::fake::day1::minimum", "-1,0");
    Settings::set("dailymax::fake::day1::mean", "0,0");
    Settings::set("dailymax::fake::day1::maximum", "1,0");
    require(story, "fi", fun, "Päivän ylin lämpötila on sunnuntaina -1...+1 astetta.");
    require(story, "sv", fun, "Dagens högsta temperatur är på söndagen -1...+1 grader.");
    require(story, "en", fun, "The maximum day temperature is on Sunday -1...+1 degrees.");
  }

  // Test the part concerning day 2
  {
    TextGenPosixTime time1(2003, 6, 1);
    TextGenPosixTime time2(2003, 6, 3);
    WeatherPeriod period(time1, time2);
    TemperatureStory story(time1, sources, area, period, "dailymax");

    Settings::set("dailymax::fake::day1::minimum", "5,0");
    Settings::set("dailymax::fake::day1::mean", "6,0");
    Settings::set("dailymax::fake::day1::maximum", "7,0");

    // change 0 degrees
    Settings::set("dailymax::fake::day2::minimum", "5,0");
    Settings::set("dailymax::fake::day2::mean", "6,0");
    Settings::set("dailymax::fake::day2::maximum", "7,0");
    require(
        story,
        "fi",
        fun,
        "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä suunnilleen sama.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag ungefär densamma.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day about the "
            "same.");

    // change 1 degrees
    Settings::set("dailymax::fake::day2::minimum", "6,0");
    Settings::set("dailymax::fake::day2::mean", "7,0");
    Settings::set("dailymax::fake::day2::maximum", "8,0");
    require(
        story,
        "fi",
        fun,
        "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä suunnilleen sama.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag ungefär densamma.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day about the "
            "same.");

    // change 2 degrees
    Settings::set("dailymax::fake::day2::minimum", "7,0");
    Settings::set("dailymax::fake::day2::mean", "8,0");
    Settings::set("dailymax::fake::day2::maximum", "9,0");
    require(
        story,
        "fi",
        fun,
        "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä hieman korkeampi.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag något högre.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day somewhat "
            "higher.");

    // change 4 degrees
    Settings::set("dailymax::fake::day2::minimum", "9,0");
    Settings::set("dailymax::fake::day2::mean", "10,0");
    Settings::set("dailymax::fake::day2::maximum", "11,0");
    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä korkeampi.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag högre.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day higher.");

    // change 6 degrees
    Settings::set("dailymax::fake::day2::minimum", "11,0");
    Settings::set("dailymax::fake::day2::mean", "12,0");
    Settings::set("dailymax::fake::day2::maximum", "13,0");
    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä huomattavasti "
            "korkeampi.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag betydligt högre.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day "
            "significantly higher.");

    // change - 2 degrees
    Settings::set("dailymax::fake::day2::minimum", "3,0");
    Settings::set("dailymax::fake::day2::mean", "4,0");
    Settings::set("dailymax::fake::day2::maximum", "5,0");
    require(
        story,
        "fi",
        fun,
        "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä hieman alempi.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag något lägre.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day somewhat "
            "lower.");

    // change -4 degrees
    Settings::set("dailymax::fake::day2::minimum", "1,0");
    Settings::set("dailymax::fake::day2::mean", "2,0");
    Settings::set("dailymax::fake::day2::maximum", "3,0");
    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä alempi.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag lägre.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day lower.");

    // change -6 degrees
    Settings::set("dailymax::fake::day2::minimum", "-1,0");
    Settings::set("dailymax::fake::day2::mean", "0,0");
    Settings::set("dailymax::fake::day2::maximum", "1,0");
    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä huomattavasti "
            "alempi.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag betydligt lägre.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day "
            "significantly lower.");
  }

  // Test the part concerning days 3 and 4
  {
    TextGenPosixTime time1(2003, 6, 1);
    TextGenPosixTime time2(2003, 6, 5);
    WeatherPeriod period(time1, time2);
    TemperatureStory story(time1, sources, area, period, "dailymax");

    Settings::set("dailymax::fake::day1::minimum", "5,0");
    Settings::set("dailymax::fake::day1::mean", "6,0");
    Settings::set("dailymax::fake::day1::maximum", "7,0");

    Settings::set("dailymax::fake::day2::minimum", "5,0");
    Settings::set("dailymax::fake::day2::mean", "6,0");
    Settings::set("dailymax::fake::day2::maximum", "7,0");

    Settings::set("dailymax::fake::day3::minimum", "8,0");
    Settings::set("dailymax::fake::day3::mean", "10,0");
    Settings::set("dailymax::fake::day3::maximum", "12,0");

    Settings::set("dailymax::fake::day4::minimum", "3,0");
    Settings::set("dailymax::fake::day4::mean", "4,0");
    Settings::set("dailymax::fake::day4::maximum", "4,0");

    require(story,
            "fi",
            fun,
            "Päivän ylin lämpötila on sunnuntaina 5...7 astetta, seuraavana päivänä suunnilleen "
            "sama, tiistaina 8...12 astetta, keskiviikkona noin 4 astetta.");
    require(story,
            "sv",
            fun,
            "Dagens högsta temperatur är på söndagen 5...7 grader, följande dag ungefär densamma, "
            "på tisdagen 8...12 grader, på onsdagen cirka 4 grader.");
    require(story,
            "en",
            fun,
            "The maximum day temperature is on Sunday 5...7 degrees, the following day about the "
            "same, on Tuesday 8...12 degrees, on Wednesday about 4 degrees.");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::weekly_minmax()
 */
// ----------------------------------------------------------------------

void temperature_weekly_minmax()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 5);
  WeatherPeriod period(time1, time2);
  TemperatureStory story(time1, sources, area, period, "weekly_minmax");

  const string fun = "temperature_weekly_minmax";

  Settings::set("weekly_minmax::day::starthour", "6");
  Settings::set("weekly_minmax::day::endhour", "18");
  Settings::set("weekly_minmax::night::starthour", "18");
  Settings::set("weekly_minmax::night::endhour", "6");

  Settings::set("weekly_minmax::fake::day::minimum", "10,0");
  Settings::set("weekly_minmax::fake::day::mean", "12,0");
  Settings::set("weekly_minmax::fake::day::maximum", "15,0");
  Settings::set("weekly_minmax::fake::night::minimum", "2,0");
  Settings::set("weekly_minmax::fake::night::mean", "4,0");
  Settings::set("weekly_minmax::fake::night::maximum", "5,0");
  require(story,
          "fi",
          fun,
          "Päivien ylin lämpötila on 10...15 astetta, öiden alin lämpötila 2...5 astetta.");
  require(
      story,
      "sv",
      fun,
      "Dagens maximi temperaturer är 10...15 grader, nattens minimi temperaturer 2...5 grader.");
  require(
      story,
      "en",
      fun,
      "Daily maximum temperature is 10...15 degrees, nightly minimum temperature 2...5 degrees.");

  Settings::set("weekly_minmax::night::mininterval", "4");
  require(story,
          "fi",
          fun,
          "Päivien ylin lämpötila on 10...15 astetta, öiden alin lämpötila noin 4 astetta.");
  require(
      story,
      "sv",
      fun,
      "Dagens maximi temperaturer är 10...15 grader, nattens minimi temperaturer cirka 4 grader.");
  require(
      story,
      "en",
      fun,
      "Daily maximum temperature is 10...15 degrees, nightly minimum temperature about 4 degrees.");

  Settings::set("weekly_minmax::day::mininterval", "6");
  Settings::set("weekly_minmax::night::mininterval", "2");
  require(story,
          "fi",
          fun,
          "Päivien ylin lämpötila on noin 12 astetta, öiden alin lämpötila 2...5 astetta.");
  require(
      story,
      "sv",
      fun,
      "Dagens maximi temperaturer är cirka 12 grader, nattens minimi temperaturer 2...5 grader.");
  require(
      story,
      "en",
      fun,
      "Daily maximum temperature is about 12 degrees, nightly minimum temperature 2...5 degrees.");

  Settings::set("weekly_minmax::emphasize_night_minimum", "true");
  require(story,
          "fi",
          fun,
          "Päivien ylin lämpötila on noin 12 astetta, öiden alin lämpötila noin 2 astetta.");
  require(
      story,
      "sv",
      fun,
      "Dagens maximi temperaturer är cirka 12 grader, nattens minimi temperaturer cirka 2 grader.");
  require(story,
          "en",
          fun,
          "Daily maximum temperature is about 12 degrees, nightly minimum temperature about 2 "
          "degrees.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::weekly_averages()
 */
// ----------------------------------------------------------------------

void temperature_weekly_averages()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 5);
  WeatherPeriod period(time1, time2);
  TemperatureStory story(time1, sources, area, period, "weekly_averages");

  const string fun = "temperature_weekly_averages";

  Settings::set("weekly_averages::day::starthour", "6");
  Settings::set("weekly_averages::day::endhour", "18");
  Settings::set("weekly_averages::night::starthour", "18");
  Settings::set("weekly_averages::night::endhour", "6");

  Settings::set("weekly_averages::fake::day::minimum", "10,0");
  Settings::set("weekly_averages::fake::day::mean", "12,0");
  Settings::set("weekly_averages::fake::day::maximum", "15,0");
  Settings::set("weekly_averages::fake::night::minimum", "2,0");
  Settings::set("weekly_averages::fake::night::mean", "4,0");
  Settings::set("weekly_averages::fake::night::maximum", "5,0");
  require(story, "fi", fun, "Päivälämpötila on 10...15 astetta, yölämpötila 2...5 astetta.");
  require(story, "sv", fun, "Dagstemperaturen är 10...15 grader, nattemperaturen 2...5 grader.");
  require(
      story, "en", fun, "Daily temperature is 10...15 degrees, nightly temperature 2...5 degrees.");

  Settings::set("weekly_averages::night::mininterval", "4");
  require(story, "fi", fun, "Päivälämpötila on 10...15 astetta, yölämpötila noin 4 astetta.");
  require(story, "sv", fun, "Dagstemperaturen är 10...15 grader, nattemperaturen cirka 4 grader.");
  require(story,
          "en",
          fun,
          "Daily temperature is 10...15 degrees, nightly temperature about 4 degrees.");

  Settings::set("weekly_averages::day::mininterval", "6");
  Settings::set("weekly_averages::night::mininterval", "2");
  require(story, "fi", fun, "Päivälämpötila on noin 12 astetta, yölämpötila 2...5 astetta.");
  require(story, "sv", fun, "Dagstemperaturen är cirka 12 grader, nattemperaturen 2...5 grader.");
  require(story,
          "en",
          fun,
          "Daily temperature is about 12 degrees, nightly temperature 2...5 degrees.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::range()
 */
// ----------------------------------------------------------------------

void temperature_range()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  const string fun = "temperature_range";

  Settings::set("range::mininterval", "2");
  Settings::set("range::always_interval_zero", "true");

  TextGenPosixTime time1(2003, 6, 1);
  TextGenPosixTime time2(2003, 6, 2);
  WeatherPeriod period(time1, time2);
  TemperatureStory story(time1, sources, area, period, "range");

  Settings::set("range::fake::minimum", "5,0");
  Settings::set("range::fake::mean", "5,0");
  Settings::set("range::fake::maximum", "5,0");
  require(story, "fi", fun, "Lämpötila on noin 5 astetta.");
  require(story, "sv", fun, "Temperaturen är cirka 5 grader.");
  require(story, "en", fun, "Temperature is about 5 degrees.");

  Settings::set("range::fake::minimum", "5,0");
  Settings::set("range::fake::mean", "6,0");
  Settings::set("range::fake::maximum", "6,0");
  require(story, "fi", fun, "Lämpötila on noin 6 astetta.");
  require(story, "sv", fun, "Temperaturen är cirka 6 grader.");
  require(story, "en", fun, "Temperature is about 6 degrees.");

  Settings::set("range::fake::minimum", "5,0");
  Settings::set("range::fake::mean", "6,0");
  Settings::set("range::fake::maximum", "7,0");
  require(story, "fi", fun, "Lämpötila on 5...7 astetta.");
  require(story, "sv", fun, "Temperaturen är 5...7 grader.");
  require(story, "en", fun, "Temperature is 5...7 degrees.");

  Settings::set("range::fake::minimum", "0,0");
  Settings::set("range::fake::mean", "0,0");
  Settings::set("range::fake::maximum", "1,0");
  require(story, "fi", fun, "Lämpötila on 0...+1 astetta.");
  require(story, "sv", fun, "Temperaturen är 0...+1 grader.");
  require(story, "en", fun, "Temperature is 0...+1 degrees.");

  Settings::set("range::fake::minimum", "-1,0");
  Settings::set("range::fake::mean", "0,0");
  Settings::set("range::fake::maximum", "0,0");
  require(story, "fi", fun, "Lämpötila on 0...-1 astetta.");
  require(story, "sv", fun, "Temperaturen är 0...-1 grader.");
  require(story, "en", fun, "Temperature is 0...-1 degrees.");

  Settings::set("range::fake::minimum", "-1,0");
  Settings::set("range::fake::mean", "0,0");
  Settings::set("range::fake::maximum", "1,0");
  require(story, "fi", fun, "Lämpötila on -1...+1 astetta.");
  require(story, "sv", fun, "Temperaturen är -1...+1 grader.");
  require(story, "en", fun, "Temperature is -1...+1 degrees.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TemperatureStory::max36_hours()
 */
// ----------------------------------------------------------------------

#ifdef LATER
struct AnomalyTestParam
{
  AnomalyTestParam(const char* d1_temperature_inlandmin,
                   const char* d1_temperature_inlandmax,
                   const char* d1_temperature_inlandmean,
                   const char* d1_temperature_coastmin,
                   const char* d1_temperature_coastmax,
                   const char* d1_temperature_coastmean,
                   const char* d1_temperature_areamin,
                   const char* d1_temperature_areamax,
                   const char* d1_temperature_areamean,

                   const char* d1_temperature_morning_inlandmin,
                   const char* d1_temperature_morning_inlandmax,
                   const char* d1_temperature_morning_inlandmean,
                   const char* d1_temperature_morning_coastmin,
                   const char* d1_temperature_morning_coastmax,
                   const char* d1_temperature_morning_coastmean,
                   const char* d1_temperature_morning_areamin,
                   const char* d1_temperature_morning_areamax,
                   const char* d1_temperature_morning_areamean,

                   const char* d1_temperature_afternoon_inlandmin,
                   const char* d1_temperature_afternoon_inlandmax,
                   const char* d1_temperature_afternoon_inlandmean,
                   const char* d1_temperature_afternoon_coastmin,
                   const char* d1_temperature_afternoon_coastmax,
                   const char* d1_temperature_afternoon_coastmean,
                   const char* d1_temperature_afternoon_areamin,
                   const char* d1_temperature_afternoon_areamax,
                   const char* d1_temperature_afternoon_areamean,

                   const char* nite_temperature_min,
                   const char* nite_temperature_max,
                   const char* nite_temperature_mean,

                   const char* d2_temperature_inlandmin,
                   const char* d2_temperature_inlandmax,
                   const char* d2_temperature_inlandmean,
                   const char* d2_temperature_coastmin,
                   const char* d2_temperature_coastmax,
                   const char* d2_temperature_coastmean,
                   const char* d2_temperature_areamin,
                   const char* d2_temperature_areamax,
                   const char* d2_temperature_areamean,

                   const char* d2_temperature_morning_inlandmin,
                   const char* d2_temperature_morning_inlandmax,
                   const char* d2_temperature_morning_inlandmean,
                   const char* d2_temperature_morning_coastmin,
                   const char* d2_temperature_morning_coastmax,
                   const char* d2_temperature_morning_coastmean,
                   const char* d2_temperature_morning_areamin,
                   const char* d2_temperature_morning_areamax,
                   const char* d2_temperature_morning_areamean,

                   const char* d2_temperature_afternoon_inlandmin,
                   const char* d2_temperature_afternoon_inlandmax,
                   const char* d2_temperature_afternoon_inlandmean,
                   const char* d2_temperature_afternoon_coastmin,
                   const char* d2_temperature_afternoon_coastmax,
                   const char* d2_temperature_afternoon_coastmean,
                   const char* d2_temperature_afternoon_areamin,
                   const char* d2_temperature_afternoon_areamax,
                   const char* d2_temperature_afternoon_areamean,

                   const char* d2_windspeed_morning_inlandmin,
                   const char* d2_windspeed_morning_inlandmax,
                   const char* d2_windspeed_morning_inlandmean,
                   const char* d2_windspeed_morning_coastmin,
                   const char* d2_windspeed_morning_coastmax,
                   const char* d2_windspeed_morning_coastmean,

                   const char* d2_windspeed_afternoon_inlandmin,
                   const char* d2_windspeed_afternoon_inlandmax,
                   const char* d2_windspeed_afternoon_inlandmean,
                   const char* d2_windspeed_afternoon_coastmin,
                   const char* d2_windspeed_afternoon_coastmax,
                   const char* d2_windspeed_afternoon_coastmean,

                   const char* d2_windchill_morning_inlandmin,
                   const char* d2_windchill_morning_inlandmax,
                   const char* d2_windchill_morning_inlandmean,
                   const char* d2_windchill_morning_coastmin,
                   const char* d2_windchill_morning_coastmax,
                   const char* d2_windchill_morning_coastmean,

                   const char* d2_windchill_afternoon_inlandmin,
                   const char* d2_windchill_afternoon_inlandmax,
                   const char* d2_windchill_afternoon_inlandmean,
                   const char* d2_windchill_afternoon_coastmin,
                   const char* d2_windchill_afternoon_coastmax,
                   const char* d2_windchill_afternoon_coastmean,
                   const char* story)
      : anomaly_d1_temperature_inlandmin(d1_temperature_inlandmin),
        anomaly_d1_temperature_inlandmax(d1_temperature_inlandmax),
        anomaly_d1_temperature_inlandmean(d1_temperature_inlandmean),
        anomaly_d1_temperature_coastmin(d1_temperature_coastmin),
        anomaly_d1_temperature_coastmax(d1_temperature_coastmax),
        anomaly_d1_temperature_coastmean(d1_temperature_coastmean),
        anomaly_d1_temperature_areamin(d1_temperature_areamin),
        anomaly_d1_temperature_areamax(d1_temperature_areamax),
        anomaly_d1_temperature_areamean(d1_temperature_areamean),

        anomaly_d1_temperature_morning_inlandmin(d1_temperature_morning_inlandmin),
        anomaly_d1_temperature_morning_inlandmax(d1_temperature_morning_inlandmax),
        anomaly_d1_temperature_morning_inlandmean(d1_temperature_morning_inlandmean),
        anomaly_d1_temperature_morning_coastmin(d1_temperature_morning_coastmin),
        anomaly_d1_temperature_morning_coastmax(d1_temperature_morning_coastmax),
        anomaly_d1_temperature_morning_coastmean(d1_temperature_morning_coastmean),
        anomaly_d1_temperature_morning_areamin(d1_temperature_morning_areamin),
        anomaly_d1_temperature_morning_areamax(d1_temperature_morning_areamax),
        anomaly_d1_temperature_morning_areamean(d1_temperature_morning_areamean),

        anomaly_d1_temperature_afternoon_inlandmin(d1_temperature_afternoon_inlandmin),
        anomaly_d1_temperature_afternoon_inlandmax(d1_temperature_afternoon_inlandmax),
        anomaly_d1_temperature_afternoon_inlandmean(d1_temperature_afternoon_inlandmean),
        anomaly_d1_temperature_afternoon_coastmin(d1_temperature_afternoon_coastmin),
        anomaly_d1_temperature_afternoon_coastmax(d1_temperature_afternoon_coastmax),
        anomaly_d1_temperature_afternoon_coastmean(d1_temperature_afternoon_coastmean),
        anomaly_d1_temperature_afternoon_areamin(d1_temperature_afternoon_areamin),
        anomaly_d1_temperature_afternoon_areamax(d1_temperature_afternoon_areamax),
        anomaly_d1_temperature_afternoon_areamean(d1_temperature_afternoon_areamean),

        anomaly_d1_windspeed_morning_inlandmin(d1_windspeed_morning_inlandmin),
        anomaly_d1_windspeed_morning_inlandmax(d1_windspeed_morning_inlandmax),
        anomaly_d1_windspeed_morning_inlandmean(d1_windspeed_morning_inlandmean),
        anomaly_d1_windspeed_morning_coastmin(d1_windspeed_morning_coastmin),
        anomaly_d1_windspeed_morning_coastmax(d1_windspeed_morning_coastmax),
        anomaly_d1_windspeed_morning_coastmean(d1_windspeed_morning_coastmean),

        anomaly_d1_windspeed_afternoon_inlandmin(d1_windspeed_afternoon_inlandmin),
        anomaly_d1_windspeed_afternoon_inlandmax(d1_windspeed_afternoon_inlandmax),
        anomaly_d1_windspeed_afternoon_inlandmean(d1_windspeed_afternoon_inlandmean),
        anomaly_d1_windspeed_afternoon_coastmin(d1_windspeed_afternoon_coastmin),
        anomaly_d1_windspeed_afternoon_coastmax(d1_windspeed_afternoon_coastmax),
        anomaly_d1_windspeed_afternoon_coastmean(d1_windspeed_afternoon_coastmean),

        anomaly_d1_windchill_morning_inlandmin(d1_windchill_morning_inlandmin),
        anomaly_d1_windchill_morning_inlandmax(d1_windchill_morning_inlandmax),
        anomaly_d1_windchill_morning_inlandmean(d1_windchill_morning_inlandmean),
        anomaly_d1_windchill_morning_coastmin(d1_windchill_morning_coastmin),
        anomaly_d1_windchill_morning_coastmax(d1_windchill_morning_coastmax),
        anomaly_d1_windchill_morning_coastmean(d1_windchill_morning_coastmean),

        anomaly_d1_windchill_afternoon_inlandmin(d1_windchill_afternoon_inlandmin),
        anomaly_d1_windchill_afternoon_inlandmax(d1_windchill_afternoon_inlandmax),
        anomaly_d1_windchill_afternoon_inlandmean(d1_windchill_afternoon_inlandmean),
        anomaly_d1_windchill_afternoon_coastmin(d1_windchill_afternoon_coastmin),
        anomaly_d1_windchill_afternoon_coastmax(d1_windchill_afternoon_coastmax),
        anomaly_d1_windchill_afternoon_coastmean(d1_windchill_afternoon_coastmean),

        anomaly_nite_temperature_min(nite_temperature_min),
        anomaly_nite_temperature_max(nite_temperature_max),
        anomaly_nite_temperature_mean(nite_temperature_mean),

        anomaly_d2_temperature_inlandmin(d2_temperature_inlandmin),
        anomaly_d2_temperature_inlandmax(d2_temperature_inlandmax),
        anomaly_d2_temperature_inlandmean(d2_temperature_inlandmean),
        anomaly_d2_temperature_coastmin(d2_temperature_coastmin),
        anomaly_d2_temperature_coastmax(d2_temperature_coastmax),
        anomaly_d2_temperature_coastmean(d2_temperature_coastmean),
        anomaly_d2_temperature_areamin(d2_temperature_areamin),
        anomaly_d2_temperature_areamax(d2_temperature_areamax),
        anomaly_d2_temperature_areamean(d2_temperature_areamean),

        anomaly_d2_temperature_morning_inlandmin(d2_temperature_morning_inlandmin),
        anomaly_d2_temperature_morning_inlandmax(d2_temperature_morning_inlandmax),
        anomaly_d2_temperature_morning_inlandmean(d2_temperature_morning_inlandmean),
        anomaly_d2_temperature_morning_coastmin(d2_temperature_morning_coastmin),
        anomaly_d2_temperature_morning_coastmax(d2_temperature_morning_coastmax),
        anomaly_d2_temperature_morning_coastmean(d2_temperature_morning_coastmean),
        anomaly_d2_temperature_morning_areamin(d2_temperature_morning_areamin),
        anomaly_d2_temperature_morning_areamax(d2_temperature_morning_areamax),
        anomaly_d2_temperature_morning_areamean(d2_temperature_morning_areamean),

        anomaly_d2_temperature_afternoon_inlandmin(d2_temperature_afternoon_inlandmin),
        anomaly_d2_temperature_afternoon_inlandmax(d2_temperature_afternoon_inlandmax),
        anomaly_d2_temperature_afternoon_inlandmean(d2_temperature_afternoon_inlandmean),
        anomaly_d2_temperature_afternoon_coastmin(d2_temperature_afternoon_coastmin),
        anomaly_d2_temperature_afternoon_coastmax(d2_temperature_afternoon_coastmax),
        anomaly_d2_temperature_afternoon_coastmean(d2_temperature_afternoon_coastmean),
        anomaly_d2_temperature_afternoon_areamin(d2_temperature_afternoon_areamin),
        anomaly_d2_temperature_afternoon_areamax(d2_temperature_afternoon_areamax),
        anomaly_d2_temperature_afternoon_areamean(d2_temperature_afternoon_areamean),

        anomaly_d2_windspeed_morning_inlandmin(d2_windspeed_morning_inlandmin),
        anomaly_d2_windspeed_morning_inlandmax(d2_windspeed_morning_inlandmax),
        anomaly_d2_windspeed_morning_inlandmean(d2_windspeed_morning_inlandmean),
        anomaly_d2_windspeed_morning_coastmin(d2_windspeed_morning_coastmin),
        anomaly_d2_windspeed_morning_coastmax(d2_windspeed_morning_coastmax),
        anomaly_d2_windspeed_morning_coastmean(d2_windspeed_morning_coastmean),

        anomaly_d2_windspeed_afternoon_inlandmin(d2_windspeed_afternoon_inlandmin),
        anomaly_d2_windspeed_afternoon_inlandmax(d2_windspeed_afternoon_inlandmax),
        anomaly_d2_windspeed_afternoon_inlandmean(d2_windspeed_afternoon_inlandmean),
        anomaly_d2_windspeed_afternoon_coastmin(d2_windspeed_afternoon_coastmin),
        anomaly_d2_windspeed_afternoon_coastmax(d2_windspeed_afternoon_coastmax),
        anomaly_d2_windspeed_afternoon_coastmean(d2_windspeed_afternoon_coastmean),

        anomaly_d2_windchill_morning_inlandmin(d2_windchill_morning_inlandmin),
        anomaly_d2_windchill_morning_inlandmax(d2_windchill_morning_inlandmax),
        anomaly_d2_windchill_morning_inlandmean(d2_windchill_morning_inlandmean),
        anomaly_d2_windchill_morning_coastmin(d2_windchill_morning_coastmin),
        anomaly_d2_windchill_morning_coastmax(d2_windchill_morning_coastmax),
        anomaly_d2_windchill_morning_coastmean(d2_windchill_morning_coastmean),

        anomaly_d2_windchill_afternoon_inlandmin(d2_windchill_afternoon_inlandmin),
        anomaly_d2_windchill_afternoon_inlandmax(d2_windchill_afternoon_inlandmax),
        anomaly_d2_windchill_afternoon_inlandmean(d2_windchill_afternoon_inlandmean),
        anomaly_d2_windchill_afternoon_coastmin(d2_windchill_afternoon_coastmin),
        anomaly_d2_windchill_afternoon_coastmax(d2_windchill_afternoon_coastmax),
        anomaly_d2_windchill_afternoon_coastmean(d2_windchill_afternoon_coastmean),
        anomaly_story(story)

  {
  }

  string anomaly_d1_temperature_inlandmin;
  string anomaly_d1_temperature_inlandmax;
  string anomaly_d1_temperature_inlandmean;
  string anomaly_d1_temperature_coastmin;
  string anomaly_d1_temperature_coastmax;
  string anomaly_d1_temperature_coastmean;
  string anomaly_d1_temperature_areamin;
  string anomaly_d1_temperature_areamax;
  string anomaly_d1_temperature_areamean;

  string anomaly_d1_temperature_morning_inlandmin;
  string anomaly_d1_temperature_morning_inlandmax;
  string anomaly_d1_temperature_morning_inlandmean;
  string anomaly_d1_temperature_morning_coastmin;
  string anomaly_d1_temperature_morning_coastmax;
  string anomaly_d1_temperature_morning_coastmean;
  string anomaly_d1_temperature_morning_areamin;
  string anomaly_d1_temperature_morning_areamax;
  string anomaly_d1_temperature_morning_areamean;

  string anomaly_d1_temperature_afternoon_inlandmin;
  string anomaly_d1_temperature_afternoon_inlandmax;
  string anomaly_d1_temperature_afternoon_inlandmean;
  string anomaly_d1_temperature_afternoon_coastmin;
  string anomaly_d1_temperature_afternoon_coastmax;
  string anomaly_d1_temperature_afternoon_coastmean;
  string anomaly_d1_temperature_afternoon_areamin;
  string anomaly_d1_temperature_afternoon_areamax;
  string anomaly_d1_temperature_afternoon_areamean anomaly_d1_windspeed_morning_inlandmin;
  string anomaly_d1_windspeed_morning_inlandmax;
  string anomaly_d1_windspeed_morning_inlandmean;
  string anomaly_d1_windspeed_morning_coastmin;
  string anomaly_d1_windspeed_morning_coastmax;
  string anomaly_d1_windspeed_morning_coastmean;

  string anomaly_d1_windspeed_afternoon_inlandmin;
  string anomaly_d1_windspeed_afternoon_inlandmax;
  string anomaly_d1_windspeed_afternoon_inlandmean;
  string anomaly_d1_windspeed_afternoon_coastmin;
  string anomaly_d1_windspeed_afternoon_coastmax;
  string anomaly_d1_windspeed_afternoon_coastmean;

  string anomaly_d1_windchill_morning_inlandmin;
  string anomaly_d1_windchill_morning_inlandmax;
  string anomaly_d1_windchill_morning_inlandmean;
  string anomaly_d1_windchill_morning_coastmin;
  string anomaly_d1_windchill_morning_coastmax;
  string anomaly_d1_windchill_morning_coastmean;

  string anomaly_d1_windchill_afternoon_inlandmin;
  string anomaly_d1_windchill_afternoon_inlandmax;
  string anomaly_d1_windchill_afternoon_inlandmean;
  string anomaly_d1_windchill_afternoon_coastmin;
  string anomaly_d1_windchill_afternoon_coastmax;
  string anomaly_d1_windchill_afternoon_coastmean;

  string anomaly_nite_temperature_min;
  string anomaly_nite_temperature_max;
  string anomaly_nite_temperature_mean;

  string anomaly_d2_temperature_inlandmin;
  string anomaly_d2_temperature_inlandmax;
  string anomaly_d2_temperature_inlandmean;
  string anomaly_d2_temperature_coastmin;
  string anomaly_d2_temperature_coastmax;
  string anomaly_d2_temperature_coastmean;
  string anomaly_d2_temperature_areamin;
  string anomaly_d2_temperature_areamax;
  string anomaly_d2_temperature_areamean;

  string anomaly_d2_temperature_morning_inlandmin;
  string anomaly_d2_temperature_morning_inlandmax;
  string anomaly_d2_temperature_morning_inlandmean;
  string anomaly_d2_temperature_morning_coastmin;
  string anomaly_d2_temperature_morning_coastmax;
  string anomaly_d2_temperature_morning_coastmean;
  string anomaly_d2_temperature_morning_areamin;
  string anomaly_d2_temperature_morning_areamax;
  string anomaly_d2_temperature_morning_areamean;

  string anomaly_d2_temperature_afternoon_inlandmin;
  string anomaly_d2_temperature_afternoon_inlandmax;
  string anomaly_d2_temperature_afternoon_inlandmean;
  string anomaly_d2_temperature_afternoon_coastmin;
  string anomaly_d2_temperature_afternoon_coastmax;
  string anomaly_d2_temperature_afternoon_coastmean;
  string anomaly_d2_temperature_afternoon_areamin;
  string anomaly_d2_temperature_afternoon_areamax;
  string anomaly_d2_temperature_afternoon_areamean;

  string anomaly_d2_windspeed_morning_inlandmin;
  string anomaly_d2_windspeed_morning_inlandmax;
  string anomaly_d2_windspeed_morning_inlandmean;
  string anomaly_d2_windspeed_morning_coastmin;
  string anomaly_d2_windspeed_morning_coastmax;
  string anomaly_d2_windspeed_morning_coastmean;

  string anomaly_d2_windspeed_afternoon_inlandmin;
  string anomaly_d2_windspeed_afternoon_inlandmax;
  string anomaly_d2_windspeed_afternoon_inlandmean;
  string anomaly_d2_windspeed_afternoon_coastmin;
  string anomaly_d2_windspeed_afternoon_coastmax;
  string anomaly_d2_windspeed_afternoon_coastmean;

  string anomaly_d2_windchill_morning_inlandmin;
  string anomaly_d2_windchill_morning_inlandmax;
  string anomaly_d2_windchill_morning_inlandmean;
  string anomaly_d2_windchill_morning_coastmin;
  string anomaly_d2_windchill_morning_coastmax;
  string anomaly_d2_windchill_morning_coastmean;

  string anomaly_d2_windchill_afternoon_inlandmin;
  string anomaly_d2_windchill_afternoon_inlandmax;
  string anomaly_d2_windchill_afternoon_inlandmean;
  string anomaly_d2_windchill_afternoon_coastmin;
  string anomaly_d2_windchill_afternoon_coastmax;
  string anomaly_d2_windchill_afternoon_coastmean;
  string anomaly_story;
};

typedef std::map<int, AnomalyTestParam> AnomalyTestCases;
#endif

struct TemperatureStoryTestParam
{
  TemperatureStoryTestParam(const char* d1_afternoon_inlandmin,
                            const char* d1_afternoon_inlandmax,
                            const char* d1_afternoon_inlandmean,
                            const char* d1_afternoon_coastmin,
                            const char* d1_afternoon_coastmax,
                            const char* d1_afternoon_coastmean,
                            const char* d1_afternoon_areamin,
                            const char* d1_afternoon_areamax,
                            const char* d1_afternoon_areamean,
                            const char* d2_afternoon_inlandmin,
                            const char* d2_afternoon_inlandmax,
                            const char* d2_afternoon_inlandmean,
                            const char* d2_afternoon_coastmin,
                            const char* d2_afternoon_coastmax,
                            const char* d2_afternoon_coastmean,
                            const char* d2_afternoon_areamin,
                            const char* d2_afternoon_areamax,
                            const char* d2_afternoon_areamean,
                            const char* story)
      : temperature_d1_afternoon_inlandmin(d1_afternoon_inlandmin),
        temperature_d1_afternoon_inlandmax(d1_afternoon_inlandmax),
        temperature_d1_afternoon_inlandmean(d1_afternoon_inlandmean),
        temperature_d1_afternoon_coastmin(d1_afternoon_coastmin),
        temperature_d1_afternoon_coastmax(d1_afternoon_coastmax),
        temperature_d1_afternoon_coastmean(d1_afternoon_coastmean),
        temperature_d1_afternoon_areamin(d1_afternoon_areamin),
        temperature_d1_afternoon_areamax(d1_afternoon_areamax),
        temperature_d1_afternoon_areamean(d1_afternoon_areamean),

        temperature_d2_afternoon_inlandmin(d2_afternoon_inlandmin),
        temperature_d2_afternoon_inlandmax(d2_afternoon_inlandmax),
        temperature_d2_afternoon_inlandmean(d2_afternoon_inlandmean),
        temperature_d2_afternoon_coastmin(d2_afternoon_coastmin),
        temperature_d2_afternoon_coastmax(d2_afternoon_coastmax),
        temperature_d2_afternoon_coastmean(d2_afternoon_coastmean),
        temperature_d2_afternoon_areamin(d2_afternoon_areamin),
        temperature_d2_afternoon_areamax(d2_afternoon_areamax),
        temperature_d2_afternoon_areamean(d2_afternoon_areamean),

        theStory(story)
  {
  }

  string temperature_d1_afternoon_inlandmin;
  string temperature_d1_afternoon_inlandmax;
  string temperature_d1_afternoon_inlandmean;
  string temperature_d1_afternoon_coastmin;
  string temperature_d1_afternoon_coastmax;
  string temperature_d1_afternoon_coastmean;
  string temperature_d1_afternoon_areamin;
  string temperature_d1_afternoon_areamax;
  string temperature_d1_afternoon_areamean;

  string temperature_d2_afternoon_inlandmin;
  string temperature_d2_afternoon_inlandmax;
  string temperature_d2_afternoon_inlandmean;
  string temperature_d2_afternoon_coastmin;
  string temperature_d2_afternoon_coastmax;
  string temperature_d2_afternoon_coastmean;
  string temperature_d2_afternoon_areamin;
  string temperature_d2_afternoon_areamax;
  string temperature_d2_afternoon_areamean;

  string theStory;
};

struct Max36HoursTestParam : public TemperatureStoryTestParam
{
  Max36HoursTestParam(const char* d1_inlandmin,
                      const char* d1_inlandmax,
                      const char* d1_inlandmean,
                      const char* d1_coastmin,
                      const char* d1_coastmax,
                      const char* d1_coastmean,
                      const char* d1_areamin,
                      const char* d1_areamax,
                      const char* d1_areamean,

                      const char* d1_morning_inlandmin,
                      const char* d1_morning_inlandmax,
                      const char* d1_morning_inlandmean,
                      const char* d1_morning_coastmin,
                      const char* d1_morning_coastmax,
                      const char* d1_morning_coastmean,
                      const char* d1_morning_areamin,
                      const char* d1_morning_areamax,
                      const char* d1_morning_areamean,

                      const char* d1_afternoon_inlandmin,
                      const char* d1_afternoon_inlandmax,
                      const char* d1_afternoon_inlandmean,
                      const char* d1_afternoon_coastmin,
                      const char* d1_afternoon_coastmax,
                      const char* d1_afternoon_coastmean,
                      const char* d1_afternoon_areamin,
                      const char* d1_afternoon_areamax,
                      const char* d1_afternoon_areamean,

                      const char* nite_inlandmin,
                      const char* nite_inlandmax,
                      const char* nite_inlandmean,
                      const char* nite_coastmin,
                      const char* nite_coastmax,
                      const char* nite_coastmean,
                      const char* nite_areamin,
                      const char* nite_areamax,
                      const char* nite_areamean,

                      const char* d2_inlandmin,
                      const char* d2_inlandmax,
                      const char* d2_inlandmean,
                      const char* d2_coastmin,
                      const char* d2_coastmax,
                      const char* d2_coastmean,
                      const char* d2_areamin,
                      const char* d2_areamax,
                      const char* d2_areamean,

                      const char* d2_morning_inlandmin,
                      const char* d2_morning_inlandmax,
                      const char* d2_morning_inlandmean,
                      const char* d2_morning_coastmin,
                      const char* d2_morning_coastmax,
                      const char* d2_morning_coastmean,
                      const char* d2_morning_areamin,
                      const char* d2_morning_areamax,
                      const char* d2_morning_areamean,

                      const char* d2_afternoon_inlandmin,
                      const char* d2_afternoon_inlandmax,
                      const char* d2_afternoon_inlandmean,
                      const char* d2_afternoon_coastmin,
                      const char* d2_afternoon_coastmax,
                      const char* d2_afternoon_coastmean,
                      const char* d2_afternoon_areamin,
                      const char* d2_afternoon_areamax,
                      const char* d2_afternoon_areamean,
                      const char* story)
      : TemperatureStoryTestParam(d1_afternoon_inlandmin,
                                  d1_afternoon_inlandmax,
                                  d1_afternoon_inlandmean,
                                  d1_afternoon_coastmin,
                                  d1_afternoon_coastmax,
                                  d1_afternoon_coastmean,
                                  d1_afternoon_areamin,
                                  d1_afternoon_areamax,
                                  d1_afternoon_areamean,
                                  d2_afternoon_inlandmin,
                                  d2_afternoon_inlandmax,
                                  d2_afternoon_inlandmean,
                                  d2_afternoon_coastmin,
                                  d2_afternoon_coastmax,
                                  d2_afternoon_coastmean,
                                  d2_afternoon_areamin,
                                  d2_afternoon_areamax,
                                  d2_afternoon_areamean,
                                  story),
        temperature_d1_inlandmin(d1_inlandmin),
        temperature_d1_inlandmax(d1_inlandmax),
        temperature_d1_inlandmean(d1_inlandmean),
        temperature_d1_coastmin(d1_coastmin),
        temperature_d1_coastmax(d1_coastmax),
        temperature_d1_coastmean(d1_coastmean),
        temperature_d1_areamin(d1_areamin),
        temperature_d1_areamax(d1_areamax),
        temperature_d1_areamean(d1_areamean),

        temperature_d1_morning_inlandmin(d1_morning_inlandmin),
        temperature_d1_morning_inlandmax(d1_morning_inlandmax),
        temperature_d1_morning_inlandmean(d1_morning_inlandmean),
        temperature_d1_morning_coastmin(d1_morning_coastmin),
        temperature_d1_morning_coastmax(d1_morning_coastmax),
        temperature_d1_morning_coastmean(d1_morning_coastmean),
        temperature_d1_morning_areamin(d1_morning_areamin),
        temperature_d1_morning_areamax(d1_morning_areamax),
        temperature_d1_morning_areamean(d1_morning_areamean),

        temperature_nite_inlandmin(nite_inlandmin),
        temperature_nite_inlandmax(nite_inlandmax),
        temperature_nite_inlandmean(nite_inlandmean),
        temperature_nite_coastmin(nite_coastmin),
        temperature_nite_coastmax(nite_coastmax),
        temperature_nite_coastmean(nite_coastmean),
        temperature_nite_areamin(nite_areamin),
        temperature_nite_areamax(nite_areamax),
        temperature_nite_areamean(nite_areamean),

        temperature_d2_inlandmin(d2_inlandmin),
        temperature_d2_inlandmax(d2_inlandmax),
        temperature_d2_inlandmean(d2_inlandmean),
        temperature_d2_coastmin(d2_coastmin),
        temperature_d2_coastmax(d2_coastmax),
        temperature_d2_coastmean(d2_coastmean),
        temperature_d2_areamin(d2_areamin),
        temperature_d2_areamax(d2_areamax),
        temperature_d2_areamean(d2_areamean),

        temperature_d2_morning_inlandmin(d2_morning_inlandmin),
        temperature_d2_morning_inlandmax(d2_morning_inlandmax),
        temperature_d2_morning_inlandmean(d2_morning_inlandmean),
        temperature_d2_morning_coastmin(d2_morning_coastmin),
        temperature_d2_morning_coastmax(d2_morning_coastmax),
        temperature_d2_morning_coastmean(d2_morning_coastmean),
        temperature_d2_morning_areamin(d2_morning_areamin),
        temperature_d2_morning_areamax(d2_morning_areamax),
        temperature_d2_morning_areamean(d2_morning_areamean)

  {
  }

  string temperature_d1_inlandmin;
  string temperature_d1_inlandmax;
  string temperature_d1_inlandmean;
  string temperature_d1_coastmin;
  string temperature_d1_coastmax;
  string temperature_d1_coastmean;
  string temperature_d1_areamin;
  string temperature_d1_areamax;
  string temperature_d1_areamean;

  string temperature_d1_morning_inlandmin;
  string temperature_d1_morning_inlandmax;
  string temperature_d1_morning_inlandmean;
  string temperature_d1_morning_coastmin;
  string temperature_d1_morning_coastmax;
  string temperature_d1_morning_coastmean;
  string temperature_d1_morning_areamin;
  string temperature_d1_morning_areamax;
  string temperature_d1_morning_areamean;

  string temperature_nite_inlandmin;
  string temperature_nite_inlandmax;
  string temperature_nite_inlandmean;
  string temperature_nite_coastmin;
  string temperature_nite_coastmax;
  string temperature_nite_coastmean;
  string temperature_nite_areamin;
  string temperature_nite_areamax;
  string temperature_nite_areamean;

  string temperature_d2_inlandmin;
  string temperature_d2_inlandmax;
  string temperature_d2_inlandmean;
  string temperature_d2_coastmin;
  string temperature_d2_coastmax;
  string temperature_d2_coastmean;
  string temperature_d2_areamin;
  string temperature_d2_areamax;
  string temperature_d2_areamean;

  string temperature_d2_morning_inlandmin;
  string temperature_d2_morning_inlandmax;
  string temperature_d2_morning_inlandmean;
  string temperature_d2_morning_coastmin;
  string temperature_d2_morning_coastmax;
  string temperature_d2_morning_coastmean;
  string temperature_d2_morning_areamin;
  string temperature_d2_morning_areamax;
  string temperature_d2_morning_areamean;
};

struct TemperatureAnomalyTestParam : public TemperatureStoryTestParam
{
  TemperatureAnomalyTestParam(const char* d1_temperature_afternoon_inlandmin,
                              const char* d1_temperature_afternoon_inlandmax,
                              const char* d1_temperature_afternoon_inlandmean,
                              const char* d1_temperature_afternoon_coastmin,
                              const char* d1_temperature_afternoon_coastmax,
                              const char* d1_temperature_afternoon_coastmean,
                              const char* d1_temperature_afternoon_areamin,
                              const char* d1_temperature_afternoon_areamax,
                              const char* d1_temperature_afternoon_areamean,

                              const char* d2_temperature_afternoon_inlandmin,
                              const char* d2_temperature_afternoon_inlandmax,
                              const char* d2_temperature_afternoon_inlandmean,
                              const char* d2_temperature_afternoon_coastmin,
                              const char* d2_temperature_afternoon_coastmax,
                              const char* d2_temperature_afternoon_coastmean,
                              const char* d2_temperature_afternoon_areamin,
                              const char* d2_temperature_afternoon_areamax,
                              const char* d2_temperature_afternoon_areamean,

                              const char* story)
      :

        TemperatureStoryTestParam(d1_temperature_afternoon_inlandmin,
                                  d1_temperature_afternoon_inlandmax,
                                  d1_temperature_afternoon_inlandmean,
                                  d1_temperature_afternoon_coastmin,
                                  d1_temperature_afternoon_coastmax,
                                  d1_temperature_afternoon_coastmean,
                                  d1_temperature_afternoon_areamin,
                                  d1_temperature_afternoon_areamax,
                                  d1_temperature_afternoon_areamean,

                                  d2_temperature_afternoon_inlandmin,
                                  d2_temperature_afternoon_inlandmax,
                                  d2_temperature_afternoon_inlandmean,
                                  d2_temperature_afternoon_coastmin,
                                  d2_temperature_afternoon_coastmax,
                                  d2_temperature_afternoon_coastmean,
                                  d2_temperature_afternoon_areamin,
                                  d2_temperature_afternoon_areamax,
                                  d2_temperature_afternoon_areamean,
                                  story)
  {
  }
};

struct WindAnomalyTestParam : public TemperatureStoryTestParam
{
  WindAnomalyTestParam(const char* d2_temperature_morning_inlandmin,
                       const char* d2_temperature_morning_inlandmax,
                       const char* d2_temperature_morning_inlandmean,
                       const char* d2_temperature_morning_coastmin,
                       const char* d2_temperature_morning_coastmax,
                       const char* d2_temperature_morning_coastmean,
                       const char* d2_temperature_morning_areamin,
                       const char* d2_temperature_morning_areamax,
                       const char* d2_temperature_morning_areamean,

                       const char* d2_temperature_afternoon_inlandmin,
                       const char* d2_temperature_afternoon_inlandmax,
                       const char* d2_temperature_afternoon_inlandmean,
                       const char* d2_temperature_afternoon_coastmin,
                       const char* d2_temperature_afternoon_coastmax,
                       const char* d2_temperature_afternoon_coastmean,
                       const char* d2_temperature_afternoon_areamin,
                       const char* d2_temperature_afternoon_areamax,
                       const char* d2_temperature_afternoon_areamean,

                       const char* d2_windspeed_morning_inlandmin,
                       const char* d2_windspeed_morning_inlandmax,
                       const char* d2_windspeed_morning_inlandmean,
                       const char* d2_windspeed_morning_coastmin,
                       const char* d2_windspeed_morning_coastmax,
                       const char* d2_windspeed_morning_coastmean,

                       const char* d2_windspeed_afternoon_inlandmin,
                       const char* d2_windspeed_afternoon_inlandmax,
                       const char* d2_windspeed_afternoon_inlandmean,
                       const char* d2_windspeed_afternoon_coastmin,
                       const char* d2_windspeed_afternoon_coastmax,
                       const char* d2_windspeed_afternoon_coastmean,

                       const char* d2_windchill_morning_inlandmin,
                       const char* d2_windchill_morning_inlandmax,
                       const char* d2_windchill_morning_inlandmean,
                       const char* d2_windchill_morning_coastmin,
                       const char* d2_windchill_morning_coastmax,
                       const char* d2_windchill_morning_coastmean,

                       const char* d2_windchill_afternoon_inlandmin,
                       const char* d2_windchill_afternoon_inlandmax,
                       const char* d2_windchill_afternoon_inlandmean,
                       const char* d2_windchill_afternoon_coastmin,
                       const char* d2_windchill_afternoon_coastmax,
                       const char* d2_windchill_afternoon_coastmean,

                       const char* story)
      :

        TemperatureStoryTestParam("",
                                  "",
                                  "",
                                  "",
                                  "",
                                  "",
                                  "",
                                  "",
                                  "",

                                  d2_temperature_afternoon_inlandmin,
                                  d2_temperature_afternoon_inlandmax,
                                  d2_temperature_afternoon_inlandmean,
                                  d2_temperature_afternoon_coastmin,
                                  d2_temperature_afternoon_coastmax,
                                  d2_temperature_afternoon_coastmean,
                                  d2_temperature_afternoon_areamin,
                                  d2_temperature_afternoon_areamax,
                                  d2_temperature_afternoon_areamean,
                                  story),
        temperature_d2_morning_inlandmin(d2_temperature_morning_inlandmin),
        temperature_d2_morning_inlandmax(d2_temperature_morning_inlandmax),
        temperature_d2_morning_inlandmean(d2_temperature_morning_inlandmean),
        temperature_d2_morning_coastmin(d2_temperature_morning_coastmin),
        temperature_d2_morning_coastmax(d2_temperature_morning_coastmax),
        temperature_d2_morning_coastmean(d2_temperature_morning_coastmean),
        temperature_d2_morning_areamin(d2_temperature_morning_areamin),
        temperature_d2_morning_areamax(d2_temperature_morning_areamax),
        temperature_d2_morning_areamean(d2_temperature_morning_areamean),

        anomaly_d2_windspeed_morning_inlandmin(d2_windspeed_morning_inlandmin),
        anomaly_d2_windspeed_morning_inlandmax(d2_windspeed_morning_inlandmax),
        anomaly_d2_windspeed_morning_inlandmean(d2_windspeed_morning_inlandmean),
        anomaly_d2_windspeed_morning_coastmin(d2_windspeed_morning_coastmin),
        anomaly_d2_windspeed_morning_coastmax(d2_windspeed_morning_coastmax),
        anomaly_d2_windspeed_morning_coastmean(d2_windspeed_morning_coastmean),

        anomaly_d2_windspeed_afternoon_inlandmin(d2_windspeed_afternoon_inlandmin),
        anomaly_d2_windspeed_afternoon_inlandmax(d2_windspeed_afternoon_inlandmax),
        anomaly_d2_windspeed_afternoon_inlandmean(d2_windspeed_afternoon_inlandmean),
        anomaly_d2_windspeed_afternoon_coastmin(d2_windspeed_afternoon_coastmin),
        anomaly_d2_windspeed_afternoon_coastmax(d2_windspeed_afternoon_coastmax),
        anomaly_d2_windspeed_afternoon_coastmean(d2_windspeed_afternoon_coastmean),

        anomaly_d2_windchill_morning_inlandmin(d2_windchill_morning_inlandmin),
        anomaly_d2_windchill_morning_inlandmax(d2_windchill_morning_inlandmax),
        anomaly_d2_windchill_morning_inlandmean(d2_windchill_morning_inlandmean),
        anomaly_d2_windchill_morning_coastmin(d2_windchill_morning_coastmin),
        anomaly_d2_windchill_morning_coastmax(d2_windchill_morning_coastmax),
        anomaly_d2_windchill_morning_coastmean(d2_windchill_morning_coastmean),

        anomaly_d2_windchill_afternoon_inlandmin(d2_windchill_afternoon_inlandmin),
        anomaly_d2_windchill_afternoon_inlandmax(d2_windchill_afternoon_inlandmax),
        anomaly_d2_windchill_afternoon_inlandmean(d2_windchill_afternoon_inlandmean),
        anomaly_d2_windchill_afternoon_coastmin(d2_windchill_afternoon_coastmin),
        anomaly_d2_windchill_afternoon_coastmax(d2_windchill_afternoon_coastmax),
        anomaly_d2_windchill_afternoon_coastmean(d2_windchill_afternoon_coastmean)
  {
  }

  string temperature_d2_morning_inlandmin;
  string temperature_d2_morning_inlandmax;
  string temperature_d2_morning_inlandmean;
  string temperature_d2_morning_coastmin;
  string temperature_d2_morning_coastmax;
  string temperature_d2_morning_coastmean;
  string temperature_d2_morning_areamin;
  string temperature_d2_morning_areamax;
  string temperature_d2_morning_areamean;

  string anomaly_d2_windspeed_morning_inlandmin;
  string anomaly_d2_windspeed_morning_inlandmax;
  string anomaly_d2_windspeed_morning_inlandmean;
  string anomaly_d2_windspeed_morning_coastmin;
  string anomaly_d2_windspeed_morning_coastmax;
  string anomaly_d2_windspeed_morning_coastmean;

  string anomaly_d2_windspeed_afternoon_inlandmin;
  string anomaly_d2_windspeed_afternoon_inlandmax;
  string anomaly_d2_windspeed_afternoon_inlandmean;
  string anomaly_d2_windspeed_afternoon_coastmin;
  string anomaly_d2_windspeed_afternoon_coastmax;
  string anomaly_d2_windspeed_afternoon_coastmean;

  string anomaly_d2_windchill_morning_inlandmin;
  string anomaly_d2_windchill_morning_inlandmax;
  string anomaly_d2_windchill_morning_inlandmean;
  string anomaly_d2_windchill_morning_coastmin;
  string anomaly_d2_windchill_morning_coastmax;
  string anomaly_d2_windchill_morning_coastmean;

  string anomaly_d2_windchill_afternoon_inlandmin;
  string anomaly_d2_windchill_afternoon_inlandmax;
  string anomaly_d2_windchill_afternoon_inlandmean;
  string anomaly_d2_windchill_afternoon_coastmin;
  string anomaly_d2_windchill_afternoon_coastmax;
  string anomaly_d2_windchill_afternoon_coastmean;
};

typedef std::map<int, TemperatureStoryTestParam*> TestCaseContainer;

double random_d() { return ((double)rand() / ((double)(RAND_MAX) + (double)(1))); }
int random_i(unsigned int scale) { return rand() % scale; }
int random_i(unsigned int min_scale, unsigned int max_scale)
{
  return rand() % (max_scale - min_scale) + min_scale;
}

std::ostream& operator<<(std::ostream& theOutput, const Max36HoursTestParam& theParam)
{
  theOutput << "***** settings *******" << endl;
  theOutput << " Area minimum d1: " << theParam.temperature_d1_areamin << endl;
  theOutput << " Area maximum d1: " << theParam.temperature_d1_areamax << endl;
  theOutput << " Area mean d1: " << theParam.temperature_d1_areamean << endl;
  theOutput << " Area morning minimum d1: " << theParam.temperature_d1_morning_areamin << endl;
  theOutput << " Area morning maximum d1: " << theParam.temperature_d1_morning_areamax << endl;
  theOutput << " Area morning mean d1: " << theParam.temperature_d1_morning_areamean << endl;
  theOutput << " Area afternoon minimum d1: " << theParam.temperature_d1_afternoon_areamin << endl;
  theOutput << " Area afternoon maximum d1: " << theParam.temperature_d1_afternoon_areamax << endl;
  theOutput << " Area afternoon mean d1: " << theParam.temperature_d1_afternoon_areamean << endl;

  theOutput << " Inland minimum d1: " << theParam.temperature_d1_inlandmin << endl;
  theOutput << " Inland maximum d1: " << theParam.temperature_d1_inlandmax << endl;
  theOutput << " Inland mean d1: " << theParam.temperature_d1_inlandmean << endl;
  theOutput << " Inland morning minimum d1: " << theParam.temperature_d1_morning_inlandmin << endl;
  theOutput << " Inland morning maximum d1: " << theParam.temperature_d1_morning_inlandmax << endl;
  theOutput << " Inland morning mean d1: " << theParam.temperature_d1_morning_inlandmean << endl;
  theOutput << " Inland afternoon minimum d1: " << theParam.temperature_d1_afternoon_inlandmin
            << endl;
  theOutput << " Inland afternoon maximum d1: " << theParam.temperature_d1_afternoon_inlandmax
            << endl;
  theOutput << " Inland afternoon mean d1: " << theParam.temperature_d1_afternoon_inlandmean
            << endl;

  theOutput << " Coast minimum d1: " << theParam.temperature_d1_coastmin << endl;
  theOutput << " Coast maximum d1: " << theParam.temperature_d1_coastmax << endl;
  theOutput << " Coast mean d1: " << theParam.temperature_d1_coastmean << endl;
  theOutput << " Coast morning minimum d1: " << theParam.temperature_d1_morning_coastmin << endl;
  theOutput << " Coast morning maximum d1: " << theParam.temperature_d1_morning_coastmax << endl;
  theOutput << " Coast morning mean d1: " << theParam.temperature_d1_morning_coastmean << endl;
  theOutput << " Coast afternoon minimum d1: " << theParam.temperature_d1_afternoon_coastmin
            << endl;
  theOutput << " Coast afternoon maximum d1: " << theParam.temperature_d1_afternoon_coastmax
            << endl;
  theOutput << " Coast afternoon mean d1: " << theParam.temperature_d1_afternoon_coastmean << endl;

  theOutput << " Area minimum night: " << theParam.temperature_nite_areamin << endl;
  theOutput << " Área maximum night: " << theParam.temperature_nite_areamax << endl;
  theOutput << " Area mean night: " << theParam.temperature_nite_areamean << endl;

  theOutput << " Inland minimum night: " << theParam.temperature_nite_inlandmin << endl;
  theOutput << " Inland maximum night: " << theParam.temperature_nite_inlandmax << endl;
  theOutput << " Inland mean night: " << theParam.temperature_nite_inlandmean << endl;

  theOutput << " Coast minimum night: " << theParam.temperature_nite_coastmin << endl;
  theOutput << " Coast maximum night: " << theParam.temperature_nite_coastmax << endl;
  theOutput << " Coast mean night: " << theParam.temperature_nite_coastmean << endl;

  theOutput << " Area minimum d2: " << theParam.temperature_d2_areamin << endl;
  theOutput << " Area maximum d2: " << theParam.temperature_d2_areamax << endl;
  theOutput << " Area mean d2: " << theParam.temperature_d2_areamean << endl;
  theOutput << " Area morning minimum d2: " << theParam.temperature_d2_morning_areamin << endl;
  theOutput << " Area morning maximum d2: " << theParam.temperature_d2_morning_areamax << endl;
  theOutput << " Area morning mean d2: " << theParam.temperature_d2_morning_areamean << endl;
  theOutput << " Area afternoon minimum d2: " << theParam.temperature_d2_afternoon_areamin << endl;
  theOutput << " Area afternoon maximum d2: " << theParam.temperature_d2_afternoon_areamax << endl;
  theOutput << " Area afternoon mean d2: " << theParam.temperature_d2_afternoon_areamean << endl;

  theOutput << " Inland minimum d2: " << theParam.temperature_d2_inlandmin << endl;
  theOutput << " Inland maximum d2: " << theParam.temperature_d2_inlandmax << endl;
  theOutput << " Inland mean d2: " << theParam.temperature_d2_inlandmean << endl;
  theOutput << " Inland morning minimum d2: " << theParam.temperature_d2_morning_inlandmin << endl;
  theOutput << " Inland morning maximum d2: " << theParam.temperature_d2_morning_inlandmax << endl;
  theOutput << " Inland morning mean d2: " << theParam.temperature_d2_morning_inlandmean << endl;
  theOutput << " Inland afternoon minimum d2: " << theParam.temperature_d2_afternoon_inlandmin
            << endl;
  theOutput << " Inland afternoon maximum d2: " << theParam.temperature_d2_afternoon_inlandmax
            << endl;
  theOutput << " Inland afternoon mean d2: " << theParam.temperature_d2_afternoon_inlandmean
            << endl;

  theOutput << " Coast minimum d2: " << theParam.temperature_d2_coastmin << endl;
  theOutput << " Coast maximum d2: " << theParam.temperature_d2_coastmax << endl;
  theOutput << " Coast mean d2: " << theParam.temperature_d2_coastmean << endl;
  theOutput << " Coast morning minimum d2: " << theParam.temperature_d2_morning_coastmin << endl;
  theOutput << " Coast morning maximum d2: " << theParam.temperature_d2_morning_coastmax << endl;
  theOutput << " Coast morning mean d2: " << theParam.temperature_d2_morning_coastmean << endl;
  theOutput << " Coast afternoon minimum d2: " << theParam.temperature_d2_afternoon_coastmin
            << endl;
  theOutput << " Coast afternoon maximum d2: " << theParam.temperature_d2_afternoon_coastmax
            << endl;
  theOutput << " Coast afternoon mean d2: " << theParam.temperature_d2_afternoon_coastmean << endl;

  return theOutput;
}
std::ostream& operator<<(std::ostream& theOutput, const TemperatureAnomalyTestParam& theParam)
{
  /*
    theOutput << "***** settings *******" << endl;
    theOutput << " Minimum temperature d1: " << theParam.anomaly_d1_temperature_min << endl;
    theOutput << " Maximum temperature d1: " << theParam.anomaly_d1_temperature_max << endl;
    theOutput << " Mean temperature d1: " << theParam.anomaly_d1_temperature_mean << endl;
    theOutput << " Minimum temperature night: " << theParam.anomaly_nite_temperature_min << endl;
    theOutput << " Maximum temperature night: " << theParam.anomaly_nite_temperature_max << endl;
    theOutput << " Mean temperature night: " << theParam.anomaly_nite_temperature_mean << endl;
    theOutput << " Minimum temperature d2: " << theParam.anomaly_d2_temperature_min << endl;
    theOutput << " Maximum temperature d2: " << theParam.anomaly_d2_temperature_max << endl;
    theOutput << " Mean temperature d2: " << theParam.anomaly_d2_temperature_mean << endl;
    theOutput << " Minimum windspeed morning d2: " << theParam.anomaly_d2_windspeed_morning_min <<
    endl;
    theOutput << " Maximum windspeed morning d2: " << theParam.anomaly_d2_windspeed_morning_max <<
    endl;
    theOutput << " Mean windspeed morning d2: " << theParam.anomaly_d2_windspeed_morning_mean <<
    endl;
    theOutput << " Minimum windspeed afternoon d2: " << theParam.anomaly_d2_windspeed_afternoon_min
    << endl;
    theOutput << " Maximum windspeed afternoon d2: " << theParam.anomaly_d2_windspeed_afternoon_max
    << endl;
    theOutput << " Mean windspeed afternoon d2: " << theParam.anomaly_d2_windspeed_afternoon_mean <<
    endl;
    theOutput << " Minimum windchill d2: " << theParam.anomaly_d2_windchill_min << endl;
    theOutput << " Maximum windchill d2: " << theParam.anomaly_d2_windchill_max << endl;
    theOutput << " Mean windchill d2: " << theParam.anomaly_d2_windchill_mean << endl;
  */

  return theOutput;
}
void tokenize(const string& str, vector<string>& tokens, const string& delimiters = ";")
{
  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  string::size_type pos = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos)
  {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}

void read_testcasefile(TestCaseContainer& testCases, const string& fileName, const bool& anomaly)
{
  ifstream input_stream;
  input_stream.open(fileName.c_str());
  char buffer[1024];
  int index = 1;

  if (!input_stream)
  {
    cout << "could not open file: " << fileName << endl;
    return;
  }

  int inputLinesCount = anomaly ? 7 : 22;

  vector<string> tokens;
  vector<string> testCaseData;
  while (!input_stream.eof())
  {
    input_stream.getline(buffer, 1023);
    std::string data(buffer);
    if (data == "test_case_begin")
    {
      int inputLines = 0;
      testCaseData.clear();
      while (inputLines < inputLinesCount || input_stream.eof())
      {
        input_stream.getline(buffer, 1023);
        data = buffer;
        if (data == "test_case_end") break;
        if (data.empty() || data.compare(0, 2, "//") == 0) continue;
        tokens.clear();
        tokenize(data, tokens, ";");
        // int tokens_size = tokens.size();
        // if(tokens.size() >= 3)
        {
          // valid line
          if (inputLines < inputLinesCount - 1)
            tokens.erase(tokens.begin() + 3, tokens.end());
          else
            tokens.erase(tokens.begin() + 1, tokens.end());

          for (unsigned int i = 0; i < tokens.size(); i++)
            testCaseData.push_back(tokens[i]);

          inputLines++;
        }
        /*
          else if(tokens.size() == 1) // the story
          {
          testCaseData.push_back(tokens[i]);
          inputLines++;
          }
        */
      }
      if (inputLines < inputLinesCount)
      {
        cout << "errorii" << endl;
        return;
      }
      else
      {
        TemperatureStoryTestParam* param = 0;
        // create a new test case
        if (anomaly)
        {
          boost::trim(testCaseData[18]);
          param = new TemperatureAnomalyTestParam(testCaseData[0].c_str(),
                                                  testCaseData[1].c_str(),
                                                  testCaseData[2].c_str(),
                                                  testCaseData[3].c_str(),
                                                  testCaseData[4].c_str(),
                                                  testCaseData[5].c_str(),
                                                  testCaseData[6].c_str(),
                                                  testCaseData[7].c_str(),
                                                  testCaseData[8].c_str(),

                                                  testCaseData[9].c_str(),
                                                  testCaseData[10].c_str(),
                                                  testCaseData[11].c_str(),
                                                  testCaseData[12].c_str(),
                                                  testCaseData[13].c_str(),
                                                  testCaseData[14].c_str(),
                                                  testCaseData[15].c_str(),
                                                  testCaseData[16].c_str(),
                                                  testCaseData[17].c_str(),
                                                  testCaseData[18].c_str());
        }
        else
        {
          boost::trim(testCaseData[63]);
          param = new Max36HoursTestParam(testCaseData[0].c_str(),
                                          testCaseData[1].c_str(),
                                          testCaseData[2].c_str(),
                                          testCaseData[3].c_str(),
                                          testCaseData[4].c_str(),
                                          testCaseData[5].c_str(),
                                          testCaseData[6].c_str(),
                                          testCaseData[7].c_str(),
                                          testCaseData[8].c_str(),

                                          testCaseData[9].c_str(),
                                          testCaseData[10].c_str(),
                                          testCaseData[11].c_str(),
                                          testCaseData[12].c_str(),
                                          testCaseData[13].c_str(),
                                          testCaseData[14].c_str(),
                                          testCaseData[15].c_str(),
                                          testCaseData[16].c_str(),
                                          testCaseData[17].c_str(),

                                          testCaseData[18].c_str(),
                                          testCaseData[19].c_str(),
                                          testCaseData[20].c_str(),
                                          testCaseData[21].c_str(),
                                          testCaseData[22].c_str(),
                                          testCaseData[23].c_str(),
                                          testCaseData[24].c_str(),
                                          testCaseData[25].c_str(),
                                          testCaseData[26].c_str(),

                                          testCaseData[27].c_str(),
                                          testCaseData[28].c_str(),
                                          testCaseData[29].c_str(),
                                          testCaseData[30].c_str(),
                                          testCaseData[31].c_str(),
                                          testCaseData[32].c_str(),
                                          testCaseData[33].c_str(),
                                          testCaseData[34].c_str(),
                                          testCaseData[35].c_str(),

                                          testCaseData[35].c_str(),
                                          testCaseData[37].c_str(),
                                          testCaseData[38].c_str(),
                                          testCaseData[39].c_str(),
                                          testCaseData[40].c_str(),
                                          testCaseData[41].c_str(),
                                          testCaseData[42].c_str(),
                                          testCaseData[43].c_str(),
                                          testCaseData[44].c_str(),

                                          testCaseData[45].c_str(),
                                          testCaseData[46].c_str(),
                                          testCaseData[47].c_str(),
                                          testCaseData[48].c_str(),
                                          testCaseData[49].c_str(),
                                          testCaseData[50].c_str(),
                                          testCaseData[51].c_str(),
                                          testCaseData[52].c_str(),
                                          testCaseData[53].c_str(),

                                          testCaseData[54].c_str(),
                                          testCaseData[55].c_str(),
                                          testCaseData[56].c_str(),
                                          testCaseData[57].c_str(),
                                          testCaseData[58].c_str(),
                                          testCaseData[59].c_str(),
                                          testCaseData[60].c_str(),
                                          testCaseData[61].c_str(),
                                          testCaseData[62].c_str(),
                                          testCaseData[63].c_str());
        }

        index++;
        testCases.insert(make_pair(index++, param));
      }
    }
    else
    {
      continue;
    }
  }
}

using namespace std;
using namespace TextGen;
using namespace TextGen;

void create_wind_anomaly_testcase_stories(TestCaseContainer& testCases,
                                          const string& theLanguage,
                                          const AnalysisSources& sources,
                                          const WeatherArea& area)
{
  TestCaseContainer::iterator iter;

  for (iter = testCases.begin(); iter != testCases.end(); iter++)
  {
    WindAnomalyTestParam* pTestParam = static_cast<WindAnomalyTestParam*>(iter->second);

    Settings::set("anomaly::fake::temperature::day2::morning::inland::min",
                  pTestParam->temperature_d2_morning_inlandmin);
    Settings::set("anomaly::fake::temperature::day2::morning::inland::max",
                  pTestParam->temperature_d2_morning_inlandmax);
    Settings::set("anomaly::fake::temperature::day2::morning::inland::mean",
                  pTestParam->temperature_d2_morning_inlandmean);
    Settings::set("anomaly::fake::temperature::day2::morning::coast::min",
                  pTestParam->temperature_d2_morning_coastmin);
    Settings::set("anomaly::fake::temperature::day2::morning::coast::max",
                  pTestParam->temperature_d2_morning_coastmax);
    Settings::set("anomaly::fake::temperature::day2::morning::coast::mean",
                  pTestParam->temperature_d2_morning_coastmean);
    Settings::set("anomaly::fake::temperature::day2::morning::area::min",
                  pTestParam->temperature_d2_morning_areamin);
    Settings::set("anomaly::fake::temperature::day2::morning::area::max",
                  pTestParam->temperature_d2_morning_areamax);
    Settings::set("anomaly::fake::temperature::day2::morning::area::mean",
                  pTestParam->temperature_d2_morning_areamean);

    Settings::set("anomaly::fake::temperature::day2::afternoon::inland::min",
                  pTestParam->temperature_d2_afternoon_inlandmin);
    Settings::set("anomaly::fake::temperature::day2::afternoon::inland::max",
                  pTestParam->temperature_d2_afternoon_inlandmax);
    Settings::set("anomaly::fake::temperature::day2::afternoon::inland::mean",
                  pTestParam->temperature_d2_afternoon_inlandmean);
    Settings::set("anomaly::fake::temperature::day2::afternoon::coast::min",
                  pTestParam->temperature_d2_afternoon_coastmin);
    Settings::set("anomaly::fake::temperature::day2::afternoon::coast::max",
                  pTestParam->temperature_d2_afternoon_coastmax);
    Settings::set("anomaly::fake::temperature::day2::afternoon::coast::mean",
                  pTestParam->temperature_d2_afternoon_coastmean);
    Settings::set("anomaly::fake::temperature::day2::afternoon::area::min",
                  pTestParam->temperature_d2_afternoon_areamin);
    Settings::set("anomaly::fake::temperature::day2::afternoon::area::max",
                  pTestParam->temperature_d2_afternoon_areamax);
    Settings::set("anomaly::fake::temperature::day2::afternoon::area::mean",
                  pTestParam->temperature_d2_afternoon_areamean);

    Settings::set("anomaly::fake::windspeed::morning::inland::min",
                  pTestParam->anomaly_d2_windspeed_morning_inlandmin);
    Settings::set("anomaly::fake::windspeed::morning::inland::max",
                  pTestParam->anomaly_d2_windspeed_morning_inlandmax);
    Settings::set("anomaly::fake::windspeed::morning::inland::mean",
                  pTestParam->anomaly_d2_windspeed_morning_inlandmean);
    Settings::set("anomaly::fake::windspeed::morning::coast::min",
                  pTestParam->anomaly_d2_windspeed_morning_coastmin);
    Settings::set("anomaly::fake::windspeed::morning::coast::max",
                  pTestParam->anomaly_d2_windspeed_morning_coastmax);
    Settings::set("anomaly::fake::windspeed::morning::coast::mean",
                  pTestParam->anomaly_d2_windspeed_morning_coastmean);
    Settings::set("anomaly::fake::windspeed::afternoon::inland::min",
                  pTestParam->anomaly_d2_windspeed_afternoon_inlandmin);
    Settings::set("anomaly::fake::windspeed::afternoon::inland::max",
                  pTestParam->anomaly_d2_windspeed_afternoon_inlandmax);
    Settings::set("anomaly::fake::windspeed::afternoon::inland::mean",
                  pTestParam->anomaly_d2_windspeed_afternoon_inlandmean);
    Settings::set("anomaly::fake::windspeed::afternoon::coast::min",
                  pTestParam->anomaly_d2_windspeed_afternoon_coastmin);
    Settings::set("anomaly::fake::windspeed::afternoon::coast::max",
                  pTestParam->anomaly_d2_windspeed_afternoon_coastmax);
    Settings::set("anomaly::fake::windspeed::afternoon::coast::mean",
                  pTestParam->anomaly_d2_windspeed_afternoon_coastmean);

    Settings::set("anomaly::fake::windchill::morning::inland::min",
                  pTestParam->anomaly_d2_windchill_morning_inlandmin);
    Settings::set("anomaly::fake::windchill::morning::inland::max",
                  pTestParam->anomaly_d2_windchill_morning_inlandmax);
    Settings::set("anomaly::fake::windchill::morning::inland::mean",
                  pTestParam->anomaly_d2_windchill_morning_inlandmean);
    Settings::set("anomaly::fake::windchill::morning::coast::min",
                  pTestParam->anomaly_d2_windchill_morning_coastmin);
    Settings::set("anomaly::fake::windchill::morning::coast::max",
                  pTestParam->anomaly_d2_windchill_morning_coastmax);
    Settings::set("anomaly::fake::windchill::morning::coast::mean",
                  pTestParam->anomaly_d2_windchill_morning_coastmean);
    Settings::set("anomaly::fake::windchill::afternoon::inland::min",
                  pTestParam->anomaly_d2_windchill_afternoon_inlandmin);
    Settings::set("anomaly::fake::windchill::afternoon::inland::max",
                  pTestParam->anomaly_d2_windchill_afternoon_inlandmax);
    Settings::set("anomaly::fake::windchill::afternoon::inland::mean",
                  pTestParam->anomaly_d2_windchill_afternoon_inlandmean);
    Settings::set("anomaly::fake::windchill::afternoon::coast::min",
                  pTestParam->anomaly_d2_windchill_afternoon_coastmin);
    Settings::set("anomaly::fake::windchill::afternoon::coast::max",
                  pTestParam->anomaly_d2_windchill_afternoon_coastmax);
    Settings::set("anomaly::fake::windchill::afternoon::coast::mean",
                  pTestParam->anomaly_d2_windchill_afternoon_coastmean);

    TextGenPosixTime time1(2009, 6, 1, 6, 0, 0);
    TextGenPosixTime time2(2009, 6, 2, 18, 0, 0);
    WeatherPeriod period_day1_night_day2(time1, time2);
    TemperatureStory story(time1, sources, area, period_day1_night_day2, "anomaly");
    const string fun = "wind_anomaly";

    pTestParam->theStory = get_story(story, theLanguage, fun);
  }
}

string get_fractile_share(const fractile_id& fractileId,
                          const float& referenceTemperature,
                          const TextGenPosixTime& timestmp)
{
  // winter
  if (timestmp.GetMonth() > 10 || timestmp.GetMonth() < 4)
  {
    if (referenceTemperature < -25)
    {
      switch (fractileId)
      {
        case FRACTILE_02:
          return "90.0,0.0";
          break;
        case FRACTILE_12:
          return "10.0,0.0";
          break;
        case FRACTILE_88:
          return "0.0,0.0";
          break;
        case FRACTILE_98:
          return "0.0,0.0";
          break;
        default:
          return "0.0,0.0";
          break;
      }
    }
    else if (referenceTemperature < -20)
    {
      switch (fractileId)
      {
        case FRACTILE_02:
          return "10.0,0.0";
          break;
        case FRACTILE_12:
          return "90.0,0.0";
          break;
        case FRACTILE_88:
          return "0.0,0.0";
          break;
        case FRACTILE_98:
          return "0.0,0.0";
          break;
        default:
          return "0.0,0.0";
          break;
      }
    }
    else if (referenceTemperature < -15)
    {
      switch (fractileId)
      {
        case FRACTILE_02:
          return "0.0,0.0";
          break;
        case FRACTILE_12:
          return "80.0,0.0";
          break;
        case FRACTILE_88:
          return "0.0,0.0";
          break;
        case FRACTILE_98:
          return "0.0,0.0";
          break;
        default:
          return "0.0,0.0";
          break;
      }
    }
    else if (referenceTemperature < -10)
    {
      switch (fractileId)
      {
        case FRACTILE_02:
          return "0.0,0.0";
          break;
        case FRACTILE_12:
          return "40.0,0.0";
          break;
        case FRACTILE_88:
          return "0.0,0.0";
          break;
        case FRACTILE_98:
          return "0.0,0.0";
          break;
        default:
          return "0.0,0.0";
          break;
      }
    }
    else if (referenceTemperature < -5)
    {
      switch (fractileId)
      {
        case FRACTILE_02:
          return "0.0,0.0";
          break;
        case FRACTILE_12:
          return "0.0,0.0";
          break;
        case FRACTILE_88:
          return "50.0,0.0";
          break;
        case FRACTILE_98:
          return "0.0,0.0";
          break;
        default:
          return "0.0,0.0";
          break;
      }
    }
    else if (referenceTemperature < 0)
    {
      switch (fractileId)
      {
        case FRACTILE_02:
          return "0.0,0.0";
          break;
        case FRACTILE_12:
          return "0.0,0.0";
          break;
        case FRACTILE_88:
          return "10.0,0.0";
          break;
        case FRACTILE_98:
          return "60.0,0.0";
          break;
        default:
          return "0.0,0.0";
          break;
      }
    }
    else if (referenceTemperature < 5)
    {
      switch (fractileId)
      {
        case FRACTILE_02:
          return "0.0,0.0";
          break;
        case FRACTILE_12:
          return "0.0,0.0";
          break;
        case FRACTILE_88:
          return "10.0,0.0";
          break;
        case FRACTILE_98:
          return "90.0,0.0";
          break;
        default:
          return "0.0,0.0";
          break;
      }
    }
  }
  else  // summer
  {
  }
  return "0.0,0.0";
}

void create_temperature_anomaly_testcase_stories(TestCaseContainer& testCases,
                                                 const string& theLanguage,
                                                 const AnalysisSources& sources,
                                                 const WeatherArea& area)
{
  TestCaseContainer::iterator iter;
  unsigned int textCaseCounter(0);

  for (iter = testCases.begin(); iter != testCases.end(); iter++)
  {
    TemperatureAnomalyTestParam* pTestParam =
        static_cast<TemperatureAnomalyTestParam*>(iter->second);

    Settings::set("anomaly::fake::temperature::day1::afternoon::inland::min",
                  pTestParam->temperature_d1_afternoon_inlandmin);
    Settings::set("anomaly::fake::temperature::day1::afternoon::inland::max",
                  pTestParam->temperature_d1_afternoon_inlandmax);
    Settings::set("anomaly::fake::temperature::day1::afternoon::inland::mean",
                  pTestParam->temperature_d1_afternoon_inlandmean);
    Settings::set("anomaly::fake::temperature::day1::afternoon::coast::min",
                  pTestParam->temperature_d1_afternoon_coastmin);
    Settings::set("anomaly::fake::temperature::day1::afternoon::coast::max",
                  pTestParam->temperature_d1_afternoon_coastmax);
    Settings::set("anomaly::fake::temperature::day1::afternoon::coast::mean",
                  pTestParam->temperature_d1_afternoon_coastmean);
    Settings::set("anomaly::fake::temperature::day1::afternoon::area::min",
                  pTestParam->temperature_d1_afternoon_areamin);
    Settings::set("anomaly::fake::temperature::day1::afternoon::area::max",
                  pTestParam->temperature_d1_afternoon_areamax);
    Settings::set("anomaly::fake::temperature::day1::afternoon::area::mean",
                  pTestParam->temperature_d1_afternoon_areamean);

    Settings::set("anomaly::fake::temperature::day2::afternoon::inland::min",
                  pTestParam->temperature_d2_afternoon_inlandmin);
    Settings::set("anomaly::fake::temperature::day2::afternoon::inland::max",
                  pTestParam->temperature_d2_afternoon_inlandmax);
    Settings::set("anomaly::fake::temperature::day2::afternoon::inland::mean",
                  pTestParam->temperature_d2_afternoon_inlandmean);
    Settings::set("anomaly::fake::temperature::day2::afternoon::coast::min",
                  pTestParam->temperature_d2_afternoon_coastmin);
    Settings::set("anomaly::fake::temperature::day2::afternoon::coast::max",
                  pTestParam->temperature_d2_afternoon_coastmax);
    Settings::set("anomaly::fake::temperature::day2::afternoon::coast::mean",
                  pTestParam->temperature_d2_afternoon_coastmean);
    Settings::set("anomaly::fake::temperature::day2::afternoon::area::min",
                  pTestParam->temperature_d2_afternoon_areamin);
    Settings::set("anomaly::fake::temperature::day2::afternoon::area::max",
                  pTestParam->temperature_d2_afternoon_areamax);
    Settings::set("anomaly::fake::temperature::day2::afternoon::area::mean",
                  pTestParam->temperature_d2_afternoon_areamean);
    Settings::set("anomaly::fake::temperature::day_before_day1::afternoon::area::min",
                  pTestParam->temperature_d1_afternoon_areamin);
    Settings::set("anomaly::fake::temperature::day_before_day1::afternoon::area::max",
                  pTestParam->temperature_d1_afternoon_areamax);
    Settings::set("anomaly::fake::temperature::day_before_day1::afternoon::area::mean",
                  pTestParam->temperature_d1_afternoon_areamean);
    Settings::set("anomaly::fake::temperature::day_after_day2::afternoon::area::min",
                  pTestParam->temperature_d2_afternoon_areamin);
    Settings::set("anomaly::fake::temperature::day_after_day2::afternoon::area::max",
                  pTestParam->temperature_d2_afternoon_areamax);
    Settings::set("anomaly::fake::temperature::day_after_day2::afternoon::area::mean",
                  pTestParam->temperature_d2_afternoon_areamean);

    //	  TextGenPosixTime time1(2009, 1, 1, 6, 0, 0);
    // TextGenPosixTime time2(2009, 1, 2, 18, 0, 0);
    TextGenPosixTime time1(2009, 6, 1, 6, 0, 0);
    TextGenPosixTime time2(2009, 6, 2, 18, 0, 0);
    WeatherPeriod period_day1_night_day2(time1, time2);
    TemperatureStory story(time1, sources, area, period_day1_night_day2, "anomaly");
    const string fun = "temperature_anomaly";

    size_t index(pTestParam->temperature_d1_afternoon_areamean.find(","));
    float mean_temperature(
        atof(pTestParam->temperature_d1_afternoon_areamean.substr(0, index).c_str()));

    Settings::set("anomaly::fake::fractile::share::F02",
                  get_fractile_share(FRACTILE_02, mean_temperature, time2));
    Settings::set("anomaly::fake::fractile::share::F12",
                  get_fractile_share(FRACTILE_12, mean_temperature, time2));
    Settings::set("anomaly::fake::fractile::share::F88",
                  get_fractile_share(FRACTILE_88, mean_temperature, time2));
    Settings::set("anomaly::fake::fractile::share::F98",
                  get_fractile_share(FRACTILE_98, mean_temperature, time2));

    pTestParam->theStory = get_story(story, theLanguage, fun);

    textCaseCounter++;
  }
}

void create_max36hours_testcase_stories(TestCaseContainer& testCases,
                                        const string& theLanguage,
                                        const AnalysisSources& sources,
                                        const WeatherArea& area)
{
  TestCaseContainer::iterator iter;

  Max36HoursTestParam* param = 0;

  for (iter = testCases.begin(); iter != testCases.end(); iter++)
  {
    param = static_cast<Max36HoursTestParam*>(iter->second);

    Settings::set("max36hours::fake::day1::inland::min", param->temperature_d1_inlandmin);
    Settings::set("max36hours::fake::day1::inland::max", param->temperature_d1_inlandmax);
    Settings::set("max36hours::fake::day1::inland::mean", param->temperature_d1_inlandmean);
    Settings::set("max36hours::fake::day1::coast::min", param->temperature_d1_coastmin);
    Settings::set("max36hours::fake::day1::coast::max", param->temperature_d1_coastmax);
    Settings::set("max36hours::fake::day1::coast::mean", param->temperature_d1_coastmean);
    Settings::set("max36hours::fake::day1::area::min", param->temperature_d1_areamin);
    Settings::set("max36hours::fake::day1::area::max", param->temperature_d1_areamax);
    Settings::set("max36hours::fake::day1::area::mean", param->temperature_d1_areamean);

    Settings::set("max36hours::fake::day1::morning::inland::min",
                  param->temperature_d1_morning_inlandmin);
    Settings::set("max36hours::fake::day1::morning::inland::max",
                  param->temperature_d1_morning_inlandmax);
    Settings::set("max36hours::fake::day1::morning::inland::mean",
                  param->temperature_d1_morning_inlandmean);
    Settings::set("max36hours::fake::day1::morning::coast::min",
                  param->temperature_d1_morning_coastmin);
    Settings::set("max36hours::fake::day1::morning::coast::max",
                  param->temperature_d1_morning_coastmax);
    Settings::set("max36hours::fake::day1::morning::coast::mean",
                  param->temperature_d1_morning_coastmean);
    Settings::set("max36hours::fake::day1::morning::area::min",
                  param->temperature_d1_morning_areamin);
    Settings::set("max36hours::fake::day1::morning::area::max",
                  param->temperature_d1_morning_areamax);
    Settings::set("max36hours::fake::day1::morning::area::mean",
                  param->temperature_d1_morning_areamean);

    Settings::set("max36hours::fake::day1::afternoon::inland::min",
                  param->temperature_d1_afternoon_inlandmin);
    Settings::set("max36hours::fake::day1::afternoon::inland::max",
                  param->temperature_d1_afternoon_inlandmax);
    Settings::set("max36hours::fake::day1::afternoon::inland::mean",
                  param->temperature_d1_afternoon_inlandmean);
    Settings::set("max36hours::fake::day1::afternoon::coast::min",
                  param->temperature_d1_afternoon_coastmin);
    Settings::set("max36hours::fake::day1::afternoon::coast::max",
                  param->temperature_d1_afternoon_coastmax);
    Settings::set("max36hours::fake::day1::afternoon::coast::mean",
                  param->temperature_d1_afternoon_coastmean);
    Settings::set("max36hours::fake::day1::afternoon::area::min",
                  param->temperature_d1_afternoon_areamin);
    Settings::set("max36hours::fake::day1::afternoon::area::max",
                  param->temperature_d1_afternoon_areamax);
    Settings::set("max36hours::fake::day1::afternoon::area::mean",
                  param->temperature_d1_afternoon_areamean);

    Settings::set("max36hours::fake::night::inland::min", param->temperature_nite_inlandmin);
    Settings::set("max36hours::fake::night::inland::max", param->temperature_nite_inlandmax);
    Settings::set("max36hours::fake::night::inland::mean", param->temperature_nite_inlandmean);
    Settings::set("max36hours::fake::night::coast::min", param->temperature_nite_coastmin);
    Settings::set("max36hours::fake::night::coast::max", param->temperature_nite_coastmax);
    Settings::set("max36hours::fake::night::coast::mean", param->temperature_nite_coastmean);
    Settings::set("max36hours::fake::night::area::min", param->temperature_nite_areamin);
    Settings::set("max36hours::fake::night::area::max", param->temperature_nite_areamax);
    Settings::set("max36hours::fake::night::area::mean", param->temperature_nite_areamean);

    Settings::set("max36hours::fake::day2::inland::min", param->temperature_d2_inlandmin);
    Settings::set("max36hours::fake::day2::inland::max", param->temperature_d2_inlandmax);
    Settings::set("max36hours::fake::day2::inland::mean", param->temperature_d2_inlandmean);
    Settings::set("max36hours::fake::day2::coast::min", param->temperature_d2_coastmin);
    Settings::set("max36hours::fake::day2::coast::max", param->temperature_d2_coastmax);
    Settings::set("max36hours::fake::day2::coast::mean", param->temperature_d2_coastmean);
    Settings::set("max36hours::fake::day2::area::min", param->temperature_d2_areamin);
    Settings::set("max36hours::fake::day2::area::max", param->temperature_d2_areamax);
    Settings::set("max36hours::fake::day2::area::mean", param->temperature_d2_areamean);

    Settings::set("max36hours::fake::day2::morning::inland::min",
                  param->temperature_d2_morning_inlandmin);
    Settings::set("max36hours::fake::day2::morning::inland::max",
                  param->temperature_d2_morning_inlandmax);
    Settings::set("max36hours::fake::day2::morning::inland::mean",
                  param->temperature_d2_morning_inlandmean);
    Settings::set("max36hours::fake::day2::morning::coast::min",
                  param->temperature_d2_morning_coastmin);
    Settings::set("max36hours::fake::day2::morning::coast::max",
                  param->temperature_d2_morning_coastmax);
    Settings::set("max36hours::fake::day2::morning::coast::mean",
                  param->temperature_d2_morning_coastmean);
    Settings::set("max36hours::fake::day2::morning::area::min",
                  param->temperature_d2_morning_areamin);
    Settings::set("max36hours::fake::day2::morning::area::max",
                  param->temperature_d2_morning_areamax);
    Settings::set("max36hours::fake::day2::morning::area::mean",
                  param->temperature_d2_morning_areamean);

    Settings::set("max36hours::fake::day2::afternoon::inland::min",
                  param->temperature_d2_afternoon_inlandmin);
    Settings::set("max36hours::fake::day2::afternoon::inland::max",
                  param->temperature_d2_afternoon_inlandmax);
    Settings::set("max36hours::fake::day2::afternoon::inland::mean",
                  param->temperature_d2_afternoon_inlandmean);
    Settings::set("max36hours::fake::day2::afternoon::coast::min",
                  param->temperature_d2_afternoon_coastmin);
    Settings::set("max36hours::fake::day2::afternoon::coast::max",
                  param->temperature_d2_afternoon_coastmax);
    Settings::set("max36hours::fake::day2::afternoon::coast::mean",
                  param->temperature_d2_afternoon_coastmean);
    Settings::set("max36hours::fake::day2::afternoon::area::min",
                  param->temperature_d2_afternoon_areamin);
    Settings::set("max36hours::fake::day2::afternoon::area::max",
                  param->temperature_d2_afternoon_areamax);
    Settings::set("max36hours::fake::day2::afternoon::area::mean",
                  param->temperature_d2_afternoon_areamean);

    TextGenPosixTime time1(2009, 6, 1, 6, 0, 0);
    TextGenPosixTime time2(2009, 6, 2, 18, 0, 0);
    WeatherPeriod period_day1_night_day2(time1, time2);
    TemperatureStory story1(time1, sources, area, period_day1_night_day2, "max36hours");
    const string fun = "temperature_max36hours";

    param->theStory = get_story(story1, theLanguage, fun);
  }
}

void create_max36hours_testcasefile(TestCaseContainer& testCases, const string& fileName)
{
  ofstream output_stream;
  output_stream.open(fileName.c_str());

  Max36HoursTestParam* param = 0;

  int i = 1;
  for (TestCaseContainer::iterator iter = testCases.begin(); iter != testCases.end(); iter++)
  {
    param = static_cast<Max36HoursTestParam*>(iter->second);

#ifdef CPP_TESTCASE

    output_stream << "testCases.insert(make_pair(i++, Max36HoursTestParam(" << endl;

    output_stream << "\"" << param->temperature_d1_inlandmin << "\",\"";
    output_stream << param->temperature_d1_inlandmax << "\",\"";
    output_stream << param->temperature_d1_inlandmean << "\", // inland day1: min, max, mean"
                  << endl;
    output_stream << "\"" << param->temperature_d1_coastmin << "\",\"";
    output_stream << param->temperature_d1_coastmax << "\",\"";
    output_stream << param->temperature_d1_coastmean << "\", // coast day1: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_areamin << "\",\"";
    output_stream << param->temperature_d1_areamax << "\",\"";
    output_stream << param->temperature_d1_areamean << "\", // area day1: min, max, mean" << endl;

    output_stream << "\"" << param->temperature_d1_morning_inlandmin << "\",\"";
    output_stream << param->temperature_d1_morning_inlandmax << "\",\"";
    output_stream << param->temperature_d1_morning_inlandmean
                  << "\", // inland day1 morning: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_morning_coastmin << "\",\"";
    output_stream << param->temperature_d1_morning_coastmax << "\",\"";
    output_stream << param->temperature_d1_morning_coastmean
                  << "\", // coast day1 morning: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_morning_areamin << "\",\"";
    output_stream << param->temperature_d1_morning_areamax << "\",\"";
    output_stream << param->temperature_d1_morning_areamean
                  << "\", // area day1 morning: min, max, mean" << endl;

    output_stream << "\"" << param->temperature_d1_afternoon_inlandmin << "\",\"";
    output_stream << param->temperature_d1_afternoon_inlandmax << "\",\"";
    output_stream << param->temperature_d1_afternoon_inlandmean
                  << "\", // inland day1 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_afternoon_coastmin << "\",\"";
    output_stream << param->temperature_d1_afternoon_coastmax << "\",\"";
    output_stream << param->temperature_d1_afternoon_coastmean
                  << "\", // coast day1 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_afternoon_areamin << "\",\"";
    output_stream << param->temperature_d1_afternoon_areamax << "\",\"";
    output_stream << param->temperature_d1_afternoon_areamean
                  << "\", // area day1 afternoon: min, max, mean" << endl;

    output_stream << "\"" << param->temperature_nite_inlandmin << "\",\"";
    output_stream << param->temperature_nite_inlandmax << "\",\"";
    output_stream << param->temperature_nite_inlandmean << "\", // inland night: min, max, mean"
                  << endl;
    output_stream << "\"" << param->temperature_nite_coastmin << "\",\"";
    output_stream << param->temperature_nite_coastmax << "\",\"";
    output_stream << param->temperature_nite_coastmean << "\", // coast night: min, max, mean"
                  << endl;

    output_stream << "\"" << param->temperature_nite_areamin << "\",\"";
    output_stream << param->temperature_nite_areamax << "\",\"";
    output_stream << param->temperature_nite_areamean << "\", // area night: min, max, mean"
                  << endl;

    output_stream << "\"" << param->temperature_d2_inlandmin << "\",\"";
    output_stream << param->temperature_d2_inlandmax << "\",\"";
    output_stream << param->temperature_d2_inlandmean << "\", // inland day2: min, max, mean"
                  << endl;
    output_stream << "\"" << param->temperature_d2_coastmin << "\",\"";
    output_stream << param->temperature_d2_coastmax << "\",\"";
    output_stream << param->temperature_d2_coastmean << "\", // coast day2: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_areamin << "\",\"";
    output_stream << param->temperature_d2_areamax << "\",\"";
    output_stream << param->temperature_d2_areamean << "\", // area day2: min, max, mean" << endl;

    output_stream << "\"" << param->temperature_d2_morning_inlandmin << "\",\"";
    output_stream << param->temperature_d2_morning_inlandmax << "\",\"";
    output_stream << param->temperature_d2_morning_inlandmean
                  << "\", // inland day2 morning: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_morning_coastmin << "\",\"";
    output_stream << param->temperature_d2_morning_coastmax << "\",\"";
    output_stream << param->temperature_d2_morning_coastmean
                  << "\", // coast day2 morning: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_morning_areamin << "\",\"";
    output_stream << param->temperature_d2_morning_areamax << "\",\"";
    output_stream << param->temperature_d2_morning_areamean
                  << "\", // area day2 morning: min, max, mean" << endl;

    output_stream << "\"" << param->temperature_d2_afternoon_inlandmin << "\",\"";
    output_stream << param->temperature_d2_afternoon_inlandmax << "\",\"";
    output_stream << param->temperature_d2_afternoon_inlandmean
                  << "\", // inland day2 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_afternoon_coastmin << "\",\"";
    output_stream << param->temperature_d2_afternoon_coastmax << "\",\"";
    output_stream << param->temperature_d2_afternoon_coastmean
                  << "\", // coast day2 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_afternoon_areamin << "\",\"";
    output_stream << param->temperature_d2_afternoon_areamax << "\",\"";
    output_stream << param->temperature_d2_afternoon_areamean
                  << "\", // area day2 afternoon: min, max, mean" << endl;

    output_stream << "\"" << param->theStory << "\"))); // the story" << endl;
#endif

    output_stream << "#" << i++ << endl;
    output_stream << "test_case_begin" << endl;
    output_stream << param->temperature_d1_inlandmin << ";";
    output_stream << param->temperature_d1_inlandmax << ";";
    output_stream << param->temperature_d1_inlandmean << "; // inland day1: min, max, mean" << endl;
    output_stream << param->temperature_d1_coastmin << ";";
    output_stream << param->temperature_d1_coastmax << ";";
    output_stream << param->temperature_d1_coastmean << "; // coast day1: min, max, mean" << endl;
    output_stream << param->temperature_d1_areamin << ";";
    output_stream << param->temperature_d1_areamax << ";";
    output_stream << param->temperature_d1_areamean << "; // area day1: min, max, mean" << endl;

    output_stream << param->temperature_d1_morning_inlandmin << ";";
    output_stream << param->temperature_d1_morning_inlandmax << ";";
    output_stream << param->temperature_d1_morning_inlandmean
                  << "; // inland day1 morning: min, max, mean" << endl;
    output_stream << param->temperature_d1_morning_coastmin << ";";
    output_stream << param->temperature_d1_morning_coastmax << ";";
    output_stream << param->temperature_d1_morning_coastmean
                  << "; // coast day1 morning: min, max, mean" << endl;
    output_stream << param->temperature_d1_morning_areamin << ";";
    output_stream << param->temperature_d1_morning_areamax << ";";
    output_stream << param->temperature_d1_morning_areamean
                  << "; // area day1 morning: min, max, mean" << endl;

    output_stream << param->temperature_d1_afternoon_inlandmin << ";";
    output_stream << param->temperature_d1_afternoon_inlandmax << ";";
    output_stream << param->temperature_d1_afternoon_inlandmean
                  << "; // inland day1 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d1_afternoon_coastmin << ";";
    output_stream << param->temperature_d1_afternoon_coastmax << ";";
    output_stream << param->temperature_d1_afternoon_coastmean
                  << "; // coast day1 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d1_afternoon_areamin << ";";
    output_stream << param->temperature_d1_afternoon_areamax << ";";
    output_stream << param->temperature_d1_afternoon_areamean
                  << "; // area day1 afternoon: min, max, mean" << endl;

    output_stream << param->temperature_nite_inlandmin << ";";
    output_stream << param->temperature_nite_inlandmax << ";";
    output_stream << param->temperature_nite_inlandmean << "; // inland night: min, max, mean"
                  << endl;
    output_stream << param->temperature_nite_coastmin << ";";
    output_stream << param->temperature_nite_coastmax << ";";
    output_stream << param->temperature_nite_coastmean << "; // coast night: min, max, mean"
                  << endl;
    output_stream << param->temperature_nite_areamin << ";";
    output_stream << param->temperature_nite_areamax << ";";
    output_stream << param->temperature_nite_areamean << "; // area night: min, max, mean" << endl;

    output_stream << param->temperature_d2_inlandmin << ";";
    output_stream << param->temperature_d2_inlandmax << ";";
    output_stream << param->temperature_d2_inlandmean << "; // inland day2: min, max, mean" << endl;
    output_stream << param->temperature_d2_coastmin << ";";
    output_stream << param->temperature_d2_coastmax << ";";
    output_stream << param->temperature_d2_coastmean << "; // coast day2: min, max, mean" << endl;
    output_stream << param->temperature_d2_areamin << ";";
    output_stream << param->temperature_d2_areamax << ";";
    output_stream << param->temperature_d2_areamean << "; // area day2: min, max, mean" << endl;

    output_stream << param->temperature_d2_morning_inlandmin << ";";
    output_stream << param->temperature_d2_morning_inlandmax << ";";
    output_stream << param->temperature_d2_morning_inlandmean
                  << "; // inland day2 morning: min, max, mean" << endl;
    output_stream << param->temperature_d2_morning_coastmin << ";";
    output_stream << param->temperature_d2_morning_coastmax << ";";
    output_stream << param->temperature_d2_morning_coastmean
                  << "; // coast day2 morning: min, max, mean" << endl;
    output_stream << param->temperature_d2_morning_areamin << ";";
    output_stream << param->temperature_d2_morning_areamax << ";";
    output_stream << param->temperature_d2_morning_areamean
                  << "; // area day2 morning: min, max, mean" << endl;

    output_stream << param->temperature_d2_afternoon_inlandmin << ";";
    output_stream << param->temperature_d2_afternoon_inlandmax << ";";
    output_stream << param->temperature_d2_afternoon_inlandmean
                  << "; // inland day2 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d2_afternoon_coastmin << ";";
    output_stream << param->temperature_d2_afternoon_coastmax << ";";
    output_stream << param->temperature_d2_afternoon_coastmean
                  << "; // coast day2 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d2_afternoon_areamin << ";";
    output_stream << param->temperature_d2_afternoon_areamax << ";";
    output_stream << param->temperature_d2_afternoon_areamean
                  << "; // area day2 afternoon: min, max, mean" << endl;

    output_stream << param->theStory << "; // the story" << endl;
    output_stream << "test_case_end" << endl;
  }
}

void create_temperature_anomaly_testcasefile(TestCaseContainer& testCases, const string& fileName)
{
  ofstream output_stream;
  output_stream.open(fileName.c_str());

  TemperatureAnomalyTestParam* param = 0;
  int i = 1;
  for (TestCaseContainer::iterator iter = testCases.begin(); iter != testCases.end(); iter++)
  {
    param = static_cast<TemperatureAnomalyTestParam*>(iter->second);

#ifdef CPP_TESTCASES

    output_stream << "testCases.insert(make_pair(i++, TemperatureAnomalyTestParam(" << endl;

    output_stream << "\"" << param->temperature_d1_afternoon_inlandmin << "\",\"";
    output_stream << param->temperature_d1_afternoon_inlandmax << "\",\"";
    output_stream << param->temperature_d1_afternoon_inlandmean
                  << "\", // inland day1 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_afternoon_coastmin << "\",\"";
    output_stream << param->temperature_d1_afternoon_coastmax << "\",\"";
    output_stream << param->temperature_d1_afternoon_coastmean
                  << "\", // coast day1 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_afternoon_areamin << "\",\"";
    output_stream << param->temperature_d1_afternoon_areamax << "\",\"";
    output_stream << param->temperature_d1_afternoon_areamean
                  << "\", // area day1 afternoon: min, max, mean" << endl;

    output_stream << "\"" << param->temperature_d2_afternoon_inlandmin << "\",\"";
    output_stream << param->temperature_d2_afternoon_inlandmax << "\",\"";
    output_stream << param->temperature_d2_afternoon_inlandmean
                  << "\", // inland day2 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_afternoon_coastmin << "\",\"";
    output_stream << param->temperature_d2_afternoon_coastmax << "\",\"";
    output_stream << param->temperature_d2_afternoon_coastmean
                  << "\", // coast day2 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_afternoon_areamin << "\",\"";
    output_stream << param->temperature_d2_afternoon_areamax << "\",\"";
    output_stream << param->temperature_d2_afternoon_areamean
                  << "\", // area day2 afternoon: min, max, mean" << endl;

    output_stream << "\"" << param->theStory << "\"))); // the story" << endl;
#endif

    output_stream << "#" << i++ << endl;
    output_stream << "test_case_begin" << endl;

    output_stream << param->temperature_d1_afternoon_inlandmin << ";";
    output_stream << param->temperature_d1_afternoon_inlandmax << ";";
    output_stream << param->temperature_d1_afternoon_inlandmean
                  << "; // inland day1 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d1_afternoon_coastmin << ";";
    output_stream << param->temperature_d1_afternoon_coastmax << ";";
    output_stream << param->temperature_d1_afternoon_coastmean
                  << "; // coast day1 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d1_afternoon_areamin << ";";
    output_stream << param->temperature_d1_afternoon_areamax << ";";
    output_stream << param->temperature_d1_afternoon_areamean
                  << "; // area day1 afternoon: min, max, mean" << endl;

    output_stream << param->temperature_d2_afternoon_inlandmin << ";";
    output_stream << param->temperature_d2_afternoon_inlandmax << ";";
    output_stream << param->temperature_d2_afternoon_inlandmean
                  << "; // inland day2 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d2_afternoon_coastmin << ";";
    output_stream << param->temperature_d2_afternoon_coastmax << ";";
    output_stream << param->temperature_d2_afternoon_coastmean
                  << "; // coast day2 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d2_afternoon_areamin << ";";
    output_stream << param->temperature_d2_afternoon_areamax << ";";
    output_stream << param->temperature_d2_afternoon_areamean
                  << "; // area day2 afternoon: min, max, mean" << endl;

    if (param->theStory.empty())
      output_stream << " ; // the story" << endl;
    else
      output_stream << param->theStory << "; // the story" << endl;

    output_stream << "test_case_end" << endl;
  }
}

void create_wind_anomaly_testcasefile(TestCaseContainer& testCases, const string& fileName)
{
  ofstream output_stream;
  output_stream.open(fileName.c_str());

  WindAnomalyTestParam* param = 0;
  int i = 1;
  for (TestCaseContainer::iterator iter = testCases.begin(); iter != testCases.end(); iter++)
  {
    param = static_cast<WindAnomalyTestParam*>(iter->second);

#ifdef CPP_TESTCASES

    output_stream << "testCases.insert(make_pair(i++, WindAnomalyTestParam(" << endl;

    output_stream << "\"" << param->temperature_d1_afternoon_inlandmin << "\",\"";
    output_stream << param->temperature_d1_afternoon_inlandmax << "\",\"";
    output_stream << param->temperature_d1_afternoon_inlandmean
                  << "\", // inland day1 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_afternoon_coastmin << "\",\"";
    output_stream << param->temperature_d1_afternoon_coastmax << "\",\"";
    output_stream << param->temperature_d1_afternoon_coastmean
                  << "\", // coast day1 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d1_afternoon_areamin << "\",\"";
    output_stream << param->temperature_d1_afternoon_areamax << "\",\"";
    output_stream << param->temperature_d1_afternoon_areamean
                  << "\", // area day1 afternoon: min, max, mean" << endl;

    output_stream << "\"" << param->temperature_d2_afternoon_inlandmin << "\",\"";
    output_stream << param->temperature_d2_afternoon_inlandmax << "\",\"";
    output_stream << param->temperature_d2_afternoon_inlandmean
                  << "\", // inland day2 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_afternoon_coastmin << "\",\"";
    output_stream << param->temperature_d2_afternoon_coastmax << "\",\"";
    output_stream << param->temperature_d2_afternoon_coastmean
                  << "\", // coast day2 afternoon: min, max, mean" << endl;
    output_stream << "\"" << param->temperature_d2_afternoon_areamin << "\",\"";
    output_stream << param->temperature_d2_afternoon_areamax << "\",\"";
    output_stream << param->temperature_d2_afternoon_areamean
                  << "\", // area day2 afternoon: min, max, mean" << endl;

    output_stream << "\"" << param->theStory << "\"))); // the story" << endl;
#endif

    output_stream << "#" << i++ << endl;
    output_stream << "test_case_begin" << endl;

    output_stream << param->temperature_d2_morning_inlandmin << ";";
    output_stream << param->temperature_d2_morning_inlandmax << ";";
    output_stream << param->temperature_d2_morning_inlandmean
                  << "; // inland day2 morning: min, max, mean" << endl;
    output_stream << param->temperature_d2_morning_coastmin << ";";
    output_stream << param->temperature_d2_morning_coastmax << ";";
    output_stream << param->temperature_d2_morning_coastmean
                  << "; // coast day2 morning: min, max, mean" << endl;

    output_stream << param->temperature_d2_morning_areamin << ";";
    output_stream << param->temperature_d2_morning_areamax << ";";
    output_stream << param->temperature_d2_morning_areamean
                  << "; // area day2 morning: min, max, mean" << endl;

    output_stream << param->temperature_d2_afternoon_inlandmin << ";";
    output_stream << param->temperature_d2_afternoon_inlandmax << ";";
    output_stream << param->temperature_d2_afternoon_inlandmean
                  << "; // inland day2 afternoon: min, max, mean" << endl;
    output_stream << param->temperature_d2_afternoon_coastmin << ";";
    output_stream << param->temperature_d2_afternoon_coastmax << ";";
    output_stream << param->temperature_d2_afternoon_coastmean
                  << "; // coast day2 afternoon: min, max, mean" << endl;

    output_stream << param->temperature_d2_afternoon_areamin << ";";
    output_stream << param->temperature_d2_afternoon_areamax << ";";
    output_stream << param->temperature_d2_afternoon_areamean
                  << "; // area day2 afternoon: min, max, mean" << endl;

    output_stream << param->anomaly_d2_windspeed_morning_inlandmin << ";";
    output_stream << param->anomaly_d2_windspeed_morning_inlandmax << ";";
    output_stream << param->anomaly_d2_windspeed_morning_inlandmean
                  << "; // inland day2 morning windspeed min, max, mean" << endl;

    output_stream << param->anomaly_d2_windspeed_morning_coastmin << ";";
    output_stream << param->anomaly_d2_windspeed_morning_coastmax << ";";
    output_stream << param->anomaly_d2_windspeed_morning_coastmean
                  << "; // coast day2 morning windspeed min, max, mean" << endl;

    output_stream << param->anomaly_d2_windspeed_afternoon_inlandmin << ";";
    output_stream << param->anomaly_d2_windspeed_afternoon_inlandmax << ";";
    output_stream << param->anomaly_d2_windspeed_afternoon_inlandmean
                  << "; // inland day2 afternoon windspeed min, max, mean" << endl;

    output_stream << param->anomaly_d2_windspeed_afternoon_coastmin << ";";
    output_stream << param->anomaly_d2_windspeed_afternoon_coastmax << ";";
    output_stream << param->anomaly_d2_windspeed_afternoon_coastmean
                  << "; // coast day2 afternoon windspeed min, max, mean" << endl;

    output_stream << param->anomaly_d2_windchill_morning_inlandmin << ";";
    output_stream << param->anomaly_d2_windchill_morning_inlandmax << ";";
    output_stream << param->anomaly_d2_windchill_morning_inlandmean
                  << "; // inland day2 morning windchill min, max, mean" << endl;

    output_stream << param->anomaly_d2_windchill_morning_coastmin << ";";
    output_stream << param->anomaly_d2_windchill_morning_coastmax << ";";
    output_stream << param->anomaly_d2_windchill_morning_coastmean
                  << "; // coast day2 morning windchill min, max, mean" << endl;

    output_stream << param->anomaly_d2_windchill_afternoon_inlandmin << ";";
    output_stream << param->anomaly_d2_windchill_afternoon_inlandmax << ";";
    output_stream << param->anomaly_d2_windchill_afternoon_inlandmean
                  << "; // inland day2 afternoon windchill min, max, mean" << endl;

    output_stream << param->anomaly_d2_windchill_afternoon_coastmin << ";";
    output_stream << param->anomaly_d2_windchill_afternoon_coastmax << ";";
    output_stream << param->anomaly_d2_windchill_afternoon_coastmean
                  << "; // coast day2 afternoon windchill min, max, mean" << endl;

    if (param->theStory.empty())
      output_stream << " ; // the story" << endl;
    else
      output_stream << param->theStory << "; // the story" << endl;

    output_stream << "test_case_end" << endl;
  }
}

int get_windchill(const float& theTemperature, const float& theWindSpeed)
{
  if (theTemperature >= 0.0) return 0;

  if (theWindSpeed < 6 && theTemperature < -15.0)
  {
    return static_cast<int>(theTemperature + (2 * theWindSpeed));
  }
  else
  {
    return static_cast<int>(theTemperature - theWindSpeed);
  }
}

void generate_testcase_parameters(TestCaseContainer& testCases,
                                  const string& language,
                                  const TemperatureTestType& testCaseType)
{
  srand(time(NULL));
  char d1_area_morning_min_str[15], d1_area_morning_max_str[15], d1_area_morning_mean_str[15],
      d1_area_afternoon_min_str[15], d1_area_afternoon_max_str[15], d1_area_afternoon_mean_str[15],
      d1_inland_morning_min_str[15], d1_inland_morning_max_str[15], d1_inland_morning_mean_str[15],
      d1_inland_afternoon_min_str[15], d1_inland_afternoon_max_str[15],
      d1_inland_afternoon_mean_str[15], d1_coast_morning_min_str[15], d1_coast_morning_max_str[15],
      d1_coast_morning_mean_str[15], d1_coast_afternoon_min_str[15], d1_coast_afternoon_max_str[15],
      d1_coast_afternoon_mean_str[15], d1_windspeed_inland_morning_min_str[15],
      d1_windspeed_inland_morning_max_str[15], d1_windspeed_inland_morning_mean_str[15],
      d1_windspeed_inland_afternoon_min_str[15], d1_windspeed_inland_afternoon_max_str[15],
      d1_windspeed_inland_afternoon_mean_str[15], d1_windspeed_coast_morning_min_str[15],
      d1_windspeed_coast_morning_max_str[15], d1_windspeed_coast_morning_mean_str[15],
      d1_windspeed_coast_afternoon_min_str[15], d1_windspeed_coast_afternoon_max_str[15],
      d1_windspeed_coast_afternoon_mean_str[15], d1_windchill_inland_morning_min_str[15],
      d1_windchill_inland_morning_max_str[15], d1_windchill_inland_morning_mean_str[15],
      d1_windchill_inland_afternoon_min_str[15], d1_windchill_inland_afternoon_max_str[15],
      d1_windchill_inland_afternoon_mean_str[15], d1_windchill_coast_morning_min_str[15],
      d1_windchill_coast_morning_max_str[15], d1_windchill_coast_morning_mean_str[15],
      d1_windchill_coast_afternoon_min_str[15], d1_windchill_coast_afternoon_max_str[15],
      d1_windchill_coast_afternoon_mean_str[15], area_night_min_str[15], area_night_max_str[15],
      area_night_mean_str[15], inland_night_min_str[15], inland_night_max_str[15],
      inland_night_mean_str[15], coast_night_min_str[15], coast_night_max_str[15],
      coast_night_mean_str[15], d1_area_min_str[15], d1_area_max_str[15], d1_area_mean_str[15],
      d1_inland_min_str[15], d1_inland_max_str[15], d1_inland_mean_str[15], d1_coast_min_str[15],
      d1_coast_max_str[15], d1_coast_mean_str[15], d2_area_morning_min_str[15],
      d2_area_morning_max_str[15], d2_area_morning_mean_str[15], d2_area_afternoon_min_str[15],
      d2_area_afternoon_max_str[15], d2_area_afternoon_mean_str[15], d2_inland_morning_min_str[15],
      d2_inland_morning_max_str[15], d2_inland_morning_mean_str[15],
      d2_inland_afternoon_min_str[15], d2_inland_afternoon_max_str[15],
      d2_inland_afternoon_mean_str[15], d2_coast_morning_min_str[15], d2_coast_morning_max_str[15],
      d2_coast_morning_mean_str[15], d2_coast_afternoon_min_str[15], d2_coast_afternoon_max_str[15],
      d2_coast_afternoon_mean_str[15], d2_area_min_str[15], d2_area_max_str[15],
      d2_area_mean_str[15], d2_inland_min_str[15], d2_inland_max_str[15], d2_inland_mean_str[15],
      d2_coast_min_str[15], d2_coast_max_str[15], d2_coast_mean_str[15],
      d2_windspeed_inland_morning_min_str[15], d2_windspeed_inland_morning_max_str[15],
      d2_windspeed_inland_morning_mean_str[15], d2_windspeed_inland_afternoon_min_str[15],
      d2_windspeed_inland_afternoon_max_str[15], d2_windspeed_inland_afternoon_mean_str[15],
      d2_windspeed_coast_morning_min_str[15], d2_windspeed_coast_morning_max_str[15],
      d2_windspeed_coast_morning_mean_str[15], d2_windspeed_coast_afternoon_min_str[15],
      d2_windspeed_coast_afternoon_max_str[15], d2_windspeed_coast_afternoon_mean_str[15],
      d2_windchill_inland_morning_min_str[15], d2_windchill_inland_morning_max_str[15],
      d2_windchill_inland_morning_mean_str[15], d2_windchill_inland_afternoon_min_str[15],
      d2_windchill_inland_afternoon_max_str[15], d2_windchill_inland_afternoon_mean_str[15],
      d2_windchill_coast_morning_min_str[15], d2_windchill_coast_morning_max_str[15],
      d2_windchill_coast_morning_mean_str[15], d2_windchill_coast_afternoon_min_str[15],
      d2_windchill_coast_afternoon_max_str[15], d2_windchill_coast_afternoon_mean_str[15];

  double d1_area_morning_min, d1_area_morning_max, d1_area_morning_mean, d1_area_afternoon_min,
      d1_area_afternoon_max, d1_area_afternoon_mean, d1_inland_morning_min, d1_inland_morning_max,
      d1_inland_morning_mean, d1_inland_afternoon_min, d1_inland_afternoon_max,
      d1_inland_afternoon_mean, d1_coast_morning_min, d1_coast_morning_max, d1_coast_morning_mean,
      d1_coast_afternoon_min, d1_coast_afternoon_max, d1_coast_afternoon_mean,
      d1_windspeed_inland_morning_min, d1_windspeed_inland_morning_max,
      d1_windspeed_inland_morning_mean, d1_windspeed_inland_afternoon_min,
      d1_windspeed_inland_afternoon_max, d1_windspeed_inland_afternoon_mean,
      d1_windspeed_coast_morning_min, d1_windspeed_coast_morning_max,
      d1_windspeed_coast_morning_mean, d1_windspeed_coast_afternoon_min,
      d1_windspeed_coast_afternoon_max, d1_windspeed_coast_afternoon_mean,
      d1_windchill_inland_morning_min, d1_windchill_inland_morning_max,
      d1_windchill_inland_morning_mean, d1_windchill_inland_afternoon_min,
      d1_windchill_inland_afternoon_max, d1_windchill_inland_afternoon_mean,
      d1_windchill_coast_morning_min, d1_windchill_coast_morning_max,
      d1_windchill_coast_morning_mean, d1_windchill_coast_afternoon_min,
      d1_windchill_coast_afternoon_max, d1_windchill_coast_afternoon_mean, area_night_min,
      area_night_max, area_night_mean, inland_night_min, inland_night_max, inland_night_mean,
      coast_night_min, coast_night_max, coast_night_mean, d1_area_min, d1_area_max, d1_area_mean,
      d1_inland_min, d1_inland_max, d1_inland_mean, d1_coast_min, d1_coast_max, d1_coast_mean,
      d2_area_morning_min, d2_area_morning_max, d2_area_morning_mean, d2_area_afternoon_min,
      d2_area_afternoon_max, d2_area_afternoon_mean, d2_inland_morning_min, d2_inland_morning_max,
      d2_inland_morning_mean, d2_inland_afternoon_min, d2_inland_afternoon_max,
      d2_inland_afternoon_mean, d2_coast_morning_min, d2_coast_morning_max, d2_coast_morning_mean,
      d2_coast_afternoon_min, d2_coast_afternoon_max, d2_coast_afternoon_mean, d2_area_min,
      d2_area_max, d2_area_mean, d2_inland_min, d2_inland_max, d2_inland_mean, d2_coast_min,
      d2_coast_max, d2_coast_mean, d2_windspeed_inland_morning_min, d2_windspeed_inland_morning_max,
      d2_windspeed_inland_morning_mean, d2_windspeed_inland_afternoon_min,
      d2_windspeed_inland_afternoon_max, d2_windspeed_inland_afternoon_mean,
      d2_windspeed_coast_morning_min, d2_windspeed_coast_morning_max,
      d2_windspeed_coast_morning_mean, d2_windspeed_coast_afternoon_min,
      d2_windspeed_coast_afternoon_max, d2_windspeed_coast_afternoon_mean,
      d2_windchill_inland_morning_min, d2_windchill_inland_morning_max,
      d2_windchill_inland_morning_mean, d2_windchill_inland_afternoon_min,
      d2_windchill_inland_afternoon_max, d2_windchill_inland_afternoon_mean,
      d2_windchill_coast_morning_min, d2_windchill_coast_morning_max,
      d2_windchill_coast_morning_mean, d2_windchill_coast_afternoon_min,
      d2_windchill_coast_afternoon_max, d2_windchill_coast_afternoon_mean;

  int index = 1;

  for (double d = -33.0 + random_d(); d <= 25.0; d += random_d())
  {
    d1_area_morning_min = d + random_d();
    d1_area_morning_max = d1_area_morning_min + random_i(2) + random_d();
    d1_area_morning_mean = (d1_area_morning_min + d1_area_morning_max) / 2.0;
    d1_inland_morning_min = d1_area_morning_min + random_d();
    d1_inland_morning_max = d1_inland_morning_min + random_d();
    d1_inland_morning_mean = (d1_inland_morning_min + d1_inland_morning_max) / 2.0;
    d1_coast_morning_min = d1_inland_morning_min - random_i(2) - random_d();
    d1_coast_morning_max = d1_coast_morning_min + random_i(2) + random_d();
    d1_coast_morning_mean = (d1_coast_morning_min + d1_coast_morning_max) / 2.0;
    d1_area_afternoon_min = d1_area_morning_max + random_d();
    d1_area_afternoon_max = d1_area_afternoon_min + random_i(5) + random_d();
    d1_area_afternoon_mean = (d1_area_afternoon_min + d1_area_afternoon_max) / 2.0;
    d1_inland_afternoon_min = d1_area_afternoon_min + random_d();
    d1_inland_afternoon_max = d1_inland_afternoon_min + random_d();
    d1_inland_afternoon_mean = (d1_inland_afternoon_min + d1_inland_afternoon_max) / 2.0;
    d1_coast_afternoon_min = d1_inland_afternoon_min - random_i(2) - random_d();
    d1_coast_afternoon_max = d1_coast_afternoon_min + random_i(5) + random_d();
    d1_coast_afternoon_mean = (d1_coast_afternoon_min + d1_coast_afternoon_max) / 2.0;

    d1_area_min = d1_area_morning_min;
    d1_area_max = d1_area_afternoon_max;
    d1_area_mean = (d1_area_min + d1_area_max) / 2.0;
    d1_inland_min = d1_inland_morning_min;
    d1_inland_max = d1_inland_afternoon_max;
    d1_inland_mean = (d1_inland_min + d1_inland_max) / 2.0;
    d1_coast_min = d1_coast_morning_min;
    d1_coast_max = d1_coast_afternoon_max;
    d1_coast_mean = (d1_coast_min + d1_coast_max) / 2.0;

    area_night_min = d1_area_morning_min - random_i(4) - random_d();
    area_night_max = area_night_min + random_i(2) + random_d();
    area_night_mean = (area_night_min + area_night_max) / 2.0;
    inland_night_min = d1_inland_morning_min - random_i(5) - random_d();
    inland_night_max = inland_night_min + random_i(2) + random_d();
    inland_night_mean = (inland_night_min + inland_night_max) / 2.0;
    coast_night_min = d1_coast_morning_min - random_i(4) - random_d();
    coast_night_max = coast_night_min + random_i(2) + random_d();
    coast_night_mean = (coast_night_min + coast_night_max) / 2.0;

    d1_windspeed_inland_morning_min = random_i(1, 10);
    d1_windspeed_inland_morning_max = random_i(1, 10);
    d1_windspeed_inland_morning_mean = random_i(1, 10);
    d1_windspeed_inland_afternoon_min = random_i(1, 10);
    d1_windspeed_inland_afternoon_max = random_i(1, 10);
    d1_windspeed_inland_afternoon_mean = random_i(1, 10);
    d1_windspeed_coast_morning_min = random_i(1, 10);
    d1_windspeed_coast_morning_max = random_i(1, 10);
    d1_windspeed_coast_morning_mean = random_i(1, 10);
    d1_windspeed_coast_afternoon_min = random_i(1, 10);
    d1_windspeed_coast_afternoon_max = random_i(1, 10);
    d1_windspeed_coast_afternoon_mean = random_i(1, 10);
    d1_windchill_inland_morning_min =
        get_windchill(d1_inland_morning_min, d1_windspeed_inland_morning_min);
    d1_windchill_inland_morning_max =
        get_windchill(d1_inland_morning_max, d1_windspeed_inland_morning_max);
    d1_windchill_inland_morning_mean =
        get_windchill(d1_inland_morning_mean, d1_windspeed_inland_morning_mean);
    d1_windchill_inland_afternoon_min =
        get_windchill(d1_inland_afternoon_min, d1_windspeed_inland_afternoon_min);
    d1_windchill_inland_afternoon_max =
        get_windchill(d1_inland_afternoon_max, d1_windspeed_inland_afternoon_max);
    d1_windchill_inland_afternoon_mean =
        get_windchill(d1_inland_afternoon_mean, d1_windspeed_inland_afternoon_mean);
    d1_windchill_coast_morning_min =
        get_windchill(d1_coast_morning_min, d1_windspeed_coast_morning_min);
    d1_windchill_coast_morning_max =
        get_windchill(d1_coast_morning_max, d1_windspeed_coast_morning_max);
    d1_windchill_coast_morning_mean =
        get_windchill(d1_coast_morning_mean, d1_windspeed_coast_morning_mean);
    d1_windchill_coast_afternoon_min =
        get_windchill(d1_coast_afternoon_min, d1_windspeed_coast_afternoon_min);
    d1_windchill_coast_afternoon_max =
        get_windchill(d1_coast_afternoon_max, d1_windspeed_coast_afternoon_max);
    d1_windchill_coast_afternoon_mean =
        get_windchill(d1_coast_afternoon_mean, d1_windspeed_coast_afternoon_mean);

    d2_area_morning_min =
        d1_area_morning_min +
        (random_d() >= 0.5 ? (random_i(6) + random_d()) : (-1 * (random_i(6) + random_d())));
    d2_area_morning_max = d2_area_morning_min + random_i(2) + random_d();
    d2_area_morning_mean = (d2_area_morning_min + d2_area_morning_max) / 2.0;
    d2_inland_morning_min = d2_area_morning_min + random_d();
    d2_inland_morning_max = d2_inland_morning_min + random_d();
    d2_inland_morning_mean = (d2_inland_morning_min + d2_inland_morning_max) / 2.0;
    d2_coast_morning_min = d2_inland_morning_min + random_d();
    d2_coast_morning_max = d2_coast_morning_min + random_i(2) + random_d();
    d2_coast_morning_mean = (d2_coast_morning_min + d2_coast_morning_max) / 2.0;
    d2_area_afternoon_min = d2_area_morning_max + random_i(3) + random_d();
    d2_area_afternoon_max = d2_area_afternoon_min + random_i(5) + random_d();
    d2_area_afternoon_mean = (d2_area_afternoon_min + d2_area_afternoon_max) / 2.0;
    d2_inland_afternoon_min = d2_area_afternoon_min + random_d();
    d2_inland_afternoon_max = d2_inland_afternoon_min + random_d();
    d2_inland_afternoon_mean = (d2_inland_afternoon_min + d2_inland_afternoon_max) / 2.0;
    d2_coast_afternoon_min = d2_inland_afternoon_min - random_i(2) - random_d();
    d2_coast_afternoon_max = d2_coast_afternoon_min + random_i(4) + random_d();
    d2_coast_afternoon_mean = (d2_coast_afternoon_min + d2_coast_afternoon_max) / 2.0;

    d2_area_min = d2_area_morning_min - random_d();
    d2_area_max = d2_area_afternoon_max + random_d();
    d2_area_mean = (d2_area_min + d2_area_max) / 2.0;
    d2_inland_min = d2_inland_morning_min - random_d();
    d2_inland_max = d2_inland_afternoon_max + random_d();
    d2_inland_mean = (d2_inland_min + d2_inland_max) / 2.0;
    d2_coast_min = d2_coast_morning_min - random_d();
    d2_coast_max = d2_coast_afternoon_max + random_d();
    d2_coast_mean = (d2_coast_min + d2_coast_max) / 2.0;

    d2_windspeed_inland_morning_min = random_i(2, 10);
    d2_windspeed_inland_morning_max = d2_windspeed_inland_morning_min + 1;
    d2_windspeed_inland_morning_mean =
        (d2_windspeed_inland_morning_min + d2_windspeed_inland_morning_max) / 2.0;
    d2_windspeed_inland_afternoon_min = random_i(2, 10);
    d2_windspeed_inland_afternoon_max = d2_windspeed_inland_afternoon_min + 1;
    d2_windspeed_inland_afternoon_mean =
        (d2_windspeed_inland_afternoon_min + d2_windspeed_inland_afternoon_max) / 2.0;
    d2_windspeed_coast_morning_min = d2_windspeed_inland_morning_min + 1;
    d2_windspeed_coast_morning_max = d2_windspeed_coast_morning_min + 1 + random_d();
    d2_windspeed_coast_morning_mean =
        (d2_windspeed_coast_morning_min + d2_windspeed_coast_morning_max) / 2.0;
    d2_windspeed_coast_afternoon_min = d2_windspeed_inland_afternoon_min + 1;
    d2_windspeed_coast_afternoon_max = d2_windspeed_coast_afternoon_min + 1 + random_d();
    d2_windspeed_coast_afternoon_mean =
        (d2_windspeed_coast_afternoon_min + d2_windspeed_coast_afternoon_max) / 2.0;
    d2_windchill_inland_morning_min =
        get_windchill(d2_inland_morning_min, d2_windspeed_inland_morning_min);
    d2_windchill_inland_morning_max =
        get_windchill(d2_inland_morning_max, d2_windspeed_inland_morning_max);
    d2_windchill_inland_morning_mean =
        get_windchill(d2_inland_morning_mean, d2_windspeed_inland_morning_mean);
    d2_windchill_inland_afternoon_min =
        get_windchill(d2_inland_afternoon_min, d2_windspeed_inland_afternoon_min);
    d2_windchill_inland_afternoon_max =
        get_windchill(d2_inland_afternoon_max, d2_windspeed_inland_afternoon_max);
    d2_windchill_inland_afternoon_mean =
        get_windchill(d2_inland_afternoon_mean, d2_windspeed_inland_afternoon_mean);
    d2_windchill_coast_morning_min =
        get_windchill(d2_coast_morning_min, d2_windspeed_coast_morning_min);
    d2_windchill_coast_morning_max =
        get_windchill(d2_coast_morning_max, d2_windspeed_coast_morning_max);
    d2_windchill_coast_morning_mean =
        get_windchill(d2_coast_morning_mean, d2_windspeed_coast_morning_mean);
    d2_windchill_coast_afternoon_min =
        get_windchill(d2_coast_afternoon_min, d2_windspeed_coast_afternoon_min);
    d2_windchill_coast_afternoon_max =
        get_windchill(d2_coast_afternoon_max, d2_windspeed_coast_afternoon_max);
    d2_windchill_coast_afternoon_mean =
        get_windchill(d2_coast_afternoon_mean, d2_windspeed_coast_afternoon_mean);

    sprintf(d1_area_morning_min_str, "%.02f,0", d1_area_morning_min);
    sprintf(d1_area_morning_max_str, "%.02f,0", d1_area_morning_max);
    sprintf(d1_area_morning_mean_str, "%.02f,0", d1_area_morning_mean);
    sprintf(d1_area_afternoon_min_str, "%.02f,0", d1_area_afternoon_min);
    sprintf(d1_area_afternoon_max_str, "%.02f,0", d1_area_afternoon_max);
    sprintf(d1_area_afternoon_mean_str, "%.02f,0", d1_area_afternoon_mean);
    sprintf(d1_inland_morning_min_str, "%.02f,0", d1_inland_morning_min);
    sprintf(d1_inland_morning_max_str, "%.02f,0", d1_inland_morning_max);
    sprintf(d1_inland_morning_mean_str, "%.02f,0", d1_inland_morning_mean);
    sprintf(d1_inland_afternoon_min_str, "%.02f,0", d1_inland_afternoon_min);
    sprintf(d1_inland_afternoon_max_str, "%.02f,0", d1_inland_afternoon_max);
    sprintf(d1_inland_afternoon_mean_str, "%.02f,0", d1_inland_afternoon_mean);
    sprintf(d1_coast_morning_min_str, "%.02f,0", d1_coast_morning_min);
    sprintf(d1_coast_morning_max_str, "%.02f,0", d1_coast_morning_max);
    sprintf(d1_coast_morning_mean_str, "%.02f,0", d1_coast_morning_mean);
    sprintf(d1_coast_afternoon_min_str, "%.02f,0", d1_coast_afternoon_min);
    sprintf(d1_coast_afternoon_max_str, "%.02f,0", d1_coast_afternoon_max);
    sprintf(d1_coast_afternoon_mean_str, "%.02f,0", d1_coast_afternoon_mean);
    sprintf(area_night_min_str, "%.02f,0", area_night_min);
    sprintf(area_night_max_str, "%.02f,0", area_night_max);
    sprintf(area_night_mean_str, "%.02f,0", area_night_mean);
    sprintf(inland_night_min_str, "%.02f,0", inland_night_min);
    sprintf(inland_night_max_str, "%.02f,0", inland_night_max);
    sprintf(inland_night_mean_str, "%.02f,0", inland_night_mean);
    sprintf(coast_night_min_str, "%.02f,0", coast_night_min);
    sprintf(coast_night_max_str, "%.02f,0", coast_night_max);
    sprintf(coast_night_mean_str, "%.02f,0", coast_night_mean);
    sprintf(d1_area_min_str, "%.02f,0", d1_area_min);
    sprintf(d1_area_max_str, "%.02f,0", d1_area_max);
    sprintf(d1_area_mean_str, "%.02f,0", d1_area_mean);
    sprintf(d1_inland_min_str, "%.02f,0", d1_inland_min);
    sprintf(d1_inland_max_str, "%.02f,0", d1_inland_max);
    sprintf(d1_inland_mean_str, "%.02f,0", d1_inland_mean);
    sprintf(d1_coast_min_str, "%.02f,0", d1_coast_min);
    sprintf(d1_coast_max_str, "%.02f,0", d1_coast_max);
    sprintf(d1_coast_mean_str, "%.02f,0", d1_coast_mean);
    sprintf(d1_windspeed_inland_morning_min_str, "%.02f,0", d1_windspeed_inland_morning_min);
    sprintf(d1_windspeed_inland_morning_max_str, "%.02f,0", d1_windspeed_inland_morning_max);
    sprintf(d1_windspeed_inland_morning_mean_str, "%.02f,0", d1_windspeed_inland_morning_mean);
    sprintf(d1_windspeed_inland_afternoon_min_str, "%.02f,0", d1_windspeed_inland_afternoon_min);
    sprintf(d1_windspeed_inland_afternoon_max_str, "%.02f,0", d1_windspeed_inland_afternoon_max);
    sprintf(d1_windspeed_inland_afternoon_mean_str, "%.02f,0", d1_windspeed_inland_afternoon_mean);
    sprintf(d1_windspeed_coast_morning_min_str, "%.02f,0", d1_windspeed_coast_morning_min);
    sprintf(d1_windspeed_coast_morning_max_str, "%.02f,0", d1_windspeed_coast_morning_max);
    sprintf(d1_windspeed_coast_morning_mean_str, "%.02f,0", d1_windspeed_coast_morning_mean);
    sprintf(d1_windspeed_coast_afternoon_min_str, "%.02f,0", d1_windspeed_coast_afternoon_min);
    sprintf(d1_windspeed_coast_afternoon_max_str, "%.02f,0", d1_windspeed_coast_afternoon_max);
    sprintf(d1_windspeed_coast_afternoon_mean_str, "%.02f,0", d1_windspeed_coast_afternoon_mean);
    sprintf(d1_windchill_inland_morning_min_str, "%.02f,0", d1_windchill_inland_morning_min);
    sprintf(d1_windchill_inland_morning_max_str, "%.02f,0", d1_windchill_inland_morning_max);
    sprintf(d1_windchill_inland_morning_mean_str, "%.02f,0", d1_windchill_inland_morning_mean);
    sprintf(d1_windchill_inland_afternoon_min_str, "%.02f,0", d1_windchill_inland_afternoon_min);
    sprintf(d1_windchill_inland_afternoon_max_str, "%.02f,0", d1_windchill_inland_afternoon_max);
    sprintf(d1_windchill_inland_afternoon_mean_str, "%.02f,0", d1_windchill_inland_afternoon_mean);
    sprintf(d1_windchill_coast_morning_min_str, "%.02f,0", d1_windchill_coast_morning_min);
    sprintf(d1_windchill_coast_morning_max_str, "%.02f,0", d1_windchill_coast_morning_max);
    sprintf(d1_windchill_coast_morning_mean_str, "%.02f,0", d1_windchill_coast_morning_mean);
    sprintf(d1_windchill_coast_afternoon_min_str, "%.02f,0", d1_windchill_coast_afternoon_min);
    sprintf(d1_windchill_coast_afternoon_max_str, "%.02f,0", d1_windchill_coast_afternoon_max);
    sprintf(d1_windchill_coast_afternoon_mean_str, "%.02f,0", d1_windchill_coast_afternoon_mean);
    sprintf(d2_area_morning_min_str, "%.02f,0", d2_area_morning_min);
    sprintf(d2_area_morning_max_str, "%.02f,0", d2_area_morning_max);
    sprintf(d2_area_morning_mean_str, "%.02f,0", d2_area_morning_mean);
    sprintf(d2_area_afternoon_min_str, "%.02f,0", d2_area_afternoon_min);
    sprintf(d2_area_afternoon_max_str, "%.02f,0", d2_area_afternoon_max);
    sprintf(d2_area_afternoon_mean_str, "%.02f,0", d2_area_afternoon_mean);
    sprintf(d2_inland_morning_min_str, "%.02f,0", d2_inland_morning_min);
    sprintf(d2_inland_morning_max_str, "%.02f,0", d2_inland_morning_max);
    sprintf(d2_inland_morning_mean_str, "%.02f,0", d2_inland_morning_mean);
    sprintf(d2_inland_afternoon_min_str, "%.02f,0", d2_inland_afternoon_min);
    sprintf(d2_inland_afternoon_max_str, "%.02f,0", d2_inland_afternoon_max);
    sprintf(d2_inland_afternoon_mean_str, "%.02f,0", d2_inland_afternoon_mean);
    sprintf(d2_coast_morning_min_str, "%.02f,0", d2_coast_morning_min);
    sprintf(d2_coast_morning_max_str, "%.02f,0", d2_coast_morning_max);
    sprintf(d2_coast_morning_mean_str, "%.02f,0", d2_coast_morning_mean);
    sprintf(d2_coast_afternoon_min_str, "%.02f,0", d2_coast_afternoon_min);
    sprintf(d2_coast_afternoon_max_str, "%.02f,0", d2_coast_afternoon_max);
    sprintf(d2_coast_afternoon_mean_str, "%.02f,0", d2_coast_afternoon_mean);
    sprintf(d2_area_min_str, "%.02f,0", d2_area_min);
    sprintf(d2_area_max_str, "%.02f,0", d2_area_max);
    sprintf(d2_area_mean_str, "%.02f,0", d2_area_mean);
    sprintf(d2_inland_min_str, "%.02f,0", d2_inland_min);
    sprintf(d2_inland_max_str, "%.02f,0", d2_inland_max);
    sprintf(d2_inland_mean_str, "%.02f,0", d2_inland_mean);
    sprintf(d2_coast_min_str, "%.02f,0", d2_coast_min);
    sprintf(d2_coast_max_str, "%.02f,0", d2_coast_max);
    sprintf(d2_coast_mean_str, "%.02f,0", d2_coast_mean);
    sprintf(d2_windspeed_inland_morning_min_str, "%.02f,0", d2_windspeed_inland_morning_min);
    sprintf(d2_windspeed_inland_morning_max_str, "%.02f,0", d2_windspeed_inland_morning_max);
    sprintf(d2_windspeed_inland_morning_mean_str, "%.02f,0", d2_windspeed_inland_morning_mean);
    sprintf(d2_windspeed_inland_afternoon_min_str, "%.02f,0", d2_windspeed_inland_afternoon_min);
    sprintf(d2_windspeed_inland_afternoon_max_str, "%.02f,0", d2_windspeed_inland_afternoon_max);
    sprintf(d2_windspeed_inland_afternoon_mean_str, "%.02f,0", d2_windspeed_inland_afternoon_mean);
    sprintf(d2_windspeed_coast_morning_min_str, "%.02f,0", d2_windspeed_coast_morning_min);
    sprintf(d2_windspeed_coast_morning_max_str, "%.02f,0", d2_windspeed_coast_morning_max);
    sprintf(d2_windspeed_coast_morning_mean_str, "%.02f,0", d2_windspeed_coast_morning_mean);
    sprintf(d2_windspeed_coast_afternoon_min_str, "%.02f,0", d2_windspeed_coast_afternoon_min);
    sprintf(d2_windspeed_coast_afternoon_max_str, "%.02f,0", d2_windspeed_coast_afternoon_max);
    sprintf(d2_windspeed_coast_afternoon_mean_str, "%.02f,0", d2_windspeed_coast_afternoon_mean);
    sprintf(d2_windchill_inland_morning_min_str, "%.02f,0", d2_windchill_inland_morning_min);
    sprintf(d2_windchill_inland_morning_max_str, "%.02f,0", d2_windchill_inland_morning_max);
    sprintf(d2_windchill_inland_morning_mean_str, "%.02f,0", d2_windchill_inland_morning_mean);
    sprintf(d2_windchill_inland_afternoon_min_str, "%.02f,0", d2_windchill_inland_afternoon_min);
    sprintf(d2_windchill_inland_afternoon_max_str, "%.02f,0", d2_windchill_inland_afternoon_max);
    sprintf(d2_windchill_inland_afternoon_mean_str, "%.02f,0", d2_windchill_inland_afternoon_mean);
    sprintf(d2_windchill_coast_morning_min_str, "%.02f,0", d2_windchill_coast_morning_min);
    sprintf(d2_windchill_coast_morning_max_str, "%.02f,0", d2_windchill_coast_morning_max);
    sprintf(d2_windchill_coast_morning_mean_str, "%.02f,0", d2_windchill_coast_morning_mean);
    sprintf(d2_windchill_coast_afternoon_min_str, "%.02f,0", d2_windchill_coast_afternoon_min);
    sprintf(d2_windchill_coast_afternoon_max_str, "%.02f,0", d2_windchill_coast_afternoon_max);
    sprintf(d2_windchill_coast_afternoon_mean_str, "%.02f,0", d2_windchill_coast_afternoon_mean);

    TemperatureStoryTestParam* param = 0;

    if (testCaseType == TEMPERATURE_ANOMALY)
    {
      param = new TemperatureAnomalyTestParam(d1_inland_afternoon_min_str,
                                              d1_inland_afternoon_max_str,
                                              d1_inland_afternoon_mean_str,

                                              d1_coast_afternoon_min_str,
                                              d1_coast_afternoon_max_str,
                                              d1_coast_afternoon_mean_str,

                                              d1_area_afternoon_min_str,
                                              d1_area_afternoon_max_str,
                                              d1_area_afternoon_mean_str,

                                              d2_inland_afternoon_min_str,
                                              d2_inland_afternoon_max_str,
                                              d2_inland_afternoon_mean_str,

                                              d2_coast_afternoon_min_str,
                                              d2_coast_afternoon_max_str,
                                              d2_coast_afternoon_mean_str,

                                              d2_area_afternoon_min_str,
                                              d2_area_afternoon_max_str,
                                              d2_area_afternoon_mean_str,
                                              "");
    }
    else
    {
      param = new Max36HoursTestParam(d1_inland_min_str,
                                      d1_inland_max_str,
                                      d1_inland_mean_str,

                                      d1_coast_min_str,
                                      d1_coast_max_str,
                                      d1_coast_mean_str,

                                      d1_area_min_str,
                                      d1_area_max_str,
                                      d1_area_mean_str,

                                      d1_inland_morning_min_str,
                                      d1_inland_morning_max_str,
                                      d1_inland_morning_mean_str,

                                      d1_coast_morning_min_str,
                                      d1_coast_morning_max_str,
                                      d1_coast_morning_mean_str,

                                      d1_area_morning_min_str,
                                      d1_area_morning_max_str,
                                      d1_area_morning_mean_str,

                                      d1_inland_afternoon_min_str,
                                      d1_inland_afternoon_max_str,
                                      d1_inland_afternoon_mean_str,

                                      d1_coast_afternoon_min_str,
                                      d1_coast_afternoon_max_str,
                                      d1_coast_afternoon_mean_str,

                                      d1_area_afternoon_min_str,
                                      d1_area_afternoon_max_str,
                                      d1_area_afternoon_mean_str,

                                      inland_night_min_str,
                                      inland_night_max_str,
                                      inland_night_mean_str,

                                      coast_night_min_str,
                                      coast_night_max_str,
                                      coast_night_mean_str,

                                      area_night_min_str,
                                      area_night_max_str,
                                      area_night_mean_str,

                                      d2_inland_min_str,
                                      d2_inland_max_str,
                                      d2_inland_mean_str,

                                      d2_coast_min_str,
                                      d2_coast_max_str,
                                      d2_coast_mean_str,

                                      d2_area_min_str,
                                      d2_area_max_str,
                                      d2_area_mean_str,

                                      d2_inland_morning_min_str,
                                      d2_inland_morning_max_str,
                                      d2_inland_morning_mean_str,

                                      d2_coast_morning_min_str,
                                      d2_coast_morning_max_str,
                                      d2_coast_morning_mean_str,

                                      d2_area_morning_min_str,
                                      d2_area_morning_max_str,
                                      d2_area_morning_mean_str,

                                      d2_inland_afternoon_min_str,
                                      d2_inland_afternoon_max_str,
                                      d2_inland_afternoon_mean_str,

                                      d2_coast_afternoon_min_str,
                                      d2_coast_afternoon_max_str,
                                      d2_coast_afternoon_mean_str,

                                      d2_area_afternoon_min_str,
                                      d2_area_afternoon_max_str,
                                      d2_area_afternoon_mean_str,
                                      "");
    }

    testCases.insert(make_pair(index++, param));
  }
}

void create_max36hours_testcases(TestCaseContainer& testCases,
                                 const string& language,
                                 const bool& isWinter)
{
  int i = 0;

  if (isWinter)
  {
    if (language == "fi")
    {
      // #1
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-31.63,0",
                        "-29.02,0",
                        "-30.32,0",  // inland day1: min, max, mean
                        "-32.68,0",
                        "-27.06,0",
                        "-29.87,0",  // coast day1: min, max, mean
                        "-31.95,0",
                        "-27.76,0",
                        "-29.85,0",  // area day1: min, max, mean
                        "-31.63,0",
                        "-30.74,0",
                        "-31.18,0",  // inland day1 morning: min, max, mean
                        "-32.68,0",
                        "-32.27,0",
                        "-32.48,0",  // coast day1 morning: min, max, mean
                        "-31.95,0",
                        "-30.74,0",
                        "-31.34,0",  // area day1 morning: min, max, mean
                        "-29.58,0",
                        "-29.02,0",
                        "-29.30,0",  // inland day1 afternoon: min, max, mean
                        "-30.16,0",
                        "-27.06,0",
                        "-28.61,0",  // coast day1 afternoon: min, max, mean
                        "-29.97,0",
                        "-27.76,0",
                        "-28.86,0",  // area day1 afternoon: min, max, mean
                        "-34.86,0",
                        "-34.75,0",
                        "-34.80,0",  // inland night: min, max, mean
                        "-36.16,0",
                        "-35.83,0",
                        "-36.00,0",  // coast night: min, max, mean
                        "-32.22,0",
                        "-32.22,0",
                        "-32.22,0",  // area night: min, max, mean
                        "-33.26,0",
                        "-29.60,0",
                        "-31.43,0",  // inland day2: min, max, mean
                        "-32.75,0",
                        "-27.61,0",
                        "-30.18,0",  // coast day2: min, max, mean
                        "-33.39,0",
                        "-25.90,0",
                        "-29.65,0",  // area day2: min, max, mean
                        "-32.40,0",
                        "-31.65,0",
                        "-32.03,0",  // inland day2 morning: min, max, mean
                        "-32.26,0",
                        "-32.08,0",
                        "-32.17,0",  // coast day2 morning: min, max, mean
                        "-32.57,0",
                        "-31.60,0",
                        "-32.09,0",  // area day2 morning: min, max, mean
                        "-30.80,0",
                        "-30.60,0",
                        "-30.70,0",  // inland day2 afternoon: min, max, mean
                        "-31.77,0",
                        "-28.31,0",
                        "-30.04,0",  // coast day2 afternoon: min, max, mean
                        "-31.26,0",
                        "-26.47,0",
                        "-28.86,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on noin -30 astetta, huomenna suunnilleen sama."  // the story
                        )));
      // #2
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-31.45,0",
                        "-29.48,0",
                        "-30.46,0",  // inland day1: min, max, mean
                        "-33.28,0",
                        "-27.41,0",
                        "-30.34,0",  // coast day1: min, max, mean
                        "-31.52,0",
                        "-26.63,0",
                        "-29.07,0",  // area day1: min, max, mean
                        "-31.45,0",
                        "-30.87,0",
                        "-31.16,0",  // inland day1 morning: min, max, mean
                        "-33.28,0",
                        "-32.70,0",
                        "-32.99,0",  // coast day1 morning: min, max, mean
                        "-31.52,0",
                        "-30.55,0",
                        "-31.04,0",  // area day1 morning: min, max, mean
                        "-30.14,0",
                        "-29.48,0",
                        "-29.81,0",  // inland day1 afternoon: min, max, mean
                        "-30.75,0",
                        "-27.41,0",
                        "-29.08,0",  // coast day1 afternoon: min, max, mean
                        "-30.55,0",
                        "-26.63,0",
                        "-28.59,0",  // area day1 afternoon: min, max, mean
                        "-34.61,0",
                        "-32.66,0",
                        "-33.63,0",  // inland night: min, max, mean
                        "-36.78,0",
                        "-36.18,0",
                        "-36.48,0",  // coast night: min, max, mean
                        "-34.91,0",
                        "-33.18,0",
                        "-34.05,0",  // area night: min, max, mean
                        "-36.11,0",
                        "-34.10,0",
                        "-35.10,0",  // inland day2: min, max, mean
                        "-35.66,0",
                        "-33.45,0",
                        "-34.55,0",  // coast day2: min, max, mean
                        "-37.37,0",
                        "-30.68,0",
                        "-34.02,0",  // area day2: min, max, mean
                        "-35.60,0",
                        "-35.43,0",
                        "-35.51,0",  // inland day2 morning: min, max, mean
                        "-35.18,0",
                        "-34.85,0",
                        "-35.02,0",  // coast day2 morning: min, max, mean
                        "-36.58,0",
                        "-36.32,0",
                        "-36.45,0",  // area day2 morning: min, max, mean
                        "-35.15,0",
                        "-34.28,0",
                        "-34.71,0",  // inland day2 afternoon: min, max, mean
                        "-35.61,0",
                        "-33.92,0",
                        "-34.76,0",  // coast day2 afternoon: min, max, mean
                        "-35.71,0",
                        "-31.36,0",
                        "-33.53,0",  // area day2 afternoon: min, max, mean
                        "Päivälämpötila on -25...-30 astetta, huomenna pakkanen on hieman "
                        "kireämpää. Yöllä pakkasta on vajaat 35 astetta."  // the story
                        )));
      // #3
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-30.82,0",
              "-29.17,0",
              "-29.99,0",  // inland day1: min, max, mean
              "-31.26,0",
              "-27.29,0",
              "-29.27,0",  // coast day1: min, max, mean
              "-31.09,0",
              "-29.45,0",
              "-30.27,0",  // area day1: min, max, mean
              "-30.82,0",
              "-30.29,0",
              "-30.55,0",  // inland day1 morning: min, max, mean
              "-31.26,0",
              "-29.46,0",
              "-30.36,0",  // coast day1 morning: min, max, mean
              "-31.09,0",
              "-30.64,0",
              "-30.87,0",  // area day1 morning: min, max, mean
              "-29.60,0",
              "-29.17,0",
              "-29.38,0",  // inland day1 afternoon: min, max, mean
              "-29.73,0",
              "-27.29,0",
              "-28.51,0",  // coast day1 afternoon: min, max, mean
              "-29.86,0",
              "-29.45,0",
              "-29.66,0",  // area day1 afternoon: min, max, mean
              "-32.23,0",
              "-30.27,0",
              "-31.25,0",  // inland night: min, max, mean
              "-35.02,0",
              "-34.56,0",
              "-34.79,0",  // coast night: min, max, mean
              "-33.07,0",
              "-31.80,0",
              "-32.43,0",  // area night: min, max, mean
              "-32.25,0",
              "-29.98,0",
              "-31.12,0",  // inland day2: min, max, mean
              "-31.50,0",
              "-30.68,0",
              "-31.09,0",  // coast day2: min, max, mean
              "-33.61,0",
              "-27.78,0",
              "-30.69,0",  // area day2: min, max, mean
              "-31.81,0",
              "-31.14,0",
              "-31.48,0",  // inland day2 morning: min, max, mean
              "-30.91,0",
              "-29.01,0",
              "-29.96,0",  // coast day2 morning: min, max, mean
              "-32.81,0",
              "-32.50,0",
              "-32.66,0",  // area day2 morning: min, max, mean
              "-30.61,0",
              "-30.47,0",
              "-30.54,0",  // inland day2 afternoon: min, max, mean
              "-32.00,0",
              "-31.16,0",
              "-31.58,0",  // coast day2 afternoon: min, max, mean
              "-31.04,0",
              "-28.11,0",
              "-29.57,0",  // area day2 afternoon: min, max, mean
              "Lämpötila on -30 asteen tuntumassa, huomenna suunnilleen sama."  // the story
              )));
      // #4
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-30.12,0",
              "-29.10,0",
              "-29.61,0",  // inland day1: min, max, mean
              "-31.57,0",
              "-29.86,0",
              "-30.71,0",  // coast day1: min, max, mean
              "-30.90,0",
              "-28.54,0",
              "-29.72,0",  // area day1: min, max, mean
              "-30.12,0",
              "-30.11,0",
              "-30.12,0",  // inland day1 morning: min, max, mean
              "-31.57,0",
              "-29.81,0",
              "-30.69,0",  // coast day1 morning: min, max, mean
              "-30.90,0",
              "-30.11,0",
              "-30.51,0",  // area day1 morning: min, max, mean
              "-29.11,0",
              "-29.10,0",
              "-29.11,0",  // inland day1 afternoon: min, max, mean
              "-30.90,0",
              "-29.86,0",
              "-30.38,0",  // coast day1 afternoon: min, max, mean
              "-29.77,0",
              "-28.54,0",
              "-29.15,0",  // area day1 afternoon: min, max, mean
              "-34.46,0",
              "-33.02,0",
              "-33.74,0",  // inland night: min, max, mean
              "-32.68,0",
              "-32.18,0",
              "-32.43,0",  // coast night: min, max, mean
              "-32.43,0",
              "-30.71,0",
              "-31.57,0",  // area night: min, max, mean
              "-26.87,0",
              "-24.61,0",
              "-25.74,0",  // inland day2: min, max, mean
              "-26.60,0",
              "-25.89,0",
              "-26.25,0",  // coast day2: min, max, mean
              "-28.59,0",
              "-22.93,0",
              "-25.76,0",  // area day2: min, max, mean
              "-26.84,0",
              "-26.37,0",
              "-26.61,0",  // inland day2 morning: min, max, mean
              "-26.10,0",
              "-25.78,0",
              "-25.94,0",  // coast day2 morning: min, max, mean
              "-27.72,0",
              "-27.37,0",
              "-27.54,0",  // area day2 morning: min, max, mean
              "-26.20,0",
              "-25.58,0",
              "-25.89,0",  // inland day2 afternoon: min, max, mean
              "-26.75,0",
              "-26.69,0",
              "-26.72,0",  // coast day2 afternoon: min, max, mean
              "-27.00,0",
              "-23.23,0",
              "-25.11,0",  // area day2 afternoon: min, max, mean
              "Lämpötila on -30 asteen tuntumassa, huomenna noin -25 astetta."  // the story
              )));
      // #5
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-30.09,0",
                                  "-28.11,0",
                                  "-29.10,0",  // inland day1: min, max, mean
                                  "-32.05,0",
                                  "-26.95,0",
                                  "-29.50,0",  // coast day1: min, max, mean
                                  "-30.59,0",
                                  "-24.70,0",
                                  "-27.65,0",  // area day1: min, max, mean
                                  "-30.09,0",
                                  "-29.78,0",
                                  "-29.93,0",  // inland day1 morning: min, max, mean
                                  "-32.05,0",
                                  "-30.79,0",
                                  "-31.42,0",  // coast day1 morning: min, max, mean
                                  "-30.59,0",
                                  "-29.60,0",
                                  "-30.10,0",  // area day1 morning: min, max, mean
                                  "-28.40,0",
                                  "-28.11,0",
                                  "-28.26,0",  // inland day1 afternoon: min, max, mean
                                  "-29.94,0",
                                  "-26.95,0",
                                  "-28.45,0",  // coast day1 afternoon: min, max, mean
                                  "-29.33,0",
                                  "-24.70,0",
                                  "-27.02,0",  // area day1 afternoon: min, max, mean
                                  "-34.93,0",
                                  "-34.86,0",
                                  "-34.90,0",  // inland night: min, max, mean
                                  "-32.12,0",
                                  "-30.35,0",
                                  "-31.23,0",  // coast night: min, max, mean
                                  "-33.82,0",
                                  "-32.87,0",
                                  "-33.35,0",  // area night: min, max, mean
                                  "-27.07,0",
                                  "-24.18,0",
                                  "-25.63,0",  // inland day2: min, max, mean
                                  "-26.91,0",
                                  "-23.97,0",
                                  "-25.44,0",  // coast day2: min, max, mean
                                  "-27.19,0",
                                  "-20.10,0",
                                  "-23.64,0",  // area day2: min, max, mean
                                  "-26.58,0",
                                  "-26.56,0",
                                  "-26.57,0",  // inland day2 morning: min, max, mean
                                  "-26.26,0",
                                  "-24.37,0",
                                  "-25.32,0",  // coast day2 morning: min, max, mean
                                  "-26.67,0",
                                  "-26.00,0",
                                  "-26.33,0",  // area day2 morning: min, max, mean
                                  "-24.76,0",
                                  "-24.67,0",
                                  "-24.71,0",  // inland day2 afternoon: min, max, mean
                                  "-25.80,0",
                                  "-24.16,0",
                                  "-24.98,0",  // coast day2 afternoon: min, max, mean
                                  "-25.41,0",
                                  "-20.69,0",
                                  "-23.05,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -25...-30 astetta, huomenna -20...-25 "
                                  "astetta. Yölämpötila on noin -35 astetta."  // the story
                                  )));
      // #6
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam("-28.73,0",
                                            "-26.32,0",
                                            "-27.53,0",  // inland day1: min, max, mean
                                            "-29.55,0",
                                            "-23.60,0",
                                            "-26.57,0",  // coast day1: min, max, mean
                                            "-29.52,0",
                                            "-23.21,0",
                                            "-26.37,0",  // area day1: min, max, mean
                                            "-28.73,0",
                                            "-28.30,0",
                                            "-28.51,0",  // inland day1 morning: min, max, mean
                                            "-29.55,0",
                                            "-27.84,0",
                                            "-28.70,0",  // coast day1 morning: min, max, mean
                                            "-29.52,0",
                                            "-28.22,0",
                                            "-28.87,0",  // area day1 morning: min, max, mean
                                            "-27.06,0",
                                            "-26.32,0",
                                            "-26.69,0",  // inland day1 afternoon: min, max, mean
                                            "-28.59,0",
                                            "-23.60,0",
                                            "-26.09,0",  // coast day1 afternoon: min, max, mean
                                            "-27.51,0",
                                            "-23.21,0",
                                            "-25.36,0",  // area day1 afternoon: min, max, mean
                                            "-29.73,0",
                                            "-28.89,0",
                                            "-29.31,0",  // inland night: min, max, mean
                                            "-31.94,0",
                                            "-30.90,0",
                                            "-31.42,0",  // coast night: min, max, mean
                                            "-29.69,0",
                                            "-28.71,0",
                                            "-29.20,0",  // area night: min, max, mean
                                            "-34.31,0",
                                            "-29.63,0",
                                            "-31.97,0",  // inland day2: min, max, mean
                                            "-33.97,0",
                                            "-30.12,0",
                                            "-32.05,0",  // coast day2: min, max, mean
                                            "-34.93,0",
                                            "-30.87,0",
                                            "-32.90,0",  // area day2: min, max, mean
                                            "-34.00,0",
                                            "-33.58,0",
                                            "-33.79,0",  // inland day2 morning: min, max, mean
                                            "-33.55,0",
                                            "-31.61,0",
                                            "-32.58,0",  // coast day2 morning: min, max, mean
                                            "-34.25,0",
                                            "-33.19,0",
                                            "-33.72,0",  // area day2 morning: min, max, mean
                                            "-30.21,0",
                                            "-29.86,0",
                                            "-30.04,0",  // inland day2 afternoon: min, max, mean
                                            "-31.12,0",
                                            "-30.90,0",
                                            "-31.01,0",  // coast day2 afternoon: min, max, mean
                                            "-31.12,0",
                                            "-31.06,0",
                                            "-31.09,0",  // area day2 afternoon: min, max, mean
                                            "Lämpötila on -25 asteen tienoilla, huomenna pakkasta "
                                            "on vähän yli 30 astetta."  // the story
                                            )));
      // #7
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-28.61,0",
              "-28.70,0",
              "-28.65,0",  // inland day1: min, max, mean
              "-28.89,0",
              "-29.81,0",
              "-29.35,0",  // coast day1: min, max, mean
              "-29.47,0",
              "-26.27,0",
              "-27.87,0",  // area day1: min, max, mean
              "-28.61,0",
              "-28.30,0",
              "-28.45,0",  // inland day1 morning: min, max, mean
              "-28.89,0",
              "-27.11,0",
              "-28.00,0",  // coast day1 morning: min, max, mean
              "-29.47,0",
              "-29.35,0",
              "-29.41,0",  // area day1 morning: min, max, mean
              "-28.96,0",
              "-28.70,0",
              "-28.83,0",  // inland day1 afternoon: min, max, mean
              "-30.48,0",
              "-29.81,0",
              "-30.15,0",  // coast day1 afternoon: min, max, mean
              "-29.13,0",
              "-26.27,0",
              "-27.70,0",  // area day1 afternoon: min, max, mean
              "-31.18,0",
              "-31.17,0",
              "-31.17,0",  // inland night: min, max, mean
              "-31.81,0",
              "-29.91,0",
              "-30.86,0",  // coast night: min, max, mean
              "-30.40,0",
              "-29.30,0",
              "-29.85,0",  // area night: min, max, mean
              "-32.40,0",
              "-29.37,0",
              "-30.88,0",  // inland day2: min, max, mean
              "-31.83,0",
              "-30.44,0",
              "-31.13,0",  // coast day2: min, max, mean
              "-33.07,0",
              "-31.11,0",
              "-32.09,0",  // area day2: min, max, mean
              "-31.76,0",
              "-30.90,0",
              "-31.33,0",  // inland day2 morning: min, max, mean
              "-31.38,0",
              "-31.18,0",
              "-31.28,0",  // coast day2 morning: min, max, mean
              "-32.13,0",
              "-31.67,0",
              "-31.90,0",  // area day2 morning: min, max, mean
              "-30.55,0",
              "-29.58,0",
              "-30.06,0",  // inland day2 afternoon: min, max, mean
              "-32.09,0",
              "-30.87,0",
              "-31.48,0",  // coast day2 afternoon: min, max, mean
              "-31.12,0",
              "-31.12,0",
              "-31.12,0",  // area day2 afternoon: min, max, mean
              "Lämpötila on -25...-30 astetta, huomenna pakkanen on hieman kireämpää."  // the story
              )));
      // #8
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-28.78,0",
                        "-26.74,0",
                        "-27.76,0",  // inland day1: min, max, mean
                        "-29.26,0",
                        "-27.89,0",
                        "-28.58,0",  // coast day1: min, max, mean
                        "-29.40,0",
                        "-26.03,0",
                        "-27.72,0",  // area day1: min, max, mean
                        "-28.78,0",
                        "-27.85,0",
                        "-28.32,0",  // inland day1 morning: min, max, mean
                        "-29.26,0",
                        "-28.85,0",
                        "-29.06,0",  // coast day1 morning: min, max, mean
                        "-29.40,0",
                        "-28.68,0",
                        "-29.04,0",  // area day1 morning: min, max, mean
                        "-27.52,0",
                        "-26.74,0",
                        "-27.13,0",  // inland day1 afternoon: min, max, mean
                        "-28.96,0",
                        "-27.89,0",
                        "-28.43,0",  // coast day1 afternoon: min, max, mean
                        "-27.99,0",
                        "-26.03,0",
                        "-27.01,0",  // area day1 afternoon: min, max, mean
                        "-32.00,0",
                        "-30.01,0",
                        "-31.01,0",  // inland night: min, max, mean
                        "-29.68,0",
                        "-29.17,0",
                        "-29.42,0",  // coast night: min, max, mean
                        "-31.33,0",
                        "-31.02,0",
                        "-31.17,0",  // area night: min, max, mean
                        "-24.77,0",
                        "-18.67,0",
                        "-21.72,0",  // inland day2: min, max, mean
                        "-24.62,0",
                        "-17.27,0",
                        "-20.94,0",  // coast day2: min, max, mean
                        "-25.02,0",
                        "-18.06,0",
                        "-21.54,0",  // area day2: min, max, mean
                        "-24.17,0",
                        "-23.89,0",
                        "-24.03,0",  // inland day2 morning: min, max, mean
                        "-23.73,0",
                        "-23.43,0",
                        "-23.58,0",  // coast day2 morning: min, max, mean
                        "-24.64,0",
                        "-23.50,0",
                        "-24.07,0",  // area day2 morning: min, max, mean
                        "-19.60,0",
                        "-19.10,0",
                        "-19.35,0",  // inland day2 afternoon: min, max, mean
                        "-20.55,0",
                        "-18.16,0",
                        "-19.35,0",  // coast day2 afternoon: min, max, mean
                        "-20.55,0",
                        "-19.04,0",
                        "-19.80,0",  // area day2 afternoon: min, max, mean
                        "Päivällä pakkasta on vähän yli 25 astetta, huomenna lähellä -20 astetta. "
                        "Yöllä pakkasta on vähän yli 30 astetta."  // the story
                        )));
      // #9
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-29.01,0",
                        "-26.28,0",
                        "-27.64,0",  // inland day1: min, max, mean
                        "-29.77,0",
                        "-24.54,0",
                        "-27.16,0",  // coast day1: min, max, mean
                        "-29.49,0",
                        "-25.94,0",
                        "-27.71,0",  // area day1: min, max, mean
                        "-29.01,0",
                        "-28.98,0",
                        "-28.99,0",  // inland day1 morning: min, max, mean
                        "-29.77,0",
                        "-29.08,0",
                        "-29.42,0",  // coast day1 morning: min, max, mean
                        "-29.49,0",
                        "-27.65,0",
                        "-28.57,0",  // area day1 morning: min, max, mean
                        "-27.01,0",
                        "-26.28,0",
                        "-26.64,0",  // inland day1 afternoon: min, max, mean
                        "-27.09,0",
                        "-24.54,0",
                        "-25.82,0",  // coast day1 afternoon: min, max, mean
                        "-27.59,0",
                        "-25.94,0",
                        "-26.76,0",  // area day1 afternoon: min, max, mean
                        "-33.37,0",
                        "-31.84,0",
                        "-32.61,0",  // inland night: min, max, mean
                        "-29.83,0",
                        "-28.29,0",
                        "-29.06,0",  // coast night: min, max, mean
                        "-32.53,0",
                        "-32.46,0",
                        "-32.50,0",  // area night: min, max, mean
                        "-26.66,0",
                        "-24.70,0",
                        "-25.68,0",  // inland day2: min, max, mean
                        "-26.41,0",
                        "-21.55,0",
                        "-23.98,0",  // coast day2: min, max, mean
                        "-26.86,0",
                        "-24.17,0",
                        "-25.52,0",  // area day2: min, max, mean
                        "-26.03,0",
                        "-25.27,0",
                        "-25.65,0",  // inland day2 morning: min, max, mean
                        "-25.59,0",
                        "-23.90,0",
                        "-24.74,0",  // coast day2 morning: min, max, mean
                        "-26.58,0",
                        "-26.23,0",
                        "-26.41,0",  // area day2 morning: min, max, mean
                        "-25.24,0",
                        "-24.86,0",
                        "-25.05,0",  // inland day2 afternoon: min, max, mean
                        "-26.12,0",
                        "-22.46,0",
                        "-24.29,0",  // coast day2 afternoon: min, max, mean
                        "-25.99,0",
                        "-24.52,0",
                        "-25.26,0",  // area day2 afternoon: min, max, mean
                        "Päivälämpötila on -25...-30 astetta, huomenna pakkanen on hieman "
                        "heikompaa. Yöllä pakkasta on vähän yli 30 astetta."  // the story
                        )));
      // #10
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-27.58,0",
                                  "-26.50,0",
                                  "-27.04,0",  // inland day1: min, max, mean
                                  "-29.07,0",
                                  "-25.76,0",
                                  "-27.42,0",  // coast day1: min, max, mean
                                  "-28.30,0",
                                  "-23.03,0",
                                  "-25.67,0",  // area day1: min, max, mean
                                  "-27.58,0",
                                  "-27.46,0",
                                  "-27.52,0",  // inland day1 morning: min, max, mean
                                  "-29.07,0",
                                  "-28.06,0",
                                  "-28.56,0",  // coast day1 morning: min, max, mean
                                  "-28.30,0",
                                  "-27.45,0",
                                  "-27.88,0",  // area day1 morning: min, max, mean
                                  "-27.04,0",
                                  "-26.50,0",
                                  "-26.77,0",  // inland day1 afternoon: min, max, mean
                                  "-28.65,0",
                                  "-25.76,0",
                                  "-27.21,0",  // coast day1 afternoon: min, max, mean
                                  "-27.27,0",
                                  "-23.03,0",
                                  "-25.15,0",  // area day1 afternoon: min, max, mean
                                  "-31.89,0",
                                  "-31.62,0",
                                  "-31.76,0",  // inland night: min, max, mean
                                  "-31.54,0",
                                  "-30.95,0",
                                  "-31.25,0",  // coast night: min, max, mean
                                  "-31.04,0",
                                  "-30.17,0",
                                  "-30.60,0",  // area night: min, max, mean
                                  "-27.08,0",
                                  "-23.15,0",
                                  "-25.11,0",  // inland day2: min, max, mean
                                  "-27.33,0",
                                  "-20.39,0",
                                  "-23.86,0",  // coast day2: min, max, mean
                                  "-28.57,0",
                                  "-21.62,0",
                                  "-25.09,0",  // area day2: min, max, mean
                                  "-26.93,0",
                                  "-26.11,0",
                                  "-26.52,0",  // inland day2 morning: min, max, mean
                                  "-26.52,0",
                                  "-24.54,0",
                                  "-25.53,0",  // coast day2 morning: min, max, mean
                                  "-27.66,0",
                                  "-26.76,0",
                                  "-27.21,0",  // area day2 morning: min, max, mean
                                  "-23.85,0",
                                  "-23.45,0",
                                  "-23.65,0",  // inland day2 afternoon: min, max, mean
                                  "-24.10,0",
                                  "-21.22,0",
                                  "-22.66,0",  // coast day2 afternoon: min, max, mean
                                  "-23.94,0",
                                  "-21.96,0",
                                  "-22.95,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -25 astetta, huomenna pakkanen on hieman "
                                  "heikompaa. Yölämpötila on noin -30 astetta."  // the story
                                  )));
      // #11
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-27.03,0",
                                  "-26.04,0",
                                  "-26.53,0",  // inland day1: min, max, mean
                                  "-27.25,0",
                                  "-23.14,0",
                                  "-25.19,0",  // coast day1: min, max, mean
                                  "-27.42,0",
                                  "-23.07,0",
                                  "-25.25,0",  // area day1: min, max, mean
                                  "-27.03,0",
                                  "-26.59,0",
                                  "-26.81,0",  // inland day1 morning: min, max, mean
                                  "-27.25,0",
                                  "-25.49,0",
                                  "-26.37,0",  // coast day1 morning: min, max, mean
                                  "-27.42,0",
                                  "-26.85,0",
                                  "-27.14,0",  // area day1 morning: min, max, mean
                                  "-26.15,0",
                                  "-26.04,0",
                                  "-26.09,0",  // inland day1 afternoon: min, max, mean
                                  "-28.05,0",
                                  "-23.14,0",
                                  "-25.60,0",  // coast day1 afternoon: min, max, mean
                                  "-26.65,0",
                                  "-23.07,0",
                                  "-24.86,0",  // area day1 afternoon: min, max, mean
                                  "-30.15,0",
                                  "-28.73,0",
                                  "-29.44,0",  // inland night: min, max, mean
                                  "-30.12,0",
                                  "-29.55,0",
                                  "-29.84,0",  // coast night: min, max, mean
                                  "-28.87,0",
                                  "-27.63,0",
                                  "-28.25,0",  // area night: min, max, mean
                                  "-30.69,0",
                                  "-25.89,0",
                                  "-28.29,0",  // inland day2: min, max, mean
                                  "-29.40,0",
                                  "-24.20,0",
                                  "-26.80,0",  // coast day2: min, max, mean
                                  "-30.08,0",
                                  "-23.91,0",
                                  "-27.00,0",  // area day2: min, max, mean
                                  "-29.86,0",
                                  "-29.26,0",
                                  "-29.56,0",  // inland day2 morning: min, max, mean
                                  "-29.35,0",
                                  "-28.93,0",
                                  "-29.14,0",  // coast day2 morning: min, max, mean
                                  "-30.07,0",
                                  "-28.45,0",
                                  "-29.26,0",  // area day2 morning: min, max, mean
                                  "-27.55,0",
                                  "-26.70,0",
                                  "-27.13,0",  // inland day2 afternoon: min, max, mean
                                  "-28.16,0",
                                  "-24.70,0",
                                  "-26.43,0",  // coast day2 afternoon: min, max, mean
                                  "-28.21,0",
                                  "-24.43,0",
                                  "-26.32,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -25 astetta, huomenna suunnilleen sama. "
                                  "Yölämpötila on noin -30 astetta."  // the story
                                  )));
      // #12
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-25.35,0",
                                  "-24.17,0",
                                  "-24.76,0",  // inland day1: min, max, mean
                                  "-25.56,0",
                                  "-19.59,0",
                                  "-22.58,0",  // coast day1: min, max, mean
                                  "-25.97,0",
                                  "-20.01,0",
                                  "-22.99,0",  // area day1: min, max, mean
                                  "-25.35,0",
                                  "-24.99,0",
                                  "-25.17,0",  // inland day1 morning: min, max, mean
                                  "-25.56,0",
                                  "-25.09,0",
                                  "-25.32,0",  // coast day1 morning: min, max, mean
                                  "-25.97,0",
                                  "-25.36,0",
                                  "-25.66,0",  // area day1 morning: min, max, mean
                                  "-24.56,0",
                                  "-24.17,0",
                                  "-24.37,0",  // inland day1 afternoon: min, max, mean
                                  "-24.58,0",
                                  "-19.59,0",
                                  "-22.09,0",  // coast day1 afternoon: min, max, mean
                                  "-24.73,0",
                                  "-20.01,0",
                                  "-22.37,0",  // area day1 afternoon: min, max, mean
                                  "-27.15,0",
                                  "-25.32,0",
                                  "-26.24,0",  // inland night: min, max, mean
                                  "-27.01,0",
                                  "-25.35,0",
                                  "-26.18,0",  // coast night: min, max, mean
                                  "-28.47,0",
                                  "-26.52,0",
                                  "-27.49,0",  // area night: min, max, mean
                                  "-30.56,0",
                                  "-26.50,0",
                                  "-28.53,0",  // inland day2: min, max, mean
                                  "-29.79,0",
                                  "-24.42,0",
                                  "-27.11,0",  // coast day2: min, max, mean
                                  "-31.37,0",
                                  "-23.12,0",
                                  "-27.24,0",  // area day2: min, max, mean
                                  "-29.93,0",
                                  "-29.52,0",
                                  "-29.72,0",  // inland day2 morning: min, max, mean
                                  "-29.14,0",
                                  "-27.73,0",
                                  "-28.44,0",  // coast day2 morning: min, max, mean
                                  "-30.59,0",
                                  "-29.23,0",
                                  "-29.91,0",  // area day2 morning: min, max, mean
                                  "-27.44,0",
                                  "-26.61,0",
                                  "-27.02,0",  // inland day2 afternoon: min, max, mean
                                  "-27.51,0",
                                  "-24.91,0",
                                  "-26.21,0",  // coast day2 afternoon: min, max, mean
                                  "-27.75,0",
                                  "-24.01,0",
                                  "-25.88,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -20...-25 astetta, huomenna pakkanen on "
                                  "hieman kireämpää. Yölämpötila on noin -25 astetta."  // the story
                                  )));
      // #13
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-25.24,0",
              "-23.22,0",
              "-24.23,0",  // inland day1: min, max, mean
              "-26.95,0",
              "-21.35,0",
              "-24.15,0",  // coast day1: min, max, mean
              "-25.54,0",
              "-19.90,0",
              "-22.72,0",  // area day1: min, max, mean
              "-25.24,0",
              "-24.74,0",
              "-24.99,0",  // inland day1 morning: min, max, mean
              "-26.95,0",
              "-25.02,0",
              "-25.99,0",  // coast day1 morning: min, max, mean
              "-25.54,0",
              "-24.43,0",
              "-24.98,0",  // area day1 morning: min, max, mean
              "-23.99,0",
              "-23.22,0",
              "-23.60,0",  // inland day1 afternoon: min, max, mean
              "-25.14,0",
              "-21.35,0",
              "-23.25,0",  // coast day1 afternoon: min, max, mean
              "-24.31,0",
              "-19.90,0",
              "-22.11,0",  // area day1 afternoon: min, max, mean
              "-27.93,0",
              "-27.25,0",
              "-27.59,0",  // inland night: min, max, mean
              "-28.42,0",
              "-28.27,0",
              "-28.34,0",  // coast night: min, max, mean
              "-26.09,0",
              "-24.55,0",
              "-25.32,0",  // area night: min, max, mean
              "-30.48,0",
              "-25.47,0",
              "-27.98,0",  // inland day2: min, max, mean
              "-29.54,0",
              "-24.83,0",
              "-27.19,0",  // coast day2: min, max, mean
              "-30.59,0",
              "-25.74,0",
              "-28.16,0",  // area day2: min, max, mean
              "-29.59,0",
              "-28.88,0",
              "-29.24,0",  // inland day2 morning: min, max, mean
              "-28.96,0",
              "-27.68,0",
              "-28.32,0",  // coast day2 morning: min, max, mean
              "-30.44,0",
              "-28.46,0",
              "-29.45,0",  // area day2 morning: min, max, mean
              "-26.60,0",
              "-26.29,0",
              "-26.45,0",  // inland day2 afternoon: min, max, mean
              "-27.38,0",
              "-25.57,0",
              "-26.47,0",  // coast day2 afternoon: min, max, mean
              "-26.87,0",
              "-26.50,0",
              "-26.69,0",  // area day2 afternoon: min, max, mean
              "Lämpötila on -20...-25 astetta, huomenna pakkanen on hieman kireämpää."  // the story
              )));
      // #14
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-24.24,0",
                                  "-21.26,0",
                                  "-22.75,0",  // inland day1: min, max, mean
                                  "-25.58,0",
                                  "-21.26,0",
                                  "-23.42,0",  // coast day1: min, max, mean
                                  "-24.88,0",
                                  "-19.99,0",
                                  "-22.43,0",  // area day1: min, max, mean
                                  "-24.24,0",
                                  "-23.74,0",
                                  "-23.99,0",  // inland day1 morning: min, max, mean
                                  "-25.58,0",
                                  "-23.63,0",
                                  "-24.61,0",  // coast day1 morning: min, max, mean
                                  "-24.88,0",
                                  "-23.16,0",
                                  "-24.02,0",  // area day1 morning: min, max, mean
                                  "-21.93,0",
                                  "-21.26,0",
                                  "-21.60,0",  // inland day1 afternoon: min, max, mean
                                  "-22.84,0",
                                  "-21.26,0",
                                  "-22.05,0",  // coast day1 afternoon: min, max, mean
                                  "-22.53,0",
                                  "-19.99,0",
                                  "-21.26,0",  // area day1 afternoon: min, max, mean
                                  "-27.80,0",
                                  "-25.85,0",
                                  "-26.82,0",  // inland night: min, max, mean
                                  "-25.98,0",
                                  "-25.52,0",
                                  "-25.75,0",  // coast night: min, max, mean
                                  "-26.19,0",
                                  "-26.06,0",
                                  "-26.13,0",  // area night: min, max, mean
                                  "-23.17,0",
                                  "-20.19,0",
                                  "-21.68,0",  // inland day2: min, max, mean
                                  "-22.83,0",
                                  "-21.11,0",
                                  "-21.97,0",  // coast day2: min, max, mean
                                  "-23.51,0",
                                  "-19.90,0",
                                  "-21.71,0",  // area day2: min, max, mean
                                  "-23.12,0",
                                  "-22.74,0",
                                  "-22.93,0",  // inland day2 morning: min, max, mean
                                  "-22.45,0",
                                  "-21.33,0",
                                  "-21.89,0",  // coast day2 morning: min, max, mean
                                  "-23.25,0",
                                  "-22.95,0",
                                  "-23.10,0",  // area day2 morning: min, max, mean
                                  "-20.70,0",
                                  "-20.27,0",
                                  "-20.49,0",  // inland day2 afternoon: min, max, mean
                                  "-21.77,0",
                                  "-21.65,0",
                                  "-21.71,0",  // coast day2 afternoon: min, max, mean
                                  "-21.30,0",
                                  "-20.78,0",
                                  "-21.04,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -20 asteen tienoilla, huomenna suunnilleen "
                                  "sama. Yöllä pakkasta on vähän yli 25 astetta."  // the story
                                  )));
      // #15
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-22.92,0",
                                  "-21.18,0",
                                  "-22.05,0",  // inland day1: min, max, mean
                                  "-24.06,0",
                                  "-22.79,0",
                                  "-23.43,0",  // coast day1: min, max, mean
                                  "-23.68,0",
                                  "-19.03,0",
                                  "-21.35,0",  // area day1: min, max, mean
                                  "-22.92,0",
                                  "-22.17,0",
                                  "-22.54,0",  // inland day1 morning: min, max, mean
                                  "-24.06,0",
                                  "-23.90,0",
                                  "-23.98,0",  // coast day1 morning: min, max, mean
                                  "-23.68,0",
                                  "-23.10,0",
                                  "-23.39,0",  // area day1 morning: min, max, mean
                                  "-21.91,0",
                                  "-21.18,0",
                                  "-21.55,0",  // inland day1 afternoon: min, max, mean
                                  "-23.27,0",
                                  "-22.79,0",
                                  "-23.03,0",  // coast day1 afternoon: min, max, mean
                                  "-22.84,0",
                                  "-19.03,0",
                                  "-20.94,0",  // area day1 afternoon: min, max, mean
                                  "-25.74,0",
                                  "-25.38,0",
                                  "-25.56,0",  // inland night: min, max, mean
                                  "-27.58,0",
                                  "-26.46,0",
                                  "-27.02,0",  // coast night: min, max, mean
                                  "-26.45,0",
                                  "-26.32,0",
                                  "-26.39,0",  // area night: min, max, mean
                                  "-19.38,0",
                                  "-14.80,0",
                                  "-17.09,0",  // inland day2: min, max, mean
                                  "-18.92,0",
                                  "-16.57,0",
                                  "-17.75,0",  // coast day2: min, max, mean
                                  "-19.93,0",
                                  "-12.75,0",
                                  "-16.34,0",  // area day2: min, max, mean
                                  "-18.69,0",
                                  "-18.06,0",
                                  "-18.37,0",  // inland day2 morning: min, max, mean
                                  "-18.68,0",
                                  "-18.32,0",
                                  "-18.50,0",  // coast day2 morning: min, max, mean
                                  "-19.01,0",
                                  "-18.25,0",
                                  "-18.63,0",  // area day2 morning: min, max, mean
                                  "-16.05,0",
                                  "-15.79,0",
                                  "-15.92,0",  // inland day2 afternoon: min, max, mean
                                  "-17.67,0",
                                  "-17.03,0",
                                  "-17.35,0",  // coast day2 afternoon: min, max, mean
                                  "-16.33,0",
                                  "-13.68,0",
                                  "-15.01,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -20 asteen tienoilla, huomenna noin -15 "
                                  "astetta. Yöllä pakkasta on vähän yli 25 astetta."  // the story
                                  )));
      // #16
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-23.39,0",
                        "-21.80,0",
                        "-22.60,0",  // inland day1: min, max, mean
                        "-24.27,0",
                        "-19.31,0",
                        "-21.79,0",  // coast day1: min, max, mean
                        "-23.65,0",
                        "-21.95,0",
                        "-22.80,0",  // area day1: min, max, mean
                        "-23.39,0",
                        "-22.58,0",
                        "-22.98,0",  // inland day1 morning: min, max, mean
                        "-24.27,0",
                        "-23.30,0",
                        "-23.79,0",  // coast day1 morning: min, max, mean
                        "-23.65,0",
                        "-23.09,0",
                        "-23.37,0",  // area day1 morning: min, max, mean
                        "-22.41,0",
                        "-21.80,0",
                        "-22.10,0",  // inland day1 afternoon: min, max, mean
                        "-23.11,0",
                        "-19.31,0",
                        "-21.21,0",  // coast day1 afternoon: min, max, mean
                        "-22.84,0",
                        "-21.95,0",
                        "-22.40,0",  // area day1 afternoon: min, max, mean
                        "-25.09,0",
                        "-23.52,0",
                        "-24.31,0",  // inland night: min, max, mean
                        "-27.48,0",
                        "-25.93,0",
                        "-26.71,0",  // coast night: min, max, mean
                        "-24.06,0",
                        "-24.04,0",
                        "-24.05,0",  // area night: min, max, mean
                        "-27.77,0",
                        "-23.48,0",
                        "-25.62,0",  // inland day2: min, max, mean
                        "-26.48,0",
                        "-22.06,0",
                        "-24.27,0",  // coast day2: min, max, mean
                        "-28.12,0",
                        "-21.02,0",
                        "-24.57,0",  // area day2: min, max, mean
                        "-26.87,0",
                        "-26.52,0",
                        "-26.70,0",  // inland day2 morning: min, max, mean
                        "-26.22,0",
                        "-25.15,0",
                        "-25.69,0",  // coast day2 morning: min, max, mean
                        "-27.46,0",
                        "-26.89,0",
                        "-27.17,0",  // area day2 morning: min, max, mean
                        "-25.11,0",
                        "-24.45,0",
                        "-24.78,0",  // inland day2 afternoon: min, max, mean
                        "-25.72,0",
                        "-22.48,0",
                        "-24.10,0",  // coast day2 afternoon: min, max, mean
                        "-25.78,0",
                        "-21.85,0",
                        "-23.81,0",  // area day2 afternoon: min, max, mean
                        "Pakkasta on vähän yli 20 astetta, huomenna suunnilleen sama."  // the story
                        )));
      // #17
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-21.85,0",
                        "-21.54,0",
                        "-21.70,0",  // inland day1: min, max, mean
                        "-22.09,0",
                        "-21.05,0",
                        "-21.57,0",  // coast day1: min, max, mean
                        "-22.73,0",
                        "-20.68,0",
                        "-21.71,0",  // area day1: min, max, mean
                        "-21.85,0",
                        "-21.82,0",
                        "-21.84,0",  // inland day1 morning: min, max, mean
                        "-22.09,0",
                        "-20.98,0",
                        "-21.53,0",  // coast day1 morning: min, max, mean
                        "-22.73,0",
                        "-22.21,0",
                        "-22.47,0",  // area day1 morning: min, max, mean
                        "-21.82,0",
                        "-21.54,0",
                        "-21.68,0",  // inland day1 afternoon: min, max, mean
                        "-23.56,0",
                        "-21.05,0",
                        "-22.30,0",  // coast day1 afternoon: min, max, mean
                        "-21.90,0",
                        "-20.68,0",
                        "-21.29,0",  // area day1 afternoon: min, max, mean
                        "-21.98,0",
                        "-21.01,0",
                        "-21.49,0",  // inland night: min, max, mean
                        "-22.62,0",
                        "-20.75,0",
                        "-21.68,0",  // coast night: min, max, mean
                        "-24.40,0",
                        "-22.63,0",
                        "-23.51,0",  // area night: min, max, mean
                        "-29.03,0",
                        "-23.68,0",
                        "-26.35,0",  // inland day2: min, max, mean
                        "-28.65,0",
                        "-22.99,0",
                        "-25.82,0",  // coast day2: min, max, mean
                        "-28.80,0",
                        "-20.09,0",
                        "-24.44,0",  // area day2: min, max, mean
                        "-28.31,0",
                        "-27.70,0",
                        "-28.00,0",  // inland day2 morning: min, max, mean
                        "-27.92,0",
                        "-26.15,0",
                        "-27.04,0",  // coast day2 morning: min, max, mean
                        "-28.40,0",
                        "-26.96,0",
                        "-27.68,0",  // area day2 morning: min, max, mean
                        "-24.35,0",
                        "-24.17,0",
                        "-24.26,0",  // inland day2 afternoon: min, max, mean
                        "-25.54,0",
                        "-23.59,0",
                        "-24.56,0",  // coast day2 afternoon: min, max, mean
                        "-24.64,0",
                        "-20.46,0",
                        "-22.55,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on noin -20 astetta, huomenna suunnilleen sama."  // the story
                        )));
      // #18
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-21.22,0",
                                  "-20.55,0",
                                  "-20.88,0",  // inland day1: min, max, mean
                                  "-21.71,0",
                                  "-20.71,0",
                                  "-21.21,0",  // coast day1: min, max, mean
                                  "-22.10,0",
                                  "-18.01,0",
                                  "-20.06,0",  // area day1: min, max, mean
                                  "-21.22,0",
                                  "-20.62,0",
                                  "-20.92,0",  // inland day1 morning: min, max, mean
                                  "-21.71,0",
                                  "-21.02,0",
                                  "-21.36,0",  // coast day1 morning: min, max, mean
                                  "-22.10,0",
                                  "-21.28,0",
                                  "-21.69,0",  // area day1 morning: min, max, mean
                                  "-20.87,0",
                                  "-20.55,0",
                                  "-20.71,0",  // inland day1 afternoon: min, max, mean
                                  "-22.20,0",
                                  "-20.71,0",
                                  "-21.45,0",  // coast day1 afternoon: min, max, mean
                                  "-21.02,0",
                                  "-18.01,0",
                                  "-19.52,0",  // area day1 afternoon: min, max, mean
                                  "-25.59,0",
                                  "-25.29,0",
                                  "-25.44,0",  // inland night: min, max, mean
                                  "-22.95,0",
                                  "-21.33,0",
                                  "-22.14,0",  // coast night: min, max, mean
                                  "-26.08,0",
                                  "-24.33,0",
                                  "-25.21,0",  // area night: min, max, mean
                                  "-19.73,0",
                                  "-17.18,0",
                                  "-18.46,0",  // inland day2: min, max, mean
                                  "-20.15,0",
                                  "-19.40,0",
                                  "-19.77,0",  // coast day2: min, max, mean
                                  "-20.66,0",
                                  "-16.11,0",
                                  "-18.38,0",  // area day2: min, max, mean
                                  "-19.46,0",
                                  "-18.82,0",
                                  "-19.14,0",  // inland day2 morning: min, max, mean
                                  "-19.29,0",
                                  "-18.77,0",
                                  "-19.03,0",  // coast day2 morning: min, max, mean
                                  "-20.29,0",
                                  "-19.51,0",
                                  "-19.90,0",  // area day2 morning: min, max, mean
                                  "-18.18,0",
                                  "-18.10,0",
                                  "-18.14,0",  // inland day2 afternoon: min, max, mean
                                  "-19.87,0",
                                  "-19.82,0",
                                  "-19.85,0",  // coast day2 afternoon: min, max, mean
                                  "-18.99,0",
                                  "-16.59,0",
                                  "-17.79,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -20 astetta, huomenna on hieman "
                                  "lauhempaa. Yölämpötila on noin -25 astetta."  // the story
                                  )));
      // #19
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-21.82,0",
                        "-19.89,0",
                        "-20.86,0",  // inland day1: min, max, mean
                        "-23.71,0",
                        "-22.04,0",
                        "-22.88,0",  // coast day1: min, max, mean
                        "-22.08,0",
                        "-16.48,0",
                        "-19.28,0",  // area day1: min, max, mean
                        "-21.82,0",
                        "-21.33,0",
                        "-21.58,0",  // inland day1 morning: min, max, mean
                        "-23.71,0",
                        "-22.55,0",
                        "-23.13,0",  // coast day1 morning: min, max, mean
                        "-22.08,0",
                        "-21.57,0",
                        "-21.82,0",  // area day1 morning: min, max, mean
                        "-20.86,0",
                        "-19.89,0",
                        "-20.37,0",  // inland day1 afternoon: min, max, mean
                        "-22.24,0",
                        "-22.04,0",
                        "-22.14,0",  // coast day1 afternoon: min, max, mean
                        "-21.16,0",
                        "-16.48,0",
                        "-18.82,0",  // area day1 afternoon: min, max, mean
                        "-23.30,0",
                        "-22.92,0",
                        "-23.11,0",  // inland night: min, max, mean
                        "-26.34,0",
                        "-24.65,0",
                        "-25.50,0",  // coast night: min, max, mean
                        "-22.75,0",
                        "-21.19,0",
                        "-21.97,0",  // area night: min, max, mean
                        "-24.28,0",
                        "-18.91,0",
                        "-21.60,0",  // inland day2: min, max, mean
                        "-23.56,0",
                        "-16.52,0",
                        "-20.04,0",  // coast day2: min, max, mean
                        "-24.26,0",
                        "-17.59,0",
                        "-20.92,0",  // area day2: min, max, mean
                        "-23.46,0",
                        "-22.73,0",
                        "-23.10,0",  // inland day2 morning: min, max, mean
                        "-23.41,0",
                        "-22.13,0",
                        "-22.77,0",  // coast day2 morning: min, max, mean
                        "-24.17,0",
                        "-22.20,0",
                        "-23.19,0",  // area day2 morning: min, max, mean
                        "-19.18,0",
                        "-19.00,0",
                        "-19.09,0",  // inland day2 afternoon: min, max, mean
                        "-20.51,0",
                        "-16.53,0",
                        "-18.52,0",  // coast day2 afternoon: min, max, mean
                        "-19.88,0",
                        "-17.85,0",
                        "-18.86,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on -15...-20 astetta, huomenna suunnilleen sama."  // the story
                        )));
      // #20
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-20.06,0",
                                  "-16.95,0",
                                  "-18.51,0",  // inland day1: min, max, mean
                                  "-21.71,0",
                                  "-13.96,0",
                                  "-17.83,0",  // coast day1: min, max, mean
                                  "-20.98,0",
                                  "-16.81,0",
                                  "-18.90,0",  // area day1: min, max, mean
                                  "-20.06,0",
                                  "-19.23,0",
                                  "-19.64,0",  // inland day1 morning: min, max, mean
                                  "-21.71,0",
                                  "-21.70,0",
                                  "-21.70,0",  // coast day1 morning: min, max, mean
                                  "-20.98,0",
                                  "-19.08,0",
                                  "-20.03,0",  // area day1 morning: min, max, mean
                                  "-17.47,0",
                                  "-16.95,0",
                                  "-17.21,0",  // inland day1 afternoon: min, max, mean
                                  "-18.33,0",
                                  "-13.96,0",
                                  "-16.14,0",  // coast day1 afternoon: min, max, mean
                                  "-18.15,0",
                                  "-16.81,0",
                                  "-17.48,0",  // area day1 afternoon: min, max, mean
                                  "-23.59,0",
                                  "-21.61,0",
                                  "-22.60,0",  // inland night: min, max, mean
                                  "-22.53,0",
                                  "-21.76,0",
                                  "-22.14,0",  // coast night: min, max, mean
                                  "-23.13,0",
                                  "-22.15,0",
                                  "-22.64,0",  // area night: min, max, mean
                                  "-17.77,0",
                                  "-12.87,0",
                                  "-15.32,0",  // inland day2: min, max, mean
                                  "-16.89,0",
                                  "-13.69,0",
                                  "-15.29,0",  // coast day2: min, max, mean
                                  "-18.33,0",
                                  "-10.79,0",
                                  "-14.56,0",  // area day2: min, max, mean
                                  "-17.14,0",
                                  "-16.23,0",
                                  "-16.68,0",  // inland day2 morning: min, max, mean
                                  "-16.62,0",
                                  "-15.27,0",
                                  "-15.95,0",  // coast day2 morning: min, max, mean
                                  "-17.64,0",
                                  "-15.86,0",
                                  "-16.75,0",  // area day2 morning: min, max, mean
                                  "-13.62,0",
                                  "-13.22,0",
                                  "-13.42,0",  // inland day2 afternoon: min, max, mean
                                  "-14.57,0",
                                  "-14.48,0",
                                  "-14.53,0",  // coast day2 afternoon: min, max, mean
                                  "-14.23,0",
                                  "-11.60,0",
                                  "-12.91,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -15 astetta, huomenna -12...-14 astetta. "
                                  "Yölämpötila on noin -25 astetta."  // the story
                                  )));
      // #21
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-19.48,0",
                                  "-18.05,0",
                                  "-18.76,0",  // inland day1: min, max, mean
                                  "-20.38,0",
                                  "-18.70,0",
                                  "-19.54,0",  // coast day1: min, max, mean
                                  "-20.47,0",
                                  "-17.87,0",
                                  "-19.17,0",  // area day1: min, max, mean
                                  "-19.48,0",
                                  "-18.86,0",
                                  "-19.17,0",  // inland day1 morning: min, max, mean
                                  "-20.38,0",
                                  "-19.43,0",
                                  "-19.90,0",  // coast day1 morning: min, max, mean
                                  "-20.47,0",
                                  "-18.69,0",
                                  "-19.58,0",  // area day1 morning: min, max, mean
                                  "-18.30,0",
                                  "-18.05,0",
                                  "-18.17,0",  // inland day1 afternoon: min, max, mean
                                  "-18.84,0",
                                  "-18.70,0",
                                  "-18.77,0",  // coast day1 afternoon: min, max, mean
                                  "-18.44,0",
                                  "-17.87,0",
                                  "-18.15,0",  // area day1 afternoon: min, max, mean
                                  "-19.66,0",
                                  "-18.44,0",
                                  "-19.05,0",  // inland night: min, max, mean
                                  "-20.78,0",
                                  "-19.03,0",
                                  "-19.90,0",  // coast night: min, max, mean
                                  "-24.27,0",
                                  "-23.75,0",
                                  "-24.01,0",  // area night: min, max, mean
                                  "-20.13,0",
                                  "-17.24,0",
                                  "-18.68,0",  // inland day2: min, max, mean
                                  "-19.73,0",
                                  "-17.24,0",
                                  "-18.48,0",  // coast day2: min, max, mean
                                  "-20.87,0",
                                  "-17.63,0",
                                  "-19.25,0",  // area day2: min, max, mean
                                  "-19.76,0",
                                  "-19.00,0",
                                  "-19.38,0",  // inland day2 morning: min, max, mean
                                  "-19.58,0",
                                  "-17.99,0",
                                  "-18.79,0",  // coast day2 morning: min, max, mean
                                  "-20.67,0",
                                  "-19.20,0",
                                  "-19.94,0",  // area day2 morning: min, max, mean
                                  "-18.70,0",
                                  "-18.09,0",
                                  "-18.39,0",  // inland day2 afternoon: min, max, mean
                                  "-19.25,0",
                                  "-17.52,0",
                                  "-18.38,0",  // coast day2 afternoon: min, max, mean
                                  "-19.06,0",
                                  "-18.58,0",
                                  "-18.82,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -20 astetta, huomenna suunnilleen sama. "
                                  "Yölämpötila on -25 asteen tuntumassa."  // the story
                                  )));
      // #22
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-19.76,0",
                        "-17.55,0",
                        "-18.65,0",  // inland day1: min, max, mean
                        "-20.71,0",
                        "-19.14,0",
                        "-19.93,0",  // coast day1: min, max, mean
                        "-19.95,0",
                        "-16.44,0",
                        "-18.19,0",  // area day1: min, max, mean
                        "-19.76,0",
                        "-19.19,0",
                        "-19.47,0",  // inland day1 morning: min, max, mean
                        "-20.71,0",
                        "-20.49,0",
                        "-20.60,0",  // coast day1 morning: min, max, mean
                        "-19.95,0",
                        "-19.34,0",
                        "-19.65,0",  // area day1 morning: min, max, mean
                        "-18.49,0",
                        "-17.55,0",
                        "-18.02,0",  // inland day1 afternoon: min, max, mean
                        "-20.42,0",
                        "-19.14,0",
                        "-19.78,0",  // coast day1 afternoon: min, max, mean
                        "-18.80,0",
                        "-16.44,0",
                        "-17.62,0",  // area day1 afternoon: min, max, mean
                        "-21.07,0",
                        "-20.87,0",
                        "-20.97,0",  // inland night: min, max, mean
                        "-21.57,0",
                        "-20.70,0",
                        "-21.14,0",  // coast night: min, max, mean
                        "-21.52,0",
                        "-20.27,0",
                        "-20.90,0",  // area night: min, max, mean
                        "-18.53,0",
                        "-14.14,0",
                        "-16.33,0",  // inland day2: min, max, mean
                        "-18.47,0",
                        "-12.67,0",
                        "-15.57,0",  // coast day2: min, max, mean
                        "-18.74,0",
                        "-10.32,0",
                        "-14.53,0",  // area day2: min, max, mean
                        "-17.75,0",
                        "-16.75,0",
                        "-17.25,0",  // inland day2 morning: min, max, mean
                        "-17.61,0",
                        "-16.14,0",
                        "-16.88,0",  // coast day2 morning: min, max, mean
                        "-18.27,0",
                        "-16.42,0",
                        "-17.34,0",  // area day2 morning: min, max, mean
                        "-15.20,0",
                        "-14.72,0",
                        "-14.96,0",  // inland day2 afternoon: min, max, mean
                        "-15.75,0",
                        "-12.85,0",
                        "-14.30,0",  // coast day2 afternoon: min, max, mean
                        "-15.41,0",
                        "-10.64,0",
                        "-13.02,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on -15...-20 astetta, huomenna -11...-15 astetta."  // the story
                        )));
      // #23
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-18.33,0",
                                  "-15.25,0",
                                  "-16.79,0",  // inland day1: min, max, mean
                                  "-19.83,0",
                                  "-10.80,0",
                                  "-15.31,0",  // coast day1: min, max, mean
                                  "-18.84,0",
                                  "-13.09,0",
                                  "-15.97,0",  // area day1: min, max, mean
                                  "-18.33,0",
                                  "-17.34,0",
                                  "-17.83,0",  // inland day1 morning: min, max, mean
                                  "-19.83,0",
                                  "-18.65,0",
                                  "-19.24,0",  // coast day1 morning: min, max, mean
                                  "-18.84,0",
                                  "-17.26,0",
                                  "-18.05,0",  // area day1 morning: min, max, mean
                                  "-15.54,0",
                                  "-15.25,0",
                                  "-15.39,0",  // inland day1 afternoon: min, max, mean
                                  "-15.77,0",
                                  "-10.80,0",
                                  "-13.28,0",  // coast day1 afternoon: min, max, mean
                                  "-16.29,0",
                                  "-13.09,0",
                                  "-14.69,0",  // area day1 afternoon: min, max, mean
                                  "-20.35,0",
                                  "-18.57,0",
                                  "-19.46,0",  // inland night: min, max, mean
                                  "-20.73,0",
                                  "-18.83,0",
                                  "-19.78,0",  // coast night: min, max, mean
                                  "-22.67,0",
                                  "-20.77,0",
                                  "-21.72,0",  // area night: min, max, mean
                                  "-16.53,0",
                                  "-13.64,0",
                                  "-15.08,0",  // inland day2: min, max, mean
                                  "-15.39,0",
                                  "-15.41,0",
                                  "-15.40,0",  // coast day2: min, max, mean
                                  "-16.95,0",
                                  "-12.28,0",
                                  "-14.62,0",  // area day2: min, max, mean
                                  "-15.91,0",
                                  "-15.66,0",
                                  "-15.79,0",  // inland day2 morning: min, max, mean
                                  "-15.01,0",
                                  "-13.32,0",
                                  "-14.17,0",  // coast day2 morning: min, max, mean
                                  "-16.84,0",
                                  "-15.51,0",
                                  "-16.18,0",  // area day2 morning: min, max, mean
                                  "-15.00,0",
                                  "-14.63,0",
                                  "-14.82,0",  // inland day2 afternoon: min, max, mean
                                  "-16.74,0",
                                  "-15.50,0",
                                  "-16.12,0",  // coast day2 afternoon: min, max, mean
                                  "-15.15,0",
                                  "-12.61,0",
                                  "-13.88,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -15 astetta, huomenna on hieman "
                                  "lauhempaa. Yölämpötila on -20...-25 astetta."  // the story
                                  )));
      // #24
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-17.82,0",
              "-16.69,0",
              "-17.26,0",  // inland day1: min, max, mean
              "-17.97,0",
              "-18.78,0",
              "-18.37,0",  // coast day1: min, max, mean
              "-18.73,0",
              "-17.29,0",
              "-18.01,0",  // area day1: min, max, mean
              "-17.82,0",
              "-17.69,0",
              "-17.76,0",  // inland day1 morning: min, max, mean
              "-17.97,0",
              "-16.02,0",
              "-16.99,0",  // coast day1 morning: min, max, mean
              "-18.73,0",
              "-18.45,0",
              "-18.59,0",  // area day1 morning: min, max, mean
              "-17.05,0",
              "-16.69,0",
              "-16.87,0",  // inland day1 afternoon: min, max, mean
              "-18.98,0",
              "-18.78,0",
              "-18.88,0",  // coast day1 afternoon: min, max, mean
              "-17.61,0",
              "-17.29,0",
              "-17.45,0",  // area day1 afternoon: min, max, mean
              "-20.13,0",
              "-19.38,0",
              "-19.75,0",  // inland night: min, max, mean
              "-20.45,0",
              "-20.06,0",
              "-20.26,0",  // coast night: min, max, mean
              "-20.76,0",
              "-20.23,0",
              "-20.50,0",  // area night: min, max, mean
              "-17.24,0",
              "-12.91,0",
              "-15.08,0",  // inland day2: min, max, mean
              "-16.51,0",
              "-12.13,0",
              "-14.32,0",  // coast day2: min, max, mean
              "-17.61,0",
              "-10.42,0",
              "-14.02,0",  // area day2: min, max, mean
              "-16.83,0",
              "-16.65,0",
              "-16.74,0",  // inland day2 morning: min, max, mean
              "-16.12,0",
              "-15.63,0",
              "-15.88,0",  // coast day2 morning: min, max, mean
              "-17.18,0",
              "-15.66,0",
              "-16.42,0",  // area day2 morning: min, max, mean
              "-13.85,0",
              "-13.38,0",
              "-13.62,0",  // inland day2 afternoon: min, max, mean
              "-14.14,0",
              "-12.77,0",
              "-13.45,0",  // coast day2 afternoon: min, max, mean
              "-14.63,0",
              "-10.49,0",
              "-12.56,0",  // area day2 afternoon: min, max, mean
              "Pakkasta on vähän yli 15 astetta, huomenna -10...-15 astetta."  // the story
              )));
      // #25
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-17.77,0",
                                  "-14.94,0",
                                  "-16.35,0",  // inland day1: min, max, mean
                                  "-19.25,0",
                                  "-15.76,0",
                                  "-17.51,0",  // coast day1: min, max, mean
                                  "-18.08,0",
                                  "-15.36,0",
                                  "-16.72,0",  // area day1: min, max, mean
                                  "-17.77,0",
                                  "-16.97,0",
                                  "-17.37,0",  // inland day1 morning: min, max, mean
                                  "-19.25,0",
                                  "-19.04,0",
                                  "-19.15,0",  // coast day1 morning: min, max, mean
                                  "-18.08,0",
                                  "-17.59,0",
                                  "-17.83,0",  // area day1 morning: min, max, mean
                                  "-15.83,0",
                                  "-14.94,0",
                                  "-15.38,0",  // inland day1 afternoon: min, max, mean
                                  "-17.08,0",
                                  "-15.76,0",
                                  "-16.42,0",  // coast day1 afternoon: min, max, mean
                                  "-16.61,0",
                                  "-15.36,0",
                                  "-15.98,0",  // area day1 afternoon: min, max, mean
                                  "-17.98,0",
                                  "-16.04,0",
                                  "-17.01,0",  // inland night: min, max, mean
                                  "-20.57,0",
                                  "-19.00,0",
                                  "-19.79,0",  // coast night: min, max, mean
                                  "-19.97,0",
                                  "-19.80,0",
                                  "-19.89,0",  // area night: min, max, mean
                                  "-21.63,0",
                                  "-18.00,0",
                                  "-19.82,0",  // inland day2: min, max, mean
                                  "-21.88,0",
                                  "-18.24,0",
                                  "-20.06,0",  // coast day2: min, max, mean
                                  "-22.22,0",
                                  "-16.05,0",
                                  "-19.14,0",  // area day2: min, max, mean
                                  "-21.56,0",
                                  "-21.15,0",
                                  "-21.35,0",  // inland day2 morning: min, max, mean
                                  "-21.55,0",
                                  "-21.40,0",
                                  "-21.48,0",  // coast day2 morning: min, max, mean
                                  "-21.89,0",
                                  "-20.99,0",
                                  "-21.44,0",  // area day2 morning: min, max, mean
                                  "-18.99,0",
                                  "-18.75,0",
                                  "-18.87,0",  // inland day2 afternoon: min, max, mean
                                  "-19.22,0",
                                  "-18.90,0",
                                  "-19.06,0",  // coast day2 afternoon: min, max, mean
                                  "-19.46,0",
                                  "-16.72,0",
                                  "-18.09,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -15 astetta, huomenna suunnilleen sama. "
                                  "Yölämpötila on -20 asteen tuntumassa."  // the story
                                  )));
      // #26
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-16.88,0",
              "-15.61,0",
              "-16.24,0",  // inland day1: min, max, mean
              "-17.88,0",
              "-14.31,0",
              "-16.09,0",  // coast day1: min, max, mean
              "-17.48,0",
              "-15.77,0",
              "-16.62,0",  // area day1: min, max, mean
              "-16.88,0",
              "-16.82,0",
              "-16.85,0",  // inland day1 morning: min, max, mean
              "-17.88,0",
              "-16.08,0",
              "-16.98,0",  // coast day1 morning: min, max, mean
              "-17.48,0",
              "-17.25,0",
              "-17.36,0",  // area day1 morning: min, max, mean
              "-16.58,0",
              "-15.61,0",
              "-16.09,0",  // inland day1 afternoon: min, max, mean
              "-17.34,0",
              "-14.31,0",
              "-15.82,0",  // coast day1 afternoon: min, max, mean
              "-17.10,0",
              "-15.77,0",
              "-16.43,0",  // area day1 afternoon: min, max, mean
              "-18.98,0",
              "-18.90,0",
              "-18.94,0",  // inland night: min, max, mean
              "-20.96,0",
              "-20.71,0",
              "-20.83,0",  // coast night: min, max, mean
              "-17.83,0",
              "-16.58,0",
              "-17.20,0",  // area night: min, max, mean
              "-23.25,0",
              "-19.63,0",
              "-21.44,0",  // inland day2: min, max, mean
              "-22.37,0",
              "-20.96,0",
              "-21.67,0",  // coast day2: min, max, mean
              "-23.44,0",
              "-18.97,0",
              "-21.21,0",  // area day2: min, max, mean
              "-22.45,0",
              "-22.21,0",
              "-22.33,0",  // inland day2 morning: min, max, mean
              "-21.78,0",
              "-20.18,0",
              "-20.98,0",  // coast day2 morning: min, max, mean
              "-23.21,0",
              "-22.13,0",
              "-22.67,0",  // area day2 morning: min, max, mean
              "-20.97,0",
              "-20.51,0",
              "-20.74,0",  // inland day2 afternoon: min, max, mean
              "-22.13,0",
              "-21.06,0",
              "-21.60,0",  // coast day2 afternoon: min, max, mean
              "-21.59,0",
              "-19.09,0",
              "-20.34,0",  // area day2 afternoon: min, max, mean
              "Lämpötila on noin -15 astetta, huomenna pakkanen on hieman kireämpää."  // the story
              )));
      // #27
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-15.72,0",
                                  "-13.05,0",
                                  "-14.39,0",  // inland day1: min, max, mean
                                  "-16.76,0",
                                  "-11.89,0",
                                  "-14.32,0",  // coast day1: min, max, mean
                                  "-16.52,0",
                                  "-13.21,0",
                                  "-14.87,0",  // area day1: min, max, mean
                                  "-15.72,0",
                                  "-14.95,0",
                                  "-15.34,0",  // inland day1 morning: min, max, mean
                                  "-16.76,0",
                                  "-15.26,0",
                                  "-16.01,0",  // coast day1 morning: min, max, mean
                                  "-16.52,0",
                                  "-14.90,0",
                                  "-15.71,0",  // area day1 morning: min, max, mean
                                  "-13.89,0",
                                  "-13.05,0",
                                  "-13.47,0",  // inland day1 afternoon: min, max, mean
                                  "-15.71,0",
                                  "-11.89,0",
                                  "-13.80,0",  // coast day1 afternoon: min, max, mean
                                  "-14.25,0",
                                  "-13.21,0",
                                  "-13.73,0",  // area day1 afternoon: min, max, mean
                                  "-19.66,0",
                                  "-19.26,0",
                                  "-19.46,0",  // inland night: min, max, mean
                                  "-17.11,0",
                                  "-16.27,0",
                                  "-16.69,0",  // coast night: min, max, mean
                                  "-20.01,0",
                                  "-18.37,0",
                                  "-19.19,0",  // area night: min, max, mean
                                  "-11.95,0",
                                  "-7.19,0",
                                  "-9.57,0",  // inland day2: min, max, mean
                                  "-11.31,0",
                                  "-6.50,0",
                                  "-8.91,0",  // coast day2: min, max, mean
                                  "-11.86,0",
                                  "-5.32,0",
                                  "-8.59,0",  // area day2: min, max, mean
                                  "-11.28,0",
                                  "-11.02,0",
                                  "-11.15,0",  // inland day2 morning: min, max, mean
                                  "-10.92,0",
                                  "-9.61,0",
                                  "-10.27,0",  // coast day2 morning: min, max, mean
                                  "-11.61,0",
                                  "-9.78,0",
                                  "-10.69,0",  // area day2 morning: min, max, mean
                                  "-8.14,0",
                                  "-7.41,0",
                                  "-7.78,0",  // inland day2 afternoon: min, max, mean
                                  "-9.23,0",
                                  "-7.41,0",
                                  "-8.32,0",  // coast day2 afternoon: min, max, mean
                                  "-8.66,0",
                                  "-5.42,0",
                                  "-7.04,0",  // area day2 afternoon: min, max, mean
                                  "Päivällä pakkasta on vajaat 15 astetta, huomenna -5...-9 "
                                  "astetta. Yölämpötila on lähellä -20 astetta."  // the story
                                  )));
      // #28
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-15.64,0",
                        "-14.96,0",
                        "-15.30,0",  // inland day1: min, max, mean
                        "-15.76,0",
                        "-13.73,0",
                        "-14.74,0",  // coast day1: min, max, mean
                        "-16.50,0",
                        "-12.60,0",
                        "-14.55,0",  // area day1: min, max, mean
                        "-15.64,0",
                        "-14.92,0",
                        "-15.28,0",  // inland day1 morning: min, max, mean
                        "-15.76,0",
                        "-15.51,0",
                        "-15.64,0",  // coast day1 morning: min, max, mean
                        "-16.50,0",
                        "-16.40,0",
                        "-16.45,0",  // area day1 morning: min, max, mean
                        "-15.28,0",
                        "-14.96,0",
                        "-15.12,0",  // inland day1 afternoon: min, max, mean
                        "-15.70,0",
                        "-13.73,0",
                        "-14.71,0",  // coast day1 afternoon: min, max, mean
                        "-15.97,0",
                        "-12.60,0",
                        "-14.28,0",  // area day1 afternoon: min, max, mean
                        "-19.08,0",
                        "-17.77,0",
                        "-18.43,0",  // inland night: min, max, mean
                        "-16.36,0",
                        "-14.82,0",
                        "-15.59,0",  // coast night: min, max, mean
                        "-17.23,0",
                        "-17.14,0",
                        "-17.18,0",  // area night: min, max, mean
                        "-14.11,0",
                        "-7.76,0",
                        "-10.93,0",  // inland day2: min, max, mean
                        "-12.69,0",
                        "-7.43,0",
                        "-10.06,0",  // coast day2: min, max, mean
                        "-14.18,0",
                        "-5.80,0",
                        "-9.99,0",  // area day2: min, max, mean
                        "-13.39,0",
                        "-12.61,0",
                        "-13.00,0",  // inland day2 morning: min, max, mean
                        "-12.48,0",
                        "-11.48,0",
                        "-11.98,0",  // coast day2 morning: min, max, mean
                        "-13.89,0",
                        "-11.98,0",
                        "-12.94,0",  // area day2 morning: min, max, mean
                        "-8.40,0",
                        "-8.29,0",
                        "-8.34,0",  // inland day2 afternoon: min, max, mean
                        "-10.30,0",
                        "-7.46,0",
                        "-8.88,0",  // coast day2 afternoon: min, max, mean
                        "-9.34,0",
                        "-5.87,0",
                        "-7.61,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on noin -15 astetta, huomenna -6...-9 astetta."  // the story
                        )));
      // #29
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-15.43,0",
                        "-14.32,0",
                        "-14.87,0",  // inland day1: min, max, mean
                        "-16.49,0",
                        "-10.88,0",
                        "-13.69,0",  // coast day1: min, max, mean
                        "-15.72,0",
                        "-12.62,0",
                        "-14.17,0",  // area day1: min, max, mean
                        "-15.43,0",
                        "-15.30,0",
                        "-15.36,0",  // inland day1 morning: min, max, mean
                        "-16.49,0",
                        "-16.42,0",
                        "-16.46,0",  // coast day1 morning: min, max, mean
                        "-15.72,0",
                        "-15.39,0",
                        "-15.55,0",  // area day1 morning: min, max, mean
                        "-14.82,0",
                        "-14.32,0",
                        "-14.57,0",  // inland day1 afternoon: min, max, mean
                        "-15.44,0",
                        "-10.88,0",
                        "-13.16,0",  // coast day1 afternoon: min, max, mean
                        "-15.33,0",
                        "-12.62,0",
                        "-13.98,0",  // area day1 afternoon: min, max, mean
                        "-19.80,0",
                        "-18.64,0",
                        "-19.22,0",  // inland night: min, max, mean
                        "-17.47,0",
                        "-15.49,0",
                        "-16.48,0",  // coast night: min, max, mean
                        "-17.14,0",
                        "-15.80,0",
                        "-16.47,0",  // area night: min, max, mean
                        "-14.01,0",
                        "-10.92,0",
                        "-12.47,0",  // inland day2: min, max, mean
                        "-14.16,0",
                        "-12.69,0",
                        "-13.42,0",  // coast day2: min, max, mean
                        "-13.96,0",
                        "-9.63,0",
                        "-11.80,0",  // area day2: min, max, mean
                        "-13.38,0",
                        "-12.55,0",
                        "-12.96,0",  // inland day2 morning: min, max, mean
                        "-13.35,0",
                        "-12.67,0",
                        "-13.01,0",  // coast day2 morning: min, max, mean
                        "-13.83,0",
                        "-12.07,0",
                        "-12.95,0",  // area day2 morning: min, max, mean
                        "-11.88,0",
                        "-11.56,0",
                        "-11.72,0",  // inland day2 afternoon: min, max, mean
                        "-13.79,0",
                        "-13.18,0",
                        "-13.48,0",  // coast day2 afternoon: min, max, mean
                        "-12.05,0",
                        "-10.13,0",
                        "-11.09,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on noin -15 astetta, huomenna on hieman lauhempaa."  // the story
                        )));
      // #30
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-15.27,0",
                                  "-13.34,0",
                                  "-14.30,0",  // inland day1: min, max, mean
                                  "-16.44,0",
                                  "-11.80,0",
                                  "-14.12,0",  // coast day1: min, max, mean
                                  "-15.60,0",
                                  "-10.56,0",
                                  "-13.08,0",  // area day1: min, max, mean
                                  "-15.27,0",
                                  "-14.49,0",
                                  "-14.88,0",  // inland day1 morning: min, max, mean
                                  "-16.44,0",
                                  "-15.68,0",
                                  "-16.06,0",  // coast day1 morning: min, max, mean
                                  "-15.60,0",
                                  "-15.18,0",
                                  "-15.39,0",  // area day1 morning: min, max, mean
                                  "-13.44,0",
                                  "-13.34,0",
                                  "-13.39,0",  // inland day1 afternoon: min, max, mean
                                  "-14.65,0",
                                  "-11.80,0",
                                  "-13.22,0",  // coast day1 afternoon: min, max, mean
                                  "-14.33,0",
                                  "-10.56,0",
                                  "-12.45,0",  // area day1 afternoon: min, max, mean
                                  "-17.89,0",
                                  "-17.15,0",
                                  "-17.52,0",  // inland night: min, max, mean
                                  "-18.43,0",
                                  "-17.72,0",
                                  "-18.08,0",  // coast night: min, max, mean
                                  "-18.22,0",
                                  "-18.22,0",
                                  "-18.22,0",  // area night: min, max, mean
                                  "-18.11,0",
                                  "-13.95,0",
                                  "-16.03,0",  // inland day2: min, max, mean
                                  "-17.98,0",
                                  "-12.29,0",
                                  "-15.13,0",  // coast day2: min, max, mean
                                  "-18.85,0",
                                  "-14.95,0",
                                  "-16.90,0",  // area day2: min, max, mean
                                  "-17.59,0",
                                  "-17.42,0",
                                  "-17.50,0",  // inland day2 morning: min, max, mean
                                  "-17.41,0",
                                  "-17.25,0",
                                  "-17.33,0",  // coast day2 morning: min, max, mean
                                  "-18.21,0",
                                  "-16.54,0",
                                  "-17.37,0",  // area day2 morning: min, max, mean
                                  "-14.80,0",
                                  "-14.16,0",
                                  "-14.48,0",  // inland day2 afternoon: min, max, mean
                                  "-16.21,0",
                                  "-12.66,0",
                                  "-14.44,0",  // coast day2 afternoon: min, max, mean
                                  "-15.04,0",
                                  "-15.02,0",
                                  "-15.03,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -11...-14 astetta, huomenna suunnilleen sama. "
                                  "Yöllä pakkasta on vajaat 20 astetta."  // the story
                                  )));
      // #31
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-14.24,0",
              "-13.02,0",
              "-13.63,0",  // inland day1: min, max, mean
              "-16.09,0",
              "-14.53,0",
              "-15.31,0",  // coast day1: min, max, mean
              "-14.91,0",
              "-13.55,0",
              "-14.23,0",  // area day1: min, max, mean
              "-14.24,0",
              "-13.56,0",
              "-13.90,0",  // inland day1 morning: min, max, mean
              "-16.09,0",
              "-14.23,0",
              "-15.16,0",  // coast day1 morning: min, max, mean
              "-14.91,0",
              "-13.91,0",
              "-14.41,0",  // area day1 morning: min, max, mean
              "-13.10,0",
              "-13.02,0",
              "-13.06,0",  // inland day1 afternoon: min, max, mean
              "-14.54,0",
              "-14.53,0",
              "-14.53,0",  // coast day1 afternoon: min, max, mean
              "-13.90,0",
              "-13.55,0",
              "-13.72,0",  // area day1 afternoon: min, max, mean
              "-15.10,0",
              "-14.56,0",
              "-14.83,0",  // inland night: min, max, mean
              "-17.41,0",
              "-16.08,0",
              "-16.75,0",  // coast night: min, max, mean
              "-17.56,0",
              "-15.70,0",
              "-16.63,0",  // area night: min, max, mean
              "-13.30,0",
              "-9.01,0",
              "-11.15,0",  // inland day2: min, max, mean
              "-12.55,0",
              "-9.26,0",
              "-10.91,0",  // coast day2: min, max, mean
              "-13.75,0",
              "-9.46,0",
              "-11.60,0",  // area day2: min, max, mean
              "-12.81,0",
              "-12.65,0",
              "-12.73,0",  // inland day2 morning: min, max, mean
              "-12.37,0",
              "-10.56,0",
              "-11.47,0",  // coast day2 morning: min, max, mean
              "-12.91,0",
              "-11.26,0",
              "-12.08,0",  // area day2 morning: min, max, mean
              "-9.89,0",
              "-9.41,0",
              "-9.65,0",  // inland day2 afternoon: min, max, mean
              "-11.86,0",
              "-10.09,0",
              "-10.98,0",  // coast day2 afternoon: min, max, mean
              "-10.34,0",
              "-9.62,0",
              "-9.98,0",  // area day2 afternoon: min, max, mean
              "Lämpötila on -15 asteen tuntumassa, huomenna on hieman lauhempaa."  // the story
              )));
      // #32
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-13.01,0",
                        "-10.17,0",
                        "-11.59,0",  // inland day1: min, max, mean
                        "-13.88,0",
                        "-9.38,0",
                        "-11.63,0",  // coast day1: min, max, mean
                        "-13.72,0",
                        "-10.32,0",
                        "-12.02,0",  // area day1: min, max, mean
                        "-13.01,0",
                        "-12.93,0",
                        "-12.97,0",  // inland day1 morning: min, max, mean
                        "-13.88,0",
                        "-12.05,0",
                        "-12.97,0",  // coast day1 morning: min, max, mean
                        "-13.72,0",
                        "-11.74,0",
                        "-12.73,0",  // area day1 morning: min, max, mean
                        "-10.70,0",
                        "-10.17,0",
                        "-10.44,0",  // inland day1 afternoon: min, max, mean
                        "-11.54,0",
                        "-9.38,0",
                        "-10.46,0",  // coast day1 afternoon: min, max, mean
                        "-11.06,0",
                        "-10.32,0",
                        "-10.69,0",  // area day1 afternoon: min, max, mean
                        "-15.40,0",
                        "-13.74,0",
                        "-14.57,0",  // inland night: min, max, mean
                        "-13.92,0",
                        "-12.52,0",
                        "-13.22,0",  // coast night: min, max, mean
                        "-14.07,0",
                        "-12.25,0",
                        "-13.16,0",  // area night: min, max, mean
                        "-8.91,0",
                        "-5.84,0",
                        "-7.37,0",  // inland day2: min, max, mean
                        "-8.97,0",
                        "-5.77,0",
                        "-7.37,0",  // coast day2: min, max, mean
                        "-9.10,0",
                        "-3.68,0",
                        "-6.39,0",  // area day2: min, max, mean
                        "-8.63,0",
                        "-7.64,0",
                        "-8.14,0",  // inland day2 morning: min, max, mean
                        "-8.24,0",
                        "-6.25,0",
                        "-7.24,0",  // coast day2 morning: min, max, mean
                        "-8.97,0",
                        "-7.24,0",
                        "-8.10,0",  // area day2 morning: min, max, mean
                        "-6.53,0",
                        "-5.94,0",
                        "-6.24,0",  // inland day2 afternoon: min, max, mean
                        "-7.40,0",
                        "-6.13,0",
                        "-6.76,0",  // coast day2 afternoon: min, max, mean
                        "-6.72,0",
                        "-4.50,0",
                        "-5.61,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on noin -10 astetta, huomenna -5 asteen tienoilla."  // the story
                        )));
      // #33
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam("-12.30,0",
                                            "-11.18,0",
                                            "-11.74,0",  // inland day1: min, max, mean
                                            "-13.01,0",
                                            "-9.03,0",
                                            "-11.02,0",  // coast day1: min, max, mean
                                            "-13.02,0",
                                            "-10.50,0",
                                            "-11.76,0",  // area day1: min, max, mean
                                            "-12.30,0",
                                            "-12.21,0",
                                            "-12.26,0",  // inland day1 morning: min, max, mean
                                            "-13.01,0",
                                            "-11.85,0",
                                            "-12.43,0",  // coast day1 morning: min, max, mean
                                            "-13.02,0",
                                            "-12.89,0",
                                            "-12.95,0",  // area day1 morning: min, max, mean
                                            "-12.09,0",
                                            "-11.18,0",
                                            "-11.64,0",  // inland day1 afternoon: min, max, mean
                                            "-13.77,0",
                                            "-9.03,0",
                                            "-11.40,0",  // coast day1 afternoon: min, max, mean
                                            "-12.18,0",
                                            "-10.50,0",
                                            "-11.34,0",  // area day1 afternoon: min, max, mean
                                            "-16.28,0",
                                            "-16.21,0",
                                            "-16.24,0",  // inland night: min, max, mean
                                            "-13.38,0",
                                            "-11.97,0",
                                            "-12.68,0",  // coast night: min, max, mean
                                            "-15.47,0",
                                            "-13.98,0",
                                            "-14.73,0",  // area night: min, max, mean
                                            "-19.34,0",
                                            "-15.74,0",
                                            "-17.54,0",  // inland day2: min, max, mean
                                            "-18.70,0",
                                            "-14.45,0",
                                            "-16.58,0",  // coast day2: min, max, mean
                                            "-19.90,0",
                                            "-12.32,0",
                                            "-16.11,0",  // area day2: min, max, mean
                                            "-18.85,0",
                                            "-18.04,0",
                                            "-18.45,0",  // inland day2 morning: min, max, mean
                                            "-18.07,0",
                                            "-17.39,0",
                                            "-17.73,0",  // coast day2 morning: min, max, mean
                                            "-18.98,0",
                                            "-17.87,0",
                                            "-18.42,0",  // area day2 morning: min, max, mean
                                            "-17.19,0",
                                            "-16.60,0",
                                            "-16.90,0",  // inland day2 afternoon: min, max, mean
                                            "-17.90,0",
                                            "-15.29,0",
                                            "-16.59,0",  // coast day2 afternoon: min, max, mean
                                            "-17.57,0",
                                            "-13.09,0",
                                            "-15.33,0",  // area day2 afternoon: min, max, mean
                                            "Lämpötila on noin -10 astetta, huomenna on hieman "
                                            "kylmempää. Yöllä sisämaassa pakkasta on vähän yli 15 "
                                            "astetta, rannikolla noin -13 astetta."  // the story
                                            )));
      // #34
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-12.71,0",
                                  "-10.53,0",
                                  "-11.62,0",  // inland day1: min, max, mean
                                  "-14.14,0",
                                  "-10.75,0",
                                  "-12.44,0",  // coast day1: min, max, mean
                                  "-13.33,0",
                                  "-7.24,0",
                                  "-10.28,0",  // area day1: min, max, mean
                                  "-12.71,0",
                                  "-12.30,0",
                                  "-12.50,0",  // inland day1 morning: min, max, mean
                                  "-14.14,0",
                                  "-13.45,0",
                                  "-13.79,0",  // coast day1 morning: min, max, mean
                                  "-13.33,0",
                                  "-12.34,0",
                                  "-12.84,0",  // area day1 morning: min, max, mean
                                  "-11.46,0",
                                  "-10.53,0",
                                  "-10.99,0",  // inland day1 afternoon: min, max, mean
                                  "-11.82,0",
                                  "-10.75,0",
                                  "-11.28,0",  // coast day1 afternoon: min, max, mean
                                  "-12.23,0",
                                  "-7.24,0",
                                  "-9.73,0",  // area day1 afternoon: min, max, mean
                                  "-16.56,0",
                                  "-14.85,0",
                                  "-15.70,0",  // inland night: min, max, mean
                                  "-14.88,0",
                                  "-14.67,0",
                                  "-14.77,0",  // coast night: min, max, mean
                                  "-15.71,0",
                                  "-14.58,0",
                                  "-15.14,0",  // area night: min, max, mean
                                  "-11.08,0",
                                  "-5.92,0",
                                  "-8.50,0",  // inland day2: min, max, mean
                                  "-11.15,0",
                                  "-4.53,0",
                                  "-7.84,0",  // coast day2: min, max, mean
                                  "-11.77,0",
                                  "-1.36,0",
                                  "-6.56,0",  // area day2: min, max, mean
                                  "-10.64,0",
                                  "-9.65,0",
                                  "-10.14,0",  // inland day2 morning: min, max, mean
                                  "-10.34,0",
                                  "-8.36,0",
                                  "-9.35,0",  // coast day2 morning: min, max, mean
                                  "-10.87,0",
                                  "-8.93,0",
                                  "-9.90,0",  // area day2 morning: min, max, mean
                                  "-6.26,0",
                                  "-6.07,0",
                                  "-6.17,0",  // inland day2 afternoon: min, max, mean
                                  "-7.53,0",
                                  "-4.84,0",
                                  "-6.18,0",  // coast day2 afternoon: min, max, mean
                                  "-6.45,0",
                                  "-2.27,0",
                                  "-4.36,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -10 asteen tienoilla, huomenna -2...-6 "
                                  "astetta. Yölämpötila on noin -15 astetta."  // the story
                                  )));
      // #35
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-12.73,0",
                                  "-11.69,0",
                                  "-12.21,0",  // inland day1: min, max, mean
                                  "-13.04,0",
                                  "-11.40,0",
                                  "-12.22,0",  // coast day1: min, max, mean
                                  "-13.05,0",
                                  "-10.67,0",
                                  "-11.86,0",  // area day1: min, max, mean
                                  "-12.73,0",
                                  "-12.57,0",
                                  "-12.65,0",  // inland day1 morning: min, max, mean
                                  "-13.04,0",
                                  "-12.30,0",
                                  "-12.67,0",  // coast day1 morning: min, max, mean
                                  "-13.05,0",
                                  "-12.17,0",
                                  "-12.61,0",  // area day1 morning: min, max, mean
                                  "-11.76,0",
                                  "-11.69,0",
                                  "-11.72,0",  // inland day1 afternoon: min, max, mean
                                  "-12.07,0",
                                  "-11.40,0",
                                  "-11.73,0",  // coast day1 afternoon: min, max, mean
                                  "-11.88,0",
                                  "-10.67,0",
                                  "-11.27,0",  // area day1 afternoon: min, max, mean
                                  "-15.44,0",
                                  "-15.01,0",
                                  "-15.22,0",  // inland night: min, max, mean
                                  "-15.36,0",
                                  "-14.17,0",
                                  "-14.77,0",  // coast night: min, max, mean
                                  "-16.73,0",
                                  "-15.12,0",
                                  "-15.92,0",  // area night: min, max, mean
                                  "-6.67,0",
                                  "-2.37,0",
                                  "-4.52,0",  // inland day2: min, max, mean
                                  "-6.12,0",
                                  "-3.44,0",
                                  "-4.78,0",  // coast day2: min, max, mean
                                  "-7.25,0",
                                  "-0.32,0",
                                  "-3.79,0",  // area day2: min, max, mean
                                  "-6.16,0",
                                  "-5.39,0",
                                  "-5.78,0",  // inland day2 morning: min, max, mean
                                  "-5.39,0",
                                  "-5.07,0",
                                  "-5.23,0",  // coast day2 morning: min, max, mean
                                  "-7.10,0",
                                  "-6.05,0",
                                  "-6.57,0",  // area day2 morning: min, max, mean
                                  "-3.13,0",
                                  "-2.55,0",
                                  "-2.84,0",  // inland day2 afternoon: min, max, mean
                                  "-4.59,0",
                                  "-3.71,0",
                                  "-4.15,0",  // coast day2 afternoon: min, max, mean
                                  "-3.30,0",
                                  "-0.49,0",
                                  "-1.90,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -10 astetta, päivällä on pikkupakkasta. "
                                  "Yölämpötila on noin -15 astetta."  // the story
                                  )));
      // #36
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-11.82,0",
                                  "-9.59,0",
                                  "-10.71,0",  // inland day1: min, max, mean
                                  "-13.15,0",
                                  "-9.23,0",
                                  "-11.19,0",  // coast day1: min, max, mean
                                  "-12.38,0",
                                  "-10.27,0",
                                  "-11.33,0",  // area day1: min, max, mean
                                  "-11.82,0",
                                  "-10.90,0",
                                  "-11.36,0",  // inland day1 morning: min, max, mean
                                  "-13.15,0",
                                  "-13.01,0",
                                  "-13.08,0",  // coast day1 morning: min, max, mean
                                  "-12.38,0",
                                  "-11.81,0",
                                  "-12.09,0",  // area day1 morning: min, max, mean
                                  "-10.48,0",
                                  "-9.59,0",
                                  "-10.04,0",  // inland day1 afternoon: min, max, mean
                                  "-11.74,0",
                                  "-9.23,0",
                                  "-10.49,0",  // coast day1 afternoon: min, max, mean
                                  "-11.16,0",
                                  "-10.27,0",
                                  "-10.72,0",  // area day1 afternoon: min, max, mean
                                  "-16.66,0",
                                  "-14.98,0",
                                  "-15.82,0",  // inland night: min, max, mean
                                  "-15.83,0",
                                  "-15.35,0",
                                  "-15.59,0",  // coast night: min, max, mean
                                  "-15.28,0",
                                  "-15.06,0",
                                  "-15.17,0",  // area night: min, max, mean
                                  "-17.80,0",
                                  "-13.23,0",
                                  "-15.51,0",  // inland day2: min, max, mean
                                  "-17.88,0",
                                  "-14.75,0",
                                  "-16.31,0",  // coast day2: min, max, mean
                                  "-18.54,0",
                                  "-10.46,0",
                                  "-14.50,0",  // area day2: min, max, mean
                                  "-17.37,0",
                                  "-16.38,0",
                                  "-16.87,0",  // inland day2 morning: min, max, mean
                                  "-16.96,0",
                                  "-15.30,0",
                                  "-16.13,0",  // coast day2 morning: min, max, mean
                                  "-18.29,0",
                                  "-17.92,0",
                                  "-18.10,0",  // area day2 morning: min, max, mean
                                  "-14.77,0",
                                  "-13.93,0",
                                  "-14.35,0",  // inland day2 afternoon: min, max, mean
                                  "-16.13,0",
                                  "-15.58,0",
                                  "-15.85,0",  // coast day2 afternoon: min, max, mean
                                  "-15.69,0",
                                  "-11.12,0",
                                  "-13.40,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -10 astetta, huomenna on hieman "
                                  "kylmempää. Yölämpötila on noin -15 astetta."  // the story
                                  )));
      // #37
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-10.89,0",
                        "-7.07,0",
                        "-8.98,0",  // inland day1: min, max, mean
                        "-12.14,0",
                        "-6.21,0",
                        "-9.18,0",  // coast day1: min, max, mean
                        "-11.15,0",
                        "-6.64,0",
                        "-8.90,0",  // area day1: min, max, mean
                        "-10.89,0",
                        "-10.52,0",
                        "-10.71,0",  // inland day1 morning: min, max, mean
                        "-12.14,0",
                        "-11.58,0",
                        "-11.86,0",  // coast day1 morning: min, max, mean
                        "-11.15,0",
                        "-9.34,0",
                        "-10.24,0",  // area day1 morning: min, max, mean
                        "-8.03,0",
                        "-7.07,0",
                        "-7.55,0",  // inland day1 afternoon: min, max, mean
                        "-9.28,0",
                        "-6.21,0",
                        "-7.75,0",  // coast day1 afternoon: min, max, mean
                        "-8.43,0",
                        "-6.64,0",
                        "-7.54,0",  // area day1 afternoon: min, max, mean
                        "-13.91,0",
                        "-13.88,0",
                        "-13.89,0",  // inland night: min, max, mean
                        "-14.16,0",
                        "-13.77,0",
                        "-13.96,0",  // coast night: min, max, mean
                        "-12.91,0",
                        "-12.70,0",
                        "-12.80,0",  // area night: min, max, mean
                        "-6.97,0",
                        "-4.42,0",
                        "-5.69,0",  // inland day2: min, max, mean
                        "-6.31,0",
                        "-3.01,0",
                        "-4.66,0",  // coast day2: min, max, mean
                        "-6.90,0",
                        "-3.66,0",
                        "-5.28,0",  // area day2: min, max, mean
                        "-6.16,0",
                        "-5.57,0",
                        "-5.86,0",  // inland day2 morning: min, max, mean
                        "-5.38,0",
                        "-3.48,0",
                        "-4.43,0",  // coast day2 morning: min, max, mean
                        "-6.26,0",
                        "-6.04,0",
                        "-6.15,0",  // area day2 morning: min, max, mean
                        "-4.89,0",
                        "-4.49,0",
                        "-4.69,0",  // inland day2 afternoon: min, max, mean
                        "-5.36,0",
                        "-3.29,0",
                        "-4.33,0",  // coast day2 afternoon: min, max, mean
                        "-4.98,0",
                        "-3.84,0",
                        "-4.41,0",  // area day2 afternoon: min, max, mean
                        "Päivälämpötila on noin -8 astetta, huomenna on hieman lauhempaa. Yöllä "
                        "pakkasta on vähän yli 10 astetta."  // the story
                        )));
      // #38
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam("-10.76,0",
                                            "-9.51,0",
                                            "-10.13,0",  // inland day1: min, max, mean
                                            "-11.00,0",
                                            "-9.54,0",
                                            "-10.27,0",  // coast day1: min, max, mean
                                            "-11.41,0",
                                            "-9.69,0",
                                            "-10.55,0",  // area day1: min, max, mean
                                            "-10.76,0",
                                            "-10.23,0",
                                            "-10.50,0",  // inland day1 morning: min, max, mean
                                            "-11.00,0",
                                            "-9.75,0",
                                            "-10.37,0",  // coast day1 morning: min, max, mean
                                            "-11.41,0",
                                            "-10.13,0",
                                            "-10.77,0",  // area day1 morning: min, max, mean
                                            "-9.94,0",
                                            "-9.51,0",
                                            "-9.73,0",  // inland day1 afternoon: min, max, mean
                                            "-11.40,0",
                                            "-9.54,0",
                                            "-10.47,0",  // coast day1 afternoon: min, max, mean
                                            "-9.99,0",
                                            "-9.69,0",
                                            "-9.84,0",  // area day1 afternoon: min, max, mean
                                            "-11.33,0",
                                            "-9.57,0",
                                            "-10.45,0",  // inland night: min, max, mean
                                            "-13.82,0",
                                            "-13.19,0",
                                            "-13.50,0",  // coast night: min, max, mean
                                            "-15.07,0",
                                            "-14.65,0",
                                            "-14.86,0",  // area night: min, max, mean
                                            "-16.13,0",
                                            "-11.84,0",
                                            "-13.99,0",  // inland day2: min, max, mean
                                            "-14.96,0",
                                            "-12.02,0",
                                            "-13.49,0",  // coast day2: min, max, mean
                                            "-16.14,0",
                                            "-8.54,0",
                                            "-12.34,0",  // area day2: min, max, mean
                                            "-15.83,0",
                                            "-15.13,0",
                                            "-15.48,0",  // inland day2 morning: min, max, mean
                                            "-14.85,0",
                                            "-14.65,0",
                                            "-14.75,0",  // coast day2 morning: min, max, mean
                                            "-15.91,0",
                                            "-15.18,0",
                                            "-15.55,0",  // area day2 morning: min, max, mean
                                            "-11.99,0",
                                            "-11.91,0",
                                            "-11.95,0",  // inland day2 afternoon: min, max, mean
                                            "-13.02,0",
                                            "-12.21,0",
                                            "-12.62,0",  // coast day2 afternoon: min, max, mean
                                            "-12.96,0",
                                            "-9.19,0",
                                            "-11.08,0",  // area day2 afternoon: min, max, mean
                                            "Päivälämpötila on -10 asteen tuntumassa, huomenna "
                                            "suunnilleen sama. Sisämaassa yölämpötila on noin -10 "
                                            "astetta, rannikolla vajaat -15 astetta."  // the story
                                            )));
      // #39
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-9.25,0",
              "-8.15,0",
              "-8.70,0",  // inland day1: min, max, mean
              "-10.53,0",
              "-8.49,0",
              "-9.51,0",  // coast day1: min, max, mean
              "-9.84,0",
              "-8.43,0",
              "-9.13,0",  // area day1: min, max, mean
              "-9.25,0",
              "-8.86,0",
              "-9.06,0",  // inland day1 morning: min, max, mean
              "-10.53,0",
              "-9.25,0",
              "-9.89,0",  // coast day1 morning: min, max, mean
              "-9.84,0",
              "-9.39,0",
              "-9.62,0",  // area day1 morning: min, max, mean
              "-8.70,0",
              "-8.15,0",
              "-8.42,0",  // inland day1 afternoon: min, max, mean
              "-10.00,0",
              "-8.49,0",
              "-9.25,0",  // coast day1 afternoon: min, max, mean
              "-8.93,0",
              "-8.43,0",
              "-8.68,0",  // area day1 afternoon: min, max, mean
              "-10.06,0",
              "-8.60,0",
              "-9.33,0",  // inland night: min, max, mean
              "-14.19,0",
              "-12.84,0",
              "-13.52,0",  // coast night: min, max, mean
              "-12.80,0",
              "-12.13,0",
              "-12.46,0",  // area night: min, max, mean
              "-9.88,0",
              "-5.89,0",
              "-7.89,0",  // inland day2: min, max, mean
              "-9.94,0",
              "-6.93,0",
              "-8.43,0",  // coast day2: min, max, mean
              "-10.21,0",
              "-2.98,0",
              "-6.59,0",  // area day2: min, max, mean
              "-9.69,0",
              "-9.35,0",
              "-9.52,0",  // inland day2 morning: min, max, mean
              "-9.28,0",
              "-8.20,0",
              "-8.74,0",  // coast day2 morning: min, max, mean
              "-9.95,0",
              "-9.56,0",
              "-9.76,0",  // area day2 morning: min, max, mean
              "-7.12,0",
              "-6.26,0",
              "-6.69,0",  // inland day2 afternoon: min, max, mean
              "-8.85,0",
              "-7.71,0",
              "-8.28,0",  // coast day2 afternoon: min, max, mean
              "-8.07,0",
              "-3.21,0",
              "-5.64,0",  // area day2 afternoon: min, max, mean
              "Päivällä pakkasta on vajaat 10 astetta, huomenna on hieman lauhempaa. Sisämaassa "
              "yölämpötila on -10 asteen tuntumassa, rannikolla noin -15 astetta."  // the story
              )));
      // #40
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-8.71,0",
                                  "-8.54,0",
                                  "-8.62,0",  // inland day1: min, max, mean
                                  "-9.92,0",
                                  "-8.28,0",
                                  "-9.10,0",  // coast day1: min, max, mean
                                  "-9.57,0",
                                  "-6.09,0",
                                  "-7.83,0",  // area day1: min, max, mean
                                  "-8.71,0",
                                  "-8.59,0",
                                  "-8.65,0",  // inland day1 morning: min, max, mean
                                  "-9.92,0",
                                  "-8.32,0",
                                  "-9.12,0",  // coast day1 morning: min, max, mean
                                  "-9.57,0",
                                  "-9.46,0",
                                  "-9.52,0",  // area day1 morning: min, max, mean
                                  "-8.99,0",
                                  "-8.54,0",
                                  "-8.76,0",  // inland day1 afternoon: min, max, mean
                                  "-10.03,0",
                                  "-8.28,0",
                                  "-9.15,0",  // coast day1 afternoon: min, max, mean
                                  "-9.17,0",
                                  "-6.09,0",
                                  "-7.63,0",  // area day1 afternoon: min, max, mean
                                  "-13.10,0",
                                  "-12.01,0",
                                  "-12.55,0",  // inland night: min, max, mean
                                  "-12.50,0",
                                  "-11.71,0",
                                  "-12.10,0",  // coast night: min, max, mean
                                  "-12.68,0",
                                  "-12.19,0",
                                  "-12.44,0",  // area night: min, max, mean
                                  "-4.26,0",
                                  "0.24,0",
                                  "-2.01,0",  // inland day2: min, max, mean
                                  "-3.61,0",
                                  "1.44,0",
                                  "-1.08,0",  // coast day2: min, max, mean
                                  "-4.56,0",
                                  "-0.74,0",
                                  "-2.65,0",  // area day2: min, max, mean
                                  "-4.16,0",
                                  "-3.63,0",
                                  "-3.89,0",  // inland day2 morning: min, max, mean
                                  "-3.56,0",
                                  "-3.01,0",
                                  "-3.29,0",  // coast day2 morning: min, max, mean
                                  "-4.35,0",
                                  "-4.01,0",
                                  "-4.18,0",  // area day2 morning: min, max, mean
                                  "-0.70,0",
                                  "-0.60,0",
                                  "-0.65,0",  // inland day2 afternoon: min, max, mean
                                  "-1.14,0",
                                  "1.27,0",
                                  "0.06,0",  // coast day2 afternoon: min, max, mean
                                  "-1.40,0",
                                  "-1.15,0",
                                  "-1.28,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -6...-9 astetta, huomenna noin -1 astetta. "
                                  "Yöllä pakkasta on vähän yli 10 astetta."  // the story
                                  )));
      // #41
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-7.66,0",
                                  "-6.61,0",
                                  "-7.14,0",  // inland day1: min, max, mean
                                  "-8.56,0",
                                  "-3.02,0",
                                  "-5.79,0",  // coast day1: min, max, mean
                                  "-8.03,0",
                                  "-5.07,0",
                                  "-6.55,0",  // area day1: min, max, mean
                                  "-7.66,0",
                                  "-7.21,0",
                                  "-7.44,0",  // inland day1 morning: min, max, mean
                                  "-8.56,0",
                                  "-7.86,0",
                                  "-8.21,0",  // coast day1 morning: min, max, mean
                                  "-8.03,0",
                                  "-7.83,0",
                                  "-7.93,0",  // area day1 morning: min, max, mean
                                  "-7.00,0",
                                  "-6.61,0",
                                  "-6.81,0",  // inland day1 afternoon: min, max, mean
                                  "-7.48,0",
                                  "-3.02,0",
                                  "-5.25,0",  // coast day1 afternoon: min, max, mean
                                  "-7.39,0",
                                  "-5.07,0",
                                  "-6.23,0",  // area day1 afternoon: min, max, mean
                                  "-10.17,0",
                                  "-8.26,0",
                                  "-9.22,0",  // inland night: min, max, mean
                                  "-8.62,0",
                                  "-7.63,0",
                                  "-8.12,0",  // coast night: min, max, mean
                                  "-11.66,0",
                                  "-9.87,0",
                                  "-10.77,0",  // area night: min, max, mean
                                  "-6.73,0",
                                  "-1.72,0",
                                  "-4.23,0",  // inland day2: min, max, mean
                                  "-6.33,0",
                                  "-2.17,0",
                                  "-4.25,0",  // coast day2: min, max, mean
                                  "-7.60,0",
                                  "-1.80,0",
                                  "-4.70,0",  // area day2: min, max, mean
                                  "-6.51,0",
                                  "-6.40,0",
                                  "-6.46,0",  // inland day2 morning: min, max, mean
                                  "-6.24,0",
                                  "-5.46,0",
                                  "-5.85,0",  // coast day2 morning: min, max, mean
                                  "-6.88,0",
                                  "-4.98,0",
                                  "-5.93,0",  // area day2 morning: min, max, mean
                                  "-2.61,0",
                                  "-2.14,0",
                                  "-2.37,0",  // inland day2 afternoon: min, max, mean
                                  "-3.99,0",
                                  "-3.01,0",
                                  "-3.50,0",  // coast day2 afternoon: min, max, mean
                                  "-3.25,0",
                                  "-2.35,0",
                                  "-2.80,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -5...-7 astetta, huomenna noin -3 astetta. "
                                  "Yölämpötila on noin -10 astetta."  // the story
                                  )));
      // #42
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-6.93,0",
                                  "-4.20,0",
                                  "-5.57,0",  // inland day1: min, max, mean
                                  "-8.49,0",
                                  "-1.21,0",
                                  "-4.85,0",  // coast day1: min, max, mean
                                  "-7.38,0",
                                  "-0.76,0",
                                  "-4.07,0",  // area day1: min, max, mean
                                  "-6.93,0",
                                  "-6.53,0",
                                  "-6.73,0",  // inland day1 morning: min, max, mean
                                  "-8.49,0",
                                  "-7.53,0",
                                  "-8.01,0",  // coast day1 morning: min, max, mean
                                  "-7.38,0",
                                  "-5.78,0",
                                  "-6.58,0",  // area day1 morning: min, max, mean
                                  "-4.83,0",
                                  "-4.20,0",
                                  "-4.52,0",  // inland day1 afternoon: min, max, mean
                                  "-5.92,0",
                                  "-1.21,0",
                                  "-3.56,0",  // coast day1 afternoon: min, max, mean
                                  "-5.45,0",
                                  "-0.76,0",
                                  "-3.10,0",  // area day1 afternoon: min, max, mean
                                  "-10.77,0",
                                  "-10.17,0",
                                  "-10.47,0",  // inland night: min, max, mean
                                  "-10.36,0",
                                  "-9.43,0",
                                  "-9.90,0",  // coast night: min, max, mean
                                  "-10.93,0",
                                  "-10.84,0",
                                  "-10.88,0",  // area night: min, max, mean
                                  "-6.68,0",
                                  "-2.54,0",
                                  "-4.61,0",  // inland day2: min, max, mean
                                  "-6.84,0",
                                  "-1.95,0",
                                  "-4.40,0",  // coast day2: min, max, mean
                                  "-7.20,0",
                                  "0.25,0",
                                  "-3.47,0",  // area day2: min, max, mean
                                  "-6.15,0",
                                  "-5.59,0",
                                  "-5.87,0",  // inland day2 morning: min, max, mean
                                  "-5.99,0",
                                  "-5.01,0",
                                  "-5.50,0",  // coast day2 morning: min, max, mean
                                  "-6.55,0",
                                  "-5.94,0",
                                  "-6.24,0",  // area day2 morning: min, max, mean
                                  "-3.89,0",
                                  "-3.20,0",
                                  "-3.54,0",  // inland day2 afternoon: min, max, mean
                                  "-3.98,0",
                                  "-2.35,0",
                                  "-3.17,0",  // coast day2 afternoon: min, max, mean
                                  "-4.21,0",
                                  "0.21,0",
                                  "-2.00,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -1...-5 astetta, huomenna on hieman "
                                  "lauhempaa. Yölämpötila on noin -10 astetta."  // the story
                                  )));
      // #43
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-6.30,0",
                        "-5.38,0",
                        "-5.84,0",  // inland day1: min, max, mean
                        "-7.44,0",
                        "-3.24,0",
                        "-5.34,0",  // coast day1: min, max, mean
                        "-6.89,0",
                        "-3.96,0",
                        "-5.43,0",  // area day1: min, max, mean
                        "-6.30,0",
                        "-6.05,0",
                        "-6.17,0",  // inland day1 morning: min, max, mean
                        "-7.44,0",
                        "-5.72,0",
                        "-6.58,0",  // coast day1 morning: min, max, mean
                        "-6.89,0",
                        "-6.53,0",
                        "-6.71,0",  // area day1 morning: min, max, mean
                        "-5.45,0",
                        "-5.38,0",
                        "-5.41,0",  // inland day1 afternoon: min, max, mean
                        "-6.09,0",
                        "-3.24,0",
                        "-4.67,0",  // coast day1 afternoon: min, max, mean
                        "-6.40,0",
                        "-3.96,0",
                        "-5.18,0",  // area day1 afternoon: min, max, mean
                        "-7.46,0",
                        "-5.52,0",
                        "-6.49,0",  // inland night: min, max, mean
                        "-8.92,0",
                        "-7.42,0",
                        "-8.17,0",  // coast night: min, max, mean
                        "-6.89,0",
                        "-5.59,0",
                        "-6.24,0",  // area night: min, max, mean
                        "-9.46,0",
                        "-4.29,0",
                        "-6.87,0",  // inland day2: min, max, mean
                        "-8.88,0",
                        "-7.12,0",
                        "-8.00,0",  // coast day2: min, max, mean
                        "-9.88,0",
                        "-1.92,0",
                        "-5.90,0",  // area day2: min, max, mean
                        "-8.51,0",
                        "-7.59,0",
                        "-8.05,0",  // inland day2 morning: min, max, mean
                        "-7.88,0",
                        "-6.45,0",
                        "-7.17,0",  // coast day2 morning: min, max, mean
                        "-9.41,0",
                        "-9.04,0",
                        "-9.22,0",  // area day2 morning: min, max, mean
                        "-6.23,0",
                        "-5.27,0",
                        "-5.75,0",  // inland day2 afternoon: min, max, mean
                        "-7.78,0",
                        "-7.21,0",
                        "-7.50,0",  // coast day2 afternoon: min, max, mean
                        "-6.61,0",
                        "-2.53,0",
                        "-4.57,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on noin -5 astetta, huomenna suunnilleen sama."  // the story
                        )));
      // #44
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam("-6.84,0",
                                            "-4.35,0",
                                            "-5.60,0",  // inland day1: min, max, mean
                                            "-8.08,0",
                                            "-3.81,0",
                                            "-5.94,0",  // coast day1: min, max, mean
                                            "-7.15,0",
                                            "-0.77,0",
                                            "-3.96,0",  // area day1: min, max, mean
                                            "-6.84,0",
                                            "-6.34,0",
                                            "-6.59,0",  // inland day1 morning: min, max, mean
                                            "-8.08,0",
                                            "-7.53,0",
                                            "-7.80,0",  // coast day1 morning: min, max, mean
                                            "-7.15,0",
                                            "-5.40,0",
                                            "-6.28,0",  // area day1 morning: min, max, mean
                                            "-4.50,0",
                                            "-4.35,0",
                                            "-4.42,0",  // inland day1 afternoon: min, max, mean
                                            "-4.71,0",
                                            "-3.81,0",
                                            "-4.26,0",  // coast day1 afternoon: min, max, mean
                                            "-4.74,0",
                                            "-0.77,0",
                                            "-2.75,0",  // area day1 afternoon: min, max, mean
                                            "-7.19,0",
                                            "-5.87,0",
                                            "-6.53,0",  // inland night: min, max, mean
                                            "-10.14,0",
                                            "-9.75,0",
                                            "-9.94,0",  // coast night: min, max, mean
                                            "-8.44,0",
                                            "-7.17,0",
                                            "-7.80,0",  // area night: min, max, mean
                                            "-11.22,0",
                                            "-7.11,0",
                                            "-9.16,0",  // inland day2: min, max, mean
                                            "-9.72,0",
                                            "-5.83,0",
                                            "-7.78,0",  // coast day2: min, max, mean
                                            "-11.28,0",
                                            "-5.33,0",
                                            "-8.31,0",  // area day2: min, max, mean
                                            "-10.44,0",
                                            "-9.97,0",
                                            "-10.21,0",  // inland day2 morning: min, max, mean
                                            "-9.62,0",
                                            "-8.78,0",
                                            "-9.20,0",  // coast day2 morning: min, max, mean
                                            "-10.55,0",
                                            "-9.40,0",
                                            "-9.98,0",  // area day2 morning: min, max, mean
                                            "-7.82,0",
                                            "-7.66,0",
                                            "-7.74,0",  // inland day2 afternoon: min, max, mean
                                            "-9.03,0",
                                            "-6.31,0",
                                            "-7.67,0",  // coast day2 afternoon: min, max, mean
                                            "-8.77,0",
                                            "-5.40,0",
                                            "-7.08,0",  // area day2 afternoon: min, max, mean
                                            "Päivälämpötila on -1...-5 astetta, huomenna on hieman "
                                            "kylmempää. Sisämaassa yölämpötila on noin -7 astetta, "
                                            "rannikolla -10 asteen tuntumassa."  // the story
                                            )));
      // #45
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam(
              "-5.39,0",
              "-2.44,0",
              "-3.91,0",  // inland day1: min, max, mean
              "-6.39,0",
              "-0.08,0",
              "-3.24,0",  // coast day1: min, max, mean
              "-5.92,0",
              "-2.09,0",
              "-4.01,0",  // area day1: min, max, mean
              "-5.39,0",
              "-4.97,0",
              "-5.18,0",  // inland day1 morning: min, max, mean
              "-6.39,0",
              "-5.95,0",
              "-6.17,0",  // coast day1 morning: min, max, mean
              "-5.92,0",
              "-3.99,0",
              "-4.96,0",  // area day1 morning: min, max, mean
              "-3.15,0",
              "-2.44,0",
              "-2.79,0",  // inland day1 afternoon: min, max, mean
              "-4.32,0",
              "-0.08,0",
              "-2.20,0",  // coast day1 afternoon: min, max, mean
              "-3.16,0",
              "-2.09,0",
              "-2.63,0",  // area day1 afternoon: min, max, mean
              "-8.90,0",
              "-8.79,0",
              "-8.85,0",  // inland night: min, max, mean
              "-9.57,0",
              "-8.14,0",
              "-8.85,0",  // coast night: min, max, mean
              "-9.78,0",
              "-7.99,0",
              "-8.88,0",  // area night: min, max, mean
              "-9.03,0",
              "-8.71,0",
              "-8.87,0",  // inland day2: min, max, mean
              "-8.97,0",
              "-5.26,0",
              "-7.12,0",  // coast day2: min, max, mean
              "-10.10,0",
              "-8.09,0",
              "-9.10,0",  // area day2: min, max, mean
              "-8.78,0",
              "-8.36,0",
              "-8.57,0",  // inland day2 morning: min, max, mean
              "-8.73,0",
              "-8.34,0",
              "-8.54,0",  // coast day2 morning: min, max, mean
              "-9.33,0",
              "-8.99,0",
              "-9.16,0",  // area day2 morning: min, max, mean
              "-8.81,0",
              "-8.80,0",
              "-8.81,0",  // inland day2 afternoon: min, max, mean
              "-9.80,0",
              "-6.18,0",
              "-7.99,0",  // coast day2 afternoon: min, max, mean
              "-8.96,0",
              "-8.22,0",
              "-8.59,0",  // area day2 afternoon: min, max, mean
              "Päivälämpötila on noin -3 astetta, päivällä pakkasta on vajaat 10 astetta. "
              "Yölämpötila on -8...-10 astetta."  // the story
              )));
      // #46
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam("-5.98,0",
                                            "-4.65,0",
                                            "-5.32,0",  // inland day1: min, max, mean
                                            "-6.81,0",
                                            "-4.67,0",
                                            "-5.74,0",  // coast day1: min, max, mean
                                            "-6.38,0",
                                            "-2.78,0",
                                            "-4.58,0",  // area day1: min, max, mean
                                            "-5.98,0",
                                            "-5.92,0",
                                            "-5.95,0",  // inland day1 morning: min, max, mean
                                            "-6.81,0",
                                            "-5.16,0",
                                            "-5.98,0",  // coast day1 morning: min, max, mean
                                            "-6.38,0",
                                            "-5.68,0",
                                            "-6.03,0",  // area day1 morning: min, max, mean
                                            "-5.35,0",
                                            "-4.65,0",
                                            "-5.00,0",  // inland day1 afternoon: min, max, mean
                                            "-5.48,0",
                                            "-4.67,0",
                                            "-5.07,0",  // coast day1 afternoon: min, max, mean
                                            "-5.46,0",
                                            "-2.78,0",
                                            "-4.12,0",  // area day1 afternoon: min, max, mean
                                            "-6.66,0",
                                            "-5.26,0",
                                            "-5.96,0",  // inland night: min, max, mean
                                            "-9.34,0",
                                            "-8.69,0",
                                            "-9.01,0",  // coast night: min, max, mean
                                            "-7.61,0",
                                            "-5.91,0",
                                            "-6.76,0",  // area night: min, max, mean
                                            "-9.15,0",
                                            "-6.74,0",
                                            "-7.94,0",  // inland day2: min, max, mean
                                            "-8.45,0",
                                            "-5.18,0",
                                            "-6.81,0",  // coast day2: min, max, mean
                                            "-10.59,0",
                                            "-3.16,0",
                                            "-6.87,0",  // area day2: min, max, mean
                                            "-8.77,0",
                                            "-8.04,0",
                                            "-8.40,0",  // inland day2 morning: min, max, mean
                                            "-7.90,0",
                                            "-7.59,0",
                                            "-7.74,0",  // coast day2 morning: min, max, mean
                                            "-9.60,0",
                                            "-9.30,0",
                                            "-9.45,0",  // area day2 morning: min, max, mean
                                            "-6.82,0",
                                            "-6.75,0",
                                            "-6.78,0",  // inland day2 afternoon: min, max, mean
                                            "-8.18,0",
                                            "-5.33,0",
                                            "-6.75,0",  // coast day2 afternoon: min, max, mean
                                            "-7.76,0",
                                            "-3.26,0",
                                            "-5.51,0",  // area day2 afternoon: min, max, mean
                                            "Lämpötila on -3...-5 astetta, huomenna suunnilleen "
                                            "sama. Sisämaassa yölämpötila on -5...-7 astetta, "
                                            "rannikolla -10 asteen tuntumassa."  // the story
                                            )));
      // #47
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-4.70,0",
                        "-1.87,0",
                        "-3.29,0",  // inland day1: min, max, mean
                        "-6.01,0",
                        "0.74,0",
                        "-2.63,0",  // coast day1: min, max, mean
                        "-5.28,0",
                        "-2.49,0",
                        "-3.89,0",  // area day1: min, max, mean
                        "-4.70,0",
                        "-4.32,0",
                        "-4.51,0",  // inland day1 morning: min, max, mean
                        "-6.01,0",
                        "-5.25,0",
                        "-5.63,0",  // coast day1 morning: min, max, mean
                        "-5.28,0",
                        "-3.42,0",
                        "-4.35,0",  // area day1 morning: min, max, mean
                        "-2.19,0",
                        "-1.87,0",
                        "-2.03,0",  // inland day1 afternoon: min, max, mean
                        "-3.87,0",
                        "0.74,0",
                        "-1.56,0",  // coast day1 afternoon: min, max, mean
                        "-2.80,0",
                        "-2.49,0",
                        "-2.64,0",  // area day1 afternoon: min, max, mean
                        "-7.30,0",
                        "-5.58,0",
                        "-6.44,0",  // inland night: min, max, mean
                        "-7.18,0",
                        "-6.81,0",
                        "-6.99,0",  // coast night: min, max, mean
                        "-5.48,0",
                        "-3.98,0",
                        "-4.73,0",  // area night: min, max, mean
                        "-6.28,0",
                        "-1.58,0",
                        "-3.93,0",  // inland day2: min, max, mean
                        "-6.38,0",
                        "-1.31,0",
                        "-3.84,0",  // coast day2: min, max, mean
                        "-6.78,0",
                        "0.07,0",
                        "-3.36,0",  // area day2: min, max, mean
                        "-5.94,0",
                        "-5.65,0",
                        "-5.79,0",  // inland day2 morning: min, max, mean
                        "-5.78,0",
                        "-4.90,0",
                        "-5.34,0",  // coast day2 morning: min, max, mean
                        "-6.49,0",
                        "-5.73,0",
                        "-6.11,0",  // area day2 morning: min, max, mean
                        "-3.35,0",
                        "-2.40,0",
                        "-2.88,0",  // inland day2 afternoon: min, max, mean
                        "-4.69,0",
                        "-2.02,0",
                        "-3.36,0",  // coast day2 afternoon: min, max, mean
                        "-3.40,0",
                        "-0.70,0",
                        "-2.05,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on noin -3 astetta, päivällä on pikkupakkasta."  // the story
                        )));
      // #48
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-4.16,0",
                        "-2.14,0",
                        "-3.15,0",  // inland day1: min, max, mean
                        "-5.40,0",
                        "-1.38,0",
                        "-3.39,0",  // coast day1: min, max, mean
                        "-5.11,0",
                        "-2.83,0",
                        "-3.97,0",  // area day1: min, max, mean
                        "-4.16,0",
                        "-3.20,0",
                        "-3.68,0",  // inland day1 morning: min, max, mean
                        "-5.40,0",
                        "-5.33,0",
                        "-5.36,0",  // coast day1 morning: min, max, mean
                        "-5.11,0",
                        "-3.34,0",
                        "-4.22,0",  // area day1 morning: min, max, mean
                        "-2.62,0",
                        "-2.14,0",
                        "-2.38,0",  // inland day1 afternoon: min, max, mean
                        "-3.17,0",
                        "-1.38,0",
                        "-2.28,0",  // coast day1 afternoon: min, max, mean
                        "-3.22,0",
                        "-2.83,0",
                        "-3.03,0",  // area day1 afternoon: min, max, mean
                        "-9.07,0",
                        "-8.99,0",
                        "-9.03,0",  // inland night: min, max, mean
                        "-7.30,0",
                        "-6.59,0",
                        "-6.94,0",  // coast night: min, max, mean
                        "-6.14,0",
                        "-5.51,0",
                        "-5.83,0",  // area night: min, max, mean
                        "-5.72,0",
                        "-0.44,0",
                        "-3.08,0",  // inland day2: min, max, mean
                        "-4.58,0",
                        "-0.38,0",
                        "-2.48,0",  // coast day2: min, max, mean
                        "-5.59,0",
                        "2.09,0",
                        "-1.75,0",  // area day2: min, max, mean
                        "-4.75,0",
                        "-4.51,0",
                        "-4.63,0",  // inland day2 morning: min, max, mean
                        "-4.25,0",
                        "-3.93,0",
                        "-4.09,0",  // coast day2 morning: min, max, mean
                        "-5.37,0",
                        "-3.74,0",
                        "-4.56,0",  // area day2 morning: min, max, mean
                        "-1.60,0",
                        "-0.79,0",
                        "-1.20,0",  // inland day2 afternoon: min, max, mean
                        "-2.63,0",
                        "-0.90,0",
                        "-1.77,0",  // coast day2 afternoon: min, max, mean
                        "-1.71,0",
                        "1.65,0",
                        "-0.03,0",  // area day2 afternoon: min, max, mean
                        "Lämpötila on noin -3 astetta, huomenna nollan tienoilla."  // the story
                        )));
      // #49
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam("-4.49,0",
                                            "-3.22,0",
                                            "-3.86,0",  // inland day1: min, max, mean
                                            "-5.36,0",
                                            "-1.54,0",
                                            "-3.45,0",  // coast day1: min, max, mean
                                            "-5.11,0",
                                            "-2.87,0",
                                            "-3.99,0",  // area day1: min, max, mean
                                            "-4.49,0",
                                            "-3.59,0",
                                            "-4.04,0",  // inland day1 morning: min, max, mean
                                            "-5.36,0",
                                            "-4.71,0",
                                            "-5.04,0",  // coast day1 morning: min, max, mean
                                            "-5.11,0",
                                            "-4.74,0",
                                            "-4.92,0",  // area day1 morning: min, max, mean
                                            "-4.12,0",
                                            "-3.22,0",
                                            "-3.67,0",  // inland day1 afternoon: min, max, mean
                                            "-5.35,0",
                                            "-1.54,0",
                                            "-3.45,0",  // coast day1 afternoon: min, max, mean
                                            "-4.54,0",
                                            "-2.87,0",
                                            "-3.71,0",  // area day1 afternoon: min, max, mean
                                            "-4.88,0",
                                            "-3.51,0",
                                            "-4.20,0",  // inland night: min, max, mean
                                            "-8.21,0",
                                            "-6.92,0",
                                            "-7.56,0",  // coast night: min, max, mean
                                            "-5.99,0",
                                            "-5.80,0",
                                            "-5.90,0",  // area night: min, max, mean
                                            "-0.98,0",
                                            "3.65,0",
                                            "1.33,0",  // inland day2: min, max, mean
                                            "0.31,0",
                                            "2.05,0",
                                            "1.18,0",  // coast day2: min, max, mean
                                            "-1.54,0",
                                            "3.42,0",
                                            "0.94,0",  // area day2: min, max, mean
                                            "-0.38,0",
                                            "-0.14,0",
                                            "-0.26,0",  // inland day2 morning: min, max, mean
                                            "0.37,0",
                                            "1.64,0",
                                            "1.01,0",  // coast day2 morning: min, max, mean
                                            "-0.82,0",
                                            "1.18,0",
                                            "0.18,0",  // area day2 morning: min, max, mean
                                            "1.89,0",
                                            "2.65,0",
                                            "2.27,0",  // inland day2 afternoon: min, max, mean
                                            "0.84,0",
                                            "1.11,0",
                                            "0.98,0",  // coast day2 afternoon: min, max, mean
                                            "1.35,0",
                                            "2.81,0",
                                            "2.08,0",  // area day2 afternoon: min, max, mean
                                            "Lämpötila on -3...-5 astetta, huomenna 1...3 astetta. "
                                            "Sisämaassa yölämpötila on -5 asteen tuntumassa, "
                                            "rannikolla noin -8 astetta."  // the story
                                            )));
      // #50
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-3.45,0",
                                  "-2.81,0",
                                  "-3.13,0",  // inland day1: min, max, mean
                                  "-4.64,0",
                                  "-2.94,0",
                                  "-3.79,0",  // coast day1: min, max, mean
                                  "-4.41,0",
                                  "1.22,0",
                                  "-1.59,0",  // area day1: min, max, mean
                                  "-3.45,0",
                                  "-3.21,0",
                                  "-3.33,0",  // inland day1 morning: min, max, mean
                                  "-4.64,0",
                                  "-3.27,0",
                                  "-3.95,0",  // coast day1 morning: min, max, mean
                                  "-4.41,0",
                                  "-3.80,0",
                                  "-4.10,0",  // area day1 morning: min, max, mean
                                  "-3.21,0",
                                  "-2.81,0",
                                  "-3.01,0",  // inland day1 afternoon: min, max, mean
                                  "-4.09,0",
                                  "-2.94,0",
                                  "-3.52,0",  // coast day1 afternoon: min, max, mean
                                  "-3.33,0",
                                  "1.22,0",
                                  "-1.06,0",  // area day1 afternoon: min, max, mean
                                  "-6.31,0",
                                  "-5.78,0",
                                  "-6.05,0",  // inland night: min, max, mean
                                  "-6.15,0",
                                  "-4.67,0",
                                  "-5.41,0",  // coast night: min, max, mean
                                  "-7.39,0",
                                  "-6.87,0",
                                  "-7.13,0",  // area night: min, max, mean
                                  "-5.10,0",
                                  "-3.67,0",
                                  "-4.39,0",  // inland day2: min, max, mean
                                  "-4.72,0",
                                  "-3.09,0",
                                  "-3.91,0",  // coast day2: min, max, mean
                                  "-6.15,0",
                                  "-1.32,0",
                                  "-3.74,0",  // area day2: min, max, mean
                                  "-4.70,0",
                                  "-4.04,0",
                                  "-4.37,0",  // inland day2 morning: min, max, mean
                                  "-4.18,0",
                                  "-3.66,0",
                                  "-3.92,0",  // coast day2 morning: min, max, mean
                                  "-5.68,0",
                                  "-5.09,0",
                                  "-5.38,0",  // area day2 morning: min, max, mean
                                  "-4.45,0",
                                  "-4.07,0",
                                  "-4.26,0",  // inland day2 afternoon: min, max, mean
                                  "-4.96,0",
                                  "-3.99,0",
                                  "-4.47,0",  // coast day2 afternoon: min, max, mean
                                  "-4.53,0",
                                  "-1.91,0",
                                  "-3.22,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -3...+1 astetta, huomenna suunnilleen sama. "
                                  "Yölämpötila on noin -7 astetta."  // the story
                                  )));
      // #51
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-3.72,0",
                                  "-2.10,0",
                                  "-2.91,0",  // inland day1: min, max, mean
                                  "-5.07,0",
                                  "1.41,0",
                                  "-1.83,0",  // coast day1: min, max, mean
                                  "-4.42,0",
                                  "-1.80,0",
                                  "-3.11,0",  // area day1: min, max, mean
                                  "-3.72,0",
                                  "-3.32,0",
                                  "-3.52,0",  // inland day1 morning: min, max, mean
                                  "-5.07,0",
                                  "-4.50,0",
                                  "-4.79,0",  // coast day1 morning: min, max, mean
                                  "-4.42,0",
                                  "-3.79,0",
                                  "-4.11,0",  // area day1 morning: min, max, mean
                                  "-2.67,0",
                                  "-2.10,0",
                                  "-2.39,0",  // inland day1 afternoon: min, max, mean
                                  "-3.31,0",
                                  "1.41,0",
                                  "-0.95,0",  // coast day1 afternoon: min, max, mean
                                  "-2.93,0",
                                  "-1.80,0",
                                  "-2.36,0",  // area day1 afternoon: min, max, mean
                                  "-7.30,0",
                                  "-6.32,0",
                                  "-6.81,0",  // inland night: min, max, mean
                                  "-8.17,0",
                                  "-7.97,0",
                                  "-8.07,0",  // coast night: min, max, mean
                                  "-5.10,0",
                                  "-5.02,0",
                                  "-5.06,0",  // area night: min, max, mean
                                  "-8.22,0",
                                  "-5.18,0",
                                  "-6.70,0",  // inland day2: min, max, mean
                                  "-8.23,0",
                                  "-2.33,0",
                                  "-5.28,0",  // coast day2: min, max, mean
                                  "-9.23,0",
                                  "-2.95,0",
                                  "-6.09,0",  // area day2: min, max, mean
                                  "-7.70,0",
                                  "-7.42,0",
                                  "-7.56,0",  // inland day2 morning: min, max, mean
                                  "-7.48,0",
                                  "-7.01,0",
                                  "-7.25,0",  // coast day2 morning: min, max, mean
                                  "-8.26,0",
                                  "-7.54,0",
                                  "-7.90,0",  // area day2 morning: min, max, mean
                                  "-5.91,0",
                                  "-5.77,0",
                                  "-5.84,0",  // inland day2 afternoon: min, max, mean
                                  "-5.96,0",
                                  "-2.51,0",
                                  "-4.23,0",  // coast day2 afternoon: min, max, mean
                                  "-6.21,0",
                                  "-3.30,0",
                                  "-4.76,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on noin -2 astetta, huomenna suunnilleen sama. "
                                  "Yölämpötila on noin -5 astetta."  // the story
                                  )));
      // #52
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-3.15,0",
                                  "-1.44,0",
                                  "-2.29,0",  // inland day1: min, max, mean
                                  "-3.77,0",
                                  "-1.72,0",
                                  "-2.75,0",  // coast day1: min, max, mean
                                  "-3.49,0",
                                  "2.07,0",
                                  "-0.71,0",  // area day1: min, max, mean
                                  "-3.15,0",
                                  "-2.95,0",
                                  "-3.05,0",  // inland day1 morning: min, max, mean
                                  "-3.77,0",
                                  "-2.81,0",
                                  "-3.29,0",  // coast day1 morning: min, max, mean
                                  "-3.49,0",
                                  "-2.32,0",
                                  "-2.90,0",  // area day1 morning: min, max, mean
                                  "-2.08,0",
                                  "-1.44,0",
                                  "-1.76,0",  // inland day1 afternoon: min, max, mean
                                  "-3.36,0",
                                  "-1.72,0",
                                  "-2.54,0",  // coast day1 afternoon: min, max, mean
                                  "-2.22,0",
                                  "2.07,0",
                                  "-0.08,0",  // area day1 afternoon: min, max, mean
                                  "-6.35,0",
                                  "-5.56,0",
                                  "-5.96,0",  // inland night: min, max, mean
                                  "-6.55,0",
                                  "-6.40,0",
                                  "-6.48,0",  // coast night: min, max, mean
                                  "-3.59,0",
                                  "-2.62,0",
                                  "-3.10,0",  // area night: min, max, mean
                                  "-7.80,0",
                                  "-3.57,0",
                                  "-5.68,0",  // inland day2: min, max, mean
                                  "-7.46,0",
                                  "-4.23,0",
                                  "-5.84,0",  // coast day2: min, max, mean
                                  "-8.02,0",
                                  "-3.01,0",
                                  "-5.52,0",  // area day2: min, max, mean
                                  "-6.84,0",
                                  "-6.33,0",
                                  "-6.59,0",  // inland day2 morning: min, max, mean
                                  "-6.75,0",
                                  "-5.62,0",
                                  "-6.19,0",  // coast day2 morning: min, max, mean
                                  "-7.09,0",
                                  "-6.62,0",
                                  "-6.86,0",  // area day2 morning: min, max, mean
                                  "-3.92,0",
                                  "-3.82,0",
                                  "-3.87,0",  // inland day2 afternoon: min, max, mean
                                  "-5.87,0",
                                  "-5.20,0",
                                  "-5.53,0",  // coast day2 afternoon: min, max, mean
                                  "-4.66,0",
                                  "-3.89,0",
                                  "-4.28,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on -2...+2 astetta, huomenna on hieman "
                                  "kylmempää. Yölämpötila on noin -3 astetta."  // the story
                                  )));
      // #53
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-3.03,0",
                                  "0.02,0",
                                  "-1.51,0",  // inland day1: min, max, mean
                                  "-4.63,0",
                                  "2.81,0",
                                  "-0.91,0",  // coast day1: min, max, mean
                                  "-3.12,0",
                                  "0.21,0",
                                  "-1.46,0",  // area day1: min, max, mean
                                  "-3.03,0",
                                  "-2.13,0",
                                  "-2.58,0",  // inland day1 morning: min, max, mean
                                  "-4.63,0",
                                  "-3.51,0",
                                  "-4.07,0",  // coast day1 morning: min, max, mean
                                  "-3.12,0",
                                  "-1.60,0",
                                  "-2.36,0",  // area day1 morning: min, max, mean
                                  "-0.81,0",
                                  "0.02,0",
                                  "-0.40,0",  // inland day1 afternoon: min, max, mean
                                  "-0.97,0",
                                  "2.81,0",
                                  "0.92,0",  // coast day1 afternoon: min, max, mean
                                  "-0.87,0",
                                  "0.21,0",
                                  "-0.33,0",  // area day1 afternoon: min, max, mean
                                  "-4.99,0",
                                  "-3.01,0",
                                  "-4.00,0",  // inland night: min, max, mean
                                  "-7.58,0",
                                  "-6.18,0",
                                  "-6.88,0",  // coast night: min, max, mean
                                  "-3.18,0",
                                  "-2.89,0",
                                  "-3.03,0",  // area night: min, max, mean
                                  "-3.40,0",
                                  "-1.71,0",
                                  "-2.56,0",  // inland day2: min, max, mean
                                  "-3.28,0",
                                  "0.30,0",
                                  "-1.49,0",  // coast day2: min, max, mean
                                  "-3.83,0",
                                  "1.59,0",
                                  "-1.12,0",  // area day2: min, max, mean
                                  "-2.73,0",
                                  "-1.77,0",
                                  "-2.25,0",  // inland day2 morning: min, max, mean
                                  "-2.54,0",
                                  "-0.87,0",
                                  "-1.71,0",  // coast day2 morning: min, max, mean
                                  "-3.72,0",
                                  "-2.94,0",
                                  "-3.33,0",  // area day2 morning: min, max, mean
                                  "-2.86,0",
                                  "-2.07,0",
                                  "-2.46,0",  // inland day2 afternoon: min, max, mean
                                  "-3.82,0",
                                  "-0.54,0",
                                  "-2.18,0",  // coast day2 afternoon: min, max, mean
                                  "-2.89,0",
                                  "1.58,0",
                                  "-0.65,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on nollan tienoilla, huomenna suunnilleen sama. "
                                  "Yölämpötila on noin -3 astetta."  // the story
                                  )));
      // #54
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam("-1.98,0",
                                            "0.73,0",
                                            "-0.62,0",  // inland day1: min, max, mean
                                            "-3.62,0",
                                            "3.56,0",
                                            "-0.03,0",  // coast day1: min, max, mean
                                            "-2.65,0",
                                            "3.36,0",
                                            "0.36,0",  // area day1: min, max, mean
                                            "-1.98,0",
                                            "-1.80,0",
                                            "-1.89,0",  // inland day1 morning: min, max, mean
                                            "-3.62,0",
                                            "-2.78,0",
                                            "-3.20,0",  // coast day1 morning: min, max, mean
                                            "-2.65,0",
                                            "-0.84,0",
                                            "-1.74,0",  // area day1 morning: min, max, mean
                                            "0.42,0",
                                            "0.73,0",
                                            "0.58,0",  // inland day1 afternoon: min, max, mean
                                            "-1.31,0",
                                            "3.56,0",
                                            "1.12,0",  // coast day1 afternoon: min, max, mean
                                            "-0.52,0",
                                            "3.36,0",
                                            "1.42,0",  // area day1 afternoon: min, max, mean
                                            "-3.32,0",
                                            "-2.81,0",
                                            "-3.07,0",  // inland night: min, max, mean
                                            "-6.73,0",
                                            "-6.42,0",
                                            "-6.57,0",  // coast night: min, max, mean
                                            "-2.65,0",
                                            "-1.72,0",
                                            "-2.19,0",  // area night: min, max, mean
                                            "0.32,0",
                                            "5.35,0",
                                            "2.84,0",  // inland day2: min, max, mean
                                            "0.99,0",
                                            "6.12,0",
                                            "3.56,0",  // coast day2: min, max, mean
                                            "0.70,0",
                                            "7.20,0",
                                            "3.95,0",  // area day2: min, max, mean
                                            "0.83,0",
                                            "1.27,0",
                                            "1.05,0",  // inland day2 morning: min, max, mean
                                            "1.18,0",
                                            "1.61,0",
                                            "1.39,0",  // coast day2 morning: min, max, mean
                                            "0.73,0",
                                            "2.15,0",
                                            "1.44,0",  // area day2 morning: min, max, mean
                                            "4.22,0",
                                            "4.54,0",
                                            "4.38,0",  // inland day2 afternoon: min, max, mean
                                            "2.31,0",
                                            "5.68,0",
                                            "3.99,0",  // coast day2 afternoon: min, max, mean
                                            "3.93,0",
                                            "6.88,0",
                                            "5.41,0",  // area day2 afternoon: min, max, mean
                                            "Päivälämpötila on -1...+3 astetta, huomenna 5 asteen "
                                            "tienoilla. Sisämaassa yölämpötila on noin -3 astetta, "
                                            "rannikolla vähän yli -5 astetta."  // the story
                                            )));
      // #55
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-1.36,0",
                        "0.09,0",
                        "-0.64,0",  // inland day1: min, max, mean
                        "-3.22,0",
                        "0.13,0",
                        "-1.55,0",  // coast day1: min, max, mean
                        "-1.78,0",
                        "2.24,0",
                        "0.23,0",  // area day1: min, max, mean
                        "-1.36,0",
                        "-1.28,0",
                        "-1.32,0",  // inland day1 morning: min, max, mean
                        "-3.22,0",
                        "-2.14,0",
                        "-2.68,0",  // coast day1 morning: min, max, mean
                        "-1.78,0",
                        "-0.91,0",
                        "-1.35,0",  // area day1 morning: min, max, mean
                        "-0.56,0",
                        "0.09,0",
                        "-0.24,0",  // inland day1 afternoon: min, max, mean
                        "-0.94,0",
                        "0.13,0",
                        "-0.40,0",  // coast day1 afternoon: min, max, mean
                        "-0.62,0",
                        "2.24,0",
                        "0.81,0",  // area day1 afternoon: min, max, mean
                        "-4.28,0",
                        "-3.24,0",
                        "-3.76,0",  // inland night: min, max, mean
                        "-7.15,0",
                        "-7.01,0",
                        "-7.08,0",  // coast night: min, max, mean
                        "-1.88,0",
                        "-1.56,0",
                        "-1.72,0",  // area night: min, max, mean
                        "-3.62,0",
                        "0.52,0",
                        "-1.55,0",  // inland day2: min, max, mean
                        "-3.40,0",
                        "0.55,0",
                        "-1.42,0",  // coast day2: min, max, mean
                        "-3.98,0",
                        "4.19,0",
                        "0.11,0",  // area day2: min, max, mean
                        "-3.07,0",
                        "-2.64,0",
                        "-2.86,0",  // inland day2 morning: min, max, mean
                        "-2.97,0",
                        "-2.44,0",
                        "-2.70,0",  // coast day2 morning: min, max, mean
                        "-3.48,0",
                        "-2.37,0",
                        "-2.92,0",  // area day2 morning: min, max, mean
                        "-0.11,0",
                        "0.49,0",
                        "0.19,0",  // inland day2 afternoon: min, max, mean
                        "-1.51,0",
                        "-0.26,0",
                        "-0.88,0",  // coast day2 afternoon: min, max, mean
                        "-0.26,0",
                        "3.65,0",
                        "1.69,0",  // area day2 afternoon: min, max, mean
                        "Päivälämpötila on -1...+2 astetta, huomenna suunnilleen sama. Sisämaassa "
                        "yölämpötila on noin -4 astetta, rannikolla noin -7 astetta."  // the story
                        )));
      // #56
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-0.78,0",
                        "1.14,0",
                        "0.18,0",  // inland day1: min, max, mean
                        "-0.93,0",
                        "-0.91,0",
                        "-0.92,0",  // coast day1: min, max, mean
                        "-1.50,0",
                        "2.15,0",
                        "0.32,0",  // area day1: min, max, mean
                        "-0.78,0",
                        "0.14,0",
                        "-0.32,0",  // inland day1 morning: min, max, mean
                        "-0.93,0",
                        "0.46,0",
                        "-0.23,0",  // coast day1 morning: min, max, mean
                        "-1.50,0",
                        "-0.02,0",
                        "-0.76,0",  // area day1 morning: min, max, mean
                        "0.78,0",
                        "1.14,0",
                        "0.96,0",  // inland day1 afternoon: min, max, mean
                        "-0.95,0",
                        "-0.91,0",
                        "-0.93,0",  // coast day1 afternoon: min, max, mean
                        "0.66,0",
                        "2.15,0",
                        "1.40,0",  // area day1 afternoon: min, max, mean
                        "-3.35,0",
                        "-2.53,0",
                        "-2.94,0",  // inland night: min, max, mean
                        "-4.11,0",
                        "-3.05,0",
                        "-3.58,0",  // coast night: min, max, mean
                        "-2.96,0",
                        "-1.95,0",
                        "-2.46,0",  // area night: min, max, mean
                        "-1.54,0",
                        "1.49,0",
                        "-0.02,0",  // inland day2: min, max, mean
                        "-1.82,0",
                        "1.32,0",
                        "-0.25,0",  // coast day2: min, max, mean
                        "-2.56,0",
                        "1.48,0",
                        "-0.54,0",  // area day2: min, max, mean
                        "-1.29,0",
                        "-0.41,0",
                        "-0.85,0",  // inland day2 morning: min, max, mean
                        "-1.09,0",
                        "0.33,0",
                        "-0.38,0",  // coast day2 morning: min, max, mean
                        "-2.13,0",
                        "-0.40,0",
                        "-1.26,0",  // area day2 morning: min, max, mean
                        "0.04,0",
                        "1.01,0",
                        "0.52,0",  // inland day2 afternoon: min, max, mean
                        "-0.06,0",
                        "0.74,0",
                        "0.34,0",  // coast day2 afternoon: min, max, mean
                        "-0.09,0",
                        "0.56,0",
                        "0.23,0",  // area day2 afternoon: min, max, mean
                        "Päivälämpötila on vähän plussan puolella, huomenna nollan tienoilla tai "
                        "vähän plussan puolella. Yölämpötila on noin -2 astetta."  // the story
                        )));
      // #57
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-0.22,0",
                                  "1.51,0",
                                  "0.65,0",  // inland day1: min, max, mean
                                  "-0.43,0",
                                  "2.68,0",
                                  "1.12,0",  // coast day1: min, max, mean
                                  "-0.55,0",
                                  "1.43,0",
                                  "0.44,0",  // area day1: min, max, mean
                                  "-0.22,0",
                                  "0.06,0",
                                  "-0.08,0",  // inland day1 morning: min, max, mean
                                  "-0.43,0",
                                  "1.15,0",
                                  "0.36,0",  // coast day1 morning: min, max, mean
                                  "-0.55,0",
                                  "-0.10,0",
                                  "-0.32,0",  // area day1 morning: min, max, mean
                                  "1.17,0",
                                  "1.51,0",
                                  "1.34,0",  // inland day1 afternoon: min, max, mean
                                  "-0.44,0",
                                  "2.68,0",
                                  "1.12,0",  // coast day1 afternoon: min, max, mean
                                  "0.54,0",
                                  "1.43,0",
                                  "0.98,0",  // area day1 afternoon: min, max, mean
                                  "-4.24,0",
                                  "-3.44,0",
                                  "-3.84,0",  // inland night: min, max, mean
                                  "-2.28,0",
                                  "-1.03,0",
                                  "-1.65,0",  // coast night: min, max, mean
                                  "-4.53,0",
                                  "-3.96,0",
                                  "-4.24,0",  // area night: min, max, mean
                                  "2.70,0",
                                  "6.46,0",
                                  "4.58,0",  // inland day2: min, max, mean
                                  "3.08,0",
                                  "4.53,0",
                                  "3.80,0",  // coast day2: min, max, mean
                                  "1.61,0",
                                  "8.87,0",
                                  "5.24,0",  // area day2: min, max, mean
                                  "2.84,0",
                                  "2.85,0",
                                  "2.85,0",  // inland day2 morning: min, max, mean
                                  "3.48,0",
                                  "5.04,0",
                                  "4.26,0",  // coast day2 morning: min, max, mean
                                  "2.06,0",
                                  "4.05,0",
                                  "3.05,0",  // area day2 morning: min, max, mean
                                  "5.83,0",
                                  "6.10,0",
                                  "5.97,0",  // inland day2 afternoon: min, max, mean
                                  "3.95,0",
                                  "3.98,0",
                                  "3.96,0",  // coast day2 afternoon: min, max, mean
                                  "5.04,0",
                                  "8.55,0",
                                  "6.79,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on vähän plussan puolella, huomenna 5...9 "
                                  "astetta. Yölämpötila on -5 asteen tuntumassa."  // the story
                                  )));
      // #58
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "-0.07,0",
                        "3.75,0",
                        "1.84,0",  // inland day1: min, max, mean
                        "-0.39,0",
                        "1.11,0",
                        "0.36,0",  // coast day1: min, max, mean
                        "-0.38,0",
                        "2.72,0",
                        "1.17,0",  // area day1: min, max, mean
                        "-0.07,0",
                        "0.53,0",
                        "0.23,0",  // inland day1 morning: min, max, mean
                        "-0.39,0",
                        "1.00,0",
                        "0.30,0",  // coast day1 morning: min, max, mean
                        "-0.38,0",
                        "1.46,0",
                        "0.54,0",  // area day1 morning: min, max, mean
                        "2.80,0",
                        "3.75,0",
                        "3.28,0",  // inland day1 afternoon: min, max, mean
                        "1.07,0",
                        "1.11,0",
                        "1.09,0",  // coast day1 afternoon: min, max, mean
                        "2.34,0",
                        "2.72,0",
                        "2.53,0",  // area day1 afternoon: min, max, mean
                        "-3.92,0",
                        "-3.89,0",
                        "-3.90,0",  // inland night: min, max, mean
                        "-1.34,0",
                        "0.20,0",
                        "-0.57,0",  // coast night: min, max, mean
                        "-1.72,0",
                        "-0.14,0",
                        "-0.93,0",  // area night: min, max, mean
                        "-2.71,0",
                        "-0.38,0",
                        "-1.55,0",  // inland day2: min, max, mean
                        "-1.82,0",
                        "1.78,0",
                        "-0.02,0",  // coast day2: min, max, mean
                        "-2.82,0",
                        "0.91,0",
                        "-0.95,0",  // area day2: min, max, mean
                        "-2.07,0",
                        "-1.45,0",
                        "-1.76,0",  // inland day2 morning: min, max, mean
                        "-1.12,0",
                        "0.00,0",
                        "-0.56,0",  // coast day2 morning: min, max, mean
                        "-2.75,0",
                        "-2.04,0",
                        "-2.39,0",  // area day2 morning: min, max, mean
                        "-1.04,0",
                        "-0.75,0",
                        "-0.90,0",  // inland day2 afternoon: min, max, mean
                        "-2.12,0",
                        "1.50,0",
                        "-0.31,0",  // coast day2 afternoon: min, max, mean
                        "-1.56,0",
                        "-0.06,0",
                        "-0.81,0",  // area day2 afternoon: min, max, mean
                        "Päivälämpötila on noin 3 astetta, huomenna 0...-2 astetta. Sisämaassa "
                        "yölämpötila on noin -4 astetta, rannikolla nollan tienoilla."  // the story
                        )));
      // #59
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("0.31,0",
                                  "0.69,0",
                                  "0.50,0",  // inland day1: min, max, mean
                                  "-1.62,0",
                                  "4.41,0",
                                  "1.39,0",  // coast day1: min, max, mean
                                  "0.00,0",
                                  "4.10,0",
                                  "2.05,0",  // area day1: min, max, mean
                                  "0.31,0",
                                  "0.50,0",
                                  "0.40,0",  // inland day1 morning: min, max, mean
                                  "-1.62,0",
                                  "-1.50,0",
                                  "-1.56,0",  // coast day1 morning: min, max, mean
                                  "0.00,0",
                                  "0.44,0",
                                  "0.22,0",  // area day1 morning: min, max, mean
                                  "0.51,0",
                                  "0.69,0",
                                  "0.60,0",  // inland day1 afternoon: min, max, mean
                                  "0.20,0",
                                  "4.41,0",
                                  "2.31,0",  // coast day1 afternoon: min, max, mean
                                  "0.49,0",
                                  "4.10,0",
                                  "2.30,0",  // area day1 afternoon: min, max, mean
                                  "-2.34,0",
                                  "-1.17,0",
                                  "-1.75,0",  // inland night: min, max, mean
                                  "-4.09,0",
                                  "-2.92,0",
                                  "-3.51,0",  // coast night: min, max, mean
                                  "-1.24,0",
                                  "-0.14,0",
                                  "-0.69,0",  // area night: min, max, mean
                                  "0.46,0",
                                  "7.78,0",
                                  "4.12,0",  // inland day2: min, max, mean
                                  "1.89,0",
                                  "6.69,0",
                                  "4.29,0",  // coast day2: min, max, mean
                                  "0.39,0",
                                  "7.73,0",
                                  "4.06,0",  // area day2: min, max, mean
                                  "1.34,0",
                                  "1.51,0",
                                  "1.42,0",  // inland day2 morning: min, max, mean
                                  "2.31,0",
                                  "2.74,0",
                                  "2.53,0",  // coast day2 morning: min, max, mean
                                  "0.86,0",
                                  "2.69,0",
                                  "1.77,0",  // area day2 morning: min, max, mean
                                  "6.07,0",
                                  "6.80,0",
                                  "6.43,0",  // inland day2 afternoon: min, max, mean
                                  "4.81,0",
                                  "5.70,0",
                                  "5.26,0",  // coast day2 afternoon: min, max, mean
                                  "5.29,0",
                                  "7.17,0",
                                  "6.23,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on 0...+4 astetta, huomenna hieman korkeampi. "
                                  "Yölämpötila on 0...-1 astetta."  // the story
                                  )));
      // #60
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-0.56,0",
                                  "1.89,0",
                                  "0.67,0",  // inland day1: min, max, mean
                                  "-1.73,0",
                                  "2.08,0",
                                  "0.17,0",  // coast day1: min, max, mean
                                  "-0.57,0",
                                  "1.79,0",
                                  "0.61,0",  // area day1: min, max, mean
                                  "-0.56,0",
                                  "-0.29,0",
                                  "-0.42,0",  // inland day1 morning: min, max, mean
                                  "-1.73,0",
                                  "-1.56,0",
                                  "-1.65,0",  // coast day1 morning: min, max, mean
                                  "-0.57,0",
                                  "0.40,0",
                                  "-0.09,0",  // area day1 morning: min, max, mean
                                  "1.71,0",
                                  "1.89,0",
                                  "1.80,0",  // inland day1 afternoon: min, max, mean
                                  "0.28,0",
                                  "2.08,0",
                                  "1.18,0",  // coast day1 afternoon: min, max, mean
                                  "1.01,0",
                                  "1.79,0",
                                  "1.40,0",  // area day1 afternoon: min, max, mean
                                  "-1.81,0",
                                  "-0.52,0",
                                  "-1.17,0",  // inland night: min, max, mean
                                  "-4.47,0",
                                  "-2.86,0",
                                  "-3.67,0",  // coast night: min, max, mean
                                  "-2.99,0",
                                  "-1.09,0",
                                  "-2.04,0",  // area night: min, max, mean
                                  "-3.44,0",
                                  "-1.35,0",
                                  "-2.39,0",  // inland day2: min, max, mean
                                  "-2.94,0",
                                  "-0.10,0",
                                  "-1.52,0",  // coast day2: min, max, mean
                                  "-4.21,0",
                                  "1.58,0",
                                  "-1.32,0",  // area day2: min, max, mean
                                  "-3.04,0",
                                  "-2.49,0",
                                  "-2.77,0",  // inland day2 morning: min, max, mean
                                  "-2.23,0",
                                  "-0.41,0",
                                  "-1.32,0",  // coast day2 morning: min, max, mean
                                  "-3.60,0",
                                  "-3.12,0",
                                  "-3.36,0",  // area day2 morning: min, max, mean
                                  "-2.28,0",
                                  "-1.53,0",
                                  "-1.90,0",  // inland day2 afternoon: min, max, mean
                                  "-3.34,0",
                                  "-0.75,0",
                                  "-2.05,0",  // coast day2 afternoon: min, max, mean
                                  "-2.84,0",
                                  "1.18,0",
                                  "-0.83,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on vähän plussan puolella, huomenna suunnilleen "
                                  "sama. Yölämpötila on -1...-3 astetta."  // the story
                                  )));
      // #61
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("-0.24,0",
                                  "1.05,0",
                                  "0.41,0",  // inland day1: min, max, mean
                                  "-1.80,0",
                                  "1.07,0",
                                  "-0.37,0",  // coast day1: min, max, mean
                                  "-0.25,0",
                                  "3.20,0",
                                  "1.47,0",  // area day1: min, max, mean
                                  "-0.24,0",
                                  "0.57,0",
                                  "0.16,0",  // inland day1 morning: min, max, mean
                                  "-1.80,0",
                                  "-0.14,0",
                                  "-0.97,0",  // coast day1 morning: min, max, mean
                                  "-0.25,0",
                                  "-0.11,0",
                                  "-0.18,0",  // area day1 morning: min, max, mean
                                  "0.71,0",
                                  "1.05,0",
                                  "0.88,0",  // inland day1 afternoon: min, max, mean
                                  "0.51,0",
                                  "1.07,0",
                                  "0.79,0",  // coast day1 afternoon: min, max, mean
                                  "0.26,0",
                                  "3.20,0",
                                  "1.73,0",  // area day1 afternoon: min, max, mean
                                  "-4.59,0",
                                  "-2.87,0",
                                  "-3.73,0",  // inland night: min, max, mean
                                  "-3.73,0",
                                  "-2.18,0",
                                  "-2.95,0",  // coast night: min, max, mean
                                  "-2.93,0",
                                  "-0.97,0",
                                  "-1.95,0",  // area night: min, max, mean
                                  "-3.12,0",
                                  "1.56,0",
                                  "-0.78,0",  // inland day2: min, max, mean
                                  "-2.34,0",
                                  "1.07,0",
                                  "-0.63,0",  // coast day2: min, max, mean
                                  "-2.61,0",
                                  "5.80,0",
                                  "1.60,0",  // area day2: min, max, mean
                                  "-2.16,0",
                                  "-1.65,0",
                                  "-1.90,0",  // inland day2 morning: min, max, mean
                                  "-1.95,0",
                                  "-1.27,0",
                                  "-1.61,0",  // coast day2 morning: min, max, mean
                                  "-2.21,0",
                                  "-1.56,0",
                                  "-1.89,0",  // area day2 morning: min, max, mean
                                  "0.80,0",
                                  "1.10,0",
                                  "0.95,0",  // inland day2 afternoon: min, max, mean
                                  "-0.54,0",
                                  "0.26,0",
                                  "-0.14,0",  // coast day2 afternoon: min, max, mean
                                  "0.25,0",
                                  "5.09,0",
                                  "2.67,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on 0...+3 astetta, huomenna suunnilleen sama. "
                                  "Yölämpötila on -1...-3 astetta."  // the story
                                  )));
      // #62
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("1.32,0",
                                  "3.12,0",
                                  "2.22,0",  // inland day1: min, max, mean
                                  "0.08,0",
                                  "5.25,0",
                                  "2.66,0",  // coast day1: min, max, mean
                                  "0.58,0",
                                  "4.67,0",
                                  "2.63,0",  // area day1: min, max, mean
                                  "1.32,0",
                                  "1.50,0",
                                  "1.41,0",  // inland day1 morning: min, max, mean
                                  "0.08,0",
                                  "1.15,0",
                                  "0.61,0",  // coast day1 morning: min, max, mean
                                  "0.58,0",
                                  "0.86,0",
                                  "0.72,0",  // area day1 morning: min, max, mean
                                  "2.73,0",
                                  "3.12,0",
                                  "2.93,0",  // inland day1 afternoon: min, max, mean
                                  "2.48,0",
                                  "5.25,0",
                                  "3.87,0",  // coast day1 afternoon: min, max, mean
                                  "1.78,0",
                                  "4.67,0",
                                  "3.23,0",  // area day1 afternoon: min, max, mean
                                  "-3.30,0",
                                  "-1.58,0",
                                  "-2.44,0",  // inland night: min, max, mean
                                  "-3.81,0",
                                  "-3.03,0",
                                  "-3.42,0",  // coast night: min, max, mean
                                  "-2.36,0",
                                  "-0.55,0",
                                  "-1.45,0",  // area night: min, max, mean
                                  "2.75,0",
                                  "7.20,0",
                                  "4.97,0",  // inland day2: min, max, mean
                                  "3.36,0",
                                  "6.70,0",
                                  "5.03,0",  // coast day2: min, max, mean
                                  "1.96,0",
                                  "7.80,0",
                                  "4.88,0",  // area day2: min, max, mean
                                  "3.38,0",
                                  "4.35,0",
                                  "3.86,0",  // inland day2 morning: min, max, mean
                                  "3.42,0",
                                  "4.56,0",
                                  "3.99,0",  // coast day2 morning: min, max, mean
                                  "2.41,0",
                                  "3.36,0",
                                  "2.88,0",  // area day2 morning: min, max, mean
                                  "6.71,0",
                                  "7.17,0",
                                  "6.94,0",  // inland day2 afternoon: min, max, mean
                                  "6.12,0",
                                  "6.27,0",
                                  "6.19,0",  // coast day2 afternoon: min, max, mean
                                  "6.17,0",
                                  "7.23,0",
                                  "6.70,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on 2...5 astetta, huomenna hieman korkeampi. "
                                  "Yölämpötila on noin -1 astetta."  // the story
                                  )));
      // #63
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "0.57,0",
                        "2.26,0",
                        "1.42,0",  // inland day1: min, max, mean
                        "-0.70,0",
                        "3.47,0",
                        "1.38,0",  // coast day1: min, max, mean
                        "0.27,0",
                        "5.79,0",
                        "3.03,0",  // area day1: min, max, mean
                        "0.57,0",
                        "0.81,0",
                        "0.69,0",  // inland day1 morning: min, max, mean
                        "-0.70,0",
                        "-0.61,0",
                        "-0.66,0",  // coast day1 morning: min, max, mean
                        "0.27,0",
                        "1.48,0",
                        "0.88,0",  // area day1 morning: min, max, mean
                        "2.06,0",
                        "2.26,0",
                        "2.16,0",  // inland day1 afternoon: min, max, mean
                        "0.43,0",
                        "3.47,0",
                        "1.95,0",  // coast day1 afternoon: min, max, mean
                        "1.89,0",
                        "5.79,0",
                        "3.84,0",  // area day1 afternoon: min, max, mean
                        "-1.94,0",
                        "-1.57,0",
                        "-1.75,0",  // inland night: min, max, mean
                        "-4.15,0",
                        "-2.20,0",
                        "-3.17,0",  // coast night: min, max, mean
                        "-0.03,0",
                        "0.41,0",
                        "0.19,0",  // area night: min, max, mean
                        "-1.25,0",
                        "2.54,0",
                        "0.64,0",  // inland day2: min, max, mean
                        "-0.85,0",
                        "4.06,0",
                        "1.60,0",  // coast day2: min, max, mean
                        "-1.18,0",
                        "3.82,0",
                        "1.32,0",  // area day2: min, max, mean
                        "-0.57,0",
                        "0.11,0",
                        "-0.23,0",  // inland day2 morning: min, max, mean
                        "-0.14,0",
                        "1.03,0",
                        "0.44,0",  // coast day2 morning: min, max, mean
                        "-0.84,0",
                        "-0.28,0",
                        "-0.56,0",  // area day2 morning: min, max, mean
                        "2.08,0",
                        "2.12,0",
                        "2.10,0",  // inland day2 afternoon: min, max, mean
                        "0.53,0",
                        "3.17,0",
                        "1.85,0",  // coast day2 afternoon: min, max, mean
                        "1.22,0",
                        "3.59,0",
                        "2.40,0",  // area day2 afternoon: min, max, mean
                        "Päivälämpötila on 2...6 astetta, huomenna suunnilleen sama. Yölämpötila "
                        "on nollan tienoilla tai vähän plussan puolella."  // the story
                        )));
      // #64
      testCases.insert(
          make_pair(i++,
                    new Max36HoursTestParam(
                        "0.98,0",
                        "3.60,0",
                        "2.29,0",  // inland day1: min, max, mean
                        "0.27,0",
                        "2.56,0",
                        "1.41,0",  // coast day1: min, max, mean
                        "0.95,0",
                        "6.17,0",
                        "3.56,0",  // area day1: min, max, mean
                        "0.98,0",
                        "1.00,0",
                        "0.99,0",  // inland day1 morning: min, max, mean
                        "0.27,0",
                        "1.61,0",
                        "0.94,0",  // coast day1 morning: min, max, mean
                        "0.95,0",
                        "1.46,0",
                        "1.20,0",  // area day1 morning: min, max, mean
                        "2.89,0",
                        "3.60,0",
                        "3.24,0",  // inland day1 afternoon: min, max, mean
                        "2.30,0",
                        "2.56,0",
                        "2.43,0",  // coast day1 afternoon: min, max, mean
                        "2.33,0",
                        "6.17,0",
                        "4.25,0",  // area day1 afternoon: min, max, mean
                        "-2.04,0",
                        "-1.31,0",
                        "-1.68,0",  // inland night: min, max, mean
                        "-1.72,0",
                        "-0.39,0",
                        "-1.06,0",  // coast night: min, max, mean
                        "-0.22,0",
                        "0.59,0",
                        "0.18,0",  // area night: min, max, mean
                        "2.94,0",
                        "6.20,0",
                        "4.57,0",  // inland day2: min, max, mean
                        "3.60,0",
                        "5.50,0",
                        "4.55,0",  // coast day2: min, max, mean
                        "1.46,0",
                        "10.52,0",
                        "5.99,0",  // area day2: min, max, mean
                        "3.15,0",
                        "3.96,0",
                        "3.55,0",  // inland day2 morning: min, max, mean
                        "3.78,0",
                        "4.19,0",
                        "3.98,0",  // coast day2 morning: min, max, mean
                        "2.29,0",
                        "3.34,0",
                        "2.81,0",  // area day2 morning: min, max, mean
                        "5.84,0",
                        "6.06,0",
                        "5.95,0",  // inland day2 afternoon: min, max, mean
                        "4.30,0",
                        "4.63,0",
                        "4.47,0",  // coast day2 afternoon: min, max, mean
                        "5.22,0",
                        "10.17,0",
                        "7.69,0",  // area day2 afternoon: min, max, mean
                        "Päivälämpötila on 2...6 astetta, huomenna hieman korkeampi. Yölämpötila "
                        "on nollan tienoilla tai vähän plussan puolella."  // the story
                        )));
      // #65
      testCases.insert(make_pair(
          i++,
          new Max36HoursTestParam("1.66,0",
                                  "4.84,0",
                                  "3.25,0",  // inland day1: min, max, mean
                                  "0.58,0",
                                  "4.45,0",
                                  "2.52,0",  // coast day1: min, max, mean
                                  "1.39,0",
                                  "5.52,0",
                                  "3.46,0",  // area day1: min, max, mean
                                  "1.66,0",
                                  "2.25,0",
                                  "1.96,0",  // inland day1 morning: min, max, mean
                                  "0.58,0",
                                  "0.91,0",
                                  "0.74,0",  // coast day1 morning: min, max, mean
                                  "1.39,0",
                                  "2.83,0",
                                  "2.11,0",  // area day1 morning: min, max, mean
                                  "4.23,0",
                                  "4.84,0",
                                  "4.53,0",  // inland day1 afternoon: min, max, mean
                                  "4.09,0",
                                  "4.45,0",
                                  "4.27,0",  // coast day1 afternoon: min, max, mean
                                  "3.32,0",
                                  "5.52,0",
                                  "4.42,0",  // area day1 afternoon: min, max, mean
                                  "1.33,0",
                                  "3.28,0",
                                  "2.30,0",  // inland night: min, max, mean
                                  "-1.11,0",
                                  "0.48,0",
                                  "-0.32,0",  // coast night: min, max, mean
                                  "-2.48,0",
                                  "-1.43,0",
                                  "-1.96,0",  // area night: min, max, mean
                                  "1.82,0",
                                  "5.10,0",
                                  "3.46,0",  // inland day2: min, max, mean
                                  "1.36,0",
                                  "4.01,0",
                                  "2.69,0",  // coast day2: min, max, mean
                                  "1.15,0",
                                  "8.72,0",
                                  "4.93,0",  // area day2: min, max, mean
                                  "1.93,0",
                                  "2.18,0",
                                  "2.05,0",  // inland day2 morning: min, max, mean
                                  "2.07,0",
                                  "3.50,0",
                                  "2.79,0",  // coast day2 morning: min, max, mean
                                  "1.18,0",
                                  "2.63,0",
                                  "1.91,0",  // area day2 morning: min, max, mean
                                  "3.98,0",
                                  "4.25,0",
                                  "4.11,0",  // inland day2 afternoon: min, max, mean
                                  "2.50,0",
                                  "3.38,0",
                                  "2.94,0",  // coast day2 afternoon: min, max, mean
                                  "3.33,0",
                                  "7.73,0",
                                  "5.53,0",  // area day2 afternoon: min, max, mean
                                  "Päivälämpötila on lähellä 5 astetta, huomenna hieman korkeampi. "
                                  "Yölämpötila on noin -2 astetta."  // the story
                                  )));
    }
  }
  else  // summer
  {
    // #66
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam(
                      "-3.05,0",
                      "-1.68,0",
                      "-2.36,0",  // inland day1: min, max, mean
                      "-3.61,0",
                      "-0.58,0",
                      "-2.09,0",  // coast day1: min, max, mean
                      "-3.99,0",
                      "0.86,0",
                      "-1.56,0",  // area day1: min, max, mean
                      "-3.05,0",
                      "-2.64,0",
                      "-2.85,0",  // inland day1 morning: min, max, mean
                      "-3.61,0",
                      "-3.17,0",
                      "-3.39,0",  // coast day1 morning: min, max, mean
                      "-3.99,0",
                      "-3.41,0",
                      "-3.70,0",  // area day1 morning: min, max, mean
                      "-2.60,0",
                      "-1.68,0",
                      "-2.14,0",  // inland day1 afternoon: min, max, mean
                      "-4.46,0",
                      "-0.58,0",
                      "-2.52,0",  // coast day1 afternoon: min, max, mean
                      "-2.82,0",
                      "0.86,0",
                      "-0.98,0",  // area day1 afternoon: min, max, mean
                      "-6.14,0",
                      "-5.43,0",
                      "-5.78,0",  // inland night: min, max, mean
                      "-4.39,0",
                      "-3.09,0",
                      "-3.74,0",  // coast night: min, max, mean
                      "-5.82,0",
                      "-4.70,0",
                      "-5.26,0",  // area night: min, max, mean
                      "-8.97,0",
                      "-6.09,0",
                      "-7.53,0",  // inland day2: min, max, mean
                      "-8.32,0",
                      "-5.10,0",
                      "-6.71,0",  // coast day2: min, max, mean
                      "-9.20,0",
                      "-7.33,0",
                      "-8.27,0",  // area day2: min, max, mean
                      "-8.40,0",
                      "-7.75,0",
                      "-8.07,0",  // inland day2 morning: min, max, mean
                      "-7.77,0",
                      "-7.37,0",
                      "-7.57,0",  // coast day2 morning: min, max, mean
                      "-9.18,0",
                      "-8.54,0",
                      "-8.86,0",  // area day2 morning: min, max, mean
                      "-7.45,0",
                      "-6.51,0",
                      "-6.98,0",  // inland day2 afternoon: min, max, mean
                      "-9.00,0",
                      "-5.95,0",
                      "-7.48,0",  // coast day2 afternoon: min, max, mean
                      "-8.35,0",
                      "-8.02,0",
                      "-8.19,0",  // area day2 afternoon: min, max, mean
                      "Päivän ylin lämpötila on -3...+1 astetta, päivällä pakkasta on vajaat 10 "
                      "astetta. Yön alin lämpötila on noin -5 astetta.")));

    // #67
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("-2.38,0",
                                "-0.11,0",
                                "-1.25,0",  // inland day1: min, max, mean
                                "-3.76,0",
                                "4.58,0",
                                "0.41,0",  // coast day1: min, max, mean
                                "-3.12,0",
                                "3.06,0",
                                "-0.03,0",  // area day1: min, max, mean
                                "-2.38,0",
                                "-2.02,0",
                                "-2.20,0",  // inland day1 morning: min, max, mean
                                "-3.76,0",
                                "-3.46,0",
                                "-3.61,0",  // coast day1 morning: min, max, mean
                                "-3.12,0",
                                "-1.21,0",
                                "-2.16,0",  // area day1 morning: min, max, mean
                                "-0.16,0",
                                "-0.11,0",
                                "-0.14,0",  // inland day1 afternoon: min, max, mean
                                "-0.37,0",
                                "4.58,0",
                                "2.10,0",  // coast day1 afternoon: min, max, mean
                                "-0.43,0",
                                "3.06,0",
                                "1.32,0",  // area day1 afternoon: min, max, mean
                                "-6.77,0",
                                "-5.13,0",
                                "-5.95,0",  // inland night: min, max, mean
                                "-4.89,0",
                                "-4.18,0",
                                "-4.53,0",  // coast night: min, max, mean
                                "-5.32,0",
                                "-4.17,0",
                                "-4.75,0",  // area night: min, max, mean
                                "-4.55,0",
                                "-0.71,0",
                                "-2.63,0",  // inland day2: min, max, mean
                                "-4.58,0",
                                "-0.76,0",
                                "-2.67,0",  // coast day2: min, max, mean
                                "-5.39,0",
                                "2.33,0",
                                "-1.53,0",  // area day2: min, max, mean
                                "-3.96,0",
                                "-3.39,0",
                                "-3.68,0",  // inland day2 morning: min, max, mean
                                "-3.93,0",
                                "-3.39,0",
                                "-3.66,0",  // coast day2 morning: min, max, mean
                                "-4.66,0",
                                "-4.09,0",
                                "-4.37,0",  // area day2 morning: min, max, mean
                                "-2.07,0",
                                "-1.36,0",
                                "-1.72,0",  // inland day2 afternoon: min, max, mean
                                "-3.61,0",
                                "-0.77,0",
                                "-2.19,0",  // coast day2 afternoon: min, max, mean
                                "-2.40,0",
                                "1.48,0",
                                "-0.46,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 0...+3 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on -5 asteen tuntumassa.")));
    // #68
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("-1.62,0",
                                "0.64,0",
                                "-0.49,0",  // inland day1: min, max, mean
                                "-1.78,0",
                                "2.87,0",
                                "0.55,0",  // coast day1: min, max, mean
                                "-2.20,0",
                                "2.70,0",
                                "0.25,0",  // area day1: min, max, mean
                                "-1.62,0",
                                "-0.66,0",
                                "-1.14,0",  // inland day1 morning: min, max, mean
                                "-1.78,0",
                                "-0.72,0",
                                "-1.25,0",  // coast day1 morning: min, max, mean
                                "-2.20,0",
                                "-0.75,0",
                                "-1.47,0",  // area day1 morning: min, max, mean
                                "0.59,0",
                                "0.64,0",
                                "0.61,0",  // inland day1 afternoon: min, max, mean
                                "-0.75,0",
                                "2.87,0",
                                "1.06,0",  // coast day1 afternoon: min, max, mean
                                "-0.04,0",
                                "2.70,0",
                                "1.33,0",  // area day1 afternoon: min, max, mean
                                "-2.61,0",
                                "-2.12,0",
                                "-2.37,0",  // inland night: min, max, mean
                                "-3.46,0",
                                "-1.48,0",
                                "-2.47,0",  // coast night: min, max, mean
                                "-5.10,0",
                                "-4.49,0",
                                "-4.79,0",  // area night: min, max, mean
                                "-6.76,0",
                                "-2.01,0",
                                "-4.39,0",  // inland day2: min, max, mean
                                "-6.45,0",
                                "-2.01,0",
                                "-4.23,0",  // coast day2: min, max, mean
                                "-7.63,0",
                                "0.47,0",
                                "-3.58,0",  // area day2: min, max, mean
                                "-6.15,0",
                                "-5.44,0",
                                "-5.80,0",  // inland day2 morning: min, max, mean
                                "-5.77,0",
                                "-5.71,0",
                                "-5.74,0",  // coast day2 morning: min, max, mean
                                "-6.73,0",
                                "-5.11,0",
                                "-5.92,0",  // area day2 morning: min, max, mean
                                "-2.69,0",
                                "-2.69,0",
                                "-2.69,0",  // inland day2 afternoon: min, max, mean
                                "-3.38,0",
                                "-2.83,0",
                                "-3.11,0",  // coast day2 afternoon: min, max, mean
                                "-3.15,0",
                                "0.11,0",
                                "-1.52,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 0...+3 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on -5 asteen tuntumassa.")));
    // #69
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("-2.37,0",
                                "-0.74,0",
                                "-1.56,0",  // inland day1: min, max, mean
                                "-4.29,0",
                                "-1.42,0",
                                "-2.86,0",  // coast day1: min, max, mean
                                "-2.58,0",
                                "1.43,0",
                                "-0.57,0",  // area day1: min, max, mean
                                "-2.37,0",
                                "-2.19,0",
                                "-2.28,0",  // inland day1 morning: min, max, mean
                                "-4.29,0",
                                "-3.22,0",
                                "-3.76,0",  // coast day1 morning: min, max, mean
                                "-2.58,0",
                                "-2.57,0",
                                "-2.58,0",  // area day1 morning: min, max, mean
                                "-1.57,0",
                                "-0.74,0",
                                "-1.15,0",  // inland day1 afternoon: min, max, mean
                                "-2.60,0",
                                "-1.42,0",
                                "-2.01,0",  // coast day1 afternoon: min, max, mean
                                "-1.60,0",
                                "1.43,0",
                                "-0.08,0",  // area day1 afternoon: min, max, mean
                                "-6.12,0",
                                "-4.36,0",
                                "-5.24,0",  // inland night: min, max, mean
                                "-4.33,0",
                                "-3.68,0",
                                "-4.00,0",  // coast night: min, max, mean
                                "-2.90,0",
                                "-2.53,0",
                                "-2.71,0",  // area night: min, max, mean
                                "-3.46,0",
                                "-1.10,0",
                                "-2.28,0",  // inland day2: min, max, mean
                                "-2.94,0",
                                "0.63,0",
                                "-1.16,0",  // coast day2: min, max, mean
                                "-3.86,0",
                                "4.09,0",
                                "0.11,0",  // area day2: min, max, mean
                                "-2.81,0",
                                "-1.95,0",
                                "-2.38,0",  // inland day2 morning: min, max, mean
                                "-2.64,0",
                                "-1.71,0",
                                "-2.18,0",  // coast day2 morning: min, max, mean
                                "-2.91,0",
                                "-2.76,0",
                                "-2.84,0",  // area day2 morning: min, max, mean
                                "-1.29,0",
                                "-1.24,0",
                                "-1.27,0",  // inland day2 afternoon: min, max, mean
                                "-2.27,0",
                                "-0.13,0",
                                "-1.20,0",  // coast day2 afternoon: min, max, mean
                                "-1.41,0",
                                "3.17,0",
                                "0.88,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on nollan tienoilla, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on noin -3 astetta.")));
    // #70
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("-1.01,0",
                                          "1.02,0",
                                          "0.00,0",  // inland day1: min, max, mean
                                          "-1.67,0",
                                          "2.88,0",
                                          "0.60,0",  // coast day1: min, max, mean
                                          "-1.81,0",
                                          "4.84,0",
                                          "1.51,0",  // area day1: min, max, mean
                                          "-1.01,0",
                                          "-0.42,0",
                                          "-0.71,0",  // inland day1 morning: min, max, mean
                                          "-1.67,0",
                                          "-1.06,0",
                                          "-1.37,0",  // coast day1 morning: min, max, mean
                                          "-1.81,0",
                                          "-0.72,0",
                                          "-1.27,0",  // area day1 morning: min, max, mean
                                          "0.14,0",
                                          "1.02,0",
                                          "0.58,0",  // inland day1 afternoon: min, max, mean
                                          "-1.18,0",
                                          "2.88,0",
                                          "0.85,0",  // coast day1 afternoon: min, max, mean
                                          "-0.13,0",
                                          "4.84,0",
                                          "2.36,0",  // area day1 afternoon: min, max, mean
                                          "-1.15,0",
                                          "-0.11,0",
                                          "-0.63,0",  // inland night: min, max, mean
                                          "-4.16,0",
                                          "-3.44,0",
                                          "-3.80,0",  // coast night: min, max, mean
                                          "-2.44,0",
                                          "-0.51,0",
                                          "-1.48,0",  // area night: min, max, mean
                                          "-4.32,0",
                                          "0.33,0",
                                          "-2.00,0",  // inland day2: min, max, mean
                                          "-2.97,0",
                                          "1.57,0",
                                          "-0.70,0",  // coast day2: min, max, mean
                                          "-3.66,0",
                                          "0.96,0",
                                          "-1.35,0",  // area day2: min, max, mean
                                          "-3.32,0",
                                          "-3.06,0",
                                          "-3.19,0",  // inland day2 morning: min, max, mean
                                          "-2.55,0",
                                          "-1.28,0",
                                          "-1.91,0",  // coast day2 morning: min, max, mean
                                          "-3.48,0",
                                          "-1.74,0",
                                          "-2.61,0",  // area day2 morning: min, max, mean
                                          "-0.54,0",
                                          "0.26,0",
                                          "-0.14,0",  // inland day2 afternoon: min, max, mean
                                          "-1.62,0",
                                          "1.04,0",
                                          "-0.29,0",  // coast day2 afternoon: min, max, mean
                                          "-0.66,0",
                                          "0.29,0",
                                          "-0.19,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 0...+5 astetta, huomenna "
                                          "nollan tienoilla. Sisämaassa yön alin lämpötila on "
                                          "0...-1 astetta, rannikolla noin -4 astetta.")));
    // #71
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("0.54,0",
                                          "1.74,0",
                                          "1.14,0",  // inland day1: min, max, mean
                                          "0.31,0",
                                          "4.29,0",
                                          "2.30,0",  // coast day1: min, max, mean
                                          "-0.43,0",
                                          "4.89,0",
                                          "2.23,0",  // area day1: min, max, mean
                                          "0.54,0",
                                          "1.50,0",
                                          "1.02,0",  // inland day1 morning: min, max, mean
                                          "0.31,0",
                                          "0.66,0",
                                          "0.49,0",  // coast day1 morning: min, max, mean
                                          "-0.43,0",
                                          "-0.04,0",
                                          "-0.24,0",  // area day1 morning: min, max, mean
                                          "1.37,0",
                                          "1.74,0",
                                          "1.56,0",  // inland day1 afternoon: min, max, mean
                                          "-0.34,0",
                                          "4.29,0",
                                          "1.98,0",  // coast day1 afternoon: min, max, mean
                                          "0.46,0",
                                          "4.89,0",
                                          "2.67,0",  // area day1 afternoon: min, max, mean
                                          "-4.33,0",
                                          "-3.33,0",
                                          "-3.83,0",  // inland night: min, max, mean
                                          "0.07,0",
                                          "0.35,0",
                                          "0.21,0",  // coast night: min, max, mean
                                          "-0.90,0",
                                          "0.99,0",
                                          "0.04,0",  // area night: min, max, mean
                                          "5.53,0",
                                          "9.04,0",
                                          "7.28,0",  // inland day2: min, max, mean
                                          "7.04,0",
                                          "8.49,0",
                                          "7.76,0",  // coast day2: min, max, mean
                                          "4.72,0",
                                          "12.07,0",
                                          "8.40,0",  // area day2: min, max, mean
                                          "6.31,0",
                                          "7.20,0",
                                          "6.76,0",  // inland day2 morning: min, max, mean
                                          "7.31,0",
                                          "8.01,0",
                                          "7.66,0",  // coast day2 morning: min, max, mean
                                          "5.54,0",
                                          "7.31,0",
                                          "6.42,0",  // area day2 morning: min, max, mean
                                          "8.73,0",
                                          "8.80,0",
                                          "8.77,0",  // inland day2 afternoon: min, max, mean
                                          "7.36,0",
                                          "7.79,0",
                                          "7.57,0",  // coast day2 afternoon: min, max, mean
                                          "8.02,0",
                                          "11.24,0",
                                          "9.63,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 0...+5 astetta, huomenna 10 "
                                          "asteen tienoilla. Sisämaassa yön alin lämpötila on noin "
                                          "-4 astetta, rannikolla vähän plussan puolella.")));
    // #72
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("0.42,0",
                                "1.10,0",
                                "0.76,0",  // inland day1: min, max, mean
                                "-0.40,0",
                                "3.75,0",
                                "1.68,0",  // coast day1: min, max, mean
                                "-0.51,0",
                                "4.74,0",
                                "2.12,0",  // area day1: min, max, mean
                                "0.42,0",
                                "1.31,0",
                                "0.87,0",  // inland day1 morning: min, max, mean
                                "-0.40,0",
                                "0.62,0",
                                "0.11,0",  // coast day1 morning: min, max, mean
                                "-0.51,0",
                                "-0.51,0",
                                "-0.51,0",  // area day1 morning: min, max, mean
                                "0.10,0",
                                "1.10,0",
                                "0.60,0",  // inland day1 afternoon: min, max, mean
                                "-0.06,0",
                                "3.75,0",
                                "1.85,0",  // coast day1 afternoon: min, max, mean
                                "0.01,0",
                                "4.74,0",
                                "2.38,0",  // area day1 afternoon: min, max, mean
                                "-0.44,0",
                                "0.92,0",
                                "0.24,0",  // inland night: min, max, mean
                                "-3.45,0",
                                "-1.64,0",
                                "-2.54,0",  // coast night: min, max, mean
                                "-4.39,0",
                                "-4.04,0",
                                "-4.21,0",  // area night: min, max, mean
                                "5.68,0",
                                "8.05,0",
                                "6.86,0",  // inland day2: min, max, mean
                                "5.28,0",
                                "8.83,0",
                                "7.05,0",  // coast day2: min, max, mean
                                "4.78,0",
                                "11.82,0",
                                "8.30,0",  // area day2: min, max, mean
                                "5.89,0",
                                "6.74,0",
                                "6.31,0",  // inland day2 morning: min, max, mean
                                "5.94,0",
                                "7.42,0",
                                "6.68,0",  // coast day2 morning: min, max, mean
                                "4.96,0",
                                "5.50,0",
                                "5.23,0",  // area day2 morning: min, max, mean
                                "7.80,0",
                                "7.91,0",
                                "7.85,0",  // inland day2 afternoon: min, max, mean
                                "7.63,0",
                                "8.76,0",
                                "8.20,0",  // coast day2 afternoon: min, max, mean
                                "6.97,0",
                                "10.97,0",
                                "8.97,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 0...+5 astetta, huomenna 7...11 astetta. "
                                "Yön alin lämpötila on noin -4 astetta.")));
    // #73
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("0.76,0",
                                "2.23,0",
                                "1.49,0",  // inland day1: min, max, mean
                                "0.33,0",
                                "1.78,0",
                                "1.05,0",  // coast day1: min, max, mean
                                "0.18,0",
                                "4.89,0",
                                "2.54,0",  // area day1: min, max, mean
                                "0.76,0",
                                "0.88,0",
                                "0.82,0",  // inland day1 morning: min, max, mean
                                "0.33,0",
                                "0.35,0",
                                "0.34,0",  // coast day1 morning: min, max, mean
                                "0.18,0",
                                "0.48,0",
                                "0.33,0",  // area day1 morning: min, max, mean
                                "1.66,0",
                                "2.23,0",
                                "1.95,0",  // inland day1 afternoon: min, max, mean
                                "0.29,0",
                                "1.78,0",
                                "1.03,0",  // coast day1 afternoon: min, max, mean
                                "1.40,0",
                                "4.89,0",
                                "3.14,0",  // area day1 afternoon: min, max, mean
                                "-2.06,0",
                                "-2.02,0",
                                "-2.04,0",  // inland night: min, max, mean
                                "-1.83,0",
                                "0.09,0",
                                "-0.87,0",  // coast night: min, max, mean
                                "-2.58,0",
                                "-2.43,0",
                                "-2.50,0",  // area night: min, max, mean
                                "-0.97,0",
                                "1.82,0",
                                "0.43,0",  // inland day2: min, max, mean
                                "0.07,0",
                                "3.40,0",
                                "1.74,0",  // coast day2: min, max, mean
                                "-1.50,0",
                                "5.43,0",
                                "1.96,0",  // area day2: min, max, mean
                                "-0.20,0",
                                "0.65,0",
                                "0.23,0",  // inland day2 morning: min, max, mean
                                "0.18,0",
                                "0.71,0",
                                "0.45,0",  // coast day2 morning: min, max, mean
                                "-0.73,0",
                                "-0.66,0",
                                "-0.70,0",  // area day2 morning: min, max, mean
                                "0.90,0",
                                "0.97,0",
                                "0.93,0",  // inland day2 afternoon: min, max, mean
                                "0.64,0",
                                "3.12,0",
                                "1.88,0",  // coast day2 afternoon: min, max, mean
                                "0.61,0",
                                "4.94,0",
                                "2.77,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 1...5 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on noin -3 astetta.")));
    // #74
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("0.97,0",
                                          "4.28,0",
                                          "2.63,0",  // inland day1: min, max, mean
                                          "-0.10,0",
                                          "5.27,0",
                                          "2.58,0",  // coast day1: min, max, mean
                                          "0.75,0",
                                          "6.77,0",
                                          "3.76,0",  // area day1: min, max, mean
                                          "0.97,0",
                                          "1.55,0",
                                          "1.26,0",  // inland day1 morning: min, max, mean
                                          "-0.10,0",
                                          "0.89,0",
                                          "0.40,0",  // coast day1 morning: min, max, mean
                                          "0.75,0",
                                          "1.92,0",
                                          "1.33,0",  // area day1 morning: min, max, mean
                                          "3.46,0",
                                          "4.28,0",
                                          "3.87,0",  // inland day1 afternoon: min, max, mean
                                          "2.45,0",
                                          "5.27,0",
                                          "3.86,0",  // coast day1 afternoon: min, max, mean
                                          "2.52,0",
                                          "6.77,0",
                                          "4.65,0",  // area day1 afternoon: min, max, mean
                                          "-0.47,0",
                                          "1.37,0",
                                          "0.45,0",  // inland night: min, max, mean
                                          "-2.60,0",
                                          "-1.99,0",
                                          "-2.30,0",  // coast night: min, max, mean
                                          "-2.04,0",
                                          "-0.55,0",
                                          "-1.29,0",  // area night: min, max, mean
                                          "6.33,0",
                                          "11.57,0",
                                          "8.95,0",  // inland day2: min, max, mean
                                          "6.86,0",
                                          "14.97,0",
                                          "10.91,0",  // coast day2: min, max, mean
                                          "6.07,0",
                                          "12.93,0",
                                          "9.50,0",  // area day2: min, max, mean
                                          "6.96,0",
                                          "7.20,0",
                                          "7.08,0",  // inland day2 morning: min, max, mean
                                          "7.33,0",
                                          "9.16,0",
                                          "8.24,0",  // coast day2 morning: min, max, mean
                                          "6.70,0",
                                          "8.07,0",
                                          "7.38,0",  // area day2 morning: min, max, mean
                                          "11.12,0",
                                          "11.19,0",
                                          "11.16,0",  // inland day2 afternoon: min, max, mean
                                          "11.01,0",
                                          "14.73,0",
                                          "12.87,0",  // coast day2 afternoon: min, max, mean
                                          "10.68,0",
                                          "12.14,0",
                                          "11.41,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 3...7 astetta, huomenna noin "
                                          "10 astetta. Yön alin lämpötila on noin -1 astetta.")));
    // #75
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("1.01,0",
                                          "1.77,0",
                                          "1.39,0",  // inland day1: min, max, mean
                                          "0.62,0",
                                          "3.85,0",
                                          "2.24,0",  // coast day1: min, max, mean
                                          "0.86,0",
                                          "3.37,0",
                                          "2.11,0",  // area day1: min, max, mean
                                          "1.01,0",
                                          "1.33,0",
                                          "1.17,0",  // inland day1 morning: min, max, mean
                                          "0.62,0",
                                          "1.12,0",
                                          "0.87,0",  // coast day1 morning: min, max, mean
                                          "0.86,0",
                                          "1.05,0",
                                          "0.96,0",  // area day1 morning: min, max, mean
                                          "1.74,0",
                                          "1.77,0",
                                          "1.75,0",  // inland day1 afternoon: min, max, mean
                                          "1.19,0",
                                          "3.85,0",
                                          "2.52,0",  // coast day1 afternoon: min, max, mean
                                          "1.26,0",
                                          "3.37,0",
                                          "2.31,0",  // area day1 afternoon: min, max, mean
                                          "-2.65,0",
                                          "-2.08,0",
                                          "-2.36,0",  // inland night: min, max, mean
                                          "-2.98,0",
                                          "-1.54,0",
                                          "-2.26,0",  // coast night: min, max, mean
                                          "-0.84,0",
                                          "0.17,0",
                                          "-0.33,0",  // area night: min, max, mean
                                          "-3.30,0",
                                          "-0.98,0",
                                          "-2.14,0",  // inland day2: min, max, mean
                                          "-3.64,0",
                                          "-1.87,0",
                                          "-2.76,0",  // coast day2: min, max, mean
                                          "-4.35,0",
                                          "0.40,0",
                                          "-1.98,0",  // area day2: min, max, mean
                                          "-3.15,0",
                                          "-2.86,0",
                                          "-3.01,0",  // inland day2 morning: min, max, mean
                                          "-2.99,0",
                                          "-2.42,0",
                                          "-2.71,0",  // coast day2 morning: min, max, mean
                                          "-4.15,0",
                                          "-2.22,0",
                                          "-3.18,0",  // area day2 morning: min, max, mean
                                          "-1.57,0",
                                          "-1.49,0",
                                          "-1.53,0",  // inland day2 afternoon: min, max, mean
                                          "-2.62,0",
                                          "-2.51,0",
                                          "-2.56,0",  // coast day2 afternoon: min, max, mean
                                          "-2.18,0",
                                          "-0.52,0",
                                          "-1.35,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 1...3 astetta, huomenna noin "
                                          "-1 astetta. Yön alin lämpötila on nollan tienoilla.")));
    // #76
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("1.23,0",
                                "1.99,0",
                                "1.61,0",  // inland day1: min, max, mean
                                "0.60,0",
                                "3.51,0",
                                "2.05,0",  // coast day1: min, max, mean
                                "0.89,0",
                                "5.93,0",
                                "3.41,0",  // area day1: min, max, mean
                                "1.23,0",
                                "1.64,0",
                                "1.43,0",  // inland day1 morning: min, max, mean
                                "0.60,0",
                                "2.36,0",
                                "1.48,0",  // coast day1 morning: min, max, mean
                                "0.89,0",
                                "0.95,0",
                                "0.92,0",  // area day1 morning: min, max, mean
                                "1.58,0",
                                "1.99,0",
                                "1.78,0",  // inland day1 afternoon: min, max, mean
                                "1.06,0",
                                "3.51,0",
                                "2.28,0",  // coast day1 afternoon: min, max, mean
                                "1.15,0",
                                "5.93,0",
                                "3.54,0",  // area day1 afternoon: min, max, mean
                                "-2.93,0",
                                "-1.37,0",
                                "-2.15,0",  // inland night: min, max, mean
                                "-3.08,0",
                                "-2.61,0",
                                "-2.85,0",  // coast night: min, max, mean
                                "-2.89,0",
                                "-1.92,0",
                                "-2.40,0",  // area night: min, max, mean
                                "-0.29,0",
                                "1.99,0",
                                "0.85,0",  // inland day2: min, max, mean
                                "0.42,0",
                                "4.84,0",
                                "2.63,0",  // coast day2: min, max, mean
                                "-0.29,0",
                                "2.72,0",
                                "1.22,0",  // area day2: min, max, mean
                                "0.56,0",
                                "0.78,0",
                                "0.67,0",  // inland day2 morning: min, max, mean
                                "0.61,0",
                                "2.48,0",
                                "1.54,0",  // coast day2 morning: min, max, mean
                                "-0.19,0",
                                "0.81,0",
                                "0.31,0",  // area day2 morning: min, max, mean
                                "1.28,0",
                                "1.68,0",
                                "1.48,0",  // inland day2 afternoon: min, max, mean
                                "1.08,0",
                                "4.14,0",
                                "2.61,0",  // coast day2 afternoon: min, max, mean
                                "1.26,0",
                                "1.90,0",
                                "1.58,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 1...6 astetta, huomenna vähän plussan "
                                "puolella. Yön alin lämpötila on noin -2 astetta.")));
    // #77
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("2.14,0",
                                          "4.12,0",
                                          "3.13,0",  // inland day1: min, max, mean
                                          "1.45,0",
                                          "4.60,0",
                                          "3.02,0",  // coast day1: min, max, mean
                                          "1.68,0",
                                          "7.77,0",
                                          "4.72,0",  // area day1: min, max, mean
                                          "2.14,0",
                                          "2.83,0",
                                          "2.49,0",  // inland day1 morning: min, max, mean
                                          "1.45,0",
                                          "2.63,0",
                                          "2.04,0",  // coast day1 morning: min, max, mean
                                          "1.68,0",
                                          "2.57,0",
                                          "2.13,0",  // area day1 morning: min, max, mean
                                          "3.73,0",
                                          "4.12,0",
                                          "3.93,0",  // inland day1 afternoon: min, max, mean
                                          "1.74,0",
                                          "4.60,0",
                                          "3.17,0",  // coast day1 afternoon: min, max, mean
                                          "3.14,0",
                                          "7.77,0",
                                          "5.45,0",  // area day1 afternoon: min, max, mean
                                          "-2.12,0",
                                          "-1.79,0",
                                          "-1.95,0",  // inland night: min, max, mean
                                          "-0.53,0",
                                          "0.10,0",
                                          "-0.22,0",  // coast night: min, max, mean
                                          "-2.09,0",
                                          "-0.43,0",
                                          "-1.26,0",  // area night: min, max, mean
                                          "-2.04,0",
                                          "-0.01,0",
                                          "-1.03,0",  // inland day2: min, max, mean
                                          "-1.36,0",
                                          "2.30,0",
                                          "0.47,0",  // coast day2: min, max, mean
                                          "-2.45,0",
                                          "2.57,0",
                                          "0.06,0",  // area day2: min, max, mean
                                          "-1.61,0",
                                          "-0.97,0",
                                          "-1.29,0",  // inland day2 morning: min, max, mean
                                          "-1.15,0",
                                          "-0.57,0",
                                          "-0.86,0",  // coast day2 morning: min, max, mean
                                          "-2.20,0",
                                          "-1.60,0",
                                          "-1.90,0",  // area day2 morning: min, max, mean
                                          "-0.28,0",
                                          "-0.02,0",
                                          "-0.15,0",  // inland day2 afternoon: min, max, mean
                                          "-1.23,0",
                                          "1.53,0",
                                          "0.15,0",  // coast day2 afternoon: min, max, mean
                                          "-0.60,0",
                                          "2.37,0",
                                          "0.88,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 3...8 astetta, huomenna hieman "
                                          "alempi. Yön alin lämpötila on 0...-2 astetta.")));
    // #78
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("2.03,0",
                                "4.08,0",
                                "3.06,0",  // inland day1: min, max, mean
                                "0.20,0",
                                "6.83,0",
                                "3.52,0",  // coast day1: min, max, mean
                                "1.84,0",
                                "4.84,0",
                                "3.34,0",  // area day1: min, max, mean
                                "2.03,0",
                                "2.74,0",
                                "2.39,0",  // inland day1 morning: min, max, mean
                                "0.20,0",
                                "0.47,0",
                                "0.33,0",  // coast day1 morning: min, max, mean
                                "1.84,0",
                                "3.14,0",
                                "2.49,0",  // area day1 morning: min, max, mean
                                "3.83,0",
                                "4.08,0",
                                "3.96,0",  // inland day1 afternoon: min, max, mean
                                "2.30,0",
                                "6.83,0",
                                "4.57,0",  // coast day1 afternoon: min, max, mean
                                "3.56,0",
                                "4.84,0",
                                "4.20,0",  // area day1 afternoon: min, max, mean
                                "-2.76,0",
                                "-2.59,0",
                                "-2.67,0",  // inland night: min, max, mean
                                "-2.56,0",
                                "-0.81,0",
                                "-1.69,0",  // coast night: min, max, mean
                                "-1.13,0",
                                "-0.50,0",
                                "-0.82,0",  // area night: min, max, mean
                                "2.14,0",
                                "6.26,0",
                                "4.20,0",  // inland day2: min, max, mean
                                "3.01,0",
                                "5.29,0",
                                "4.15,0",  // coast day2: min, max, mean
                                "1.79,0",
                                "8.50,0",
                                "5.14,0",  // area day2: min, max, mean
                                "2.55,0",
                                "2.62,0",
                                "2.58,0",  // inland day2 morning: min, max, mean
                                "3.08,0",
                                "3.92,0",
                                "3.50,0",  // coast day2 morning: min, max, mean
                                "2.09,0",
                                "2.20,0",
                                "2.14,0",  // area day2 morning: min, max, mean
                                "5.01,0",
                                "5.91,0",
                                "5.46,0",  // inland day2 afternoon: min, max, mean
                                "3.57,0",
                                "4.49,0",
                                "4.03,0",  // coast day2 afternoon: min, max, mean
                                "4.02,0",
                                "7.99,0",
                                "6.01,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 5 asteen tuntumassa, huomenna hieman "
                                "korkeampi. Yön alin lämpötila on noin -1 astetta.")));
    // #79
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("3.58,0",
                                "4.96,0",
                                "4.27,0",  // inland day1: min, max, mean
                                "2.55,0",
                                "2.79,0",
                                "2.67,0",  // coast day1: min, max, mean
                                "2.62,0",
                                "8.27,0",
                                "5.45,0",  // area day1: min, max, mean
                                "3.58,0",
                                "3.84,0",
                                "3.71,0",  // inland day1 morning: min, max, mean
                                "2.55,0",
                                "4.47,0",
                                "3.51,0",  // coast day1 morning: min, max, mean
                                "2.62,0",
                                "2.66,0",
                                "2.64,0",  // area day1 morning: min, max, mean
                                "4.04,0",
                                "4.96,0",
                                "4.50,0",  // inland day1 afternoon: min, max, mean
                                "2.63,0",
                                "2.79,0",
                                "2.71,0",  // coast day1 afternoon: min, max, mean
                                "3.53,0",
                                "8.27,0",
                                "5.90,0",  // area day1 afternoon: min, max, mean
                                "-0.23,0",
                                "1.55,0",
                                "0.66,0",  // inland night: min, max, mean
                                "-0.86,0",
                                "-0.23,0",
                                "-0.54,0",  // coast night: min, max, mean
                                "-0.94,0",
                                "0.62,0",
                                "-0.16,0",  // area night: min, max, mean
                                "1.77,0",
                                "4.70,0",
                                "3.23,0",  // inland day2: min, max, mean
                                "2.73,0",
                                "3.51,0",
                                "3.12,0",  // coast day2: min, max, mean
                                "1.35,0",
                                "8.91,0",
                                "5.13,0",  // area day2: min, max, mean
                                "2.27,0",
                                "2.54,0",
                                "2.41,0",  // inland day2 morning: min, max, mean
                                "2.76,0",
                                "4.49,0",
                                "3.62,0",  // coast day2 morning: min, max, mean
                                "1.93,0",
                                "2.36,0",
                                "2.15,0",  // area day2 morning: min, max, mean
                                "3.91,0",
                                "4.50,0",
                                "4.20,0",  // inland day2 afternoon: min, max, mean
                                "2.00,0",
                                "3.28,0",
                                "2.64,0",  // coast day2 afternoon: min, max, mean
                                "3.25,0",
                                "7.92,0",
                                "5.58,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 4...8 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on nollan tienoilla.")));
    // #80
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("3.44,0",
                                          "5.68,0",
                                          "4.56,0",  // inland day1: min, max, mean
                                          "2.82,0",
                                          "5.20,0",
                                          "4.01,0",  // coast day1: min, max, mean
                                          "3.09,0",
                                          "6.23,0",
                                          "4.66,0",  // area day1: min, max, mean
                                          "3.44,0",
                                          "4.27,0",
                                          "3.86,0",  // inland day1 morning: min, max, mean
                                          "2.82,0",
                                          "3.40,0",
                                          "3.11,0",  // coast day1 morning: min, max, mean
                                          "3.09,0",
                                          "4.42,0",
                                          "3.75,0",  // area day1 morning: min, max, mean
                                          "5.64,0",
                                          "5.68,0",
                                          "5.66,0",  // inland day1 afternoon: min, max, mean
                                          "4.17,0",
                                          "5.20,0",
                                          "4.69,0",  // coast day1 afternoon: min, max, mean
                                          "4.76,0",
                                          "6.23,0",
                                          "5.49,0",  // area day1 afternoon: min, max, mean
                                          "0.63,0",
                                          "0.77,0",
                                          "0.70,0",  // inland night: min, max, mean
                                          "2.25,0",
                                          "2.28,0",
                                          "2.26,0",  // coast night: min, max, mean
                                          "-0.31,0",
                                          "1.38,0",
                                          "0.53,0",  // area night: min, max, mean
                                          "1.87,0",
                                          "6.85,0",
                                          "4.36,0",  // inland day2: min, max, mean
                                          "2.61,0",
                                          "8.77,0",
                                          "5.69,0",  // coast day2: min, max, mean
                                          "1.33,0",
                                          "8.40,0",
                                          "4.87,0",  // area day2: min, max, mean
                                          "2.21,0",
                                          "2.61,0",
                                          "2.41,0",  // inland day2 morning: min, max, mean
                                          "2.82,0",
                                          "3.84,0",
                                          "3.33,0",  // coast day2 morning: min, max, mean
                                          "2.12,0",
                                          "3.05,0",
                                          "2.58,0",  // area day2 morning: min, max, mean
                                          "5.96,0",
                                          "6.45,0",
                                          "6.21,0",  // inland day2 afternoon: min, max, mean
                                          "4.43,0",
                                          "8.23,0",
                                          "6.33,0",  // coast day2 afternoon: min, max, mean
                                          "5.44,0",
                                          "7.45,0",
                                          "6.44,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on noin 5 astetta, huomenna "
                                          "suunnilleen sama. Yön alin lämpötila on nollan "
                                          "tienoilla tai vähän plussan puolella.")));
    // #81
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("5.10,0",
                                "5.95,0",
                                "5.53,0",  // inland day1: min, max, mean
                                "4.74,0",
                                "5.43,0",
                                "5.09,0",  // coast day1: min, max, mean
                                "4.15,0",
                                "5.87,0",
                                "5.01,0",  // area day1: min, max, mean
                                "5.10,0",
                                "5.69,0",
                                "5.40,0",  // inland day1 morning: min, max, mean
                                "4.74,0",
                                "6.00,0",
                                "5.37,0",  // coast day1 morning: min, max, mean
                                "4.15,0",
                                "4.84,0",
                                "4.49,0",  // area day1 morning: min, max, mean
                                "5.43,0",
                                "5.95,0",
                                "5.69,0",  // inland day1 afternoon: min, max, mean
                                "3.72,0",
                                "5.43,0",
                                "4.58,0",  // coast day1 afternoon: min, max, mean
                                "5.22,0",
                                "5.87,0",
                                "5.54,0",  // area day1 afternoon: min, max, mean
                                "3.21,0",
                                "3.99,0",
                                "3.60,0",  // inland night: min, max, mean
                                "1.55,0",
                                "3.28,0",
                                "2.41,0",  // coast night: min, max, mean
                                "0.24,0",
                                "1.44,0",
                                "0.84,0",  // area night: min, max, mean
                                "2.37,0",
                                "5.77,0",
                                "4.07,0",  // inland day2: min, max, mean
                                "3.62,0",
                                "7.14,0",
                                "5.38,0",  // coast day2: min, max, mean
                                "2.89,0",
                                "9.11,0",
                                "6.00,0",  // area day2: min, max, mean
                                "3.25,0",
                                "3.43,0",
                                "3.34,0",  // inland day2 morning: min, max, mean
                                "3.82,0",
                                "5.49,0",
                                "4.65,0",  // coast day2 morning: min, max, mean
                                "2.94,0",
                                "3.27,0",
                                "3.11,0",  // area day2 morning: min, max, mean
                                "4.96,0",
                                "5.05,0",
                                "5.01,0",  // inland day2 afternoon: min, max, mean
                                "3.61,0",
                                "6.88,0",
                                "5.24,0",  // coast day2 afternoon: min, max, mean
                                "4.72,0",
                                "8.89,0",
                                "6.81,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 5 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on vähän plussan puolella.")));
    // #82
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("4.51,0",
                                "5.30,0",
                                "4.91,0",  // inland day1: min, max, mean
                                "2.66,0",
                                "4.39,0",
                                "3.52,0",  // coast day1: min, max, mean
                                "3.84,0",
                                "4.89,0",
                                "4.36,0",  // area day1: min, max, mean
                                "4.51,0",
                                "4.60,0",
                                "4.56,0",  // inland day1 morning: min, max, mean
                                "2.66,0",
                                "4.46,0",
                                "3.56,0",  // coast day1 morning: min, max, mean
                                "3.84,0",
                                "4.11,0",
                                "3.98,0",  // area day1 morning: min, max, mean
                                "4.72,0",
                                "5.30,0",
                                "5.01,0",  // inland day1 afternoon: min, max, mean
                                "3.54,0",
                                "4.39,0",
                                "3.96,0",  // coast day1 afternoon: min, max, mean
                                "4.63,0",
                                "4.89,0",
                                "4.76,0",  // area day1 afternoon: min, max, mean
                                "-0.11,0",
                                "0.25,0",
                                "0.07,0",  // inland night: min, max, mean
                                "1.86,0",
                                "2.37,0",
                                "2.11,0",  // coast night: min, max, mean
                                "2.62,0",
                                "3.83,0",
                                "3.22,0",  // area night: min, max, mean
                                "4.66,0",
                                "8.23,0",
                                "6.45,0",  // inland day2: min, max, mean
                                "5.48,0",
                                "8.72,0",
                                "7.10,0",  // coast day2: min, max, mean
                                "3.46,0",
                                "12.16,0",
                                "7.81,0",  // area day2: min, max, mean
                                "4.88,0",
                                "5.01,0",
                                "4.95,0",  // inland day2 morning: min, max, mean
                                "5.64,0",
                                "5.68,0",
                                "5.66,0",  // coast day2 morning: min, max, mean
                                "3.89,0",
                                "4.65,0",
                                "4.27,0",  // area day2 morning: min, max, mean
                                "7.65,0",
                                "7.66,0",
                                "7.66,0",  // inland day2 afternoon: min, max, mean
                                "5.70,0",
                                "7.77,0",
                                "6.73,0",  // coast day2 afternoon: min, max, mean
                                "6.97,0",
                                "11.36,0",
                                "9.17,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 5 asteen tuntumassa, huomenna 7...11 "
                                "astetta. Yön alin lämpötila on noin 3 astetta.")));
    // #83
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("5.16,0",
                                "7.97,0",
                                "6.57,0",  // inland day1: min, max, mean
                                "3.45,0",
                                "7.90,0",
                                "5.68,0",  // coast day1: min, max, mean
                                "4.58,0",
                                "7.28,0",
                                "5.93,0",  // area day1: min, max, mean
                                "5.16,0",
                                "5.57,0",
                                "5.36,0",  // inland day1 morning: min, max, mean
                                "3.45,0",
                                "5.38,0",
                                "4.41,0",  // coast day1 morning: min, max, mean
                                "4.58,0",
                                "6.29,0",
                                "5.44,0",  // area day1 morning: min, max, mean
                                "7.43,0",
                                "7.97,0",
                                "7.70,0",  // inland day1 afternoon: min, max, mean
                                "7.02,0",
                                "7.90,0",
                                "7.46,0",  // coast day1 afternoon: min, max, mean
                                "7.03,0",
                                "7.28,0",
                                "7.16,0",  // area day1 afternoon: min, max, mean
                                "3.29,0",
                                "4.27,0",
                                "3.78,0",  // inland night: min, max, mean
                                "1.37,0",
                                "2.85,0",
                                "2.11,0",  // coast night: min, max, mean
                                "3.78,0",
                                "5.50,0",
                                "4.64,0",  // area night: min, max, mean
                                "-1.08,0",
                                "2.99,0",
                                "0.95,0",  // inland day2: min, max, mean
                                "-1.04,0",
                                "1.79,0",
                                "0.37,0",  // coast day2: min, max, mean
                                "-1.04,0",
                                "3.20,0",
                                "1.08,0",  // area day2: min, max, mean
                                "-0.68,0",
                                "-0.54,0",
                                "-0.61,0",  // inland day2 morning: min, max, mean
                                "-0.16,0",
                                "1.27,0",
                                "0.56,0",  // coast day2 morning: min, max, mean
                                "-0.83,0",
                                "-0.24,0",
                                "-0.54,0",  // area day2 morning: min, max, mean
                                "1.71,0",
                                "2.44,0",
                                "2.07,0",  // inland day2 afternoon: min, max, mean
                                "0.27,0",
                                "1.22,0",
                                "0.74,0",  // coast day2 afternoon: min, max, mean
                                "0.74,0",
                                "2.86,0",
                                "1.80,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 7 astetta, huomenna hieman alempi. "
                                "Yön alin lämpötila on lähellä 5 astetta.")));
    // #84
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("5.99,0",
                                "8.56,0",
                                "7.27,0",  // inland day1: min, max, mean
                                "5.61,0",
                                "8.11,0",
                                "6.86,0",  // coast day1: min, max, mean
                                "5.75,0",
                                "11.43,0",
                                "8.59,0",  // area day1: min, max, mean
                                "5.99,0",
                                "6.13,0",
                                "6.06,0",  // inland day1 morning: min, max, mean
                                "5.61,0",
                                "6.62,0",
                                "6.12,0",  // coast day1 morning: min, max, mean
                                "5.75,0",
                                "7.63,0",
                                "6.69,0",  // area day1 morning: min, max, mean
                                "8.55,0",
                                "8.56,0",
                                "8.56,0",  // inland day1 afternoon: min, max, mean
                                "7.71,0",
                                "8.11,0",
                                "7.91,0",  // coast day1 afternoon: min, max, mean
                                "8.44,0",
                                "11.43,0",
                                "9.93,0",  // area day1 afternoon: min, max, mean
                                "2.83,0",
                                "2.98,0",
                                "2.91,0",  // inland night: min, max, mean
                                "2.74,0",
                                "2.87,0",
                                "2.81,0",  // coast night: min, max, mean
                                "4.58,0",
                                "4.97,0",
                                "4.77,0",  // area night: min, max, mean
                                "2.66,0",
                                "4.67,0",
                                "3.66,0",  // inland day2: min, max, mean
                                "2.41,0",
                                "7.72,0",
                                "5.06,0",  // coast day2: min, max, mean
                                "2.24,0",
                                "5.69,0",
                                "3.96,0",  // area day2: min, max, mean
                                "2.97,0",
                                "3.57,0",
                                "3.27,0",  // inland day2 morning: min, max, mean
                                "3.18,0",
                                "3.39,0",
                                "3.28,0",  // coast day2 morning: min, max, mean
                                "2.35,0",
                                "2.51,0",
                                "2.43,0",  // area day2 morning: min, max, mean
                                "4.40,0",
                                "4.52,0",
                                "4.46,0",  // inland day2 afternoon: min, max, mean
                                "4.07,0",
                                "7.16,0",
                                "5.61,0",  // coast day2 afternoon: min, max, mean
                                "3.88,0",
                                "4.75,0",
                                "4.32,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 10 asteen tienoilla, huomenna hieman "
                                "alempi. Yön alin lämpötila on 5 asteen tuntumassa.")));
    // #85
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("5.77,0",
                                "8.01,0",
                                "6.89,0",  // inland day1: min, max, mean
                                "3.88,0",
                                "6.14,0",
                                "5.01,0",  // coast day1: min, max, mean
                                "5.48,0",
                                "8.42,0",
                                "6.95,0",  // area day1: min, max, mean
                                "5.77,0",
                                "6.01,0",
                                "5.89,0",  // inland day1 morning: min, max, mean
                                "3.88,0",
                                "5.31,0",
                                "4.59,0",  // coast day1 morning: min, max, mean
                                "5.48,0",
                                "6.51,0",
                                "6.00,0",  // area day1 morning: min, max, mean
                                "7.10,0",
                                "8.01,0",
                                "7.55,0",  // inland day1 afternoon: min, max, mean
                                "5.50,0",
                                "6.14,0",
                                "5.82,0",  // coast day1 afternoon: min, max, mean
                                "6.62,0",
                                "8.42,0",
                                "7.52,0",  // area day1 afternoon: min, max, mean
                                "2.86,0",
                                "4.47,0",
                                "3.67,0",  // inland night: min, max, mean
                                "2.06,0",
                                "3.28,0",
                                "2.67,0",  // coast night: min, max, mean
                                "1.74,0",
                                "3.53,0",
                                "2.63,0",  // area night: min, max, mean
                                "4.32,0",
                                "8.03,0",
                                "6.17,0",  // inland day2: min, max, mean
                                "5.39,0",
                                "8.65,0",
                                "7.02,0",  // coast day2: min, max, mean
                                "3.56,0",
                                "7.82,0",
                                "5.69,0",  // area day2: min, max, mean
                                "5.14,0",
                                "5.46,0",
                                "5.30,0",  // inland day2 morning: min, max, mean
                                "5.87,0",
                                "5.94,0",
                                "5.90,0",  // coast day2 morning: min, max, mean
                                "4.53,0",
                                "6.41,0",
                                "5.47,0",  // area day2 morning: min, max, mean
                                "6.82,0",
                                "7.25,0",
                                "7.03,0",  // inland day2 afternoon: min, max, mean
                                "5.36,0",
                                "7.70,0",
                                "6.53,0",  // coast day2 afternoon: min, max, mean
                                "6.81,0",
                                "6.88,0",
                                "6.84,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 8 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on 2...4 astetta.")));
    // #86
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("7.33,0",
                                          "10.82,0",
                                          "9.07,0",  // inland day1: min, max, mean
                                          "5.46,0",
                                          "9.99,0",
                                          "7.72,0",  // coast day1: min, max, mean
                                          "6.78,0",
                                          "13.96,0",
                                          "10.37,0",  // area day1: min, max, mean
                                          "7.33,0",
                                          "7.75,0",
                                          "7.54,0",  // inland day1 morning: min, max, mean
                                          "5.46,0",
                                          "5.86,0",
                                          "5.66,0",  // coast day1 morning: min, max, mean
                                          "6.78,0",
                                          "8.22,0",
                                          "7.50,0",  // area day1 morning: min, max, mean
                                          "10.09,0",
                                          "10.82,0",
                                          "10.45,0",  // inland day1 afternoon: min, max, mean
                                          "8.73,0",
                                          "9.99,0",
                                          "9.36,0",  // coast day1 afternoon: min, max, mean
                                          "9.16,0",
                                          "13.96,0",
                                          "11.56,0",  // area day1 afternoon: min, max, mean
                                          "4.32,0",
                                          "5.04,0",
                                          "4.68,0",  // inland night: min, max, mean
                                          "1.56,0",
                                          "3.31,0",
                                          "2.43,0",  // coast night: min, max, mean
                                          "2.83,0",
                                          "4.78,0",
                                          "3.80,0",  // area night: min, max, mean
                                          "5.07,0",
                                          "7.45,0",
                                          "6.26,0",  // inland day2: min, max, mean
                                          "5.51,0",
                                          "5.33,0",
                                          "5.42,0",  // coast day2: min, max, mean
                                          "3.82,0",
                                          "7.99,0",
                                          "5.91,0",  // area day2: min, max, mean
                                          "5.39,0",
                                          "5.55,0",
                                          "5.47,0",  // inland day2 morning: min, max, mean
                                          "6.17,0",
                                          "7.56,0",
                                          "6.87,0",  // coast day2 morning: min, max, mean
                                          "4.51,0",
                                          "5.28,0",
                                          "4.90,0",  // area day2 morning: min, max, mean
                                          "6.21,0",
                                          "7.09,0",
                                          "6.65,0",  // inland day2 afternoon: min, max, mean
                                          "4.64,0",
                                          "5.12,0",
                                          "4.88,0",  // coast day2 afternoon: min, max, mean
                                          "5.88,0",
                                          "7.06,0",
                                          "6.47,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 9...14 astetta, huomenna "
                                          "hieman alempi. Yön alin lämpötila on 3...5 astetta.")));
    // #87
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("7.05,0",
                                "8.64,0",
                                "7.84,0",  // inland day1: min, max, mean
                                "6.40,0",
                                "7.56,0",
                                "6.98,0",  // coast day1: min, max, mean
                                "6.55,0",
                                "8.55,0",
                                "7.55,0",  // area day1: min, max, mean
                                "7.05,0",
                                "7.62,0",
                                "7.33,0",  // inland day1 morning: min, max, mean
                                "6.40,0",
                                "6.43,0",
                                "6.41,0",  // coast day1 morning: min, max, mean
                                "6.55,0",
                                "7.88,0",
                                "7.22,0",  // area day1 morning: min, max, mean
                                "8.37,0",
                                "8.64,0",
                                "8.50,0",  // inland day1 afternoon: min, max, mean
                                "7.04,0",
                                "7.56,0",
                                "7.30,0",  // coast day1 afternoon: min, max, mean
                                "7.92,0",
                                "8.55,0",
                                "8.24,0",  // area day1 afternoon: min, max, mean
                                "2.47,0",
                                "3.82,0",
                                "3.15,0",  // inland night: min, max, mean
                                "3.30,0",
                                "5.18,0",
                                "4.24,0",  // coast night: min, max, mean
                                "2.73,0",
                                "4.63,0",
                                "3.68,0",  // area night: min, max, mean
                                "3.33,0",
                                "8.89,0",
                                "6.11,0",  // inland day2: min, max, mean
                                "3.37,0",
                                "8.77,0",
                                "6.07,0",  // coast day2: min, max, mean
                                "2.65,0",
                                "8.78,0",
                                "5.71,0",  // area day2: min, max, mean
                                "3.90,0",
                                "4.72,0",
                                "4.31,0",  // inland day2 morning: min, max, mean
                                "4.28,0",
                                "5.32,0",
                                "4.80,0",  // coast day2 morning: min, max, mean
                                "3.59,0",
                                "5.00,0",
                                "4.29,0",  // area day2 morning: min, max, mean
                                "7.91,0",
                                "8.01,0",
                                "7.96,0",  // inland day2 afternoon: min, max, mean
                                "6.42,0",
                                "8.51,0",
                                "7.47,0",  // coast day2 afternoon: min, max, mean
                                "7.28,0",
                                "8.12,0",
                                "7.70,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 8 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on 3...5 astetta.")));
    // #88
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("7.09,0",
                                "7.34,0",
                                "7.22,0",  // inland day1: min, max, mean
                                "5.66,0",
                                "8.07,0",
                                "6.87,0",  // coast day1: min, max, mean
                                "6.49,0",
                                "9.05,0",
                                "7.77,0",  // area day1: min, max, mean
                                "7.09,0",
                                "7.71,0",
                                "7.40,0",  // inland day1 morning: min, max, mean
                                "5.66,0",
                                "7.30,0",
                                "6.48,0",  // coast day1 morning: min, max, mean
                                "6.49,0",
                                "6.67,0",
                                "6.58,0",  // area day1 morning: min, max, mean
                                "7.22,0",
                                "7.34,0",
                                "7.28,0",  // inland day1 afternoon: min, max, mean
                                "6.04,0",
                                "8.07,0",
                                "7.06,0",  // coast day1 afternoon: min, max, mean
                                "7.14,0",
                                "9.05,0",
                                "8.10,0",  // area day1 afternoon: min, max, mean
                                "5.24,0",
                                "6.21,0",
                                "5.73,0",  // inland night: min, max, mean
                                "5.42,0",
                                "6.87,0",
                                "6.15,0",  // coast night: min, max, mean
                                "6.34,0",
                                "8.15,0",
                                "7.25,0",  // area night: min, max, mean
                                "9.77,0",
                                "12.22,0",
                                "11.00,0",  // inland day2: min, max, mean
                                "9.25,0",
                                "10.36,0",
                                "9.81,0",  // coast day2: min, max, mean
                                "8.41,0",
                                "14.92,0",
                                "11.66,0",  // area day2: min, max, mean
                                "9.82,0",
                                "10.07,0",
                                "9.95,0",  // inland day2 morning: min, max, mean
                                "10.00,0",
                                "10.23,0",
                                "10.11,0",  // coast day2 morning: min, max, mean
                                "9.12,0",
                                "9.15,0",
                                "9.14,0",  // area day2 morning: min, max, mean
                                "11.24,0",
                                "11.65,0",
                                "11.44,0",  // inland day2 afternoon: min, max, mean
                                "9.77,0",
                                "10.18,0",
                                "9.97,0",  // coast day2 afternoon: min, max, mean
                                "10.71,0",
                                "14.42,0",
                                "12.57,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 7...9 astetta, huomenna 11...14 astetta. "
                                "Yön alin lämpötila on 6...8 astetta.")));
    // #89
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("8.18,0",
                                "9.61,0",
                                "8.90,0",  // inland day1: min, max, mean
                                "7.06,0",
                                "11.64,0",
                                "9.35,0",  // coast day1: min, max, mean
                                "7.32,0",
                                "12.05,0",
                                "9.68,0",  // area day1: min, max, mean
                                "8.18,0",
                                "8.66,0",
                                "8.42,0",  // inland day1 morning: min, max, mean
                                "7.06,0",
                                "8.74,0",
                                "7.90,0",  // coast day1 morning: min, max, mean
                                "7.32,0",
                                "8.46,0",
                                "7.89,0",  // area day1 morning: min, max, mean
                                "9.24,0",
                                "9.61,0",
                                "9.42,0",  // inland day1 afternoon: min, max, mean
                                "8.40,0",
                                "11.64,0",
                                "10.02,0",  // coast day1 afternoon: min, max, mean
                                "8.81,0",
                                "12.05,0",
                                "10.43,0",  // area day1 afternoon: min, max, mean
                                "5.65,0",
                                "6.62,0",
                                "6.14,0",  // inland night: min, max, mean
                                "5.14,0",
                                "6.15,0",
                                "5.64,0",  // coast night: min, max, mean
                                "6.57,0",
                                "6.76,0",
                                "6.67,0",  // area night: min, max, mean
                                "1.78,0",
                                "4.43,0",
                                "3.11,0",  // inland day2: min, max, mean
                                "2.87,0",
                                "5.99,0",
                                "4.43,0",  // coast day2: min, max, mean
                                "0.69,0",
                                "8.98,0",
                                "4.83,0",  // area day2: min, max, mean
                                "2.65,0",
                                "2.98,0",
                                "2.81,0",  // inland day2 morning: min, max, mean
                                "3.50,0",
                                "4.78,0",
                                "4.14,0",  // coast day2 morning: min, max, mean
                                "1.67,0",
                                "1.83,0",
                                "1.75,0",  // area day2 morning: min, max, mean
                                "3.83,0",
                                "4.03,0",
                                "3.93,0",  // inland day2 afternoon: min, max, mean
                                "2.41,0",
                                "5.71,0",
                                "4.06,0",  // coast day2 afternoon: min, max, mean
                                "3.34,0",
                                "8.30,0",
                                "5.82,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 9...12 astetta, huomenna hieman alempi. "
                                "Yön alin lämpötila on vähän yli 5 astetta.")));
    // #90
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("8.62,0",
                                "9.71,0",
                                "9.17,0",  // inland day1: min, max, mean
                                "8.53,0",
                                "13.45,0",
                                "10.99,0",  // coast day1: min, max, mean
                                "7.86,0",
                                "11.75,0",
                                "9.81,0",  // area day1: min, max, mean
                                "8.62,0",
                                "8.93,0",
                                "8.78,0",  // inland day1 morning: min, max, mean
                                "8.53,0",
                                "9.38,0",
                                "8.96,0",  // coast day1 morning: min, max, mean
                                "7.86,0",
                                "9.01,0",
                                "8.43,0",  // area day1 morning: min, max, mean
                                "9.58,0",
                                "9.71,0",
                                "9.64,0",  // inland day1 afternoon: min, max, mean
                                "9.18,0",
                                "13.45,0",
                                "11.32,0",  // coast day1 afternoon: min, max, mean
                                "9.39,0",
                                "11.75,0",
                                "10.57,0",  // area day1 afternoon: min, max, mean
                                "5.97,0",
                                "6.43,0",
                                "6.20,0",  // inland night: min, max, mean
                                "6.39,0",
                                "6.81,0",
                                "6.60,0",  // coast night: min, max, mean
                                "6.54,0",
                                "7.83,0",
                                "7.19,0",  // area night: min, max, mean
                                "7.19,0",
                                "11.60,0",
                                "9.40,0",  // inland day2: min, max, mean
                                "8.18,0",
                                "10.37,0",
                                "9.28,0",  // coast day2: min, max, mean
                                "6.93,0",
                                "12.54,0",
                                "9.73,0",  // area day2: min, max, mean
                                "7.87,0",
                                "8.13,0",
                                "8.00,0",  // inland day2 morning: min, max, mean
                                "8.79,0",
                                "10.30,0",
                                "9.55,0",  // coast day2 morning: min, max, mean
                                "7.53,0",
                                "7.84,0",
                                "7.69,0",  // area day2 morning: min, max, mean
                                "9.99,0",
                                "10.74,0",
                                "10.37,0",  // inland day2 afternoon: min, max, mean
                                "8.87,0",
                                "9.77,0",
                                "9.32,0",  // coast day2 afternoon: min, max, mean
                                "9.69,0",
                                "11.80,0",
                                "10.74,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 10 asteen tienoilla, huomenna "
                                "suunnilleen sama. Yön alin lämpötila on noin 7 astetta.")));
    // #91
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("9.00,0",
                                "10.75,0",
                                "9.88,0",  // inland day1: min, max, mean
                                "8.25,0",
                                "11.57,0",
                                "9.91,0",  // coast day1: min, max, mean
                                "8.51,0",
                                "13.91,0",
                                "11.21,0",  // area day1: min, max, mean
                                "9.00,0",
                                "9.16,0",
                                "9.08,0",  // inland day1 morning: min, max, mean
                                "8.25,0",
                                "8.39,0",
                                "8.32,0",  // coast day1 morning: min, max, mean
                                "8.51,0",
                                "9.65,0",
                                "9.08,0",  // area day1 morning: min, max, mean
                                "10.35,0",
                                "10.75,0",
                                "10.55,0",  // inland day1 afternoon: min, max, mean
                                "10.17,0",
                                "11.57,0",
                                "10.87,0",  // coast day1 afternoon: min, max, mean
                                "9.92,0",
                                "13.91,0",
                                "11.91,0",  // area day1 afternoon: min, max, mean
                                "4.13,0",
                                "5.96,0",
                                "5.05,0",  // inland night: min, max, mean
                                "7.91,0",
                                "7.93,0",
                                "7.92,0",  // coast night: min, max, mean
                                "4.96,0",
                                "6.06,0",
                                "5.51,0",  // area night: min, max, mean
                                "12.96,0",
                                "16.17,0",
                                "14.57,0",  // inland day2: min, max, mean
                                "13.45,0",
                                "18.19,0",
                                "15.82,0",  // coast day2: min, max, mean
                                "12.33,0",
                                "20.82,0",
                                "16.57,0",  // area day2: min, max, mean
                                "13.74,0",
                                "14.35,0",
                                "14.05,0",  // inland day2 morning: min, max, mean
                                "13.93,0",
                                "14.56,0",
                                "14.25,0",  // coast day2 morning: min, max, mean
                                "13.30,0",
                                "14.14,0",
                                "13.72,0",  // area day2 morning: min, max, mean
                                "15.41,0",
                                "15.99,0",
                                "15.70,0",  // inland day2 afternoon: min, max, mean
                                "15.15,0",
                                "17.63,0",
                                "16.39,0",  // coast day2 afternoon: min, max, mean
                                "15.36,0",
                                "20.35,0",
                                "17.86,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 10...14 astetta, huomenna 15...20 "
                                "astetta. Yön alin lämpötila on noin 5 astetta.")));
    // #92
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("10.00,0",
                                "12.72,0",
                                "11.36,0",  // inland day1: min, max, mean
                                "9.77,0",
                                "13.16,0",
                                "11.47,0",  // coast day1: min, max, mean
                                "9.39,0",
                                "12.43,0",
                                "10.91,0",  // area day1: min, max, mean
                                "10.00,0",
                                "10.87,0",
                                "10.44,0",  // inland day1 morning: min, max, mean
                                "9.77,0",
                                "10.97,0",
                                "10.37,0",  // coast day1 morning: min, max, mean
                                "9.39,0",
                                "11.16,0",
                                "10.27,0",  // area day1 morning: min, max, mean
                                "12.36,0",
                                "12.72,0",
                                "12.54,0",  // inland day1 afternoon: min, max, mean
                                "11.43,0",
                                "13.16,0",
                                "12.29,0",  // coast day1 afternoon: min, max, mean
                                "12.01,0",
                                "12.43,0",
                                "12.22,0",  // area day1 afternoon: min, max, mean
                                "9.12,0",
                                "10.62,0",
                                "9.87,0",  // inland night: min, max, mean
                                "9.70,0",
                                "11.10,0",
                                "10.40,0",  // coast night: min, max, mean
                                "7.22,0",
                                "8.21,0",
                                "7.72,0",  // area night: min, max, mean
                                "10.92,0",
                                "13.44,0",
                                "12.18,0",  // inland day2: min, max, mean
                                "11.55,0",
                                "11.66,0",
                                "11.61,0",  // coast day2: min, max, mean
                                "9.68,0",
                                "14.47,0",
                                "12.08,0",  // area day2: min, max, mean
                                "11.26,0",
                                "12.12,0",
                                "11.69,0",  // inland day2 morning: min, max, mean
                                "12.18,0",
                                "13.74,0",
                                "12.96,0",  // coast day2 morning: min, max, mean
                                "10.48,0",
                                "10.86,0",
                                "10.67,0",  // area day2 morning: min, max, mean
                                "12.16,0",
                                "12.63,0",
                                "12.40,0",  // inland day2 afternoon: min, max, mean
                                "10.76,0",
                                "11.29,0",
                                "11.03,0",  // coast day2 afternoon: min, max, mean
                                "11.76,0",
                                "14.16,0",
                                "12.96,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 12 astetta, huomenna hieman "
                                "korkeampi. Yön alin lämpötila on noin 8 astetta.")));
    // #93
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("10.38,0",
                                "13.09,0",
                                "11.73,0",  // inland day1: min, max, mean
                                "10.13,0",
                                "11.42,0",
                                "10.78,0",  // coast day1: min, max, mean
                                "9.99,0",
                                "16.09,0",
                                "13.04,0",  // area day1: min, max, mean
                                "10.38,0",
                                "11.34,0",
                                "10.86,0",  // inland day1 morning: min, max, mean
                                "10.13,0",
                                "11.69,0",
                                "10.91,0",  // coast day1 morning: min, max, mean
                                "9.99,0",
                                "11.82,0",
                                "10.90,0",  // area day1 morning: min, max, mean
                                "12.89,0",
                                "13.09,0",
                                "12.99,0",  // inland day1 afternoon: min, max, mean
                                "11.16,0",
                                "11.42,0",
                                "11.29,0",  // coast day1 afternoon: min, max, mean
                                "12.63,0",
                                "16.09,0",
                                "14.36,0",  // area day1 afternoon: min, max, mean
                                "8.51,0",
                                "10.42,0",
                                "9.47,0",  // inland night: min, max, mean
                                "9.58,0",
                                "9.64,0",
                                "9.61,0",  // coast night: min, max, mean
                                "6.80,0",
                                "8.71,0",
                                "7.75,0",  // area night: min, max, mean
                                "10.37,0",
                                "13.39,0",
                                "11.88,0",  // inland day2: min, max, mean
                                "11.44,0",
                                "14.16,0",
                                "12.80,0",  // coast day2: min, max, mean
                                "10.36,0",
                                "13.29,0",
                                "11.83,0",  // area day2: min, max, mean
                                "10.62,0",
                                "11.50,0",
                                "11.06,0",  // inland day2 morning: min, max, mean
                                "11.46,0",
                                "13.40,0",
                                "12.43,0",  // coast day2 morning: min, max, mean
                                "10.47,0",
                                "10.70,0",
                                "10.59,0",  // area day2 morning: min, max, mean
                                "12.37,0",
                                "13.10,0",
                                "12.73,0",  // inland day2 afternoon: min, max, mean
                                "11.05,0",
                                "13.18,0",
                                "12.12,0",  // coast day2 afternoon: min, max, mean
                                "11.58,0",
                                "12.88,0",
                                "12.23,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 13...16 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on 7...9 astetta.")));
    // #94
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("11.18,0",
                                "12.15,0",
                                "11.66,0",  // inland day1: min, max, mean
                                "10.65,0",
                                "13.27,0",
                                "11.96,0",  // coast day1: min, max, mean
                                "10.53,0",
                                "12.89,0",
                                "11.71,0",  // area day1: min, max, mean
                                "11.18,0",
                                "11.89,0",
                                "11.53,0",  // inland day1 morning: min, max, mean
                                "10.65,0",
                                "12.40,0",
                                "11.53,0",  // coast day1 morning: min, max, mean
                                "10.53,0",
                                "10.79,0",
                                "10.66,0",  // area day1 morning: min, max, mean
                                "11.54,0",
                                "12.15,0",
                                "11.84,0",  // inland day1 afternoon: min, max, mean
                                "10.53,0",
                                "13.27,0",
                                "11.90,0",  // coast day1 afternoon: min, max, mean
                                "11.26,0",
                                "12.89,0",
                                "12.08,0",  // area day1 afternoon: min, max, mean
                                "11.03,0",
                                "11.84,0",
                                "11.44,0",  // inland night: min, max, mean
                                "8.94,0",
                                "10.78,0",
                                "9.86,0",  // coast night: min, max, mean
                                "7.95,0",
                                "8.15,0",
                                "8.05,0",  // area night: min, max, mean
                                "15.81,0",
                                "17.32,0",
                                "16.56,0",  // inland day2: min, max, mean
                                "16.03,0",
                                "16.19,0",
                                "16.11,0",  // coast day2: min, max, mean
                                "14.68,0",
                                "18.44,0",
                                "16.56,0",  // area day2: min, max, mean
                                "16.01,0",
                                "16.72,0",
                                "16.37,0",  // inland day2 morning: min, max, mean
                                "16.83,0",
                                "18.39,0",
                                "17.61,0",  // coast day2 morning: min, max, mean
                                "15.04,0",
                                "15.08,0",
                                "15.06,0",  // area day2 morning: min, max, mean
                                "16.63,0",
                                "17.21,0",
                                "16.92,0",  // inland day2 afternoon: min, max, mean
                                "15.37,0",
                                "15.82,0",
                                "15.60,0",  // coast day2 afternoon: min, max, mean
                                "16.45,0",
                                "17.68,0",
                                "17.07,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 11...13 astetta, huomenna 16...18 "
                                "astetta. Yön alin lämpötila on noin 8 astetta.")));
    // #95
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("12.47,0",
                                "14.63,0",
                                "13.55,0",  // inland day1: min, max, mean
                                "10.94,0",
                                "14.11,0",
                                "12.52,0",  // coast day1: min, max, mean
                                "11.65,0",
                                "14.25,0",
                                "12.95,0",  // area day1: min, max, mean
                                "12.47,0",
                                "12.67,0",
                                "12.57,0",  // inland day1 morning: min, max, mean
                                "10.94,0",
                                "11.26,0",
                                "11.10,0",  // coast day1 morning: min, max, mean
                                "11.65,0",
                                "13.46,0",
                                "12.55,0",  // area day1 morning: min, max, mean
                                "14.37,0",
                                "14.63,0",
                                "14.50,0",  // inland day1 afternoon: min, max, mean
                                "12.98,0",
                                "14.11,0",
                                "13.55,0",  // coast day1 afternoon: min, max, mean
                                "13.56,0",
                                "14.25,0",
                                "13.91,0",  // area day1 afternoon: min, max, mean
                                "8.87,0",
                                "10.11,0",
                                "9.49,0",  // inland night: min, max, mean
                                "7.18,0",
                                "7.33,0",
                                "7.25,0",  // coast night: min, max, mean
                                "7.99,0",
                                "8.56,0",
                                "8.27,0",  // area night: min, max, mean
                                "14.04,0",
                                "19.53,0",
                                "16.79,0",  // inland day2: min, max, mean
                                "14.65,0",
                                "19.18,0",
                                "16.91,0",  // coast day2: min, max, mean
                                "13.74,0",
                                "21.06,0",
                                "17.40,0",  // area day2: min, max, mean
                                "14.48,0",
                                "15.13,0",
                                "14.81,0",  // inland day2 morning: min, max, mean
                                "15.35,0",
                                "16.50,0",
                                "15.92,0",  // coast day2 morning: min, max, mean
                                "14.46,0",
                                "14.51,0",
                                "14.49,0",  // area day2 morning: min, max, mean
                                "17.85,0",
                                "18.60,0",
                                "18.22,0",  // inland day2 afternoon: min, max, mean
                                "17.52,0",
                                "19.11,0",
                                "18.32,0",  // coast day2 afternoon: min, max, mean
                                "17.06,0",
                                "20.86,0",
                                "18.96,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 15 asteen tuntumassa, huomenna 17...21 "
                                "astetta. Yön alin lämpötila on noin 8 astetta.")));
    // #96
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("12.40,0",
                                "15.88,0",
                                "14.14,0",  // inland day1: min, max, mean
                                "11.57,0",
                                "17.96,0",
                                "14.77,0",  // coast day1: min, max, mean
                                "12.22,0",
                                "19.61,0",
                                "15.92,0",  // area day1: min, max, mean
                                "12.40,0",
                                "12.67,0",
                                "12.54,0",  // inland day1 morning: min, max, mean
                                "11.57,0",
                                "12.86,0",
                                "12.21,0",  // coast day1 morning: min, max, mean
                                "12.22,0",
                                "13.80,0",
                                "13.01,0",  // area day1 morning: min, max, mean
                                "15.51,0",
                                "15.88,0",
                                "15.69,0",  // inland day1 afternoon: min, max, mean
                                "14.02,0",
                                "17.96,0",
                                "15.99,0",  // coast day1 afternoon: min, max, mean
                                "14.78,0",
                                "19.61,0",
                                "17.19,0",  // area day1 afternoon: min, max, mean
                                "9.81,0",
                                "9.84,0",
                                "9.83,0",  // inland night: min, max, mean
                                "8.06,0",
                                "8.89,0",
                                "8.48,0",  // coast night: min, max, mean
                                "9.71,0",
                                "11.29,0",
                                "10.50,0",  // area night: min, max, mean
                                "15.12,0",
                                "21.19,0",
                                "18.16,0",  // inland day2: min, max, mean
                                "15.39,0",
                                "20.45,0",
                                "17.92,0",  // coast day2: min, max, mean
                                "15.06,0",
                                "20.97,0",
                                "18.01,0",  // area day2: min, max, mean
                                "15.92,0",
                                "16.05,0",
                                "15.98,0",  // inland day2 morning: min, max, mean
                                "16.21,0",
                                "16.99,0",
                                "16.60,0",  // coast day2 morning: min, max, mean
                                "15.61,0",
                                "16.52,0",
                                "16.07,0",  // area day2 morning: min, max, mean
                                "20.10,0",
                                "20.75,0",
                                "20.43,0",  // inland day2 afternoon: min, max, mean
                                "19.25,0",
                                "20.20,0",
                                "19.73,0",  // coast day2 afternoon: min, max, mean
                                "19.30,0",
                                "20.46,0",
                                "19.88,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 15...20 astetta, huomenna hieman "
                                "korkeampi. Yön alin lämpötila on noin 10 astetta.")));
    // #97
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("13.15,0",
                                          "14.23,0",
                                          "13.69,0",  // inland day1: min, max, mean
                                          "12.28,0",
                                          "14.31,0",
                                          "13.30,0",  // coast day1: min, max, mean
                                          "12.40,0",
                                          "16.31,0",
                                          "14.36,0",  // area day1: min, max, mean
                                          "13.15,0",
                                          "13.31,0",
                                          "13.23,0",  // inland day1 morning: min, max, mean
                                          "12.28,0",
                                          "13.08,0",
                                          "12.68,0",  // coast day1 morning: min, max, mean
                                          "12.40,0",
                                          "13.08,0",
                                          "12.74,0",  // area day1 morning: min, max, mean
                                          "13.75,0",
                                          "14.23,0",
                                          "13.99,0",  // inland day1 afternoon: min, max, mean
                                          "12.08,0",
                                          "14.31,0",
                                          "13.19,0",  // coast day1 afternoon: min, max, mean
                                          "13.73,0",
                                          "16.31,0",
                                          "15.02,0",  // area day1 afternoon: min, max, mean
                                          "12.92,0",
                                          "13.07,0",
                                          "13.00,0",  // inland night: min, max, mean
                                          "8.53,0",
                                          "8.74,0",
                                          "8.64,0",  // coast night: min, max, mean
                                          "9.31,0",
                                          "10.98,0",
                                          "10.15,0",  // area night: min, max, mean
                                          "8.11,0",
                                          "10.87,0",
                                          "9.49,0",  // inland day2: min, max, mean
                                          "8.96,0",
                                          "11.93,0",
                                          "10.44,0",  // coast day2: min, max, mean
                                          "7.51,0",
                                          "14.92,0",
                                          "11.21,0",  // area day2: min, max, mean
                                          "8.55,0",
                                          "9.24,0",
                                          "8.89,0",  // inland day2 morning: min, max, mean
                                          "9.36,0",
                                          "10.84,0",
                                          "10.10,0",  // coast day2 morning: min, max, mean
                                          "8.51,0",
                                          "10.17,0",
                                          "9.34,0",  // area day2 morning: min, max, mean
                                          "10.36,0",
                                          "10.44,0",
                                          "10.40,0",  // inland day2 afternoon: min, max, mean
                                          "10.06,0",
                                          "11.80,0",
                                          "10.93,0",  // coast day2 afternoon: min, max, mean
                                          "10.19,0",
                                          "14.15,0",
                                          "12.17,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on noin 15 astetta, huomenna "
                                          "hieman alempi. Sisämaassa yön alin lämpötila on noin 13 "
                                          "astetta, rannikolla 10 asteen tuntumassa.")));
    // #98
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("13.56,0",
                                "16.09,0",
                                "14.83,0",  // inland day1: min, max, mean
                                "11.75,0",
                                "19.95,0",
                                "15.85,0",  // coast day1: min, max, mean
                                "13.43,0",
                                "14.54,0",
                                "13.99,0",  // area day1: min, max, mean
                                "13.56,0",
                                "13.90,0",
                                "13.73,0",  // inland day1 morning: min, max, mean
                                "11.75,0",
                                "12.74,0",
                                "12.25,0",  // coast day1 morning: min, max, mean
                                "13.43,0",
                                "14.25,0",
                                "13.84,0",  // area day1 morning: min, max, mean
                                "15.50,0",
                                "16.09,0",
                                "15.80,0",  // inland day1 afternoon: min, max, mean
                                "15.45,0",
                                "19.95,0",
                                "17.70,0",  // coast day1 afternoon: min, max, mean
                                "14.53,0",
                                "14.54,0",
                                "14.54,0",  // area day1 afternoon: min, max, mean
                                "8.63,0",
                                "9.24,0",
                                "8.94,0",  // inland night: min, max, mean
                                "8.75,0",
                                "10.12,0",
                                "9.44,0",  // coast night: min, max, mean
                                "13.39,0",
                                "14.49,0",
                                "13.94,0",  // area night: min, max, mean
                                "16.53,0",
                                "19.91,0",
                                "18.22,0",  // inland day2: min, max, mean
                                "17.54,0",
                                "19.05,0",
                                "18.30,0",  // coast day2: min, max, mean
                                "16.86,0",
                                "22.84,0",
                                "19.85,0",  // area day2: min, max, mean
                                "17.27,0",
                                "17.55,0",
                                "17.41,0",  // inland day2 morning: min, max, mean
                                "17.75,0",
                                "19.52,0",
                                "18.64,0",  // coast day2 morning: min, max, mean
                                "17.20,0",
                                "17.35,0",
                                "17.28,0",  // area day2 morning: min, max, mean
                                "19.16,0",
                                "19.54,0",
                                "19.35,0",  // inland day2 afternoon: min, max, mean
                                "17.72,0",
                                "18.73,0",
                                "18.22,0",  // coast day2 afternoon: min, max, mean
                                "18.95,0",
                                "22.03,0",
                                "20.49,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 15 asteen tuntumassa, huomenna 19...22 "
                                "astetta. Yön alin lämpötila on vajaat 15 astetta.")));
    // #99
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("14.47,0",
                                "15.94,0",
                                "15.20,0",  // inland day1: min, max, mean
                                "12.92,0",
                                "18.01,0",
                                "15.47,0",  // coast day1: min, max, mean
                                "14.20,0",
                                "19.72,0",
                                "16.96,0",  // area day1: min, max, mean
                                "14.47,0",
                                "15.02,0",
                                "14.74,0",  // inland day1 morning: min, max, mean
                                "12.92,0",
                                "13.82,0",
                                "13.37,0",  // coast day1 morning: min, max, mean
                                "14.20,0",
                                "14.92,0",
                                "14.56,0",  // area day1 morning: min, max, mean
                                "15.23,0",
                                "15.94,0",
                                "15.59,0",  // inland day1 afternoon: min, max, mean
                                "13.85,0",
                                "18.01,0",
                                "15.93,0",  // coast day1 afternoon: min, max, mean
                                "15.23,0",
                                "19.72,0",
                                "17.47,0",  // area day1 afternoon: min, max, mean
                                "11.60,0",
                                "13.46,0",
                                "12.53,0",  // inland night: min, max, mean
                                "12.00,0",
                                "12.40,0",
                                "12.20,0",  // coast night: min, max, mean
                                "11.39,0",
                                "12.33,0",
                                "11.86,0",  // area night: min, max, mean
                                "16.09,0",
                                "19.69,0",
                                "17.89,0",  // inland day2: min, max, mean
                                "16.71,0",
                                "19.31,0",
                                "18.01,0",  // coast day2: min, max, mean
                                "15.15,0",
                                "22.83,0",
                                "18.99,0",  // area day2: min, max, mean
                                "16.45,0",
                                "17.08,0",
                                "16.77,0",  // inland day2 morning: min, max, mean
                                "17.35,0",
                                "17.47,0",
                                "17.41,0",  // coast day2 morning: min, max, mean
                                "15.92,0",
                                "16.69,0",
                                "16.31,0",  // area day2 morning: min, max, mean
                                "19.32,0",
                                "19.40,0",
                                "19.36,0",  // inland day2 afternoon: min, max, mean
                                "17.81,0",
                                "18.58,0",
                                "18.19,0",  // coast day2 afternoon: min, max, mean
                                "18.57,0",
                                "22.56,0",
                                "20.56,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 15...20 astetta, huomenna hieman "
                                "korkeampi. Yön alin lämpötila on noin 12 astetta.")));
    // #100
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("14.76,0",
                                "17.89,0",
                                "16.33,0",  // inland day1: min, max, mean
                                "12.89,0",
                                "20.01,0",
                                "16.45,0",  // coast day1: min, max, mean
                                "14.52,0",
                                "19.82,0",
                                "17.17,0",  // area day1: min, max, mean
                                "14.76,0",
                                "15.64,0",
                                "15.20,0",  // inland day1 morning: min, max, mean
                                "12.89,0",
                                "14.54,0",
                                "13.72,0",  // coast day1 morning: min, max, mean
                                "14.52,0",
                                "15.30,0",
                                "14.91,0",  // area day1 morning: min, max, mean
                                "17.29,0",
                                "17.89,0",
                                "17.59,0",  // inland day1 afternoon: min, max, mean
                                "17.22,0",
                                "20.01,0",
                                "18.62,0",  // coast day1 afternoon: min, max, mean
                                "16.30,0",
                                "19.82,0",
                                "18.06,0",  // area day1 afternoon: min, max, mean
                                "9.90,0",
                                "10.27,0",
                                "10.09,0",  // inland night: min, max, mean
                                "9.40,0",
                                "10.35,0",
                                "9.88,0",  // coast night: min, max, mean
                                "11.25,0",
                                "13.07,0",
                                "12.16,0",  // area night: min, max, mean
                                "16.41,0",
                                "19.83,0",
                                "18.12,0",  // inland day2: min, max, mean
                                "17.26,0",
                                "18.95,0",
                                "18.11,0",  // coast day2: min, max, mean
                                "16.26,0",
                                "19.70,0",
                                "17.98,0",  // area day2: min, max, mean
                                "16.88,0",
                                "17.74,0",
                                "17.31,0",  // inland day2 morning: min, max, mean
                                "17.72,0",
                                "19.14,0",
                                "18.43,0",  // coast day2 morning: min, max, mean
                                "16.45,0",
                                "17.74,0",
                                "17.10,0",  // area day2 morning: min, max, mean
                                "18.78,0",
                                "19.28,0",
                                "19.03,0",  // inland day2 afternoon: min, max, mean
                                "18.72,0",
                                "18.86,0",
                                "18.79,0",  // coast day2 afternoon: min, max, mean
                                "18.08,0",
                                "19.19,0",
                                "18.64,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 16...20 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on 11...13 astetta.")));
    // #101
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("14.64,0",
                                "16.26,0",
                                "15.45,0",  // inland day1: min, max, mean
                                "13.55,0",
                                "16.52,0",
                                "15.04,0",  // coast day1: min, max, mean
                                "14.42,0",
                                "17.25,0",
                                "15.83,0",  // area day1: min, max, mean
                                "14.64,0",
                                "15.15,0",
                                "14.89,0",  // inland day1 morning: min, max, mean
                                "13.55,0",
                                "15.12,0",
                                "14.34,0",  // coast day1 morning: min, max, mean
                                "14.42,0",
                                "14.85,0",
                                "14.63,0",  // area day1 morning: min, max, mean
                                "16.14,0",
                                "16.26,0",
                                "16.20,0",  // inland day1 afternoon: min, max, mean
                                "14.86,0",
                                "16.52,0",
                                "15.69,0",  // coast day1 afternoon: min, max, mean
                                "15.35,0",
                                "17.25,0",
                                "16.30,0",  // area day1 afternoon: min, max, mean
                                "10.24,0",
                                "10.66,0",
                                "10.45,0",  // inland night: min, max, mean
                                "12.69,0",
                                "12.87,0",
                                "12.78,0",  // coast night: min, max, mean
                                "10.55,0",
                                "11.79,0",
                                "11.17,0",  // area night: min, max, mean
                                "11.65,0",
                                "15.24,0",
                                "13.45,0",  // inland day2: min, max, mean
                                "12.38,0",
                                "16.19,0",
                                "14.28,0",  // coast day2: min, max, mean
                                "11.59,0",
                                "15.98,0",
                                "13.79,0",  // area day2: min, max, mean
                                "12.14,0",
                                "12.51,0",
                                "12.32,0",  // inland day2 morning: min, max, mean
                                "12.67,0",
                                "13.90,0",
                                "13.29,0",  // coast day2 morning: min, max, mean
                                "12.03,0",
                                "12.84,0",
                                "12.43,0",  // area day2 morning: min, max, mean
                                "13.86,0",
                                "14.66,0",
                                "14.26,0",  // inland day2 afternoon: min, max, mean
                                "13.36,0",
                                "16.05,0",
                                "14.70,0",  // coast day2 afternoon: min, max, mean
                                "13.14,0",
                                "15.60,0",
                                "14.37,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 15...17 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on noin 10 astetta.")));
    // #102
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("15.75,0",
                                "18.65,0",
                                "17.20,0",  // inland day1: min, max, mean
                                "14.19,0",
                                "20.59,0",
                                "17.39,0",  // coast day1: min, max, mean
                                "15.56,0",
                                "18.17,0",
                                "16.87,0",  // area day1: min, max, mean
                                "15.75,0",
                                "16.64,0",
                                "16.19,0",  // inland day1 morning: min, max, mean
                                "14.19,0",
                                "15.49,0",
                                "14.84,0",  // coast day1 morning: min, max, mean
                                "15.56,0",
                                "16.78,0",
                                "16.17,0",  // area day1 morning: min, max, mean
                                "17.98,0",
                                "18.65,0",
                                "18.31,0",  // inland day1 afternoon: min, max, mean
                                "17.77,0",
                                "20.59,0",
                                "19.18,0",  // coast day1 afternoon: min, max, mean
                                "17.57,0",
                                "18.17,0",
                                "17.87,0",  // area day1 afternoon: min, max, mean
                                "13.90,0",
                                "15.21,0",
                                "14.56,0",  // inland night: min, max, mean
                                "13.77,0",
                                "14.99,0",
                                "14.38,0",  // coast night: min, max, mean
                                "12.88,0",
                                "14.44,0",
                                "13.66,0",  // area night: min, max, mean
                                "19.57,0",
                                "25.72,0",
                                "22.64,0",  // inland day2: min, max, mean
                                "20.73,0",
                                "25.36,0",
                                "23.04,0",  // coast day2: min, max, mean
                                "19.61,0",
                                "24.68,0",
                                "22.15,0",  // area day2: min, max, mean
                                "20.56,0",
                                "21.40,0",
                                "20.98,0",  // inland day2 morning: min, max, mean
                                "21.03,0",
                                "21.13,0",
                                "21.08,0",  // coast day2 morning: min, max, mean
                                "20.06,0",
                                "21.34,0",
                                "20.70,0",  // area day2 morning: min, max, mean
                                "24.36,0",
                                "24.96,0",
                                "24.66,0",  // inland day2 afternoon: min, max, mean
                                "23.03,0",
                                "24.57,0",
                                "23.80,0",  // coast day2 afternoon: min, max, mean
                                "23.70,0",
                                "24.58,0",
                                "24.14,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 18 astetta, huomenna 25 asteen "
                                "tuntumassa. Yön alin lämpötila on noin 15 astetta.")));
    // #103
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("17.01,0",
                                "18.69,0",
                                "17.85,0",  // inland day1: min, max, mean
                                "16.57,0",
                                "21.87,0",
                                "19.22,0",  // coast day1: min, max, mean
                                "16.41,0",
                                "21.49,0",
                                "18.95,0",  // area day1: min, max, mean
                                "17.01,0",
                                "17.65,0",
                                "17.33,0",  // inland day1 morning: min, max, mean
                                "16.57,0",
                                "18.50,0",
                                "17.54,0",  // coast day1 morning: min, max, mean
                                "16.41,0",
                                "17.66,0",
                                "17.04,0",  // area day1 morning: min, max, mean
                                "18.68,0",
                                "18.69,0",
                                "18.68,0",  // inland day1 afternoon: min, max, mean
                                "17.60,0",
                                "21.87,0",
                                "19.74,0",  // coast day1 afternoon: min, max, mean
                                "18.20,0",
                                "21.49,0",
                                "19.84,0",  // area day1 afternoon: min, max, mean
                                "14.51,0",
                                "16.14,0",
                                "15.33,0",  // inland night: min, max, mean
                                "14.37,0",
                                "15.79,0",
                                "15.08,0",  // coast night: min, max, mean
                                "15.57,0",
                                "17.56,0",
                                "16.57,0",  // area night: min, max, mean
                                "14.01,0",
                                "18.22,0",
                                "16.11,0",  // inland day2: min, max, mean
                                "14.40,0",
                                "17.27,0",
                                "15.84,0",  // coast day2: min, max, mean
                                "12.83,0",
                                "18.92,0",
                                "15.87,0",  // area day2: min, max, mean
                                "14.72,0",
                                "15.12,0",
                                "14.92,0",  // inland day2 morning: min, max, mean
                                "14.80,0",
                                "15.92,0",
                                "15.36,0",  // coast day2 morning: min, max, mean
                                "13.73,0",
                                "14.92,0",
                                "14.32,0",  // area day2 morning: min, max, mean
                                "16.91,0",
                                "17.65,0",
                                "17.28,0",  // inland day2 afternoon: min, max, mean
                                "15.09,0",
                                "16.61,0",
                                "15.85,0",  // coast day2 afternoon: min, max, mean
                                "16.60,0",
                                "17.95,0",
                                "17.28,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 20 asteen tienoilla, huomenna "
                                "suunnilleen sama. Yön alin lämpötila on 16...18 astetta.")));
    // #104
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("18.00,0",
                                "18.87,0",
                                "18.44,0",  // inland day1: min, max, mean
                                "16.78,0",
                                "20.02,0",
                                "18.40,0",  // coast day1: min, max, mean
                                "17.17,0",
                                "22.64,0",
                                "19.90,0",  // area day1: min, max, mean
                                "18.00,0",
                                "18.61,0",
                                "18.31,0",  // inland day1 morning: min, max, mean
                                "16.78,0",
                                "16.92,0",
                                "16.85,0",  // coast day1 morning: min, max, mean
                                "17.17,0",
                                "17.47,0",
                                "17.32,0",  // area day1 morning: min, max, mean
                                "18.77,0",
                                "18.87,0",
                                "18.82,0",  // inland day1 afternoon: min, max, mean
                                "17.07,0",
                                "20.02,0",
                                "18.54,0",  // coast day1 afternoon: min, max, mean
                                "17.81,0",
                                "22.64,0",
                                "20.22,0",  // area day1 afternoon: min, max, mean
                                "16.59,0",
                                "18.15,0",
                                "17.37,0",  // inland night: min, max, mean
                                "15.60,0",
                                "16.99,0",
                                "16.30,0",  // coast night: min, max, mean
                                "16.99,0",
                                "17.83,0",
                                "17.41,0",  // area night: min, max, mean
                                "11.98,0",
                                "17.63,0",
                                "14.80,0",  // inland day2: min, max, mean
                                "13.21,0",
                                "19.68,0",
                                "16.45,0",  // coast day2: min, max, mean
                                "12.02,0",
                                "17.55,0",
                                "14.78,0",  // area day2: min, max, mean
                                "12.83,0",
                                "13.26,0",
                                "13.05,0",  // inland day2 morning: min, max, mean
                                "13.58,0",
                                "13.86,0",
                                "13.72,0",  // coast day2 morning: min, max, mean
                                "12.20,0",
                                "13.07,0",
                                "12.64,0",  // area day2 morning: min, max, mean
                                "16.04,0",
                                "16.80,0",
                                "16.42,0",  // inland day2 afternoon: min, max, mean
                                "15.68,0",
                                "19.45,0",
                                "17.56,0",  // coast day2 afternoon: min, max, mean
                                "15.78,0",
                                "16.60,0",
                                "16.19,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 18...23 astetta, huomenna hieman alempi. "
                                "Yön alin lämpötila on noin 17 astetta.")));
    // #105
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("17.47,0",
                                          "18.58,0",
                                          "18.03,0",  // inland day1: min, max, mean
                                          "17.47,0",
                                          "17.82,0",
                                          "17.64,0",  // coast day1: min, max, mean
                                          "16.89,0",
                                          "20.96,0",
                                          "18.93,0",  // area day1: min, max, mean
                                          "17.47,0",
                                          "18.45,0",
                                          "17.96,0",  // inland day1 morning: min, max, mean
                                          "17.47,0",
                                          "18.23,0",
                                          "17.85,0",  // coast day1 morning: min, max, mean
                                          "16.89,0",
                                          "17.21,0",
                                          "17.05,0",  // area day1 morning: min, max, mean
                                          "18.45,0",
                                          "18.58,0",
                                          "18.51,0",  // inland day1 afternoon: min, max, mean
                                          "17.73,0",
                                          "17.82,0",
                                          "17.78,0",  // coast day1 afternoon: min, max, mean
                                          "17.49,0",
                                          "20.96,0",
                                          "19.22,0",  // area day1 afternoon: min, max, mean
                                          "17.37,0",
                                          "19.35,0",
                                          "18.36,0",  // inland night: min, max, mean
                                          "13.80,0",
                                          "14.54,0",
                                          "14.17,0",  // coast night: min, max, mean
                                          "13.00,0",
                                          "14.80,0",
                                          "13.90,0",  // area night: min, max, mean
                                          "20.83,0",
                                          "25.09,0",
                                          "22.96,0",  // inland day2: min, max, mean
                                          "20.33,0",
                                          "24.67,0",
                                          "22.50,0",  // coast day2: min, max, mean
                                          "20.56,0",
                                          "26.36,0",
                                          "23.46,0",  // area day2: min, max, mean
                                          "21.04,0",
                                          "21.37,0",
                                          "21.20,0",  // inland day2 morning: min, max, mean
                                          "21.26,0",
                                          "22.86,0",
                                          "22.06,0",  // coast day2 morning: min, max, mean
                                          "20.90,0",
                                          "22.27,0",
                                          "21.58,0",  // area day2 morning: min, max, mean
                                          "23.82,0",
                                          "24.34,0",
                                          "24.08,0",  // inland day2 afternoon: min, max, mean
                                          "23.31,0",
                                          "24.19,0",
                                          "23.75,0",  // coast day2 afternoon: min, max, mean
                                          "23.05,0",
                                          "26.26,0",
                                          "24.65,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 17...21 astetta, huomenna 25 "
                                          "asteen tienoilla. Sisämaassa yön alin lämpötila on "
                                          "17...19 astetta, rannikolla 15 asteen tuntumassa.")));
    // #106
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("17.44,0",
                                "19.96,0",
                                "18.70,0",  // inland day1: min, max, mean
                                "15.85,0",
                                "21.81,0",
                                "18.83,0",  // coast day1: min, max, mean
                                "17.18,0",
                                "21.05,0",
                                "19.11,0",  // area day1: min, max, mean
                                "17.44,0",
                                "18.31,0",
                                "17.88,0",  // inland day1 morning: min, max, mean
                                "15.85,0",
                                "16.37,0",
                                "16.11,0",  // coast day1 morning: min, max, mean
                                "17.18,0",
                                "18.53,0",
                                "17.86,0",  // area day1 morning: min, max, mean
                                "19.64,0",
                                "19.96,0",
                                "19.80,0",  // inland day1 afternoon: min, max, mean
                                "18.22,0",
                                "21.81,0",
                                "20.01,0",  // coast day1 afternoon: min, max, mean
                                "18.74,0",
                                "21.05,0",
                                "19.89,0",  // area day1 afternoon: min, max, mean
                                "15.76,0",
                                "16.51,0",
                                "16.14,0",  // inland night: min, max, mean
                                "13.67,0",
                                "15.36,0",
                                "14.51,0",  // coast night: min, max, mean
                                "14.11,0",
                                "14.89,0",
                                "14.50,0",  // area night: min, max, mean
                                "16.91,0",
                                "20.54,0",
                                "18.73,0",  // inland day2: min, max, mean
                                "17.46,0",
                                "22.54,0",
                                "20.00,0",  // coast day2: min, max, mean
                                "16.19,0",
                                "20.48,0",
                                "18.33,0",  // area day2: min, max, mean
                                "17.37,0",
                                "18.25,0",
                                "17.81,0",  // inland day2 morning: min, max, mean
                                "17.74,0",
                                "18.55,0",
                                "18.15,0",  // coast day2 morning: min, max, mean
                                "16.60,0",
                                "18.29,0",
                                "17.45,0",  // area day2 morning: min, max, mean
                                "19.25,0",
                                "19.58,0",
                                "19.41,0",  // inland day2 afternoon: min, max, mean
                                "18.13,0",
                                "21.70,0",
                                "19.92,0",  // coast day2 afternoon: min, max, mean
                                "18.40,0",
                                "20.26,0",
                                "19.33,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 20 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on 15 asteen tuntumassa.")));
    // #107
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("19.03,0",
                                "22.24,0",
                                "20.63,0",  // inland day1: min, max, mean
                                "17.90,0",
                                "23.58,0",
                                "20.74,0",  // coast day1: min, max, mean
                                "18.78,0",
                                "25.94,0",
                                "22.36,0",  // area day1: min, max, mean
                                "19.03,0",
                                "19.48,0",
                                "19.25,0",  // inland day1 morning: min, max, mean
                                "17.90,0",
                                "18.77,0",
                                "18.33,0",  // coast day1 morning: min, max, mean
                                "18.78,0",
                                "20.04,0",
                                "19.41,0",  // area day1 morning: min, max, mean
                                "21.40,0",
                                "22.24,0",
                                "21.82,0",  // inland day1 afternoon: min, max, mean
                                "19.65,0",
                                "23.58,0",
                                "21.61,0",  // coast day1 afternoon: min, max, mean
                                "20.98,0",
                                "25.94,0",
                                "23.46,0",  // area day1 afternoon: min, max, mean
                                "14.72,0",
                                "15.46,0",
                                "15.09,0",  // inland night: min, max, mean
                                "15.20,0",
                                "17.11,0",
                                "16.15,0",  // coast night: min, max, mean
                                "16.33,0",
                                "17.33,0",
                                "16.83,0",  // area night: min, max, mean
                                "24.51,0",
                                "27.86,0",
                                "26.19,0",  // inland day2: min, max, mean
                                "25.84,0",
                                "27.43,0",
                                "26.64,0",  // coast day2: min, max, mean
                                "23.91,0",
                                "30.34,0",
                                "27.13,0",  // area day2: min, max, mean
                                "25.22,0",
                                "25.48,0",
                                "25.35,0",  // inland day2 morning: min, max, mean
                                "25.89,0",
                                "26.49,0",
                                "26.19,0",  // coast day2 morning: min, max, mean
                                "24.56,0",
                                "25.46,0",
                                "25.01,0",  // area day2 morning: min, max, mean
                                "27.21,0",
                                "27.55,0",
                                "27.38,0",  // inland day2 afternoon: min, max, mean
                                "26.99,0",
                                "27.04,0",
                                "27.02,0",  // coast day2 afternoon: min, max, mean
                                "26.47,0",
                                "29.42,0",
                                "27.94,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 21...26 astetta, huomenna 26...29 "
                                "astetta. Yön alin lämpötila on noin 17 astetta.")));
    // #108
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("20.00,0",
                                "22.32,0",
                                "21.16,0",  // inland day1: min, max, mean
                                "19.95,0",
                                "21.69,0",
                                "20.82,0",  // coast day1: min, max, mean
                                "19.22,0",
                                "23.05,0",
                                "21.13,0",  // area day1: min, max, mean
                                "20.00,0",
                                "20.88,0",
                                "20.44,0",  // inland day1 morning: min, max, mean
                                "19.95,0",
                                "21.42,0",
                                "20.69,0",  // coast day1 morning: min, max, mean
                                "19.22,0",
                                "20.93,0",
                                "20.07,0",  // area day1 morning: min, max, mean
                                "22.13,0",
                                "22.32,0",
                                "22.22,0",  // inland day1 afternoon: min, max, mean
                                "20.24,0",
                                "21.69,0",
                                "20.96,0",  // coast day1 afternoon: min, max, mean
                                "21.58,0",
                                "23.05,0",
                                "22.31,0",  // area day1 afternoon: min, max, mean
                                "16.59,0",
                                "17.59,0",
                                "17.09,0",  // inland night: min, max, mean
                                "19.84,0",
                                "20.21,0",
                                "20.03,0",  // coast night: min, max, mean
                                "15.31,0",
                                "16.37,0",
                                "15.84,0",  // area night: min, max, mean
                                "17.56,0",
                                "21.38,0",
                                "19.47,0",  // inland day2: min, max, mean
                                "17.80,0",
                                "22.36,0",
                                "20.08,0",  // coast day2: min, max, mean
                                "17.07,0",
                                "21.00,0",
                                "19.03,0",  // area day2: min, max, mean
                                "18.07,0",
                                "18.81,0",
                                "18.44,0",  // inland day2 morning: min, max, mean
                                "18.52,0",
                                "20.35,0",
                                "19.43,0",  // coast day2 morning: min, max, mean
                                "18.00,0",
                                "19.30,0",
                                "18.65,0",  // area day2 morning: min, max, mean
                                "20.51,0",
                                "21.28,0",
                                "20.89,0",  // inland day2 afternoon: min, max, mean
                                "19.97,0",
                                "21.73,0",
                                "20.85,0",  // coast day2 afternoon: min, max, mean
                                "19.90,0",
                                "20.52,0",
                                "20.21,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 22 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on noin 15 astetta.")));
    // #109
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("20.18,0",
                                          "22.39,0",
                                          "21.28,0",  // inland day1: min, max, mean
                                          "18.85,0",
                                          "25.28,0",
                                          "22.07,0",  // coast day1: min, max, mean
                                          "19.59,0",
                                          "23.91,0",
                                          "21.75,0",  // area day1: min, max, mean
                                          "20.18,0",
                                          "21.09,0",
                                          "20.63,0",  // inland day1 morning: min, max, mean
                                          "18.85,0",
                                          "19.66,0",
                                          "19.25,0",  // coast day1 morning: min, max, mean
                                          "19.59,0",
                                          "21.34,0",
                                          "20.46,0",  // area day1 morning: min, max, mean
                                          "21.91,0",
                                          "22.39,0",
                                          "22.15,0",  // inland day1 afternoon: min, max, mean
                                          "20.73,0",
                                          "25.28,0",
                                          "23.00,0",  // coast day1 afternoon: min, max, mean
                                          "21.50,0",
                                          "23.91,0",
                                          "22.70,0",  // area day1 afternoon: min, max, mean
                                          "19.59,0",
                                          "20.69,0",
                                          "20.14,0",  // inland night: min, max, mean
                                          "15.83,0",
                                          "16.92,0",
                                          "16.38,0",  // coast night: min, max, mean
                                          "18.63,0",
                                          "20.44,0",
                                          "19.54,0",  // area night: min, max, mean
                                          "14.99,0",
                                          "18.56,0",
                                          "16.77,0",  // inland day2: min, max, mean
                                          "16.13,0",
                                          "20.59,0",
                                          "18.36,0",  // coast day2: min, max, mean
                                          "14.33,0",
                                          "23.09,0",
                                          "18.71,0",  // area day2: min, max, mean
                                          "15.78,0",
                                          "16.68,0",
                                          "16.23,0",  // inland day2 morning: min, max, mean
                                          "16.40,0",
                                          "16.78,0",
                                          "16.59,0",  // coast day2 morning: min, max, mean
                                          "15.29,0",
                                          "16.58,0",
                                          "15.94,0",  // area day2 morning: min, max, mean
                                          "17.52,0",
                                          "18.39,0",
                                          "17.95,0",  // inland day2 afternoon: min, max, mean
                                          "16.17,0",
                                          "19.67,0",
                                          "17.92,0",  // coast day2 afternoon: min, max, mean
                                          "17.50,0",
                                          "22.43,0",
                                          "19.97,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 22...24 astetta, huomenna "
                                          "suunnilleen sama. Sisämaassa yön alin lämpötila on noin "
                                          "20 astetta, rannikolla noin 15 astetta.")));
    // #110
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("20.05,0",
                                          "23.22,0",
                                          "21.64,0",  // inland day1: min, max, mean
                                          "19.63,0",
                                          "24.40,0",
                                          "22.02,0",  // coast day1: min, max, mean
                                          "19.53,0",
                                          "25.91,0",
                                          "22.72,0",  // area day1: min, max, mean
                                          "20.05,0",
                                          "20.86,0",
                                          "20.45,0",  // inland day1 morning: min, max, mean
                                          "19.63,0",
                                          "19.94,0",
                                          "19.79,0",  // coast day1 morning: min, max, mean
                                          "19.53,0",
                                          "20.89,0",
                                          "20.21,0",  // area day1 morning: min, max, mean
                                          "22.46,0",
                                          "23.22,0",
                                          "22.84,0",  // inland day1 afternoon: min, max, mean
                                          "20.82,0",
                                          "24.40,0",
                                          "22.61,0",  // coast day1 afternoon: min, max, mean
                                          "21.69,0",
                                          "25.91,0",
                                          "23.80,0",  // area day1 afternoon: min, max, mean
                                          "19.34,0",
                                          "21.27,0",
                                          "20.30,0",  // inland night: min, max, mean
                                          "16.86,0",
                                          "17.68,0",
                                          "17.27,0",  // coast night: min, max, mean
                                          "17.31,0",
                                          "18.58,0",
                                          "17.95,0",  // area night: min, max, mean
                                          "23.26,0",
                                          "25.16,0",
                                          "24.21,0",  // inland day2: min, max, mean
                                          "23.91,0",
                                          "27.56,0",
                                          "25.74,0",  // coast day2: min, max, mean
                                          "21.94,0",
                                          "29.56,0",
                                          "25.75,0",  // area day2: min, max, mean
                                          "23.56,0",
                                          "23.95,0",
                                          "23.75,0",  // inland day2 morning: min, max, mean
                                          "24.24,0",
                                          "25.18,0",
                                          "24.71,0",  // coast day2 morning: min, max, mean
                                          "22.69,0",
                                          "22.77,0",
                                          "22.73,0",  // area day2 morning: min, max, mean
                                          "24.90,0",
                                          "24.95,0",
                                          "24.93,0",  // inland day2 afternoon: min, max, mean
                                          "24.10,0",
                                          "27.20,0",
                                          "25.65,0",  // coast day2 afternoon: min, max, mean
                                          "24.87,0",
                                          "29.36,0",
                                          "27.11,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 22...26 astetta, huomenna "
                                          "hieman korkeampi. Sisämaassa yön alin lämpötila on noin "
                                          "20 astetta, rannikolla noin 17 astetta.")));
    // #111
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("20.01,0",
                                "22.22,0",
                                "21.11,0",  // inland day1: min, max, mean
                                "19.52,0",
                                "23.01,0",
                                "21.27,0",  // coast day1: min, max, mean
                                "19.91,0",
                                "22.24,0",
                                "21.08,0",  // area day1: min, max, mean
                                "20.01,0",
                                "20.28,0",
                                "20.15,0",  // inland day1 morning: min, max, mean
                                "19.52,0",
                                "19.91,0",
                                "19.71,0",  // coast day1 morning: min, max, mean
                                "19.91,0",
                                "21.34,0",
                                "20.62,0",  // area day1 morning: min, max, mean
                                "21.81,0",
                                "22.22,0",
                                "22.01,0",  // inland day1 afternoon: min, max, mean
                                "21.70,0",
                                "23.01,0",
                                "22.35,0",  // coast day1 afternoon: min, max, mean
                                "21.75,0",
                                "22.24,0",
                                "22.00,0",  // area day1 afternoon: min, max, mean
                                "15.74,0",
                                "16.18,0",
                                "15.96,0",  // inland night: min, max, mean
                                "18.12,0",
                                "18.23,0",
                                "18.18,0",  // coast night: min, max, mean
                                "17.13,0",
                                "18.31,0",
                                "17.72,0",  // area night: min, max, mean
                                "19.14,0",
                                "24.68,0",
                                "21.91,0",  // inland day2: min, max, mean
                                "19.40,0",
                                "24.11,0",
                                "21.75,0",  // coast day2: min, max, mean
                                "18.59,0",
                                "26.54,0",
                                "22.56,0",  // area day2: min, max, mean
                                "19.59,0",
                                "19.69,0",
                                "19.64,0",  // inland day2 morning: min, max, mean
                                "20.18,0",
                                "20.34,0",
                                "20.26,0",  // coast day2 morning: min, max, mean
                                "19.03,0",
                                "19.64,0",
                                "19.34,0",  // area day2 morning: min, max, mean
                                "22.92,0",
                                "23.70,0",
                                "23.31,0",  // inland day2 afternoon: min, max, mean
                                "22.91,0",
                                "23.27,0",
                                "23.09,0",  // coast day2 afternoon: min, max, mean
                                "22.06,0",
                                "26.35,0",
                                "24.20,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 22 astetta, huomenna hieman "
                                "korkeampi. Yön alin lämpötila on noin 18 astetta.")));
    // #112
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("21.97,0",
                                "24.44,0",
                                "23.21,0",  // inland day1: min, max, mean
                                "20.21,0",
                                "27.99,0",
                                "24.10,0",  // coast day1: min, max, mean
                                "21.31,0",
                                "25.90,0",
                                "23.60,0",  // area day1: min, max, mean
                                "21.97,0",
                                "22.49,0",
                                "22.23,0",  // inland day1 morning: min, max, mean
                                "20.21,0",
                                "21.77,0",
                                "20.99,0",  // coast day1 morning: min, max, mean
                                "21.31,0",
                                "22.98,0",
                                "22.14,0",  // area day1 morning: min, max, mean
                                "24.27,0",
                                "24.44,0",
                                "24.36,0",  // inland day1 afternoon: min, max, mean
                                "24.13,0",
                                "27.99,0",
                                "26.06,0",  // coast day1 afternoon: min, max, mean
                                "23.91,0",
                                "25.90,0",
                                "24.90,0",  // area day1 afternoon: min, max, mean
                                "19.68,0",
                                "20.19,0",
                                "19.94,0",  // inland night: min, max, mean
                                "19.88,0",
                                "21.58,0",
                                "20.73,0",  // coast night: min, max, mean
                                "21.09,0",
                                "22.31,0",
                                "21.70,0",  // area night: min, max, mean
                                "22.35,0",
                                "26.50,0",
                                "24.43,0",  // inland day2: min, max, mean
                                "23.01,0",
                                "29.32,0",
                                "26.16,0",  // coast day2: min, max, mean
                                "21.93,0",
                                "26.09,0",
                                "24.01,0",  // area day2: min, max, mean
                                "22.72,0",
                                "23.00,0",
                                "22.86,0",  // inland day2 morning: min, max, mean
                                "23.62,0",
                                "25.21,0",
                                "24.42,0",  // coast day2 morning: min, max, mean
                                "22.58,0",
                                "24.46,0",
                                "23.52,0",  // area day2 morning: min, max, mean
                                "26.00,0",
                                "26.16,0",
                                "26.08,0",  // inland day2 afternoon: min, max, mean
                                "25.35,0",
                                "28.93,0",
                                "27.14,0",  // coast day2 afternoon: min, max, mean
                                "25.48,0",
                                "25.60,0",
                                "25.54,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on lähellä 25 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on noin 22 astetta.")));
    // #113
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("22.90,0",
                                "24.67,0",
                                "23.78,0",  // inland day1: min, max, mean
                                "21.66,0",
                                "23.77,0",
                                "22.71,0",  // coast day1: min, max, mean
                                "21.94,0",
                                "23.76,0",
                                "22.85,0",  // area day1: min, max, mean
                                "22.90,0",
                                "23.69,0",
                                "23.29,0",  // inland day1 morning: min, max, mean
                                "21.66,0",
                                "23.39,0",
                                "22.52,0",  // coast day1 morning: min, max, mean
                                "21.94,0",
                                "22.18,0",
                                "22.06,0",  // area day1 morning: min, max, mean
                                "23.73,0",
                                "24.67,0",
                                "24.20,0",  // inland day1 afternoon: min, max, mean
                                "21.85,0",
                                "23.77,0",
                                "22.81,0",  // coast day1 afternoon: min, max, mean
                                "23.01,0",
                                "23.76,0",
                                "23.39,0",  // area day1 afternoon: min, max, mean
                                "17.99,0",
                                "18.72,0",
                                "18.36,0",  // inland night: min, max, mean
                                "19.95,0",
                                "20.82,0",
                                "20.39,0",  // coast night: min, max, mean
                                "18.97,0",
                                "19.20,0",
                                "19.08,0",  // area night: min, max, mean
                                "26.34,0",
                                "30.70,0",
                                "28.52,0",  // inland day2: min, max, mean
                                "27.65,0",
                                "33.18,0",
                                "30.42,0",  // coast day2: min, max, mean
                                "26.54,0",
                                "32.98,0",
                                "29.76,0",  // area day2: min, max, mean
                                "27.33,0",
                                "28.27,0",
                                "27.80,0",  // inland day2 morning: min, max, mean
                                "27.96,0",
                                "28.47,0",
                                "28.22,0",  // coast day2 morning: min, max, mean
                                "26.94,0",
                                "27.82,0",
                                "27.38,0",  // area day2 morning: min, max, mean
                                "29.95,0",
                                "30.08,0",
                                "30.02,0",  // inland day2 afternoon: min, max, mean
                                "29.81,0",
                                "32.89,0",
                                "31.35,0",  // coast day2 afternoon: min, max, mean
                                "29.52,0",
                                "32.23,0",
                                "30.88,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on vajaat 25 astetta, huomenna 30...32 "
                                "astetta. Yön alin lämpötila on 20 asteen tuntumassa.")));
    // #114
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("22.17,0",
                                "24.69,0",
                                "23.43,0",  // inland day1: min, max, mean
                                "22.13,0",
                                "25.76,0",
                                "23.94,0",  // coast day1: min, max, mean
                                "22.07,0",
                                "27.31,0",
                                "24.69,0",  // area day1: min, max, mean
                                "22.17,0",
                                "22.45,0",
                                "22.31,0",  // inland day1 morning: min, max, mean
                                "22.13,0",
                                "22.50,0",
                                "22.31,0",  // coast day1 morning: min, max, mean
                                "22.07,0",
                                "22.69,0",
                                "22.38,0",  // area day1 morning: min, max, mean
                                "24.22,0",
                                "24.69,0",
                                "24.46,0",  // inland day1 afternoon: min, max, mean
                                "24.12,0",
                                "25.76,0",
                                "24.94,0",  // coast day1 afternoon: min, max, mean
                                "23.24,0",
                                "27.31,0",
                                "25.27,0",  // area day1 afternoon: min, max, mean
                                "21.50,0",
                                "22.74,0",
                                "22.12,0",  // inland night: min, max, mean
                                "19.43,0",
                                "19.57,0",
                                "19.50,0",  // coast night: min, max, mean
                                "21.11,0",
                                "22.95,0",
                                "22.03,0",  // area night: min, max, mean
                                "18.54,0",
                                "23.15,0",
                                "20.84,0",  // inland day2: min, max, mean
                                "18.94,0",
                                "21.55,0",
                                "20.25,0",  // coast day2: min, max, mean
                                "17.92,0",
                                "25.41,0",
                                "21.66,0",  // area day2: min, max, mean
                                "18.95,0",
                                "19.47,0",
                                "19.21,0",  // inland day2 morning: min, max, mean
                                "19.44,0",
                                "21.00,0",
                                "20.22,0",  // coast day2 morning: min, max, mean
                                "18.10,0",
                                "18.49,0",
                                "18.29,0",  // area day2 morning: min, max, mean
                                "21.27,0",
                                "22.26,0",
                                "21.76,0",  // inland day2 afternoon: min, max, mean
                                "19.71,0",
                                "21.20,0",
                                "20.46,0",  // coast day2 afternoon: min, max, mean
                                "21.26,0",
                                "24.74,0",
                                "23.00,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 23...27 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on 21...23 astetta.")));
    // #115
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("23.67,0",
                                "25.59,0",
                                "24.63,0",  // inland day1: min, max, mean
                                "23.25,0",
                                "26.21,0",
                                "24.73,0",  // coast day1: min, max, mean
                                "22.77,0",
                                "28.62,0",
                                "25.70,0",  // area day1: min, max, mean
                                "23.67,0",
                                "24.31,0",
                                "23.99,0",  // inland day1 morning: min, max, mean
                                "23.25,0",
                                "23.64,0",
                                "23.45,0",  // coast day1 morning: min, max, mean
                                "22.77,0",
                                "23.49,0",
                                "23.13,0",  // area day1 morning: min, max, mean
                                "24.67,0",
                                "25.59,0",
                                "25.13,0",  // inland day1 afternoon: min, max, mean
                                "24.48,0",
                                "26.21,0",
                                "25.34,0",  // coast day1 afternoon: min, max, mean
                                "24.46,0",
                                "28.62,0",
                                "26.54,0",  // area day1 afternoon: min, max, mean
                                "20.87,0",
                                "22.23,0",
                                "21.55,0",  // inland night: min, max, mean
                                "21.53,0",
                                "22.93,0",
                                "22.23,0",  // coast night: min, max, mean
                                "21.18,0",
                                "22.17,0",
                                "21.67,0",  // area night: min, max, mean
                                "19.98,0",
                                "25.22,0",
                                "22.60,0",  // inland day2: min, max, mean
                                "20.24,0",
                                "24.54,0",
                                "22.39,0",  // coast day2: min, max, mean
                                "19.10,0",
                                "25.12,0",
                                "22.11,0",  // area day2: min, max, mean
                                "20.87,0",
                                "21.07,0",
                                "20.97,0",  // inland day2 morning: min, max, mean
                                "20.92,0",
                                "22.02,0",
                                "21.47,0",  // coast day2 morning: min, max, mean
                                "20.00,0",
                                "20.73,0",
                                "20.36,0",  // area day2 morning: min, max, mean
                                "24.03,0",
                                "24.38,0",
                                "24.21,0",  // inland day2 afternoon: min, max, mean
                                "23.18,0",
                                "23.90,0",
                                "23.54,0",  // coast day2 afternoon: min, max, mean
                                "23.20,0",
                                "24.72,0",
                                "23.96,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 24...29 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on noin 22 astetta.")));
    // #116
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("24.30,0",
                                "26.47,0",
                                "25.39,0",  // inland day1: min, max, mean
                                "23.55,0",
                                "27.15,0",
                                "25.35,0",  // coast day1: min, max, mean
                                "23.75,0",
                                "27.81,0",
                                "25.78,0",  // area day1: min, max, mean
                                "24.30,0",
                                "25.12,0",
                                "24.71,0",  // inland day1 morning: min, max, mean
                                "23.55,0",
                                "24.59,0",
                                "24.07,0",  // coast day1 morning: min, max, mean
                                "23.75,0",
                                "24.45,0",
                                "24.10,0",  // area day1 morning: min, max, mean
                                "26.05,0",
                                "26.47,0",
                                "26.26,0",  // inland day1 afternoon: min, max, mean
                                "24.95,0",
                                "27.15,0",
                                "26.05,0",  // coast day1 afternoon: min, max, mean
                                "25.30,0",
                                "27.81,0",
                                "26.55,0",  // area day1 afternoon: min, max, mean
                                "21.37,0",
                                "23.28,0",
                                "22.32,0",  // inland night: min, max, mean
                                "20.56,0",
                                "22.24,0",
                                "21.40,0",  // coast night: min, max, mean
                                "22.37,0",
                                "22.61,0",
                                "22.49,0",  // area night: min, max, mean
                                "28.63,0",
                                "31.71,0",
                                "30.17,0",  // inland day2: min, max, mean
                                "29.12,0",
                                "30.93,0",
                                "30.02,0",  // coast day2: min, max, mean
                                "28.02,0",
                                "35.09,0",
                                "31.55,0",  // area day2: min, max, mean
                                "28.96,0",
                                "29.25,0",
                                "29.10,0",  // inland day2 morning: min, max, mean
                                "29.60,0",
                                "30.98,0",
                                "30.29,0",  // coast day2 morning: min, max, mean
                                "28.52,0",
                                "29.31,0",
                                "28.91,0",  // area day2 morning: min, max, mean
                                "30.42,0",
                                "30.87,0",
                                "30.65,0",  // inland day2 afternoon: min, max, mean
                                "28.70,0",
                                "30.41,0",
                                "29.56,0",  // coast day2 afternoon: min, max, mean
                                "30.40,0",
                                "34.20,0",
                                "32.30,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 25...28 astetta, huomenna 30...34 "
                                "astetta. Yön alin lämpötila on noin 22 astetta.")));
    // #117
    testCases.insert(
        make_pair(i++,
                  new Max36HoursTestParam("25.00,0",
                                          "25.89,0",
                                          "25.44,0",  // inland day1: min, max, mean
                                          "23.14,0",
                                          "27.40,0",
                                          "25.27,0",  // coast day1: min, max, mean
                                          "24.42,0",
                                          "25.11,0",
                                          "24.77,0",  // area day1: min, max, mean
                                          "25.00,0",
                                          "25.32,0",
                                          "25.16,0",  // inland day1 morning: min, max, mean
                                          "23.14,0",
                                          "23.44,0",
                                          "23.29,0",  // coast day1 morning: min, max, mean
                                          "24.42,0",
                                          "24.47,0",
                                          "24.44,0",  // area day1 morning: min, max, mean
                                          "25.26,0",
                                          "25.89,0",
                                          "25.57,0",  // inland day1 afternoon: min, max, mean
                                          "24.26,0",
                                          "27.40,0",
                                          "25.83,0",  // coast day1 afternoon: min, max, mean
                                          "24.71,0",
                                          "25.11,0",
                                          "24.91,0",  // area day1 afternoon: min, max, mean
                                          "24.52,0",
                                          "25.49,0",
                                          "25.00,0",  // inland night: min, max, mean
                                          "19.72,0",
                                          "20.97,0",
                                          "20.35,0",  // coast night: min, max, mean
                                          "21.29,0",
                                          "21.31,0",
                                          "21.30,0",  // area night: min, max, mean
                                          "28.99,0",
                                          "31.58,0",
                                          "30.29,0",  // inland day2: min, max, mean
                                          "30.29,0",
                                          "30.43,0",
                                          "30.36,0",  // coast day2: min, max, mean
                                          "28.53,0",
                                          "32.99,0",
                                          "30.76,0",  // area day2: min, max, mean
                                          "29.88,0",
                                          "30.37,0",
                                          "30.12,0",  // inland day2 morning: min, max, mean
                                          "30.43,0",
                                          "30.56,0",
                                          "30.49,0",  // coast day2 morning: min, max, mean
                                          "28.94,0",
                                          "29.36,0",
                                          "29.15,0",  // area day2 morning: min, max, mean
                                          "31.38,0",
                                          "31.40,0",
                                          "31.39,0",  // inland day2 afternoon: min, max, mean
                                          "30.17,0",
                                          "30.33,0",
                                          "30.25,0",  // coast day2 afternoon: min, max, mean
                                          "30.89,0",
                                          "32.95,0",
                                          "31.92,0",  // area day2 afternoon: min, max, mean
                                          "Päivän ylin lämpötila on 25 asteen tuntumassa, huomenna "
                                          "31...33 astetta. Sisämaassa yön alin lämpötila on noin "
                                          "25 astetta, rannikolla noin 20 astetta.")));
    // #118
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("25.42,0",
                                "26.47,0",
                                "25.95,0",  // inland day1: min, max, mean
                                "24.05,0",
                                "25.42,0",
                                "24.73,0",  // coast day1: min, max, mean
                                "24.54,0",
                                "27.73,0",
                                "26.13,0",  // area day1: min, max, mean
                                "25.42,0",
                                "25.97,0",
                                "25.70,0",  // inland day1 morning: min, max, mean
                                "24.05,0",
                                "24.63,0",
                                "24.34,0",  // coast day1 morning: min, max, mean
                                "24.54,0",
                                "25.12,0",
                                "24.83,0",  // area day1 morning: min, max, mean
                                "25.75,0",
                                "26.47,0",
                                "26.11,0",  // inland day1 afternoon: min, max, mean
                                "24.61,0",
                                "25.42,0",
                                "25.01,0",  // coast day1 afternoon: min, max, mean
                                "25.62,0",
                                "27.73,0",
                                "26.68,0",  // area day1 afternoon: min, max, mean
                                "23.02,0",
                                "24.36,0",
                                "23.69,0",  // inland night: min, max, mean
                                "23.29,0",
                                "24.21,0",
                                "23.75,0",  // coast night: min, max, mean
                                "23.18,0",
                                "23.63,0",
                                "23.41,0",  // area night: min, max, mean
                                "28.95,0",
                                "34.15,0",
                                "31.55,0",  // inland day2: min, max, mean
                                "30.41,0",
                                "37.49,0",
                                "33.95,0",  // coast day2: min, max, mean
                                "28.96,0",
                                "38.83,0",
                                "33.89,0",  // area day2: min, max, mean
                                "29.90,0",
                                "29.92,0",
                                "29.91,0",  // inland day2 morning: min, max, mean
                                "30.84,0",
                                "31.08,0",
                                "30.96,0",  // coast day2 morning: min, max, mean
                                "29.62,0",
                                "31.15,0",
                                "30.38,0",  // area day2 morning: min, max, mean
                                "33.24,0",
                                "34.11,0",
                                "33.67,0",  // inland day2 afternoon: min, max, mean
                                "33.07,0",
                                "36.62,0",
                                "34.84,0",  // coast day2 afternoon: min, max, mean
                                "33.02,0",
                                "37.99,0",
                                "35.51,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 26...28 astetta, huomenna 33...38 "
                                "astetta. Yön alin lämpötila on vajaat 25 astetta.")));
    // #119
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("25.65,0",
                                "27.82,0",
                                "26.74,0",  // inland day1: min, max, mean
                                "24.44,0",
                                "26.35,0",
                                "25.40,0",  // coast day1: min, max, mean
                                "25.46,0",
                                "28.83,0",
                                "27.15,0",  // area day1: min, max, mean
                                "25.65,0",
                                "26.61,0",
                                "26.13,0",  // inland day1 morning: min, max, mean
                                "24.44,0",
                                "25.92,0",
                                "25.18,0",  // coast day1 morning: min, max, mean
                                "25.46,0",
                                "27.03,0",
                                "26.25,0",  // area day1 morning: min, max, mean
                                "27.57,0",
                                "27.82,0",
                                "27.70,0",  // inland day1 afternoon: min, max, mean
                                "25.61,0",
                                "26.35,0",
                                "25.98,0",  // coast day1 afternoon: min, max, mean
                                "27.48,0",
                                "28.83,0",
                                "28.15,0",  // area day1 afternoon: min, max, mean
                                "22.21,0",
                                "22.56,0",
                                "22.38,0",  // inland night: min, max, mean
                                "24.31,0",
                                "24.59,0",
                                "24.45,0",  // coast night: min, max, mean
                                "21.93,0",
                                "22.51,0",
                                "22.22,0",  // area night: min, max, mean
                                "24.78,0",
                                "29.68,0",
                                "27.23,0",  // inland day2: min, max, mean
                                "25.09,0",
                                "30.10,0",
                                "27.60,0",  // coast day2: min, max, mean
                                "24.71,0",
                                "31.41,0",
                                "28.06,0",  // area day2: min, max, mean
                                "25.65,0",
                                "25.81,0",
                                "25.73,0",  // inland day2 morning: min, max, mean
                                "25.99,0",
                                "27.55,0",
                                "26.77,0",  // coast day2 morning: min, max, mean
                                "25.57,0",
                                "26.16,0",
                                "25.86,0",  // area day2 morning: min, max, mean
                                "28.69,0",
                                "28.93,0",
                                "28.81,0",  // inland day2 afternoon: min, max, mean
                                "27.53,0",
                                "29.21,0",
                                "28.37,0",  // coast day2 afternoon: min, max, mean
                                "28.25,0",
                                "31.18,0",
                                "29.72,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 27...29 astetta, huomenna hieman "
                                "korkeampi. Yön alin lämpötila on noin 22 astetta.")));
    // #120
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("25.72,0",
                                "28.09,0",
                                "26.90,0",  // inland day1: min, max, mean
                                "24.89,0",
                                "29.43,0",
                                "27.16,0",  // coast day1: min, max, mean
                                "25.05,0",
                                "30.07,0",
                                "27.56,0",  // area day1: min, max, mean
                                "25.72,0",
                                "26.34,0",
                                "26.03,0",  // inland day1 morning: min, max, mean
                                "24.89,0",
                                "26.00,0",
                                "25.45,0",  // coast day1 morning: min, max, mean
                                "25.05,0",
                                "26.47,0",
                                "25.76,0",  // area day1 morning: min, max, mean
                                "27.27,0",
                                "28.09,0",
                                "27.68,0",  // inland day1 afternoon: min, max, mean
                                "26.63,0",
                                "29.43,0",
                                "28.03,0",  // coast day1 afternoon: min, max, mean
                                "26.87,0",
                                "30.07,0",
                                "28.47,0",  // area day1 afternoon: min, max, mean
                                "21.32,0",
                                "22.63,0",
                                "21.98,0",  // inland night: min, max, mean
                                "22.13,0",
                                "22.76,0",
                                "22.45,0",  // coast night: min, max, mean
                                "21.49,0",
                                "22.89,0",
                                "22.19,0",  // area night: min, max, mean
                                "23.83,0",
                                "26.45,0",
                                "25.14,0",  // inland day2: min, max, mean
                                "23.36,0",
                                "25.81,0",
                                "24.58,0",  // coast day2: min, max, mean
                                "22.30,0",
                                "30.94,0",
                                "26.62,0",  // area day2: min, max, mean
                                "23.93,0",
                                "24.68,0",
                                "24.31,0",  // inland day2 morning: min, max, mean
                                "24.00,0",
                                "25.41,0",
                                "24.70,0",  // coast day2 morning: min, max, mean
                                "23.10,0",
                                "25.00,0",
                                "24.05,0",  // area day2 morning: min, max, mean
                                "25.95,0",
                                "26.02,0",
                                "25.98,0",  // inland day2 afternoon: min, max, mean
                                "24.11,0",
                                "25.71,0",
                                "24.91,0",  // coast day2 afternoon: min, max, mean
                                "25.82,0",
                                "30.58,0",
                                "28.20,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on 27...30 astetta, huomenna suunnilleen "
                                "sama. Yön alin lämpötila on 21...23 astetta.")));
    // #121
    testCases.insert(make_pair(
        i++,
        new Max36HoursTestParam("25.83,0",
                                "28.61,0",
                                "27.22,0",  // inland day1: min, max, mean
                                "24.60,0",
                                "31.90,0",
                                "28.25,0",  // coast day1: min, max, mean
                                "25.35,0",
                                "27.98,0",
                                "26.67,0",  // area day1: min, max, mean
                                "25.83,0",
                                "26.73,0",
                                "26.28,0",  // inland day1 morning: min, max, mean
                                "24.60,0",
                                "25.80,0",
                                "25.20,0",  // coast day1 morning: min, max, mean
                                "25.35,0",
                                "27.33,0",
                                "26.34,0",  // area day1 morning: min, max, mean
                                "28.42,0",
                                "28.61,0",
                                "28.52,0",  // inland day1 afternoon: min, max, mean
                                "26.90,0",
                                "31.90,0",
                                "29.40,0",  // coast day1 afternoon: min, max, mean
                                "27.97,0",
                                "27.98,0",
                                "27.97,0",  // area day1 afternoon: min, max, mean
                                "21.60,0",
                                "23.12,0",
                                "22.36,0",  // inland night: min, max, mean
                                "23.50,0",
                                "24.36,0",
                                "23.93,0",  // coast night: min, max, mean
                                "25.04,0",
                                "25.61,0",
                                "25.32,0",  // area night: min, max, mean
                                "23.18,0",
                                "29.18,0",
                                "26.18,0",  // inland day2: min, max, mean
                                "23.96,0",
                                "32.23,0",
                                "28.09,0",  // coast day2: min, max, mean
                                "23.64,0",
                                "33.04,0",
                                "28.34,0",  // area day2: min, max, mean
                                "24.07,0",
                                "24.33,0",
                                "24.20,0",  // inland day2 morning: min, max, mean
                                "24.84,0",
                                "24.90,0",
                                "24.87,0",  // coast day2 morning: min, max, mean
                                "23.98,0",
                                "25.54,0",
                                "24.76,0",  // area day2 morning: min, max, mean
                                "28.71,0",
                                "29.11,0",
                                "28.91,0",  // inland day2 afternoon: min, max, mean
                                "27.63,0",
                                "31.42,0",
                                "29.53,0",  // coast day2 afternoon: min, max, mean
                                "28.37,0",
                                "32.92,0",
                                "30.65,0",  // area day2 afternoon: min, max, mean
                                "Päivän ylin lämpötila on noin 28 astetta, huomenna hieman "
                                "korkeampi. Yön alin lämpötila on noin 25 astetta.")));
  }
}

void create_anomaly_testcases(TestCaseContainer& testCases,
                              const string& language,
                              const bool& isWinter)
{
  int i = 0;

  if (isWinter)
  {
    if (language == "fi")
    {
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("-29.55,0",
                                          "-28.67,0",
                                          "-29.11,0",  // inland day1 afternoon: min, max, mean
                                          "-30.30,0",
                                          "-29.86,0",
                                          "-30.08,0",  // coast day1 afternoon: min, max, mean
                                          "-30.43,0",
                                          "-28.27,0",
                                          "-29.35,0",  // area day1 afternoon: min, max, mean
                                          "-31.60,0",
                                          "-31.21,0",
                                          "-31.40,0",  // inland day2 afternoon: min, max, mean
                                          "-32.80,0",
                                          "-28.91,0",
                                          "-30.85,0",  // coast day2 afternoon: min, max, mean
                                          "-31.83,0",
                                          "-29.28,0",
                                          "-30.55,0",  // area day2 afternoon: min, max, mean
                                          "Huomenna sää on poikkeuksellisen kylmää."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("-30.77,0",
                                          "-30.63,0",
                                          "-30.70,0",  // inland day1 afternoon: min, max, mean
                                          "-32.22,0",
                                          "-29.35,0",
                                          "-30.79,0",  // coast day1 afternoon: min, max, mean
                                          "-30.82,0",
                                          "-30.38,0",
                                          "-30.60,0",  // area day1 afternoon: min, max, mean
                                          "-34.43,0",
                                          "-33.69,0",
                                          "-34.06,0",  // inland day2 afternoon: min, max, mean
                                          "-34.82,0",
                                          "-34.77,0",
                                          "-34.79,0",  // coast day2 afternoon: min, max, mean
                                          "-35.28,0",
                                          "-31.47,0",
                                          "-33.37,0",  // area day2 afternoon: min, max, mean
                                          "Sää on poikkeuksellisen kylmää."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("-30.92,0",
                                          "-30.35,0",
                                          "-30.64,0",  // inland day1 afternoon: min, max, mean
                                          "-31.89,0",
                                          "-31.34,0",
                                          "-31.61,0",  // coast day1 afternoon: min, max, mean
                                          "-31.15,0",
                                          "-30.82,0",
                                          "-30.98,0",  // area day1 afternoon: min, max, mean
                                          "-21.96,0",
                                          "-21.00,0",
                                          "-21.48,0",  // inland day2 afternoon: min, max, mean
                                          "-22.73,0",
                                          "-20.53,0",
                                          "-21.63,0",  // coast day2 afternoon: min, max, mean
                                          "-22.78,0",
                                          "-22.02,0",
                                          "-22.40,0",  // area day2 afternoon: min, max, mean
                                          "Kireä pakkanen heikkenee."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("-23.87,0",
                                          "-23.49,0",
                                          "-23.68,0",  // inland day1 afternoon: min, max, mean
                                          "-24.20,0",
                                          "-19.31,0",
                                          "-21.75,0",  // coast day1 afternoon: min, max, mean
                                          "-24.06,0",
                                          "-22.72,0",
                                          "-23.39,0",  // area day1 afternoon: min, max, mean
                                          "-25.70,0",
                                          "-25.39,0",
                                          "-25.55,0",  // inland day2 afternoon: min, max, mean
                                          "-27.42,0",
                                          "-24.99,0",
                                          "-26.21,0",  // coast day2 afternoon: min, max, mean
                                          "-26.22,0",
                                          "-24.83,0",
                                          "-25.52,0",       // area day2 afternoon: min, max, mean
                                          "Sää on kylmää."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("-15.98,0",
                                          "-15.31,0",
                                          "-15.65,0",  // inland day1 afternoon: min, max, mean
                                          "-17.92,0",
                                          "-16.06,0",
                                          "-16.99,0",  // coast day1 afternoon: min, max, mean
                                          "-16.91,0",
                                          "-13.15,0",
                                          "-15.03,0",  // area day1 afternoon: min, max, mean
                                          "-10.12,0",
                                          "-10.05,0",
                                          "-10.09,0",  // inland day2 afternoon: min, max, mean
                                          "-11.38,0",
                                          "-10.85,0",
                                          "-11.12,0",  // coast day2 afternoon: min, max, mean
                                          "-11.09,0",
                                          "-8.61,0",
                                          "-9.85,0",  // area day2 afternoon: min, max, mean
                                          "Pakkanen heikkenee."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("-9.51,0",
                                          "-9.16,0",
                                          "-9.33,0",  // inland day1 afternoon: min, max, mean
                                          "-10.62,0",
                                          "-6.27,0",
                                          "-8.44,0",  // coast day1 afternoon: min, max, mean
                                          "-9.60,0",
                                          "-7.50,0",
                                          "-8.55,0",  // area day1 afternoon: min, max, mean
                                          "-2.73,0",
                                          "-2.54,0",
                                          "-2.64,0",  // inland day2 afternoon: min, max, mean
                                          "-3.39,0",
                                          "-2.44,0",
                                          "-2.92,0",  // coast day2 afternoon: min, max, mean
                                          "-3.01,0",
                                          "-1.72,0",
                                          "-2.36,0",      // area day2 afternoon: min, max, mean
                                          "Sää lauhtuu."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("-3.61,0",
                                          "-3.49,0",
                                          "-3.55,0",  // inland day1 afternoon: min, max, mean
                                          "-4.87,0",
                                          "-4.07,0",
                                          "-4.47,0",  // coast day1 afternoon: min, max, mean
                                          "-3.83,0",
                                          "-1.43,0",
                                          "-2.63,0",  // area day1 afternoon: min, max, mean
                                          "-0.03,0",
                                          "0.54,0",
                                          "0.25,0",  // inland day2 afternoon: min, max, mean
                                          "-1.51,0",
                                          "-0.02,0",
                                          "-0.77,0",  // coast day2 afternoon: min, max, mean
                                          "-0.76,0",
                                          "3.85,0",
                                          "1.54,0",  // area day2 afternoon: min, max, mean
                                          "Sää on edelleen lauhaa."  // the story
                                          )));
    }
  }
  else  // summer
  {
    if (language == "fi")
    {
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("14.03,0",
                                          "14.67,0",
                                          "14.35,0",  // inland day1 afternoon: min, max, mean
                                          "13.19,0",
                                          "17.85,0",
                                          "15.52,0",  // coast day1 afternoon: min, max, mean
                                          "13.61,0",
                                          "17.74,0",
                                          "15.68,0",  // area day1 afternoon: min, max, mean
                                          "13.55,0",
                                          "14.40,0",
                                          "13.97,0",  // inland day2 afternoon: min, max, mean
                                          "12.75,0",
                                          "13.33,0",
                                          "13.04,0",  // coast day2 afternoon: min, max, mean
                                          "12.84,0",
                                          "13.22,0",
                                          "13.03,0",       // area day2 afternoon: min, max, mean
                                          "Sää viilenee."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("16.15,0",
                                          "16.89,0",
                                          "16.52,0",  // inland day1 afternoon: min, max, mean
                                          "15.58,0",
                                          "18.02,0",
                                          "16.80,0",  // coast day1 afternoon: min, max, mean
                                          "15.81,0",
                                          "19.70,0",
                                          "17.75,0",  // area day1 afternoon: min, max, mean
                                          "13.17,0",
                                          "14.12,0",
                                          "13.65,0",  // inland day2 afternoon: min, max, mean
                                          "12.91,0",
                                          "14.71,0",
                                          "13.81,0",  // coast day2 afternoon: min, max, mean
                                          "12.66,0",
                                          "14.75,0",
                                          "13.70,0",       // area day2 afternoon: min, max, mean
                                          "Sää viilenee."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("4.45,0",
                                          "5.43,0",
                                          "4.94,0",  // inland day1 afternoon: min, max, mean
                                          "4.03,0",
                                          "6.55,0",
                                          "5.29,0",  // coast day1 afternoon: min, max, mean
                                          "4.16,0",
                                          "7.99,0",
                                          "6.08,0",  // area day1 afternoon: min, max, mean
                                          "4.54,0",
                                          "4.77,0",
                                          "4.66,0",  // inland day2 afternoon: min, max, mean
                                          "3.60,0",
                                          "5.30,0",
                                          "4.45,0",  // coast day2 afternoon: min, max, mean
                                          "4.44,0",
                                          "5.84,0",
                                          "5.14,0",  // area day2 afternoon: min, max, mean
                                          "Kolea sää jatkuu."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("9.04,0",
                                          "9.33,0",
                                          "9.19,0",  // inland day1 afternoon: min, max, mean
                                          "8.47,0",
                                          "10.45,0",
                                          "9.46,0",  // coast day1 afternoon: min, max, mean
                                          "8.25,0",
                                          "11.62,0",
                                          "9.93,0",  // area day1 afternoon: min, max, mean
                                          "14.72,0",
                                          "15.32,0",
                                          "15.02,0",  // inland day2 afternoon: min, max, mean
                                          "13.57,0",
                                          "17.05,0",
                                          "15.31,0",  // coast day2 afternoon: min, max, mean
                                          "13.84,0",
                                          "14.73,0",
                                          "14.29,0",       // area day2 afternoon: min, max, mean
                                          "Sää lämpenee."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("13.66,0",
                                          "13.93,0",
                                          "13.80,0",  // inland day1 afternoon: min, max, mean
                                          "13.38,0",
                                          "16.30,0",
                                          "14.84,0",  // coast day1 afternoon: min, max, mean
                                          "13.23,0",
                                          "15.63,0",
                                          "14.43,0",  // area day1 afternoon: min, max, mean
                                          "22.20,0",
                                          "22.64,0",
                                          "22.42,0",  // inland day2 afternoon: min, max, mean
                                          "21.68,0",
                                          "21.70,0",
                                          "21.69,0",  // coast day2 afternoon: min, max, mean
                                          "21.84,0",
                                          "24.93,0",
                                          "23.38,0",  // area day2 afternoon: min, max, mean
                                          "Sää lämpenee huomattavasti."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("20.93,0",
                                          "21.37,0",
                                          "21.15,0",  // inland day1 afternoon: min, max, mean
                                          "19.14,0",
                                          "23.10,0",
                                          "21.12,0",  // coast day1 afternoon: min, max, mean
                                          "20.85,0",
                                          "23.59,0",
                                          "22.22,0",  // area day1 afternoon: min, max, mean
                                          "25.07,0",
                                          "25.99,0",
                                          "25.53,0",  // inland day2 afternoon: min, max, mean
                                          "23.39,0",
                                          "26.39,0",
                                          "24.89,0",  // coast day2 afternoon: min, max, mean
                                          "24.54,0",
                                          "27.67,0",
                                          "26.11,0",  // area day2 afternoon: min, max, mean
                                          "Sää muuttuu helteiseksi."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("22.86,0",
                                          "23.07,0",
                                          "22.96,0",  // inland day1 afternoon: min, max, mean
                                          "22.63,0",
                                          "22.87,0",
                                          "22.75,0",  // coast day1 afternoon: min, max, mean
                                          "22.33,0",
                                          "25.18,0",
                                          "23.75,0",  // area day1 afternoon: min, max, mean
                                          "25.16,0",
                                          "25.58,0",
                                          "25.37,0",  // inland day2 afternoon: min, max, mean
                                          "23.25,0",
                                          "25.04,0",
                                          "24.14,0",  // coast day2 afternoon: min, max, mean
                                          "24.75,0",
                                          "26.11,0",
                                          "25.43,0",  // area day2 afternoon: min, max, mean
                                          "Helteinen sää jatkuu."  // the story
                                          )));
      testCases.insert(make_pair(
          i++,
          new TemperatureAnomalyTestParam("25.80,0",
                                          "26.61,0",
                                          "26.20,0",  // inland day1 afternoon: min, max, mean
                                          "25.38,0",
                                          "27.76,0",
                                          "26.57,0",  // coast day1 afternoon: min, max, mean
                                          "25.30,0",
                                          "27.62,0",
                                          "26.46,0",  // area day1 afternoon: min, max, mean
                                          "33.20,0",
                                          "34.02,0",
                                          "33.61,0",  // inland day2 afternoon: min, max, mean
                                          "32.76,0",
                                          "35.82,0",
                                          "34.29,0",  // coast day2 afternoon: min, max, mean
                                          "32.38,0",
                                          "33.03,0",
                                          "32.70,0",  // area day2 afternoon: min, max, mean
                                          "Helteinen sää jatkuu."  // the story
                                          )));
    }
  }
}
// void generate_testcasefile(const char* theFileName, const char* theLanguage, const bool& anomaly
// = false)
void generate_testcasefile(const char* theFileName,
                           const char* theLanguage,
                           const TemperatureTestType& theTestType)
{
  AnalysisSources sources;
  WeatherArea area("25,60");

  TestCaseContainer testCases;

  generate_testcase_parameters(testCases, theLanguage, theTestType);

  if (theTestType == TEMPERATURE_ANOMALY)
  {
    create_temperature_anomaly_testcase_stories(testCases, theLanguage, sources, area);
    create_temperature_anomaly_testcasefile(testCases, theFileName);
  }
  else if (theTestType == TEMPERATURE_MAX36_HOURS)
  {
    create_max36hours_testcase_stories(testCases, theLanguage, sources, area);
    create_max36hours_testcasefile(testCases, theFileName);
  }
  else if (theTestType == WIND_ANOMALY)
  {
    create_wind_anomaly_testcase_stories(testCases, theLanguage, sources, area);
    create_wind_anomaly_testcasefile(testCases, theFileName);
  }
}

void delete_testcases(TestCaseContainer& testCases)
{
  TestCaseContainer::iterator iter;
  for (iter = testCases.begin(); iter != testCases.end(); iter++)
    delete iter->second;
  testCases.clear();
}

void temperature_max36hours_season(const WeatherPeriod& period)
{
  AnalysisSources sources;
  WeatherArea area("25,60");
  const string fun = "temperature_max36hours";

  TemperatureStory story1(period.localStartTime(), sources, area, period, "max36hours");
  bool isWinter = SeasonTools::isWinterHalf(period.localEndTime(), "max36hours");

  TestCaseContainer testCases;
  TestCaseContainer::iterator iter;

  const char* languages[] = {"fi", "sv", "en"};

  create_max36hours_testcases(testCases, "fi", isWinter);

  // here you should iterate all languages
  for (int i = 0; i < 1; i++)
  {
    Max36HoursTestParam* param = 0;
    for (iter = testCases.begin(); iter != testCases.end(); iter++)
    {
      param = static_cast<Max36HoursTestParam*>(iter->second);

      Settings::set("max36hours::fake::day1::inland::min", param->temperature_d1_inlandmin);
      Settings::set("max36hours::fake::day1::inland::max", param->temperature_d1_inlandmax);
      Settings::set("max36hours::fake::day1::inland::mean", param->temperature_d1_inlandmean);
      Settings::set("max36hours::fake::day1::coast::min", param->temperature_d1_coastmin);
      Settings::set("max36hours::fake::day1::coast::max", param->temperature_d1_coastmax);
      Settings::set("max36hours::fake::day1::coast::mean", param->temperature_d1_coastmean);
      Settings::set("max36hours::fake::day1::area::min", param->temperature_d1_areamin);
      Settings::set("max36hours::fake::day1::area::max", param->temperature_d1_areamax);
      Settings::set("max36hours::fake::day1::area::mean", param->temperature_d1_areamean);

      Settings::set("max36hours::fake::day1::morning::inland::min",
                    param->temperature_d1_morning_inlandmin);
      Settings::set("max36hours::fake::day1::morning::inland::max",
                    param->temperature_d1_morning_inlandmax);
      Settings::set("max36hours::fake::day1::morning::inland::mean",
                    param->temperature_d1_morning_inlandmean);
      Settings::set("max36hours::fake::day1::morning::coast::min",
                    param->temperature_d1_morning_coastmin);
      Settings::set("max36hours::fake::day1::morning::coast::max",
                    param->temperature_d1_morning_coastmax);
      Settings::set("max36hours::fake::day1::morning::coast::mean",
                    param->temperature_d1_morning_coastmean);
      Settings::set("max36hours::fake::day1::morning::area::min",
                    param->temperature_d1_morning_areamin);
      Settings::set("max36hours::fake::day1::morning::area::max",
                    param->temperature_d1_morning_areamax);
      Settings::set("max36hours::fake::day1::morning::area::mean",
                    param->temperature_d1_morning_areamean);

      Settings::set("max36hours::fake::day1::afternoon::inland::min",
                    param->temperature_d1_afternoon_inlandmin);
      Settings::set("max36hours::fake::day1::afternoon::inland::max",
                    param->temperature_d1_afternoon_inlandmax);
      Settings::set("max36hours::fake::day1::afternoon::inland::mean",
                    param->temperature_d1_afternoon_inlandmean);
      Settings::set("max36hours::fake::day1::afternoon::coast::min",
                    param->temperature_d1_afternoon_coastmin);
      Settings::set("max36hours::fake::day1::afternoon::coast::max",
                    param->temperature_d1_afternoon_coastmax);
      Settings::set("max36hours::fake::day1::afternoon::coast::mean",
                    param->temperature_d1_afternoon_coastmean);
      Settings::set("max36hours::fake::day1::afternoon::area::min",
                    param->temperature_d1_afternoon_areamin);
      Settings::set("max36hours::fake::day1::afternoon::area::max",
                    param->temperature_d1_afternoon_areamax);
      Settings::set("max36hours::fake::day1::afternoon::area::mean",
                    param->temperature_d1_afternoon_areamean);
      Settings::set("max36hours::fake::temperature::day_before_day1::afternoon::area::min",
                    param->temperature_d1_afternoon_areamin);
      Settings::set("max36hours::fake::temperature::day_before_day1::afternoon::area::max",
                    param->temperature_d1_afternoon_areamax);
      Settings::set("max36hours::fake::temperature::day_before_day1::afternoon::area::mean",
                    param->temperature_d1_afternoon_areamean);

      Settings::set("max36hours::fake::night::inland::min", param->temperature_nite_inlandmin);
      Settings::set("max36hours::fake::night::inland::max", param->temperature_nite_inlandmax);
      Settings::set("max36hours::fake::night::inland::mean", param->temperature_nite_inlandmean);
      Settings::set("max36hours::fake::night::coast::min", param->temperature_nite_coastmin);
      Settings::set("max36hours::fake::night::coast::max", param->temperature_nite_coastmax);
      Settings::set("max36hours::fake::night::coast::mean", param->temperature_nite_coastmean);
      Settings::set("max36hours::fake::night::area::min", param->temperature_nite_areamin);
      Settings::set("max36hours::fake::night::area::max", param->temperature_nite_areamax);
      Settings::set("max36hours::fake::night::area::mean", param->temperature_nite_areamean);

      Settings::set("max36hours::fake::day2::inland::min", param->temperature_d2_inlandmin);
      Settings::set("max36hours::fake::day2::inland::max", param->temperature_d2_inlandmax);
      Settings::set("max36hours::fake::day2::inland::mean", param->temperature_d2_inlandmean);
      Settings::set("max36hours::fake::day2::coast::min", param->temperature_d2_coastmin);
      Settings::set("max36hours::fake::day2::coast::max", param->temperature_d2_coastmax);
      Settings::set("max36hours::fake::day2::coast::mean", param->temperature_d2_coastmean);
      Settings::set("max36hours::fake::day2::area::min", param->temperature_d2_areamin);
      Settings::set("max36hours::fake::day2::area::max", param->temperature_d2_areamax);
      Settings::set("max36hours::fake::day2::area::mean", param->temperature_d2_areamean);

      Settings::set("max36hours::fake::day2::morning::inland::min",
                    param->temperature_d2_morning_inlandmin);
      Settings::set("max36hours::fake::day2::morning::inland::max",
                    param->temperature_d2_morning_inlandmax);
      Settings::set("max36hours::fake::day2::morning::inland::mean",
                    param->temperature_d2_morning_inlandmean);
      Settings::set("max36hours::fake::day2::morning::coast::min",
                    param->temperature_d2_morning_coastmin);
      Settings::set("max36hours::fake::day2::morning::coast::max",
                    param->temperature_d2_morning_coastmax);
      Settings::set("max36hours::fake::day2::morning::coast::mean",
                    param->temperature_d2_morning_coastmean);
      Settings::set("max36hours::fake::day2::morning::area::min",
                    param->temperature_d2_morning_areamin);
      Settings::set("max36hours::fake::day2::morning::area::max",
                    param->temperature_d2_morning_areamax);
      Settings::set("max36hours::fake::day2::morning::area::mean",
                    param->temperature_d2_morning_areamean);

      Settings::set("max36hours::fake::day2::afternoon::inland::min",
                    param->temperature_d2_afternoon_inlandmin);
      Settings::set("max36hours::fake::day2::afternoon::inland::max",
                    param->temperature_d2_afternoon_inlandmax);
      Settings::set("max36hours::fake::day2::afternoon::inland::mean",
                    param->temperature_d2_afternoon_inlandmean);
      Settings::set("max36hours::fake::day2::afternoon::coast::min",
                    param->temperature_d2_afternoon_coastmin);
      Settings::set("max36hours::fake::day2::afternoon::coast::max",
                    param->temperature_d2_afternoon_coastmax);
      Settings::set("max36hours::fake::day2::afternoon::coast::mean",
                    param->temperature_d2_afternoon_coastmean);
      Settings::set("max36hours::fake::day2::afternoon::area::min",
                    param->temperature_d2_afternoon_areamin);
      Settings::set("max36hours::fake::day2::afternoon::area::max",
                    param->temperature_d2_afternoon_areamax);
      Settings::set("max36hours::fake::day2::afternoon::area::mean",
                    param->temperature_d2_afternoon_areamean);
      Settings::set("max36hours::fake::temperature::day_before_day1::afternoon::area::min",
                    param->temperature_d2_afternoon_areamin);
      Settings::set("max36hours::fake::temperature::day_before_day1::afternoon::area::max",
                    param->temperature_d2_afternoon_areamax);
      Settings::set("max36hours::fake::temperature::day_before_day1::afternoon::area::mean",
                    param->temperature_d2_afternoon_areamean);

      require(story1, languages[i], fun, param->theStory);
    }
    delete_testcases(testCases);
  }
}

void temperature_max36hours()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  Settings::set("textgen::tmax_climatology",
                "/smartmet/data/climate/suomi/daily/querydata/1970-2001/tmax_textgen.sqd");
  Settings::set("textgen::fractiles_climatology",
                "/smartmet/data/climate/suomi/daily/querydata/1970-2001/tmax_textgen.sqd");
  Settings::set("max36hours::fake::fractile02_limit", "2.0");
  Settings::set("max36hours::day::starthour", "6");
  Settings::set("max36hours::day::maxstarthour", "11");
  Settings::set("max36hours::day::endhour", "18");
  Settings::set("max36hours::night::starthour", "18");
  Settings::set("max36hours::night::endhour", "6");
  Settings::set("max36hours::fake::area_percentage", "10.0");
  Settings::set("max36hours::fake::growing_season_on", "false");

  TextGenPosixTime summertime1(2009, 6, 1, 6, 0, 0);
  TextGenPosixTime summertime2(2009, 6, 2, 18, 0, 0);
  TextGenPosixTime wintertime1(2009, 1, 1, 6, 0, 0);
  TextGenPosixTime wintertime2(2009, 1, 2, 18, 0, 0);

  WeatherPeriod summerPeriod(summertime1, summertime2);
  WeatherPeriod winterPeriod(wintertime1, wintertime2);

  temperature_max36hours_season(summerPeriod);
  temperature_max36hours_season(winterPeriod);

  //	generate_testcasefile("max36hours_testcases.txt", "fi", TEMPERATURE_MAX36_HOURS);

  TEST_PASSED();
}

shared_ptr<NFmiQueryData> theQD;

void read_querydata(const std::string& theFilename) { theQD.reset(new NFmiQueryData(theFilename)); }
void temperature_season_anomaly(const WeatherPeriod& period)
{
  const string fun = "temperature_anomaly";
  bool isWinter = SeasonTools::isWinterHalf(period.localEndTime(), "anomaly");
  AnalysisSources sources;
  WeatherArea area("25,60");

  TemperatureStory story(period.localStartTime(), sources, area, period, "anomaly");

  TestCaseContainer testCases;
  TestCaseContainer::iterator iter;

  const char* languages[] = {"fi", "sv", "en"};

  // here you should iterate all languages
  for (int i = 0; i < 1; i++)
  {
    create_anomaly_testcases(testCases, languages[i], isWinter);

    TemperatureAnomalyTestParam* param = 0;

    unsigned int textCaseCounter(0);
    for (iter = testCases.begin(); iter != testCases.end(); iter++)
    {
      param = static_cast<TemperatureAnomalyTestParam*>(iter->second);

      size_t index(param->temperature_d1_afternoon_areamean.find(","));
      float mean_temperature(
          atof(param->temperature_d1_afternoon_areamean.substr(0, index).c_str()));

      Settings::set("anomaly::fake::fractile::share::F02",
                    get_fractile_share(FRACTILE_02, mean_temperature, period.localEndTime()));
      Settings::set("anomaly::fake::fractile::share::F12",
                    get_fractile_share(FRACTILE_12, mean_temperature, period.localEndTime()));
      Settings::set("anomaly::fake::fractile::share::F88",
                    get_fractile_share(FRACTILE_88, mean_temperature, period.localEndTime()));
      Settings::set("anomaly::fake::fractile::share::F98",
                    get_fractile_share(FRACTILE_98, mean_temperature, period.localEndTime()));

      Settings::set("anomaly::fake::temperature::day1::afternoon::area::min",
                    param->temperature_d1_afternoon_areamin);
      Settings::set("anomaly::fake::temperature::day1::afternoon::area::max",
                    param->temperature_d1_afternoon_areamax);
      Settings::set("anomaly::fake::temperature::day1::afternoon::area::mean",
                    param->temperature_d1_afternoon_areamean);
      Settings::set("anomaly::fake::temperature::day1::afternoon::inland::min",
                    param->temperature_d1_afternoon_inlandmin);
      Settings::set("anomaly::fake::temperature::day1::afternoon::inland::max",
                    param->temperature_d1_afternoon_inlandmax);
      Settings::set("anomaly::fake::temperature::day1::afternoon::inland::mean",
                    param->temperature_d1_afternoon_inlandmean);
      Settings::set("anomaly::fake::temperature::day1::afternoon::coast::min",
                    param->temperature_d1_afternoon_coastmin);
      Settings::set("anomaly::fake::temperature::day1::afternoon::coast::max",
                    param->temperature_d1_afternoon_coastmax);
      Settings::set("anomaly::fake::temperature::day1::afternoon::coast::mean",
                    param->temperature_d1_afternoon_coastmean);

      Settings::set("anomaly::fake::temperature::day2::afternoon::area::min",
                    param->temperature_d2_afternoon_areamin);
      Settings::set("anomaly::fake::temperature::day2::afternoon::area::max",
                    param->temperature_d2_afternoon_areamax);
      Settings::set("anomaly::fake::temperature::day2::afternoon::area::mean",
                    param->temperature_d2_afternoon_areamean);
      Settings::set("anomaly::fake::temperature::day2::afternoon::inland::min",
                    param->temperature_d2_afternoon_inlandmin);
      Settings::set("anomaly::fake::temperature::day2::afternoon::inland::max",
                    param->temperature_d2_afternoon_inlandmax);
      Settings::set("anomaly::fake::temperature::day2::afternoon::inland::mean",
                    param->temperature_d2_afternoon_inlandmean);
      Settings::set("anomaly::fake::temperature::day2::afternoon::coast::min",
                    param->temperature_d2_afternoon_coastmin);
      Settings::set("anomaly::fake::temperature::day2::afternoon::coast::max",
                    param->temperature_d2_afternoon_coastmax);
      Settings::set("anomaly::fake::temperature::day2::afternoon::coast::mean",
                    param->temperature_d2_afternoon_coastmean);

      Settings::set("anomaly::fake::temperature::day_before_day1::afternoon::area::min",
                    param->temperature_d1_afternoon_areamin);
      Settings::set("anomaly::fake::temperature::day_before_day1::afternoon::area::max",
                    param->temperature_d1_afternoon_areamax);
      Settings::set("anomaly::fake::temperature::day_before_day1::afternoon::area::mean",
                    param->temperature_d1_afternoon_areamean);
      Settings::set("anomaly::fake::temperature::day_after_day2::afternoon::area::min",
                    param->temperature_d2_afternoon_areamin);
      Settings::set("anomaly::fake::temperature::day_after_day2::afternoon::area::max",
                    param->temperature_d2_afternoon_areamax);
      Settings::set("anomaly::fake::temperature::day_after_day2::afternoon::area::mean",
                    param->temperature_d2_afternoon_areamean);

      require(story, languages[i], fun, param->theStory);
      textCaseCounter++;
    }
    delete_testcases(testCases);
  }
}

void temperature_anomaly()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  Settings::set("textgen::tmax_climatology",
                "/smartmet/data/climate/suomi/daily/querydata/1970-2001/tmax_textgen.sqd");
  Settings::set("textgen::fractiles_climatology",
                "/smartmet/data/climate/suomi/daily/querydata/1970-2001/tmax_textgen.sqd");
  Settings::set("anomaly::fake::fractile02_limit", "2.0");
  Settings::set("anomaly::day::starthour", "6");
  Settings::set("anomaly::day::maxstarthour", "11");
  Settings::set("anomaly::day::endhour", "18");
  Settings::set("anomaly::night::starthour", "18");
  Settings::set("anomaly::night::endhour", "6");

  // winter fractiles
  Settings::set("anomaly::fake::fractile::winter::F02", "-25.0");
  Settings::set("anomaly::fake::fractile::winter::F12", "-20.0");
  Settings::set("anomaly::fake::fractile::winter::F37", "-15.0");
  Settings::set("anomaly::fake::fractile::winter::F50", "-10.0");
  Settings::set("anomaly::fake::fractile::winter::F63", "-5.0");
  Settings::set("anomaly::fake::fractile::winter::F88", "0.0");
  Settings::set("anomaly::fake::fractile::winter::F98", "3.0");
  // summer fractiles
  Settings::set("anomaly::fake::fractile::summer::F02", "5.0");
  Settings::set("anomaly::fake::fractile::summer::F12", "12.0");
  Settings::set("anomaly::fake::fractile::summer::F37", "14.0");
  Settings::set("anomaly::fake::fractile::summer::F50", "16.0");
  Settings::set("anomaly::fake::fractile::summer::F63", "19.0");
  Settings::set("anomaly::fake::fractile::summer::F88", "22.0");
  Settings::set("anomaly::fake::fractile::summer::F98", "25.0");
  Settings::set("anomaly::fake::growing_season_on", "false");

  TextGenPosixTime summertime1(2009, 6, 1, 6, 0, 0);
  TextGenPosixTime summertime2(2009, 6, 2, 18, 0, 0);
  TextGenPosixTime wintertime1(2009, 1, 1, 6, 0, 0);
  TextGenPosixTime wintertime2(2009, 1, 2, 18, 0, 0);

  WeatherPeriod summerPeriod(summertime1, summertime2);
  WeatherPeriod winterPeriod(wintertime1, wintertime2);

  temperature_season_anomaly(summerPeriod);
  temperature_season_anomaly(winterPeriod);

  // generate_testcasefile("temperature_anomaly_testcases.txt", "fi", TEMPERATURE_ANOMALY);

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief The actual test driver
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(temperature_max36hours);
    TEST(temperature_anomaly);
    TEST(temperature_day);
    TEST(temperature_mean);
    TEST(temperature_meanmax);
    TEST(temperature_meanmin);
    TEST(temperature_dailymax);
    TEST(temperature_weekly_minmax);
    TEST(temperature_weekly_averages);
    TEST(temperature_range);
  }

};  // class tests

}  // namespace TemperatureStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  NFmiSettings::Set("textgen::units::celsius::format", "phrase");
  Settings::set(NFmiSettings::ToString());

  using namespace TemperatureStoryTest;
  MessageLogger::open("my.log");

  cout << endl << "TemperatureStory tests" << endl << "======================" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
