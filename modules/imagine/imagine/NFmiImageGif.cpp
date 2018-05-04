// ======================================================================
//
// NFmiImage addendum - GIF reading and writing
//
// Code originally from the pbmplus package

// ======================================================================

#include "NFmiImage.h"
#include "NFmiEsriBuffer.h"

#include <set>
#include <map>
#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

namespace Imagine
{
//! Maximum number of colors allowed in a GIF file
const int MaxColors = 256;

//! The bit used to indicate interlacing
const unsigned char Interlace = 0x40;

//! The bit used to indicate a local color map
const unsigned char LocalColorMap = 0x80;

//! Maximum number of LZW code bits
const int MaxLzwBits = 12;

static bool zeroblock = false;

// ----------------------------------------------------------------------
/*!
 * \brief Test if bit is set
 */
// ----------------------------------------------------------------------

template <typename T>
inline bool bitset(T theByte, T theBit)
{
  return ((theByte & theBit) == theBit);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert chars to unsigned int
 */
// ----------------------------------------------------------------------

inline unsigned int LM_to_uint(unsigned char a, unsigned char b) { return ((b << 8) | a); }
// ----------------------------------------------------------------------
/*!
 * \brief Reader function
 */
// ----------------------------------------------------------------------

inline bool readok(FILE *theInput, unsigned char *theBuffer, int theLength)
{
  return (fread(theBuffer, theLength, 1, theInput) != 0);
}

// ----------------------------------------------------------------------
/*!
 * \brief GIF information holder
 */
// ----------------------------------------------------------------------

struct GifScreen
{
  unsigned int width;
  unsigned int height;
  unsigned char colormap[3][MaxColors];
  unsigned int bitpixel;
  unsigned int colorresolution;
  unsigned int background;
  unsigned int aspectratio;

  // Gif89

  int transparent;
  int delaytime;
  int inputflag;
  int disposal;

  GifScreen()
      : width(0),
        height(0),
        bitpixel(0),
        colorresolution(0),
        background(0),
        aspectratio(0),
        transparent(-1),
        delaytime(-1),
        inputflag(-1),
        disposal(0)
  {
  }
};

// ----------------------------------------------------------------------
/*!
 * \brief Read GIF color map from the input file
 */
// ----------------------------------------------------------------------

void read_colormap(FILE *theInput, int theBits, unsigned char theColorMap[3][MaxColors])
{
  unsigned char rgb[3];
  for (int i = 0; i < theBits; ++i)
  {
    if (!readok(theInput, rgb, 3)) throw runtime_error("Bad GIF colormap");
    theColorMap[0][i] = rgb[0];
    theColorMap[1][i] = rgb[1];
    theColorMap[2][i] = rgb[2];
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Read an extension data block
 */
// ----------------------------------------------------------------------

int getdatablock(FILE *theInput, unsigned char *theBuffer)
{
  int count;
  if ((count = fgetc(theInput)) == EOF) throw runtime_error("Block size missing from GIF stream");

  zeroblock = (count == 0);

  if (count != 0)
  {
    if (!readok(theInput, theBuffer, count)) throw runtime_error("Error reading GIF data block");
  }

  return count;
}

// ----------------------------------------------------------------------
/*!
 * \brief Read GIF extension block
 */
// ----------------------------------------------------------------------

void read_extension(FILE *theInput, int theLabel, GifScreen &theScreen)
{
  unsigned char buffer[256];

  switch (theLabel)
  {
    case 0x01:  // plain text extension
    case 0xff:  // application extension
    case 0xfe:  // comment extension
      break;
    case 0xf9:  // graphic control extension
    {
      getdatablock(theInput, buffer);
      theScreen.disposal = (buffer[0] >> 2) & 0x7;
      theScreen.inputflag = (buffer[0] >> 1) & 0x1;
      theScreen.delaytime = LM_to_uint(buffer[1], buffer[2]);
      if ((buffer[0] & 0x1) != 0) theScreen.transparent = buffer[3];
      break;
    }
    default:
      throw runtime_error("Unknown extension block in GIF");
  }

  // skip remaining info
  while (getdatablock(theInput, buffer) != 0)
    ;
}

// ----------------------------------------------------------------------
/*!
 * \brief Get a code from LZW input
 */
// ----------------------------------------------------------------------

int getcode(FILE *theInput, int theCodeSize, bool theFlushFlag)
{
  static unsigned char buf[280];
  static int curbit = 0;
  static int lastbit = 0;
  static int lastbyte = 0;
  static bool done = false;

  if (theFlushFlag)
  {
    curbit = 0;
    lastbit = 0;
    done = false;
    return 0;
  }

  if (curbit + theCodeSize >= lastbit)
  {
    if (done)
    {
      if (curbit >= lastbit) throw runtime_error("Ran off the end of GIF LZW bits");
      return -1;
    }

    buf[0] = buf[lastbyte - 2];
    buf[1] = buf[lastbyte - 1];

    int count = getdatablock(theInput, &buf[2]);

    if (count == 0) done = true;

    lastbyte = 2 + count;
    curbit = (curbit - lastbit) + 16;
    lastbit = (2 + count) * 8;
  }

  int ret = 0;
  for (int i = curbit, j = 0; j < theCodeSize; ++i, ++j)
    ret |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;

  curbit += theCodeSize;

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Read next LZW byte
 */
// ----------------------------------------------------------------------

int lzwreadbyte(FILE *theInput, bool theFlag, int theInputCodeSize)
{
  static int set_code_size = 0;
  static int code_size = 0;
  static int clear_code = 0;
  static int end_code = 0;
  static int max_code_size = 0;
  static int max_code = 0;
  static int firstcode = 0;
  static int oldcode = 0;
  static bool fresh = false;
  static int table[2][1 << MaxLzwBits];
  static int stack[(1 << (MaxLzwBits)) * 2], *sp;

  int i;

  if (theFlag)
  {
    set_code_size = theInputCodeSize;
    code_size = set_code_size + 1;
    clear_code = 1 << set_code_size;
    end_code = clear_code + 1;
    max_code_size = 2 * clear_code;
    max_code = clear_code + 2;

    getcode(theInput, 0, true);

    fresh = true;

    for (i = 0; i < clear_code; ++i)
    {
      table[0][i] = 0;
      table[1][i] = i;
    }
    for (; i < (1 << MaxLzwBits); ++i)
      table[0][i] = table[1][0] = 0;  // BUG???

    sp = stack;
    return 0;
  }

  else if (fresh)
  {
    fresh = false;
    do
      firstcode = oldcode = getcode(theInput, code_size, false);
    while (firstcode == clear_code);
    return firstcode;
  }

  if (sp > stack) return *--sp;

  int code;
  while ((code = getcode(theInput, code_size, false)) >= 0)
  {
    if (code == clear_code)
    {
      for (i = 0; i < clear_code; ++i)
      {
        table[0][i] = 0;
        table[1][i] = i;
      }
      for (; i < (1 << MaxLzwBits); ++i)
        table[0][i] = table[1][i] = 0;
      code_size = set_code_size + 1;
      max_code_size = 2 * clear_code;
      max_code = clear_code + 2;
      sp = stack;
      firstcode = oldcode = getcode(theInput, code_size, false);
      return firstcode;
    }
    else if (code == end_code)
    {
      if (zeroblock) return -2;

      unsigned char buf[260];
      while (getdatablock(theInput, buf) > 0)
        ;

      return -2;
    }

    int incode = code;

    if (code >= max_code)
    {
      *sp++ = firstcode;
      code = oldcode;
    }
    while (code >= clear_code)
    {
      *sp++ = table[1][code];
      if (code == table[0][code]) throw runtime_error("Circular table entry in GIF");
      code = table[0][code];
    }

    *sp++ = firstcode = table[1][code];

    code = max_code;
    if (code < (1 << MaxLzwBits))
    {
      table[0][code] = oldcode;
      table[1][code] = firstcode;
      ++max_code;
      if ((max_code >= max_code_size) && (max_code_size < (1 << MaxLzwBits)))
      {
        max_code_size *= 2;
        ++code_size;
      }
    }

    oldcode = incode;

    if (sp > stack) return *--sp;
  }

  return code;
}

// ----------------------------------------------------------------------
/*!
 * \brief Read image data from GIF
 */
// ----------------------------------------------------------------------

void read_image(FILE *theInput,
                NFmiImage &theImage,
                unsigned int theWidth,
                unsigned int theHeight,
                int theTransparent,
                const unsigned char theColorMap[3][MaxColors],
                bool theInterlaceFlag,
                bool theIgnoreFlag)
{
  // Initialize decompression

  int c;
  if ((c = fgetc(theInput)) == EOF) throw runtime_error("Image data missing in GIF");

  if (lzwreadbyte(theInput, true, c) < 0) throw runtime_error("Error read GIF image section");

  // If this is an uninteresting part of the GIF animation, ignore it

  if (theIgnoreFlag)
  {
    while (lzwreadbyte(theInput, false, c) >= 0)
      ;
    return;
  }

  unsigned int xpos = 0, ypos = 0, pass = 0;
  int v;
  while ((v = lzwreadbyte(theInput, false, c)) >= 0)
  {
    if (v == theTransparent)
      theImage(xpos, ypos) = NFmiColorTools::TransparentColor;
    else
      theImage(xpos, ypos) =
          NFmiColorTools::MakeColor(theColorMap[0][v], theColorMap[1][v], theColorMap[2][v]);
    ++xpos;
    if (xpos == theWidth)
    {
      xpos = 0;
      if (!theInterlaceFlag)
        ++ypos;
      else
      {
        switch (pass)
        {
          case 0:
            ypos += 8;
            break;
          case 1:
            ypos += 8;
            break;
          case 2:
            ypos += 4;
            break;
          case 3:
            ypos += 2;
            break;
        }
        if (ypos >= theHeight)
        {
          ++pass;
          switch (pass)
          {
            case 1:
              ypos = 4;
              break;
            case 2:
              ypos = 2;
              break;
            case 3:
              ypos = 1;
              break;
            default:
              goto fini;
          }
        }
      }
    }
    if (ypos >= theHeight) break;
  }

fini:

  // We do not throw since pbmplus merely warns about this
  if (lzwreadbyte(theInput, false, c) >= 0)
  {
  }  // throw runtime_error("Too much data in GIF image section");
}

// ----------------------------------------------------------------------
// Read GIF image
// ----------------------------------------------------------------------

void NFmiImage::ReadGIF(FILE *in)
{
  // read the first image, if the GIF is an animation
  const int imagenumber = 1;

  // General purpose read buffer
  unsigned char buf[16];

  // Read and verify the signature
  if (!readok(in, buf, 6)) throw runtime_error("Failed to read GIF magic number");

  if (buf[0] != 'G' || buf[1] != 'I' || buf[2] != 'F' || buf[3] != '8' ||
      (buf[4] != '7' && buf[4] != '9') || buf[5] != 'a')
  {
    throw runtime_error("GIF file being read does not have GIF87a or GIF89a signature");
  }

  // Read the screen descriptor
  if (!readok(in, buf, 7)) throw runtime_error("Failed to read GIF screen descriptor");

  // Establish the screen info

  GifScreen screen;
  screen.width = LM_to_uint(buf[0], buf[1]);
  screen.height = LM_to_uint(buf[2], buf[3]);
  screen.bitpixel = 2 << (buf[4] & 0x07);
  screen.colorresolution = (((buf[4] & 0x70) >> 3) + 1);
  screen.background = buf[5];
  screen.aspectratio = buf[6];

  // Read global color map if there is one

  if (bitset(buf[4], LocalColorMap))
  {
    read_colormap(in, screen.bitpixel, screen.colormap);
  }

  // We ignore the aspect ratio settings, do not even warn about it
  // See giftoppm code for more details

  // Allocate the image
  Allocate(screen.width, screen.height);

  // Now we enter a loop to read the image information

  int imagecount = 0;
  for (;;)
  {
    int c;
    if ((c = fgetc(in)) == EOF) throw runtime_error("GIF file ended abrubtly");

    // GIF terminator
    if (c == ';')
    {
      if (imagecount < imagenumber) throw runtime_error("Too few images in the GIF file");
      return;
    }

    // Extension block
    if (c == '!')
    {
      int label;
      if ((label = fgetc(in)) == EOF) throw runtime_error("Read error in GIF extension block");
      read_extension(in, label, screen);
      continue;
    }

    // Must be a start character then..
    if (c != ',')
      continue;  // throw runtime_error("Block start character missing from GIF stream");

    // Now we have an image to read
    ++imagecount;

    if (!readok(in, buf, 9)) throw runtime_error("Failed to read GIF frame size");

    const bool use_global_colormap = !bitset(buf[8], LocalColorMap);
    const int bitpixel = 1 << ((buf[8] & 0x07) + 1);

    const int width = LM_to_uint(buf[4], buf[5]);
    const int height = LM_to_uint(buf[6], buf[7]);
    const bool interlace = bitset(buf[8], Interlace);
    const bool ignore = (imagecount != imagenumber);

    if (use_global_colormap)
    {
      read_image(in, *this, width, height, screen.transparent, screen.colormap, interlace, ignore);
    }
    else
    {
      unsigned char colormap[3][MaxColors];
      read_colormap(in, bitpixel, colormap);
      read_image(in, *this, width, height, screen.transparent, colormap, interlace, ignore);
    }
  }
}

// ----------------------------------------------------------------------
// Write GIF image
// ----------------------------------------------------------------------

// ImageMagick defines

#define MaxCode(number_bits) ((1 << (number_bits)) - 1)
#define MaxHashTable 5003
#define MaxGIFBits 12
#define MaxGIFTable (1 << MaxGIFBits)
#define GIFOutputCode(code)                          \
  {                                                  \
    if (bitsnow > 0)                                 \
      datum |= (static_cast<long>(code) << bitsnow); \
    else                                             \
      datum = static_cast<long>(code);               \
    bitsnow += number_bits;                          \
    while (bitsnow >= 8)                             \
    {                                                \
      packet[byte_count++] = (datum & 0xff);         \
      if (byte_count >= 254)                         \
      {                                              \
        fputc(byte_count, out);                      \
        fwrite(packet.c_str(), 1, byte_count, out);  \
        byte_count = 0;                              \
      }                                              \
      datum >>= 8;                                   \
      bitsnow -= 8;                                  \
    }                                                \
    if (free_code > max_code)                        \
    {                                                \
      number_bits++;                                 \
      if (number_bits == MaxGIFBits)                 \
        max_code = MaxGIFTable;                      \
      else                                           \
        max_code = MaxCode(number_bits);             \
    }                                                \
  }

void NFmiImage::WriteGIF(FILE *out) const
{
  int i, j;

  // Establish whether a palette version can be made

  set<NFmiColorTools::Color> theColors;

// Opaque threshold must be > 0, we have binary transparency

#if (defined IMAGINE_FORMAT_JPEG) || (defined IMAGINE_FORMAT_PNG)
  int opaquethreshold = itsAlphaLimit;
  if (opaquethreshold < 0) opaquethreshold = NFmiColorTools::MaxAlpha / 2;

  bool ignorealpha = !itsSaveAlphaFlag;
#else
  int opaquethreshold = NFmiColorTools::MaxAlpha / 2;
  bool ignorealpha = true;  // Mikä arvo tälle?
#endif

  bool overflow = AddColors(theColors, MaxColors, opaquethreshold, ignorealpha);

  // If overflow occurred, we must quantize the image

  if (overflow)
  {
    cerr << "Error: Quantization not implemented in WriteGIF" << endl;
    return;
  }

  // Build the palette

  vector<NFmiColorTools::Color> colors;
  map<NFmiColorTools::Color, int> colormap;
  set<NFmiColorTools::Color>::const_iterator iter;

  bool hastransparent = false;
  int num_colors = 0;

  for (iter = theColors.begin(); iter != theColors.end(); iter++)
  {
    if (NFmiColorTools::GetAlpha(*iter) > 0)
      hastransparent = true;
    else
    {
      colormap.insert(make_pair(*iter, num_colors++));
      colors.push_back(*iter);
    }
  }

  int total_colors = num_colors;
  if (hastransparent) total_colors++;

  // The number of bits needed per pixel. Note that we must use
  // total_colors instead of num_colors, in case there is
  // a transparent color index.

  int bits;
  for (bits = 1; bits < 8; bits++)
    if (1 << bits >= total_colors) break;

  // Write the magic number

  string header = "";
  header.reserve(1000);  // An overestimated header size (3*256+n)

  if (hastransparent)
    header += "GIF89a";
  else
    header += "GIF87a";

  // Image width and height

  header += NFmiEsriBuffer::LittleEndianShort(itsWidth);
  header += NFmiEsriBuffer::LittleEndianShort(itsHeight);

  // Mode flags

  unsigned char mode = 0x80;  // Global color map on
  mode |= (8 - 1) << 4;       // Resolution
  mode |= bits - 1;           // Bits per pixel

  header += mode;

  // Background color chosen to be color 0

  header += '\0';

  // Future expansion byte

  header += '\0';

  // The global color map

  unsigned char r, g, b;

  for (i = 0; i < num_colors; i++)
  {
    r = static_cast<unsigned char>(NFmiColorTools::GetRed(colors[i]));
    g = static_cast<unsigned char>(NFmiColorTools::GetGreen(colors[i]));
    b = static_cast<unsigned char>(NFmiColorTools::GetBlue(colors[i]));
    header += r;
    header += g;
    header += b;
  }

  // Pad with zeros to get 2^bits colors. Note that using
  // header += "\x00\x00\x00" would not work, since it is
  // interpreted as a c-string and thus terminates immediately.
  // Using string("\x00\x00\x00") would not work either.

  const int mapsize = 1 << bits;
  for (i = num_colors; i < mapsize; i++)
    for (j = 0; j < 3; j++)
      header += '\x00';  // This wouldn't work: += "\x00\x00\x00

  // Write GIF89a extension if necessary

  if (hastransparent)
  {
    header += '\x21';
    header += '\xf9';
    header += '\x04';
    header += '\x01';                                  // matte flag
    header += NFmiEsriBuffer::LittleEndianShort(0);    // delay=0
    header += static_cast<unsigned char>(num_colors);  // trans. idx
    header += '\x00';
  }

  // Comment block would go here - we don't output one

  // Initial code size

  unsigned char initcodesize = std::max(bits, 2);

  header += ',';                                   // image separator
  header += NFmiEsriBuffer::LittleEndianShort(0);  // left offset
  header += NFmiEsriBuffer::LittleEndianShort(0);  // top offset
  header += NFmiEsriBuffer::LittleEndianShort(itsWidth);
  header += NFmiEsriBuffer::LittleEndianShort(itsHeight);
  header += '\x00';
  header += initcodesize;  // initial code size

  fwrite(header.c_str(), 1, header.size(), out);

  // Write the raster itself

  // Allocate encoder tables

  string packet(256, '\0');
  string hash_suffix(MaxHashTable, '\0');

  vector<short> hash_code(MaxHashTable, 0);
  vector<short> hash_prefix(MaxHashTable, 0);

  // Initialize GIF encoder

  const short data_size = std::max(bits, 2) + 1;
  const short clear_code = 1 << (data_size - 1);
  const short end_code = clear_code + 1;

  int number_bits = data_size;
  short max_code = MaxCode(number_bits);
  short free_code = clear_code + 2;
  int byte_count = 0;
  long datum = 0;
  int bitsnow = 0;

  GIFOutputCode(clear_code);

  // Encode pixels

  short waiting_code = 0;

  for (int pixpos = 0; pixpos < itsWidth * itsHeight; pixpos++)
  {
    // Get the color, simplify it

    NFmiColorTools::Color c = itsPixels[pixpos];
    c = NFmiColorTools::Simplify(c, opaquethreshold, ignorealpha);

    // Convert to colormap index

    int a = NFmiColorTools::GetAlpha(c);
    short index = (a == 0 ? colormap[c] : num_colors);

    // Probe hash table

    int k = (static_cast<int>(index) << (MaxGIFBits - 8)) + waiting_code;

    if (k >= MaxHashTable) k -= MaxHashTable;

    bool next_pixel = false;
    int displacement = 1;

    if (hash_code[k] > 0)
    {
      if ((hash_prefix[k] == waiting_code) && (hash_suffix[k] == index))
      {
        waiting_code = hash_code[k];
        continue;
      }
      if (k != 0) displacement = MaxHashTable - k;
      for (;;)
      {
        k -= displacement;
        if (k < 0) k += MaxHashTable;
        if (hash_code[k] == 0) break;
        if ((hash_prefix[k] == waiting_code) && (hash_suffix[k] == index))
        {
          waiting_code = hash_code[k];
          next_pixel = true;
          break;
        }
      }
      if (next_pixel) continue;
    }
    GIFOutputCode(waiting_code);
    if (free_code < MaxGIFTable)
    {
      hash_code[k] = free_code++;
      hash_prefix[k] = waiting_code;
      hash_suffix[k] = index;
    }
    else
    {
      for (k = 0; k < MaxHashTable; k++)
        hash_code[k] = 0;
      free_code = clear_code + 2;
      GIFOutputCode(clear_code);
      number_bits = data_size;
      max_code = MaxCode(number_bits);
    }
    waiting_code = index;
  }

  // Flush out the buffered code

  GIFOutputCode(waiting_code);
  GIFOutputCode(end_code);
  if (bitsnow > 0) packet[byte_count++] = (datum & 0xff);

  // Flush accumulated data.

  if (byte_count > 0)
  {
    fputc(byte_count, out);
    fwrite(packet.c_str(), 1, byte_count, out);
    byte_count = 0;
  }

  // End GIF writing

  fputc(0, out);    // zero-length packet to end data
  fputc(';', out);  // GIF terminator
}

}  // namespace Imagine

// ======================================================================
