// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::CloudinessStory
 */
// ======================================================================
/*!
 * \class TextGen::CloudinessStory
 *
 * \brief Generates stories on cloudiness
 *
 */
// ======================================================================

#include "CloudinessStory.h"
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

CloudinessStory::~CloudinessStory() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theForecastTime The forecast time
 * \param theSources The analysis sources
 * \param theArea The area to be analyzed
 * \param thePeriod The time interval to be analyzed
 * \param theVariable The associated configuration variable
 */
// ----------------------------------------------------------------------

CloudinessStory::CloudinessStory(const TextGenPosixTime& theForecastTime,
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

bool CloudinessStory::hasStory(const string& theName)
{
  if (theName == "cloudiness_overview") return true;
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

Paragraph CloudinessStory::makeStory(const string& theName) const
{
  if (theName == "cloudiness_overview") return overview();

  throw TextGenError("CloudinessStory cannot make story " + theName);
}

}  // namespace TextGen

// ======================================================================
