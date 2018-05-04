// ======================================================================
/*!
 * \file NFmiParamDescriptor.cpp
 * \brief Implementation of class NFmiParamDescriptor
 */
// ======================================================================
/*!
 * \class NFmiParamDescriptor
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiParamDescriptor.h"
#include "NFmiDataDescriptor.h"
#include "NFmiParamBag.h"
#include <cassert>

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiParamDescriptor::NFmiParamDescriptor()
    : itsParamBag(nullptr), itsActivity(nullptr), fInterpolate(false)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParamBag Undocumented
 * \param interpolate Undocumented
 */
// ----------------------------------------------------------------------

NFmiParamDescriptor::NFmiParamDescriptor(const NFmiParamBag &theParamBag, bool interpolate)
    : itsParamBag(new NFmiParamBag(theParamBag)), itsActivity(nullptr), fInterpolate(interpolate)
{
  itsActivity = new bool[static_cast<int>(itsParamBag->GetSize())];
  for (int i = 0; i < static_cast<int>(itsParamBag->GetSize()); i++)
    itsActivity[i] = true;
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theParamDescriptor The other object being copied
 */
// ----------------------------------------------------------------------

NFmiParamDescriptor::NFmiParamDescriptor(const NFmiParamDescriptor &theParamDescriptor)
    : NFmiDataDescriptor(),
      itsParamBag(theParamDescriptor.itsParamBag
                      ? new NFmiParamBag(*(theParamDescriptor.itsParamBag))
                      : nullptr),
      itsActivity(nullptr),
      fInterpolate(theParamDescriptor.fInterpolate)
{
  itsActivity = new bool[static_cast<int>(itsParamBag->GetSize())];  // 5.3.1997/Marko
  for (int i = 0; i < static_cast<int>(itsParamBag->GetSize()); i++)
    itsActivity[i] = theParamDescriptor.itsActivity[i];
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiParamDescriptor::Destroy()
{
  if (itsActivity != nullptr)
  {
    delete[] static_cast<bool *>(itsActivity);
    itsActivity = nullptr;
  }
  if (itsParamBag != nullptr)
  {
    delete itsParamBag;
    itsParamBag = nullptr;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent The new parameter
 * \return True on success
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::Param(const NFmiDataIdent &theDataIdent)
{
  return itsParamBag->Current(theDataIdent, true);
}

// ----------------------------------------------------------------------
/*!
 * \param theParam The new parameter
 * \return True on success
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::Param(const NFmiParam &theParam)
{
  return itsParamBag->Current(theParam, true);
}

// ----------------------------------------------------------------------
/*!
 * \param theParam The new parameter
 * \return True on success
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::Param(FmiParameterName theParam)
{
  return itsParamBag->SetCurrent(theParam, false);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \bug Must fix this to use the respective method in NFmiParamBag
 */
// ----------------------------------------------------------------------

// TÄMÄ PITÄÄ KORJATA KÄYTTÄMÄÄN PARAMBAGIN VASTAAVAA METODIA?!?!?!

unsigned long NFmiParamDescriptor::SizeActive() const
{
  unsigned long theActiveSize = 0;

  for (unsigned long i = 0; i < itsParamBag->GetSize(); i++)
    if (itsActivity[i] == true) theActiveSize++;

  return theActiveSize;
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::SetActivity(bool theActivityState, bool fIgnoreSubParam)
{
  // ei käytetä enää parDesc:in aktiviteetti taulua, vaan antaa parambagin hoitaa homma
  return itsParamBag->SetCurrentActive(theActivityState == true, fIgnoreSubParam == true);
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::SetActivity(bool theActivityState)
{
  // ei käytetä enää parDesc:in aktiviteetti taulua, vaan antaa parambagin hoitaa homma
  return itsParamBag->SetCurrentActive(theActivityState == true);
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \param theIndex Undocumented
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::SetActivity(bool theActivityState,
                                      unsigned long theIndex,
                                      bool fIgnoreSubParam)
{
  return itsParamBag->SetActive(theIndex, theActivityState, fIgnoreSubParam == true);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::NextActive()
{
  while (Next())
    if (IsActive()) return true;

  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::NextActive(bool fIgnoreSubParam)
{
  while (Next(fIgnoreSubParam))
    if (IsActive()) return true;

  return false;
}
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theParamDescriptor The other object being copied
 * \result The object assigned to
 * \bug Must protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiParamDescriptor &NFmiParamDescriptor::operator=(const NFmiParamDescriptor &theParamDescriptor)
{
  Destroy();

  itsParamBag =
      theParamDescriptor.itsParamBag ? new NFmiParamBag(*theParamDescriptor.itsParamBag) : nullptr;

  if (itsParamBag)
  {
    itsActivity = new bool[itsParamBag->GetSize()];
    for (int i = 0; i < static_cast<int>(itsParamBag->GetSize()); i++)
      itsActivity[i] = theParamDescriptor.itsActivity[i];
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theParamDescriptor The object being compared to
 * \result True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::operator==(const NFmiParamDescriptor &theParamDescriptor) const
{
  bool retVal = false;
  if (this->Size() == theParamDescriptor.Size())
  {
    for (int i = 0; i < static_cast<int>(Size()); i++)
      if (!(this->itsActivity[i] == theParamDescriptor.itsActivity[i])) return false;
  }
  if (this->itsParamBag && theParamDescriptor.itsParamBag)
  {
    return ((this->fInterpolate == theParamDescriptor.fInterpolate) &&
            (*(this->itsParamBag) == *(theParamDescriptor.itsParamBag)));
  }
  return retVal;
}
// ----------------------------------------------------------------------
/*!
 * \param theCombine Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

const NFmiParamDescriptor NFmiParamDescriptor::Combine(const NFmiParamDescriptor &theCombine)
{
  if (itsParamBag && theCombine.itsParamBag)
    return NFmiParamDescriptor(itsParamBag->Combine(*(theCombine).itsParamBag));
  else if (itsParamBag)
    return NFmiParamDescriptor(*itsParamBag);
  else if (theCombine.itsParamBag)
    return NFmiParamDescriptor(*theCombine.itsParamBag);
  else
    return NFmiParamDescriptor();
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \result The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiParamDescriptor::Write(std::ostream &file) const
{
  file << ClassId() << " " << ClassName() << std::endl;

  file << fInterpolate << " "
       << "0 "
       << "0 "
       << "0 " << std::endl;  // Varalla tulevaisuuta varten

  file << *itsParamBag;

  for (unsigned long i = 0; i < itsParamBag->GetSize(); i++)
    file << itsActivity[i] << " ";

  file << std::endl;

  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \result The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiParamDescriptor::Read(std::istream &file)
{
  Destroy();

  unsigned long classIdent;
  char dirty[30];
  file >> classIdent >> dirty;

  unsigned long theReserve;
  unsigned long theInterpolate;
  file >> theInterpolate >> theReserve >> theReserve >> theReserve;
  fInterpolate = theInterpolate != 0;

  itsParamBag = new NFmiParamBag;

  file >> *itsParamBag;

  itsActivity = new bool[static_cast<int>(itsParamBag->GetSize())];
  itsParamBag->Reset();
  for (unsigned long i = 0; i < itsParamBag->GetSize(); i++)
  {
    if (FmiInfoVersion >= 3)
      file >> itsActivity[i];
    else
    {
      itsParamBag->Next();
      itsActivity[i] = itsParamBag->Current()->IsDataParam();
    }
  }

  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param fIgnoreSubParam Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiParamDescriptor::Param(unsigned long theIndex, bool fIgnoreSubParam) const
{
  // 18.8.2000/Marko lisäsi tarkastuksia.
  // Paluuarvo on ongelmallinen, koska parambagi voi palauttaa 0 pointterin.
  // ParamDescriptorinkin pitäisi kai palauttaa pointteri?
  // Laitoin assertin jolloin ohjelma pysähtyy debug moodissa mutta ei release moodissa
  // jos 0-pointteri.

  NFmiDataIdent *param = itsParamBag->Param(theIndex, fIgnoreSubParam == true);

  // 28.12.2001/Marko Poistin assertin ja palauttaa nyt virhetilanteessa dummy-dataidentin.
  // assert(param); // korjaa ohjelmaasi jos se pysähtyy tähän, tämä on vakava virhe (theIndex on
  // pielessä)

  if (param)
    return *param;
  else
  {
    // 28.12.2001/Marko Tämä on hätäviritys 'virhetilanteeseen',
    // jolloin palautetaan 1. parametrin tuottaja
    return *(itsParamBag->Param(0, fIgnoreSubParam == true));
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param fIgnoreSubParam Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiParamDescriptor::EditParam(unsigned long theIndex, bool fIgnoreSubParam)
{
  // 18.8.2000/Marko lisäsi tarkastuksia.
  // Paluuarvo on ongelmallinen, koska parambagi voi palauttaa 0 pointterin.
  // ParamDescriptorinkin pitäisi kai palauttaa pointteri?
  // Laitoin assertin jolloin ohjelma pysähtyy debug moodissa mutta ei release moodissa
  // jos 0-pointteri.

  NFmiDataIdent *param = itsParamBag->Param(theIndex, fIgnoreSubParam == true);

  // 28.12.2001/Marko Poistin assertin ja palauttaa nyt virhetilanteessa dummy-dataidentin.
  // assert(param); // korjaa ohjelmaasi jos se pysähtyy tähän, tämä on vakava virhe (theIndex on
  // pielessä)

  if (param)
    return *param;
  else
  {
    // 28.12.2001/Marko Tämä on hätäviritys virhetilanteeseen.
    static NFmiParam dummyParam(kFmiBadParameter, "virheparametri, korjaa koodiasi");
    static NFmiDataIdent dummy(dummyParam);
    return dummy;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::Index(unsigned long theIndex)
{
  return itsParamBag->SetCurrentIndex(theIndex);
}

// ----------------------------------------------------------------------
/*!
 * Katsoo, löytyykö minkään parametrin aliparametreistä annettua parametriä.
 * Jos löytyy palauttaa true.
 *
 * \param theParam Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::FindSubParam(const NFmiParam &theParam)
{
  return itsParamBag->FindSubParam(theParam);
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::FindSubParam(const NFmiDataIdent &theDataIdent)
{
  return itsParamBag->FindSubParam(theDataIdent);
}

// ======================================================================
