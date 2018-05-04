#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "CloudinessStory.h"
#include "Story.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>
#include <boost/locale.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace boost;

namespace CloudinessStoryTest
{
shared_ptr<TextGen::Dictionary> dict;
TextGen::PlainTextFormatter formatter;

string require(const TextGen::Story& theStory,
               const string& theLanguage,
               const string& theName,
               const string& theExpected)
{
  try
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
  catch (...)
  {
    std::cerr << "Failed to get expected result: " << theExpected << std::endl;
    throw;
  }
}

#define REQUIRE(story, lang, name, expected)     \
  result = require(story, lang, name, expected); \
  if (!result.empty()) TEST_FAILED(result.c_str());

// ----------------------------------------------------------------------
/*!
 * \brief Test CloudinessStory::overview()
 */
// ----------------------------------------------------------------------

void overview()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  const string fun = "cloudiness_overview";

  string result;

  Settings::set("cloudiness_overview::day::starthour", "0");
  Settings::set("cloudiness_overview::day::endhour", "0");

  // 1-day forecasts for today
  {
    TextGenPosixTime time1(2003, 6, 3, 0, 0);
    TextGenPosixTime time2(2003, 6, 4, 0, 0);
    WeatherPeriod period(time1, time2);
    CloudinessStory story(time1, sources, area, period, "cloudiness_overview");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä.");
    REQUIRE(story, "sv", fun, "Mulet.");
    REQUIRE(story, "en", fun, "Cloudy.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "70,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä tai puolipilvistä.");
    REQUIRE(story, "sv", fun, "Mulet eller halvmulet.");
    REQUIRE(story, "en", fun, "Cloudy or partly cloudy.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "70,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "30,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    REQUIRE(story, "fi", fun, "Vaihtelevaa pilvisyyttä.");
    REQUIRE(story, "sv", fun, "Växlande molnighet.");
    REQUIRE(story, "en", fun, "Variable cloudiness.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "10,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "10,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    REQUIRE(story, "fi", fun, "Enimmäkseen puolipilvistä.");
    REQUIRE(story, "sv", fun, "Mestadels halvmulet.");
    REQUIRE(story, "en", fun, "Mostly partly cloudy.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "10,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "80,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    REQUIRE(story, "fi", fun, "Enimmäkseen selkeää.");
    REQUIRE(story, "sv", fun, "Mestadels klart.");
    REQUIRE(story, "en", fun, "Mostly sunny.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "30,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "30,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "90,0");
    REQUIRE(story, "fi", fun, "Pilvistyvää.");
    REQUIRE(story, "sv", fun, "Ökad molnighet.");
    REQUIRE(story, "en", fun, "Increasing cloudiness.");
  }

  // 2-day forecasts starting today
  {
    TextGenPosixTime time1(2003, 6, 3, 0, 0);
    TextGenPosixTime time2(2003, 6, 4, 0, 0);
    TextGenPosixTime time3(2003, 6, 5, 0, 0);
    WeatherPeriod period(time1, time3);
    CloudinessStory story(time1, sources, area, period, "cloudiness_overview");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day2::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day2::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day2::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä.");
    REQUIRE(story, "sv", fun, "Mulet.");
    REQUIRE(story, "en", fun, "Cloudy.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day2::cloudy", "10,0");
    Settings::set("cloudiness_overview::fake::day2::clear", "10,0");
    Settings::set("cloudiness_overview::fake::day2::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä tai puolipilvistä.");
    REQUIRE(story, "sv", fun, "Mulet eller halvmulet.");
    REQUIRE(story, "en", fun, "Cloudy or partly cloudy.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day2::cloudy", "0,0");
    Settings::set("cloudiness_overview::fake::day2::clear", "80,0");
    Settings::set("cloudiness_overview::fake::day2::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä, huomenna enimmäkseen selkeää.");
    REQUIRE(story, "sv", fun, "Mulet, i morgon mestadels klart.");
    REQUIRE(story, "en", fun, "Cloudy, tomorrow mostly sunny.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "30,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "30,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day2::cloudy", "0,0");
    Settings::set("cloudiness_overview::fake::day2::clear", "90,0");
    Settings::set("cloudiness_overview::fake::day2::trend", "0,0");
    REQUIRE(story, "fi", fun, "Vaihtelevaa pilvisyyttä, huomenna selkeää.");
    REQUIRE(story, "sv", fun, "Växlande molnighet, i morgon klart.");
    REQUIRE(story, "en", fun, "Variable cloudiness, tomorrow sunny.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "30,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "30,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day2::cloudy", "0,0");
    Settings::set("cloudiness_overview::fake::day2::clear", "80,0");
    Settings::set("cloudiness_overview::fake::day2::trend", "0,0");
    REQUIRE(story, "fi", fun, "Vaihtelevaa pilvisyyttä, huomenna enimmäkseen selkeää.");
    REQUIRE(story, "sv", fun, "Växlande molnighet, i morgon mestadels klart.");
    REQUIRE(story, "en", fun, "Variable cloudiness, tomorrow mostly sunny.");
  }

  // 3-day forecasts starting today
  {
    TextGenPosixTime time1(2003, 6, 3, 0, 0);
    TextGenPosixTime time2(2003, 6, 4, 0, 0);
    TextGenPosixTime time3(2003, 6, 5, 0, 0);
    TextGenPosixTime time4(2003, 6, 6, 0, 0);
    WeatherPeriod period(time1, time4);
    CloudinessStory story(time1, sources, area, period, "cloudiness_overview");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day2::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day2::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day2::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day3::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day3::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day3::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä.");
    REQUIRE(story, "sv", fun, "Mulet.");
    REQUIRE(story, "en", fun, "Cloudy.");

    Settings::set("cloudiness_overview::fake::day3::cloudy", "10,0");
    Settings::set("cloudiness_overview::fake::day3::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day3::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä tai puolipilvistä.");
    REQUIRE(story, "sv", fun, "Mulet eller halvmulet.");
    REQUIRE(story, "en", fun, "Cloudy or partly cloudy.");

    Settings::set("cloudiness_overview::fake::day3::cloudy", "0,0");
    Settings::set("cloudiness_overview::fake::day3::clear", "90,0");
    Settings::set("cloudiness_overview::fake::day3::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä, torstaina selkeää.");
    REQUIRE(story, "sv", fun, "Mulet, på torsdagen klart.");
    REQUIRE(story, "en", fun, "Cloudy, on Thursday sunny.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day2::cloudy", "20,0");
    Settings::set("cloudiness_overview::fake::day2::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day2::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day3::cloudy", "10,0");
    Settings::set("cloudiness_overview::fake::day3::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day3::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä tai puolipilvistä.");
    REQUIRE(story, "sv", fun, "Mulet eller halvmulet.");
    REQUIRE(story, "en", fun, "Cloudy or partly cloudy.");

    Settings::set("cloudiness_overview::fake::day1::cloudy", "90,0");
    Settings::set("cloudiness_overview::fake::day1::clear", "0,0");
    Settings::set("cloudiness_overview::fake::day1::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day2::cloudy", "10,0");
    Settings::set("cloudiness_overview::fake::day2::clear", "80,0");
    Settings::set("cloudiness_overview::fake::day2::trend", "0,0");
    Settings::set("cloudiness_overview::fake::day3::cloudy", "10,0");
    Settings::set("cloudiness_overview::fake::day3::clear", "90,0");
    Settings::set("cloudiness_overview::fake::day3::trend", "0,0");
    REQUIRE(story, "fi", fun, "Pilvistä, huomenna ja seuraavana päivänä enimmäkseen selkeää.");
    REQUIRE(story, "sv", fun, "Mulet, i morgon och följande dag mestadels klart.");
    REQUIRE(story, "en", fun, "Cloudy, tomorrow and the following day mostly sunny.");
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
  void test(void) { TEST(overview); }
};  // class tests

}  // namespace CloudinessStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace CloudinessStoryTest;

  cout << endl << "CloudinessStory tests" << endl << "=====================" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
