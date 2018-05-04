#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Header.h"
#include "HeaderFactory.h"
#include "PlainTextFormatter.h"
#include <calculator/WeatherArea.h>
#include <calculator/WeatherPeriod.h>
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace HeaderFactoryTest
{
shared_ptr<TextGen::Dictionary> dict;
TextGen::PlainTextFormatter formatter;

string require(const string& theLanguage,
               const TextGen::WeatherArea& theArea,
               const TextGen::WeatherPeriod& thePeriod,
               const string& theName,
               const string& theResult)
{
  dict->init(theLanguage);
  formatter.dictionary(dict);

  TextGen::Header header = TextGen::HeaderFactory::create(theArea, thePeriod, theName);

  const string value = header.realize(formatter);

  if (value != theResult)
    return (value + " <> " + theResult);
  else
    return "";
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "none"
 */
// ----------------------------------------------------------------------

void header_none()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "none");

  WeatherArea area("25,60");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1), TextGenPosixTime(2003, 6, 2));

  string result;

  result = require("fi", area, period, var, "");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "until"
 */
// ----------------------------------------------------------------------

void header_until()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "until");

  WeatherArea area("25,60");
  WeatherPeriod period1(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 2, 6));
  WeatherPeriod period2(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 2, 18));

  string result;

  result = require("fi", area, period1, var, "Odotettavissa maanantaiaamuun asti");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period1, var, "Utsikter till måndag morgon");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period1, var, "Expected weather until Monday morning");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("fi", area, period2, var, "Odotettavissa maanantai-iltaan asti");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period2, var, "Utsikter till måndag kväll");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period2, var, "Expected weather until Monday evening");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "from_until"
 */
// ----------------------------------------------------------------------

void header_from_until()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "from_until");

  WeatherArea area("25,60");
  WeatherPeriod period1(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 2, 6));
  WeatherPeriod period2(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 2, 18));

  string result;

  result = require("fi", area, period1, var, "Odotettavissa sunnuntaiaamusta maanantaiaamuun");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result =
      require("sv", area, period1, var, "Utsikter från och med söndag morgon till måndag morgon");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require(
      "en", area, period1, var, "Expected weather from Sunday morning until Monday morning");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("fi", area, period2, var, "Odotettavissa sunnuntaiaamusta maanantai-iltaan");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result =
      require("sv", area, period2, var, "Utsikter från och med söndag morgon till måndag kväll");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require(
      "en", area, period2, var, "Expected weather from Sunday morning until Monday evening");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "several_days"
 */
// ----------------------------------------------------------------------

void header_several_days()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "several_days");

  WeatherArea area("25,60");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 3, 6));

  string result;

  result = require("fi", area, period, var, "Sunnuntaiaamusta alkavan kahden vuorokauden sää");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result =
      require("sv", area, period, var, "Från och med söndag morgon för de följande två dygnen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "From Sunday morning for the next two days");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "report_area"
 */
// ----------------------------------------------------------------------

void header_report_area()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "report_area");

  WeatherArea area("25,60:30", "uusimaa");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 3, 6));

  string result;

  result = require("fi", area, period, var, "Sääennuste Uudellemaalle sunnuntaina kello 6");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Väderrapport för Nyland söndag klockan 6");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Weather report for Uusimaa on Sunday 6 o'clock");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "report_time"
 */
// ----------------------------------------------------------------------

void header_report_time()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "report_time");

  WeatherArea area("25,60:30", "uusimaa");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 3, 6));

  string result;

  result = require("fi", area, period, var, "Sääennuste sunnuntaina kello 6");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Väderrapport söndag klockan 6");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Weather report on Sunday 6 o'clock");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "report_location"
 */
// ----------------------------------------------------------------------

void header_report_location()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "report_location");

  WeatherArea area("25,60", "helsinki");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 3, 6));

  string result;

  result = require("fi", area, period, var, "Helsinki");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Helsinki");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Helsinki");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "morning"
 */
// ----------------------------------------------------------------------

void header_morning()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "morning");
  Settings::set(var + "::weekdays", "false");

  WeatherArea area("25,60");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 1, 12));

  string result;

  result = require("fi", area, period, var, "Odotettavissa aamulla");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på morgonen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather in the morning");
  if (!result.empty()) TEST_FAILED(result.c_str());

  Settings::set(var + "::weekdays", "true");

  result = require("fi", area, period, var, "Odotettavissa sunnuntaina aamulla");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på söndag morgonen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather on Sunday morning");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "forenoon"
 */
// ----------------------------------------------------------------------

void header_forenoon()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "forenoon");
  Settings::set(var + "::weekdays", "false");

  WeatherArea area("25,60");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 1, 12));

  string result;

  result = require("fi", area, period, var, "Odotettavissa aamupäivällä");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på förmiddagen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather in the forenoon");
  if (!result.empty()) TEST_FAILED(result.c_str());

  Settings::set(var + "::weekdays", "true");

  result = require("fi", area, period, var, "Odotettavissa sunnuntaina aamupäivällä");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på söndag förmiddagen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather on Sunday forenoon");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "afternoon"
 */
// ----------------------------------------------------------------------

void header_afternoon()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "afternoon");
  Settings::set(var + "::weekdays", "false");

  WeatherArea area("25,60");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 1, 12));

  string result;

  result = require("fi", area, period, var, "Odotettavissa iltapäivällä");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på eftermiddagen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather in the afternoon");
  if (!result.empty()) TEST_FAILED(result.c_str());

  Settings::set(var + "::weekdays", "true");

  result = require("fi", area, period, var, "Odotettavissa sunnuntaina iltapäivällä");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på söndag eftermiddagen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather on Sunday afternoon");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "evening"
 */
// ----------------------------------------------------------------------

void header_evening()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "evening");
  Settings::set(var + "::weekdays", "false");

  WeatherArea area("25,60");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 1, 12));

  string result;

  result = require("fi", area, period, var, "Odotettavissa illalla");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på kvällen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather in the evening");
  if (!result.empty()) TEST_FAILED(result.c_str());

  Settings::set(var + "::weekdays", "true");

  result = require("fi", area, period, var, "Odotettavissa sunnuntaina illalla");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på söndag kvällen");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather on Sunday evening");
  if (!result.empty()) TEST_FAILED(result.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test HeaderFactory::create() with type "clock_range"
 */
// ----------------------------------------------------------------------

void header_clock_range()
{
  using namespace TextGen;

  string var = "variable";
  Settings::set(var + "::type", "clock_range");
  Settings::set(var + "::weekdays", "false");

  WeatherArea area("25,60");
  WeatherPeriod period(TextGenPosixTime(2003, 6, 1, 6, 0), TextGenPosixTime(2003, 6, 1, 12));

  string result;

  result = require("fi", area, period, var, "Odotettavissa kello 6-12");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter klockan 6-12");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather 6-12 o'clock");
  if (!result.empty()) TEST_FAILED(result.c_str());

  Settings::set(var + "::weekdays", "true");

  result = require("fi", area, period, var, "Odotettavissa sunnuntaina kello 6-12");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("sv", area, period, var, "Utsikter på söndagen klockan 6-12");
  if (!result.empty()) TEST_FAILED(result.c_str());

  result = require("en", area, period, var, "Expected weather on Sunday 6-12 o'clock");
  if (!result.empty()) TEST_FAILED(result.c_str());

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
    TEST(header_none);
    TEST(header_until);
    TEST(header_from_until);
    TEST(header_several_days);
    TEST(header_report_time);
    TEST(header_report_area);
    TEST(header_report_location);
    TEST(header_morning);
    TEST(header_forenoon);
    TEST(header_afternoon);
    TEST(header_evening);
    TEST(header_clock_range);
  }

};  // class tests

}  // namespace HeaderFactoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace HeaderFactoryTest;

  cout << endl << "HeaderFactory tests" << endl << "======================" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
