#ifndef IMAGINE_WITH_CAIRO

// ======================================================================
/*!
 * \file
 * \brief Implementation of singleton class Imagine::NFmiFreeType
 */
// ======================================================================
/*!
 * \class Imagine::NFmiFreeType
 *
 * \brief FreeType instance management class
 *
 * The class makes sure FreeType is always properly initialized
 * on entry and FreeType memory deallocated on exit.
 *
 * Any code requiring a face must always use
 * \code
 * NFmiFreeType::Instance().Face(name,width,size)
 * \endcode
 * The singleton makes sure the FreeType library is initialized
 * when the first face is requested.
 *
 */
// ======================================================================

#ifdef UNIX

#include "NFmiFreeType.h"
#include "NFmiColorBlend.h"
#include "NFmiFace.h"
#include "NFmiPath.h"
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiSettings.h>

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
}

// Required on Mandrake 9.0 (freetype 9.0.3, freetype 9.3.3 is fine)

#include FT_GLYPH_H
#ifndef FT_PIXEL_MODE_GRAY
#define FT_PIXEL_MODE_GRAY ft_pixel_mode_grays
#define FT_KERNING_DEFAULT ft_kerning_default
#define FT_RENDER_MODE_NORMAL ft_render_mode_normal
#endif

#include <cmath>
#include <map>
#include <stdexcept>
#include <vector>

using namespace std;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Compute glyph sequence bounding box
 */
// ----------------------------------------------------------------------

FT_BBox compute_bbox(const vector<FT_Glyph>& theGlyphs, const vector<FT_Vector>& thePositions)
{
  FT_BBox bbox;

  // initialize string bbox to "empty" values
  bbox.xMin = bbox.yMin = 32000;
  bbox.xMax = bbox.yMax = -32000;

  // for each glyph image, compute its bounding box,
  // translate it, and grow the string bbox

  for (string::size_type i = 0; i < theGlyphs.size(); i++)
  {
    FT_BBox glyph_bbox;
    FT_Glyph_Get_CBox(theGlyphs[i], ft_glyph_bbox_pixels, &glyph_bbox);

    glyph_bbox.xMin += thePositions[i].x;
    glyph_bbox.xMax += thePositions[i].x;
    glyph_bbox.yMin += thePositions[i].y;
    glyph_bbox.yMax += thePositions[i].y;

    if (glyph_bbox.xMin < bbox.xMin) bbox.xMin = glyph_bbox.xMin;
    if (glyph_bbox.yMin < bbox.yMin) bbox.yMin = glyph_bbox.yMin;
    if (glyph_bbox.xMax > bbox.xMax) bbox.xMax = glyph_bbox.xMax;
    if (glyph_bbox.yMax > bbox.yMax) bbox.yMax = glyph_bbox.yMax;
  }

  // check that we really grew the string bbox

  if (bbox.xMin > bbox.xMax)
  {
    bbox.xMin = 0;
    bbox.yMin = 0;
    bbox.xMax = 0;
    bbox.yMax = 0;
  }

  // return string bbox

  return bbox;
}

}  // namespace anonymous

namespace Imagine
{
// ----------------------------------------------------------------------
/*!
 * \brief Implementation hiding pimple
 */
// ----------------------------------------------------------------------

class NFmiFreeType::Pimple
{
 public:
  typedef map<string, FT_Face> Faces;

  bool itsInitialized;
  FT_Library itsLibrary;             //!< Freetype library reference
  Faces itsFaces;                    //!< Face name to Face ID mapping
  map<string, string> itsFontPaths;  //!< Font name to path cache

 public:
  ~Pimple();
  Pimple();

  const string& findFont(const string& theName);

  FT_Face getFont(const string& theFont);

  template <class T>
  void Draw(T theBlender,
            const FT_Face& theFace,
            NFmiImage& theImage,
            FT_Int theX,
            FT_Int theY,
            const string& theText,
            NFmiAlignment theAlignment,
            NFmiColorTools::Color theColor,
            bool theBackgroundOn,
            int theBackgroundWidth,
            int theBackgroundHeight,
            NFmiColorTools::Color theBackgroundColor,
            NFmiColorTools::NFmiBlendRule theBackgroundRule);

  template <class T>
  void Draw(T theBlender,
            NFmiImage& theImage,
            NFmiColorTools::Color theColor,
            const FT_Bitmap& theBitmap,
            FT_Int theX,
            FT_Int theY);

};  // class NFmiFreeType::Pimple

// ----------------------------------------------------------------------
/*!
 * \brief Pimple destructor
 */
// ----------------------------------------------------------------------

NFmiFreeType::Pimple::~Pimple()
{
// According to valgrind this should NOT be done
#if 0
	for(Faces::iterator it = itsFaces.begin(); it != itsFaces.end(); ++it)
	  delete it->second;
#endif  // UNIX

  if (itsInitialized) FT_Done_FreeType(itsLibrary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Pimple constructor initializes Freetype
 *
 * Note that the Pimple constructor is called only when
 * NFmiFreeType::Instance is called for the first time.
 * This implies rendering is about to be done, and hence
 * initialization should be done.
 *
 * The Pimple is NOT constructed before the Instance method
 * is called, which ensures no CPU time is wasted if
 * Freetype is not used.
 *
 */
// ----------------------------------------------------------------------

NFmiFreeType::Pimple::Pimple() : itsInitialized(false), itsLibrary()
{
  FT_Error error = FT_Init_FreeType(&itsLibrary);
  if (error) throw runtime_error("Initializing FreeType failed");

  itsInitialized = true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Find a FreeType font face
 */
// ----------------------------------------------------------------------

const string& NFmiFreeType::Pimple::findFont(const string& theName)
{
  map<string, string>::const_iterator it = itsFontPaths.find(theName);

  if (it != itsFontPaths.end()) return it->second;

  const string path = NFmiSettings::Optional<string>("imagine::font_path", ".");
  const string file = NFmiFileSystem::FileComplete(theName, path);

  return (itsFontPaths[theName] = file);
}

// ----------------------------------------------------------------------
/*!
 * \brief Get a FreeType font face
 */
// ----------------------------------------------------------------------

FT_Face NFmiFreeType::Pimple::getFont(const string& theFont)
{
  Faces::const_iterator it = itsFaces.find(theFont);
  if (it != itsFaces.end()) return it->second;

  FT_Face face;

  FT_Error error = FT_New_Face(itsLibrary, theFont.c_str(), 0, &face);

  if (error == FT_Err_Unknown_File_Format)
    throw runtime_error("Unknown font format in '" + theFont + "'");

  if (error) throw runtime_error("Failed while reading font '" + theFont + "'");

  itsFaces.insert(Faces::value_type(theFont, face));

  return face;
}

// ----------------------------------------------------------------------
/*!
 * \brief Render the given text
 */
// ----------------------------------------------------------------------

template <class T>
void NFmiFreeType::Pimple::Draw(T theBlender,
                                const FT_Face& theFace,
                                NFmiImage& theImage,
                                FT_Int theX,
                                FT_Int theY,
                                const string& theText,
                                NFmiAlignment theAlignment,
                                NFmiColorTools::Color theColor,
                                bool theBackgroundOn,
                                int theBackgroundWidth,
                                int theBackgroundHeight,
                                NFmiColorTools::Color theBackgroundColor,
                                NFmiColorTools::NFmiBlendRule theBackgroundRule)
{
  FT_Error error;
  FT_GlyphSlot slot = theFace->glyph;
  FT_UInt glyph_index;
  FT_Vector pen;

  vector<FT_Glyph> glyphs(theText.size());
  vector<FT_Vector> pos(theText.size());

  // start at (0,0)
  pen.x = 0;
  pen.y = 0;

  FT_Bool use_kerning = FT_HAS_KERNING(theFace);
  FT_UInt previous = 0;

  string::size_type glyphpos = 0;
  string::size_type i = 0;
  while (i < theText.size())
  {
    unsigned short ch = static_cast<unsigned char>(theText[i]);

    if (ch < 0xc0)
    {
      ++i;
    }
    else if (ch < 0xe0)
    {
      if (i + 1 < theText.size() && static_cast<unsigned char>(theText[i + 1] & 0xc0) == 0x80)
      {
        ch = ((theText[i] & 0x1f) << 6 | (theText[i + 1] & 0x3f));
        i += 2;
      }
      else
        ++i;
    }
    else if (ch < 0xf0)
    {
      if (i + 2 < theText.size() && static_cast<unsigned char>(theText[i + 1] & 0xc0) == 0x80 &&
          static_cast<unsigned char>(theText[i + 2] & 0xc0) == 0x80)
      {
        ch = ((theText[i] & 0xf) << 12 | (theText[i + 1] & 0x3f) << 6 | (theText[i + 2] & 0x3f));
        i += 3;
      }
      else
        ++i;
    }
    else
    {
      if (i + 3 < theText.size() && static_cast<unsigned char>(theText[i + 1] & 0xc0) == 0x80 &&
          static_cast<unsigned char>(theText[i + 2] & 0xc0) == 0x80 &&
          static_cast<unsigned char>(theText[i + 3] & 0xc0) == 0x80)
      {
        ch = ((theText[i] & 0xf) << 18 | (theText[i + 1] & 0x3f) << 12 |
              (theText[i + 2] & 0x3f << 6) | (theText[i + 3] & 0x3f));
        i += 4;
      }
      else
        ++i;
    }

    glyph_index = FT_Get_Char_Index(theFace, ch);

    // Retrieve kerning distance and move pen accordingly
    if (use_kerning && previous != 0 && glyph_index != 0)
    {
      FT_Vector delta;
      FT_Get_Kerning(theFace, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
      pen.x += (delta.x >> 6);
    }

    // load glyph image into the slot without rendering

    error = FT_Load_Glyph(theFace, glyph_index, FT_LOAD_DEFAULT);
    if (error) continue;

    // Extract glyph image and store it in our table
    error = FT_Get_Glyph(theFace->glyph, &glyphs[glyphpos]);
    if (error) continue;

    // store current pen position

    pos[glyphpos].x = pen.x;
    pos[glyphpos].y = pen.y;

#if 1
    FT_Glyph image = glyphs[glyphpos];
    error = FT_Glyph_To_Bitmap(&image, FT_RENDER_MODE_NORMAL, &pen, 0);
    if (!error)
    {
      FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(image);
      pos[glyphpos].x += bit->left;
      pos[glyphpos].y -= bit->top;
    }
#endif

    glyphpos++;

    // Increment pen position
    pen.x += (slot->advance.x >> 6);

    // Record current glyph index
    previous = glyph_index;
  }

  // Compute bounding box
  const FT_BBox bbox = compute_bbox(glyphs, pos);

  // string pixel size

  const int width = bbox.xMax - bbox.xMin;
  const int height = bbox.yMax - bbox.yMin;

  // Compute start position in 26.6 cartesian pixels

  const double xfactor = XAlignmentFactor(theAlignment);
  const double yfactor = YAlignmentFactor(theAlignment);

  const int start_x = static_cast<int>(round(theX - xfactor * width));
  const int start_y = static_cast<int>(round(theY + (1 - yfactor) * height));

  // Render the background

  if (theBackgroundOn)
  {
    const int x1 = start_x - theBackgroundWidth;
    const int y2 = start_y + theBackgroundHeight;
    const int x2 = x1 + width + 2 * theBackgroundWidth;
    const int y1 = y2 - height - 2 * theBackgroundHeight;

    NFmiPath path;
    path.MoveTo(x1, y1);
    path.LineTo(x2, y1);
    path.LineTo(x2, y2);
    path.LineTo(x1, y2);
    path.CloseLineTo();

    path.Fill(theImage, theBackgroundColor, theBackgroundRule);
  }

  // And render the glyphs

  for (i = 0; i < glyphpos; i++)
  {
    FT_Glyph image = glyphs[i];

    pen.x = start_x + pos[i].x;
    pen.y = start_y + pos[i].y;

    error = FT_Glyph_To_Bitmap(&image, FT_RENDER_MODE_NORMAL, &pen, 0);

    if (!error)
    {
      FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(image);

#if 0
			this->Draw(theBlender, theImage, theColor,
					   bit->bitmap,
					   pen.x + bit->left,
					   pen.y - bit->top);
#else
      this->Draw(theBlender, theImage, theColor, bit->bitmap, pen.x, pen.y);
#endif

      FT_Done_Glyph(image);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Render the given glyph
 */
// ----------------------------------------------------------------------

template <class T>
void NFmiFreeType::Pimple::Draw(T theBlender,
                                NFmiImage& theImage,
                                NFmiColorTools::Color theColor,
                                const FT_Bitmap& theBitmap,
                                FT_Int theX,
                                FT_Int theY)
{
  FT_Int i, j, p, q;
  FT_Int x = theX;
  FT_Int y = theY;

  FT_Int x_max = x + theBitmap.width;
  FT_Int y_max = y + theBitmap.rows;

  for (i = x, p = 0; i < x_max; i++, p++)
  {
    for (j = y, q = 0; j < y_max; j++, q++)
    {
      if (i < 0 || j < 0 || i >= theImage.Width() || j >= theImage.Height()) continue;

      int alpha = 0;
      if (theBitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
        alpha = theBitmap.buffer[q * theBitmap.width + p];
      else
      {
        int value = theBitmap.buffer[q * (theBitmap.pitch) + (p >> 3)];
        int bit = (value << (p & 7)) & 128;
        alpha = (bit != 0 ? 255 : 0);
      }

      if (alpha == 255)
        theImage(i, j) = theBlender.Blend(theColor, theImage(i, j));
      else
      {
        int a = NFmiColorTools::GetAlpha(theColor);
        int aa = static_cast<int>(a + (1.0 - alpha / 255.0) * (NFmiColorTools::MaxAlpha - a));
        NFmiColorTools::Color c = NFmiColorTools::ReplaceAlpha(theColor, aa);
        theImage(i, j) = theBlender.Blend(c, theImage(i, j));
      }
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiFreeType::~NFmiFreeType() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor used privately by Instance()
 */
// ----------------------------------------------------------------------

NFmiFreeType::NFmiFreeType() : itsPimple(new Pimple()) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return an instance of NFmiFreeType
 *
 * \return A reference to a NFmiFreeType singleton
 */
// ----------------------------------------------------------------------

NFmiFreeType& NFmiFreeType::Instance()
{
  static NFmiFreeType freetype;
  return freetype;
}

// ----------------------------------------------------------------------
/*!
 * \brief Render text onto image
 *
 */
// ----------------------------------------------------------------------

void NFmiFreeType::Draw(NFmiImage& theImage,
                        const std::string& theFont,
                        int theWidth,
                        int theHeight,
                        int theX,
                        int theY,
                        const string& theText,
                        NFmiAlignment theAlignment,
                        NFmiColorTools::Color theColor,
                        NFmiColorTools::NFmiBlendRule theRule,
                        bool theBackgroundOn,
                        int theBackgroundWidth,
                        int theBackgroundHeight,
                        NFmiColorTools::Color theBackgroundColor,
                        NFmiColorTools::NFmiBlendRule theBackgroundRule) const
{
  // Note that it is not possible to call this method except via
  // Instance(), which ensures the Pimple and hence Freetype
  // have been properly initialized

  // Quick exit if color is not real

  if (theColor == NFmiColorTools::NoColor) return;

  // When the color is opaque or transparent, some rules will simplify.
  // Instead of using ifs in the innermost loop, we will simplify the
  // rule itself here.

  int alpha = NFmiColorTools::GetAlpha(theColor);
  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::Simplify(theRule, alpha);
  // If the result is ColorKeep, the source alpha is such that there
  // is nothing to do!

  if (rule == NFmiColorTools::kFmiColorKeep) return;

  // Now we construct the needed FT_Face object

  if (theWidth < 0 || theHeight < 0)
    throw runtime_error("Face width and height cannot both be zero");

  // Find the face

  const string file = itsPimple->findFont(theFont);

  // Create the face

  FT_Face face = itsPimple->getFont(file);

  FT_Error error = FT_Set_Pixel_Sizes(face, theWidth, theHeight);

  if (error)
    throw runtime_error("Failed to set font size " + NFmiStringTools::Convert(theWidth) + 'x' +
                        NFmiStringTools::Convert(theHeight) + " in font '" + file + "'");
  // And render

  switch (rule)
  {
    case NFmiColorTools::kFmiColorClear:
      itsPimple->Draw(NFmiColorBlendClear(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorCopy:
      itsPimple->Draw(NFmiColorBlendCopy(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorAddContrast:
      itsPimple->Draw(NFmiColorBlendAddContrast(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorReduceContrast:
      itsPimple->Draw(NFmiColorBlendReduceConstrast(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorOver:
      itsPimple->Draw(NFmiColorBlendOver(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorUnder:
      itsPimple->Draw(NFmiColorBlendUnder(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorIn:
      itsPimple->Draw(NFmiColorBlendIn(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorKeepIn:
      itsPimple->Draw(NFmiColorBlendKeepIn(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorOut:
      itsPimple->Draw(NFmiColorBlendOut(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorKeepOut:
      itsPimple->Draw(NFmiColorBlendKeepOut(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorAtop:
      itsPimple->Draw(NFmiColorBlendAtop(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorKeepAtop:
      itsPimple->Draw(NFmiColorBlendKeepAtop(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorXor:
      itsPimple->Draw(NFmiColorBlendXor(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorPlus:
      itsPimple->Draw(NFmiColorBlendPlus(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorMinus:
      itsPimple->Draw(NFmiColorBlendMinus(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorAdd:
      itsPimple->Draw(NFmiColorBlendAdd(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorSubstract:
      itsPimple->Draw(NFmiColorBlendSubstract(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorMultiply:
      itsPimple->Draw(NFmiColorBlendMultiply(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorDifference:
      itsPimple->Draw(NFmiColorBlendDifference(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorCopyRed:
      itsPimple->Draw(NFmiColorBlendCopyRed(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorCopyGreen:
      itsPimple->Draw(NFmiColorBlendCopyGreen(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorCopyBlue:
      itsPimple->Draw(NFmiColorBlendCopyBlue(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorCopyMatte:
      itsPimple->Draw(NFmiColorBlendCopyMatte(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorCopyHue:
      itsPimple->Draw(NFmiColorBlendCopyHue(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorCopyLightness:
      itsPimple->Draw(NFmiColorBlendCopyLightness(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorCopySaturation:
      itsPimple->Draw(NFmiColorBlendCopySaturation(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorKeepMatte:
      itsPimple->Draw(NFmiColorBlendKeepMatte(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorKeepHue:
      itsPimple->Draw(NFmiColorBlendKeepHue(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorKeepLightness:
      itsPimple->Draw(NFmiColorBlendKeepLightness(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorKeepSaturation:
      itsPimple->Draw(NFmiColorBlendKeepSaturation(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorBumpmap:
      itsPimple->Draw(NFmiColorBlendBumpmap(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorDentmap:
      itsPimple->Draw(NFmiColorBlendDentmap(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorOnOpaque:
      itsPimple->Draw(NFmiColorBlendOnOpaque(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;
    case NFmiColorTools::kFmiColorOnTransparent:
      itsPimple->Draw(NFmiColorBlendOnTransparent(),
                      face,
                      theImage,
                      theX,
                      theY,
                      theText,
                      theAlignment,
                      theColor,
                      theBackgroundOn,
                      theBackgroundWidth,
                      theBackgroundHeight,
                      theBackgroundColor,
                      theBackgroundRule);
      break;

    // Some special cases
    case NFmiColorTools::kFmiColorKeep:
    case NFmiColorTools::kFmiColorRuleMissing:
      break;
  }
}

}  // namespace Imagine

#endif  // UNIX

// ======================================================================

#endif
// IMAGINE_WITH_CAIRO
