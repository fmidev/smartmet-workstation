// ======================================================================
/*!
 * \file
 *†\brief Implementation of method TextGen::RoadStory::condition_overview
 */
// ======================================================================

#include "RoadStory.h"
#include "DebugTextFormatter.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "MessageLogger.h"
#include "MorningAndEveningPeriodGenerator.h"
#include "Paragraph.h"
#include "PeriodPhraseFactory.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include "ValueAcceptor.h"
#include <calculator/WeatherResult.h>

#include <boost/lexical_cast.hpp>

#include <map>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief The various road conditions
 *
 * The numbers are fixed by the Ties‰‰-malli, and must not be changed.
 */
// ----------------------------------------------------------------------

enum RoadConditionType
{
  DRY = 1,
  MOIST,
  WET,
  SLUSH,
  FROST,
  PARTLY_ICY,
  ICY,
  SNOW
};

const int min_condition = DRY;
const int max_condition = SNOW;

// ----------------------------------------------------------------------
/*!
 * \brief Return the importance of a road condition
 */
// ----------------------------------------------------------------------

unsigned int condition_importance(int theCondition)
{
  switch (RoadConditionType(theCondition))
  {
    case DRY:
      return 0;
    case MOIST:
      return 1;
    case WET:
      return 2;
    case SNOW:
      return 3;
    case SLUSH:
      return 4;
    case FROST:
      return 5;
    case PARTLY_ICY:
      return 6;
    case ICY:
      return 7;
  }
  throw TextGenError("Unknown road condition in condition_importance");
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert road condition enumeration to string
 */
// ----------------------------------------------------------------------

const char* condition_name(RoadConditionType theCondition)
{
  switch (theCondition)
  {
    case DRY:
      return "dry";
    case MOIST:
      return "moist";
    case WET:
      return "wet";
    case SLUSH:
      return "slush";
    case FROST:
      return "frost";
    case PARTLY_ICY:
      return "partly_icy";
    case ICY:
      return "icy";
    case SNOW:
      return "snow";
  }
  throw TextGenError("Unknown road condition in condition_name");
}

// ----------------------------------------------------------------------
/*!
 * \brief Road condition percentages
 */
// ----------------------------------------------------------------------

class ConditionPercentages
{
 public:
  ~ConditionPercentages() {}
  ConditionPercentages() : itsPercentages(max_condition - min_condition + 1, 0) {}
  const double& operator[](int i) const { return itsPercentages[i - 1]; }
  double& operator[](int i) { return itsPercentages[i - 1]; }
 private:
  std::vector<double> itsPercentages;
};

// ----------------------------------------------------------------------
/*!
 * \brief Calculate road condition percentages for given period
 *
 * \todo Optimize the integration loop so that you first calculate
 *       the minimum and maximum road condition, or possibly just
 *       the maximum, and then loop over those values only.
 */
// ----------------------------------------------------------------------

const ConditionPercentages calculate_percentages(const WeatherPeriod& thePeriod,
                                                 int thePeriodIndex,
                                                 const TextGen::AnalysisSources& theSources,
                                                 const TextGen::WeatherArea& theArea,
                                                 const std::string& theVar)
{
  GridForecaster forecaster;

  ConditionPercentages percentages;
  for (int i = min_condition; i <= max_condition; i++)
  {
    const RoadConditionType c = RoadConditionType(i);

    const string fake = (theVar + "::fake::period" + lexical_cast<string>(thePeriodIndex) + "::" +
                         condition_name(c) + "::percentage");

    ValueAcceptor condfilter;
    condfilter.value(c);

    WeatherResult result = forecaster.analyze(fake,
                                              theSources,
                                              RoadCondition,
                                              Mean,
                                              Percentage,
                                              theArea,
                                              thePeriod,
                                              DefaultAcceptor(),
                                              DefaultAcceptor(),
                                              condfilter);

    percentages[c] = result.value();
  }

  return percentages;
}

// ----------------------------------------------------------------------
/*!
 * \brief Find the most frequent condition type
 *
 * Equal cases are resolved by the importance of the type
 */
// ----------------------------------------------------------------------

RoadConditionType find_most_general_condition(const ConditionPercentages& thePercentages)
{
  int ibest = min_condition;
  for (int i = min_condition + 1; i <= max_condition; i++)
  {
    if (thePercentages[i] > thePercentages[ibest])
      ibest = i;
    else if (thePercentages[i] == thePercentages[ibest] &&
             condition_importance(i) > condition_importance(ibest))
      ibest = i;
  }
  return RoadConditionType(ibest);
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a places-phrase for a condition
 */
// ----------------------------------------------------------------------

const char* condition_places_phrase(RoadConditionType theType,
                                    double thePercentage,
                                    int theGenerallyLimit,
                                    int theManyPlacesLimit,
                                    int theSomePlacesLimit)
{
  if (thePercentage < theSomePlacesLimit) return "";
  if (thePercentage < theManyPlacesLimit) return "paikoin";
  if (thePercentage < theGenerallyLimit) return "monin paikoin";

  switch (theType)
  {
    case DRY:
    case MOIST:
    case WET:
    case SNOW:
    case SLUSH:
      return "";
    case FROST:
    case PARTLY_ICY:
    case ICY:
      return "yleisesti";
  }

  // should never happen
  return "";
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a single road condition phrase
 */
// ----------------------------------------------------------------------

const Sentence condition_phrase(RoadConditionType theType,
                                double thePercentage,
                                int theGenerallyLimit,
                                int theManyPlacesLimit,
                                int theSomePlacesLimit)
{
  const char* places_phrase = condition_places_phrase(
      theType, thePercentage, theGenerallyLimit, theManyPlacesLimit, theSomePlacesLimit);

  Sentence tiet_ovat;
  Sentence teilla_on;
  tiet_ovat << "tiet ovat" << places_phrase;
  teilla_on << "teilla on" << places_phrase;

  switch (theType)
  {
    case DRY:
      return (tiet_ovat << "kuivia");
    case MOIST:
      return (tiet_ovat << "kosteita");
    case WET:
      return (tiet_ovat << "markia");
    case SNOW:
      return (teilla_on << "lunta");
    case SLUSH:
      return (teilla_on << "sohjoa");
    case FROST:
      return (teilla_on << "kuuraa");
    case PARTLY_ICY:
      return (tiet_ovat << "osittain jaisia");
    case ICY:
      return (tiet_ovat << "jaisia");
  }

  // Unreachable

  throw TextGenError("Internal error in roadcondition_overview condition_phrase function");
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate the second phrase of the manyplaces-type sentence
 */
// ----------------------------------------------------------------------

const Sentence second_places_sentence(RoadConditionType thePrimaryType,
                                      RoadConditionType theSecondaryType)
{
  Sentence sentence;
  switch (thePrimaryType)
  {
    case ICY:
    case PARTLY_ICY:
      switch (theSecondaryType)
      {
        case PARTLY_ICY:
          sentence << "paikoin"
                   << "osittain jaisia";
          break;
        case FROST:
          sentence << "paikoin"
                   << "kuuraisia";
          break;
        case ICY:
          sentence << "paikoin"
                   << "jaisia";
          break;
        case SLUSH:
        case SNOW:
        case WET:
        case MOIST:
        case DRY:
          break;
      }
      break;
    case FROST:
      switch (theSecondaryType)
      {
        case ICY:
          sentence << "paikoin"
                   << "tiet ovat (sivulause)"
                   << "jaisia";
          break;
        case PARTLY_ICY:
          sentence << "paikoin"
                   << "tiet ovat (sivulause)"
                   << "osittain jaisia";
          break;
        case FROST:
        case SLUSH:
        case SNOW:
        case WET:
        case MOIST:
        case DRY:
          break;
      }
      break;
    case SLUSH:
      switch (theSecondaryType)
      {
        case ICY:
          sentence << "paikoin"
                   << "tiet ovat (sivulause)"
                   << "jaisia";
          break;
        case PARTLY_ICY:
          sentence << "paikoin"
                   << "tiet ovat (sivulause)"
                   << "osittain jaisia";
          break;
        case FROST:
          sentence << "paikoin"
                   << "kuuraa";
          break;
        case SNOW:
          sentence << "paikoin"
                   << "lunta";
          break;
        case SLUSH:
        case WET:
        case MOIST:
        case DRY:
          break;
      }
      break;
    case SNOW:
      switch (theSecondaryType)
      {
        case ICY:
          sentence << "paikoin"
                   << "tiet ovat (sivulause)"
                   << "jaisia";
          break;
        case PARTLY_ICY:
          sentence << "paikoin"
                   << "tiet ovat (sivulause)"
                   << "osittain jaisia";
          break;
        case FROST:
          sentence << "paikoin"
                   << "kuuraa";
          break;
        case SLUSH:
          sentence << "paikoin"
                   << "sohjoa";
          break;
        case SNOW:
        case WET:
        case MOIST:
        case DRY:
          break;
      }
      break;
    case WET:
    case MOIST:
      switch (theSecondaryType)
      {
        case ICY:
          sentence << "paikoin"
                   << "jaisia";
          break;
        case PARTLY_ICY:
          sentence << "paikoin"
                   << "osittain jaisia";
          break;
        case FROST:
          sentence << "paikoin"
                   << "kuuraisia";
          break;
        case SLUSH:
          sentence << "paikoin"
                   << "sohjoisia";
          break;
        case SNOW:
          sentence << "paikoin"
                   << "lumisia";
          break;
        case WET:
          sentence << "paikoin"
                   << "markia";
          break;
        case MOIST:
          sentence << "paikoin"
                   << "kosteita";
          break;
        case DRY:
          break;
      }
      break;
    case DRY:
      // should never happen
      break;
  }
  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a sentence from road condition percentages
 */
// ----------------------------------------------------------------------

const Sentence condition_sentence(const ConditionPercentages& thePercentages, const string& theVar)
{
  Sentence sentence;

  // Read the related configuration settings

  using Settings::optional_percentage;
  const int generally_limit = optional_percentage(theVar + "::generally_limit", 90);
  const int manyplaces_limit = optional_percentage(theVar + "::manyplaces_limit", 50);
  const int someplaces_limit = optional_percentage(theVar + "::someplaces_limit", 10);

  // Find the most frequent condition

  RoadConditionType firsttype = find_most_general_condition(thePercentages);

  // Handle the case when the type dominates the others

  if (thePercentages[firsttype] >= generally_limit)
  {
    sentence << condition_phrase(
        firsttype, thePercentages[firsttype], generally_limit, manyplaces_limit, someplaces_limit);
    return sentence;
  }

  // List all "someplaces" types that occur in order of importance
  // The set may include firsttype, if no condition occurs in many places

  map<int, RoadConditionType, std::greater<int> > someplacestypes;

  for (int i = min_condition; i <= max_condition; i++)
  {
    const RoadConditionType condition = RoadConditionType(i);

    if (thePercentages[condition] >= someplaces_limit &&
        thePercentages[condition] < manyplaces_limit)
    {
      const int importance = condition_importance(condition);
      someplacestypes.insert(make_pair(importance, condition));
    }
  }

  // Handle the case when there is one type in many places
  // Note that we always ignore it if the "many places" type is DRY,
  // instead we report on two most important "places" types

  if (thePercentages[firsttype] >= manyplaces_limit && firsttype != DRY)
  {
    sentence << condition_phrase(
        firsttype, thePercentages[firsttype], generally_limit, manyplaces_limit, someplaces_limit);

    // Then report the most important "someplaces" condition

    if (!someplacestypes.empty())
    {
      Sentence s = second_places_sentence(firsttype, someplacestypes.begin()->second);
      if (!s.empty()) sentence << Delimiter(",") << s;
    }
    return sentence;
  }

  // Report on the two most important types

  if (someplacestypes.empty())
  {
    sentence << condition_phrase(
        firsttype, thePercentages[firsttype], generally_limit, manyplaces_limit, someplaces_limit);
    return sentence;
  }

  firsttype = someplacestypes.begin()->second;

  sentence << condition_phrase(
      firsttype, thePercentages[firsttype], generally_limit, manyplaces_limit, someplaces_limit);

  if (someplacestypes.size() > 1)
  {
    RoadConditionType secondtype = (++someplacestypes.begin())->second;

    if (firsttype == ICY || firsttype == PARTLY_ICY)
    {
      if (secondtype == PARTLY_ICY)
        sentence << "tai"
                 << "osittain jaisia";
      else if (secondtype == FROST)
        sentence << "tai"
                 << "kuuraisia";
    }
    else if (firsttype == SLUSH && secondtype == SNOW)
      sentence << "tai"
               << "lunta";
    else if (firsttype == WET && secondtype == MOIST)
      sentence << "tai"
               << "kosteita";
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period describes a morning
 *
 * The periods are morning, day, evening, night
 */
// ----------------------------------------------------------------------

bool is_morning(const WeatherPeriod& thePeriod, const string& theVar)
{
  using Settings::require_hour;

  const int starthour = require_hour(theVar + "::morning::starthour");
  const int endhour = require_hour(theVar + "::day::starthour");

  const TextGenPosixTime& starttime = thePeriod.localStartTime();
  const TextGenPosixTime& endtime = thePeriod.localEndTime();

  if (!TimeTools::isSameDay(starttime, endtime)) return false;

  if (starttime.GetHour() < starthour || starttime.GetHour() > endhour) return false;

  if (endtime.GetHour() < starthour || endtime.GetHour() > endhour) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period describes a day
 *
 * The periods are morning, day, evening, night
 */
// ----------------------------------------------------------------------

bool is_day(const WeatherPeriod& thePeriod, const string& theVar)
{
  using Settings::require_hour;

  const int starthour = require_hour(theVar + "::day::starthour");
  const int endhour = require_hour(theVar + "::evening::starthour");

  const TextGenPosixTime& starttime = thePeriod.localStartTime();
  const TextGenPosixTime& endtime = thePeriod.localEndTime();

  if (!TimeTools::isSameDay(starttime, endtime)) return false;

  if (starttime.GetHour() < starthour || starttime.GetHour() > endhour) return false;

  if (endtime.GetHour() < starthour || endtime.GetHour() > endhour) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period describes an evening
 *
 * The periods are morning, day, evening, night
 */
// ----------------------------------------------------------------------

bool is_evening(const WeatherPeriod& thePeriod, const string& theVar)
{
  using Settings::require_hour;

  const int starthour = require_hour(theVar + "::evening::starthour");
  const int endhour = require_hour(theVar + "::night::starthour");

  const TextGenPosixTime& starttime = thePeriod.localStartTime();
  const TextGenPosixTime& endtime = thePeriod.localEndTime();

  if (!TimeTools::isSameDay(starttime, endtime)) return false;

  if (starttime.GetHour() < starthour || starttime.GetHour() > endhour) return false;

  if (endtime.GetHour() < starthour || endtime.GetHour() > endhour) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period describes a night
 *
 * The periods are morning, day, evening, night
 */
// ----------------------------------------------------------------------

bool is_night(const WeatherPeriod& thePeriod, const string& theVar)
{
  using Settings::require_hour;

  const int starthour = require_hour(theVar + "::night::starthour");
  const int endhour = require_hour(theVar + "::morning::starthour");

  const TextGenPosixTime& starttime = thePeriod.localStartTime();
  const TextGenPosixTime& endtime = thePeriod.localEndTime();

  if (!TimeTools::isNextDay(starttime, endtime)) return false;

  if (starttime.GetHour() < starthour && starttime.GetHour() > endhour) return false;

  if (endtime.GetHour() < starthour && endtime.GetHour() > endhour) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a timephrase for a period some weekday
 */
// ----------------------------------------------------------------------

const Sentence during_period_phrase_weekday(const WeatherPeriod& thePeriod, const string& theVar)
{
  const int startdaynumber = thePeriod.localStartTime().GetWeekday();
  const int enddaynumber = thePeriod.localEndTime().GetWeekday();

  const string startday = lexical_cast<string>(startdaynumber);
  const string endday = lexical_cast<string>(enddaynumber);

  Sentence sentence;
  if (is_morning(thePeriod, theVar))
    sentence << startday + "-aamuna";
  else if (is_day(thePeriod, theVar))
    sentence << startday + "-paivalla";
  else if (is_evening(thePeriod, theVar))
    sentence << startday + "-iltana";
  else if (is_night(thePeriod, theVar))
    sentence << endday + "-vastaisena yona";
  else
  {
    ostringstream msg;
    msg << "roadcondition overview: "
        << "Could not classify period " << thePeriod.localStartTime() << " ... "
        << thePeriod.localEndTime() << " as morning, day, evening or night";
    throw TextGenError(msg.str());
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a timephrase for a period tomorrow
 */
// ----------------------------------------------------------------------

const Sentence during_period_phrase_tomorrow(const WeatherPeriod& thePeriod, const string& theVar)
{
  Sentence sentence;
  if (is_morning(thePeriod, theVar))
    sentence << "huomisaamuna";
  else if (is_day(thePeriod, theVar))
    sentence << "huomenna paivalla";
  else if (is_evening(thePeriod, theVar))
    sentence << "huomisiltana";
  else if (is_night(thePeriod, theVar))
    return during_period_phrase_weekday(thePeriod, theVar);
  else
  {
    ostringstream msg;
    msg << "roadcondition overview: "
        << "Could not classify period " << thePeriod.localStartTime() << " ... "
        << thePeriod.localEndTime() << " as morning, day, evening or night";
    throw TextGenError(msg.str());
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a timephrase for a period today
 */
// ----------------------------------------------------------------------

const Sentence during_period_phrase_today(const WeatherPeriod& thePeriod, const string& theVar)
{
  Sentence sentence;
  if (is_morning(thePeriod, theVar))
    sentence << "aamulla";
  else if (is_day(thePeriod, theVar))
    sentence << "paivalla";
  else if (is_evening(thePeriod, theVar))
    sentence << "illalla";
  else if (is_night(thePeriod, theVar))
    sentence << "yolla";
  else
  {
    ostringstream msg;
    msg << "roadcondition overview: "
        << "Could not classify period " << thePeriod.localStartTime() << " ... "
        << thePeriod.localEndTime() << " as morning, day, evening or night";
    throw TextGenError(msg.str());
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a timephrase for a period
 */
// ----------------------------------------------------------------------

const Sentence during_period_phrase(const WeatherPeriod& thePeriod,
                                    const TextGenPosixTime& theLastTime,
                                    const TextGenPosixTime& theForecastTime,
                                    const string& theVar)
{
  if (TimeTools::isSameDay(theLastTime, thePeriod.localStartTime()))
    return during_period_phrase_today(thePeriod, theVar);

  if (TimeTools::isSameDay(theForecastTime, thePeriod.localStartTime()))
    return during_period_phrase_today(thePeriod, theVar);

  if (TimeTools::isNextDay(theForecastTime, thePeriod.localStartTime()))
    return during_period_phrase_tomorrow(thePeriod, theVar);

  return during_period_phrase_weekday(thePeriod, theVar);
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a timephrase for a period starting some weekday
 */
// ----------------------------------------------------------------------

const Sentence starting_period_phrase_weekday(const WeatherPeriod& thePeriod, const string& theVar)
{
  const int startdaynumber = thePeriod.localStartTime().GetWeekday();
  const int enddaynumber = thePeriod.localEndTime().GetWeekday();

  const string startday = lexical_cast<string>(startdaynumber);
  const string endday = lexical_cast<string>(enddaynumber);

  Sentence sentence;
  if (is_morning(thePeriod, theVar))
    sentence << startday + "-aamusta alkaen";
  else if (is_day(thePeriod, theVar))
    sentence << startday + "-aamupaivasta alkaen";
  else if (is_evening(thePeriod, theVar))
    sentence << startday + "-illasta alkaen";
  else if (is_night(thePeriod, theVar))
    sentence << endday + "-vastaisesta yosta alkaen";
  else
  {
    ostringstream msg;
    msg << "roadcondition overview: "
        << "Could not classify period " << thePeriod.localStartTime() << " ... "
        << thePeriod.localEndTime() << " as morning, day, evening or night";
    throw TextGenError(msg.str());
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a timephrase for a period starting tomorrow
 */
// ----------------------------------------------------------------------

const Sentence starting_period_phrase_tomorrow(const WeatherPeriod& thePeriod, const string& theVar)
{
  Sentence sentence;
  if (is_morning(thePeriod, theVar))
    sentence << "huomisaamusta alkaen";
  else if (is_day(thePeriod, theVar))
    sentence << "huomisaamupaivasta alkaen";
  else if (is_evening(thePeriod, theVar))
    sentence << "huomisillasta alkaen";
  else if (is_night(thePeriod, theVar))
    return starting_period_phrase_weekday(thePeriod, theVar);
  else
  {
    ostringstream msg;
    msg << "roadcondition overview: "
        << "Could not classify period " << thePeriod.localStartTime() << " ... "
        << thePeriod.localEndTime() << " as morning, day, evening or night";
    throw TextGenError(msg.str());
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a timephrase for a period starting today
 */
// ----------------------------------------------------------------------

const Sentence starting_period_phrase_today(const WeatherPeriod& thePeriod, const string& theVar)
{
  Sentence sentence;
  if (is_morning(thePeriod, theVar))
    sentence << "aamusta alkaen";
  else if (is_day(thePeriod, theVar))
    sentence << "aamupaivasta alkaen";
  else if (is_evening(thePeriod, theVar))
    sentence << "illasta alkaen";
  else if (is_night(thePeriod, theVar))
    sentence << "yosta alkaen";
  else
  {
    ostringstream msg;
    msg << "roadcondition overview: "
        << "Could not classify period " << thePeriod.localStartTime() << " ... "
        << thePeriod.localEndTime() << " as morning, day, evening or night";
    throw TextGenError(msg.str());
  }

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a timephrase for a starting period
 */
// ----------------------------------------------------------------------

const Sentence starting_period_phrase(const WeatherPeriod& thePeriod,
                                      const TextGenPosixTime& theLastTime,
                                      const TextGenPosixTime& theForecastTime,
                                      const string& theVar)
{
  if (TimeTools::isSameDay(theLastTime, thePeriod.localStartTime()))
    return starting_period_phrase_today(thePeriod, theVar);

  if (TimeTools::isSameDay(theForecastTime, thePeriod.localStartTime()))
    return starting_period_phrase_today(thePeriod, theVar);

  if (TimeTools::isNextDay(theForecastTime, thePeriod.localStartTime()))
    return starting_period_phrase_tomorrow(thePeriod, theVar);

  return starting_period_phrase_weekday(thePeriod, theVar);
}
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate story on road conditions
 *
 * \return The story
 *
 *†\see page_roadcondition_overview
 */
// ----------------------------------------------------------------------

Paragraph RoadStory::condition_overview() const
{
  MessageLogger log("RoadStory::condition_overview");

  Paragraph paragraph;

  // the period we wish to analyze is at most 30 hours,
  // but of course must not exceed the period length itself

  const int maxhours = Settings::optional_int(itsVar + "::maxhours", 30);

  const TextGenPosixTime time1(itsPeriod.localStartTime());
  TextGenPosixTime time2 = TimeTools::addHours(time1, maxhours);
  if (itsPeriod.localEndTime().IsLessThan(time2)) time2 = itsPeriod.localEndTime();

  const WeatherPeriod fullperiod(time1, time2);

  log << "Analysis period is " << time1 << " ... " << time2 << endl;

  // Calculate the results for each period

  MorningAndEveningPeriodGenerator generator(fullperiod, itsVar);

  if (generator.size() == 0)
  {
    log << "Analysis period is too short!" << endl;
    log << paragraph;
    return paragraph;
  }

  // Calculate the percentages, initial sentences and their
  // dummy realizations

  vector<WeatherPeriod> periods;
  vector<Sentence> sentences;
  vector<string> realizations;

  DebugTextFormatter formatter;

  log << "Individual period results:" << endl;
  for (unsigned int i = 1; i <= generator.size(); i++)
  {
    const WeatherPeriod period = generator.period(i);

    const ConditionPercentages result =
        calculate_percentages(period, i, itsSources, itsArea, itsVar);

    const Sentence sentence = condition_sentence(result, itsVar);
    const string realization = formatter.format(sentence);

    periods.push_back(period);
    sentences.push_back(sentence);
    realizations.push_back(realization);

    log << period.localStartTime() << " ... " << period.localEndTime() << ": " << realization
        << endl;

    for (int j = min_condition; j <= max_condition; j++)
      log << '\t' << condition_name(RoadConditionType(j)) << '\t' << result[j] << " %" << endl;
  }

  // Algorithm:
  // For each start period
  //   Find the number of similar periods
  //   Generate common text for the periods
  //   Skip the similar periods
  // Next

  // Some old date guaranteed to be different than any
  // period to be handled:

  TextGenPosixTime last_mentioned_date(1970, 1, 1);

  Sentence sentence;

  for (unsigned int i = 0; i < periods.size(); i++)
  {
    // Common periods will be inclusive range i...j

    unsigned int j;
    for (j = i; j < periods.size() - 1; j++)
      if (realizations[i] != realizations[j + 1]) break;

    // Generate the text

    if (TimeTools::isSameDay(last_mentioned_date, periods[i].localStartTime()))
      sentence << Delimiter(",");
    else
    {
      paragraph << sentence;
      sentence.clear();
    }

    if (i == j)
    {
      sentence << during_period_phrase(periods[i], last_mentioned_date, itsForecastTime, itsVar);
    }
    else
    {
      sentence << starting_period_phrase(periods[i], last_mentioned_date, itsForecastTime, itsVar);
    }
    sentence << sentences[i];

    // update the last mentioned date
    last_mentioned_date = periods[i].localStartTime();

    // and move on to the next periods
    i = j;
  }
  paragraph << sentence;

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
