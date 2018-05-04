// ======================================================================
/*!
 * \file NFmiParamDataModifierList.h
 * \brief Interface of class NFmiParamDataModifierList
 */
// ======================================================================

#pragma once

#include "NFmiParamDataModifier.h"
#include "NFmiPtrList.h"

class NFmiDataIdent;
class NFmiLevel;

//! Undocumented
class _FMI_DLL NFmiParamDataModifierList
{
 public:
  virtual ~NFmiParamDataModifierList(void);
  NFmiParamDataModifierList(void);
  bool Add(NFmiParamDataModifier* theModifier);
  bool Reset(void);
  bool Next(void);
  NFmiParamDataModifier* Current(void);
  bool Remove(bool fDeleteData = false);
  void Clear(bool fDeleteData = false);
  unsigned long NumberOfItems(void);

  bool Index(unsigned long theIndex);
  bool Find(const NFmiDataIdent& theParam);
  bool Find(const NFmiDataIdent& theParam, const NFmiLevel* theLevel);

 private:
  NFmiPtrList<NFmiParamDataModifier> itsList;  // omistaa/tuhoaa sisällön
  NFmiPtrList<NFmiParamDataModifier>::Iterator itsIter;

};  // class NFmiParamDataModifierList

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be const
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiParamDataModifierList::NumberOfItems(void)
{
  return itsList.NumberOfItems();
}

// ======================================================================
