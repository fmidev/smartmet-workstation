/*
* ImagineXr.cpp
*
* Replacement for 'Imagine' library, using Cairo drawing
*/
#include "imagine-config.h"

#ifdef IMAGINE_WITH_CAIRO

#include "ImagineXr.h"

#include "NFmiImage.h"

#include <iostream>
#include <stdexcept>
#include <deque>

#include <boost/algorithm/string.hpp>

#define DEFAULT_LINE_WIDTH 0.4  // Cairo itself has 2.0 as default
                                // 0.2 is too little; starts graying the lines out

#ifdef IMAGINE_USAGE
#define USAGE(msg, ...) fprintf(stderr, ">>> #%d: " msg "\n", __LINE__, __VA_ARGS__)
#define USAGE0(msg) USAGE("%s", msg)
#else
// no debugging
#define USAGE(msg, ...) /* nothing */
#define USAGE0(msg)     /* nothing */
#endif

#define WARNING(msg, ...) fprintf(stderr, "*** WARNING: " msg "\n", __VA_ARGS__)

// Note: 'ColorName()' does not give has stuff, only pre-known colors are named
//
#ifdef IMAGINE_USAGE
static const char *COLORNAME(NFmiColorTools::Color col)
{
  static char buf[100];
  sprintf(buf,
          "%02x%02x%02x%02x",
          127 - NFmiColorTools::GetAlpha(col),
          NFmiColorTools::GetRed(col),
          NFmiColorTools::GetGreen(col),
          NFmiColorTools::GetBlue(col));
  return buf;
}
#define ALIGNNAME(v) AlignmentName(v).c_str()
#endif

#define RULENAME(v) BlendName(v).c_str()

static void Defaults(Cairo::RefPtr<Cairo::Context> cr)
{
  cr->set_line_width(DEFAULT_LINE_WIDTH);

  cr->set_fill_rule(FILL_RULE_EVEN_ODD);

  // Note: not using antialias is not really beneficial with Cairo;
  //       it's not faster and causes breakage in shallow lines.
  //
  cr->set_antialias(ANTIALIAS_DEFAULT);
  //
  // ANTIALIAS_DEFAULT / ANTIALIAS_NONE / ANTIALIAS_GRAY / ANTIALIAS_SUBPIXEL
}

ImagineXr::ImagineXr(int w, int h, const string &to_fn, const string &fmt_)
    : fn(to_fn), fmt(fmt_), width(w), height(h), face(0), font_bg_color(NFmiColorTools::NoColor)
{
  USAGE(
      "ImagineXr(): format=%s, fn=%s width=%d, height=%d", fmt.c_str(), fn.c_str(), width, height);

  if (fn == "") throw runtime_error("No output filename given!");

  if (fmt == "pdf")
  {
    pdf_surf = Cairo::PdfSurface::create(fn, width, height);
  }
  else
  {
    /* 'ImageSurface' does not need the filename at construction.
    */
    image_surf = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
  }

  Defaults(cr = Cairo::Context::create(surf()));
}

ImagineXr::ImagineXr(const string &from_png_fn)
    : fn(""), fmt("png"), face(0), font_bg_color(NFmiColorTools::NoColor)
{
  USAGE("ImagineXr(): from_fn=%s", from_png_fn.c_str());
  image_surf = Cairo::ImageSurface::create_from_png(from_png_fn);
  width = image_surf->get_width();
  height = image_surf->get_height();

  Defaults(cr = Cairo::Context::create(image_surf));
}

/*
* Temporary image, not to be saved on disk.
*/
ImagineXr::ImagineXr(int w, int h)
    : fn(""), fmt("png"), width(w), height(h), face(0), font_bg_color(NFmiColorTools::NoColor)
{
  USAGE("ImagineXr(): width=%d, height=%d", width, height);

  image_surf = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
  Defaults(cr = Cairo::Context::create(surf()));
}

ImagineXr::ImagineXr(const ImagineXr &o)
    : pdf_surf(o.pdf_surf),
      image_surf(o.image_surf),
      fn(o.fn),
      fmt(o.fmt),
      width(o.width),
      height(o.height),
      face(0),
      font_bg_color(NFmiColorTools::NoColor)
{
  USAGE0("ImagineXr(): copy");
  Defaults(cr = Cairo::Context::create(surf()));
}

/*
* Destructor: do nothing
*/
ImagineXr::~ImagineXr() {}
void ImagineXr::SetRGB(NFmiColorTools::Color col)
{
  if (col == NFmiColorTools::NoColor) return;

  cr->set_source_rgb(NFmiColorTools::GetRed(col) / 255.0,
                     NFmiColorTools::GetGreen(col) / 255.0,
                     NFmiColorTools::GetBlue(col) / 255.0);
}

void ImagineXr::SetRGBA(NFmiColorTools::Color col)
{
  if (col == NFmiColorTools::NoColor) return;

  cr->set_source_rgba(NFmiColorTools::GetRed(col) / 255.0,
                      NFmiColorTools::GetGreen(col) / 255.0,
                      NFmiColorTools::GetBlue(col) / 255.0,
                      1.0 - (NFmiColorTools::GetAlpha(col) / 127.0));
}

void ImagineXr::SetBlend(enum NFmiColorTools::NFmiBlendRule rule)
{
  Cairo::Operator op;
  //
  // OPERATOR_CLEAR 	    nothing in the combined image
  // OPERATOR_SOURCE 	    only SOURCE in the combined image
  // OPERATOR_OVER        SOURCE over DEST in the combined image
  // OPERATOR_IN          only SOURCE that overlaps DEST will be drawn
  // OPERATOR_OUT         only SOURCE that does not overlap DEST will be drawn
  // OPERATOR_ATOP        SOURCE that overlaps DEST, on top of rest of DEST
  // OPERATOR_DEST        like above, inverted
  // OPERATOR_DEST_OVER 	like above, inverted
  // OPERATOR_DEST_IN     like above, inverted
  // OPERATOR_DEST_OUT    like above, inverted
  // OPERATOR_DEST_ATOP   like above, inverted
  // OPERATOR_XOR         parts of SOURCE and DEST that don't overlap are drawn
  // OPERATOR_ADD
  // OPERATOR_SATURATE
  //
  // Ref: i.e. <http://www.ibm.com/developerworks/java/library/j-mer0918/>

  // There are many more 'NFmiColorTools' blending rules than what Cairo knows.
  //
  switch (rule)
  {
    case kFmiColorRuleMissing:
      return;  // no rule; change nothing

    /* Standard rules by Porter-Duff
    */
    case kFmiColorClear:
      op = OPERATOR_CLEAR;
      break;  // Fs=0, Fd=0
    case kFmiColorCopy:
      op = OPERATOR_SOURCE;
      break;  // Fs=1, Fd=0
    case kFmiColorKeep:
      op = OPERATOR_DEST;
      break;  // Fs=0, Fd=1
    case kFmiColorOver:
      op = OPERATOR_OVER;
      break;  // Fs=1, Fd=1-As
    case kFmiColorUnder:
      op = OPERATOR_DEST_OVER;
      break;  // Fs=1-Ad, Fd=1
    case kFmiColorIn:
      op = OPERATOR_IN;
      break;  // Fs=Ad, Fd=0
    case kFmiColorKeepIn:
      op = OPERATOR_DEST_IN;
      break;  // Fs=0, Fd=As
    case kFmiColorOut:
      op = OPERATOR_OUT;
      break;  // Fs=1-Ad, Fd=0
    case kFmiColorKeepOut:
      op = OPERATOR_DEST_OUT;
      break;  // Fs=0, Fd=1-As
    case kFmiColorAtop:
      op = OPERATOR_ATOP;
      break;  // FS=Ad, Fd=1-As
    case kFmiColorKeepAtop:
      op = OPERATOR_DEST_ATOP;
      break;  // FS=1-Ad, Fd=As
    case kFmiColorXor:
      op = OPERATOR_XOR;
      break;  // FS=1-Ad, Fd=1-As

    /* Additional rules not by Porter-Duff
    */
    case kFmiColorPlus:
      op = OPERATOR_ADD;
      break;  // Fs=1, Fd=1

    default:
      WARNING("Blending rule not supported: %s", RULENAME(rule));
      return;
  }

  // "Sets the compositing operator to be used for all drawing operations."
  //
  cr->set_operator(op);
}

/*
* Path for filling
*
* Returns 'true' if there were elements, 'false' if path empty
*/
bool ImagineXr::SetPath(const std::deque<NFmiPathElement> &path)
{
  std::deque<NFmiPathElement>::const_iterator iter = path.begin();
  if (iter == path.end()) return false;  // no elements (don't close either)

  for (; iter != path.end(); ++iter)
  {
    float x = iter->x;
    float y = iter->y;

    switch (iter->op)
    {
      case kFmiMoveTo:
        USAGE("\t\tmove_to( %.2f, %.2f )", x, y);
        cr->move_to(x, y);
#if 0
    // DEBUG
    { static unsigned int nnn=1;
    char buf[10]; sprintf( buf, "%d", nnn++ );
    cr->save();
    cr->set_source_rgb( 0,0,0 );    // black
    cr->show_text( string(buf) );
    cr->restore(); }
#endif
        break;

      case kFmiLineTo:
      case kFmiGhostLineTo:
        USAGE("\t\tline_to( %.2f, %.2f )", x, y);
        cr->line_to(x, y);
        break;

      // 'kFmiConicTo' and 'kFmiCubicTo' aren't used (says Mika H.)
      //
      default:
        throw runtime_error("Unexpected path primitive");
    }
  }
  return true;
}

/*
* Modify 'x' and 'y' by the chosen alignment
*/
void ImagineXr::ApplyAlignment(enum NFmiAlignment alignment, int &x, int &y, int w, int h)
{
  switch (alignment)
  {
    case kFmiAlignMissing:
    case kFmiAlignNorthWest:
      break;  // no modifications

    case kFmiAlignCenter:
      x -= w / 2;
      y -= h / 2;
      break;
    case kFmiAlignNorth:
      x -= w / 2;
      break;
    case kFmiAlignNorthEast:
      x -= w;
      break;
    case kFmiAlignEast:
      x -= w;
      y -= h / 2;
      break;
    case kFmiAlignSouthEast:
      x -= w;
      y -= h;
      break;
    case kFmiAlignSouth:
      x -= w / 2;
      y -= h;
      break;
    case kFmiAlignSouthWest:
      y -= h;
      break;
    case kFmiAlignWest:
      y -= h / 2;
      break;
  }
}

const int32_t *ImagineXr::ARGB_32() const
{
  if (!image_surf) throw runtime_error("Pixels not supported for PDF canvas!");

  return (const int32_t *)image_surf->get_data();
}

/*
* Cairo alpha:
*   "Pre-multiplied alpha is used (50% transparent red is 0x80800000, not 0x80ff0000)"
*
*   This also means opaque A is 0xff, full transparency 0x00000000. In other
*   words A is stored negated.
*
* NFmiColorTools::Color:
*   0..0x7f alpha (0=opaque), separate of RGB (they always range 0..0xff)
*/
static inline NFmiColorTools::Color ARGB32_to_NFmiColor(int32_t v)
{
  int a = (v >> 24) & 0xff;

  if (a == 0xff)
    return v & 0x00ffffff;  // fully opaque (RGB are non-scaled)
  else if (a == 0)
    return 0x7f000000;  // fully transparent (no color information)
  else
  {
    // Transparency 1..0xfe; need to scale up RGB
    //
    double scale = 255.0 / double(a);  // range (1..255]

    int rgb = (int)(double(v & 0x00ffffff) * scale);
    int a = (int)(scale / 2.0 + 0.5);  // 127=full alpha

    if (a == 127)
      a = 126;  // we don't come here with full alpha; neither should we leave
    else if (a == 0)
      a = 1;  // nor opaqueness

    return a << 24 | rgb;
  }
}

static inline int32_t /* ARGB_32 */ NFmiColor_to_ARGB32(NFmiColorTools::Color v)
{
  int a = (v >> 24) & 0xff;

  if (a == 0)
    return v | 0xff000000;  // fully opaque (RGB are non-scaled)
  else if (a == 0x7f)
    return 0;  // fully transparent (no color information)
  else
  {
    // Transparency 1..0x7e; need to scale up RGB
    //
    double scale = 255.0 / double(a);  // range (1..255]

    int rgb = (int)(double(v & 0x00ffffff) * scale);
    int a = (int)(scale / 2.0 + 0.5);  // 127=full alpha

    if (a == 127)
      a = 126;  // we don't come here with full alpha; neither should we leave
    else if (a == 0)
      a = 1;  // (nor opaqueness)

    return a << 24 | rgb;
  }
}

const NFmiColorTools::Color *ImagineXr::NFmiColorBuf(NFmiColorTools::Color *buf) const
{
  unsigned n = Width() * Height();
  const int32_t *argb = ARGB_32();

  for (unsigned i = 0; i < n; i++)
  {
    buf[i] = ARGB32_to_NFmiColor(argb[i]);
  }
  return buf;
}

/*
* Get a single point from the current bitmap
*
* Note: NFmiColorTools has 'A' as opacity value (0=transparent) whereas Cairo
*       has it as transparency (0=opaque).
*/
NFmiColorTools::Color ImagineXr::operator()(int x, int y) const
{
  int w = Width();
  int h = Height();

  USAGE("operator (): x=%d, y=%d", x, y);

  if (x < 0 || y < 0 || x >= w || y >= h) return NFmiColorTools::NoColor;  // out of bounds

  return ARGB32_to_NFmiColor(ARGB_32()[y * w + x]);
}

/*
* Paint the whole surface with 'color' (an ARGB_32 value)
*/
void ImagineXr::Erase(const NFmiColorTools::Color &color)
{
  USAGE("Erase(): color=%s", COLORNAME(color));
  SetRGB(color);
  cr->paint();
}

/*
* Composite image over another (one stamp)
*/
void ImagineXr::Composite(const ImagineXr &img2,
                          NFmiColorTools::NFmiBlendRule rule,
                          NFmiAlignment alignment,
                          int x,
                          int y,
                          float opaque)
{
  USAGE("Composite(): rule=%s, align=%s, x=%d, y=%d, opaque=%.3f",
        RULENAME(rule),
        ALIGNNAME(alignment),
        x,
        y,
        opaque);

  ApplyAlignment(alignment, x, y, img2.Width(), img2.Height());

  /* This part is tricky to be backward compatible; several alternatives below.
  *
  * QD-Contour test code uses 'Copy' blending rule with us; and seems to
  * imply it should only affect the size of the source, not elsewhere.
  *
  * Cairo takes 'Copy' to mean the remaining target picture is wiped out,
  * leaving only the last stamped image on it (rest is white).
  */

  // Selected this one in a meeting with Mika H. and Mikko R. 21-Aug-2008:
  // ignoring the alpha/opaqueness factor (kept in the calling interface
  // for easier old/new merger in QD-Contour)

  // Use given rule; limit changes in image to the size of the stamp.
  // This works, and is backward compatible, but ignores 'opaque'ness
  // because there is no 'cr->fill_with_alpha()'.
  //
  SetBlend(rule);

  cr->set_source(img2.image_surf, x, y);
  cr->rectangle(x, y, img2.Width(), img2.Height());
  cr->fill();

  if (opaque != 1.0)
  {
    WARNING("Ignored transparency %.2f in compositing for Cairo", opaque);
  }

  /** Another alternative (turned back):
      //
      // Temporarily switch to OVER blending rule (will make it work, but...
      // essentially ignores 'rule' unless it's 'Copy' and the stamping image
      // has no transparent areas)
      //
      SetBlend( kFmiColorOver );  (void)rule;

      cr->set_source( img2.image_surf, x,y );

      // 'opaque' (not '1.0-opaque') gives right result: 1.0 causes full
      // "stamping", 0.0 no stamp at all.
      //
      cr->paint_with_alpha( opaque );
  **/
}

/*
*
*/
void ImagineXr::Stroke(const std::deque<NFmiPathElement> &path,
                       float line_width,
                       NFmiColorTools::Color color,
                       NFmiColorTools::NFmiBlendRule rule)
{
  USAGE(
      "Stroke(): line_width=%.2f, color=%s, rule=%s", line_width, COLORNAME(color), RULENAME(rule));

  SetBlend(rule);
  SetRGBA(color);

  if (SetPath(path))
  {
    cr->set_line_width(line_width);
    cr->stroke();
  }
}

/*
* Path fill
*/
void ImagineXr::Fill(const std::deque<NFmiPathElement> &path,
                     NFmiColorTools::Color color,
                     NFmiBlendRule rule)
{
  USAGE("Fill(): color=%s, rule=%s", COLORNAME(color), RULENAME(rule));

  SetBlend(rule);
  SetRGBA(color);

  if (SetPath(path))
  {
    cr->fill();
  }
}

/*
* Fill an area with a 'pattern' image
*/
void ImagineXr::Fill(const std::deque<NFmiPathElement> &path,
                     const ImagineXr &img2,
                     NFmiBlendRule rule,
                     float opaque)
{
  USAGE("Fill(): pattern=yyy, rule=%s, opaque=%.3f", RULENAME(rule), opaque);

  SetBlend(rule);
  if (!SetPath(path)) return;  // get out if no path

  RefPtr<SurfacePattern> pattern = SurfacePattern::create(img2.surf());
  pattern->set_extend(EXTEND_REPEAT);

  cr->set_source(pattern);

// Simple optimization:
//      PNG generation on Cairo 1.6.4: no speedup measured
//      PDF generation on Cairo 1.6.4: 837ms vs. 893ms measured (6.7% speedup)
//
#if 1
  if (opaque == 1.0)
  {  // 308kB without this
    cr->fill();
    return;
  }
#endif

  // Clipping here seems to be ok even for PDF output (tested on Cairo 1.6.4)
  //
  cr->clip();
  cr->paint_with_alpha(opaque);  // seems to be needed as 'opaque', not '1.0-opaque'
  cr->reset_clip();
}

/*
* Prepare a font to be used by 'DrawFace()'
*
* 'fontspec':
*   "<fontname>:<points>"
*   "<fontname>:<width>x<height>"   old format, not supported for Cairo (i.e.
* "misc/6x13B.pcf.gz:6x13")
*
* TBD: Add FONT_SLANT, FONT_WEIGHT options to the string.
*/
void ImagineXr::MakeFace(const string &fontspec,
                         const NFmiColorTools::Color backcolor,
                         int xm,
                         int ym)
{
  USAGE("MakeFace(): fontspec=%s, backcolor=%s, xm=%d, ym=%d",
        fontspec.c_str(),
        COLORNAME(backcolor),
        xm,
        ym);

  string font_name = "Courier";
  int font_size = 9;

  vector<string> to_vec;
  boost::split(to_vec, fontspec, boost::is_any_of(":,"));

  if (strchr(to_vec[1].c_str(), 'x'))
  {
    WARNING("Old style fontspec '%s' replaced with default (use \"<fontname>:<points>\" instead).",
            fontspec.c_str());
  }
  else
  {
    font_name = to_vec[0];
    font_size = atoi(to_vec[1].c_str());  // hehe, C++ sure (gimme regexps)
  }

  cr->select_font_face(font_name, FONT_SLANT_NORMAL, FONT_WEIGHT_NORMAL);
  cr->set_font_size(font_size);

  font_bg_color = backcolor;
  font_bg_mx = xm;
  font_bg_my = ym;
}

/*
* Returns 'false' if 'text' was not in UTF-8 format.
*/
bool ImagineXr::DrawFace(int x_,
                         int y_,
                         const std::string &text,
                         NFmiColorTools::Color color,
                         NFmiAlignment alignment,
                         NFmiBlendRule rule)
{
  bool ok = true;

  USAGE("DrawFace(): x=%d, y=%d, text=%s, color=%s, align=%s, rule=%s",
        x_,
        y_,
        text.c_str(),
        COLORNAME(color),
        ALIGNNAME(alignment),
        RULENAME(rule));

  /*
  * 'text' needs to be UTF-8 encoded; otherwise Cairo throws an exception.
  */
  try
  {
    TextExtents te;
    cr->get_text_extents(text, te);

    int x = x_, y = y_;
    ApplyAlignment(alignment, x, y, (int)(te.width + 0.5), (int)(te.height + 0.5));

    // Paint a background box if set up so
    //
    if (font_bg_color != NFmiColorTools::NoColor)
    {
      int mx = font_bg_mx;
      int my = font_bg_my;

      SetBlend(kFmiColorCopy);
      SetRGBA(font_bg_color);

      cr->rectangle(x - mx, y - my, te.width + 2 * mx, te.height + 2 * my);
      cr->fill();
    }

    SetBlend(rule);
    SetRGBA(color);

    cr->move_to(x - 1, y + te.height);
    cr->show_text(text);
  }
  catch (Cairo::logic_error err)
  {
    /* Cairo (1.6.4) exceptions behave REALLY funny: for non-valid UTF8, it
    * throws "input string not valid UTF8". Replacing that with 'X' throws
    * another exception. At one point the exception was "success".
    *
    * Better just to not print anything if we get an exception?
    */
    string what(err.what());

    // "input string not valid UTF8"
    WARNING("%s: %s", what.c_str(), text.c_str());

    ok = false;
  }
  return ok;
}

/*
* Write IMAGE surface to ".png" file, PDF surface to ".pdf"
*/
void ImagineXr::Write() const
{
  if (fn == "") throw runtime_error("Temporary image; not intended to be written");

  if (pdf_surf)
  {
    // Nothing we can (have to) do; file is most likely already on the disk
  }
  else
  {
    if (fmt != "png") throw runtime_error(string("Cairo can write only to png format, not") + fmt);

    image_surf->write_to_png(fn);
  }
}

#endif
// IMAGINE_WITH_CAIRO
