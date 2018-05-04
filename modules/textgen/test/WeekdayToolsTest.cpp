#include <regression/tframe.h>
#include "WeekdayTools.h"
#include <calculator/TextGenPosixTime.h>
#include <newbase/NFmiSettings.h>
#include <boost/locale.hpp>

using namespace std;

namespace WeekdayToolsTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::on_weekday()
 */
// ----------------------------------------------------------------------

void on_weekday()
{
  using namespace TextGen;

  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 1)) != "7-na")
    TEST_FAILED("June 1 2003 should return '7-na'");
  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 2)) != "1-na")
    TEST_FAILED("June 2 2003 should return '1-na'");
  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 3)) != "2-na")
    TEST_FAILED("June 3 2003 should return '2-na'");
  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 4)) != "3-na")
    TEST_FAILED("June 4 2003 should return '3-na'");
  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 5)) != "4-na")
    TEST_FAILED("June 5 2003 should return '4-na'");
  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 6)) != "5-na")
    TEST_FAILED("June 6 2003 should return '5-na'");
  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 7)) != "6-na")
    TEST_FAILED("June 7 2003 should return '6-na'");
  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 8)) != "7-na")
    TEST_FAILED("June 8 2003 should return '7-na'");
  if (WeekdayTools::on_weekday(TextGenPosixTime(2003, 6, 9)) != "1-na")
    TEST_FAILED("June 9 2003 should return '1-na'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::on_weekday_time()
 */
// ----------------------------------------------------------------------

void on_weekday_time()
{
  using namespace TextGen;

  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 1)) != "7-na kello")
    TEST_FAILED("June 1 2003 should return '7-na kello'");
  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 2)) != "1-na kello")
    TEST_FAILED("June 2 2003 should return '1-na kello'");
  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 3)) != "2-na kello")
    TEST_FAILED("June 3 2003 should return '2-na kello'");
  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 4)) != "3-na kello")
    TEST_FAILED("June 4 2003 should return '3-na kello'");
  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 5)) != "4-na kello")
    TEST_FAILED("June 5 2003 should return '4-na kello'");
  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 6)) != "5-na kello")
    TEST_FAILED("June 6 2003 should return '5-na kello'");
  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 7)) != "6-na kello")
    TEST_FAILED("June 7 2003 should return '6-na kello'");
  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 8)) != "7-na kello")
    TEST_FAILED("June 8 2003 should return '7-na kello'");
  if (WeekdayTools::on_weekday_time(TextGenPosixTime(2003, 6, 9)) != "1-na kello")
    TEST_FAILED("June 9 2003 should return '1-na kello'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::night_against_weekday()
 */
// ----------------------------------------------------------------------

void night_against_weekday()
{
  using namespace TextGen;

  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 1)) != "7-vastaisena yona")
    TEST_FAILED("June 1 2003 should return '7-vastaisena yönä'");
  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 2)) != "1-vastaisena yona")
    TEST_FAILED("June 2 2003 should return '1-vastaisena yönä'");
  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 3)) != "2-vastaisena yona")
    TEST_FAILED("June 3 2003 should return '2-vastaisena yönä'");
  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 4)) != "3-vastaisena yona")
    TEST_FAILED("June 4 2003 should return '3-vastaisena yönä'");
  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 5)) != "4-vastaisena yona")
    TEST_FAILED("June 5 2003 should return '4-vastaisena yönä'");
  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 6)) != "5-vastaisena yona")
    TEST_FAILED("June 6 2003 should return '5-vastaisena yönä'");
  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 7)) != "6-vastaisena yona")
    TEST_FAILED("June 7 2003 should return '6-vastaisena yönä'");
  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 8)) != "7-vastaisena yona")
    TEST_FAILED("June 8 2003 should return '7-vastaisena yönä'");
  if (WeekdayTools::night_against_weekday(TextGenPosixTime(2003, 6, 9)) != "1-vastaisena yona")
    TEST_FAILED("June 9 2003 should return '1-vastaisena yönä'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::until_weekday_morning()
 */
// ----------------------------------------------------------------------

void until_weekday_morning()
{
  using namespace TextGen;

  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 1)) != "7-aamuun")
    TEST_FAILED("June 1 2003 should return '7-aamuun'");
  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 2)) != "1-aamuun")
    TEST_FAILED("June 2 2003 should return '1-aamuun'");
  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 3)) != "2-aamuun")
    TEST_FAILED("June 3 2003 should return '2-aamuun'");
  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 4)) != "3-aamuun")
    TEST_FAILED("June 4 2003 should return '3-aamuun'");
  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 5)) != "4-aamuun")
    TEST_FAILED("June 5 2003 should return '4-aamuun'");
  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 6)) != "5-aamuun")
    TEST_FAILED("June 6 2003 should return '5-aamuun'");
  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 7)) != "6-aamuun")
    TEST_FAILED("June 7 2003 should return '6-aamuun'");
  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 8)) != "7-aamuun")
    TEST_FAILED("June 8 2003 should return '7-aamuun'");
  if (WeekdayTools::until_weekday_morning(TextGenPosixTime(2003, 6, 9)) != "1-aamuun")
    TEST_FAILED("June 9 2003 should return '1-aamuun'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::until_weekday_evening()
 */
// ----------------------------------------------------------------------

void until_weekday_evening()
{
  using namespace TextGen;

  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 1)) != "7-iltaan")
    TEST_FAILED("June 1 2003 should return '7-iltaan'");
  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 2)) != "1-iltaan")
    TEST_FAILED("June 2 2003 should return '1-iltaan'");
  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 3)) != "2-iltaan")
    TEST_FAILED("June 3 2003 should return '2-iltaan'");
  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 4)) != "3-iltaan")
    TEST_FAILED("June 4 2003 should return '3-iltaan'");
  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 5)) != "4-iltaan")
    TEST_FAILED("June 5 2003 should return '4-iltaan'");
  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 6)) != "5-iltaan")
    TEST_FAILED("June 6 2003 should return '5-iltaan'");
  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 7)) != "6-iltaan")
    TEST_FAILED("June 7 2003 should return '6-iltaan'");
  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 8)) != "7-iltaan")
    TEST_FAILED("June 8 2003 should return '7-iltaan'");
  if (WeekdayTools::until_weekday_evening(TextGenPosixTime(2003, 6, 9)) != "1-iltaan")
    TEST_FAILED("June 9 2003 should return '1-iltaan'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::from_weekday_morning()
 */
// ----------------------------------------------------------------------

void from_weekday_morning()
{
  using namespace TextGen;

  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 1)) != "7-aamusta")
    TEST_FAILED("June 1 2003 should return '7-aamusta'");
  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 2)) != "1-aamusta")
    TEST_FAILED("June 2 2003 should return '1-aamusta'");
  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 3)) != "2-aamusta")
    TEST_FAILED("June 3 2003 should return '2-aamusta'");
  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 4)) != "3-aamusta")
    TEST_FAILED("June 4 2003 should return '3-aamusta'");
  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 5)) != "4-aamusta")
    TEST_FAILED("June 5 2003 should return '4-aamusta'");
  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 6)) != "5-aamusta")
    TEST_FAILED("June 6 2003 should return '5-aamusta'");
  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 7)) != "6-aamusta")
    TEST_FAILED("June 7 2003 should return '6-aamusta'");
  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 8)) != "7-aamusta")
    TEST_FAILED("June 8 2003 should return '7-aamusta'");
  if (WeekdayTools::from_weekday_morning(TextGenPosixTime(2003, 6, 9)) != "1-aamusta")
    TEST_FAILED("June 9 2003 should return '1-aamusta'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::from_weekday_evening()
 */
// ----------------------------------------------------------------------

void from_weekday_evening()
{
  using namespace TextGen;

  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 1)) != "7-illasta")
    TEST_FAILED("June 1 2003 should return '7-illasta'");
  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 2)) != "1-illasta")
    TEST_FAILED("June 2 2003 should return '1-illasta'");
  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 3)) != "2-illasta")
    TEST_FAILED("June 3 2003 should return '2-illasta'");
  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 4)) != "3-illasta")
    TEST_FAILED("June 4 2003 should return '3-illasta'");
  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 5)) != "4-illasta")
    TEST_FAILED("June 5 2003 should return '4-illasta'");
  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 6)) != "5-illasta")
    TEST_FAILED("June 6 2003 should return '5-illasta'");
  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 7)) != "6-illasta")
    TEST_FAILED("June 7 2003 should return '6-illasta'");
  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 8)) != "7-illasta")
    TEST_FAILED("June 8 2003 should return '7-illasta'");
  if (WeekdayTools::from_weekday_evening(TextGenPosixTime(2003, 6, 9)) != "1-illasta")
    TEST_FAILED("June 9 2003 should return '1-illasta'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::until_weekday_time()
 */
// ----------------------------------------------------------------------

void until_weekday_time()
{
  using namespace TextGen;

  if (WeekdayTools::until_weekday_time(TextGenPosixTime(2003, 6, 1, 6)) != "7-aamuun")
    TEST_FAILED("June 1 2003 06:00 should return '7-aamuun'");

  if (WeekdayTools::until_weekday_time(TextGenPosixTime(2003, 6, 1, 18)) != "7-iltaan")
    TEST_FAILED("June 1 2003 18:00 should return '7-iltaan'");

  try
  {
    WeekdayTools::until_weekday_time(TextGenPosixTime(2003, 6, 1));
    TEST_FAILED("June 1 2003 00:00 should throw");
  }
  catch (...)

      TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::from_weekday_time()
 */
// ----------------------------------------------------------------------

void from_weekday_time()
{
  using namespace TextGen;

  if (WeekdayTools::from_weekday_time(TextGenPosixTime(2003, 6, 1, 6)) != "7-aamusta")
    TEST_FAILED("June 1 2003 06:00 should return '7-aamusta'");

  if (WeekdayTools::from_weekday_time(TextGenPosixTime(2003, 6, 1, 18)) != "7-illasta")
    TEST_FAILED("June 1 2003 18:00 should return '7-illasta'");

  try
  {
    WeekdayTools::from_weekday_time(TextGenPosixTime(2003, 6, 1));
    TEST_FAILED("June 1 2003 00:00 should throw");
  }
  catch (...)

      TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::on_weekday_morning()
 */
// ----------------------------------------------------------------------

void on_weekday_morning()
{
  using namespace TextGen;

  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 1)) != "7-aamulla")
    TEST_FAILED("June 1 2003 should return '7-aamulla'");
  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 2)) != "1-aamulla")
    TEST_FAILED("June 2 2003 should return '1-aamulla'");
  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 3)) != "2-aamulla")
    TEST_FAILED("June 3 2003 should return '2-aamulla'");
  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 4)) != "3-aamulla")
    TEST_FAILED("June 4 2003 should return '3-aamulla'");
  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 5)) != "4-aamulla")
    TEST_FAILED("June 5 2003 should return '4-aamulla'");
  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 6)) != "5-aamulla")
    TEST_FAILED("June 6 2003 should return '5-aamulla'");
  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 7)) != "6-aamulla")
    TEST_FAILED("June 7 2003 should return '6-aamulla'");
  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 8)) != "7-aamulla")
    TEST_FAILED("June 8 2003 should return '7-aamulla'");
  if (WeekdayTools::on_weekday_morning(TextGenPosixTime(2003, 6, 9)) != "1-aamulla")
    TEST_FAILED("June 9 2003 should return '1-aamulla'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::on_weekday_forenoon()
 */
// ----------------------------------------------------------------------

void on_weekday_forenoon()
{
  using namespace TextGen;

  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 1)) != "7-aamupaivalla")
    TEST_FAILED("June 1 2003 should return '7-aamupaivalla'");
  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 2)) != "1-aamupaivalla")
    TEST_FAILED("June 2 2003 should return '1-aamupaivalla'");
  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 3)) != "2-aamupaivalla")
    TEST_FAILED("June 3 2003 should return '2-aamupaivalla'");
  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 4)) != "3-aamupaivalla")
    TEST_FAILED("June 4 2003 should return '3-aamupaivalla'");
  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 5)) != "4-aamupaivalla")
    TEST_FAILED("June 5 2003 should return '4-aamupaivalla'");
  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 6)) != "5-aamupaivalla")
    TEST_FAILED("June 6 2003 should return '5-aamupaivalla'");
  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 7)) != "6-aamupaivalla")
    TEST_FAILED("June 7 2003 should return '6-aamupaivalla'");
  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 8)) != "7-aamupaivalla")
    TEST_FAILED("June 8 2003 should return '7-aamupaivalla'");
  if (WeekdayTools::on_weekday_forenoon(TextGenPosixTime(2003, 6, 9)) != "1-aamupaivalla")
    TEST_FAILED("June 9 2003 should return '1-aamupaivalla'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::on_weekday_afternoon()
 */
// ----------------------------------------------------------------------

void on_weekday_afternoon()
{
  using namespace TextGen;

  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 1)) != "7-iltapaivalla")
    TEST_FAILED("June 1 2003 should return '7-iltapaivalla'");
  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 2)) != "1-iltapaivalla")
    TEST_FAILED("June 2 2003 should return '1-iltapaivalla'");
  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 3)) != "2-iltapaivalla")
    TEST_FAILED("June 3 2003 should return '2-iltapaivalla'");
  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 4)) != "3-iltapaivalla")
    TEST_FAILED("June 4 2003 should return '3-iltapaivalla'");
  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 5)) != "4-iltapaivalla")
    TEST_FAILED("June 5 2003 should return '4-iltapaivalla'");
  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 6)) != "5-iltapaivalla")
    TEST_FAILED("June 6 2003 should return '5-iltapaivalla'");
  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 7)) != "6-iltapaivalla")
    TEST_FAILED("June 7 2003 should return '6-iltapaivalla'");
  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 8)) != "7-iltapaivalla")
    TEST_FAILED("June 8 2003 should return '7-iltapaivalla'");
  if (WeekdayTools::on_weekday_afternoon(TextGenPosixTime(2003, 6, 9)) != "1-iltapaivalla")
    TEST_FAILED("June 9 2003 should return '1-iltapaivalla'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WeekdayTools::on_weekday_evening()
 */
// ----------------------------------------------------------------------

void on_weekday_evening()
{
  using namespace TextGen;

  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 1)) != "7-illalla")
    TEST_FAILED("June 1 2003 should return '7-illalla'");
  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 2)) != "1-illalla")
    TEST_FAILED("June 2 2003 should return '1-illalla'");
  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 3)) != "2-illalla")
    TEST_FAILED("June 3 2003 should return '2-illalla'");
  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 4)) != "3-illalla")
    TEST_FAILED("June 4 2003 should return '3-illalla'");
  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 5)) != "4-illalla")
    TEST_FAILED("June 5 2003 should return '4-illalla'");
  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 6)) != "5-illalla")
    TEST_FAILED("June 6 2003 should return '5-illalla'");
  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 7)) != "6-illalla")
    TEST_FAILED("June 7 2003 should return '6-illalla'");
  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 8)) != "7-illalla")
    TEST_FAILED("June 8 2003 should return '7-illalla'");
  if (WeekdayTools::on_weekday_evening(TextGenPosixTime(2003, 6, 9)) != "1-illalla")
    TEST_FAILED("June 9 2003 should return '1-illalla'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

//! The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(on_weekday);
    TEST(on_weekday_time);
    TEST(night_against_weekday);
    TEST(until_weekday_morning);
    TEST(until_weekday_evening);
    TEST(from_weekday_morning);
    TEST(from_weekday_evening);
    TEST(until_weekday_time);
    TEST(from_weekday_time);
    TEST(on_weekday_morning);
    TEST(on_weekday_forenoon);
    TEST(on_weekday_afternoon);
    TEST(on_weekday_evening);
  }

};  // class tests

}  // namespace WeekdayToolsTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");

  cout << endl << "WeekdayTools tester" << endl << "===================" << endl;
  WeekdayToolsTest::tests t;
  return t.run();
}
