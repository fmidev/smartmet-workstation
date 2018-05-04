#include <regression/tframe.h>
#include <calculator/IntervalPeriodGenerator.h>
#include <calculator/WeatherPeriod.h>
#include <calculator/TextGenPosixTime.h>
#include <newbase/NFmiSettings.h>

#include <boost/locale.hpp>

using namespace std;

namespace IntervalPeriodGeneratorTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test IntervalPeriodGenerator::size()
 */
// ----------------------------------------------------------------------

void size(void)
{
  using namespace TextGen;

  {
    // A simple period from 0 to 24
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 0, 0), TextGenPosixTime(2000, 1, 2, 0, 0));

    {
      // 0-3, 3-6, 6-9, 9-12, 12-15, 15-18, 18-21, 21-24
      IntervalPeriodGenerator generator(period, 0, 3, 3);
      if (generator.size() != 8) TEST_FAILED("0-24 divides into 8 3-hour intervals");
    }

    {
      // 0-4, 4-8, 8-12, 12-16, 16-20, 20-24
      IntervalPeriodGenerator generator(period, 0, 4, 4);
      if (generator.size() != 6) TEST_FAILED("0-24 divides into 6 4-hour intervals");
    }

    {
      // 0-6, 6-12, 12-18, 18-24
      IntervalPeriodGenerator generator(period, 0, 6, 6);
      if (generator.size() != 4) TEST_FAILED("0-24 divides into 4 6-hour intervals");
    }

    {
      // 0-8, 8-16, 16-24
      IntervalPeriodGenerator generator(period, 0, 8, 8);
      if (generator.size() != 3) TEST_FAILED("0-24 divides into 3 8-hour intervals");
    }

    {
      // 0-12, 12-24
      IntervalPeriodGenerator generator(period, 0, 12, 12);
      if (generator.size() != 2) TEST_FAILED("0-24 divides into 2 12-hour intervals");
    }

    {
      // 0-24
      IntervalPeriodGenerator generator(period, 0, 24, 24);
      if (generator.size() != 1) TEST_FAILED("0-24 divides into 1 24-hour intervals");
    }

    {
      // 2-5, 5-8, 8-11, 11-14, 14-17, 17-20, 20-23
      IntervalPeriodGenerator generator(period, 2, 3, 3);
      if (generator.size() != 7)
        TEST_FAILED("0-24 with start hour 2 divides into 7 3-hour intervals");
    }

    {
      // 2-6, 6-10, 10-14, 14-18, 18-22
      IntervalPeriodGenerator generator(period, 2, 4, 4);
      if (generator.size() != 5)
        TEST_FAILED("0-24 with start hour 2 divides into 5 4-hour intervals");
    }

    {
      // 2-8, 8-14, 14-20
      IntervalPeriodGenerator generator(period, 2, 6, 6);
      if (generator.size() != 3)
        TEST_FAILED("0-24 with start hour 2 divides into 3 6-hour intervals");
    }

    {
      // 2-10, 10-18
      IntervalPeriodGenerator generator(period, 2, 8, 8);
      if (generator.size() != 2)
        TEST_FAILED("0-24 with start hour 2 divides into 2 8-hour intervals");
    }

    {
      // 2-14
      IntervalPeriodGenerator generator(period, 2, 12, 12);
      if (generator.size() != 1)
        TEST_FAILED("0-24 with start hour 2 divides into 1 12-hour intervals");
    }

    {
      IntervalPeriodGenerator generator(period, 2, 24, 24);
      if (generator.size() != 0)
        TEST_FAILED("0-24 with start hour 2 divides into 0 24-hour intervals");
    }

    {
      // 0-2, 2-5, 5-8, 8-11, 11-14, 14-17, 17-20, 20-23
      IntervalPeriodGenerator generator(period, 2, 3, 2);
      if (generator.size() != 8)
        TEST_FAILED("0-24 with start hour 2 divides into 8 2/3-hour intervals");
    }

    {
      // 0-2, 2-6, 6-10, 10-14, 14-18, 18-22, 22-24
      IntervalPeriodGenerator generator(period, 2, 4, 2);
      if (generator.size() != 7)
        TEST_FAILED("0-24 with start hour 2 divides into 5 2/4-hour intervals");
    }

    {
      // 2-8, 8-14, 14-20, 20-24
      IntervalPeriodGenerator generator(period, 2, 6, 3);
      if (generator.size() != 4)
        TEST_FAILED("0-24 with start hour 2 divides into 3 3/6-hour intervals");
    }

    {
      // 2-10, 10-18, 18-24
      IntervalPeriodGenerator generator(period, 2, 8, 4);
      if (generator.size() != 3)
        TEST_FAILED("0-24 with start hour 2 divides into 3 4/8-hour intervals");
    }

    {
      // 2-14, 14-24
      IntervalPeriodGenerator generator(period, 2, 12, 6);
      if (generator.size() != 2)
        TEST_FAILED("0-24 with start hour 2 divides into 1 6/12-hour intervals");
    }

    {
      // 2-24
      IntervalPeriodGenerator generator(period, 2, 24, 12);
      if (generator.size() != 1)
        TEST_FAILED("0-24 with start hour 2 divides into 1 12/24-hour intervals");
    }
  }

  {
    // A simple several day period from 0 to 24
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 0, 0), TextGenPosixTime(2000, 1, 11, 0, 0));

    {
      // 1-2, 2-3, 3-4, ... 9-10, 10-11
      IntervalPeriodGenerator generator(period, 0, 24, 24);

      if (generator.size() != 10) TEST_FAILED("24h divides 10 days into 10 intervals");
    }

    {
      // 1-4, 4-7, 7-10
      IntervalPeriodGenerator generator(period, 0, 3 * 24, 3 * 24);
      if (generator.size() != 3) TEST_FAILED("72h divides 10 days into 3 intervals");
    }
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test IntervalPeriodGenerator::period()
 */
// ----------------------------------------------------------------------

void period(void)
{
  using namespace TextGen;

  {
    // A simple period from 0 to 24
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 0, 0), TextGenPosixTime(2000, 1, 2, 0, 0));

    {
      // 0-3, 3-6, 6-9, 9-12, 12-15, 15-18, 18-21, 21-24
      IntervalPeriodGenerator generator(period, 0, 3, 3);

      WeatherPeriod p = generator.period(1);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 0, 0), TextGenPosixTime(2000, 1, 1, 3, 0)))
        TEST_FAILED("Failed to generate period 1");

      p = generator.period(2);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 3, 0), TextGenPosixTime(2000, 1, 1, 6, 0)))
        TEST_FAILED("Failed to generate period 2");

      p = generator.period(3);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 6, 0), TextGenPosixTime(2000, 1, 1, 9, 0)))
        TEST_FAILED("Failed to generate period 3");

      p = generator.period(8);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 21, 0), TextGenPosixTime(2000, 1, 2, 0, 0)))
        TEST_FAILED("Failed to generate period 8");
    }

    {
      // 2-5, 5-8, 8-11, 11-14, 14-17, 17-20, 20-23
      IntervalPeriodGenerator generator(period, 2, 3, 3);

      WeatherPeriod p = generator.period(1);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 2, 0), TextGenPosixTime(2000, 1, 1, 5, 0)))
        TEST_FAILED("Failed to generate period 1");

      p = generator.period(7);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 20, 0), TextGenPosixTime(2000, 1, 1, 23, 0)))
        TEST_FAILED("Failed to generate period 2");
    }

    {
      // 0-2, 2-5, 5-8, 8-11, 11-14, 14-17, 17-20, 20-23
      IntervalPeriodGenerator generator(period, 2, 3, 2);

      WeatherPeriod p = generator.period(1);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 0, 0), TextGenPosixTime(2000, 1, 1, 2, 0)))
        TEST_FAILED("Failed to generate period 1");

      p = generator.period(2);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 2, 0), TextGenPosixTime(2000, 1, 1, 5, 0)))
        TEST_FAILED("Failed to generate period 2");

      p = generator.period(8);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 20, 0), TextGenPosixTime(2000, 1, 1, 23, 0)))
        TEST_FAILED("Failed to generate period 3");
    }

    {
      // 2-8, 8-14, 14-20, 20-24
      IntervalPeriodGenerator generator(period, 2, 6, 3);

      WeatherPeriod p = generator.period(1);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 2, 0), TextGenPosixTime(2000, 1, 1, 8, 0)))
        TEST_FAILED("Failed to generate period 1");

      p = generator.period(4);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 20, 0), TextGenPosixTime(2000, 1, 2, 0, 0)))
        TEST_FAILED("Failed to generate period 4");
    }
  }

  {
    // A simple several day period from 0 to 24
    WeatherPeriod period(TextGenPosixTime(2000, 1, 1, 0, 0), TextGenPosixTime(2000, 1, 11, 0, 0));

    {
      IntervalPeriodGenerator generator(period, 0, 24, 24);

      WeatherPeriod p = generator.period(1);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 0, 0), TextGenPosixTime(2000, 1, 2, 0, 0)))
        TEST_FAILED("Failed to generate day period 1");

      p = generator.period(3);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 3, 0, 0), TextGenPosixTime(2000, 1, 4, 0, 0)))
        TEST_FAILED("Failed to generate day period 3");
    }

    {
      IntervalPeriodGenerator generator(period, 0, 3 * 24, 3 * 24);

      WeatherPeriod p = generator.period(1);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 1, 0, 0), TextGenPosixTime(2000, 1, 4, 0, 0)))
        TEST_FAILED("Failed to generate 3-day period 1");

      p = generator.period(3);
      if (p !=
          WeatherPeriod(TextGenPosixTime(2000, 1, 7, 0, 0), TextGenPosixTime(2000, 1, 10, 0, 0)))
        TEST_FAILED("Failed to generate 3-day period 3");
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

}  // namespace IntervalPeriodGeneratorTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");

  cout << endl
       << "IntervalPeriodGenerator tester" << endl
       << "==============================" << endl;
  IntervalPeriodGeneratorTest::tests t;
  return t.run();
}
