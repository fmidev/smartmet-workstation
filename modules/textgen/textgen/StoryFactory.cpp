// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::StoryFactory
 */
// ======================================================================
/*!
 * \namespace TextGen::StoryFactory
 *
 * \brief Generates brief paragraphs of desired type
 */
// ======================================================================

#include "StoryFactory.h"
#include <calculator/AnalysisSources.h>
#include "CloudinessStory.h"
#include "Delimiter.h"
#include "DewPointStory.h"
#include "ForestStory.h"
#include "FrostStory.h"
// #include "FrostStoryAk.h"
#include "Paragraph.h"
#include "PrecipitationStory.h"
#include "PressureStory.h"
#include "RelativeHumidityStory.h"
#include "RoadStory.h"
#include "SpecialStory.h"
#include "TemperatureStory.h"
#include <calculator/TextGenError.h>
#include "WaveStory.h"
#include "WeatherStory.h"
#include "WindStory.h"

#include <calculator/TextGenPosixTime.h>

using namespace TextGen;
using namespace std;

namespace TextGen
{
namespace StoryFactory
{
// ----------------------------------------------------------------------
/*!
 * \brief Create a story on the desired subject
 *
 * Throws if the given name is not recognized.
 *
 * \param theForecastTime The forecast time
 * \param theSources The associated analysis sources
 * \param theArea The area for which to generate the story
 * \param thePeriod The period for which to generate the story
 * \param theName The story to create
 * \param theVariable The configuration variable prefix
 */
// ----------------------------------------------------------------------

Paragraph create(const TextGenPosixTime& theForecastTime,
                 const AnalysisSources& theSources,
                 const WeatherArea& theArea,
                 const WeatherPeriod& thePeriod,
                 const string& theName,
                 const string& theVariable)
{
  if (TemperatureStory::hasStory(theName))
  {
    TemperatureStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (PrecipitationStory::hasStory(theName))
  {
    PrecipitationStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (CloudinessStory::hasStory(theName))
  {
    CloudinessStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (WeatherStory::hasStory(theName))
  {
    WeatherStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (WindStory::hasStory(theName))
  {
    WindStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (FrostStory::hasStory(theName))
  {
    FrostStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

#if 0
	  if(FrostStoryAk::hasStory(theName))  // AKa 30-Sep-2009
		{
		  FrostStoryAk story(theForecastTime,theSources,theArea,thePeriod,theVariable);
		  return story.makeStory(theName);
		}
#endif

  if (RelativeHumidityStory::hasStory(theName))
  {
    RelativeHumidityStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (RoadStory::hasStory(theName))
  {
    RoadStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (ForestStory::hasStory(theName))
  {
    ForestStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (DewPointStory::hasStory(theName))
  {
    DewPointStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (PressureStory::hasStory(theName))
  {
    PressureStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (WaveStory::hasStory(theName))
  {
    WaveStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  if (SpecialStory::hasStory(theName))
  {
    SpecialStory story(theForecastTime, theSources, theArea, thePeriod, theVariable);
    return story.makeStory(theName);
  }

  throw TextGenError("StoryFactory: Unrecognized story '" + theName + "'");
}

}  // namespace StoryFactory
}  // namespace TextGen

// ======================================================================
