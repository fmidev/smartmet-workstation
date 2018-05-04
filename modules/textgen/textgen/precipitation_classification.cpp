// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::PrecipitationStory::classification
 */
// ======================================================================

#include "PrecipitationStory.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include "IntegerRange.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include <calculator/RangeAcceptor.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include "UnitFactory.h"
#include <calculator/WeatherResult.h>

#include <newbase/NFmiStringTools.h>
#include <boost/lexical_cast.hpp>
#include <list>

using namespace boost;
using namespace std;
using namespace TextGen;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Parse rain amount class specification string
 *
 * The input is of the form "A...B,C...D,D...E"
 *
 * Throws if an error is detected in the range specifications.
 *
 * \param theVariable The variable containing the classes
 * \return List of range limits
 */
// ----------------------------------------------------------------------

const list<pair<int, int> > parse_classes(const std::string& theVariable)
{
  using namespace TextGen;

  const string value = Settings::require(theVariable);

  if (value.empty()) throw TextGenError(theVariable + " value must not be empty");

  list<pair<int, int> > output;

  vector<string> clist = NFmiStringTools::Split(value);
  for (vector<string>::const_iterator it = clist.begin(); it != clist.end(); ++it)
  {
    vector<string> rlist = NFmiStringTools::Split(*it, "...");
    if (rlist.size() != 2)
      throw TextGenError(*it + " is not of form A...B in variable " + theVariable);
    int lolimit = lexical_cast<int>(rlist[0]);
    int hilimit = lexical_cast<int>(rlist[1]);
    if (hilimit <= lolimit)
      throw TextGenError(*it + " has upper limit <= lower limit in variable " + theVariable);

    output.push_back(make_pair(lolimit, hilimit));
  }

  return output;
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract the maximum number from a list of pairs of integers
 *
 * \param theList The list of pairs of integers
 * \return The maximum number
 */
// ----------------------------------------------------------------------

int rainlimit(const list<pair<int, int> >& theList)
{
  using namespace TextGen;

  if (theList.empty())
    throw TextGenError("Internal error, trying to extract maximum rain from empty list");

  int ret = theList.front().first;
  for (list<pair<int, int> >::const_iterator it = theList.begin(); it != theList.end(); ++it)
  {
    ret = std::max(ret, it->first);
    ret = std::max(ret, it->second);
  }
  return ret;
}

}  // namespace anonymous

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on total precipitation classification
 *
 * Sample stories:
 *
 * -# "Sadesumma yli N millimetria."
 * -# "Sadesumma monin paikoin yli N millimetria."
 * -# "Sadesumma paikoin yli N millimetria."
 * -# "Sadesumma 0 millimetria."
 * -# "Sadesumma N...M millimetria."
 * -# "Sadesumma N...M millimetria, paikoin enemman."
 * -# "Sadesumma N...M millimetria, monin paikoin enemman."
 *
 * \return The story
 *
 */
// ----------------------------------------------------------------------

Paragraph PrecipitationStory::classification() const
{
  MessageLogger log("PrecipitationStory::classification");

  const string rangeseparator = Settings::optional_string(itsVar + "::rangeseparator", "-");

  Paragraph paragraph;
  Sentence sentence;

  GridForecaster forecaster;

  RangeAcceptor rainlimits;
  rainlimits.lowerLimit(Settings::optional_double(itsVar + "::minrain", 0));

  // Gather the results

  WeatherResult minresult = forecaster.analyze(itsVar + "::fake::minimum",
                                               itsSources,
                                               Precipitation,
                                               Minimum,
                                               Sum,
                                               itsArea,
                                               itsPeriod,
                                               DefaultAcceptor(),
                                               rainlimits);

  WeatherResult meanresult = forecaster.analyze(itsVar + "::fake::mean",
                                                itsSources,
                                                Precipitation,
                                                Mean,
                                                Sum,
                                                itsArea,
                                                itsPeriod,
                                                DefaultAcceptor(),
                                                rainlimits);

  WeatherResult maxresult = forecaster.analyze(itsVar + "::fake::maximum",
                                               itsSources,
                                               Precipitation,
                                               Maximum,
                                               Sum,
                                               itsArea,
                                               itsPeriod,
                                               DefaultAcceptor(),
                                               rainlimits);

  // Check for invalid results
  if (minresult.value() == kFloatMissing || maxresult.value() == kFloatMissing ||
      meanresult.value() == kFloatMissing)
    throw TextGenError("Total precipitation not available");

  log << "Precipitation Minimum(Sum) " << minresult << endl;
  log << "Precipitation Mean(Sum) " << meanresult << endl;
  log << "Precipitation Maximum(Sum) " << maxresult << endl;

  // Parse the classes

  const string classvariable = itsVar + "::classes";
  list<pair<int, int> > classes = parse_classes(classvariable);

  // Establish the maximum rain amount in the classes

  const int maxrainlimit = rainlimit(classes);

  // Handle the simple case of no rain

  if (maxresult.value() == 0)
  {
    sentence << "sadesumma" << 0 << *UnitFactory::create(Millimeters);
    paragraph << sentence;
    return paragraph;
  }

  // Handle the case when maximum rain exceeds the biggest
  // rain amount class

  if (maxresult.value() > maxrainlimit)
  {
    const string variable1 = itsVar + "::max_some_places";
    const string variable2 = itsVar + "::max_many_places";

    int phrase = 1;

    if (minresult.value() < maxrainlimit)
    {
      RangeAcceptor percentagelimits;
      percentagelimits.lowerLimit(maxrainlimit);

      WeatherResult probresult = forecaster.analyze(itsVar + "::fake::percentage",
                                                    itsSources,
                                                    Precipitation,
                                                    Percentage,
                                                    Sum,
                                                    itsArea,
                                                    itsPeriod,
                                                    rainlimits,
                                                    DefaultAcceptor(),
                                                    percentagelimits);

      log << "Precipitation Percentage(Sum) " << probresult << endl;

      const int limit1 = Settings::optional_int(variable1, -1);
      const int limit2 = Settings::optional_int(variable2, -1);

      if (probresult.value() == kFloatMissing)
        phrase = 1;
      else if (probresult.value() >= limit2)
        phrase = 3;
      else if (probresult.value() >= limit1)
        phrase = 2;
      else
        phrase = 4;
    }

    if (phrase < 4)
    {
      sentence << "sadesumma";
      if (phrase == 2)
        sentence << "paikoin";
      else if (phrase == 3)
        sentence << "monin paikoin";
      sentence << "yli" << maxrainlimit << *UnitFactory::create(Millimeters);
      paragraph << sentence;
      log << paragraph;
      return paragraph;
    }

    // FALLTHROUGH FOR PHRASE 4, WE RETURN N...M INSTEAD!

    // here we forge result to be back within the largest
    // available rain class

    if (meanresult.value() > maxrainlimit) meanresult = WeatherResult(maxrainlimit, 1);
  }

  // Find the first class with the correct mean rain amount

  list<pair<int, int> >::const_iterator it;
  for (it = classes.begin(); it != classes.end(); ++it)
  {
    if (meanresult.value() >= it->first && meanresult.value() <= it->second) break;
  }

  if (it == classes.end()) throw TextGenError(itsVar + " has gaps in the ranges");

  const int lolimit = it->first;
  const int hilimit = it->second;

  sentence << "sadesumma" << IntegerRange(lolimit, hilimit, rangeseparator)
           << *UnitFactory::create(Millimeters);

  // Lisaa tarvittaessa "paikoin enemman" tai "monin paikoin enemman" peraan

  if (maxresult.value() > hilimit)
  {
    const string variable1 = itsVar + "::some_places";
    const string variable2 = itsVar + "::many_places";

    int phrase = 1;

    RangeAcceptor percentagelimits;
    percentagelimits.lowerLimit(hilimit);

    WeatherResult probresult = forecaster.analyze(itsVar + "::fake::percentage",
                                                  itsSources,
                                                  Precipitation,
                                                  Percentage,
                                                  Sum,
                                                  itsArea,
                                                  itsPeriod,
                                                  rainlimits,
                                                  DefaultAcceptor(),
                                                  percentagelimits);

    log << "Precipitation Percentage(Sum) " << probresult << endl;

    const int limit1 = Settings::optional_int(variable1, -1);
    const int limit2 = Settings::optional_int(variable2, -1);

    if (probresult.value() == kFloatMissing)
      phrase = 1;
    else if (probresult.value() >= limit2)
      phrase = 3;
    else if (probresult.value() >= limit1)
      phrase = 2;
    else
      phrase = 1;

    if (phrase == 2)
      sentence << Delimiter(",") << "paikoin enemman";
    else if (phrase == 3)
      sentence << Delimiter(",") << "monin paikoin enemman";
  }

  paragraph << sentence;
  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
