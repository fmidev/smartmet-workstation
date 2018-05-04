// ======================================================================
/*!
 * \file NFmiAffine.h
 * \brief Definition of a class to perform affine transformations.
 *
 * \b History:
 *
 * \li 27.11.2001, Mika Heiskanen\par
 * Implemented
 */
// ======================================================================

#pragma once

#include <newbase/NFmiGlobals.h>

namespace Imagine
{
//! A class to perform affine transformations
class _FMI_DLL NFmiAffine
{
 public:
  ~NFmiAffine();
  NFmiAffine();
  NFmiAffine(double theA, double theB, double theC, double theD, double theE, double theF);

  NFmiAffine(const NFmiAffine& theAffine);
  NFmiAffine& operator=(const NFmiAffine& theAffine);

  void Translate(double tx, double ty);
  void Scale(double s);
  void Scale(double sx, double sy);
  void Rotate(double a);
  void Multiply(const NFmiAffine& theAffine);
  double X(double x, double y);
  double Y(double x, double y);

 private:
  double itsA;
  double itsB;
  double itsC;
  double itsD;
  double itsE;
  double itsF;
};

}  // namespace Imagine


// ======================================================================
