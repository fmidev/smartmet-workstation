#include <regression/tframe.h>
#include "NightAndDayPeriodGenerator.h"
#include <calculator/WeatherPeriod.h>
#include <calculator/TextGenPosixTime.h>

using namespace std;

namespace NightAndDayPeriodGeneratorTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test NightAndDayPeriodGenerator::size()
 */
// ----------------------------------------------------------------------

void size(void)
{
  using namespace TextGen;

  // A simple period from 12 to 17
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 1, 17, 0));

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 18, 18, 06);
      if (generator.size() != 0) TEST_FAILED("Size should be zero - 12-18 does not fit into 12-17");
    }

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 17, 18, 06);
      if (generator.size() != 1) TEST_FAILED("Size should be one - 12-17 does fit into 12-18(17)");
    }
  }

  // A two-day period from 12 to 17
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 2, 17, 0));

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 18, 18, 06);
      if (generator.size() != 2) TEST_FAILED("Size should be 2, 12-18 + 18-06");
    }

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 17, 18, 06);
      if (generator.size() != 3) TEST_FAILED("Size should be 3, 12-18 + 18-06 + 06-17");
    }
  }

  // A three-day period from 12 to 17
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 3, 17, 0));

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 18, 18, 06);
      if (generator.size() != 4) TEST_FAILED("Size should be 4, 12-18 + 18-06 + 06-18 + 18-06");
    }

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 17, 18, 06);
      if (generator.size() != 5)
        TEST_FAILED("Size should be 4, 12-18 + 18-06 + 06-18 + 18-06 + 06-17");
    }
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NightAndDayPeriodGenerator::period()
 */
// ----------------------------------------------------------------------

void period(void)
{
  using namespace TextGen;

  // A simple period from 12 to 17
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 1, 17, 0));

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 18, 18, 06);
      try
      {
        generator.period(1);
        TEST_FAILED("period(1) should have failed - 12-18 does not fit into 12-17");
      }
      catch (...)
      {
      }
    }

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 17, 18, 06);

      WeatherPeriod p = generator.period(1);
      if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 12), TextGenPosixTime(2000, 1, 1, 17)))
        TEST_FAILED("Failed to generate correct first 12-17 period");
    }
  }

  // A two-day period from 12 to 17
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 2, 17, 0));

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 18, 18, 06);
      WeatherPeriod p = generator.period(1);
      if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 12), TextGenPosixTime(2000, 1, 1, 18)))
        TEST_FAILED("Failed to generate correct first 12-18 period");
      p = generator.period(2);
      if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 18), TextGenPosixTime(2000, 1, 2, 06)))
        TEST_FAILED("Failed to generate correct second 18-06 period");
    }

    {
      NightAndDayPeriodGenerator generator(period, 06, 18, 12, 17, 18, 06);
      WeatherPeriod p = generator.period(1);
      if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 12), TextGenPosixTime(2000, 1, 1, 18)))
        TEST_FAILED("Failed to generate correct first/2 12-18 period");
      p = generator.period(2);
      if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 18), TextGenPosixTime(2000, 1, 2, 06)))
        TEST_FAILED("Failed to generate correct second 18-06 period");
      p = generator.period(3);
      if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 2, 06), TextGenPosixTime(2000, 1, 2, 17)))
        TEST_FAILED("Failed to generate correct third 12-17 period");
    }
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * The actual test driver
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(size);
    TEST(period);
  }

};  // class tests

}  // namespace NightAndDayPeriodGeneratorTest

int main(void)
{
  cout << endl
       << "NightAndDayPeriodGenerator tester" << endl
       << "=================================" << endl;
  NightAndDayPeriodGeneratorTest::tests t;
  return t.run();
}
