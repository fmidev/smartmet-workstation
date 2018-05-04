// ======================================================================
/*!
 * \file NFmiParamDataModifierList.cpp
 * \brief Implementation of class NFmiParamDataModifierList
 */
// ======================================================================
/*!
 * \class NFmiParamDataModifierList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiParamDataModifierList.h"
#include "NFmiDataIdent.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiParamDataModifierList::~NFmiParamDataModifierList() { Clear(true); }
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiParamDataModifierList::NFmiParamDataModifierList() : itsList(), itsIter() {}
// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Add(NFmiParamDataModifier* theModifier)
{
  return itsList.AddEnd(theModifier) == true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Reset()
{
  itsIter = itsList.Start();
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Next() { return itsIter.Next() == true; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiParamDataModifier* NFmiParamDataModifierList::Current() { return itsIter.CurrentPtr(); }
// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Remove(bool fDeleteData)
{
  if (itsIter.Remove(fDeleteData)) return true;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented
 */
// ----------------------------------------------------------------------

void NFmiParamDataModifierList::Clear(bool fDeleteData) { itsList.Clear(fDeleteData); }
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Index(unsigned long theIndex)
{
  itsIter = itsList.Index(theIndex);
  if (Current()) return true;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Find(const NFmiDataIdent& theParam)
{
  NFmiDataIdent* paramPtr = nullptr;
  for (Reset(); Next();)
  {
    paramPtr = Current()->Param();
    if (paramPtr && (*paramPtr == theParam)) return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Find(const NFmiDataIdent& theParam, const NFmiLevel* theLevel)
{
  for (Reset(); Next();)
  {
    if (Current()->Match(theParam, theLevel)) return true;
  }
  return false;
}

// ======================================================================
