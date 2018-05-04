#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "WeatherStory.h"
#include "Story.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace WeatherStoryTest
{
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

// ----------------------------------------------------------------------
/*!
 * \brief Test WeatherStory::short_overview()
 */
// ----------------------------------------------------------------------

void short_overview()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  const string fun = "weather_shortoverview";

  TextGenPosixTime time1(2003, 6, 1);
  TextGenPosixTime time2(2003, 6, 4);
  WeatherPeriod period(time1, time2);
  WeatherStory story(time1, sources, area, period, "a");

  Settings::set("a::cloudiness::clear", "40");
  Settings::set("a::cloudiness::cloudy", "70");
  Settings::set("a::cloudiness::single_limit", "60");
  Settings::set("a::cloudiness::double_limit", "20");
  Settings::set("a::precipitation::rainy", "1");
  Settings::set("a::precipitation::partly_rainy", "0.1");
  Settings::set("a::precipitation::unstable", "50");

  // Test cloudiness phrases
  {
    Settings::set("a::fake::day1::precipitation", "0,0");
    Settings::set("a::fake::day2::precipitation", "0,0");
    Settings::set("a::fake::day3::precipitation", "0,0");

    Settings::set("a::fake::clear_percentage", "70,0");
    Settings::set("a::fake::cloudy_percentage", "20,0");
    require(story, "fi", fun, "Enimmäkseen selkeää, poutaa.");
    require(story, "sv", fun, "Mestadels klart, uppehåll.");
    require(story, "en", fun, "Mostly sunny, fair weather.");

    Settings::set("a::fake::clear_percentage", "20,0");
    Settings::set("a::fake::cloudy_percentage", "70,0");
    require(story, "fi", fun, "Enimmäkseen pilvistä, poutaa.");
    require(story, "sv", fun, "Mestadels mulet, uppehåll.");
    require(story, "en", fun, "Mostly cloudy, fair weather.");

    Settings::set("a::fake::clear_percentage", "20,0");
    Settings::set("a::fake::cloudy_percentage", "20,0");
    require(story, "fi", fun, "Enimmäkseen puolipilvistä, poutaa.");
    require(story, "sv", fun, "Mestadels halvmulet, uppehåll.");
    require(story, "en", fun, "Mostly partly cloudy, fair weather.");

    Settings::set("a::fake::clear_percentage", "10,0");
    Settings::set("a::fake::cloudy_percentage", "40,0");
    require(story, "fi", fun, "Enimmäkseen pilvistä tai puolipilvistä, poutaa.");
    require(story, "sv", fun, "Mestadels mulet eller halvmulet, uppehåll.");
    require(story, "en", fun, "Mostly cloudy or partly cloudy, fair weather.");

    Settings::set("a::fake::clear_percentage", "40,0");
    Settings::set("a::fake::cloudy_percentage", "10,0");
    require(story, "fi", fun, "Enimmäkseen selkeää tai puolipilvistä, poutaa.");
    require(story, "sv", fun, "Mestadels klart eller halvmulet, uppehåll.");
    require(story, "en", fun, "Mostly sunny or partly cloudy, fair weather.");

    Settings::set("a::fake::clear_percentage", "30,0");
    Settings::set("a::fake::cloudy_percentage", "30,0");
    require(story, "fi", fun, "Vaihtelevaa pilvisyyttä, poutaa.");
    require(story, "sv", fun, "Växlande molnighet, uppehåll.");
    require(story, "en", fun, "Variable cloudiness, fair weather.");
  }

  // Test rain phrases
  {
    Settings::set("a::fake::clear_percentage", "30,0");
    Settings::set("a::fake::cloudy_percentage", "30,0");

    Settings::set("a::fake::day1::precipitation", "0,0");
    Settings::set("a::fake::day2::precipitation", "0,0");
    Settings::set("a::fake::day3::precipitation", "0,0");
    require(story, "fi", fun, "Vaihtelevaa pilvisyyttä, poutaa.");
    require(story, "sv", fun, "Växlande molnighet, uppehåll.");
    require(story, "en", fun, "Variable cloudiness, fair weather.");

    Settings::set("a::fake::day1::precipitation", "0.2,0");
    Settings::set("a::fake::day2::precipitation", "0,0");
    Settings::set("a::fake::day3::precipitation", "0,0");
    require(story, "fi", fun, "Vaihtelevaa pilvisyyttä, sunnuntaina paikoin sadetta.");
    require(story, "sv", fun, "Växlande molnighet, på söndagen lokalt regn.");
    require(story, "en", fun, "Variable cloudiness, on Sunday in some places rain.");

    Settings::set("a::fake::day1::precipitation", "2,0");
    Settings::set("a::fake::day2::precipitation", "0,0");
    Settings::set("a::fake::day3::precipitation", "0,0");
    require(story, "fi", fun, "Vaihtelevaa pilvisyyttä, sunnuntaina sadetta.");
    require(story, "sv", fun, "Växlande molnighet, på söndagen regn.");
    require(story, "en", fun, "Variable cloudiness, on Sunday rain.");

    Settings::set("a::fake::day1::precipitation", "0,0");
    Settings::set("a::fake::day2::precipitation", "2,0");
    Settings::set("a::fake::day3::precipitation", "0,0");
    require(story, "fi", fun, "Vaihtelevaa pilvisyyttä, maanantaina sadetta.");
    require(story, "sv", fun, "Växlande molnighet, på måndagen regn.");
    require(story, "en", fun, "Variable cloudiness, on Monday rain.");

    Settings::set("a::fake::day1::precipitation", "0.1,0");
    Settings::set("a::fake::day2::precipitation", "2,0");
    Settings::set("a::fake::day3::precipitation", "0,0");
    require(story, "fi", fun, "Vaihtelevaa pilvisyyttä, ajoittain sateista.");
    require(story, "sv", fun, "Växlande molnighet, tidvis regnigt.");
    require(story, "en", fun, "Variable cloudiness, intermittent rain.");

    Settings::set("a::fake::day1::precipitation", "2,0");
    Settings::set("a::fake::day2::precipitation", "2,0");
    Settings::set("a::fake::day3::precipitation", "0,0");
    require(story, "fi", fun, "Sää on epävakaista.");
    require(story, "sv", fun, "Ostadigt väder.");
    require(story, "en", fun, "The weather is unstable.");

    Settings::set("a::fake::day1::precipitation", "2,0");
    Settings::set("a::fake::day2::precipitation", "2,0");
    Settings::set("a::fake::day3::precipitation", "2,0");
    require(story, "fi", fun, "Sää on epävakaista.");
    require(story, "sv", fun, "Ostadigt väder.");
    require(story, "en", fun, "The weather is unstable.");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeatherStory::thunderprobability()
 */
// ----------------------------------------------------------------------

void thunderprobability()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  const string fun = "weather_thunderprobability";

  TextGenPosixTime time1(2003, 6, 1);
  TextGenPosixTime time2(2003, 6, 4);
  WeatherPeriod period(time1, time2);
  WeatherStory story(time1, sources, area, period, "b");

  Settings::set("b::precision", "10");
  Settings::set("b::limit", "10");

  Settings::set("b::fake::probability", "0,0");
  require(story, "fi", fun, "");
  require(story, "sv", fun, "");
  require(story, "en", fun, "");

  Settings::set("b::fake::probability", "5,0");
  require(story, "fi", fun, "Ukkosen todennäköisyys on 10%.");
  require(story, "sv", fun, "Sannolikheten för åska är 10%.");
  require(story, "en", fun, "Probability of thunder is 10%.");

  Settings::set("b::fake::probability", "64,0");
  require(story, "fi", fun, "Ukkosen todennäköisyys on 60%.");
  require(story, "sv", fun, "Sannolikheten för åska är 60%.");
  require(story, "en", fun, "Probability of thunder is 60%.");

  Settings::set("b::fake::probability", "99,0");
  require(story, "fi", fun, "Ukkosen todennäköisyys on 100%.");
  require(story, "sv", fun, "Sannolikheten för åska är 100%.");
  require(story, "en", fun, "Probability of thunder is 100%.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeatherStory::thunderprobability_simplified()
 */
// ----------------------------------------------------------------------

void thunderprobability_simplified()
{
  using namespace std;
  using namespace TextGen;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");

  const string fun = "weather_thunderprobability_simplified";

  TextGenPosixTime time1(2003, 6, 1);
  TextGenPosixTime time2(2003, 6, 4);
  WeatherPeriod period(time1, time2);
  WeatherStory story(time1, sources, area, period, "b");

  Settings::set("b::precision", "10");
  Settings::set("b::limit", "30");

  Settings::set("b::fake::probability", "15,0");
  require(story, "fi", fun, "");
  require(story, "sv", fun, "");
  require(story, "en", fun, "");

  Settings::set("b::fake::probability", "50,0");
  require(story, "fi", fun, "Ukkoskuurot paikoin mahdollisia.");
  require(story, "sv", fun, "Lokala åskskurar möjliga.");
  require(story, "en", fun, "Possible local thunder showers.");

  Settings::set("b::fake::probability", "90,0");
  require(story, "fi", fun, "Ukkoskuurot todennäköisiä.");
  require(story, "sv", fun, "Åskskurar sannolika.");
  require(story, "en", fun, "Probable thunder showers.");

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
    TEST(short_overview);
    TEST(thunderprobability);
    TEST(thunderprobability_simplified);
  }

};  // class tests

}  // namespace WeatherStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace WeatherStoryTest;

  cout << endl << "WeatherStory tests" << endl << "==================" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
