// ======================================================================
/*!
 * \file NFmiVPlaceDescriptor.cpp
 * \brief Implementation of class NFmiVPlaceDescriptor
 */
// ----------------------------------------------------------------------

#include "NFmiVPlaceDescriptor.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor::~NFmiVPlaceDescriptor() { Destroy(); }
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor::NFmiVPlaceDescriptor() : itsLevelBag(nullptr), itsActivity(nullptr) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theLevelBag Undocumented
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor::NFmiVPlaceDescriptor(NFmiLevelBag &theLevelBag)
    : itsLevelBag(new NFmiLevelBag(theLevelBag)), itsActivity(nullptr)
{
  itsActivity = new bool[static_cast<int>(itsLevelBag->GetSize())];
  for (int i = 0; i < static_cast<int>(itsLevelBag->GetSize()); i++)
    itsActivity[i] = true;
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theVPlaceDescriptor The other object being copied
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor::NFmiVPlaceDescriptor(const NFmiVPlaceDescriptor &theVPlaceDescriptor)
    : NFmiDataDescriptor(),
      itsLevelBag(theVPlaceDescriptor.itsLevelBag
                      ? new NFmiLevelBag(*theVPlaceDescriptor.itsLevelBag)
                      : nullptr),
      itsActivity(nullptr)
{
  if (itsLevelBag)
  {
    itsActivity = new bool[static_cast<int>(itsLevelBag->GetSize())];
    for (int i = 0; i < static_cast<int>(itsLevelBag->GetSize()); i++)
    {
      itsActivity[i] = true;
    }
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiVPlaceDescriptor::Destroy()
{
  if (itsActivity)
  {
    delete[] static_cast<bool *>(itsActivity);
    itsActivity = nullptr;
  }
  if (itsLevelBag)
  {
    delete itsLevelBag;
    itsLevelBag = nullptr;
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::Next()
{
  if (itsLevelBag)
    return itsLevelBag->Next();
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::Previous()  // 15.1.1997/Marko
{
  if (itsLevelBag)
    return itsLevelBag->Previous();
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevel *NFmiVPlaceDescriptor::Level() const
{
  if (itsLevelBag)
    return itsLevelBag->Level();
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevel *NFmiVPlaceDescriptor::Level(unsigned long theIndex) const
{
  if (itsLevelBag)
    return itsLevelBag->Level(theIndex);
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevel *NFmiVPlaceDescriptor::LevelMinValue() const
{
  if (itsLevelBag)
    return itsLevelBag->LevelMinValue();
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevel *NFmiVPlaceDescriptor::LevelMaxValue() const
{
  if (itsLevelBag)
    return itsLevelBag->LevelMaxValue();
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \param theLevel Undocumented
 * \return Undocumented
 * \todo Siirrä koodi NFmiLevelBag:iin!
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::Level(const NFmiLevel &theLevel)
{
  bool tempBoolean;
  itsLevelBag->Reset();

  do
  {
    tempBoolean = itsLevelBag->Next();
    if (!tempBoolean) break;
  } while (!(*itsLevelBag->Level() == theLevel));

  if (tempBoolean)
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theLevelBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVPlaceDescriptor::LevelBag(NFmiLevelBag &theLevelBag)
{
  if (itsLevelBag) Destroy();
  itsLevelBag = new NFmiLevelBag(theLevelBag);

  itsActivity = new bool[Size()];
  for (int i = 0; i < static_cast<int>(Size()); i++)
    itsActivity[i] = true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiVPlaceDescriptor::Index() const
{
  if (itsLevelBag)
    return itsLevelBag->CurrentIndex();
  else
    return static_cast<unsigned long>(-1);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiVPlaceDescriptor::Size() const
{
  if (itsLevelBag)
    return itsLevelBag->GetSize();
  else
    return static_cast<unsigned long>(0);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::IsLevel() const
{
  if (itsLevelBag)
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::NextActive()
{
  while (Next())
    if (IsActive()) return true;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::SetActivity(bool theActivityState)
{
  bool temp = itsActivity[Index()];
  itsActivity[Index()] = theActivityState;
  return temp;
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 * \param theVPlaceDescriptor The other object being copied
 * \return The object assigned to
 * \bug Should protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor &NFmiVPlaceDescriptor::operator=(
    const NFmiVPlaceDescriptor &theVPlaceDescriptor)
{
  Destroy();

  itsLevelBag = theVPlaceDescriptor.itsLevelBag ? new NFmiLevelBag(*theVPlaceDescriptor.itsLevelBag)
                                                : nullptr;

  if (itsLevelBag)
  {
    itsActivity = new bool[itsLevelBag->GetSize()];
    for (int i = 0; i < static_cast<int>(itsLevelBag->GetSize()); i++)
      itsActivity[i] = theVPlaceDescriptor.itsActivity[i];
  }

  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theVPlaceDescriptor The other object being compared to
 * \return True, if the objects are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::operator==(const NFmiVPlaceDescriptor &theVPlaceDescriptor) const
{
  bool retVal = false;
  if (this->itsLevelBag && theVPlaceDescriptor.itsLevelBag)
    if (*(this->itsLevelBag) == *(theVPlaceDescriptor.itsLevelBag)) retVal = true;
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

std::ostream &NFmiVPlaceDescriptor::Write(std::ostream &file) const
{
  if (itsLevelBag)
    file << *itsLevelBag;
  else
  {
    // Huom: Unixissa suora tulostus hankalaa, pitää castata objekti
    //       constiksi jne, hankalampaa lukea.
    NFmiLevelBag tmp;
    file << tmp;
  }
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

std::istream &NFmiVPlaceDescriptor::Read(std::istream &file)
{
  Destroy();

  itsLevelBag = new NFmiLevelBag;
  file >> *itsLevelBag;

  itsActivity = new bool[static_cast<int>(itsLevelBag->GetSize())];
  for (int i = 0; i < static_cast<int>(itsLevelBag->GetSize()); i++)
    itsActivity[i] = true;

  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::Index(unsigned long theIndex)
{
  return itsLevelBag->SetCurrentIndex(theIndex);
}

// ======================================================================
