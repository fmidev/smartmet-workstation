// ======================================================================
//
// Utilities for reading/ writing values from/to a character buffer
//
// ======================================================================

#include "NFmiEsriBuffer.h"
#include <algorithm>  // swap()
#include <iostream>
#include <cstring>

#ifdef UNIX
extern "C" {
#include <stdint.h>
}
#else
typedef unsigned int uint32_t;
#endif

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Test whether the CPU is little endian
//
// CPU is little endian, if integer 1 is represented in memory so that
// the value of the first byte (smallest memory address) is 1.
// ----------------------------------------------------------------------

bool NFmiEsriBuffer::IsCpuLittleEndian(void)
{
  int i = 1;
  return (*reinterpret_cast<unsigned char *>(&i) == 1);
}

// ----------------------------------------------------------------------
// Read a big endian integer from a char buffer
// ----------------------------------------------------------------------

int NFmiEsriBuffer::BigEndianInt(const std::string &theBuffer, int thePos)
{
  if (IsCpuLittleEndian())
  {
    unsigned char tmp[4];
    tmp[0] = theBuffer[thePos + 3];
    tmp[1] = theBuffer[thePos + 2];
    tmp[2] = theBuffer[thePos + 1];
    tmp[3] = theBuffer[thePos];
    return *reinterpret_cast<uint32_t *>(tmp);
  }
  else
    return *reinterpret_cast<const uint32_t *>(theBuffer.data() + thePos);
}

// ----------------------------------------------------------------------
// Read a little endian integer from a char buffer
// ----------------------------------------------------------------------

int NFmiEsriBuffer::LittleEndianInt(const std::string &theBuffer, int thePos)
{
  if (IsCpuLittleEndian())
    return *reinterpret_cast<const int *>(theBuffer.data() + thePos);
  else
  {
    unsigned char tmp[4];
    tmp[0] = theBuffer[thePos + 3];
    tmp[1] = theBuffer[thePos + 2];
    tmp[2] = theBuffer[thePos + 1];
    tmp[3] = theBuffer[thePos];
    return *reinterpret_cast<uint32_t *>(tmp);
  }
}

// ----------------------------------------------------------------------
// Read a little endian short integer from a char buffer
// Warning: Substituting *256 with <<8 does not work with g++
//          Must be some fancy undocumented feature...
// ----------------------------------------------------------------------

int NFmiEsriBuffer::LittleEndianShort(const std::string &theBuffer, int thePos)
{
  // The casts are necessary to get unsigned
  return (static_cast<unsigned char>(theBuffer[thePos]) +
          static_cast<unsigned char>(theBuffer[thePos + 1]) * 256);
}

// ----------------------------------------------------------------------
// Read a big endian short integer from a char buffer
// Warning: Substituting *256 with <<8 does not work with g++
//          Must be some fancy undocumented feature...
// ----------------------------------------------------------------------

int NFmiEsriBuffer::BigEndianShort(const std::string &theBuffer, int thePos)
{
  // The casts are necessary to get unsigned
  return (static_cast<unsigned char>(theBuffer[thePos]) * 256 +
          static_cast<unsigned char>(theBuffer[thePos + 1]));
}

// ----------------------------------------------------------------------
// Read a little endian double from a char buffer.
// ----------------------------------------------------------------------

double NFmiEsriBuffer::LittleEndianDouble(const std::string &theBuffer, int thePos)
{
  unsigned char tmp[8];

  if (IsCpuLittleEndian())
  {
    memcpy(tmp, theBuffer.data() + thePos, 8);
  }
  else
  {
    tmp[0] = theBuffer[thePos + 7];
    tmp[1] = theBuffer[thePos + 6];
    tmp[2] = theBuffer[thePos + 5];
    tmp[3] = theBuffer[thePos + 4];
    tmp[4] = theBuffer[thePos + 3];
    tmp[5] = theBuffer[thePos + 2];
    tmp[6] = theBuffer[thePos + 1];
    tmp[7] = theBuffer[thePos];
  }
  return *reinterpret_cast<const double *>(tmp);
}

// ----------------------------------------------------------------------
// Return a big endian integer buffer
// ----------------------------------------------------------------------

const string NFmiEsriBuffer::BigEndianInt(int theValue)
{
  int value = theValue;

  // Initialize string by reinterpreting 4 value bytes as characters

  unsigned char *svalue = reinterpret_cast<unsigned char *>(&value);

  // string tmp(svalue,4);  // Perkele kun ei toimi g++:ssa

  string tmp(4, '\0');
  for (int i = 0; i < 4; i++)
    tmp[i] = svalue[i];

  if (IsCpuLittleEndian())
  {
    swap(tmp[0], tmp[3]);
    swap(tmp[1], tmp[2]);
  }
  return tmp;
}

// ----------------------------------------------------------------------
// Return a little endian integer buffer
// ----------------------------------------------------------------------

const string NFmiEsriBuffer::LittleEndianInt(int theValue)
{
  int value = theValue;

  // Initialize string by reinterpreting 4 value bytes as characters

  unsigned char *svalue = reinterpret_cast<unsigned char *>(&value);

  // string tmp(svalue,4);  // Perkele kun ei toimi g++:ssa

  string tmp(4, '\0');
  for (int i = 0; i < 4; i++)
    tmp[i] = svalue[i];

  if (!IsCpuLittleEndian())
  {
    swap(tmp[0], tmp[3]);
    swap(tmp[1], tmp[2]);
  }
  return tmp;
}

// ----------------------------------------------------------------------
// Return a little endian unsigned short buffer
// ----------------------------------------------------------------------

const string NFmiEsriBuffer::LittleEndianShort(int theValue)
{
  string tmp(2, '\0');
  tmp[0] = theValue % 256;
  tmp[1] = theValue / 256;
  return tmp;
}

// ----------------------------------------------------------------------
// Return a little endian double buffer
// ----------------------------------------------------------------------

const string NFmiEsriBuffer::LittleEndianDouble(double theValue)
{
  double value = theValue;

  // Initialize string by reinterpreting 8 value bytes as characters

  unsigned char *svalue = reinterpret_cast<unsigned char *>(&value);

  // string tmp(svalue,8);  // Perkele kun ei toimi g++:ssa

  string tmp(8, '\0');
  for (int i = 0; i < 8; i++)
    tmp[i] = svalue[i];

  if (!IsCpuLittleEndian())
  {
    swap(tmp[0], tmp[7]);
    swap(tmp[1], tmp[6]);
    swap(tmp[2], tmp[5]);
    swap(tmp[3], tmp[4]);
  }
  return tmp;
}

// ----------------------------------------------------------------------
// Read the desired number of characters into given string
//
// Why this? Because std::string sucks. This should have been
// there.
//
// Returns true if got all characters.
// ----------------------------------------------------------------------

bool NFmiEsriBuffer::EsriRead(std::istream &is, string &theString, unsigned int theLength)
{
  // Note: Using a sizeable read-buffer for read is *significantly*
  // faster than reading 1 character at a time, the difference is
  // counted in seconds for largish files.

  // Note: It is essential that the string is resized to zero, since
  //       in loops the same string may be used to read records again
  //       and again. Without the resize the string would keep on
  //       growing, keeping the old contents in the beginning.

  theString.resize(0);
  theString.reserve(theLength);

  const int bufsize = 1024;

  char buffer[bufsize];
  int remainingsize = theLength;
  while (remainingsize > 0)
  {
    int readsize = (remainingsize < bufsize ? remainingsize : bufsize);
    is.read(buffer, readsize);
    theString.append(buffer, readsize);
    remainingsize -= readsize;
  }

  return (!is.fail() && (theString.size() == theLength));
}
}  // namespace Imagine

// ======================================================================
