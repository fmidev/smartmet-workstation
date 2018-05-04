// ======================================================================
/*!
 * \file
 * \brief Implementation for namespace TextGen::CloudinessStoryTools
 */
// ======================================================================
/*!
 * \namespace TextGen::CloudinessStoryTools
 *
 * \brief Tools for TextGen::CloudinessStory and similar classes
 *
 * This namespace contains utility functions which are not
 * exclusive for handling cloudiness alone. For example,
 * TextGen::WeatherStory most likely has use for these
 * functions.
 */
// ======================================================================

#include "CloudinessStoryTools.h"
#include <calculator/AnalysisSources.h>
#include <calculator/GridForecaster.h>
#include "MessageLogger.h"
#include <calculator/RangeAcceptor.h>
#include <calculator/Settings.h>
#include "Sentence.h"
#include <calculator/WeatherResult.h>

#include <boost/lexical_cast.hpp>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
namespace CloudinessStoryTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Calculate cloudiness type
 *
 * \param theVar The settings variable controlling the conversion
 * \param theCloudyPercentage The percentage of cloudy
 * \param theClearPercentage The percentage of clear
 * \param theTrend The trend
 * \return Respective cloudiness type
 *
 * \see \ref page_story_cloudiness_overview for the algorithm
 *
 */
// ----------------------------------------------------------------------

CloudinessType cloudiness_type(const std::string& theVar,
                               double theCloudyPercentage,
                               double theClearPercentage,
                               double theTrend)
{
  using namespace Settings;

  const int single_class_limit = optional_percentage(theVar + "::single_class_limit", 90);
  const int mostly_class_limit = optional_percentage(theVar + "::mostly_class_limit", 80);
  const int no_class_limit = optional_percentage(theVar + "::no_class_limit", 20);
  const int trend_limit = optional_percentage(theVar + "::trend_limit", 80);

  const double partlycloudy = 100 - theCloudyPercentage - theClearPercentage;

  if (theCloudyPercentage >= single_class_limit) return Cloudy;
  if (theClearPercentage >= single_class_limit) return Clear;
  if (partlycloudy >= single_class_limit) return PartlyCloudy;

  if (theCloudyPercentage >= mostly_class_limit) return MostlyCloudy;
  if (theClearPercentage >= mostly_class_limit) return MostlyClear;
  if (partlycloudy >= mostly_class_limit) return MostlyPartlyCloudy;

  if (theClearPercentage < no_class_limit) return CloudyOrPartlyCloudy;
  if (theCloudyPercentage < no_class_limit) return ClearOrPartlyCloudy;

  if (theTrend >= trend_limit) return IncreasingCloudiness;
  if (-theTrend >= trend_limit) return DecreasingCloudiness;

  return VariableCloudiness;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test two cloudiness types for similarity
 *
 * \param theType1 The first cloudiness type
 * \param theType2 The second cloudiness type
 * \return A pair, if second is true then first is the common type.
 *         If second is false, value of first is unspecified.
 *
 * Note that the first cloudiness type is assumed to precede
 * the second in time.
 *
 * The similarities of cloudiness types are defined at
 * \ref subsection_cloudiness_overview_similarity
 */
// ----------------------------------------------------------------------

CommonCloudiness similar_type(CloudinessType theType1, CloudinessType theType2)
{
  static std::pair<CloudinessType, bool> notsimilar(VariableCloudiness, false);

  switch (theType1)
  {
    case Cloudy:
    {
      switch (theType2)
      {
        case Cloudy:
          return CommonCloudiness(Cloudy, true);
        case MostlyCloudy:
          return CommonCloudiness(MostlyCloudy, true);
        case CloudyOrPartlyCloudy:
        case MostlyPartlyCloudy:
        case PartlyCloudy:
          return CommonCloudiness(CloudyOrPartlyCloudy, true);
        case Clear:
        case MostlyClear:
        case ClearOrPartlyCloudy:
        case DecreasingCloudiness:
        case IncreasingCloudiness:
        case VariableCloudiness:
          return notsimilar;
      }
    }
    case IncreasingCloudiness:
    {
      switch (theType2)
      {
        case IncreasingCloudiness:
        case Cloudy:
        case MostlyCloudy:
          return CommonCloudiness(IncreasingCloudiness, true);
        case VariableCloudiness:
          return CommonCloudiness(VariableCloudiness, true);
        case PartlyCloudy:
        case Clear:
        case MostlyPartlyCloudy:
        case MostlyClear:
        case CloudyOrPartlyCloudy:
        case ClearOrPartlyCloudy:
        case DecreasingCloudiness:
          return notsimilar;
      }
    }
    case MostlyCloudy:
    {
      switch (theType2)
      {
        case MostlyCloudy:
        case Cloudy:
          return CommonCloudiness(MostlyCloudy, true);
        case CloudyOrPartlyCloudy:
        case MostlyPartlyCloudy:
        case PartlyCloudy:
          return CommonCloudiness(CloudyOrPartlyCloudy, true);
        case Clear:
        case MostlyClear:
        case ClearOrPartlyCloudy:
        case DecreasingCloudiness:
        case IncreasingCloudiness:
        case VariableCloudiness:
          return notsimilar;
      }
    }
    case CloudyOrPartlyCloudy:
    {
      switch (theType2)
      {
        case Cloudy:
        case CloudyOrPartlyCloudy:
        case MostlyCloudy:
        case MostlyPartlyCloudy:
        case PartlyCloudy:
          return CommonCloudiness(CloudyOrPartlyCloudy, true);
        case VariableCloudiness:
        case ClearOrPartlyCloudy:
          return CommonCloudiness(VariableCloudiness, true);
        case Clear:
        case MostlyClear:
        case DecreasingCloudiness:
        case IncreasingCloudiness:
          return notsimilar;
      }
    }
    case MostlyPartlyCloudy:
    {
      switch (theType2)
      {
        case MostlyPartlyCloudy:
        case PartlyCloudy:
          return CommonCloudiness(MostlyPartlyCloudy, true);
        case Cloudy:
        case MostlyCloudy:
          return CommonCloudiness(CloudyOrPartlyCloudy, true);
        case CloudyOrPartlyCloudy:
        case VariableCloudiness:
          return CommonCloudiness(VariableCloudiness, true);
        case ClearOrPartlyCloudy:
        case MostlyClear:
        case Clear:
          return CommonCloudiness(ClearOrPartlyCloudy, true);
        case DecreasingCloudiness:
        case IncreasingCloudiness:
          return notsimilar;
      }
    }
    case VariableCloudiness:
    {
      switch (theType2)
      {
        case VariableCloudiness:
        case IncreasingCloudiness:
        case MostlyCloudy:
        case CloudyOrPartlyCloudy:
        case MostlyPartlyCloudy:
        case PartlyCloudy:
        case ClearOrPartlyCloudy:
          return CommonCloudiness(VariableCloudiness, true);
        case Cloudy:
        case Clear:
        case MostlyClear:
        case DecreasingCloudiness:
          return notsimilar;
      }
    }
    case PartlyCloudy:
    {
      switch (theType2)
      {
        case PartlyCloudy:
          return CommonCloudiness(PartlyCloudy, true);
        case Cloudy:
        case MostlyCloudy:
        case CloudyOrPartlyCloudy:
          return CommonCloudiness(CloudyOrPartlyCloudy, true);
        case MostlyPartlyCloudy:
          return CommonCloudiness(MostlyPartlyCloudy, true);
        case VariableCloudiness:
          return CommonCloudiness(VariableCloudiness, true);
        case ClearOrPartlyCloudy:
        case MostlyClear:
        case Clear:
          return CommonCloudiness(ClearOrPartlyCloudy, true);
        case DecreasingCloudiness:
        case IncreasingCloudiness:
          return notsimilar;
      }
    }
    case ClearOrPartlyCloudy:
    {
      switch (theType2)
      {
        case ClearOrPartlyCloudy:
        case MostlyPartlyCloudy:
        case PartlyCloudy:
        case MostlyClear:
        case Clear:
          return CommonCloudiness(ClearOrPartlyCloudy, true);
        case MostlyCloudy:
        case CloudyOrPartlyCloudy:
        case VariableCloudiness:
          return CommonCloudiness(VariableCloudiness, true);
        case Cloudy:
        case DecreasingCloudiness:
        case IncreasingCloudiness:
          return notsimilar;
      }
    }
    case MostlyClear:
    {
      switch (theType2)
      {
        case MostlyClear:
        case Clear:
          return CommonCloudiness(MostlyClear, true);
        case MostlyPartlyCloudy:
        case PartlyCloudy:
        case ClearOrPartlyCloudy:
          return CommonCloudiness(ClearOrPartlyCloudy, true);
        case Cloudy:
        case MostlyCloudy:
        case CloudyOrPartlyCloudy:
        case DecreasingCloudiness:
        case IncreasingCloudiness:
        case VariableCloudiness:
          return notsimilar;
      }
    }
    case DecreasingCloudiness:
    {
      switch (theType2)
      {
        case DecreasingCloudiness:
        case MostlyClear:
        case Clear:
          return CommonCloudiness(DecreasingCloudiness, true);
        case Cloudy:
        case PartlyCloudy:
        case MostlyCloudy:
        case MostlyPartlyCloudy:
        case CloudyOrPartlyCloudy:
        case ClearOrPartlyCloudy:
        case IncreasingCloudiness:
        case VariableCloudiness:
          return notsimilar;
      }
    }
    case Clear:
    {
      switch (theType2)
      {
        case Clear:
          return CommonCloudiness(Clear, true);
        case MostlyPartlyCloudy:
        case PartlyCloudy:
        case ClearOrPartlyCloudy:
          return CommonCloudiness(ClearOrPartlyCloudy, true);
        case MostlyClear:
          return CommonCloudiness(MostlyClear, true);
        case CloudyOrPartlyCloudy:
        case Cloudy:
        case MostlyCloudy:
        case DecreasingCloudiness:
        case IncreasingCloudiness:
        case VariableCloudiness:
          return notsimilar;
      }
    }
  }

  // no matching pair found - return "not similar" answer
  return notsimilar;
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate similar cloudiness type for a vector
 *
 * \param theTypes Vector of cloudiness types
 * \return A pair, if second is true then first is the common type.
 *         If second is false, value of first is unspecified.
 *
 * The algorithm is explained at
 * \ref subsection_cloudiness_overview_similarity
 */
// ----------------------------------------------------------------------

CommonCloudiness similar_type(const std::vector<CloudinessType>& theTypes)
{
  return similar_type(theTypes, 0, theTypes.size() - 1);
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate similar cloudiness type for a vector
 *
 * \param theTypes Vector of cloudiness types
 * \param theStart The first index of the range to be analyzed
 * \param theEnd The end index of the range to be analyzed
 * \return A pair, if second is true then first is the common type.
 *         If second is false, value of first is unspecified.
 *
 * Note: The value at theTypes[theEnd] is included in the analysis.
 *
 * The algorithm is explained at
 * \ref subsection_cloudiness_overview_similarity
 */
// ----------------------------------------------------------------------

CommonCloudiness similar_type(const std::vector<CloudinessType>& theTypes,
                              unsigned int theStart,
                              unsigned int theEnd)
{
  // handle zero types case
  if (theTypes.empty() || theEnd < theStart) return CommonCloudiness(VariableCloudiness, false);

  // handle a single type case
  if (theStart == theEnd) return CommonCloudiness(theTypes[theStart], true);

  // handle two type case
  if (theEnd == theStart + 1) return similar_type(theTypes[theStart], theTypes[theEnd]);

  // merge adjacent pairs, if any merge fails, so does the
  // entire merge and we abort

  std::vector<CloudinessType> similar_types;
  for (unsigned int i = theStart; i < theEnd; i++)
  {
    CommonCloudiness tmp = similar_type(theTypes[i], theTypes[i + 1]);
    if (!tmp.second) return CommonCloudiness(VariableCloudiness, false);
    similar_types.push_back(tmp.first);
  }

  // recursion
  return similar_type(similar_types);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns cloudiness phrase for given cloudiness type
 *
 * \param theType The cloudiness type
 * \return The respective cloudiness phrase
 */
// ----------------------------------------------------------------------

Sentence cloudiness_phrase(CloudinessType theType)
{
  Sentence sentence;
  switch (theType)
  {
    case MostlyCloudy:
      sentence << "enimmakseen";
    case Cloudy:
      sentence << "pilvista";
      break;
    case MostlyPartlyCloudy:
      sentence << "enimmakseen";
    case PartlyCloudy:
      sentence << "puolipilvista";
      break;
    case MostlyClear:
      sentence << "enimmakseen";
    case Clear:
      sentence << "selkeaa";
      break;
    case CloudyOrPartlyCloudy:
      sentence << "pilvista"
               << "tai"
               << "puolipilvista";
      break;
    case ClearOrPartlyCloudy:
      sentence << "selkeaa"
               << "tai"
               << "puolipilvista";
      break;
    case DecreasingCloudiness:
      sentence << "selkenevaa";
      break;
    case IncreasingCloudiness:
      sentence << "pilvistyvaa";
      break;
    case VariableCloudiness:
      sentence << "vaihtelevaa pilvisyytta";
      break;
  }
  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate a cloudiness phrase for given period
 *
 * \param theSources The analysis sources
 * \param theArea The area to be analyzed
 * \param thePeriod The rainy period to be analyzed
 * \param theVar The control variable
 * \param theDay The day in question
 * \return The phrase
 */
// ----------------------------------------------------------------------

Sentence cloudiness_phrase(const AnalysisSources& theSources,
                           const WeatherArea& theArea,
                           const WeatherPeriod& thePeriod,
                           const string& theVar,
                           int theDay)
{
  MessageLogger log("CloudinessStoryTools::cloudiness_phrase");

  using namespace Settings;

  const int clear = optional_percentage(theVar + "::clear", 40);
  const int cloudy = optional_percentage(theVar + "::cloudy", 70);

  RangeAcceptor cloudylimits;
  cloudylimits.lowerLimit(cloudy);

  RangeAcceptor clearlimits;
  clearlimits.upperLimit(clear);

  GridForecaster forecaster;

  const string daystr = "day" + lexical_cast<string>(theDay);

  const WeatherResult cloudy_percentage =
      forecaster.analyze(theVar + "::fake::" + daystr + "::cloudy",
                         theSources,
                         Cloudiness,
                         Mean,
                         Percentage,
                         theArea,
                         thePeriod,
                         DefaultAcceptor(),
                         DefaultAcceptor(),
                         cloudylimits);

  const WeatherResult clear_percentage =
      forecaster.analyze(theVar + "::fake::" + daystr + "::clear",
                         theSources,
                         Cloudiness,
                         Mean,
                         Percentage,
                         theArea,
                         thePeriod,
                         DefaultAcceptor(),
                         DefaultAcceptor(),
                         clearlimits);

  const WeatherResult trend = forecaster.analyze(theVar + "::fake::" + daystr + "::trend",
                                                 theSources,
                                                 Cloudiness,
                                                 Mean,
                                                 Trend,
                                                 theArea,
                                                 thePeriod);

  log << "Cloudiness Mean(Percentage(cloudy)) " << daystr << " = " << cloudy_percentage << endl;
  log << "Cloudiness Mean(Percentage(clear)) " << daystr << " = " << clear_percentage << endl;
  log << "Cloudiness Mean(Trend) " << daystr << " = " << trend << endl;

  CloudinessType ctype =
      cloudiness_type(theVar, cloudy_percentage.value(), clear_percentage.value(), trend.value());

  Sentence s;
  s << cloudiness_phrase(ctype);
  return s;
}

}  // namespace CloudinessStoryTools
}  // namespace TextGen

// ======================================================================
