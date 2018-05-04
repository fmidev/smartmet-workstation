// ======================================================================
/*!
 * \file NFmiBox.cpp
 * \brief Implementation of class NFmiBox
 */
// ======================================================================
/*!
 * \class NFmiBox
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiBox.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiBox::~NFmiBox() { delete[] itsValues; }
// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiBox::Initialize(unsigned long newSize)
{
  itsSize = newSize;
  if (itsValues)
  {
    delete itsValues;
    itsValues = nullptr;
  }
  if (itsSize > 0) itsValues = new float[itsSize];
  return itsValues ? true : false;
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiBox::Write(std::ostream& file) const
{
  for (unsigned long i = 0; i < itsSize; i++)
    file << itsValues[i] << " ";

  file << std::endl;

  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream& NFmiBox::Read(std::istream& file)
{
  for (unsigned long i = 0; i < itsSize; i++)
    file >> itsValues[i];

  return file;
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theBox The object being copied
 */
// ----------------------------------------------------------------------

NFmiBox::NFmiBox(NFmiBox& theBox) : itsValues(new float[theBox.itsSize]), itsSize(theBox.itsSize)
{
  for (unsigned long i = 0; i < itsSize; i++)
  {
    itsValues[i] = theBox.itsValues[i];
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

NFmiBox::NFmiBox(long theSize) : itsValues(nullptr), itsSize(theSize)
{
  if (itsSize) itsValues = new float[itsSize];
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiBox::GetSize() const { return itsSize; }
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float& NFmiBox::operator[](long theIndex) { return itsValues[theIndex]; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiBox::CalcSize() { return 0; }
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiBox::IsInside(unsigned long theIndex) const { return (theIndex < itsSize); }
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiBox::Value(unsigned long theIndex)
{
  return IsInside(theIndex) ? itsValues[theIndex] : kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theFillValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

void NFmiBox::Fill(float theFillValue)
{
  for (unsigned long theIndex = 0; theIndex < itsSize; theIndex++)
    itsValues[theIndex] = theFillValue;
}

// ----------------------------------------------------------------------
