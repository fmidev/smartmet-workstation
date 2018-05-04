// ======================================================================
/*!
 * \file NFmiDataIdent.cpp
 * \brief Implementation of class NFmiDataIdent
 */
// ======================================================================
/*!
 * \class NFmiDataIdent
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataIdent.h"
#include "NFmiParamBag.h"
#include "NFmiVoidPtrList.h"
#include <fstream>

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theProducer Undocumented
 * \param theType Undocumented
 * \param isGroup Undocumented
 * \param isActive Undocumented
 * \param containsIndividualParams Undocumented
 * \param isDataParam Undocumented
 * \param hasDataParam Undocumented
 * \param theSubParamBag Undocumented
 * \param theSecondaryProducerList Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent::NFmiDataIdent(const NFmiParam &theParam,
                             const NFmiProducer &theProducer,
                             FmiParamType theType,
                             bool isGroup,
                             bool isActive,
                             bool containsIndividualParams,
                             bool isDataParam,
                             bool hasDataParam,
                             NFmiParamBag *theSubParamBag,
                             NFmiVoidPtrList *theSecondaryProducerList)
    : itsParam(theParam.Clone()),
      itsProducer(new NFmiProducer(theProducer)),
      itsType(theType),
      fIsActive(isActive),
      fIsGroup(isGroup),
      fContainsIndividualParams(containsIndividualParams),
      fIsDataParam(isDataParam),
      fHasDataParams(hasDataParam),
      itsDataParams(theSubParamBag ? new NFmiParamBag(*theSubParamBag) : nullptr),
      itsSecondaryProducers(nullptr),
      itsSecondaryProducerIterator(nullptr),
      itsCurrentSecondaryProducer(nullptr)
{
  if (theSecondaryProducerList && theSecondaryProducerList->NumberOfItems() > 0)
  {
    itsSecondaryProducers = new NFmiVoidPtrList;
    NFmiVoidPtrIterator it(theSecondaryProducerList);
    void *vPt;
    while (it.Next(vPt))
      itsSecondaryProducers->AddEnd(
          static_cast<void *>(new NFmiProducer(*static_cast<NFmiProducer *>(vPt))));
    itsSecondaryProducerIterator = new NFmiVoidPtrIterator(itsSecondaryProducers);
    itsCurrentSecondaryProducer = new NFmiVoidPtrData(itsSecondaryProducerIterator->Current());
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theDataIdent Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent::NFmiDataIdent(const NFmiDataIdent &theDataIdent)
    : itsParam(theDataIdent.itsParam->Clone()),
      itsProducer(new NFmiProducer(*(theDataIdent.itsProducer))),
      itsType(theDataIdent.itsType),
      fIsActive(theDataIdent.fIsActive),
      fIsGroup(theDataIdent.fIsGroup),
      fContainsIndividualParams(theDataIdent.fContainsIndividualParams),
      fIsDataParam(theDataIdent.fIsDataParam),
      fHasDataParams(theDataIdent.fHasDataParams),
      itsDataParams(theDataIdent.itsDataParams ? new NFmiParamBag(*(theDataIdent.itsDataParams))
                                               : nullptr),
      itsSecondaryProducers(nullptr),
      itsSecondaryProducerIterator(nullptr),
      itsCurrentSecondaryProducer(nullptr)
{
  if (theDataIdent.itsSecondaryProducers && theDataIdent.itsSecondaryProducers->NumberOfItems() > 0)
  {
    itsSecondaryProducers = new NFmiVoidPtrList;
    NFmiVoidPtrIterator it(theDataIdent.itsSecondaryProducers);
    void *vPt;
    while (it.Next(vPt))
      itsSecondaryProducers->AddEnd(
          static_cast<void *>(new NFmiProducer(*static_cast<NFmiProducer *>(vPt))));
    itsSecondaryProducerIterator = new NFmiVoidPtrIterator(itsSecondaryProducers);
    itsCurrentSecondaryProducer = new NFmiVoidPtrData(itsSecondaryProducerIterator->Current());
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataIdent::Destroy()
{
  delete itsParam;
  delete itsProducer;
  delete itsDataParams;
  if (itsSecondaryProducers)
  {
    NFmiVoidPtrIterator iter(itsSecondaryProducers);
    void *vPt;  // iter.Reset() ??
    while (iter.Next(vPt))
      delete (static_cast<NFmiProducer *>(vPt));
    delete itsSecondaryProducers;
  }
  delete itsSecondaryProducerIterator;
  delete itsCurrentSecondaryProducer;
}

// ----------------------------------------------------------------------
/*!
 * Etsii aliparametrilistasta annettua parametria ja palauttaa true jos löytyi
 *
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataIdent::IsDataParam(const FmiParameterName &theParam)
{
  if (itsDataParams)
  {
    for (ResetDataParams(); NextDataParam();)
    {
      if ((FmiParameterName(CurrentDataParam().GetParam()->GetIdent())) == theParam) return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Etsii aliparametrilistasta annettua parametria ja palauttaa true jos löytyi
 *
 * \param theDataIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataIdent::IsDataParam(const NFmiDataIdent &theDataIdent)
{
  if (itsDataParams)
  {
    for (ResetDataParams(); NextDataParam();)
    {
      if (CurrentDataParam() == theDataIdent) return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theProducer Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIdent::SetProducers(const NFmiProducer &theProducer)
{
  *itsProducer = theProducer;
  if (fHasDataParams)
  {
    for (ResetDataParams(); NextDataParam();)
    {
      CurrentDataParam().SetProducers(theProducer);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theDataIdent The other object being copied
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiDataIdent::operator=(const NFmiDataIdent &theDataIdent)
{
  Destroy();
  itsParam = theDataIdent.itsParam->Clone();
  itsProducer = new NFmiProducer(*(theDataIdent.itsProducer));
  itsType = theDataIdent.itsType;
  fIsGroup = theDataIdent.fIsGroup;
  fIsActive = theDataIdent.fIsActive;
  fContainsIndividualParams = theDataIdent.fContainsIndividualParams;
  fIsDataParam = theDataIdent.fIsDataParam;
  fHasDataParams = theDataIdent.fHasDataParams;
  itsDataParams =
      theDataIdent.itsDataParams ? new NFmiParamBag(*(theDataIdent.itsDataParams)) : nullptr;
  if (theDataIdent.itsSecondaryProducers && theDataIdent.itsSecondaryProducers->NumberOfItems() > 0)
  {
    itsSecondaryProducers = new NFmiVoidPtrList;
    NFmiVoidPtrIterator it(theDataIdent.itsSecondaryProducers);
    void *vPt;
    while (it.Next(vPt))
      itsSecondaryProducers->AddEnd(
          static_cast<void *>(new NFmiProducer(*static_cast<NFmiProducer *>(vPt))));
    itsSecondaryProducerIterator = new NFmiVoidPtrIterator(itsSecondaryProducers);
    itsCurrentSecondaryProducer = new NFmiVoidPtrData(itsSecondaryProducerIterator->Current());
  }
  else  // 24.2.1999/Marko Lisäsin else haaran, koska muuten kaatuu jos on ensin ollut
        // dataparamsseja ja sitten sijoitetaan identti missä ei ole dataparamsseja
  {
    itsSecondaryProducers = nullptr;
    itsSecondaryProducerIterator = nullptr;
    itsCurrentSecondaryProducer = nullptr;
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataIdent::Write(std::ostream &file) const
{
  file << (*itsParam);
  file << (*itsProducer);
  file << itsType << " ";
  file << fIsGroup << " ";
  file << fIsActive << " ";
  file << fContainsIndividualParams << " ";
  file << fIsDataParam << " ";
  file << fHasDataParams << " ";

  if (FmiInfoVersion >= 4)
  {
    file << "0 ";               // Varattu
    file << "0 " << std::endl;  // Varattu
    if (fHasDataParams)
    {
      file << *itsDataParams;
    }

    if (itsSecondaryProducers)
    {
      file << itsSecondaryProducers->NumberOfItems() << std::endl;

      NFmiVoidPtrIterator theProdItem(*itsSecondaryProducers);
      void *theVoid;
      while (theProdItem.Next(theVoid))
        file << *(static_cast<NFmiProducer *>(theVoid));
    }
    else
      file << 0 << std::endl;
  }
  else
  {
    file << std::endl;
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

std::istream &NFmiDataIdent::Read(std::istream &file)
{
  unsigned long fReserve;

  Destroy();

  itsParam = new NFmiParam;
  itsProducer = new NFmiProducer;

  file >> *itsParam;
  file >> *itsProducer;
  file >> itsType;
  file >> fIsGroup;
  file >> fIsActive;
  file >> fContainsIndividualParams;
  file >> fIsDataParam;
  file >> fHasDataParams;

  if (FmiInfoVersion >= 4)
  {
    file >> fReserve;
    file >> fReserve;
    if (fHasDataParams)
    {
      delete itsDataParams;
      itsDataParams = new NFmiParamBag;
      file >> *itsDataParams;
    }

    long theNumberOfItems;
    file >> theNumberOfItems;

    if (theNumberOfItems)
    {
      itsSecondaryProducers = new NFmiVoidPtrList;

      NFmiProducer theProducer;
      for (int i = 0; i < theNumberOfItems; i++)
      {
        file >> theProducer;
        itsSecondaryProducers->AddEnd(static_cast<void *>(new NFmiProducer(theProducer)));
      }
      itsSecondaryProducerIterator = new NFmiVoidPtrIterator(itsSecondaryProducers);
      itsCurrentSecondaryProducer = new NFmiVoidPtrData(itsSecondaryProducerIterator->Current());
    }
  }
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// seuraavia metodeja ei saa inline:ksi, koska NFmiDataIdent ja NFmiParamBag
// eivät voi include:oida toisiaan ristiin ja NFmiParambag includoi jo NFmiDataIdent:in

const NFmiDataIdent &NFmiDataIdent::FirstDataParam() const { return *itsDataParams->GetFirst(); }

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataIdent::ResetDataParams() { itsDataParams->Reset(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiDataIdent::CurrentDataParam() { return *(itsDataParams->Current()); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataIdent::NextDataParam() { return itsDataParams->Next(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// HUOM!! ei mene yli viimeisen vaan menee viimeiseen

bool NFmiDataIdent::ResetLastDataParams()
{
  return itsDataParams->SetCurrentIndex(itsDataParams->GetSize() - 1);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataIdent::PreviousDataParam() { return itsDataParams->Previous(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataIdent::NextActiveDataParam() { return itsDataParams->NextActive(); }
// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param isActive Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIdent::SetActiveDataParam(const NFmiParam &theParam, bool isActive)
{
  itsDataParams->SetActive(theParam, isActive);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiDataIdent::CurrentActiveDataParam() { return *(itsDataParams->Current()); }
// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIdent::SetParam(const NFmiParam &theParam)
{
  if (itsParam) delete itsParam;
  itsParam = theParam.Clone();
  fIsDataParam = true;
}

// ----------------------------------------------------------------------
/*!
 * \param theProducer Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIdent::SetProducer(const NFmiProducer &theProducer)
{
  if (itsProducer) delete itsProducer;
  itsProducer = new NFmiProducer(theProducer);
  if (itsDataParams) itsDataParams->SetProducer(theProducer);
}

// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIdent::SetIncrementalType(bool newState)
{
  if (newState)
  {
    if (!IsIncremental()) itsType += kIncrementalParam;
  }
  else
  {
    if (IsIncremental()) itsType -= kIncrementalParam;
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataIdent::ResetSecondaryProducer() { itsSecondaryProducerIterator->Reset(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiProducer *NFmiDataIdent::CurrentSecondaryProducer() const
{
  return static_cast<NFmiProducer *>(itsCurrentSecondaryProducer->GetVoidPtr());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataIdent::NextSecondaryProducer()
{
  return itsSecondaryProducerIterator->NextPtr(itsCurrentSecondaryProducer);
}

// ======================================================================
