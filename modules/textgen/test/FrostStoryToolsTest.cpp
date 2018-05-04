#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "FrostStoryTools.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace FrostStoryToolsTest
{
shared_ptr<TextGen::Dictionary> dict;

// ----------------------------------------------------------------------
/*!
 * \brief Test FrostStoryTools::frost_sentence
 */
// ----------------------------------------------------------------------

void frost_sentence() { TEST_NOT_IMPLEMENTED(); }
// ----------------------------------------------------------------------
/*!
 * \brief Test FrostStoryTools::severe_frost_sentence
 */
// ----------------------------------------------------------------------

void severe_frost_sentence() { TEST_NOT_IMPLEMENTED(); }
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
    TEST(frost_sentence);
    TEST(severe_frost_sentence);
  }

};  // class tests

}  // namespace FrostStoryToolsTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  NFmiSettings::Set("textgen::units::celsius::format", "phrase");
  Settings::set(NFmiSettings::ToString());

  using namespace FrostStoryToolsTest;

  cout << endl << "FrostStoryTools tests" << endl << "=====================" << endl;

  dict.reset(TextGen::DictionaryFactory::create("null"));

  tests t;
  return t.run();
}
