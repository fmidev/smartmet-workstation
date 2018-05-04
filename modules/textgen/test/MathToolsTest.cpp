#include <regression/tframe.h>
#include <calculator/MathTools.h>
#include <boost/lexical_cast.hpp>

namespace MathToolsTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test MathTools::to_precision()
 */
// ----------------------------------------------------------------------

void to_precision()
{
  if (MathTools::to_precision(10, 1) != 10) TEST_FAILED("to_precision(10,1) failed");

  if (MathTools::to_precision(15, 5) != 15) TEST_FAILED("to_precision(15,5) failed");

  if (MathTools::to_precision(15, 2) != 16) TEST_FAILED("to_precision(15,2) failed");

  if (MathTools::to_precision(15, 10) != 20) TEST_FAILED("to_precision(15,10) failed");

  if (MathTools::to_precision(15, 20) != 20) TEST_FAILED("to_precision(15,20) failed");

  if (MathTools::to_precision(15, 50) != 0) TEST_FAILED("to_precision(15,50) failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MathTools::mean()
 */
// ----------------------------------------------------------------------

void mean()
{
  if (MathTools::mean(0, 1) != 0.5) TEST_FAILED("mean(0,1) failed to return 0.5");
  if (MathTools::mean(2, 4) != 3) TEST_FAILED("mean(2,4) failed to return 3");

  if (MathTools::mean(1, 2, 3) != 2) TEST_FAILED("mean(1,2,3) failed to return 2");
  if (MathTools::mean(1, 3, 5) != 3) TEST_FAILED("mean(1,3,5) failed to return 3");

  if (MathTools::mean(1, 2, 3, 4) != 2.5) TEST_FAILED("mean(1,2,3,4) failed to return 2.5");
  if (MathTools::mean(1, 2, 4, 5) != 3) TEST_FAILED("mean(1,2,4,5) failed to return 3");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MathTools::min()
 */
// ----------------------------------------------------------------------

void min()
{
  if (MathTools::min(0, 1) != 0) TEST_FAILED("min(0,1) failed to return 0");
  if (MathTools::min(4, 2) != 2) TEST_FAILED("min(4,2) failed to return 2");

  if (MathTools::min(1, 2, 3) != 1) TEST_FAILED("min(1,2,3) failed to return 1");
  if (MathTools::min(5, 3, 1) != 1) TEST_FAILED("min(5,3,1) failed to return 1");
  if (MathTools::min(5, 1, 3) != 1) TEST_FAILED("min(5,1,3) failed to return 1");

  if (MathTools::min(1, 2, 3, 4) != 1) TEST_FAILED("min(1,2,3,4) failed to return 1");
  if (MathTools::min(2, 3, 4, 1) != 1) TEST_FAILED("min(2,3,4,1) failed to return 1");
  if (MathTools::min(3, 4, 1, 2) != 1) TEST_FAILED("min(3,4,1,2) failed to return 1");
  if (MathTools::min(4, 1, 2, 1) != 1) TEST_FAILED("min(4,1,2,1) failed to return 1");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MathTools::max()
 */
// ----------------------------------------------------------------------

void max()
{
  if (MathTools::max(0, 1) != 1) TEST_FAILED("max(0,1) failed to return 1");
  if (MathTools::max(4, 2) != 4) TEST_FAILED("max(4,2) failed to return 4");

  if (MathTools::max(1, 2, 3) != 3) TEST_FAILED("max(1,2,3) failed to return 3");
  if (MathTools::max(5, 3, 1) != 5) TEST_FAILED("max(5,3,1) failed to return 5");
  if (MathTools::max(5, 1, 3) != 5) TEST_FAILED("max(5,1,3) failed to return 5");

  if (MathTools::max(1, 2, 3, 4) != 4) TEST_FAILED("max(1,2,3,4) failed to return 4");
  if (MathTools::max(2, 3, 4, 1) != 4) TEST_FAILED("max(2,3,4,1) failed to return 4");
  if (MathTools::max(3, 4, 1, 2) != 4) TEST_FAILED("max(3,4,1,2) failed to return 4");
  if (MathTools::max(4, 1, 2, 1) != 4) TEST_FAILED("max(4,1,2,1) failed to return 4");

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
    TEST(to_precision);
    TEST(mean);
    TEST(min);
    TEST(max);
  }

};  // class tests

}  // namespace MathToolsTest

int main(void)
{
  using namespace std;
  cout << endl << "MathTools tester" << endl << "================" << endl;
  MathToolsTest::tests t;
  return t.run();
}
