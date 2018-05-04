// ======================================================================
/*!
 * \file NFmiCalculationCondition.h
 * \brief Interface of class NFmiCalculationCondition
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiString.h"

//! Undocumented
class _FMI_DLL NFmiCalculationCondition
{
 public:
  NFmiCalculationCondition(void);
  NFmiCalculationCondition(FmiMaskOperation theCondition,
                           double theLowerLimit,
                           double theUpperLimit = 1);

  void Condition(FmiMaskOperation theCondition);
  double UpperLimit(void) const;
  double LowerLimit(void) const;

  FmiMaskOperation Condition(void) const;
  void UpperLimit(double newValue);
  void LowerLimit(double newValue);

  bool IsMasked(double theValue) const;
  double MaskValue(double theValue) const;
  bool IsRampMask(void) const;
  const NFmiString MaskString(const NFmiString& theParamName) const;

  void Write(std::ostream& os) const;
  void Read(std::istream& is);

 private:
  // Enum tyyppi, määrittelee eri operaatiot kuten: ==, >, <, >= jne.
  FmiMaskOperation itsCondition;
  double itsUpperLimit;
  double itsLowerLimit;

};  // class NFmiCalculationCondition

inline std::ostream& operator<<(std::ostream& os, const NFmiCalculationCondition& item)
{
  item.Write(os);
  return os;
}
inline std::istream& operator>>(std::istream& is, NFmiCalculationCondition& item)
{
  item.Read(is);
  return is;
}

// ----------------------------------------------------------------------
/*!
 * \param theCondition Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculationCondition::Condition(FmiMaskOperation theCondition)
{
  itsCondition = theCondition;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiMaskOperation NFmiCalculationCondition::Condition(void) const { return itsCondition; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiCalculationCondition::UpperLimit(void) const { return itsUpperLimit; }
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculationCondition::UpperLimit(double newValue) { itsUpperLimit = newValue; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiCalculationCondition::LowerLimit(void) const { return itsLowerLimit; }
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculationCondition::LowerLimit(double newValue) { itsLowerLimit = newValue; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiCalculationCondition::IsRampMask(void) const
{
  return (itsCondition == kFmiMaskRisingRamp || itsCondition == kFmiMaskLoweringRamp ||
          itsCondition == kFmiMaskDoubleRamp);
}

// ======================================================================
