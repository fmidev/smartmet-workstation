/*
* NOTE: This file is to be edited in "Windows Latin-1" encoding (NOT UTF-8)
*/
#include <regression/tframe.h>

#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "WindStory.h"
#include "Story.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace WindStoryTest
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

#define REQUIRE(story, lang, name, expected)     \
  result = require(story, lang, name, expected); \
  if (!result.empty()) TEST_FAILED(result.c_str());

// ----------------------------------------------------------------------
/*!
 * \brief WindStory::daily_ranges
 */
// ----------------------------------------------------------------------

void wind_daily_ranges()
{
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  const WeatherArea area("25,60");

  const string fun = "wind_daily_ranges";

  const TextGenPosixTime time1(2003, 6, 1, 0, 0);
  const TextGenPosixTime time2(2003, 6, 2, 0, 0);
  const TextGenPosixTime time3(2003, 6, 3, 0, 0);
  const TextGenPosixTime time4(2003, 6, 4, 0, 0);

  string result;

  Settings::set("a::day::starthour", "6");
  Settings::set("a::day::endhour", "18");

  // Test 1-day forecasts
  {
    const WeatherPeriod period(time1, time2);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");

    Settings::set("a::fake::day1::direction::mean", "0,0");
    //
    REQUIRE(story, "fi", fun, "Pohjoistuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "Nordlig vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Northerly wind 0-2 m/s.");

    Settings::set("a::fake::day1::direction::mean", "0,30");
    //
    REQUIRE(story, "fi", fun, "Pohjoisenpuoleista tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "Vind omkring nord 0-2 m/s.");
    REQUIRE(story, "en", fun, "Mainly northerly wind 0-2 m/s.");

    Settings::set("a::fake::day1::direction::mean", "0,50");
    //
    REQUIRE(story, "fi", fun, "Suunnaltaan vaihtelevaa tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "Varierande vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Variable wind 0-2 m/s.");

    Settings::set("a::today::phrases", "today,tomorrow,weekday");

    Settings::set("a::fake::day1::direction::mean", "0,0");
    //
    REQUIRE(story, "fi", fun, "Tänään pohjoistuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I dag nordlig vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Today northerly wind 0-2 m/s.");
  }

  // Test 1-day forecast for tomorrow
  {
    const WeatherPeriod period(time2, time3);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day1::direction::mean", "0,0");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    //
    REQUIRE(story, "fi", fun, "Huomenna pohjoistuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I morgon nordlig vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Tomorrow northerly wind 0-2 m/s.");

    Settings::set("a::today::phrases", "today,tomorrow,weekday");
    //
    REQUIRE(story, "fi", fun, "Huomenna pohjoistuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I morgon nordlig vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Tomorrow northerly wind 0-2 m/s.");
  }

  // Test 1-day forecast for a later day
  {
    const WeatherPeriod period(time3, time4);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day1::direction::mean", "0,0");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    //
    REQUIRE(story, "fi", fun, "Tiistaina pohjoistuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "På tisdagen nordlig vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "On Tuesday northerly wind 0-2 m/s.");
  }

  // Test 2-day forecast starting today
  {
    const WeatherPeriod period(time1, time3);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    Settings::set("a::next_day::phrases", "tomorrow,following_day,weekday");
    Settings::set("a::days::phrases", "none,today,tomorrow,followingday,weekday");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::mean", "3,0");
    Settings::set("a::fake::day2::speed::minimum", "4,0");
    Settings::set("a::fake::day2::speed::maximum", "5,0");

    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    //
    REQUIRE(story, "fi", fun, "Pohjoistuulta 0-2 m/s, huomenna itätuulta 4-5 m/s.");
    REQUIRE(story, "sv", fun, "Nordlig vind 0-2 m/s, i morgon ostlig vind 4-5 m/s.");
    REQUIRE(story, "en", fun, "Northerly wind 0-2 m/s, tomorrow easterly wind 4-5 m/s.");

    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,30");
    //
    REQUIRE(story, "fi", fun, "Koillisenpuoleista tuulta 0-2 m/s, huomenna 4-5 m/s.");
    REQUIRE(story, "sv", fun, "Vind omkring nordost 0-2 m/s, i morgon 4-5 m/s.");
    REQUIRE(story, "en", fun, "Mainly north-easterly wind 0-2 m/s, tomorrow 4-5 m/s.");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::mean", "1,0");
    Settings::set("a::fake::day2::speed::minimum", "0,0");
    Settings::set("a::fake::day2::speed::maximum", "2,0");

    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    //
    REQUIRE(story, "fi", fun, "Pohjoistuulta 0-2 m/s, huomenna itätuulta.");
    REQUIRE(story, "sv", fun, "Nordlig vind 0-2 m/s, i morgon ostlig vind.");
    REQUIRE(story, "en", fun, "Northerly wind 0-2 m/s, tomorrow easterly wind.");

    Settings::set("a::days::phrases", "none,today,tomorrow,followingday,weekday");
    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,30");
    //
    REQUIRE(story, "fi", fun, "Koillisenpuoleista tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "Vind omkring nordost 0-2 m/s.");
    REQUIRE(story, "en", fun, "Mainly north-easterly wind 0-2 m/s.");

    Settings::set("a::days::phrases", "today,tomorrow,followingday,weekday,none");
    //
    REQUIRE(story, "fi", fun, "Tänään ja huomenna koillisenpuoleista tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I dag och i morgon vind omkring nordost 0-2 m/s.");
    REQUIRE(story, "en", fun, "Today and tomorrow mainly north-easterly wind 0-2 m/s.");

    Settings::set("a::days::phrases", "today,weekday,tomorrow,followingday,none");
    //
    REQUIRE(story, "fi", fun, "Tänään ja maanantaina koillisenpuoleista tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I dag och på måndagen vind omkring nordost 0-2 m/s.");
    REQUIRE(story, "en", fun, "Today and on Monday mainly north-easterly wind 0-2 m/s.");
  }

  // Test 2-day forecast starting tomorrow
  {
    const WeatherPeriod period(time2, time4);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    Settings::set("a::next_day::phrases", "tomorrow,following_day,weekday");
    Settings::set("a::days::phrases", "none,today,tomorrow,followingday,weekday");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::mean", "3,0");
    Settings::set("a::fake::day2::speed::minimum", "4,0");
    Settings::set("a::fake::day2::speed::maximum", "5,0");

    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    //
    REQUIRE(story, "fi", fun, "Huomenna pohjoistuulta 0-2 m/s, tiistaina itätuulta 4-5 m/s.");
    REQUIRE(story, "sv", fun, "I morgon nordlig vind 0-2 m/s, på tisdagen ostlig vind 4-5 m/s.");
    REQUIRE(story, "en", fun, "Tomorrow northerly wind 0-2 m/s, on Tuesday easterly wind 4-5 m/s.");

    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,30");
    //
    REQUIRE(story, "fi", fun, "Huomenna koillisenpuoleista tuulta 0-2 m/s, tiistaina 4-5 m/s.");
    REQUIRE(story, "sv", fun, "I morgon vind omkring nordost 0-2 m/s, på tisdagen 4-5 m/s.");
    REQUIRE(story, "en", fun, "Tomorrow mainly north-easterly wind 0-2 m/s, on Tuesday 4-5 m/s.");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::mean", "1,0");
    Settings::set("a::fake::day2::speed::minimum", "0,0");
    Settings::set("a::fake::day2::speed::maximum", "2,0");

    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    //
    REQUIRE(story, "fi", fun, "Huomenna pohjoistuulta 0-2 m/s, tiistaina itätuulta.");
    REQUIRE(story, "sv", fun, "I morgon nordlig vind 0-2 m/s, på tisdagen ostlig vind.");
    REQUIRE(story, "en", fun, "Tomorrow northerly wind 0-2 m/s, on Tuesday easterly wind.");

    Settings::set("a::days::phrases", "none,today,tomorrow,followingday,weekday");
    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,30");
    //
    REQUIRE(story, "fi", fun, "Huomenna ja seuraavana päivänä koillisenpuoleista tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I morgon och följande dag vind omkring nordost 0-2 m/s.");
    REQUIRE(story, "en", fun, "Tomorrow and the following day mainly north-easterly wind 0-2 m/s.");

    Settings::set("a::days::phrases", "today,tomorrow,followingday,weekday,none");
    //
    REQUIRE(story, "fi", fun, "Huomenna ja seuraavana päivänä koillisenpuoleista tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I morgon och följande dag vind omkring nordost 0-2 m/s.");
    REQUIRE(story, "en", fun, "Tomorrow and the following day mainly north-easterly wind 0-2 m/s.");

    Settings::set("a::days::phrases", "weekday");
    //
    REQUIRE(story, "fi", fun, "Maanantaina ja tiistaina koillisenpuoleista tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "På måndagen och på tisdagen vind omkring nordost 0-2 m/s.");
    REQUIRE(story, "en", fun, "On Monday and on Tuesday mainly north-easterly wind 0-2 m/s.");
  }

  // Test 3-day forecast starting today
  {
    const WeatherPeriod period(time1, time4);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    Settings::set("a::next_day::phrases", "tomorrow,following_day,weekday");
    Settings::set("a::days::phrases", "none,today,tomorrow,followingday,weekday");

    // W1<>W2<>W3

    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::minimum", "3,0");
    Settings::set("a::fake::day2::speed::mean", "4,0");
    Settings::set("a::fake::day2::speed::maximum", "5,0");
    Settings::set("a::fake::day3::speed::minimum", "6,0");
    Settings::set("a::fake::day3::speed::mean", "7,0");
    Settings::set("a::fake::day3::speed::maximum", "8,0");

    // D1<>D2<>D3
    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::day3::direction::mean", "180,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    //
    REQUIRE(story,
            "fi",
            fun,
            "Pohjoistuulta 0-2 m/s, huomenna itätuulta 3-5 m/s, tiistaina etelätuulta 6-8 m/s.");
    REQUIRE(story,
            "sv",
            fun,
            "Nordlig vind 0-2 m/s, i morgon ostlig vind 3-5 m/s, på tisdagen sydlig vind 6-8 m/s.");
    REQUIRE(story,
            "en",
            fun,
            "Northerly wind 0-2 m/s, tomorrow easterly wind 3-5 m/s, on Tuesday southerly wind 6-8 "
            "m/s.");

    // D1==D2<>D3
    Settings::set("a::fake::days1-2::direction::mean", "45,30");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    //
    REQUIRE(story,
            "fi",
            fun,
            "Koillisenpuoleista tuulta 0-2 m/s, huomenna 3-5 m/s, tiistaina etelätuulta 6-8 m/s.");
    REQUIRE(story,
            "sv",
            fun,
            "Vind omkring nordost 0-2 m/s, i morgon 3-5 m/s, på tisdagen sydlig vind 6-8 m/s.");
    REQUIRE(
        story,
        "en",
        fun,
        "Mainly north-easterly wind 0-2 m/s, tomorrow 3-5 m/s, on Tuesday southerly wind 6-8 m/s.");

    // D1<>D2==D3
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,30");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    //
    REQUIRE(story,
            "fi",
            fun,
            "Pohjoistuulta 0-2 m/s, huomenna kaakonpuoleista tuulta 3-5 m/s, tiistaina 6-8 m/s.");
    REQUIRE(story,
            "sv",
            fun,
            "Nordlig vind 0-2 m/s, i morgon vind omkring sydost 3-5 m/s, på tisdagen 6-8 m/s.");
    REQUIRE(
        story,
        "en",
        fun,
        "Northerly wind 0-2 m/s, tomorrow mainly south-easterly wind 3-5 m/s, on Tuesday 6-8 m/s.");

    // D1==D2==D3
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,30");
    //
    REQUIRE(story, "fi", fun, "Idänpuoleista tuulta 0-2 m/s, huomenna 3-5 m/s, tiistaina 6-8 m/s.");
    REQUIRE(story, "sv", fun, "Vind omkring ost 0-2 m/s, i morgon 3-5 m/s, på tisdagen 6-8 m/s.");
    REQUIRE(
        story, "en", fun, "Mainly easterly wind 0-2 m/s, tomorrow 3-5 m/s, on Tuesday 6-8 m/s.");

    // W1==W2<>W3

    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::minimum", "0,0");
    Settings::set("a::fake::day2::speed::mean", "1,0");
    Settings::set("a::fake::day2::speed::maximum", "2,0");
    Settings::set("a::fake::day3::speed::minimum", "6,0");
    Settings::set("a::fake::day3::speed::mean", "7,0");
    Settings::set("a::fake::day3::speed::maximum", "8,0");

    // D1<>D2<>D3
    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::day3::direction::mean", "180,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    //
    REQUIRE(story,
            "fi",
            fun,
            "Pohjoistuulta 0-2 m/s, huomenna itätuulta, tiistaina etelätuulta 6-8 m/s.");
    REQUIRE(story,
            "sv",
            fun,
            "Nordlig vind 0-2 m/s, i morgon ostlig vind, på tisdagen sydlig vind 6-8 m/s.");
    REQUIRE(story,
            "en",
            fun,
            "Northerly wind 0-2 m/s, tomorrow easterly wind, on Tuesday southerly wind 6-8 m/s.");

    // D1==D2<>D3
    Settings::set("a::fake::days1-2::direction::mean", "45,30");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    REQUIRE(story, "fi", fun, "Koillisenpuoleista tuulta 0-2 m/s, tiistaina etelätuulta 6-8 m/s.");
    REQUIRE(story, "sv", fun, "Vind omkring nordost 0-2 m/s, på tisdagen sydlig vind 6-8 m/s.");
    REQUIRE(
        story, "en", fun, "Mainly north-easterly wind 0-2 m/s, on Tuesday southerly wind 6-8 m/s.");

    // D1<>D2==D3
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,30");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    REQUIRE(story,
            "fi",
            fun,
            "Pohjoistuulta 0-2 m/s, huomenna kaakonpuoleista tuulta, tiistaina 6-8 m/s.");
    REQUIRE(story,
            "sv",
            fun,
            "Nordlig vind 0-2 m/s, i morgon vind omkring sydost, på tisdagen 6-8 m/s.");
    REQUIRE(story,
            "en",
            fun,
            "Northerly wind 0-2 m/s, tomorrow mainly south-easterly wind, on Tuesday 6-8 m/s.");

    // W1<>W2==W3

    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::minimum", "3,0");
    Settings::set("a::fake::day2::speed::mean", "4,0");
    Settings::set("a::fake::day2::speed::maximum", "5,0");
    Settings::set("a::fake::day3::speed::minimum", "3,0");
    Settings::set("a::fake::day3::speed::mean", "4,0");
    Settings::set("a::fake::day3::speed::maximum", "5,0");

    // D1<>D2<>D3
    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::day3::direction::mean", "180,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    REQUIRE(story,
            "fi",
            fun,
            "Pohjoistuulta 0-2 m/s, huomenna itätuulta 3-5 m/s, tiistaina etelätuulta.");
    REQUIRE(story,
            "sv",
            fun,
            "Nordlig vind 0-2 m/s, i morgon ostlig vind 3-5 m/s, på tisdagen sydlig vind.");
    REQUIRE(story,
            "en",
            fun,
            "Northerly wind 0-2 m/s, tomorrow easterly wind 3-5 m/s, on Tuesday southerly wind.");

    // D1==D2<>D3
    Settings::set("a::fake::days1-2::direction::mean", "45,30");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    REQUIRE(story,
            "fi",
            fun,
            "Koillisenpuoleista tuulta 0-2 m/s, huomenna 3-5 m/s, tiistaina etelätuulta.");
    REQUIRE(story,
            "sv",
            fun,
            "Vind omkring nordost 0-2 m/s, i morgon 3-5 m/s, på tisdagen sydlig vind.");
    REQUIRE(story,
            "en",
            fun,
            "Mainly north-easterly wind 0-2 m/s, tomorrow 3-5 m/s, on Tuesday southerly wind.");

    // D1<>D2==D3
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,30");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    REQUIRE(story,
            "fi",
            fun,
            "Pohjoistuulta 0-2 m/s, maanantaista alkaen kaakonpuoleista tuulta 3-5 m/s.");
    REQUIRE(story,
            "sv",
            fun,
            "Nordlig vind 0-2 m/s, från och med måndagen vind omkring sydost 3-5 m/s.");
    REQUIRE(story,
            "en",
            fun,
            "Northerly wind 0-2 m/s, from Monday onwards mainly south-easterly wind 3-5 m/s.");

    // D1==D2==D3
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,30");
    REQUIRE(story, "fi", fun, "Idänpuoleista tuulta 0-2 m/s, maanantaista alkaen 3-5 m/s.");
    REQUIRE(story, "sv", fun, "Vind omkring ost 0-2 m/s, från och med måndagen 3-5 m/s.");
    REQUIRE(story, "en", fun, "Mainly easterly wind 0-2 m/s, from Monday onwards 3-5 m/s.");

    // W1==W2==W3

    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::minimum", "0,0");
    Settings::set("a::fake::day2::speed::mean", "1,0");
    Settings::set("a::fake::day2::speed::maximum", "2,0");
    Settings::set("a::fake::day3::speed::minimum", "0,0");
    Settings::set("a::fake::day3::speed::mean", "1,0");
    Settings::set("a::fake::day3::speed::maximum", "2,0");

    // D1<>D2<>D3
    Settings::set("a::fake::day1::direction::mean", "0,0");
    Settings::set("a::fake::day2::direction::mean", "90,0");
    Settings::set("a::fake::day3::direction::mean", "180,0");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    REQUIRE(story, "fi", fun, "Pohjoistuulta 0-2 m/s, huomenna itätuulta, tiistaina etelätuulta.");
    REQUIRE(
        story, "sv", fun, "Nordlig vind 0-2 m/s, i morgon ostlig vind, på tisdagen sydlig vind.");
    REQUIRE(story,
            "en",
            fun,
            "Northerly wind 0-2 m/s, tomorrow easterly wind, on Tuesday southerly wind.");

    // D1==D2<>D3
    Settings::set("a::fake::days1-2::direction::mean", "45,30");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    REQUIRE(story, "fi", fun, "Koillisenpuoleista tuulta 0-2 m/s, tiistaina etelätuulta.");
    REQUIRE(story, "sv", fun, "Vind omkring nordost 0-2 m/s, på tisdagen sydlig vind.");
    REQUIRE(story, "en", fun, "Mainly north-easterly wind 0-2 m/s, on Tuesday southerly wind.");

    // D1<>D2==D3
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,30");
    Settings::set("a::fake::days1-3::direction::mean", "90,90");
    REQUIRE(story, "fi", fun, "Pohjoistuulta 0-2 m/s, maanantaista alkaen kaakonpuoleista tuulta.");
    REQUIRE(story, "sv", fun, "Nordlig vind 0-2 m/s, från och med måndagen vind omkring sydost.");
    REQUIRE(story,
            "en",
            fun,
            "Northerly wind 0-2 m/s, from Monday onwards mainly south-easterly wind.");

    // D1==D2==D3
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,30");
    REQUIRE(story, "fi", fun, "Idänpuoleista tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "Vind omkring ost 0-2 m/s.");
    REQUIRE(story, "en", fun, "Mainly easterly wind 0-2 m/s.");
  }

  // SAME THING ALL OVER AGAIN, BUT WITH VARIABLE WINDS

  // Test 1-day forecasts
  {
    const WeatherPeriod period(time1, time2);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");

    Settings::set("a::fake::day1::direction::mean", "0,50");
    REQUIRE(story, "fi", fun, "Suunnaltaan vaihtelevaa tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "Varierande vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Variable wind 0-2 m/s.");

    Settings::set("a::today::phrases", "today,tomorrow,weekday");

    Settings::set("a::fake::day1::direction::mean", "0,50");
    REQUIRE(story, "fi", fun, "Tänään suunnaltaan vaihtelevaa tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I dag varierande vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Today variable wind 0-2 m/s.");
  }

  // Test 1-day forecast for tomorrow

  {
    const WeatherPeriod period(time2, time3);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day1::direction::mean", "0,50");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    REQUIRE(story, "fi", fun, "Huomenna suunnaltaan vaihtelevaa tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I morgon varierande vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Tomorrow variable wind 0-2 m/s.");

    Settings::set("a::today::phrases", "today,tomorrow,weekday");
    REQUIRE(story, "fi", fun, "Huomenna suunnaltaan vaihtelevaa tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "I morgon varierande vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Tomorrow variable wind 0-2 m/s.");
  }

  // Test 1-day forecast for a later day

  {
    const WeatherPeriod period(time3, time4);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day1::direction::mean", "0,50");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    REQUIRE(story, "fi", fun, "Tiistaina suunnaltaan vaihtelevaa tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "På tisdagen varierande vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "On Tuesday variable wind 0-2 m/s.");
  }

  // Test 2-day forecast starting today

  {
    const WeatherPeriod period(time1, time3);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    Settings::set("a::next_day::phrases", "tomorrow,following_day,weekday");
    Settings::set("a::days::phrases", "none,today,tomorrow,followingday,weekday");

    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::mean", "3,0");
    Settings::set("a::fake::day2::speed::minimum", "4,0");
    Settings::set("a::fake::day2::speed::maximum", "5,0");

    Settings::set("a::fake::day1::direction::mean", "0,50");
    Settings::set("a::fake::day2::direction::mean", "90,50");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    REQUIRE(story, "fi", fun, "Suunnaltaan vaihtelevaa tuulta 0-2 m/s, huomenna 4-5 m/s.");
    REQUIRE(story, "sv", fun, "Varierande vind 0-2 m/s, i morgon 4-5 m/s.");
    REQUIRE(story, "en", fun, "Variable wind 0-2 m/s, tomorrow 4-5 m/s.");
  }

  // Test 3-day forecast starting today

  {
    const WeatherPeriod period(time1, time4);
    WindStory story(time1, sources, area, period, "a");

    Settings::set("a::today::phrases", "none,today,tomorrow,weekday");
    Settings::set("a::next_day::phrases", "tomorrow,following_day,weekday");
    Settings::set("a::days::phrases", "none,today,tomorrow,followingday,weekday");

    // W1<>W2<>W3

    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::minimum", "3,0");
    Settings::set("a::fake::day2::speed::mean", "4,0");
    Settings::set("a::fake::day2::speed::maximum", "5,0");
    Settings::set("a::fake::day3::speed::minimum", "6,0");
    Settings::set("a::fake::day3::speed::mean", "7,0");
    Settings::set("a::fake::day3::speed::maximum", "8,0");

    Settings::set("a::fake::day1::direction::mean", "0,50");
    Settings::set("a::fake::day2::direction::mean", "90,50");
    Settings::set("a::fake::day3::direction::mean", "180,50");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,50");
    REQUIRE(story,
            "fi",
            fun,
            "Suunnaltaan vaihtelevaa tuulta 0-2 m/s, huomenna 3-5 m/s, tiistaina 6-8 m/s.");
    REQUIRE(story, "sv", fun, "Varierande vind 0-2 m/s, i morgon 3-5 m/s, på tisdagen 6-8 m/s.");
    REQUIRE(story, "en", fun, "Variable wind 0-2 m/s, tomorrow 3-5 m/s, on Tuesday 6-8 m/s.");

    // W1==W2<>W3

    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::minimum", "0,0");
    Settings::set("a::fake::day2::speed::mean", "1,0");
    Settings::set("a::fake::day2::speed::maximum", "2,0");
    Settings::set("a::fake::day3::speed::minimum", "6,0");
    Settings::set("a::fake::day3::speed::mean", "7,0");
    Settings::set("a::fake::day3::speed::maximum", "8,0");

    Settings::set("a::fake::day1::direction::mean", "0,50");
    Settings::set("a::fake::day2::direction::mean", "90,50");
    Settings::set("a::fake::day3::direction::mean", "180,50");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,50");
    REQUIRE(story, "fi", fun, "Suunnaltaan vaihtelevaa tuulta 0-2 m/s, tiistaina 6-8 m/s.");
    REQUIRE(story, "sv", fun, "Varierande vind 0-2 m/s, på tisdagen 6-8 m/s.");
    REQUIRE(story, "en", fun, "Variable wind 0-2 m/s, on Tuesday 6-8 m/s.");

    // W1<>W2==W3

    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::minimum", "3,0");
    Settings::set("a::fake::day2::speed::mean", "4,0");
    Settings::set("a::fake::day2::speed::maximum", "5,0");
    Settings::set("a::fake::day3::speed::minimum", "3,0");
    Settings::set("a::fake::day3::speed::mean", "4,0");
    Settings::set("a::fake::day3::speed::maximum", "5,0");

    Settings::set("a::fake::day1::direction::mean", "0,50");
    Settings::set("a::fake::day2::direction::mean", "90,50");
    Settings::set("a::fake::day3::direction::mean", "180,50");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,50");

    REQUIRE(
        story, "fi", fun, "Suunnaltaan vaihtelevaa tuulta 0-2 m/s, maanantaista alkaen 3-5 m/s.");
    REQUIRE(story, "sv", fun, "Varierande vind 0-2 m/s, från och med måndagen 3-5 m/s.");
    REQUIRE(story, "en", fun, "Variable wind 0-2 m/s, from Monday onwards 3-5 m/s.");

    // W1==W2==W3

    Settings::set("a::fake::day1::speed::minimum", "0,0");
    Settings::set("a::fake::day1::speed::mean", "1,0");
    Settings::set("a::fake::day1::speed::maximum", "2,0");
    Settings::set("a::fake::day2::speed::minimum", "0,0");
    Settings::set("a::fake::day2::speed::mean", "1,0");
    Settings::set("a::fake::day2::speed::maximum", "2,0");
    Settings::set("a::fake::day3::speed::minimum", "0,0");
    Settings::set("a::fake::day3::speed::mean", "1,0");
    Settings::set("a::fake::day3::speed::maximum", "2,0");

    Settings::set("a::fake::day1::direction::mean", "0,50");
    Settings::set("a::fake::day2::direction::mean", "90,50");
    Settings::set("a::fake::day3::direction::mean", "180,50");
    Settings::set("a::fake::days1-2::direction::mean", "45,50");
    Settings::set("a::fake::days2-3::direction::mean", "135,50");
    Settings::set("a::fake::days1-3::direction::mean", "90,50");

    REQUIRE(story, "fi", fun, "Suunnaltaan vaihtelevaa tuulta 0-2 m/s.");
    REQUIRE(story, "sv", fun, "Varierande vind 0-2 m/s.");
    REQUIRE(story, "en", fun, "Variable wind 0-2 m/s.");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief WindStory::range
 */
// ----------------------------------------------------------------------

void wind_range()
{
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  const WeatherArea area("25,60");

  const string fun = "wind_range";

  const TextGenPosixTime time1(2003, 6, 1, 0, 0);
  const TextGenPosixTime time2(2003, 6, 2, 0, 0);

  string result;

  Settings::set("a::day::starthour", "6");
  Settings::set("a::day::endhour", "18");

  const WeatherPeriod period(time1, time2);
  WindStory story(time1, sources, area, period, "a");

  Settings::set("a::fake::speed::mean", "1,0");
  Settings::set("a::fake::speed::minimum", "0,0");
  Settings::set("a::fake::speed::maximum", "2,0");

  Settings::set("a::fake::direction::mean", "0,0");
  REQUIRE(story, "fi", fun, "Pohjoistuulta 0-2 m/s.");
  REQUIRE(story, "sv", fun, "Nordlig vind 0-2 m/s.");
  REQUIRE(story, "en", fun, "Northerly wind 0-2 m/s.");

  Settings::set("a::fake::direction::mean", "0,30");
  REQUIRE(story, "fi", fun, "Pohjoisenpuoleista tuulta 0-2 m/s.");
  REQUIRE(story, "sv", fun, "Vind omkring nord 0-2 m/s.");
  REQUIRE(story, "en", fun, "Mainly northerly wind 0-2 m/s.");

  Settings::set("a::fake::direction::mean", "0,50");
  REQUIRE(story, "fi", fun, "Suunnaltaan vaihtelevaa tuulta 0-2 m/s.");
  REQUIRE(story, "sv", fun, "Varierande vind 0-2 m/s.");
  REQUIRE(story, "en", fun, "Variable wind 0-2 m/s.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WindStory::simple_overview
 */
// ----------------------------------------------------------------------

void wind_simple_overview() { TEST_NOT_IMPLEMENTED(); }
// ----------------------------------------------------------------------
/*!
 * \brief Test WindStory::overview
 */
// ----------------------------------------------------------------------

void wind_overview() { TEST_NOT_IMPLEMENTED(); }
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
    TEST(wind_daily_ranges);
    TEST(wind_simple_overview);
    TEST(wind_overview);
    TEST(wind_range);
  }

};  // class tests

}  // namespace WindStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace WindStoryTest;

  cout << endl << "WindStory tests" << endl << "===============" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
