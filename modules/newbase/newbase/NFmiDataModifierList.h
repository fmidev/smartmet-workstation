// ======================================================================
/*!
 * \file NFmiDataModifierList.h
 * \brief Interface of class NFmiDataModifierList
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiPtrList.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierList : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierList(void);
  // NFmiDataModifier(const NFmiDataModifier & theModier); // compiler generates
  NFmiDataModifierList(NFmiCombinedParam* thePotentialCombinedParam = 0);

  bool Add(NFmiDataModifier* theModifier);
  virtual void Clear(void);

  bool IsCombinedParam(void);

  bool BoolOperation(float);
  virtual float FloatOperation(float theValue);

  using NFmiDataModifier::CalculationResult;
  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::WriteExpressionBody;
  virtual float CalculationResult(void);
  virtual void Calculate(float NotInUse);
  virtual std::ostream& WriteExpressionBody(std::ostream& file);

 public:
  virtual double FloatValue(void);
  bool Reset(void);
  bool Next(void);
  NFmiDataModifier* Current(void);
  bool Remove(bool fDeleteData = false);
  void Clear(bool fDeleteData = false);
  unsigned long NumberOfItems(void);

 protected:
  double itsReturnValue;
  NFmiPtrList<NFmiDataModifier> itsList;
  NFmiPtrList<NFmiDataModifier>::Iterator itsIter;

};  // class NFmiDataModifierList

// ----------------------------------------------------------------------
/*!
 * return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataModifierList::BoolOperation(float) { return false; }
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifierList::FloatOperation(float theValue) { return theValue; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifierList::CalculationResult(void)
{
  return static_cast<float>(itsReturnValue);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifierList::Clear(void) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiDataModifierList::NumberOfItems(void) { return itsList.NumberOfItems(); }

// ======================================================================
