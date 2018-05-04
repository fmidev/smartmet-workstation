#include <regression/tframe.h>
#include "NullDictionary.h"
#include "Real.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

namespace RealTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  // Should succeed
  Real n1(12.9);

  // Should succeed
  Real n2(n1);

  // Should succeed
  Real n3 = n1;

  Real n4(0.0);
  n4 = n1;

  TEST_PASSED();
}

//! Test realize() for reals
void realize_real(void)
{
  using namespace TextGen;

  Real n1(0.0);
  if (n1.realize(NullDictionary()) != "0.0") TEST_FAILED("realization of real(0.0) failed");

  Real n2(123.4321);
  if (n2.realize(NullDictionary()) != "123.4") TEST_FAILED("realization of real(123.4321) failed");

  Real n3(123.4321, 2);
  if (n3.realize(NullDictionary()) != "123.43")
    TEST_FAILED("realization of real(123.4321,2) failed");

  Real n4(123.4321, 3, false);
  if (n4.realize(NullDictionary()) != "123,432")
    TEST_FAILED("realization of real(123.4321,2.false) failed");

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
    TEST(realize_real);
  }

};  // class tests

}  // namespace RealTest

int main(void)
{
  cout << endl << "Real tester" << endl << "===========" << endl;
  RealTest::tests t;
  return t.run();
}
