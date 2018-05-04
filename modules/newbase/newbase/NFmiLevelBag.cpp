// ======================================================================
/*!
 * \file NFmiLevelBag.cpp
 * \brief Implementation of class NFmiLevelBag
 */
// ======================================================================
/*!
 * \class NFmiLevelBag
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiLevelBag.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLevelBag::~NFmiLevelBag()
{
  if (itsLevels) delete[] static_cast<NFmiLevel *>(itsLevels);
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLevelBag::NFmiLevelBag() : itsLevels(nullptr), itsStep(0) {}
// ----------------------------------------------------------------------
/*!
 * \param theLevelType Undocumented
 * \param theMinValue Undocumented
 * \param theMaxValue Undocumented
 * \param theStep Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevelBag::NFmiLevelBag(FmiLevelType theLevelType,
                           float theMinValue,
                           float theMaxValue,
                           float theStep)
    : NFmiSize(theStep ? static_cast<unsigned long>(((theMaxValue - theMinValue) / theStep) + 1)
                       : 1),
      itsLevels(nullptr),
      itsStep(theStep)
{
  itsLevels = new NFmiLevel[itsSize];
  for (unsigned long i = 0; i < itsSize; i++)
    itsLevels[i] = NFmiLevel(theLevelType, theMinValue + i * theStep);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theLevelArray Undocumented
 * \param numOfLevels Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevelBag::NFmiLevelBag(NFmiLevel *theLevelArray, unsigned long numOfLevels)
    : NFmiSize(numOfLevels),
      itsLevels(theLevelArray ? new NFmiLevel[GetSize()] : nullptr),
      itsStep(0ul)
{
  for (unsigned int i = 0; i < GetSize(); i++)
    itsLevels[i] = theLevelArray[i];
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theBag The other object being copied
 */
// ----------------------------------------------------------------------

NFmiLevelBag::NFmiLevelBag(const NFmiLevelBag &theBag)
    : NFmiSize(theBag.itsIndex, theBag.itsSize),
      itsLevels(theBag.itsLevels ? new NFmiLevel[itsSize] : nullptr),
      itsStep(theBag.itsStep)
{
  for (unsigned int i = 0; i < itsSize; i++)
    itsLevels[i] = (theBag.itsLevels)[i];
}

// ----------------------------------------------------------------------
/*!
 * \param theBag Undocumented
 * \todo Toimiiko vai eikö? Oli Markon epäilevä kommentti
 */
// ----------------------------------------------------------------------

const NFmiLevelBag NFmiLevelBag::Combine(const NFmiLevelBag &theBag) const
{
  return NFmiLevelBag(theBag);
}

// ----------------------------------------------------------------------
/*!
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLevelBag::Level(const NFmiLevel &theLevel)
{
  Reset();
  while (Next())
  {
    if (theLevel == *Level()) return true;
  }

  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLevelBag::AddLevel(const NFmiLevel &theLevel)
{
  int i;
  for (i = 0; i < static_cast<int>(itsSize); i++)
    if (itsLevels[i] == theLevel) break;

  if (i == static_cast<int>(itsSize))
  {
    int j;
    NFmiLevel *tempLevels;
    tempLevels = new NFmiLevel[GetSize() + 1];

    for (j = 0; j < static_cast<int>(itsSize); j++)
      tempLevels[j] = itsLevels[j];

    tempLevels[j] = theLevel;

    if (itsLevels) delete[] itsLevels;

    itsLevels = new NFmiLevel[itsSize + 1];
    itsSize = GetSize() + 1;
    for (j = 0; j < static_cast<int>(GetSize()); j++)
      itsLevels[j] = tempLevels[j];

    itsStep = 0ul;  // askel ei välttämättä enää vakio

    delete[] tempLevels;

    return true;
  }

  return false;
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theLevelBag The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiLevelBag &NFmiLevelBag::operator=(const NFmiLevelBag &theLevelBag)
{
  itsIndex = theLevelBag.CurrentIndex();
  itsSize = theLevelBag.GetSize();

  delete[] itsLevels;  // Salla 140998

  itsLevels = new NFmiLevel[theLevelBag.GetSize()];
  for (int i = 0; i < static_cast<int>(theLevelBag.GetSize()); i++)
    itsLevels[i] = theLevelBag.itsLevels[i];

  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison operator
 *
 * \param theLevelBag The bag being compared to
 * \return True, if the bags are equal
 */
// ----------------------------------------------------------------------

bool NFmiLevelBag::operator==(const NFmiLevelBag &theLevelBag) const
{
  bool retVal = false;
  if (itsSize == theLevelBag.itsSize)
  {
    for (int i = 0;
         i < static_cast<int>(this->GetSize()) && static_cast<int>(theLevelBag.GetSize());
         i++)
    {
      if (!(this->itsLevels[i] == theLevelBag.itsLevels[i]))
        return false;
      else
        retVal = true;
    }
  }
  return retVal;
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiLevelBag::Write(std::ostream &file) const
{
  NFmiSize::Write(file);

  for (unsigned long i = 0; i < GetSize(); i++)
  {
    file << itsLevels[i];
  }

  file << itsStep << std::endl;
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiLevelBag::Read(std::istream &file)
{
  if (itsLevels) delete[] static_cast<NFmiLevel *>(itsLevels);

  NFmiSize::Read(file);
  if (GetSize())  // 11.02.97 Vili Add size check
    itsLevels = new NFmiLevel[GetSize()];

  for (unsigned long i = 0; i < GetSize(); i++)
  {
    file >> itsLevels[i];
  }

  file >> itsStep;

  Reset();
  return file;
}

// ======================================================================
