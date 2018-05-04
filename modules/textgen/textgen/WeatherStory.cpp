// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::WeatherStory
 */
// ======================================================================
/*!
 * \class TextGen::WeatherStory
 *
 * \brief Generates stories on weather
 *
 * \see page_tarinat
 */
// ======================================================================

#include "WeatherStory.h"
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

WeatherStory::~WeatherStory() {}
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

WeatherStory::WeatherStory(const TextGenPosixTime& theForecastTime,
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

bool WeatherStory::hasStory(const string& theName)
{
  if (theName == "weather_overview") return true;
  if (theName == "weather_forecast") return true;
  if (theName == "weather_shortoverview") return true;
  if (theName == "weather_thunderprobability") return true;
  if (theName == "weather_thunderprobability_simplified") return true;
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

Paragraph WeatherStory::makeStory(const string& theName) const
{
  if (theName == "weather_overview") return overview();
  if (theName == "weather_forecast") return forecast();
  if (theName == "weather_shortoverview") return shortoverview();
  if (theName == "weather_thunderprobability") return thunderprobability();
  if (theName == "weather_thunderprobability_simplified") return thunderprobability_simplified();
  throw TextGenError("WeatherStory cannot make story " + theName);
}

}  // namespace TextGen

// ======================================================================
