// ======================================================================
/*!
 * \file NFmiSmoother.h
 * \brief Interface of class NFmiSmoother
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"  // for NFmiDataMatrix
#include "NFmiGlobals.h"     // for kFloatMissing
#include "NFmiPoint.h"       // for NFmiPoint
#include <string>

//! Undocumented
class _FMI_DLL NFmiSmoother
{
 public:
  //! Different smoothening methods

  enum NFmiSmootherMethod
  {
    kFmiSmootherMissing,
    kFmiSmootherNone,
    kFmiSmootherNeighbourhood,
    kFmiSmootherPseudoGaussian
  };

  // Constructors

  NFmiSmoother(NFmiSmootherMethod theSmoother, int theFactor = 1, float theRadius = 0.0);
  NFmiSmoother(const std::string& theSmootherName, int theFactor = 1, float theRadius = 0.0);

  static NFmiSmootherMethod SmootherValue(const std::string& theName);
  static const std::string SmootherName(NFmiSmootherMethod theSmoother);

  NFmiSmootherMethod Smoother(void) const;
  int Factor(void) const;
  float Radius(void) const;

  void Smoother(NFmiSmootherMethod smoother);
  void Factor(int factor);
  void Radius(float radius);

  // Smoothen the given data

  const NFmiDataMatrix<float> Smoothen(const NFmiDataMatrix<NFmiPoint>& thePts,
                                       const NFmiDataMatrix<float>& theValues) const;

  const checkedVector<float> Smoothen(const checkedVector<float>& theX,
                                      const checkedVector<float>& theY) const;

  float Weight(float distance) const;

 private:
  // Disable void constructor
  NFmiSmoother(void);

  const NFmiDataMatrix<float> SmoothenKernel(const NFmiDataMatrix<NFmiPoint>& thePts,
                                             const NFmiDataMatrix<float>& theValues) const;

  const checkedVector<float> SmoothenKernel(const checkedVector<float>& theX,
                                            const checkedVector<float>& theY) const;

  // Data members

  NFmiSmootherMethod itsSmoother;
  float itsRadius;
  int itsFactor;

};  // class NFmiSmoother

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiSmoother::NFmiSmootherMethod NFmiSmoother::Smoother(void) const { return itsSmoother; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiSmoother::Factor(void) const { return itsFactor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiSmoother::Radius(void) const { return itsRadius; }
// ----------------------------------------------------------------------
/*!
 * \param smoother Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSmoother::Smoother(NFmiSmootherMethod smoother) { itsSmoother = smoother; }
// ----------------------------------------------------------------------
/*!
 * \param factor Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSmoother::Factor(int factor) { itsFactor = factor; }
// ----------------------------------------------------------------------
/*!
 * \param radius Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSmoother::Radius(float radius) { itsRadius = radius; }
// ----------------------------------------------------------------------
/*!
 * The weighting function. This is public so that external
 * programs may plot the weight functions.
 *
 * \param theDistance The distance for the weight to be calculated
 * \return The calculated weight function
 */
// ----------------------------------------------------------------------

inline float NFmiSmoother::Weight(float theDistance) const
{
  float weight;
  switch (Smoother())
  {
    case kFmiSmootherNeighbourhood:
    {
      float delta = static_cast<float>(3.0 / Radius() * theDistance * Factor());
      weight = static_cast<float>((1.0 / (1.0 + delta * delta)));
      break;
    }
    case kFmiSmootherPseudoGaussian:
    {
      float a = Radius();
      float x = theDistance * Factor();
      weight = (a * a / (a * a + x * x));
      break;
    }
    default:
    {
      weight = (theDistance == 0.0f ? 1.0f : 0.0f);
    }
  }
  return weight;
}

// ======================================================================
