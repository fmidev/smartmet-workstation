// ======================================================================
/*!
 * \file NFmiIntegrationSelector.h
 * \brief Interface of class NFmiIntegrationSelector
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierBase.h"

//! Undocumented
enum FmiIntegrationSelector
{
  kFmiNoIntegration,
  kFmiMin,
  kFmiMax,
  kFmiMean,
  kFmiProb
};

//! Undocumented
class _FMI_DLL NFmiIntegrationSelector
{
 public:
  virtual ~NFmiIntegrationSelector(void);
  NFmiIntegrationSelector(FmiIntegrationSelector theSelectorType = kFmiNoIntegration,
                          FmiProbabilityCondition theProbabilityCondition = kNoCondition,
                          float theProbabilityLowerLimit = kFloatMissing,
                          float theProbabilityUpperLimit = kFloatMissing,
                          float theProbabilityScale = 100);

  NFmiIntegrationSelector(const NFmiIntegrationSelector& theSelector);

  FmiIntegrationSelector Type(void) const;
  FmiProbabilityCondition ProbabilityCondition(void) const;
  float ProbabilityLowerLimit(void) const;
  float ProbabilityUpperLimit(void) const;
  float ProbabilityScale(void) const;

  void Type(FmiIntegrationSelector theType);
  void ProbabilityCondition(FmiProbabilityCondition theCondition);
  void ProbabilityLowerLimit(float theLimit);
  void ProbabilityUpperLimit(float theLimit);
  void ProbabilityScale(float theScale);
  void ScaleLowerLimit(float theFactor);
  void ScaleUpperLimit(float theFactor);

 private:
  FmiIntegrationSelector itsType;
  FmiProbabilityCondition itsProbabilityCondition;
  float itsProbabilityLowerLimit;
  float itsProbabilityUpperLimit;
  float itsProbabilityScale;

};  // class NFmiIntegrationSelector

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiIntegrationSelector NFmiIntegrationSelector::Type(void) const { return itsType; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiProbabilityCondition NFmiIntegrationSelector::ProbabilityCondition(void) const
{
  return itsProbabilityCondition;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiIntegrationSelector::ProbabilityLowerLimit(void) const
{
  return itsProbabilityLowerLimit;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiIntegrationSelector::ProbabilityUpperLimit(void) const
{
  return itsProbabilityUpperLimit;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiIntegrationSelector::ProbabilityScale(void) const { return itsProbabilityScale; }
// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::Type(FmiIntegrationSelector theType) { itsType = theType; }
// ----------------------------------------------------------------------
/*!
 * \param theCondition Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ProbabilityCondition(FmiProbabilityCondition theCondition)
{
  itsProbabilityCondition = theCondition;
}

// ----------------------------------------------------------------------
/*!
 * \param theLimit Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ProbabilityLowerLimit(float theLimit)
{
  itsProbabilityLowerLimit = theLimit;
}

// ----------------------------------------------------------------------
/*!
 * \param theLimit Undocumented
 *
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ProbabilityUpperLimit(float theLimit)
{
  itsProbabilityUpperLimit = theLimit;
}

// ----------------------------------------------------------------------
/*!
 * \param theScale Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ProbabilityScale(float theScale)
{
  itsProbabilityScale = theScale;
}

// ----------------------------------------------------------------------
/*!
 * \param theFactor Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ScaleLowerLimit(float theFactor)
{
  if (itsProbabilityLowerLimit != kFloatMissing) itsProbabilityLowerLimit *= theFactor;
}

// ----------------------------------------------------------------------
/*!
 * \param theFactor Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ScaleUpperLimit(float theFactor)
{
  if (itsProbabilityLowerLimit != kFloatMissing) itsProbabilityUpperLimit *= theFactor;
}

// ======================================================================
