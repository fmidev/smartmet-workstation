// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::RoadStory::warning_shortview
 */
// ======================================================================

#include "RoadStory.h"
#include "DebugTextFormatter.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "MessageLogger.h"
#include <calculator/HourPeriodGenerator.h>
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
 * \brief The various road warnings
 *
 * The numbers are fixed by the Ties‰‰-malli, and must not be changed.
 */
// ----------------------------------------------------------------------

enum RoadWarningType
{
  NORMAL = 0,
  FROSTY,
  ICING,
  SNOWY,
  FROSTSLIPPERY,
  WINDWARNING,
  WHIRLING,
  HEAVY_SNOWFALL,
  FASTWORSENING,
  SLEET_TO_ICY,
  RAIN_TO_ICY,
  ICYRAIN
};

const int min_warning = NORMAL;
const int max_warning = ICYRAIN;

// ----------------------------------------------------------------------
/*!
 * \brief Return the importance of a road warning
 */
// ----------------------------------------------------------------------

unsigned int warning_importance(int theWarning)
{
  switch (RoadWarningType(theWarning))
  {
    case NORMAL:
      return 0;
    case FROSTY:
      return 1;
    case ICING:
      return 2;
    case SNOWY:
      return 3;
    case FROSTSLIPPERY:
      return 4;
    case WINDWARNING:
      return 5;
    case WHIRLING:
      return 6;
    case HEAVY_SNOWFALL:
      return 7;
    case FASTWORSENING:
      return 8;
    case SLEET_TO_ICY:
      return 9;
    case RAIN_TO_ICY:
      return 10;
    case ICYRAIN:
      return 11;
  }

  throw TextGenError("Unknown road warning in warning_importance");
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert road warning enumeration to string
 */
// ----------------------------------------------------------------------

const char* warning_name(RoadWarningType theWarning)
{
  switch (theWarning)
  {
    case NORMAL:
      return "normal";
    case FROSTY:
      return "frosty";
    case ICING:
      return "icing";
    case SNOWY:
      return "snowy";
    case FROSTSLIPPERY:
      return "frostslippery";
    case WINDWARNING:
      return "windwarning";
    case WHIRLING:
      return "whirling";
    case HEAVY_SNOWFALL:
      return "heavy_snowfall";
    case FASTWORSENING:
      return "fastworsening";
    case SLEET_TO_ICY:
      return "sleet_to_icy";
    case RAIN_TO_ICY:
      return "rain_to_icy";
    case ICYRAIN:
      return "icyrain";
  }
  throw TextGenError("Unknown road warning in warning_name");
}

// ----------------------------------------------------------------------
/*!
 * \brief Road warning percentages
 */
// ----------------------------------------------------------------------

class WarningPercentages
{
 public:
  ~WarningPercentages() {}
  WarningPercentages() : itsPercentages(max_warning - min_warning + 1, 0) {}
  const double& operator[](int i) const { return itsPercentages[i]; }
  double& operator[](int i) { return itsPercentages[i]; }
 private:
  std::vector<double> itsPercentages;
};

// ----------------------------------------------------------------------
/*!
 * \brief Calculate road warning percentages for given period
 *
 * \todo Optimize the integration loop so that you first calculate
 *       the minimum and maximum road warning, or possibly just
 *       the maximum, and then loop over those values only.
 */
// ----------------------------------------------------------------------

const WarningPercentages calculate_percentages(const WeatherPeriod& thePeriod,
                                               int thePeriodIndex,
                                               const TextGen::AnalysisSources& theSources,
                                               const TextGen::WeatherArea& theArea,
                                               const std::string& theVar)
{
  GridForecaster forecaster;

  WarningPercentages percentages;
  for (int i = min_warning; i <= max_warning; i++)
  {
    const RoadWarningType c = RoadWarningType(i);

    const string fake = (theVar + "::fake::period" + lexical_cast<string>(thePeriodIndex) + "::" +
                         warning_name(c) + "::percentage");

    ValueAcceptor warnfilter;
    warnfilter.value(c);

    WeatherResult result = forecaster.analyze(fake,
                                              theSources,
                                              RoadWarning,
                                              Maximum,  // Mean -> Maximum PI 7.9.05
                                              Percentage,
                                              theArea,
                                              thePeriod,
                                              DefaultAcceptor(),
                                              DefaultAcceptor(),
                                              warnfilter);

    percentages[c] = result.value();
  }

  return percentages;
}

// ----------------------------------------------------------------------
/*!
 * \brief Find the most frequent warning type
 *
 * Equal cases are resolved by the importance of the type
 */
// ----------------------------------------------------------------------

RoadWarningType find_most_general_warning(const WarningPercentages& thePercentages)
{
  int ibest = min_warning;
  for (int i = min_warning + 1; i <= max_warning; i++)
  {
    if (thePercentages[i] > thePercentages[ibest])
      ibest = i;
    else if (thePercentages[i] == thePercentages[ibest] &&
             warning_importance(i) > warning_importance(ibest))
      ibest = i;
  }
  return RoadWarningType(ibest);
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a places-phrase for a warning
 */
// ----------------------------------------------------------------------

const char* warning_places_phrase(RoadWarningType theType,
                                  double thePercentage,
                                  int theGenerallyLimit,
                                  int theManyPlacesLimit,
                                  int theSomePlacesLimit)
{
  if (thePercentage < theSomePlacesLimit)
    return "";
  else if (thePercentage < theManyPlacesLimit)
    return "paikoin";
  else if (thePercentage < theGenerallyLimit)
    return "monin paikoin";
  else
    return "yleisesti";
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a single road warning phrase
 */
// ----------------------------------------------------------------------

const Sentence warning_phrase(RoadWarningType theType,
                              double thePercentage,
                              int theGenerallyLimit,
                              int theManyPlacesLimit,
                              int theSomePlacesLimit)
{
  const char* places_phrase = warning_places_phrase(theType,
                                                    thePercentage,
                                                    theGenerallyLimit,
                                                    theManyPlacesLimit,

                                                    theSomePlacesLimit);

  Sentence tienpinta_muuttuu;
  Sentence tie_on_muuttumassa;
  Sentence keli_huononee;
  Sentence vaikeuttaa_liikennetta;
  Sentence keli_on;
  tienpinta_muuttuu << "tienpinta muuttuu" << places_phrase;
  tie_on_muuttumassa << "tie on muuttumassa" << places_phrase;
  keli_huononee << "keli huononee" << places_phrase;
  vaikeuttaa_liikennetta << "vaikeuttaa liikennetta" << places_phrase;
  keli_on << "keli on" << places_phrase;
  Sentence marka_tienpinta;
  Sentence pakkasliukkaus;
  Sentence puuskainen_tuuli;
  Sentence lumipyry;
  Sentence lumisade;
  Sentence kelin_huononeminen;
  Sentence rantasade;
  Sentence alijaahtynyt;
  marka_tienpinta << "marka" << tienpinta_muuttuu;
  pakkasliukkaus << "pakkasliukkaus";
  puuskainen_tuuli << "voimakas puuskainen tuuli";
  lumipyry << "lumipyry";
  lumisade << "runsas lumisade";
  kelin_huononeminen << "kelin nopea huononeminen";
  rantasade << "rantasade jaiselle";
  alijaahtynyt << "alijaahtynyt sade";

  switch (theType)
  {
    case NORMAL:
      return (keli_on << "normaali");
    case FROSTY:
      return (tienpinta_muuttuu << "kuuraiseksi");
    case ICING:
      return (marka_tienpinta << "jaiseksi tai osittain jaiseksi");
    case SNOWY:
      return (keli_huononee << "lumisateen takia");
    case FROSTSLIPPERY:
      return (pakkasliukkaus << vaikeuttaa_liikennetta);
    case WINDWARNING:
      return (puuskainen_tuuli << vaikeuttaa_liikennetta);
    case WHIRLING:
      return (lumipyry << vaikeuttaa_liikennetta);
    case HEAVY_SNOWFALL:
      return (lumisade << vaikeuttaa_liikennetta);
    case FASTWORSENING:
      return (kelin_huononeminen << vaikeuttaa_liikennetta);
    case SLEET_TO_ICY:
      return (rantasade << vaikeuttaa_liikennetta);
    case RAIN_TO_ICY:
      return (tie_on_muuttumassa << "jaiseksi vesisateesta johtuen");
    case ICYRAIN:
      return (alijaahtynyt << vaikeuttaa_liikennetta);
  }

  // Unreachable

  throw TextGenError("Internal error in roadwarning_shortview warning_phrase function");
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate the second phrase of the manyplaces-type sentence
 */
// ----------------------------------------------------------------------

const Sentence second_places_sentence(RoadWarningType thePrimaryType,
                                      RoadWarningType theSecondaryType)
{
  Sentence sentence;

  switch (thePrimaryType)
  {
    case ICYRAIN:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
          sentence << "paikoin"
                   << "tie on muuttumassa"
                   << "jaiseksi vesisateesta johtuen";
          break;
        case SLEET_TO_ICY:
          sentence << "paikoin"
                   << "rantasade jaiselle"
                   << "vaikeuttaa liikennetta";
          break;
        case FASTWORSENING:
          sentence << "paikoin"
                   << "kelin nopea huononeminen"
                   << "vaikeuttaa liikennetta";
          break;
        case HEAVY_SNOWFALL:
          sentence << "paikoin"
                   << "runsas lumisade"
                   << "vaikeuttaa liikennetta";
          break;
        case WHIRLING:
          sentence << "paikoin"
                   << "lumipyry"
                   << "vaikeuttaa liikennetta";
          break;
        case WINDWARNING:
          sentence << "paikoin"
                   << "voimakas puuskainen tuuli"
                   << "vaikeuttaa liikennetta";
          break;
        case FROSTSLIPPERY:
          sentence << "paikoin"
                   << "pakkasliukkaus"
                   << "vaikeuttaa liikennetta";
          break;
        case SNOWY:
          sentence << "paikoin"
                   << "keli huononee"
                   << "lumisateen takia";
          break;
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case RAIN_TO_ICY:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
          sentence << "paikoin"
                   << "rantasade jaiselle"
                   << "vaikeuttaa liikennetta";
          break;
        case FASTWORSENING:
          sentence << "paikoin"
                   << "kelin nopea huononeminen"
                   << "vaikeuttaa liikennetta";
          break;
        case HEAVY_SNOWFALL:
          sentence << "paikoin"
                   << "runsas lumisade"
                   << "vaikeuttaa liikennetta";
          break;
        case WHIRLING:
          sentence << "paikoin"
                   << "lumipyry"
                   << "vaikeuttaa liikennetta";
          break;
        case WINDWARNING:
          sentence << "paikoin"
                   << "voimakas puuskainen tuuli"
                   << "vaikeuttaa liikennetta";
          break;
        case FROSTSLIPPERY:
          sentence << "paikoin"
                   << "pakkasliukkaus"
                   << "vaikeuttaa liikennetta";
          break;
        case SNOWY:
          sentence << "paikoin"
                   << "keli huononee"
                   << "lumisateen takia";
          break;
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case SLEET_TO_ICY:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
          sentence << "paikoin"
                   << "kelin nopea huononeminen"
                   << "vaikeuttaa liikennetta";
          break;
        case HEAVY_SNOWFALL:
          sentence << "paikoin"
                   << "runsas lumisade"
                   << "vaikeuttaa liikennetta";
          break;
        case WHIRLING:
          sentence << "paikoin"
                   << "lumipyry"
                   << "vaikeuttaa liikennetta";
          break;
        case WINDWARNING:
          sentence << "paikoin"
                   << "voimakas puuskainen tuuli"
                   << "vaikeuttaa liikennetta";
          break;
        case FROSTSLIPPERY:
          sentence << "paikoin"
                   << "pakkasliukkaus"
                   << "vaikeuttaa liikennetta";
          break;
        case SNOWY:
          sentence << "paikoin"
                   << "keli huononee"
                   << "lumisateen takia";
          break;
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case FASTWORSENING:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
        case HEAVY_SNOWFALL:
          sentence << "paikoin"
                   << "runsas lumisade"
                   << "vaikeuttaa liikennetta";
          break;
        case WHIRLING:
          sentence << "paikoin"
                   << "lumipyry"
                   << "vaikeuttaa liikennetta";
          break;
        case WINDWARNING:
          sentence << "paikoin"
                   << "voimakas puuskainen tuuli"
                   << "vaikeuttaa liikennetta";
          break;
        case FROSTSLIPPERY:
          sentence << "paikoin"
                   << "pakkasliukkaus"
                   << "vaikeuttaa liikennetta";
          break;
        case SNOWY:
          sentence << "paikoin"
                   << "keli huononee"
                   << "lumisateen takia";
          break;
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case HEAVY_SNOWFALL:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
        case HEAVY_SNOWFALL:
        case WHIRLING:
          sentence << "paikoin"
                   << "lumipyry"
                   << "vaikeuttaa liikennetta";
          break;
        case WINDWARNING:
          sentence << "paikoin"
                   << "voimakas puuskainen tuuli"
                   << "vaikeuttaa liikennetta";
          break;
        case FROSTSLIPPERY:
          sentence << "paikoin"
                   << "pakkasliukkaus"
                   << "vaikeuttaa liikennetta";
          break;
        case SNOWY:
          sentence << "paikoin"
                   << "keli huononee"
                   << "lumisateen takia";
          break;
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case WHIRLING:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
        case HEAVY_SNOWFALL:
        case WHIRLING:
        case WINDWARNING:
          sentence << "paikoin"
                   << "voimakas puuskainen tuuli"
                   << "vaikeuttaa liikennetta";
          break;
        case FROSTSLIPPERY:
          sentence << "paikoin"
                   << "pakkasliukkaus"
                   << "vaikeuttaa liikennetta";
          break;
        case SNOWY:
          sentence << "paikoin"
                   << "keli huononee"
                   << "lumisateen takia";
          break;
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case WINDWARNING:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
        case HEAVY_SNOWFALL:
        case WHIRLING:
        case WINDWARNING:
        case FROSTSLIPPERY:
          sentence << "paikoin"
                   << "pakkasliukkaus"
                   << "vaikeuttaa liikennetta";
          break;
        case SNOWY:
          sentence << "paikoin"
                   << "keli huononee"
                   << "lumisateen takia";
          break;
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case FROSTSLIPPERY:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
        case HEAVY_SNOWFALL:
        case WHIRLING:
        case WINDWARNING:
        case FROSTSLIPPERY:
        case SNOWY:
          sentence << "paikoin"
                   << "keli huononee"
                   << "lumisateen takia";
          break;
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case SNOWY:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
        case HEAVY_SNOWFALL:
        case WHIRLING:
        case WINDWARNING:
        case FROSTSLIPPERY:
        case SNOWY:
        case ICING:
          sentence << "paikoin"
                   << "marka tienpinta muuttuu"
                   << "jaiseksi tai osittain jaiseksi";
          break;
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case ICING:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
        case HEAVY_SNOWFALL:
        case WHIRLING:
        case WINDWARNING:
        case FROSTSLIPPERY:
        case SNOWY:
        case ICING:
        case FROSTY:
          sentence << "paikoin"
                   << "tienpinta muuttuu"
                   << "kuuraiseksi";
          break;
        case NORMAL:
          break;
      }
      break;
    case FROSTY:
      switch (theSecondaryType)
      {
        case ICYRAIN:
        case RAIN_TO_ICY:
        case SLEET_TO_ICY:
        case FASTWORSENING:
        case HEAVY_SNOWFALL:
        case WHIRLING:
        case WINDWARNING:
        case FROSTSLIPPERY:
        case SNOWY:
        case ICING:
        case FROSTY:
        case NORMAL:
          break;
      }
    case NORMAL:
      break;
  }
  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a sentence from road warning percentages
 */
// ----------------------------------------------------------------------

const Sentence warning_sentence(const WarningPercentages& thePercentages, const string& theVar)
{
  Sentence sentence;

  // Read the related configuration settings

  using Settings::optional_percentage;
  const int generally_limit = optional_percentage(theVar + "::generally_limit", 90);
  const int manyplaces_limit = optional_percentage(theVar + "::manyplaces_limit", 50);
  const int someplaces_limit = optional_percentage(theVar + "::someplaces_limit", 10);

  // Find the most frequent warning

  RoadWarningType firsttype = find_most_general_warning(thePercentages);

  // Handle the case when the type dominates the others

  if (thePercentages[firsttype] >= generally_limit)
  {
    sentence << warning_phrase(
        firsttype, thePercentages[firsttype], generally_limit, manyplaces_limit, someplaces_limit);
    return sentence;
  }

  // List all "someplaces" types that occur in order of importance
  // The set may include firsttype, if no condition occurs in many places

  map<int, RoadWarningType, std::greater<int> > someplacestypes;

  for (int i = min_warning; i <= max_warning; i++)
  {
    const RoadWarningType warning = RoadWarningType(i);

    if (thePercentages[warning] >= someplaces_limit && thePercentages[warning] < manyplaces_limit)
    {
      const int importance = warning_importance(warning);
      someplacestypes.insert(make_pair(importance, warning));
    }
  }

  // Handle the case when there is one type in many places
  // Note that we always ignore it if the "many places" type is NORMAL,
  // instead we report on two most important "places" types

  if (thePercentages[firsttype] >= manyplaces_limit && firsttype != NORMAL)
  {
    sentence << warning_phrase(
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
    sentence << warning_phrase(
        firsttype, thePercentages[firsttype], generally_limit, manyplaces_limit, someplaces_limit);
    return sentence;
  }

  firsttype = someplacestypes.begin()->second;

  sentence << warning_phrase(
      firsttype, thePercentages[firsttype], generally_limit, manyplaces_limit, someplaces_limit);
  /*
        if(someplacestypes.size() > 1)
        {
        RoadWarningType secondtype = (++someplacestypes.begin())->second;

        if(firsttype == ICING)
        {
        if(secondtype == PARTLY_ICING)
        sentence << "tai" << "osittain jaisiksi";
        else if(secondtype == FROSTY)
        sentence << "tai" << "kuuraisiksi";
        }
        }
  */
  return sentence;
}
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate story on road warnings
 *
 * \return The story
 *
 *†\see page_roadcondition_warning
 */
// ----------------------------------------------------------------------

Paragraph RoadStory::warning_shortview() const
{
  MessageLogger log("RoadStory::warning_shortview");

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

  HourPeriodGenerator generator(fullperiod, itsVar);

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

  DebugTextFormatter formatter;

  log << "Individual period results:" << endl;
  for (unsigned int i = 1; i <= generator.size(); i++)
  {
    const WeatherPeriod period = generator.period(i);

    const WarningPercentages result = calculate_percentages(period, i, itsSources, itsArea, itsVar);

    for (int j = min_warning; j <= max_warning; j++)
    {
      if (result[j] == kFloatMissing)
      {
        Sentence s;
        s << "tieto puuttuu";
        paragraph << s;
        log << paragraph;
        return paragraph;
      }
    }

    const Sentence sentence = warning_sentence(result, itsVar);
    const string realization = formatter.format(sentence);

    periods.push_back(period);
    sentences.push_back(sentence);

    log << period.localStartTime() << " ... " << period.localEndTime() << ": " << realization
        << endl;

    for (int j = min_warning; j <= max_warning; j++)
      log << '\t' << warning_name(RoadWarningType(j)) << '\t' << result[j] << " %" << endl;
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
    sentence << sentences[i];
  }
  paragraph << sentence;

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
