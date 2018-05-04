#include <regression/tframe.h>
#include "DictionaryFactory.h"
#include "LocationPhrase.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;

namespace LocationPhraseTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  // Should succeed
  LocationPhrase s(std::string("helsinki"));
  // Should succeed
  LocationPhrase s2(s);
  // Should succeed
  LocationPhrase s3 = s;
  // Should succeed
  LocationPhrase s4("tampere");
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

  {
    LocationPhrase loc("tukholma");
    ;
    if (loc.realize(*english) != "Tukholma")
      TEST_FAILED("realization of tukholma in English failed");

    if (loc.realize(*finnish) != "Tukholma")
      TEST_FAILED("realization of tukholma in Finnish failed");
  }

  {
    LocationPhrase loc("pohjois-lappi");
    ;
    if (loc.realize(*english) != "Pohjois-Lappi")
      TEST_FAILED("realization of pohjois-lappi in English failed, got " + loc.realize(*english));

    if (loc.realize(*finnish) != "Pohjois-Lappi")
      TEST_FAILED("realization of pohjois-lappi in Finnish failed, got " + loc.realize(*finnish));
  }

  {
    LocationPhrase loc("nummi-pusula");
    ;
    if (loc.realize(*english) != "Nummi-Pusula")
      TEST_FAILED("realization of nummi-pusula in English failed, got " + loc.realize(*english));

    if (loc.realize(*finnish) != "Nummi-Pusula")
      TEST_FAILED("realization of pohjois-lappi in Finnish failed, got " + loc.realize(*finnish));
  }

  {
    LocationPhrase loc("ulan bator");
    ;
    if (loc.realize(*english) != "Ulan Bator")
      TEST_FAILED("realization of ulan bator in English failed");

    if (loc.realize(*finnish) != "Ulan Bator")
      TEST_FAILED("realization of ulan bator in Finnish failed");
  }

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

}  // namespace LocationPhraseTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  cout << endl << "LocationPhrase tester" << endl << "=====================" << endl;
  LocationPhraseTest::tests t;
  return t.run();
}
