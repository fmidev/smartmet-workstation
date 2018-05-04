#include <regression/tframe.h>
#include "DictionaryFactory.h"
#include "Phrase.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;

namespace PhraseTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  // Should succeed
  Phrase s(std::string("foobar"));
  // Should succeed
  Phrase s2(s);
  // Should succeed
  Phrase s3 = s;
  // Should succeed
  Phrase s4("foobar");
  s4 = s;

  TEST_PASSED();
}

//! Test realize()
void realize(void)
{
  using namespace boost;
  using namespace TextGen;

  shared_ptr<Dictionary> finnish(DictionaryFactory::create("mysql"));
  finnish->init("fi");

  shared_ptr<Dictionary> english(DictionaryFactory::create("mysql"));
  english->init("en");

  Phrase s1("lampotila");
  ;
  if (s1.realize(*english) != "temperature")
    TEST_FAILED("realization of lampötila in English failed");

  if (s1.realize(*finnish) != "lämpötila")
    TEST_FAILED("realization of lampötila in Finnish failed");

  TEST_PASSED();
}

//! The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(structors);
    TEST(realize);
  }

};  // class tests

}  // namespace PhraseTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  cout << endl << "Phrase tester" << endl << "=============" << endl;
  PhraseTest::tests t;
  return t.run();
}
