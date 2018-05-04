#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "PrecipitationStory.h"
#include "Story.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace PrecipitationStoryTest
{
shared_ptr<TextGen::Dictionary> dict;
TextGen::PlainTextFormatter formatter;

string require(const TextGen::Story& theStory,
               const string& theLanguage,
               const string& theName,
               const string& theExpected)
{
  dict->init(theLanguage);
  formatter.dictionary(dict);

  TextGen::Paragraph para = theStory.makeStory(theName);
  const string result = para.realize(formatter);

  if (result != theExpected)
    return (result + " < > " + theExpected);
  else
    return "";
}

#define REQUIRE(story, lan, name, expected)     \
  result = require(story, lan, name, expected); \
  if (!result.empty()) TEST_FAILED(result.c_str());

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::total()
 */
// ----------------------------------------------------------------------

void precipitation_total()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  PrecipitationStory story(time1, sources, area, period, "total");

  const string fun = "precipitation_total";
  string result;

  Settings::set("total::fake::mean", "0.1,0");
  REQUIRE(story, "fi", fun, "Sadesumma 0 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 0 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 0 millimeters.");

  Settings::set("total::fake::mean", "0.5,0");
  REQUIRE(story, "fi", fun, "Sadesumma 1 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 1 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 1 millimeters.");

  Settings::set("total::fake::mean", "10,0");
  REQUIRE(story, "fi", fun, "Sadesumma 10 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 10 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 10 millimeters.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::total_day()
 */
// ----------------------------------------------------------------------

void precipitation_total_day()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  PrecipitationStory story(time1, sources, area, period, "total_day");

  const string fun = "precipitation_total_day";
  string result;

  Settings::set("total_day::fake::mean", "0.0,0");
  REQUIRE(story, "fi", fun, "Sadesumma 0 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 0 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 0 millimeters.");

  Settings::set("total_day::fake::mean", "0.1,0");
  REQUIRE(story, "fi", fun, "Sadesumma alle yksi millimetri.");
  REQUIRE(story, "sv", fun, "Nederbördssumman under en millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation below one millimeter.");

  Settings::set("total_day::fake::mean", "0.5,0");
  REQUIRE(story, "fi", fun, "Sadesumma 1 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 1 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 1 millimeters.");

  Settings::set("total_day::fake::mean", "10,0");
  REQUIRE(story, "fi", fun, "Sadesumma 10 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 10 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 10 millimeters.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::range()
 */
// ----------------------------------------------------------------------

void precipitation_range()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  PrecipitationStory story(time1, sources, area, period, "range");

  const string fun = "precipitation_range";
  string result;

  Settings::set("range::maxrain", "20");

  Settings::set("range::fake::minimum", "0,0");
  Settings::set("range::fake::maximum", "0,0");
  REQUIRE(story, "fi", fun, "Sadesumma on 0 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman är 0 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation is 0 millimeters.");

  Settings::set("range::fake::minimum", "5,0");
  Settings::set("range::fake::maximum", "5,0");
  REQUIRE(story, "fi", fun, "Sadesumma on 5 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman är 5 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation is 5 millimeters.");

  Settings::set("range::fake::minimum", "5,0");
  Settings::set("range::fake::maximum", "10,0");
  REQUIRE(story, "fi", fun, "Sadesumma on 5-10 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman är 5-10 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation is 5-10 millimeters.");

  Settings::set("range::fake::minimum", "15,0");
  Settings::set("range::fake::maximum", "25,0");
  REQUIRE(story, "fi", fun, "Sadesumma on 15-25 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman är 15-25 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation is 15-25 millimeters.");

  Settings::set("range::fake::minimum", "25,0");
  Settings::set("range::fake::maximum", "30,0");
  REQUIRE(story, "fi", fun, "Sadesumma on yli 20 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman är över 20 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation is over 20 millimeters.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::sums()
 */
// ----------------------------------------------------------------------

void precipitation_sums()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 3);
  WeatherPeriod period(time1, time2);
  PrecipitationStory story(time1, sources, area, period, "sums");

  const string fun = "precipitation_sums";
  string result;

  Settings::set("sums::mininterval", "3");

  Settings::set("sums::fake::period1::minimum", "0,0");
  Settings::set("sums::fake::period1::maximum", "0,0");
  Settings::set("sums::fake::period1::mean", "0,0");
  Settings::set("sums::fake::period2::minimum", "0,0");
  Settings::set("sums::fake::period2::maximum", "0,0");
  Settings::set("sums::fake::period2::mean", "0,0");
  REQUIRE(story, "fi", fun, "Seuraavan 24 tunnin sademäärä on 0 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman för de följande 24 timmar är 0 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation for the next 24 hours is 0 millimeters.");

  Settings::set("sums::fake::period2::minimum", "5,0");
  Settings::set("sums::fake::period2::maximum", "10,0");
  Settings::set("sums::fake::period2::mean", "8,0");
  REQUIRE(
      story,
      "fi",
      fun,
      "Ensimmäisen 12 tunnin sademäärä on 0 millimetriä, seuraavan 12 tunnin 5-10 millimetriä.");
  REQUIRE(story,
          "sv",
          fun,
          "Nederbördssumman för de första 12 timmar är 0 millimeter, för de följande 12 timmar "
          "5-10 millimeter.");
  REQUIRE(story,
          "en",
          fun,
          "Total precipitation for the first 12 hours is 0 millimeters, for the following 12 hours "
          "5-10 millimeters.");

  Settings::set("sums::fake::period1::minimum", "0,0");
  Settings::set("sums::fake::period1::maximum", "2,0");
  Settings::set("sums::fake::period1::mean", "1,0");
  REQUIRE(story,
          "fi",
          fun,
          "Ensimmäisen 12 tunnin sademäärä on noin 1 millimetriä, seuraavan 12 tunnin 5-10 "
          "millimetriä.");
  REQUIRE(story,
          "sv",
          fun,
          "Nederbördssumman för de första 12 timmar är cirka 1 millimeter, för de följande 12 "
          "timmar 5-10 millimeter.");
  REQUIRE(story,
          "en",
          fun,
          "Total precipitation for the first 12 hours is about 1 millimeters, for the following 12 "
          "hours 5-10 millimeters.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::classification()
 */
// ----------------------------------------------------------------------

void precipitation_classification()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  PrecipitationStory story(time1, sources, area, period, "classification");

  string result;

  const string fun = "precipitation_classification";
  Settings::set("classification::classes", "0...2,2...5,5...10,10...20");
  Settings::set("classification::max_some_places", "30");
  Settings::set("classification::max_many_places", "70");
  Settings::set("classification::some_places", "30");
  Settings::set("classification::many_places", "70");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "0,0");
  Settings::set("classification::fake::maximum", "0,0");
  Settings::set("classification::fake::percentage", "0,0");
  REQUIRE(story, "fi", fun, "Sadesumma 0 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 0 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 0 millimeters.");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "1,0");
  Settings::set("classification::fake::maximum", "2,0");
  Settings::set("classification::fake::percentage", "0,0");
  REQUIRE(story, "fi", fun, "Sadesumma 0-2 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 0-2 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 0-2 millimeters.");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "4,0");
  Settings::set("classification::fake::maximum", "6,0");
  Settings::set("classification::fake::percentage", "0,0");
  REQUIRE(story, "fi", fun, "Sadesumma 2-5 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 2-5 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 2-5 millimeters.");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "4,0");
  Settings::set("classification::fake::maximum", "6,0");
  Settings::set("classification::fake::percentage", "40,0");
  REQUIRE(story, "fi", fun, "Sadesumma 2-5 millimetriä, paikoin enemmän.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 2-5 millimeter, lokalt mera.");
  REQUIRE(story, "en", fun, "Total precipitation 2-5 millimeters, more in some places.");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "4,0");
  Settings::set("classification::fake::maximum", "6,0");
  Settings::set("classification::fake::percentage", "80,0");
  REQUIRE(story, "fi", fun, "Sadesumma 2-5 millimetriä, monin paikoin enemmän.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 2-5 millimeter, flerstädes mera.");
  REQUIRE(story, "en", fun, "Total precipitation 2-5 millimeters, more in many places.");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "15,0");
  Settings::set("classification::fake::maximum", "20,0");
  Settings::set("classification::fake::percentage", "0,0");
  REQUIRE(story, "fi", fun, "Sadesumma 10-20 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 10-20 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 10-20 millimeters.");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "15,0");
  Settings::set("classification::fake::maximum", "25,0");
  Settings::set("classification::fake::percentage", "10,0");
  REQUIRE(story, "fi", fun, "Sadesumma 10-20 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman 10-20 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation 10-20 millimeters.");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "15,0");
  Settings::set("classification::fake::maximum", "25,0");
  Settings::set("classification::fake::percentage", "40,0");
  REQUIRE(story, "fi", fun, "Sadesumma paikoin yli 20 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman lokalt över 20 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation in some places over 20 millimeters.");

  Settings::set("classification::fake::minimum", "0,0");
  Settings::set("classification::fake::mean", "15,0");
  Settings::set("classification::fake::maximum", "25,0");
  Settings::set("classification::fake::percentage", "80,0");
  REQUIRE(story, "fi", fun, "Sadesumma monin paikoin yli 20 millimetriä.");
  REQUIRE(story, "sv", fun, "Nederbördssumman flerstädes över 20 millimeter.");
  REQUIRE(story, "en", fun, "Total precipitation in many places over 20 millimeters.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::pop_twodays
 */
// ----------------------------------------------------------------------

void pop_twodays()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  const string fun = "pop_twodays";

  string result;

  Settings::set("pop_twodays::day::starthour", "6");
  Settings::set("pop_twodays::day::endhour", "18");
  Settings::set("pop_twodays::precision", "10");
  Settings::set("pop_twodays::limit", "10");
  Settings::set("pop_twodays::comparison::significantly_greater", "50");
  Settings::set("pop_twodays::comparison::greater", "30");
  Settings::set("pop_twodays::comparison::somewhat_greater", "10");
  Settings::set("pop_twodays::comparison::somewhat_smaller", "10");
  Settings::set("pop_twodays::comparison::smaller", "30");
  Settings::set("pop_twodays::comparison::significantly_smaller", "50");

  // 1-day forecasts
  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 6, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_twodays");

    Settings::set("pop_twodays::fake::day1::maximum", "10,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 10%.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 10%.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 10%.");
  }

  // Another 1-day forecast, because 17 < 18 (minendhour)

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 17, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_twodays");

    Settings::set("pop_twodays::fake::day1::maximum", "20,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 20%.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 20%.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 20%.");
  }

  // Another 1-day forecast with small PoP

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 6, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_twodays");

    Settings::set("pop_twodays::fake::day1::maximum", "0,0");
    REQUIRE(story, "fi", fun, "");
    REQUIRE(story, "sv", fun, "");
    REQUIRE(story, "en", fun, "");
  }

  // 2-day forecasts

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 18, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_twodays");

    Settings::set("pop_twodays::fake::day1::maximum", "50,0");
    Settings::set("pop_twodays::fake::day2::maximum", "50,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna sama.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon densamma.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow the same.");

    Settings::set("pop_twodays::fake::day1::maximum", "60,0");
    Settings::set("pop_twodays::fake::day2::maximum", "10,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 60%, huomenna huomattavasti pienempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 60%, i morgon betydligt mindre.");
    REQUIRE(
        story, "en", fun, "Probability of precipitation is 60%, tomorrow significantly smaller.");

    Settings::set("pop_twodays::fake::day1::maximum", "50,0");
    Settings::set("pop_twodays::fake::day2::maximum", "20,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna pienempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon mindre.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow smaller.");

    Settings::set("pop_twodays::fake::day1::maximum", "50,0");
    Settings::set("pop_twodays::fake::day2::maximum", "40,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna hieman pienempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon något mindre.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow somewhat smaller.");

    Settings::set("pop_twodays::fake::day1::maximum", "50,0");
    Settings::set("pop_twodays::fake::day2::maximum", "60,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna hieman suurempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon något större.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow somewhat greater.");

    Settings::set("pop_twodays::fake::day1::maximum", "50,0");
    Settings::set("pop_twodays::fake::day2::maximum", "80,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna suurempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon större.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow greater.");

    Settings::set("pop_twodays::fake::day1::maximum", "50,0");
    Settings::set("pop_twodays::fake::day2::maximum", "100,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna huomattavasti suurempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon betydligt större.");
    REQUIRE(
        story, "en", fun, "Probability of precipitation is 50%, tomorrow significantly greater.");

    Settings::set("pop_twodays::fake::day1::maximum", "50,0");
    Settings::set("pop_twodays::fake::day2::maximum", "0,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%.");

    Settings::set("pop_twodays::fake::day1::maximum", "0,0");
    Settings::set("pop_twodays::fake::day2::maximum", "50,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on huomenna 50%.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är i morgon 50%.");
    REQUIRE(story, "en", fun, "Probability of precipitation is tomorrow 50%.");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::pop_days
 */
// ----------------------------------------------------------------------

void pop_days()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  const string fun = "pop_days";

  string result;

  Settings::set("pop_days::day::starthour", "6");
  Settings::set("pop_days::day::endhour", "18");
  Settings::set("pop_days::precision", "10");
  Settings::set("pop_days::minimum", "10");
  Settings::set("pop_days::maximum", "90");
  Settings::set("pop_days::comparison::significantly_greater", "50");
  Settings::set("pop_days::comparison::greater", "30");
  Settings::set("pop_days::comparison::somewhat_greater", "10");
  Settings::set("pop_days::comparison::somewhat_smaller", "10");
  Settings::set("pop_days::comparison::smaller", "30");
  Settings::set("pop_days::comparison::significantly_smaller", "50");

  // 1-day forecasts
  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 6, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_days");

    Settings::set("pop_days::fake::day1::meanmean", "0,0");
    Settings::set("pop_days::fake::day1::meanmax", "40,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 20%.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 20%.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 20%.");
  }

  // Another 1-day forecast, because 17 < 18 (minendhour)

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 17, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_days");

    Settings::set("pop_days::fake::day1::meanmean", "20,0");
    Settings::set("pop_days::fake::day1::meanmax", "40,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 30%.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 30%.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 30%.");
  }

  // Another 1-day forecast with small PoP

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 6, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_days");

    Settings::set("pop_days::fake::day1::meanmean", "0,0");
    Settings::set("pop_days::fake::day1::meanmax", "0,0");
    REQUIRE(story, "fi", fun, "");
    REQUIRE(story, "sv", fun, "");
    REQUIRE(story, "en", fun, "");
  }

  // Another 1-day forecast with large PoP

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 6, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_days");

    Settings::set("pop_days::fake::day1::meanmean", "90,0");
    Settings::set("pop_days::fake::day1::meanmax", "100,0");
    REQUIRE(story, "fi", fun, "");
    REQUIRE(story, "sv", fun, "");
    REQUIRE(story, "en", fun, "");
  }

  // 2-day forecasts

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 18, 0);
    WeatherPeriod period(time1, time2);
    PrecipitationStory story(time1, sources, area, period, "pop_days");

    Settings::set("pop_days::fake::day1::meanmean", "50,0");
    Settings::set("pop_days::fake::day1::meanmax", "50,0");
    Settings::set("pop_days::fake::day2::meanmean", "50,0");
    Settings::set("pop_days::fake::day2::meanmax", "50,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna sama.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon densamma.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow the same.");

    Settings::set("pop_days::fake::day1::meanmean", "60,0");
    Settings::set("pop_days::fake::day1::meanmax", "60,0");
    Settings::set("pop_days::fake::day2::meanmean", "10,0");
    Settings::set("pop_days::fake::day2::meanmax", "10,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 60%, huomenna huomattavasti pienempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 60%, i morgon betydligt mindre.");
    REQUIRE(
        story, "en", fun, "Probability of precipitation is 60%, tomorrow significantly smaller.");

    Settings::set("pop_days::fake::day1::meanmean", "50,0");
    Settings::set("pop_days::fake::day1::meanmax", "50,0");
    Settings::set("pop_days::fake::day2::meanmean", "20,0");
    Settings::set("pop_days::fake::day2::meanmax", "20,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna pienempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon mindre.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow smaller.");

    Settings::set("pop_days::fake::day1::meanmean", "50,0");
    Settings::set("pop_days::fake::day1::meanmax", "50,0");
    Settings::set("pop_days::fake::day2::meanmean", "40,0");
    Settings::set("pop_days::fake::day2::meanmax", "40,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna hieman pienempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon något mindre.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow somewhat smaller.");

    Settings::set("pop_days::fake::day1::meanmean", "50,0");
    Settings::set("pop_days::fake::day1::meanmax", "50,0");
    Settings::set("pop_days::fake::day2::meanmean", "60,0");
    Settings::set("pop_days::fake::day2::meanmax", "60,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna hieman suurempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon något större.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow somewhat greater.");

    Settings::set("pop_days::fake::day1::meanmean", "50,0");
    Settings::set("pop_days::fake::day1::meanmax", "50,0");
    Settings::set("pop_days::fake::day2::meanmean", "80,0");
    Settings::set("pop_days::fake::day2::meanmax", "80,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%, huomenna suurempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%, i morgon större.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%, tomorrow greater.");

    Settings::set("pop_days::fake::day1::meanmean", "40,0");
    Settings::set("pop_days::fake::day1::meanmax", "40,0");
    Settings::set("pop_days::fake::day2::meanmean", "90,0");
    Settings::set("pop_days::fake::day2::meanmax", "90,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 40%, huomenna huomattavasti suurempi.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 40%, i morgon betydligt större.");
    REQUIRE(
        story, "en", fun, "Probability of precipitation is 40%, tomorrow significantly greater.");

    Settings::set("pop_days::fake::day1::meanmean", "50,0");
    Settings::set("pop_days::fake::day1::meanmax", "50,0");
    Settings::set("pop_days::fake::day2::meanmean", "0,0");
    Settings::set("pop_days::fake::day2::meanmax", "0,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 50%.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 50%.");
    REQUIRE(story, "en", fun, "Probability of precipitation is 50%.");

    Settings::set("pop_days::fake::day1::meanmean", "0,0");
    Settings::set("pop_days::fake::day1::meanmax", "0,0");
    Settings::set("pop_days::fake::day2::meanmean", "50,0");
    Settings::set("pop_days::fake::day2::meanmax", "50,0");
    REQUIRE(story, "fi", fun, "Sateen todennäköisyys on huomenna 50%.");
    REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är i morgon 50%.");
    REQUIRE(story, "en", fun, "Probability of precipitation is tomorrow 50%.");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::pop_max
 */
// ----------------------------------------------------------------------

void pop_max()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  const string fun = "pop_max";

  string result;

  Settings::set("pop_max::precision", "10");
  Settings::set("pop_max::minimum", "10");
  Settings::set("pop_max::maximum", "90");

  TextGenPosixTime time1(2003, 6, 3, 6, 0);
  TextGenPosixTime time2(2003, 6, 4, 6, 0);
  WeatherPeriod period(time1, time2);
  PrecipitationStory story(time1, sources, area, period, "pop_max");

  Settings::set("pop_max::fake::max", "0,0");
  REQUIRE(story, "fi", fun, "");
  REQUIRE(story, "sv", fun, "");
  REQUIRE(story, "en", fun, "");

  Settings::set("pop_max::fake::max", "100,0");
  REQUIRE(story, "fi", fun, "");
  REQUIRE(story, "sv", fun, "");
  REQUIRE(story, "en", fun, "");

  Settings::set("pop_max::fake::max", "10,0");
  REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 10%.");
  REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 10%.");
  REQUIRE(story, "en", fun, "Probability of precipitation is 10%.");

  Settings::set("pop_max::fake::max", "90,0");
  REQUIRE(story, "fi", fun, "Sateen todennäköisyys on 90%.");
  REQUIRE(story, "sv", fun, "Sannolikheten för nederbörd är 90%.");
  REQUIRE(story, "en", fun, "Probability of precipitation is 90%.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PrecipitationStory::daily_sums()
 */
// ----------------------------------------------------------------------

void precipitation_daily_sums()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 3);
  TextGenPosixTime time3(2000, 1, 4);

  const string fun = "precipitation_daily_sums";
  string result;

  Settings::set("daily_sums::mininterval", "3");

  Settings::set("daily_sums::day::starthour", "0");
  Settings::set("daily_sums::day::maxstarthour", "12");
  Settings::set("daily_sums::day::endhour", "0");
  Settings::set("daily_sums::today::phrases", "today,tomorrow");

  // 2-day forecasts, zero rains not ignored
  {
    Settings::set("daily_sums::ignore_fair_days", "false");
    WeatherPeriod period(time1, time2);

    PrecipitationStory story(time1, sources, area, period, "daily_sums");

    Settings::set("daily_sums::fake::day1::minimum", "0,0");
    Settings::set("daily_sums::fake::day1::maximum", "0,0");
    Settings::set("daily_sums::fake::day1::mean", "0,0");
    Settings::set("daily_sums::fake::day2::minimum", "0,0");
    Settings::set("daily_sums::fake::day2::maximum", "0,0");
    Settings::set("daily_sums::fake::day2::mean", "0,0");
    REQUIRE(story, "fi", fun, "Sadesumma on tänään 0 millimetriä, huomenna sama.");
    REQUIRE(story, "sv", fun, "Nederbördssumman är i dag 0 millimeter, i morgon densamma.");
    REQUIRE(story, "en", fun, "Total precipitation is today 0 millimeters, tomorrow the same.");

    Settings::set("daily_sums::fake::day2::minimum", "5,0");
    Settings::set("daily_sums::fake::day2::maximum", "10,0");
    Settings::set("daily_sums::fake::day2::mean", "8,0");
    REQUIRE(story, "fi", fun, "Sadesumma on tänään 0 millimetriä, huomenna 5-10 millimetriä.");
    REQUIRE(story, "sv", fun, "Nederbördssumman är i dag 0 millimeter, i morgon 5-10 millimeter.");
    REQUIRE(
        story, "en", fun, "Total precipitation is today 0 millimeters, tomorrow 5-10 millimeters.");

    Settings::set("daily_sums::fake::day1::minimum", "0,0");
    Settings::set("daily_sums::fake::day1::maximum", "2,0");
    Settings::set("daily_sums::fake::day1::mean", "1,0");
    REQUIRE(story, "fi", fun, "Sadesumma on tänään noin 1 millimetriä, huomenna 5-10 millimetriä.");
    REQUIRE(story,
            "sv",
            fun,
            "Nederbördssumman är i dag cirka 1 millimeter, i morgon 5-10 millimeter.");
    REQUIRE(story,
            "en",
            fun,
            "Total precipitation is today about 1 millimeters, tomorrow 5-10 millimeters.");

    Settings::set("daily_sums::fake::day2::minimum", "0,0");
    Settings::set("daily_sums::fake::day2::maximum", "2,0");
    Settings::set("daily_sums::fake::day2::mean", "1,0");
    REQUIRE(story, "fi", fun, "Sadesumma on tänään noin 1 millimetriä, huomenna sama.");
    REQUIRE(story, "sv", fun, "Nederbördssumman är i dag cirka 1 millimeter, i morgon densamma.");
    REQUIRE(
        story, "en", fun, "Total precipitation is today about 1 millimeters, tomorrow the same.");

    Settings::set("daily_sums::fake::day2::minimum", "0,0");
    Settings::set("daily_sums::fake::day2::maximum", "2,0");
    Settings::set("daily_sums::fake::day2::mean", "1,0");
    REQUIRE(story, "fi", fun, "Sadesumma on tänään noin 1 millimetriä, huomenna sama.");
    REQUIRE(story, "sv", fun, "Nederbördssumman är i dag cirka 1 millimeter, i morgon densamma.");
    REQUIRE(
        story, "en", fun, "Total precipitation is today about 1 millimeters, tomorrow the same.");
  }

  // 2-day forecasts, zero rains ignored
  {
    Settings::set("daily_sums::ignore_fair_days", "true");
    WeatherPeriod period(time1, time2);

    PrecipitationStory story(time1, sources, area, period, "daily_sums");

    Settings::set("daily_sums::fake::day1::minimum", "0,0");
    Settings::set("daily_sums::fake::day1::maximum", "0,0");
    Settings::set("daily_sums::fake::day1::mean", "0,0");
    Settings::set("daily_sums::fake::day2::minimum", "0,0");
    Settings::set("daily_sums::fake::day2::maximum", "0,0");
    Settings::set("daily_sums::fake::day2::mean", "0,0");
    REQUIRE(story, "fi", fun, "");
    REQUIRE(story, "sv", fun, "");
    REQUIRE(story, "en", fun, "");

    Settings::set("daily_sums::fake::day2::minimum", "5,0");
    Settings::set("daily_sums::fake::day2::maximum", "10,0");
    Settings::set("daily_sums::fake::day2::mean", "8,0");
    REQUIRE(story, "fi", fun, "Sadesumma on huomenna 5-10 millimetriä.");
    REQUIRE(story, "sv", fun, "Nederbördssumman är i morgon 5-10 millimeter.");
    REQUIRE(story, "en", fun, "Total precipitation is tomorrow 5-10 millimeters.");

    Settings::set("daily_sums::fake::day1::minimum", "0,0");
    Settings::set("daily_sums::fake::day1::maximum", "2,0");
    Settings::set("daily_sums::fake::day1::mean", "1,0");
    REQUIRE(story, "fi", fun, "Sadesumma on tänään noin 1 millimetriä, huomenna 5-10 millimetriä.");
    REQUIRE(story,
            "sv",
            fun,
            "Nederbördssumman är i dag cirka 1 millimeter, i morgon 5-10 millimeter.");
    REQUIRE(story,
            "en",
            fun,
            "Total precipitation is today about 1 millimeters, tomorrow 5-10 millimeters.");

    Settings::set("daily_sums::fake::day2::minimum", "0,0");
    Settings::set("daily_sums::fake::day2::maximum", "2,0");
    Settings::set("daily_sums::fake::day2::mean", "1,0");
    REQUIRE(story, "fi", fun, "Sadesumma on tänään noin 1 millimetriä, huomenna sama.");
    REQUIRE(story, "sv", fun, "Nederbördssumman är i dag cirka 1 millimeter, i morgon densamma.");
    REQUIRE(
        story, "en", fun, "Total precipitation is today about 1 millimeters, tomorrow the same.");

    Settings::set("daily_sums::fake::day2::minimum", "0,0");
    Settings::set("daily_sums::fake::day2::maximum", "2,0");
    Settings::set("daily_sums::fake::day2::mean", "1,0");
    REQUIRE(story, "fi", fun, "Sadesumma on tänään noin 1 millimetriä, huomenna sama.");
    REQUIRE(story, "sv", fun, "Nederbördssumman är i dag cirka 1 millimeter, i morgon densamma.");
    REQUIRE(
        story, "en", fun, "Total precipitation is today about 1 millimeters, tomorrow the same.");
  }

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
    TEST(precipitation_total);
    TEST(precipitation_total_day);
    TEST(precipitation_range);
    TEST(precipitation_classification);
    TEST(precipitation_sums);
    TEST(precipitation_daily_sums);
    TEST(pop_twodays);
    TEST(pop_days);
    TEST(pop_max);
  }

};  // class tests

}  // namespace PrecipitationStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  Settings::set("textgen::database", "textgen2");

  using namespace PrecipitationStoryTest;

  cout << endl << "PrecipitationStory tests" << endl << "========================" << endl;

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::units::millimeters::format", "phrase");
  Settings::set(NFmiSettings::ToString());

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
