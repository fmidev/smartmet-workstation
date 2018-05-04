/*
* NOTE: This file is to be edited in "Windows Latin-1" encoding (NOT UTF-8)
*/
#include <regression/tframe.h>

#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "WaveStory.h"
#include "Story.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace WaveStoryTest
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
 * \brief WaveStory::range
 */
// ----------------------------------------------------------------------

void wave_range()
{
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  const WeatherArea area("25,60");

  const string fun = "wave_range";

  const TextGenPosixTime time1(2003, 6, 1, 0, 0);
  const TextGenPosixTime time2(2003, 6, 2, 0, 0);

  string result;

  Settings::set("a::day::starthour", "6");
  Settings::set("a::day::endhour", "18");

  const WeatherPeriod period(time1, time2);
  WaveStory story(time1, sources, area, period, "a");

  Settings::set("a::fake::height::mean", "1.9,0");
  Settings::set("a::fake::height::minimum", "1.8,0");
  Settings::set("a::fake::height::maximum", "2.2,0");

  REQUIRE(story, "fi", fun, "Aallonkorkeus on noin 2.0m.");
  REQUIRE(story, "sv", fun, "Våghöjden är cirka 2.0m.");
  REQUIRE(story, "en", fun, "Wave height is around 2.0m.");

  Settings::set("a::fake::height::mean", "4.7,0");
  Settings::set("a::fake::height::minimum", "3.1,0");
  Settings::set("a::fake::height::maximum", "7.3,0");

  REQUIRE(story, "fi", fun, "Aallonkorkeus on 3.0-7.5m.");
  REQUIRE(story, "sv", fun, "Våghöjden är 3.0-7.5m.");
  REQUIRE(story, "en", fun, "Wave height is 3.0-7.5m.");

  Settings::set("a::fake::height::mean", "0.2,0");
  Settings::set("a::fake::height::minimum", "0.1,0");
  Settings::set("a::fake::height::maximum", "0.2,0");

  REQUIRE(story, "fi", fun, "Aallonkorkeus on alle 0.5m.");
  REQUIRE(story, "sv", fun, "Våghöjden är mindre än 0.5m.");
  REQUIRE(story, "en", fun, "Wave height is less than 0.5m.");

  Settings::set("a::fake::height::mean", "0.8,0");
  Settings::set("a::fake::height::minimum", "0.1,0");
  Settings::set("a::fake::height::maximum", "1.3,0");

  REQUIRE(story, "fi", fun, "Aallonkorkeus on enimmillään 1.5m.");
  REQUIRE(story, "sv", fun, "Våghöjden är högst 1.5m.");
  REQUIRE(story, "en", fun, "Wave height is at most 1.5m.");

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
  void test(void) { TEST(wave_range); }
};  // class tests

}  // namespace WindStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace WaveStoryTest;

  cout << endl << "WaveStory tests" << endl << "===============" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
