#include <regression/tframe.h>
#include "CssTextFormatter.h"
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
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
TextGen::CssTextFormatter formatter;

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
 * \brief Test SpecialStory::text()
 */
// ----------------------------------------------------------------------

void css_text()
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

  require(story, "fi", fun, "<span>\nThis is the value of the string.\n</span>\n", "text");
  require(story, "sv", fun, "<span>\nThis is the value of the string.\n</span>\n", "text");
  require(story, "en", fun, "<span>\nThis is the value of the string.\n</span>\n", "text");

  Settings::set("text::value", "@data/special_text.fi");
  require(story, "fi", fun, "<span>\n\nSuomenkielinen teksti.\n</span>\n");

  Settings::set("text::value", "@data/special_text.en");
  require(story, "fi", fun, "<span>\n\nEnglish text.\n</span>\n");

  Settings::set("text::value", "@data/special_text.php");
  require(story, "fi", fun, "<span>\n\nSuomenkieltä PHP-ohjelmasta\n</span>\n");

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
  void test(void) { TEST(css_text); }
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

  cout << endl << "CssTextFormatter tests" << endl << "=================" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
