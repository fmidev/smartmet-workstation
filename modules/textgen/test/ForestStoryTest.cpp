#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "ForestStory.h"
#include "Story.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace ForestStoryTest
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
 * \brief Test ForestStory::forestfireindex_twodays
 */
// ----------------------------------------------------------------------

void forestfireindex_twodays()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  const string fun = "forestfireindex_twodays";

  string result;

  Settings::set("forestfireindex_twodays::day::starthour", "6");
  Settings::set("forestfireindex_twodays::day::endhour", "18");

  // 1-day forecasts
  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 6, 0);
    WeatherPeriod period(time1, time2);
    ForestStory story(time1, sources, area, period, "forestfireindex_twodays");

    Settings::set("forestfireindex_twodays::fake::day1::maximum", "10,0");
    REQUIRE(story, "fi", fun, "Metsäpaloindeksi on 10.0.");
    REQUIRE(story, "sv", fun, "Indexet för skogsbrand är 10.0.");
    REQUIRE(story, "en", fun, "The forest fire warning index is 10.0.");
  }

  // Another 1-day forecast, because 17 < 18 (minendhour)

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 17, 0);
    WeatherPeriod period(time1, time2);
    ForestStory story(time1, sources, area, period, "forestfireindex_twodays");

    Settings::set("forestfireindex_twodays::fake::day1::maximum", "20,0");
    REQUIRE(story, "fi", fun, "Metsäpaloindeksi on 20.0.");
    REQUIRE(story, "sv", fun, "Indexet för skogsbrand är 20.0.");
    REQUIRE(story, "en", fun, "The forest fire warning index is 20.0.");
  }

  // Another 1-day forecast with small PoP

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 6, 0);
    WeatherPeriod period(time1, time2);
    ForestStory story(time1, sources, area, period, "forestfireindex_twodays");

    Settings::set("forestfireindex_twodays::fake::day1::maximum", "0,0");
    REQUIRE(story, "fi", fun, "Metsäpaloindeksi on 0.0.");
    REQUIRE(story, "sv", fun, "Indexet för skogsbrand är 0.0.");
    REQUIRE(story, "en", fun, "The forest fire warning index is 0.0.");
  }

  // 2-day forecasts

  {
    TextGenPosixTime time1(2003, 6, 3, 6, 0);
    TextGenPosixTime time2(2003, 6, 4, 18, 0);
    WeatherPeriod period(time1, time2);
    ForestStory story(time1, sources, area, period, "forestfireindex_twodays");

    Settings::set("forestfireindex_twodays::fake::day1::maximum", "50,0");
    Settings::set("forestfireindex_twodays::fake::day2::maximum", "50,0");
    REQUIRE(story, "fi", fun, "Metsäpaloindeksi on 50.0, huomenna sama.");
    REQUIRE(story, "sv", fun, "Indexet för skogsbrand är 50.0, i morgon densamma.");
    REQUIRE(story, "en", fun, "The forest fire warning index is 50.0, tomorrow the same.");

    Settings::set("forestfireindex_twodays::fake::day1::maximum", "60,0");
    Settings::set("forestfireindex_twodays::fake::day2::maximum", "10,0");
    REQUIRE(story, "fi", fun, "Metsäpaloindeksi on 60.0, huomenna 10.0.");
    REQUIRE(story, "sv", fun, "Indexet för skogsbrand är 60.0, i morgon 10.0.");
    REQUIRE(story, "en", fun, "The forest fire warning index is 60.0, tomorrow 10.0.");
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
  void test(void) { TEST(forestfireindex_twodays); }
};  // class tests

}  // namespace ForestStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace ForestStoryTest;

  cout << endl << "ForestStory tests" << endl << "=================" << endl;

  NFmiSettings::Init();

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
