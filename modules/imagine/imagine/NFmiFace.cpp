#include "imagine-config.h"

#ifndef IMAGINE_WITH_CAIRO

// ======================================================================
/*!
 * \file
 * \brief Implementation of class Imagine::NFmiFace
 */
// ======================================================================
/*!
 * \class Imagine::NFmiFace
 *
 * \brief A font class providing text rendering services
 *
 * The class implements a simple interface for rendering any
 * font type supported by the FreeType library. The face
 * objects are instantiated by giving the filename of the
 * font object as an argument to the constructor. If the
 * filename is relative, the search paths defined by
 * \code
 * imagine::font_path
 * \endcode
 * are used. Normally the value is something like
 * \code
 * imagine::font_path = /smartmet/share/fonts:/usr/lib/X11/fonts
 * \endcode
 *
 */
// ======================================================================

#ifdef UNIX

#include "NFmiFace.h"
#include "NFmiColorBlend.h"
#include "NFmiFreeType.h"
#include "NFmiImage.h"

#include <newbase/NFmiStringTools.h>

#include <stdexcept>
#include <vector>

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiFace::~NFmiFace() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor based on a font specification
 *
 * The font specification is of the form
 * \code
 * <fontname>:<width>x<height>
 * \endcode
 * where either width or height may be zero.
 */
// ----------------------------------------------------------------------

NFmiFace::NFmiFace(const std::string& theFontSpec)
    : itsBackgroundOn(false),
      itsBackgroundWidth(3),
      itsBackgroundHeight(3),
      itsBackgroundColor(NFmiColorTools::MakeColor(180, 180, 180, 32)),
      itsBackgroundRule(NFmiColorTools::kFmiColorOnOpaque)
{
  vector<string> words = NFmiStringTools::Split(theFontSpec, ":");
  if (words.size() != 2) throw runtime_error("Invalid font specification '" + theFontSpec + "'");
  itsFile = words[0];

  vector<int> sizes = NFmiStringTools::Split<vector<int> >(words[1], "x");
  if (sizes.size() != 2) throw runtime_error("Invalid font specification '" + theFontSpec + "'");

  itsWidth = sizes[0];
  itsHeight = sizes[1];
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor based on a font name and size
 */
// ----------------------------------------------------------------------

NFmiFace::NFmiFace(const std::string& theFile, int theWidth, int theHeight)
    : itsFile(theFile),
      itsWidth(theWidth),
      itsHeight(theHeight),
      itsBackgroundOn(false),
      itsBackgroundWidth(3),
      itsBackgroundHeight(3),
      itsBackgroundColor(NFmiColorTools::MakeColor(180, 180, 180, 32)),
      itsBackgroundRule(NFmiColorTools::kFmiColorOnOpaque)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Set background rendering on or off
 */
// ----------------------------------------------------------------------

void NFmiFace::Background(bool theMode) { itsBackgroundOn = theMode; }
// ----------------------------------------------------------------------
/*!
 * \brief Set background margins
 *
 * \param theWidth The extra padding in x-direction
 * \param theHeight The extra padding in y-direction
 */
// ----------------------------------------------------------------------

void NFmiFace::BackgroundMargin(int theWidth, int theHeight)
{
  if (theWidth < 0 || theHeight < 0) throw runtime_error("Background margins must be nonnegative");
  itsBackgroundWidth = theWidth;
  itsBackgroundHeight = theHeight;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set background color
 *
 * \param theColor The background color
 */
// ----------------------------------------------------------------------

void NFmiFace::BackgroundColor(NFmiColorTools::Color theColor) { itsBackgroundColor = theColor; }
// ----------------------------------------------------------------------
/*!
 * \brief Set background blending rule
 *
 * \param theRule The blending rule
 */
// ----------------------------------------------------------------------

void NFmiFace::BackgroundRule(NFmiColorTools::NFmiBlendRule theRule)
{
  itsBackgroundRule = theRule;
}

// ----------------------------------------------------------------------
/*!
 * \brief Render text onto image
 *
 * \param theText The text object to render
 * \param theImage The image to render into
 * \param theColor To color of the text
 * \param theRule The color blending rule
 */
// ----------------------------------------------------------------------

void NFmiFace::Draw(NFmiImage& theImage,
                    int theX,
                    int theY,
                    const string& theText,
                    NFmiAlignment theAlignment,
                    NFmiColorTools::Color theColor,
                    NFmiColorTools::NFmiBlendRule theRule) const
{
  NFmiFreeType::Instance().Draw(theImage,
                                itsFile,
                                itsWidth,
                                itsHeight,
                                theX,
                                theY,
                                theText,
                                theAlignment,
                                theColor,
                                theRule,
                                itsBackgroundOn,
                                itsBackgroundWidth,
                                itsBackgroundHeight,
                                itsBackgroundColor,
                                itsBackgroundRule);
}

}  // namespace Imagine

#endif  // UNIX

// ======================================================================

#endif
// IMAGINE_WITH_CAIRO
