#include "imagine-config.h"

#ifndef IMAGINE_WITH_CAIRO
// ======================================================================
/*!
 * \file NFmiDrawable.cpp
 * \brief Default implementations for the interface of a drawable object.
 *
 * \b History:
 *
 * \li 27.08.2001, Mika Heiskanen\par
 * Implemented.
 *
 */
// ======================================================================

#include "NFmiDrawable.h"
#include "NFmiImage.h"
#include "NFmiFillMap.h"

namespace Imagine
{
// ----------------------------------------------------------------------
/*!
 * Fill onto given image using various colour blending rules.
 * Most drawables are rendered by creating a NFmiFillMap and then rendering
 * the fillmap. Note that NFmiFillMap is a NFmiDrawable, but it has an
 * overloaded Fill method to replace the one provided here.
 */
// ----------------------------------------------------------------------

void NFmiDrawable::Fill(NFmiImage& theImage,
                        NFmiColorTools::Color theColor,
                        NFmiColorTools::NFmiBlendRule theRule) const
{
  // Quick exit if color is not real

  if (theColor == NFmiColorTools::NoColor) return;

  // Create fillmap, clip it based on image height

  NFmiFillMap fmap(0.0, theImage.Height());

  // Add the drawable to the fillmap

  Add(fmap);

  // Render the fill map

  fmap.Fill(theImage, theColor, theRule);
}

// ----------------------------------------------------------------------
/*!
 * Fill onto given image using the given pattern.
 * All drawables are rendered by creating a NFmiFillMap and then rendering
 * the fillmap. Note that NFmiFillMap is a NFmiDrawable, but it has an
 * overloaded Fill method to replace the one provided here.
 */
// ----------------------------------------------------------------------

void NFmiDrawable::Fill(NFmiImage& theImage,
                        const NFmiImage& thePattern,
                        NFmiColorTools::NFmiBlendRule theRule,
                        float theAlphaFactor) const
{
  // Create fillmap, clip it based on image height

  NFmiFillMap fmap(0.0, theImage.Height());

  // Add the drawable to the fillmap

  Add(fmap);

  // Render the fill map

  fmap.Fill(theImage, thePattern, theRule, theAlphaFactor);
}

// ----------------------------------------------------------------------
/*!
 * Stroke onto given image using various colour blending rules.
 * At the moment there are no common parts for stroking the
 * various NFmiDrawable objects, hence this is empty. However, in
 * the future we may add a Path method for NFmiDrawable objects, which
 * may then be used to stroke the object.
 */
// ----------------------------------------------------------------------

void NFmiDrawable::Stroke(NFmiImage& theImage,
                          NFmiColorTools::Color theColor,
                          NFmiColorTools::NFmiBlendRule theRule) const
{
}

}  // namespace Imagine

// ======================================================================

#endif
// not IMAGINE_WITH_CAIRO
