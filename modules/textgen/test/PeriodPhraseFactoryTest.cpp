#include <regression/tframe.h>
#include "PeriodPhraseFactory.h"
#include "DictionaryFactory.h"
#include "PlainTextFormatter.h"
#include "Sentence.h"
#include <calculator/WeatherPeriod.h>
#include <calculator/TextGenPosixTime.h>
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace PeriodPhraseFactoryTest
{
shared_ptr<TextGen::Dictionary> dict;
TextGen::PlainTextFormatter formatter;

string require(const string& theMode,
               const string& theVariable,
               const TextGenPosixTime& theForecastTime,
               const TextGen::WeatherPeriod& thePeriod,
               const string& theLanguage,
               const string& theResult)
{
  using namespace TextGen;

  dict->init(theLanguage);
  formatter.dictionary(dict);

  const Sentence sentence =
      PeriodPhraseFactory::create(theMode, theVariable, theForecastTime, thePeriod);

  const string result = sentence.realize(formatter);
  if (result != theResult)
    return (result + " <> " + theResult);
  else
    return "";
}

#define REQUIRE(mode, var, time, period, lang, expected)     \
  result = require(mode, var, time, period, lang, expected); \
  if (!result.empty()) TEST_FAILED(result.c_str());

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("until_tonight");
 */
// ----------------------------------------------------------------------

void until_tonight()
{
  const TextGenPosixTime day1(2003, 06, 29);
  const TextGenPosixTime day2(2003, 06, 30);
  const TextGenPosixTime day3(2003, 07, 01);

  const string mode = "until_tonight";
  const string var = mode;
  const string phrasesvar = var + "::" + mode + "::phrases";

  const TextGenPosixTime ftime(day1);
  const TextGen::WeatherPeriod period1(day1, day2);
  const TextGen::WeatherPeriod period2(day2, day3);

  string result;

  REQUIRE(mode, var, ftime, period1, "fi", "");
  REQUIRE(mode, var, ftime, period1, "sv", "");
  REQUIRE(mode, var, ftime, period1, "en", "");

  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaina.");
  REQUIRE(mode, var, ftime, period2, "sv", "På måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Monday.");

  Settings::set(phrasesvar, "weekday");
  REQUIRE(mode, var, ftime, period1, "fi", "Sunnuntaina.");
  REQUIRE(mode, var, ftime, period1, "sv", "På söndagen.");
  REQUIRE(mode, var, ftime, period1, "en", "On Sunday.");

  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaina.");
  REQUIRE(mode, var, ftime, period2, "sv", "På måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Monday.");

  Settings::set(phrasesvar, "today");
  REQUIRE(mode, var, ftime, period1, "fi", "Tänään.");
  REQUIRE(mode, var, ftime, period1, "sv", "I dag.");
  REQUIRE(mode, var, ftime, period1, "en", "Today.");

  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaina.");
  REQUIRE(mode, var, ftime, period2, "sv", "På måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Monday.");

  Settings::set(phrasesvar, "atday");
  REQUIRE(mode, var, ftime, period1, "fi", "Päivällä.");
  REQUIRE(mode, var, ftime, period1, "sv", "På dagen.");
  REQUIRE(mode, var, ftime, period1, "en", "During the day.");

  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaina.");
  REQUIRE(mode, var, ftime, period2, "sv", "På måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Monday.");

  Settings::set(phrasesvar, "none");
  REQUIRE(mode, var, ftime, period1, "fi", "");
  REQUIRE(mode, var, ftime, period1, "sv", "");
  REQUIRE(mode, var, ftime, period1, "en", "");

  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaina.");
  REQUIRE(mode, var, ftime, period2, "sv", "På måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Monday.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("until_morning");
 */
// ----------------------------------------------------------------------

void until_morning()
{
  const TextGenPosixTime day1(2003, 06, 29);
  const TextGenPosixTime day2(2003, 06, 30);
  const TextGenPosixTime day3(2003, 07, 01);

  const string mode = "until_morning";
  const string var = mode;
  const string phrasesvar = var + "::" + mode + "::phrases";

  const TextGenPosixTime ftime(day1);
  const TextGen::WeatherPeriod period1(day1, day2);
  const TextGen::WeatherPeriod period2(day2, day3);

  string result;

  REQUIRE(mode, var, ftime, period1, "fi", "");
  REQUIRE(mode, var, ftime, period1, "sv", "");
  REQUIRE(mode, var, ftime, period1, "en", "");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  Settings::set(phrasesvar, "weekday");
  REQUIRE(mode, var, ftime, period1, "fi", "Maanantain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period1, "sv", "Natten mot måndagen.");
  REQUIRE(mode, var, ftime, period1, "en", "On Monday night.");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  Settings::set(phrasesvar, "tonight");
  REQUIRE(mode, var, ftime, period1, "fi", "Ensi yönä.");
  REQUIRE(mode, var, ftime, period1, "sv", "I natt.");
  REQUIRE(mode, var, ftime, period1, "en", "At night.");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  Settings::set(phrasesvar, "atnight");
  REQUIRE(mode, var, ftime, period1, "fi", "Yöllä.");
  REQUIRE(mode, var, ftime, period1, "sv", "På natten.");
  REQUIRE(mode, var, ftime, period1, "en", "At night.");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  Settings::set(phrasesvar, "none");
  REQUIRE(mode, var, ftime, period1, "fi", "");
  REQUIRE(mode, var, ftime, period1, "sv", "");
  REQUIRE(mode, var, ftime, period1, "en", "");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("today");
 */
// ----------------------------------------------------------------------

void today()
{
  const TextGenPosixTime day1(2003, 06, 29);
  const TextGenPosixTime day2(2003, 06, 30);
  const TextGenPosixTime day3(2003, 07, 01);

  const string mode = "today";
  const string var = mode;
  const string phrasesvar = var + "::" + mode + "::phrases";

  const TextGenPosixTime ftime(day1);
  const TextGen::WeatherPeriod period1(day1, day2);
  const TextGen::WeatherPeriod period2(day2, day3);

  string result;

  REQUIRE(mode, var, ftime, period1, "fi", "");
  REQUIRE(mode, var, ftime, period1, "sv", "");
  REQUIRE(mode, var, ftime, period1, "en", "");

  REQUIRE(mode, var, ftime, period2, "fi", "Huomenna.");
  REQUIRE(mode, var, ftime, period2, "sv", "I morgon.");
  REQUIRE(mode, var, ftime, period2, "en", "Tomorrow.");

  Settings::set(phrasesvar, "today");
  REQUIRE(mode, var, ftime, period1, "fi", "Tänään.");
  REQUIRE(mode, var, ftime, period1, "sv", "I dag.");
  REQUIRE(mode, var, ftime, period1, "en", "Today.");

  REQUIRE(mode, var, ftime, period2, "fi", "Huomenna.");
  REQUIRE(mode, var, ftime, period2, "sv", "I morgon.");
  REQUIRE(mode, var, ftime, period2, "en", "Tomorrow.");

  Settings::set(phrasesvar, "atday");
  REQUIRE(mode, var, ftime, period1, "fi", "Päivällä.");
  REQUIRE(mode, var, ftime, period1, "sv", "På dagen.");
  REQUIRE(mode, var, ftime, period1, "en", "During the day.");

  Settings::set(phrasesvar, "weekday");
  REQUIRE(mode, var, ftime, period1, "fi", "Sunnuntaina.");
  REQUIRE(mode, var, ftime, period1, "sv", "På söndagen.");
  REQUIRE(mode, var, ftime, period1, "en", "On Sunday.");

  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaina.");
  REQUIRE(mode, var, ftime, period2, "sv", "På måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Monday.");

  Settings::set(phrasesvar, "tomorrow");
  REQUIRE(mode, var, ftime, period1, "fi", "");
  REQUIRE(mode, var, ftime, period1, "sv", "");
  REQUIRE(mode, var, ftime, period1, "en", "");

  REQUIRE(mode, var, ftime, period2, "fi", "Huomenna.");
  REQUIRE(mode, var, ftime, period2, "sv", "I morgon.");
  REQUIRE(mode, var, ftime, period2, "en", "Tomorrow.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("tonight");
 */
// ----------------------------------------------------------------------

void tonight()
{
  const TextGenPosixTime day1(2003, 06, 29, 18);
  const TextGenPosixTime day2(2003, 06, 30, 06);
  const TextGenPosixTime day3(2003, 07, 01, 06);

  const string mode = "tonight";
  const string var = mode;
  const string phrasesvar = var + "::" + mode + "::phrases";

  const TextGenPosixTime ftime(day1);
  const TextGen::WeatherPeriod period1(day1, day2);
  const TextGen::WeatherPeriod period2(day2, day3);

  string result;

  REQUIRE(mode, var, ftime, period1, "fi", "");
  REQUIRE(mode, var, ftime, period1, "sv", "");
  REQUIRE(mode, var, ftime, period1, "en", "");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  Settings::set(phrasesvar, "tonight");
  REQUIRE(mode, var, ftime, period1, "fi", "Ensi yönä.");
  REQUIRE(mode, var, ftime, period1, "sv", "I natt.");
  REQUIRE(mode, var, ftime, period1, "en", "At night.");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  Settings::set(phrasesvar, "atnight");
  REQUIRE(mode, var, ftime, period1, "fi", "Yöllä.");
  REQUIRE(mode, var, ftime, period1, "sv", "På natten.");
  REQUIRE(mode, var, ftime, period1, "en", "At night.");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  Settings::set(phrasesvar, "weekday");
  REQUIRE(mode, var, ftime, period1, "fi", "Maanantain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period1, "sv", "Natten mot måndagen.");
  REQUIRE(mode, var, ftime, period1, "en", "On Monday night.");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  Settings::set(phrasesvar, "none");
  REQUIRE(mode, var, ftime, period1, "fi", "");
  REQUIRE(mode, var, ftime, period1, "sv", "");
  REQUIRE(mode, var, ftime, period1, "en", "");

  REQUIRE(mode, var, ftime, period2, "fi", "Tiistain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Natten mot tisdagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Tuesday night.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("next_night");
 */
// ----------------------------------------------------------------------

void next_night()
{
  const TextGenPosixTime day1(2003, 06, 29);
  const TextGenPosixTime day2(2003, 06, 30);

  const string mode = "next_night";
  const string var = mode;
  const string phrasesvar = var + "::" + mode + "::phrases";

  const TextGenPosixTime ftime(day1);
  const TextGen::WeatherPeriod period(day1, day2);

  string result;

  REQUIRE(mode, var, ftime, period, "fi", "Ensi yönä.");
  REQUIRE(mode, var, ftime, period, "sv", "I natt.");
  REQUIRE(mode, var, ftime, period, "en", "At night.");

  Settings::set(phrasesvar, "weekday");
  REQUIRE(mode, var, ftime, period, "fi", "Maanantain vastaisena yönä.");
  REQUIRE(mode, var, ftime, period, "sv", "Natten mot måndagen.");
  REQUIRE(mode, var, ftime, period, "en", "On Monday night.");

  Settings::set(phrasesvar, "followingnight");
  REQUIRE(mode, var, ftime, period, "fi", "Seuraavana yönä.");
  REQUIRE(mode, var, ftime, period, "sv", "Följande natt.");
  REQUIRE(mode, var, ftime, period, "en", "The following night.");

  Settings::set(phrasesvar, "tonight");
  REQUIRE(mode, var, ftime, period, "fi", "Ensi yönä.");
  REQUIRE(mode, var, ftime, period, "sv", "I natt.");
  REQUIRE(mode, var, ftime, period, "en", "At night.");

  Settings::set(phrasesvar, "atnight");
  REQUIRE(mode, var, ftime, period, "fi", "Yöllä.");
  REQUIRE(mode, var, ftime, period, "sv", "På natten.");
  REQUIRE(mode, var, ftime, period, "en", "At night.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("next_day");
 */
// ----------------------------------------------------------------------

void next_day()
{
  const TextGenPosixTime day1(2003, 06, 29);
  const TextGenPosixTime day2(2003, 06, 30);
  const TextGenPosixTime day3(2003, 07, 01);

  const string mode = "next_day";
  const string var = mode;
  const string phrasesvar = var + "::" + mode + "::phrases";

  const TextGenPosixTime ftime(day1);
  const TextGen::WeatherPeriod period1(day1, day2);
  const TextGen::WeatherPeriod period2(day2, day3);

  string result;

  REQUIRE(mode, var, ftime, period1, "fi", "Seuraavana päivänä.");
  REQUIRE(mode, var, ftime, period1, "sv", "Följande dag.");
  REQUIRE(mode, var, ftime, period1, "en", "The following day.");

  REQUIRE(mode, var, ftime, period2, "fi", "Huomenna.");
  REQUIRE(mode, var, ftime, period2, "sv", "I morgon.");
  REQUIRE(mode, var, ftime, period2, "en", "Tomorrow.");

  Settings::set(phrasesvar, "weekday");
  REQUIRE(mode, var, ftime, period1, "fi", "Sunnuntaina.");
  REQUIRE(mode, var, ftime, period1, "sv", "På söndagen.");
  REQUIRE(mode, var, ftime, period1, "en", "On Sunday.");

  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaina.");
  REQUIRE(mode, var, ftime, period2, "sv", "På måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "On Monday.");

  Settings::set(phrasesvar, "followingday");
  REQUIRE(mode, var, ftime, period1, "fi", "Seuraavana päivänä.");
  REQUIRE(mode, var, ftime, period1, "sv", "Följande dag.");
  REQUIRE(mode, var, ftime, period1, "en", "The following day.");

  REQUIRE(mode, var, ftime, period2, "fi", "Seuraavana päivänä.");
  REQUIRE(mode, var, ftime, period2, "sv", "Följande dag.");
  REQUIRE(mode, var, ftime, period2, "en", "The following day.");

  Settings::set(phrasesvar, "tomorrow");
  REQUIRE(mode, var, ftime, period1, "fi", "Seuraavana päivänä.");
  REQUIRE(mode, var, ftime, period1, "sv", "Följande dag.");
  REQUIRE(mode, var, ftime, period1, "en", "The following day.");

  REQUIRE(mode, var, ftime, period2, "fi", "Huomenna.");
  REQUIRE(mode, var, ftime, period2, "sv", "I morgon.");
  REQUIRE(mode, var, ftime, period2, "en", "Tomorrow.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("next_days");
 */
// ----------------------------------------------------------------------

void next_days()
{
  const TextGenPosixTime day1(2003, 06, 29);
  const TextGenPosixTime day2(2003, 06, 30);
  const TextGenPosixTime day3(2003, 07, 01);

  const string mode = "next_days";
  const string var = mode;
  const string phrasesvar = var + "::" + mode + "::phrases";

  const TextGenPosixTime ftime(day1);
  const TextGen::WeatherPeriod period1(day1, day2);
  const TextGen::WeatherPeriod period2(day2, day3);

  string result;

  REQUIRE(mode, var, ftime, period1, "fi", "Sunnuntaista alkaen.");
  REQUIRE(mode, var, ftime, period1, "sv", "Från och med söndagen.");
  REQUIRE(mode, var, ftime, period1, "en", "From Sunday onwards.");

  // REQUIRE(mode,var,ftime,period2,"fi","Huomisesta alkaen.");
  // REQUIRE(mode,var,ftime,period2,"sv","Från i morgon.");
  // REQUIRE(mode,var,ftime,period2,"en","Starting tomorrow.");
  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaista alkaen.");
  REQUIRE(mode, var, ftime, period2, "sv", "Från och med måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "From Monday onwards.");

  Settings::set(phrasesvar, "weekday");
  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaista alkaen.");
  REQUIRE(mode, var, ftime, period2, "sv", "Från och med måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "From Monday onwards.");

  Settings::set(phrasesvar, "tomorrow");
  // REQUIRE(mode,var,ftime,period2,"fi","Huomisesta alkaen.");
  // REQUIRE(mode,var,ftime,period2,"sv","Från i morgon.");
  // REQUIRE(mode,var,ftime,period2,"en","Starting tomorrow.");
  REQUIRE(mode, var, ftime, period2, "fi", "Maanantaista alkaen.");
  REQUIRE(mode, var, ftime, period2, "sv", "Från och med måndagen.");
  REQUIRE(mode, var, ftime, period2, "en", "From Monday onwards.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("days");
 */
// ----------------------------------------------------------------------

void days()
{
  const TextGenPosixTime day1(2003, 06, 29);
  const TextGenPosixTime day2(2003, 06, 30);
  const TextGenPosixTime day3(2003, 07, 01);
  const TextGenPosixTime day4(2003, 07, 02);

  const string mode = "days";
  const string var = mode;
  const string phrasesvar = var + "::" + mode + "::phrases";

  const TextGenPosixTime ftime(day1);
  string result;

  Settings::set("days::day::starthour", "06");
  Settings::set("days::day::endhour", "18");

  // 1 day, starting today
  {
    const TextGen::WeatherPeriod period(day1, day2);

    Settings::set(mode + "::days::phrases", "none");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "today");
    REQUIRE(mode, var, ftime, period, "fi", "Tänään.");
    REQUIRE(mode, var, ftime, period, "sv", "I dag.");
    REQUIRE(mode, var, ftime, period, "en", "Today.");

    Settings::set(mode + "::days::phrases", "tomorrow");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "followingday");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "weekday");
    REQUIRE(mode, var, ftime, period, "fi", "Sunnuntaina.");
    REQUIRE(mode, var, ftime, period, "sv", "På söndagen.");
    REQUIRE(mode, var, ftime, period, "en", "On Sunday.");
  }

  // 1 day, starting tomorrow
  {
    const TextGen::WeatherPeriod period(day2, day3);

    Settings::set(mode + "::days::phrases", "none");
    REQUIRE(mode, var, ftime, period, "fi", "Huomenna.");
    REQUIRE(mode, var, ftime, period, "sv", "I morgon.");
    REQUIRE(mode, var, ftime, period, "en", "Tomorrow.");

    Settings::set(mode + "::days::phrases", "today");
    REQUIRE(mode, var, ftime, period, "fi", "Huomenna.");
    REQUIRE(mode, var, ftime, period, "sv", "I morgon.");
    REQUIRE(mode, var, ftime, period, "en", "Tomorrow.");

    Settings::set(mode + "::days::phrases", "tomorrow");
    REQUIRE(mode, var, ftime, period, "fi", "Huomenna.");
    REQUIRE(mode, var, ftime, period, "sv", "I morgon.");
    REQUIRE(mode, var, ftime, period, "en", "Tomorrow.");

    Settings::set(mode + "::days::phrases", "followingday");
    REQUIRE(mode, var, ftime, period, "fi", "Huomenna.");
    REQUIRE(mode, var, ftime, period, "sv", "I morgon.");
    REQUIRE(mode, var, ftime, period, "en", "Tomorrow.");

    Settings::set(mode + "::days::phrases", "weekday");
    REQUIRE(mode, var, ftime, period, "fi", "Maanantaina.");
    REQUIRE(mode, var, ftime, period, "sv", "På måndagen.");
    REQUIRE(mode, var, ftime, period, "en", "On Monday.");
  }

  // 2 days, starting today
  {
    const TextGen::WeatherPeriod period(day1, day3);

    Settings::set(mode + "::days::phrases", "none");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "today");
    REQUIRE(mode, var, ftime, period, "fi", "Tänään ja huomenna.");
    REQUIRE(mode, var, ftime, period, "sv", "I dag och i morgon.");
    REQUIRE(mode, var, ftime, period, "en", "Today and tomorrow.");

    Settings::set(mode + "::days::phrases", "tomorrow");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "followingday");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "weekday");
    REQUIRE(mode, var, ftime, period, "fi", "Sunnuntaina ja maanantaina.");
    REQUIRE(mode, var, ftime, period, "sv", "På söndagen och på måndagen.");
    REQUIRE(mode, var, ftime, period, "en", "On Sunday and on Monday.");
  }

  // 2 days, starting tomorrow
  {
    const TextGen::WeatherPeriod period(day2, day4);

    Settings::set(mode + "::days::phrases", "none");
    REQUIRE(mode, var, ftime, period, "fi", "Huomenna ja seuraavana päivänä.");
    REQUIRE(mode, var, ftime, period, "sv", "I morgon och följande dag.");
    REQUIRE(mode, var, ftime, period, "en", "Tomorrow and the following day.");

    Settings::set(mode + "::days::phrases", "today");
    REQUIRE(mode, var, ftime, period, "fi", "Huomenna ja seuraavana päivänä.");
    REQUIRE(mode, var, ftime, period, "sv", "I morgon och följande dag.");
    REQUIRE(mode, var, ftime, period, "en", "Tomorrow and the following day.");

    Settings::set(mode + "::days::phrases", "tomorrow");
    REQUIRE(mode, var, ftime, period, "fi", "Huomenna ja seuraavana päivänä.");
    REQUIRE(mode, var, ftime, period, "sv", "I morgon och följande dag.");
    REQUIRE(mode, var, ftime, period, "en", "Tomorrow and the following day.");

    Settings::set(mode + "::days::phrases", "followingday");
    REQUIRE(mode, var, ftime, period, "fi", "Huomenna ja seuraavana päivänä.");
    REQUIRE(mode, var, ftime, period, "sv", "I morgon och följande dag.");
    REQUIRE(mode, var, ftime, period, "en", "Tomorrow and the following day.");

    Settings::set(mode + "::days::phrases", "weekday");
    REQUIRE(mode, var, ftime, period, "fi", "Maanantaina ja tiistaina.");
    REQUIRE(mode, var, ftime, period, "sv", "På måndagen och på tisdagen.");
    REQUIRE(mode, var, ftime, period, "en", "On Monday and on Tuesday.");
  }

  // 3 days, starting today
  {
    const TextGen::WeatherPeriod period(day1, day4);

    Settings::set(mode + "::days::phrases", "none");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "today");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "tomorrow");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "followingday");
    REQUIRE(mode, var, ftime, period, "fi", "");
    REQUIRE(mode, var, ftime, period, "sv", "");
    REQUIRE(mode, var, ftime, period, "en", "");

    Settings::set(mode + "::days::phrases", "weekday");
    REQUIRE(mode, var, ftime, period, "fi", "Sunnuntaista alkaen.");
    REQUIRE(mode, var, ftime, period, "sv", "Från och med söndagen.");
    REQUIRE(mode, var, ftime, period, "en", "From Sunday onwards.");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PeriodPhraseFactory::create("remaining_day");
 */
// ----------------------------------------------------------------------

void remaining_day()
{
  using TextGen::WeatherPeriod;

  const TextGenPosixTime ftime(2003, 06, 29);

  const TextGenPosixTime hour00(2003, 06, 29, 0);
  const TextGenPosixTime hour01(2003, 06, 29, 1);
  const TextGenPosixTime hour02(2003, 06, 29, 2);
  const TextGenPosixTime hour03(2003, 06, 29, 3);
  const TextGenPosixTime hour04(2003, 06, 29, 4);
  const TextGenPosixTime hour05(2003, 06, 29, 5);
  const TextGenPosixTime hour06(2003, 06, 29, 6);
  const TextGenPosixTime hour07(2003, 06, 29, 7);
  const TextGenPosixTime hour08(2003, 06, 29, 8);
  const TextGenPosixTime hour09(2003, 06, 29, 9);
  const TextGenPosixTime hour10(2003, 06, 29, 10);
  const TextGenPosixTime hour11(2003, 06, 29, 11);
  const TextGenPosixTime hour12(2003, 06, 29, 12);
  const TextGenPosixTime hour13(2003, 06, 29, 13);
  const TextGenPosixTime hour14(2003, 06, 29, 14);
  const TextGenPosixTime hour15(2003, 06, 29, 15);
  const TextGenPosixTime hour16(2003, 06, 29, 16);
  const TextGenPosixTime hour17(2003, 06, 29, 17);
  const TextGenPosixTime hour18(2003, 06, 29, 18);
  const TextGenPosixTime hour19(2003, 06, 29, 19);
  const TextGenPosixTime hour20(2003, 06, 29, 20);
  const TextGenPosixTime hour21(2003, 06, 29, 21);
  const TextGenPosixTime hour22(2003, 06, 29, 22);
  const TextGenPosixTime hour23(2003, 06, 29, 23);
  const TextGenPosixTime hour24(2003, 06, 30, 0);

  const string mode = "remaining_day";
  const string var = mode;

  string result;

  REQUIRE(mode, var, ftime, WeatherPeriod(hour00, hour24), "fi", "");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour01, hour24), "fi", "");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour02, hour24), "fi", "");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour03, hour24), "fi", "Aamuyöstä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour04, hour24), "fi", "Aamuyöstä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour05, hour24), "fi", "Aamuyöstä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour06, hour24), "fi", "Aamusta alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour07, hour24), "fi", "Aamusta alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour08, hour24), "fi", "Aamusta alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour09, hour24), "fi", "Aamupäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour10, hour24), "fi", "Aamupäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour11, hour24), "fi", "Keskipäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour12, hour24), "fi", "Keskipäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour13, hour24), "fi", "Iltapäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour14, hour24), "fi", "Iltapäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour15, hour24), "fi", "Iltapäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour16, hour24), "fi", "Iltapäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour17, hour24), "fi", "Iltapäivästä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour18, hour24), "fi", "Illasta alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour19, hour24), "fi", "Illasta alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour20, hour24), "fi", "Illasta alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour21, hour24), "fi", "Illasta alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour22, hour24), "fi", "Yöstä alkaen.");
  REQUIRE(mode, var, ftime, WeatherPeriod(hour23, hour24), "fi", "Yöstä alkaen.");

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
    TEST(until_tonight);
    TEST(until_morning);
    TEST(today);
    TEST(tonight);
    TEST(next_night);
    TEST(next_day);
    TEST(next_days);
    TEST(days);
    TEST(remaining_day);
  }

};  // class tests

}  // namespace PeriodPhraseFactoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace PeriodPhraseFactoryTest;

  cout << endl << "PeriodPhraseFactory tests" << endl << "=========================" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
