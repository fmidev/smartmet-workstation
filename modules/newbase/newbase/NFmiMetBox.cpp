// ======================================================================
/*!
 * \file NFmiMetBox.cpp
 * \brief Implementation of class NFmiMetBox
 */
// ======================================================================
/*!
 * \class NFmiMetBox
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiMetBox.h"

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theTimeDescriptor Undocumented
 * \param theStationDescriptor Undocumented
 * \param theParamDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetBox::NFmiMetBox(const NFmiTimeBag &theTimeDescriptor,
                       const NFmiLocationBag &theStationDescriptor,
                       const NFmiParamBag &theParamDescriptor)
    : itsTimeDescriptor(new NFmiTimeBag(theTimeDescriptor)),
      itsParamDescriptor(new NFmiParamBag(theParamDescriptor)),
      itsStationDescriptor(new NFmiLocationBag(theStationDescriptor)),
      itsData(nullptr)
{
  itsData = new NFmiBox(CalcSize());
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiMetBox::NFmiMetBox()
    : itsTimeDescriptor(nullptr),
      itsParamDescriptor(nullptr),
      itsStationDescriptor(nullptr),
      itsData(nullptr)
{
  itsData = new NFmiBox;
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiMetBox::~NFmiMetBox()
{
  delete itsTimeDescriptor;
  delete itsStationDescriptor;
  delete itsParamDescriptor;

  delete itsData;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiMetBox::CalcTimeAddition()
{
  return itsParamDescriptor->GetSize() * itsStationDescriptor->GetSize();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiMetBox::CalcStationAddition()
{
  return itsParamDescriptor ? itsParamDescriptor->GetSize() : 0;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
long NFmiMetBox::CalcSize()
{
  return (itsTimeDescriptor->GetSize() * itsParamDescriptor->GetSize() *
          itsStationDescriptor->GetSize());
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeIndex Undocumented
 * \param theStationIndex Undocumented
 * \param theParamIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiMetBox::CalcBoxIndex(unsigned long theTimeIndex,
                              unsigned long theStationIndex,
                              unsigned long theParamIndex)
{
  return (theTimeIndex * CalcTimeAddition() + theStationIndex * CalcStationAddition() +
          theParamIndex);
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \result The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiMetBox::Write(std::ostream &file) const
{
  unsigned short FmiInfoVersionOld = FmiInfoVersion;

  file << "@$\260\243BOX@$\260\243"  // '°' => \260 ja '£' => \243, koska cpp tiedoston character
                                     // set muutettu Utf-8:ksi ja string literalien non-ascii
                                     // koodaus muutettava.
       << " ";
  file << "VER " << FmiBoxVersion << std::endl;

  for (const auto &i : itsHeader)
  {
    file << i;
  }

  FmiInfoVersion = 3;  // Halutaan kirjoittaa Box vanhalla versiolla

  file << *itsTimeDescriptor << *itsParamDescriptor << *itsStationDescriptor << *itsData
       << std::endl;

  FmiInfoVersion = FmiInfoVersionOld;

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

std::istream &NFmiMetBox::Read(std::istream &file)
{
  unsigned short oldVersionNumber = FmiBoxVersion;

  char tmpchars[12];
  file >> tmpchars;

  char space;
  file.get(space);

  char ver;
  file.get(ver);
  if (ver == 'V')
  {
    char tmpchars2[3];
    file >> tmpchars2;
    file >> FmiBoxVersion;  // ..really, this modifies a system wide global? --AKa 3-Jun-10

    for (auto &i : itsHeader)
    {
      file >> i;
    }
  }
  else
  {
    FmiBoxVersion = 1;
    file.putback(ver);
  }

  if (!itsTimeDescriptor) itsTimeDescriptor = new NFmiTimeBag;
  if (!itsParamDescriptor) itsParamDescriptor = new NFmiParamBag;
  if (!itsStationDescriptor) itsStationDescriptor = new NFmiLocationBag;

  file >> *itsTimeDescriptor >> *itsParamDescriptor >> *itsStationDescriptor;

  if (!itsData)
    itsData = new NFmiBox(CalcSize());
  else
    itsData->Initialize(CalcSize());

  file >> *itsData;

  FmiBoxVersion = oldVersionNumber;
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBox Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetBoxIterator::NFmiMetBoxIterator(NFmiMetBox *theBox)
    : itsBox(theBox),
      itsTimeDescriptor(new NFmiTimeBag(theBox->GetTimeDescriptor())),
      itsParamDescriptor(new NFmiParamBag(theBox->GetParamDescriptor())),
      itsStationDescriptor(new NFmiLocationBag(theBox->GetStationDescriptor()))
{
}

// ----------------------------------------------------------------------
/*!
 * \param theBox Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theStationDescriptor Undocumented
 * \param theParamDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetBoxIterator::NFmiMetBoxIterator(NFmiMetBox *theBox,
                                       const NFmiTimeBag &theTimeDescriptor,
                                       const NFmiLocationBag &theStationDescriptor,
                                       const NFmiParamBag &theParamDescriptor)
    : itsBox(theBox),
      itsTimeDescriptor(new NFmiTimeBag(theTimeDescriptor)),
      itsParamDescriptor(new NFmiParamBag(theParamDescriptor)),
      itsStationDescriptor(new NFmiLocationBag(theStationDescriptor))
{
}

// ----------------------------------------------------------------------
/*!
 * \param theIterator Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetBoxIterator::NFmiMetBoxIterator(const NFmiMetBoxIterator &theIterator)
    : itsBox(theIterator.itsBox),
      itsTimeDescriptor(new NFmiTimeBag(*(theIterator.itsTimeDescriptor))),
      itsParamDescriptor(new NFmiParamBag(*(theIterator.itsParamDescriptor))),
      itsStationDescriptor(new NFmiLocationBag(*(theIterator.itsStationDescriptor)))
{
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiMetBoxIterator::~NFmiMetBoxIterator()
{
  delete itsTimeDescriptor;
  delete itsStationDescriptor;
  delete itsParamDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiMetBoxIterator::CalcBoxIndex()
{
  return (static_cast<NFmiMetBox *>(itsBox))
      ->CalcBoxIndex(itsTimeDescriptor->CurrentIndex(),
                     itsStationDescriptor->CurrentIndex(),
                     itsParamDescriptor->CurrentIndex());
}

// ----------------------------------------------------------------------
/*!
 * \param theIterator Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::MapCursorFrom(const NFmiMetBoxIterator &theIterator)
{
  return (itsTimeDescriptor->SetCurrent(theIterator.itsTimeDescriptor->CurrentTime()) &&
          itsStationDescriptor->Location(*(theIterator.itsStationDescriptor)->Location()) &&
          itsParamDescriptor->SetCurrent(theIterator.itsParamDescriptor->CurrentParam()));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiMetBoxIterator::CurrentValue() { return itsBox->Value(CalcBoxIndex()); }
// ----------------------------------------------------------------------
/*!
 * \param theBoxValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::NextTimeValue(float &theBoxValue)
{
  bool isInside = itsTimeDescriptor->Next();
  if (isInside)
  {
    theBoxValue = itsBox->Value(CalcBoxIndex());
  }

  return isInside;
}

// ----------------------------------------------------------------------
/*!
 * \param theBoxValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::PreviousTimeValue(float &theBoxValue)
{
  bool isInside = itsTimeDescriptor->Previous();
  if (isInside)
  {
    theBoxValue = itsBox->Value(CalcBoxIndex());
  }
  return isInside;
}

// ----------------------------------------------------------------------
/*!
 * \param theBoxValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::NextStationValue(float &theBoxValue)
{
  bool isInside = itsStationDescriptor->Next();
  if (isInside)
  {
    theBoxValue = itsBox->Value(CalcBoxIndex());
  }
  return isInside;
}

// ----------------------------------------------------------------------
/*!
 * \param theBoxValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::PreviousStationValue(float &theBoxValue)
{
  bool isInside = itsStationDescriptor->Previous();
  if (isInside)
  {
    theBoxValue = itsBox->Value(CalcBoxIndex());
  }
  return isInside;
}

// ----------------------------------------------------------------------
/*!
 * \param theBoxValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::NextParamValue(float &theBoxValue)
{
  bool isInside = itsParamDescriptor->Next();
  if (isInside)
  {
    theBoxValue = itsBox->Value(CalcBoxIndex());
  }
  return isInside;
}

// ----------------------------------------------------------------------
/*!
 * \param theBoxValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::PreviousParamValue(float &theBoxValue)
{
  bool isInside = itsParamDescriptor->Previous();
  if (isInside)
  {
    theBoxValue = itsBox->Value(CalcBoxIndex());
  }
  return isInside;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::NextTime() { return itsTimeDescriptor->Next(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::PreviousTime() { return itsTimeDescriptor->Previous(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::NextStation() { return itsStationDescriptor->Next(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::PreviousStation() { return itsStationDescriptor->Previous(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::NextParam() { return itsParamDescriptor->Next(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::NextActiveParam() { return itsParamDescriptor->NextActive(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::NextDataParam() { return itsParamDescriptor->NextData(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetBoxIterator::PreviousParam() { return itsParamDescriptor->Previous(); }
// ======================================================================
