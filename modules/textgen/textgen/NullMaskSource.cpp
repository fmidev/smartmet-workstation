// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::NullMaskSource
 */
// ======================================================================
/*!
 * \class TextGen::NullMaskSource
 *
 * \brief Provides mask services to clients
 *
 * A null mask source which always returns an empty index mask.
 * Used as a substitute mask source for coasts, lands etc when
 * no coast or land area is specified.
 *
 */
// ======================================================================

#include "NullMaskSource.h"

#include <calculator/TextGenError.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherSource.h>
#include <newbase/NFmiIndexMask.h>

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

NullMaskSource::NullMaskSource() : itsData(new NFmiIndexMask()) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return the mask for the given area
 *
 * \param theArea The weather area
 * \param theData The data name
 * \param theWeatherSource The source for weather data
 */
// ----------------------------------------------------------------------

NullMaskSource::mask_type NullMaskSource::mask(const WeatherArea& theArea,
                                               const std::string& theData,
                                               const WeatherSource& theWeatherSource) const
{
  return itsData;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the mask source for the given area
 *
 * \param theArea The weather area
 * \param theData The data name
 * \param theWeatherSource The source for weather data
 */
// ----------------------------------------------------------------------

NullMaskSource::masks_type NullMaskSource::masks(const WeatherArea& theArea,
                                                 const std::string& theData,
                                                 const WeatherSource& theWeatherSource) const
{
  throw TextGenError("NullMaskSource::masks not implemented");
}

}  // namespace TextGen

// ======================================================================
