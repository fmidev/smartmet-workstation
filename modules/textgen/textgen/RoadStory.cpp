// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::RoadStory
 */
// ======================================================================
/*!
 * \class TextGen::RoadStory
 *
 * \brief Generates stories on road forecasts
 *
 * \see page_tarinat
 */
// ======================================================================

#include "RoadStory.h"
#include "Paragraph.h"
#include <calculator/TextGenError.h>

using namespace TextGen;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

RoadStory::~RoadStory() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theForecastTime The forecast time
 * \param theSources The analysis sources
 * \param theArea The area to be analyzed
 * \param thePeriod The period to be analyzed
 * \param theVariable The associated configuration variable
*/
// ----------------------------------------------------------------------

RoadStory::RoadStory(const TextGenPosixTime& theForecastTime,
                     const AnalysisSources& theSources,
                     const WeatherArea& theArea,
                     const WeatherPeriod& thePeriod,
                     const string& theVariable)
    : itsForecastTime(theForecastTime),
      itsSources(theSources),
      itsArea(theArea),
      itsPeriod(thePeriod),
      itsVar(theVariable)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Test whether the given story is known to this class
 *
 * \param theName The story name
 * \return True if this class can generate the story
 */
// ----------------------------------------------------------------------

bool RoadStory::hasStory(const string& theName)
{
  if (theName == "roadtemperature_daynightranges") return true;
  if (theName == "roadcondition_overview") return true;
  if (theName == "roadwarning_overview") return true;
  if (theName == "roadcondition_shortview") return true;
  if (theName == "roadwarning_shortview") return true;
  if (theName == "roadtemperature_shortrange") return true;

  return false;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate the desired story
 *
 * Throws if the story name is not recognized.
 *
 * \param theName The story name
 * \return Paragraph containing the story
 */
// ----------------------------------------------------------------------.

Paragraph RoadStory::makeStory(const string& theName) const
{
  if (theName == "roadtemperature_daynightranges") return daynightranges();
  if (theName == "roadtemperature_shortrange") return shortrange();
  if (theName == "roadcondition_overview") return condition_overview();
  if (theName == "roadwarning_overview") return warning_overview();
  if (theName == "roadwarning_shortview") return warning_shortview();
  if (theName == "roadcondition_shortview") return condition_shortview();

  throw TextGenError("RoadStory cannot make story " + theName);
}

}  // namespace TextGen

// ======================================================================
