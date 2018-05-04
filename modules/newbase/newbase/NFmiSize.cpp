// ======================================================================
/*!
 * \file NFmiSize.cpp
 * \brief Implementation of class NFmiSize
 */
// ======================================================================
/*!
 * \class NFmiSize
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiSize.h"
#include <fstream>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiSize::NFmiSize() : itsSize(0), itsIndex(-1) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

NFmiSize::NFmiSize(unsigned long theSize) : itsSize(theSize), itsIndex(-1) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theIndex Undocumented
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

NFmiSize::NFmiSize(unsigned long theIndex, unsigned long theSize)
    : itsSize(theSize), itsIndex(theIndex)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theSize The object being copied
 * \todo Use initialization lists
 */
// ----------------------------------------------------------------------

NFmiSize::NFmiSize(const NFmiSize& theSize) = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSize::First()
{
  Reset();
  return Next();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiSize::GetSize() const { return itsSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiSize::CurrentIndex() const { return itsIndex; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSize::Previous() { return --itsIndex >= 0; }
// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSize::SetSize(unsigned long newSize) { itsSize = newSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const char* NFmiSize::ClassName() const { return "NFmiSize"; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSize::Next()
{
  if (itsIndex + 1 < static_cast<long>(itsSize))
  {
    itsIndex++;
    return true;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \param directionToIter Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSize::Reset(FmiDirection directionToIter)
{
  if (directionToIter == kForward)
    itsIndex = -1;
  else
    itsIndex = GetSize();
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSize::SetCurrentIndex(unsigned long theIndex)
{
  if (theIndex < itsSize)
  {
    itsIndex = theIndex;
    return true;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * Write the NFmiSize object to the given putput stream.
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiSize::Write(std::ostream& file) const
{
  file << itsSize << "\n";
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Replace the active NFmiSize with the contents from the given
 * input stream.
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
//----------------------------------------------------------------------------

std::istream& NFmiSize::Read(std::istream& file)
{
  file >> itsSize;
  Reset();
  return file;
}

// ======================================================================
