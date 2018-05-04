// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::RelativeHumidityStory
 */
// ======================================================================
/*!
 * \class TextGen::RelativeHumidityStory
 *
 * \brief Generates stories on relativehumidity
 *
 * \see page_tarinat
 */
// ======================================================================

#include "RelativeHumidityStory.h"
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

RelativeHumidityStory::~RelativeHumidityStory() {}
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

RelativeHumidityStory::RelativeHumidityStory(const TextGenPosixTime& theForecastTime,
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

bool RelativeHumidityStory::hasStory(const string& theName)
{
  if (theName == "relativehumidity_lowest") return true;
  if (theName == "relativehumidity_day") return true;
  if (theName == "relativehumidity_range") return true;
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

Paragraph RelativeHumidityStory::makeStory(const string& theName) const
{
  if (theName == "relativehumidity_lowest") return lowest();
  if (theName == "relativehumidity_day") return day();
  if (theName == "relativehumidity_range") return range();

  throw TextGenError("RelativeHumidityStory cannot make story " + theName);
}

}  // namespace TextGen

// ======================================================================
