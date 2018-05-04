// ======================================================================
/*!
 * \file
 * \brief Implementation of class NFmiColor
 */
// ======================================================================
/*!
 * \class NFmiColor
 *
 * \brief Color representation
 *
 * Color with alpha value 0 yields completely opaque color, whereas
 * alpha value 1 creates a fully transparent color, Intermediate
 * alpha values faciliate semitransparent colors.
 *
 * NOTE: This color class uses NON-FILTERING TRANSPARENCY (alpha transparency)
 * meaning that light color won't change when passing thru colored surfaces;
 * only the light intensity gets damped by the factor of alpha value in
 * question
 *
 */
// ======================================================================

#include "NFmiColor.h"

// Overlays foreground color 'theForegroundColor' on top of the
// current color based on the transparency of the foreground color
// only. The current color is treated as a completely opaque
// background color with zero transparency. The returned color
// will also be a 100% opaque color

void NFmiColor::Overlay(const NFmiColor& theForegroundColor)
{
  float foregroundTransparency = theForegroundColor.Alpha();

  if (foregroundTransparency == 1.0)
  {
    // Backround color can be seen clearly as a day
    return;
  }

  if (foregroundTransparency == 0.0)
  {
    // No backround color can be seen thru the
    // view-blocking foreground color
    SetRGBA(theForegroundColor);
    return;
  }

  // This is something inbetween: backround color is
  // partially obscured by the foreground color

  SetRGBA(
      foregroundTransparency * Red() + (1.0f - foregroundTransparency) * theForegroundColor.Red(),
      foregroundTransparency * Green() +
          (1.0f - foregroundTransparency) * theForegroundColor.Green(),
      foregroundTransparency * Blue() +
          (1.0f - foregroundTransparency) * theForegroundColor.Blue());
}
