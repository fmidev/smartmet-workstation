#include <regression/tframe.h>
#include "DebugDictionary.h"
#include "Sentence.h"
#include "Integer.h"
#include "PlainTextFormatter.h"
#include "TextFormatterTools.h"
#include "UnitFactory.h"
#include "WeatherTime.h"
#include "TimePeriod.h"
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
using namespace TextGen;

namespace TextFormatterToolsTest
{
shared_ptr<TextGen::Dictionary> dict;

// ----------------------------------------------------------------------
/*!
 * \brief Test TextFormatterTools::capitalize
 */
// ----------------------------------------------------------------------

void capitalize()
{
  string tmp = "testi 1";
  string res = TextFormatterTools::capitalize(tmp);
  if (res != "Testi 1") TEST_FAILED("Failed to capitalize 'testi 1', got " + res);

  tmp = "testi 2";
  res = TextFormatterTools::capitalize(tmp);
  if (res != "Testi 2") TEST_FAILED("Failed to handle 'Testi 2', got " + res);

  tmp = "ähtäri";
  res = TextFormatterTools::capitalize(tmp);
  if (res != "Ähtäri") TEST_FAILED("Failed to capitalize 'ähtäri', got " + res);

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TextFormatterTools::punctuate
 */
// ----------------------------------------------------------------------

void punctuate()
{
  string tmp = "testi 1";
  TextFormatterTools::punctuate(tmp);
  if (tmp != "testi 1.") TEST_FAILED("Failed to punctuate 'testi 1'");

  tmp = "";
  TextFormatterTools::punctuate(tmp);
  if (tmp != "") TEST_FAILED("Failed to punctuate ''");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TextFormatterTools::realize
 */
// ----------------------------------------------------------------------

void realize()
{
  string tmp;

  PlainTextFormatter formatter;
  formatter.dictionary(dict);

  // Test 1: normal case
  {
    Sentence s;
    s << "lämpötila"
      << "on"
      << "[1] asteen paikkeilla" << TextGen::Integer(10);

    tmp = TextFormatterTools::realize(s.begin(), s.end(), formatter, " ", "");
    if (tmp != "lämpötila on 10 asteen paikkeilla") TEST_FAILED("Test 1 failed: " + tmp);
  }

  // Test 2: normal case with 2 values
  {
    Sentence s;
    s << "lämpötila"
      << "on"
      << "[1] viiva [2] astetta" << TextGen::Integer(10) << TextGen::Integer(15);

    tmp = TextFormatterTools::realize(s.begin(), s.end(), formatter, " ", "");
    if (tmp != "lämpötila on 10 viiva 15 astetta") TEST_FAILED("Test 2 failed: " + tmp);
  }

  // Test 3: degrees
  {
    Settings::set("textgen::units::celsius::format", "phrase");

    Sentence s;
    s << "lämpötila"
      << "on noin"
      << "[1] [2]" << TextGen::Integer(10) << *UnitFactory::create(DegreesCelsius);

    tmp = TextFormatterTools::realize(s.begin(), s.end(), formatter, " ", "");
    if (tmp != "lämpötila on noin 10 astetta") TEST_FAILED("Test 3 failed: " + tmp);
  }

  // Test 4: SI units
  {
    Settings::set("textgen::units::celsius::format", "SI");

    Sentence s;
    s << "lämpötila"
      << "on noin"
      << "[1] [2]" << TextGen::Integer(10) << *UnitFactory::create(DegreesCelsius);

    tmp = TextFormatterTools::realize(s.begin(), s.end(), formatter, " ", "");
    if (tmp != "lämpötila on noin 10°C") TEST_FAILED("Test 4 failed: " + tmp);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TextFormatterTools::format_time functions
 */
// ----------------------------------------------------------------------

void format_time()
{
  PlainTextFormatter formatter;
  formatter.dictionary(dict);

  // Test 1: format_time(const TextGenPosixTime& theTime, const std::string& theFormattingString)
  {
    TextGenPosixTime nfmiTime(2012, 8, 9, 14, 39);

    Sentence s;
    s << WeatherTime(nfmiTime);

    string tmp = TextFormatterTools::format_time(nfmiTime, "%d.%m.%Y %H:%M");
    if (tmp != "09.08.2012 14:39") TEST_FAILED("format_time-test 1 failed: " + tmp);
  }

  // Test 2: std::string format_time(const TextGenPosixTime& theTime, const std::string&
  // theStoryVar,	const std::string& theFormatterName)
  {
    TextGenPosixTime nfmiTime(2012, 8, 9, 14, 39);

    Sentence s;
    s << WeatherTime(nfmiTime);

    Settings::set("textgen::part1::story::test::timeformat", "%d.%m.%Y %H");

    string tmp =
        TextFormatterTools::format_time(nfmiTime, "textgen::part1::story::test", "%d.%m.%Y %H");
    if (tmp != "09.08.2012 14") TEST_FAILED("format_time-test 2 failed: " + tmp);
  }
  // Test 3: std::string format_time(const WeatherPeriod& thePeriod, const std::string& theStoryVar,
  // const std::string& theFormatterName)
  {
    TextGenPosixTime startTime(2012, 8, 9, 14, 39);
    TextGenPosixTime endTime(2012, 8, 10, 12, 00);
    WeatherPeriod weatherPeriod(startTime, endTime);

    Sentence s;
    s << TimePeriod(weatherPeriod);

    Settings::set("textgen::part1::story::test::plain::startformat", "%d.%m.%Y %H:%M - ");
    Settings::set("textgen::part1::story::test::plain::endformat", "%d.%m.%Y %H:%M");

    string tmp =
        TextFormatterTools::format_time(weatherPeriod, "textgen::part1::story::test", "plain");
    if (tmp != "09.08.2012 14:39 - 10.08.2012 12:00")
      TEST_FAILED("format_time-test 3 failed: " + tmp);
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
    TEST(capitalize);
    TEST(punctuate);
    TEST(realize);
    TEST(format_time);
  }

};  // class tests

}  // namespace TextFormatterToolsTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace TextFormatterToolsTest;

  cout << endl << "TextFormatterTools tests" << endl << "========================" << endl;

  dict.reset(new TextGen::DebugDictionary());

  tests t;
  return t.run();
}
