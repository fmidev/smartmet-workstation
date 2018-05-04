// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::WeatherResultTools
 */
// ======================================================================
/*!
 * \namespace TextGen::WeatherResultTools
 *
 * \brief Utility functions operating on WeatherResult objects
 */
// ======================================================================

#include "WeatherResultTools.h"
#include "MathTools.h"
#include "Settings.h"

#include <newbase/NFmiGlobals.h>
#include <cmath>
#include <cstdlib>
#include <utility>

namespace TextGen
{
namespace WeatherResultTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Return the result with smaller magnitude
 *
 * \param theResult1 The first result
 * \param theResult2 The second result
 * \return The smaller result
 */
// ----------------------------------------------------------------------

WeatherResult min(const WeatherResult& theResult1, const WeatherResult& theResult2)
{
  WeatherResult result(std::min(theResult1.value(), theResult2.value()),
                       std::max(theResult1.error(), theResult2.error()));
  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the result with greater magnitude
 *
 * \param theResult1 The first result
 * \param theResult2 The second result
 * \return The greater result
 */
// ----------------------------------------------------------------------

WeatherResult max(const WeatherResult& theResult1, const WeatherResult& theResult2)
{
  WeatherResult result(std::max(theResult1.value(), theResult2.value()),
                       std::max(theResult1.error(), theResult2.error()));
  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the mean result
 *
 * \param theResult1 The first result
 * \param theResult2 The second result
 * \return The mean result
 */
// ----------------------------------------------------------------------

WeatherResult mean(const WeatherResult& theResult1, const WeatherResult& theResult2)
{
  WeatherResult result(MathTools::mean(theResult1.value(), theResult2.value()),
                       std::max(theResult1.error(), theResult2.error()));
  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the smallest result
 *
 * \param theResult1 The first result
 * \param theResult2 The second result
 * \param theResult3 The third result result
 * \return The smallest result
 */
// ----------------------------------------------------------------------

WeatherResult min(const WeatherResult& theResult1,
                  const WeatherResult& theResult2,
                  const WeatherResult& theResult3)
{
  WeatherResult result(MathTools::min(theResult1.value(), theResult2.value(), theResult3.value()),
                       MathTools::max(theResult1.error(), theResult2.error(), theResult3.error()));
  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the largest result
 *
 * \param theResult1 The first result
 * \param theResult2 The second result
 * \param theResult3 The third result result
 * \return The largest result
 */
// ----------------------------------------------------------------------

WeatherResult max(const WeatherResult& theResult1,
                  const WeatherResult& theResult2,
                  const WeatherResult& theResult3)
{
  WeatherResult result(MathTools::max(theResult1.value(), theResult2.value(), theResult3.value()),
                       MathTools::max(theResult1.error(), theResult2.error(), theResult3.error()));
  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the mean result
 *
 * \param theResult1 The first result
 * \param theResult2 The second result
 * \param theResult3 The third result result
 * \return The mean result
 */
// ----------------------------------------------------------------------

WeatherResult mean(const WeatherResult& theResult1,
                   const WeatherResult& theResult2,
                   const WeatherResult& theResult3)
{
  WeatherResult result(MathTools::mean(theResult1.value(), theResult2.value(), theResult3.value()),
                       MathTools::max(theResult1.error(), theResult2.error(), theResult3.error()));
  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if two ranges are similar
 *
 * The control variables are
 * \code
 * ::same::minimum = [0-X] (=0)
 * ::same::maximum = [0-X] (=0)
 * \endcode
 *
 * \param theMinimum1 The first range minimum
 * \param theMaximum1 The first range maximum
 * \param theMinimum2 The second range minimum
 * \param theMaximum2 The second range maximum
 * \param theVar The variable containing similarity limits
 * \return True if the intervals are close enough
 */
// ----------------------------------------------------------------------

bool isSimilarRange(const WeatherResult& theMinimum1,
                    const WeatherResult& theMaximum1,
                    const WeatherResult& theMinimum2,
                    const WeatherResult& theMaximum2,
                    const std::string& theVar)
{
  using Settings::optional_int;

  const int same_minimum = optional_int(theVar + "::same::minimum", 0);
  const int same_maximum = optional_int(theVar + "::same::maximum", 0);

  const int minvalue1 = static_cast<int>(round(theMinimum1.value()));
  const int maxvalue1 = static_cast<int>(round(theMaximum1.value()));
  const int minvalue2 = static_cast<int>(round(theMinimum2.value()));
  const int maxvalue2 = static_cast<int>(round(theMaximum2.value()));

  return (abs(minvalue1 - minvalue2) <= same_minimum && abs(maxvalue1 - maxvalue2) <= same_maximum);
}

}  // namespace WeatherResultTools
}  // namespace TextGen

// ======================================================================
