#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "PlainTextFormatter.h"
#include "RoadStory.h"
#include "Story.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace RoadStoryTest
{
shared_ptr<TextGen::Dictionary> dict;
TextGen::PlainTextFormatter formatter;

// ----------------------------------------------------------------------
/*!
 * \brief Test RoadStory::daynightranges()
 */
// ----------------------------------------------------------------------

void roadtemperature_daynightranges() { TEST_NOT_IMPLEMENTED(); }
// ----------------------------------------------------------------------
/*!
 * \brief Test RoadStory::condition_overview()
 */
// ----------------------------------------------------------------------

void roadcondition_overview() { TEST_NOT_IMPLEMENTED(); }
// ----------------------------------------------------------------------
/*!
 * \brief Test RoadStory::warning_overview()
 */
// ----------------------------------------------------------------------

void roadwarning_overview() { TEST_NOT_IMPLEMENTED(); }
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
    TEST(roadtemperature_daynightranges);
    TEST(roadcondition_overview);
    TEST(roadwarning_overview);
  }

};  // class tests

}  // namespace RoadStoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  using namespace RoadStoryTest;

  cout << endl << "RoadStory tests" << endl << "===============" << endl;

  dict.reset(TextGen::DictionaryFactory::create("multimysql"));

  dict->init("fi");
  dict->init("sv");
  dict->init("en");

  tests t;
  return t.run();
}
