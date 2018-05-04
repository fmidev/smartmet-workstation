#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "SpecialStory.h"
#include "Story.h"
#include "StoryTag.h"
#include "MessageLogger.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace SpecialStoryTest
{
shared_ptr<TextGen::Dictionary> dict;
TextGen::PlainTextFormatter formatter;

void require(const TextGen::Story& theStory,
             const string& theLanguage,
             const string& theName,
             const string& theExpected,
             const string& theStoryVar = "")
{
  dict->init(theLanguage);
  formatter.dictionary(dict);

  TextGen::Paragraph para;
  para << TextGen::StoryTag(theStoryVar, true);
  para << theStory.makeStory(theName);
  para << TextGen::StoryTag(theStoryVar, false);
  const string value = para.realize(formatter);

  if (value != theExpected)
  {
    const string msg = "'" + value + "' <> '" + theExpected + "'";
    TEST_FAILED(msg.c_str());
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test SpecialStory::none()
 */
// ----------------------------------------------------------------------

void special_none()
{
  using namespace std;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  SpecialStory story(time1, sources, area, period, "none");

  const string fun = "none";

  require(story, "fi", fun, "");
  require(story, "sv", fun, "");
  require(story, "en", fun, "");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test SpecialStory::text()
 */
// ----------------------------------------------------------------------

void special_text()
{
  using namespace std;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1);
  TextGenPosixTime time2(2000, 1, 2);
  WeatherPeriod period(time1, time2);
  SpecialStory story(time1, sources, area, period, "text");

  const string fun = "text";

  Settings::set("text::value", "This is the value of the string.");

  require(story, "fi", fun, "This is the value of the string.", "text");
  require(story, "sv", fun, "This is the value of the string.", "text");
  require(story, "en", fun, "This is the value of the string.", "text");

  Settings::set("text::value", "@data/special_text.fi");
  require(story, "fi", fun, "Suomenkielinen teksti.");

  Settings::set("text::value", "@data/special_text.en");
  require(story, "fi", fun, "English text.");

  Settings::set("text::value", "@data/special_text.php");
  require(story, "fi", fun, "Suomenkieltä PHP-ohjelmasta");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test SpecialStory::date()
 */
// ----------------------------------------------------------------------

void special_date()
{
  using namespace std;
  using namespace TextGen;

  AnalysisSources sources;
  WeatherArea area("25,60");
  TextGenPosixTime time1(2000, 1, 1, 6, 30);
  TextGenPosixTime time2(2000, 1, 2, 9, 30);
  WeatherPeriod period(time1, time2);
  SpecialStory story(time1, sources, area, period, "date");

  const string fun = "date";

  // Note! plain text formatter used

  // startformat and endformat included
  Settings::set("textgen::part1::story::date::plain::startformat", "%d.%m.%Y %H:%M - ");
  Settings::set("textgen::part1::story::date::plain::endformat", "%d.%m.%Y %H:%M");

  require(story, "fi", fun, "01.01.2000 06:30 - 02.01.2000 09:30", "textgen::part1::story::date");
  require(story, "sv", fun, "01.01.2000 06:30 - 02.01.2000 09:30", "textgen::part1::story::date");
  require(story, "en", fun, "01.01.2000 06:30 - 02.01.2000 09:30", "textgen::part1::story::date");

  Settings::clear();
  // endformat modified
  Settings::set("textgen::part1::story::date::plain::startformat", "%d.%m.%Y %H - ");
  Settings::set("textgen::part1::story::date::plain::endformat", "%H");

  require(story, "fi", fun, "01.01.2000 06 - 09", "textgen::part1::story::date");
  require(story, "sv", fun, "01.01.2000 06 - 09", "textgen::part1::story::date");
  require(story, "en", fun, "01.01.2000 06 - 09", "textgen::part1::story::date");

  Settings::clear();
  // endformat missing
  Settings::set("textgen::part1::story::date::plain::startformat", "%d.%m.%Y %H:%M");

  require(story, "fi", fun, "01.01.2000 06:30", "textgen::part1::story::date");
  require(story, "sv", fun, "01.01.2000 06:30", "textgen::part1::story::date");
  require(story, "en", fun, "01.01.2000 06:30", "textgen::part1::story::date");

  Settings::clear();
  // endformat is here NOT formatter specific
  Settings::set("textgen::part1::story::date::plain::startformat", "%d.%m.%Y %H:%M - ");
  Settings::set("textgen::part1::story::date::endformat", "%d.%m.%Y %H:%M");

  require(story, "fi", fun, "01.01.2000 06:30 - 02.01.2000 09:30", "textgen::part1::story::date");
  require(story, "sv", fun, "01.01.2000 06:30 - 02.01.2000 09:30", "textgen::part1::story::date");
  require(story, "en", fun, "01.01.2000 06:30 - 02.01.2000 09:30", "textgen::part1::story::date");

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
    TEST(special_none);
    TEST(special_text);
    TEST(special_date);
  }

};  // class tests

}  // namespace SpecialStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  NFmiSettings::Set("textgen::specialseason", "true");
  Settings::set(NFmiSettings::ToString());

  using namespace SpecialStoryTest;

  MessageLogger::open("my.log");

  cout << endl << "SpecialStory tests" << endl << "=================" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
