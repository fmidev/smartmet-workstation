#include <regression/tframe.h>
#include "FireWarnings.h"
#include <calculator/TextGenPosixTime.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;

namespace FireWarningsTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test FireWarnings constructors
 */
// ----------------------------------------------------------------------

void constructors()
{
  using namespace std;
  using namespace TextGen;

  // 24.05.2005 has data
  try
  {
    TextGenPosixTime date(2005, 5, 24);
    FireWarnings warnings("data", date);
  }
  catch (...)
  {
    TEST_FAILED("Failed to construct a FireWarnings object for 24.05.2005");
  }

  // Next day finds previous day data
  try
  {
    TextGenPosixTime date(2005, 5, 25);
    FireWarnings warnings("data", date);
  }
  catch (...)
  {
    TEST_FAILED("Failed to construct a FireWarnings object for 25.05.2005");
  }

  // The following day must fail
  try
  {
    TextGenPosixTime date(2005, 5, 26);
    FireWarnings warnings("data", date);
    TEST_FAILED("Should have failed to construct a FireWarnings object for 26.05.2005");
  }
  catch (...)
  {
  }

  // And the day before 24.05.2005
  try
  {
    TextGenPosixTime date(2005, 5, 23);
    FireWarnings warnings("data", date);
    TEST_FAILED("Should have failed to construct a FireWarnings object for 23.05.2005");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test FireWarnings::state
 */
// ----------------------------------------------------------------------

void state()
{
  using namespace std;
  using namespace TextGen;

  TextGenPosixTime date(2005, 5, 24);
  FireWarnings warnings("data", date);

  if (warnings.state(1) != FireWarnings::FireWarning)
    TEST_FAILED("Warning state for area 1 should be 2 (FireWarning");

  if (warnings.state(2) != FireWarnings::None)
    TEST_FAILED("Warning state for area 2 should be 0 (None");

  if (warnings.state(3) != FireWarnings::FireWarning)
    TEST_FAILED("Warning state for area 3 should be 2 (FireWarning");

  if (warnings.state(32) != FireWarnings::GrassFireWarning)
    TEST_FAILED("Warning state for area 32 should be 1 (GrassFireWarning");

  try
  {
    warnings.state(0);
    TEST_FAILED("Should not get a warning code for area 0");
  }
  catch (...)
  {
  }

  try
  {
    warnings.state(100);
    TEST_FAILED("Should not get a warning code for area 100");
  }
  catch (...)
  {
  }

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
    TEST(constructors);
    TEST(state);
  }

};  // class tests

}  // namespace FireWarningsTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");

  using namespace FireWarningsTest;

  cout << endl << "FireWarnings tests" << endl << "==================" << endl;

  tests t;
  return t.run();
}
