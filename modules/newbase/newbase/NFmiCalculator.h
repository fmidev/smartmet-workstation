// ======================================================================
/*!
 * \file NFmiCalculator.h
 * \brief Interface of class NFmiCalculator
 */
// ======================================================================

#pragma once

#include "NFmiDataIterator.h"
#include "NFmiDataModifier.h"

//! Undocumented

class _FMI_DLL NFmiCalculator
{
 public:
  virtual ~NFmiCalculator(void);
  NFmiCalculator(NFmiQueryInfo* theData, NFmiDataModifier* theDataModifier);
  NFmiCalculator(NFmiDataIterator* theDataIterator, NFmiDataModifier* theDataModifier);

  virtual double FloatValue(void);
  virtual NFmiCombinedParam* CombinedValue(void);
  float CalculatedValue(void);

  void SetData(NFmiQueryInfo* theData);

  void DataIterator(NFmiDataIterator* theIterator);
  NFmiDataIterator* DataIterator(void);

  void SetDataModifier(NFmiDataModifier* theModifier);
  NFmiDataModifier* DataModifier(void);
  NFmiDataModifier* CalculatedModifier(void);

 private:
  NFmiCalculator(const NFmiCalculator& theCalculator);
  NFmiCalculator& operator=(const NFmiCalculator& theCalculator);

  NFmiDataModifier* itsDataModifier;  // ei omista
  NFmiDataIterator* itsDataIterator;  // ei omista
  NFmiQueryInfo* itsData;             // ei omista

};  // class NFmiCalculator

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculator::SetDataModifier(NFmiDataModifier* theModifier)
{
  itsDataModifier = theModifier;
}

// ----------------------------------------------------------------------
/*!
 * \param theIterator Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculator::DataIterator(NFmiDataIterator* theIterator)
{
  itsDataIterator = theIterator;
}

// ----------------------------------------------------------------------
/*!
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculator::SetData(NFmiQueryInfo* theData) { itsData = theData; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be a const method
 */
// ----------------------------------------------------------------------

inline NFmiDataIterator* NFmiCalculator::DataIterator(void) { return itsDataIterator; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be a const method
 */
// ----------------------------------------------------------------------

inline NFmiDataModifier* NFmiCalculator::DataModifier(void) { return itsDataModifier; }

// ======================================================================
