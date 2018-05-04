// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::FrostStory
 */
// ======================================================================
/*!
 * \class TextGen::FrostStory
 *
 * \brief Generates stories on frost
 *
 * \see page_tarinat
 */
// ======================================================================

#include "FrostStory.h"
#include <calculator/AnalysisSources.h>
#include "Paragraph.h"
#include <calculator/TextGenError.h>

using namespace std;
using namespace TextGen;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

FrostStory::~FrostStory() {}
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

FrostStory::FrostStory(const TextGenPosixTime& theForecastTime,
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

bool FrostStory::hasStory(const string& theName)
{
  if (theName == "frost_mean") return true;
  if (theName == "frost_maximum") return true;
  if (theName == "frost_range") return true;
  if (theName == "frost_twonights") return true;
  if (theName == "frost_onenight") return true;
  if (theName == "frost_day") return true;
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

Paragraph FrostStory::makeStory(const string& theName) const
{
  if (theName == "frost_mean") return mean();
  if (theName == "frost_maximum") return maximum();
  if (theName == "frost_range") return range();
  if (theName == "frost_twonights") return twonights();
  if (theName == "frost_day") return day();
  if (theName == "frost_onenight") return onenight();

  throw TextGenError("FrostStory cannot make story " + theName);
}

}  // namespace TextGen

// ======================================================================
