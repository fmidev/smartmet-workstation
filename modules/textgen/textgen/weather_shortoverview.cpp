// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::WeatherStory::shortoverview
 */
// ======================================================================

#include "WeatherStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "Integer.h"
#include <calculator/MathTools.h>
#include "MessageLogger.h"
#include <calculator/NullPeriodGenerator.h>
#include "Paragraph.h"
#include <calculator/RangeAcceptor.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherPeriodTools.h>
#include <calculator/WeatherResult.h>
#include "WeekdayTools.h"

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generator brief overview story on weather
 *
 * \return The story
 *
 * \see page_weather_shortoverview
 *
 * \todo Precipitation form output is missing
 */
// ----------------------------------------------------------------------

Paragraph WeatherStory::shortoverview() const
{
  MessageLogger log("WeatherStory::shortoverview");

  using namespace Settings;
  using namespace WeatherPeriodTools;

  Paragraph paragraph;
  Sentence c_sentence;
  Sentence r_sentence;

  const bool c_fullrange = optional_bool(itsVar + "::cloudiness::fullrange", true);
  const int c_starthour = optional_hour(itsVar + "::cloudiness::day::starthour", 0);
  const int c_endhour = optional_hour(itsVar + "::cloudiness::day::endhour", 0);
  const int c_maxstarthour = optional_hour(itsVar + "::cloudiness::day::maxstarthour", c_starthour);
  const int c_minendhour = optional_hour(itsVar + "::cloudiness::day::minendhour", c_endhour);

  const int c_clear = optional_percentage(itsVar + "::cloudiness::clear", 40);
  const int c_cloudy = optional_percentage(itsVar + "::cloudiness::cloudy", 70);

  const int c_single_limit = optional_percentage(itsVar + "::cloudiness::single_limit", 60);
  const int c_double_limit = optional_percentage(itsVar + "::cloudiness::double_limit", 20);

  const int r_starthour = optional_hour(itsVar + "::precipitation::day::starthour", 0);
  const int r_endhour = optional_hour(itsVar + "::precipitation::day::endhour", 0);
  const int r_maxstarthour =
      optional_hour(itsVar + "::precipitation::day::maxstarthour", r_starthour);
  const int r_minendhour = optional_hour(itsVar + "::precipitation::day::minendhour", r_endhour);

  const double r_rainy = optional_double(itsVar + "::precipitation::rainy", 1);
  const double r_partly_rainy = optional_double(itsVar + "::precipitation::partly_rainy", 0.1);
  const int r_unstable = optional_percentage(itsVar + "::precipitation::unstable", 50);

  GridForecaster forecaster;

  // Generate cloudiness story first

  {
    RangeAcceptor n1limits, n3limits;
    n1limits.upperLimit(c_clear);
    n3limits.lowerLimit(c_cloudy);

    boost::shared_ptr<WeatherPeriodGenerator> periods;
    if (c_fullrange)
      periods = boost::shared_ptr<WeatherPeriodGenerator>(new NullPeriodGenerator(itsPeriod));
    else
      periods = boost::shared_ptr<WeatherPeriodGenerator>(
          new HourPeriodGenerator(itsPeriod, c_starthour, c_endhour, c_maxstarthour, c_minendhour));

    const WeatherResult n1result = forecaster.analyze(itsVar + "::fake::clear_percentage",
                                                      itsSources,
                                                      Cloudiness,
                                                      Mean,
                                                      (c_fullrange ? Percentage : Mean),
                                                      (c_fullrange ? NullFunction : Percentage),
                                                      itsArea,
                                                      *periods,
                                                      DefaultAcceptor(),
                                                      DefaultAcceptor(),
                                                      n1limits);

    const WeatherResult n3result = forecaster.analyze(itsVar + "::fake::cloudy_percentage",
                                                      itsSources,
                                                      Cloudiness,
                                                      Mean,
                                                      (c_fullrange ? Percentage : Mean),
                                                      (c_fullrange ? NullFunction : Percentage),
                                                      itsArea,
                                                      *periods,
                                                      DefaultAcceptor(),
                                                      DefaultAcceptor(),
                                                      n3limits);

    if (n1result.value() == kFloatMissing || n3result.value() == kFloatMissing)
      throw TextGenError("Cloudiness not available");

    log << "Cloudiness clear  Mean(Mean) = " << n1result << endl;
    log << "Cloudiness cloudy Mean(Mean) = " << n3result << endl;

    // n1+n2+n3 = 100
    const float n1 = n1result.value();
    const float n3 = n3result.value();
    const float n2 = 100 - n1 - n3;

    if (n1 >= c_single_limit)
      c_sentence << "enimmakseen"
                 << "selkeaa";
    else if (n2 >= c_single_limit)
      c_sentence << "enimmakseen"
                 << "puolipilvista";
    else if (n3 >= c_single_limit)
      c_sentence << "enimmakseen"
                 << "pilvista";
    else if (n1 < c_double_limit)
      c_sentence << "enimmakseen"
                 << "pilvista"
                 << "tai"
                 << "puolipilvista";
    else if (n3 < c_double_limit)
      c_sentence << "enimmakseen"
                 << "selkeaa"
                 << "tai"
                 << "puolipilvista";
    else
      c_sentence << "vaihtelevaa pilvisyytta";
  }

  // Sentence on rain

  bool unstable_weather = false;

  {
    typedef vector<WeatherResult> container;
    container results;
    HourPeriodGenerator generator(itsPeriod, r_starthour, r_endhour, r_maxstarthour, r_minendhour);

    WeatherPeriod last_rainy_period = generator.period(1);
    WeatherPeriod last_partly_rainy_period = generator.period(1);

    const int days = generator.size();
    int rainy_days = 0;
    int partly_rainy_days = 0;

    for (HourPeriodGenerator::size_type i = 1; i <= generator.size(); i++)
    {
      WeatherPeriod period = generator.period(i);

      const string day = "day" + lexical_cast<string>(i);
      const string var = itsVar + "::fake::" + day + "::precipitation";

      const WeatherResult result =
          forecaster.analyze(var, itsSources, Precipitation, Mean, Sum, itsArea, period);

      if (result.value() == kFloatMissing) throw TextGenError("Precipitation not available");

      log << "Precipitation Mean(Sum) day " << i << " = " << result << endl;

      if (result.value() >= r_rainy)
      {
        ++rainy_days;
        last_rainy_period = period;
      }
      if (result.value() >= r_partly_rainy)
      {
        ++partly_rainy_days;
        last_partly_rainy_period = period;
      }

      results.push_back(result);
    }

    if (rainy_days == 0 && partly_rainy_days == 0)
      r_sentence << "poutaa";
    else if (rainy_days == 1 && partly_rainy_days == 1)
      r_sentence << WeekdayTools::on_weekday(last_rainy_period.localStartTime()) << "sadetta";
    else if (rainy_days == 0 && partly_rainy_days == 1)
      r_sentence << WeekdayTools::on_weekday(last_partly_rainy_period.localStartTime()) << "paikoin"
                 << "sadetta";
    else if (100 * static_cast<float>(rainy_days) / days >= r_unstable)
    {
      r_sentence << "saa on epavakaista";
      unstable_weather = true;
    }
    else
      r_sentence << "ajoittain sateista";
  }

  if (unstable_weather)
    paragraph << r_sentence;
  else
  {
    c_sentence << Delimiter(",") << r_sentence;
    paragraph << c_sentence;
  }

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
