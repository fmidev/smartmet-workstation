#include <regression/tframe.h>
#include "MorningAndEveningPeriodGenerator.h"
#include <calculator/WeatherPeriod.h>
#include <calculator/TextGenPosixTime.h>

using namespace std;

namespace MorningAndEveningPeriodGeneratorTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test MorningAndEveningPeriodGenerator::size()
 */
// ----------------------------------------------------------------------

void size(void)
{
  using namespace TextGen;

  // A simple period from 12 to 18
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 1, 18, 0));

    {
      MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 9, 18, 22);
      if (generator.size() != 0) TEST_FAILED("Size should be zero - 12-18 does not fit into 9-18");
    }

    {
      MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 12, 18, 22);
      if (generator.size() != 1) TEST_FAILED("Size should be one - 12-18 fits into 12-18");
    }
  }

  // A two-day period from 12 to 18
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 2, 18, 0));

    {
      MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 9, 18, 22);
      if (generator.size() != 4) TEST_FAILED("Size should be 4, 18-22,22-6,6-9,9-18");
    }

    {
      MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 12, 18, 22);
      if (generator.size() != 5) TEST_FAILED("Size should be 5, 12-18,18-22,22-6,6-9,9-18");
    }
  }

  // A three-day period from 12 to 18
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 3, 18, 0));

    {
      MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 9, 18, 22);
      if (generator.size() != 8)
        TEST_FAILED("Size should be 8, 18-22,22-6,6-9,9-18,18-22,22-6,6-9,9-18");
    }
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MorningAndEveningPeriodGenerator::period()
 */
// ----------------------------------------------------------------------

void period(void)
{
  using namespace TextGen;

  // A simple period from 12 to 18
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 1, 18, 0));

    {
      MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 9, 18, 22);
      try
      {
        (void)generator.period(1);
        TEST_FAILED("Should have thrown - there are 0 periods");
      }
      catch (...)
      {
      }
    }

    {
      MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 12, 18, 22);
      WeatherPeriod p = generator.period(1);
      if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 12), TextGenPosixTime(2000, 1, 1, 18)))
        TEST_FAILED("Failed to generate correct first 12-18 period");
    }
  }

  // A two-day period from 12 to 18
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 2, 18, 0));

    MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 9, 18, 22);
    WeatherPeriod p = generator.period(1);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 18), TextGenPosixTime(2000, 1, 1, 22)))
      TEST_FAILED("Failed to generator 1st of 4 periods (18-22)");
    p = generator.period(2);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 22), TextGenPosixTime(2000, 1, 2, 6)))
      TEST_FAILED("Failed to generator 2nd of 4 periods (22-06)");
    p = generator.period(3);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 2, 6), TextGenPosixTime(2000, 1, 2, 9)))
      TEST_FAILED("Failed to generator 3rd of 4 periods (06-09)");
    p = generator.period(4);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 2, 9), TextGenPosixTime(2000, 1, 2, 18)))
      TEST_FAILED("Failed to generator 4th of 4 periods (09-18)");
  }

  // A three-day period from 12 to 18
  {
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 12, 0), TextGenPosixTime(2000, 1, 3, 18, 0));

    MorningAndEveningPeriodGenerator generator(period, 6, 9, 18, 22, 6, 9, 18, 22);

    WeatherPeriod p = generator.period(1);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 18), TextGenPosixTime(2000, 1, 1, 22)))
      TEST_FAILED("Failed to generator 1st of 8 periods (18-22)");
    p = generator.period(2);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 1, 22), TextGenPosixTime(2000, 1, 2, 6)))
      TEST_FAILED("Failed to generator 2nd of 8 periods (22-06)");
    p = generator.period(3);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 2, 6), TextGenPosixTime(2000, 1, 2, 9)))
      TEST_FAILED("Failed to generator 3rd of 8 periods (06-09)");
    p = generator.period(4);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 2, 9), TextGenPosixTime(2000, 1, 2, 18)))
      TEST_FAILED("Failed to generator 4th of 8 periods (09-18)");
    p = generator.period(5);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 2, 18), TextGenPosixTime(2000, 1, 2, 22)))
      TEST_FAILED("Failed to generator 5th of 8 periods (18-22)");
    p = generator.period(6);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 2, 22), TextGenPosixTime(2000, 1, 3, 6)))
      TEST_FAILED("Failed to generator 6th of 8 periods (22-06)");
    p = generator.period(7);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 3, 6), TextGenPosixTime(2000, 1, 3, 9)))
      TEST_FAILED("Failed to generator 7th of 8 periods (06-09)");
    p = generator.period(8);
    if (p != WeatherPeriod(TextGenPosixTime(2000, 1, 3, 9), TextGenPosixTime(2000, 1, 3, 18)))
      TEST_FAILED("Failed to generator 8th of 8 periods (09-18)");
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

}  // namespace MorningAndEveningPeriodGeneratorTest

int main(void)
{
  cout << endl
       << "MorningAndEveningPeriodGenerator tester" << endl
       << "=======================================" << endl;
  MorningAndEveningPeriodGeneratorTest::tests t;
  return t.run();
}
