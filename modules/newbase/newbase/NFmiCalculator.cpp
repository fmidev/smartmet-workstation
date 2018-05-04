// ======================================================================
/*!
 * \file NFmiCalculator.cpp
 * \brief Implementation of class NFmiCalculator
 */
// ======================================================================
/*!
 * \class NFmiCalculator
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiCalculator.h"

// ----------------------------------------------------------------------.
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiCalculator::~NFmiCalculator() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theDataIterator Undocumented
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

NFmiCalculator::NFmiCalculator(NFmiDataIterator* theDataIterator, NFmiDataModifier* theDataModifier)
    : itsDataModifier(theDataModifier), itsDataIterator(theDataIterator), itsData(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theData Undocumented
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

NFmiCalculator::NFmiCalculator(NFmiQueryInfo* theData, NFmiDataModifier* theDataModifier)
    : itsDataModifier(theDataModifier), itsDataIterator(nullptr), itsData(theData)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiCalculator::FloatValue()
{
  if (itsDataIterator && itsDataModifier)
  {
    itsDataIterator->DoForEach(itsDataModifier);
    return itsDataModifier->CalculationResult();
  }
  else if (itsData && itsDataModifier)
  {
    return itsDataModifier->FloatValue();
  }
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam* NFmiCalculator::CombinedValue()
{
  if (itsDataIterator && itsDataModifier)
  {
    itsDataIterator->DoForEach(itsDataModifier);
    return itsDataModifier->CombinedCalculationResult();
  }
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifier* NFmiCalculator::CalculatedModifier()
{
  if (itsDataIterator && itsDataModifier)
  {
    itsDataIterator->DoForEach(itsDataModifier);
    return itsDataModifier;
  }
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Tämä on kuten FloatValue, paitsi että juoksutus tehdään täällä loopissa
// ja laskut tehdään täällä eikä iteratorin DoForEach-metodissa.

float NFmiCalculator::CalculatedValue() { return kFloatMissing; }
// ======================================================================
