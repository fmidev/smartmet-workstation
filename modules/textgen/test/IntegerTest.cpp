#include <regression/tframe.h>
#include "NullDictionary.h"
#include "Integer.h"

#include <newbase/NFmiSettings.h>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;

namespace IntegerTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  // Should succeed
  Integer n1(12);

  // Should succeed
  Integer n2(n1);

  // Should succeed
  Integer n3 = n1;

  Integer n4(0);
  n4 = n1;

  TEST_PASSED();
}

//! Test realize() for ints
void realize_int(void)
{
  using namespace TextGen;

  Integer n1(0);
  if (n1.realize(NullDictionary()) != "0") TEST_FAILED("realization of int(0) failed");

  Integer n2(123);
  if (n2.realize(NullDictionary()) != "123") TEST_FAILED("realization of int(123) failed");

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
    TEST(realize_int);
  }

};  // class tests

}  // namespace IntegerTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");

  cout << endl << "Integer tester" << endl << "==============" << endl;
  IntegerTest::tests t;
  return t.run();
}
