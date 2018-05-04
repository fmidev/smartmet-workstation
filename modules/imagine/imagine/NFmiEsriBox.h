// ======================================================================
//
// Class to define a bounding box in 2D, plus bounding box for measure
//
// History:
//
// 31.08.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once

#include <newbase/NFmiGlobals.h>

namespace Imagine
{
class _FMI_DLL NFmiEsriBox
{
 public:
  // Constructors, destructors

  NFmiEsriBox(void);

#ifdef NO_COMPILER_OPTIMIZED
  ~NFmiEsriBox(void);
  NFmiEsriBox(const NFmiEsriBox& theBox);
  NFmiEsriBox& operator=(const NFmiEsriBox& theBox);
#endif

  void Init(void);

  // Data access

  bool IsValid(void) const { return itsValidity; }
  double Xmin(void) const { return itsXmin; }
  double Xmax(void) const { return itsXmax; }
  double Ymin(void) const { return itsYmin; }
  double Ymax(void) const { return itsYmax; }
  double Mmin(void) const { return itsMmin; }
  double Mmax(void) const { return itsMmax; }
  double Zmin(void) const { return itsZmin; }
  double Zmax(void) const { return itsZmax; }
  // Update utilities

  void Update(double theX, double theY, double theM, double theZ);
  void Update(double theX, double theY, double theM);
  void Update(double theX, double theY);

  void Update(const NFmiEsriBox& theBox);

 private:
  bool itsValidity;
  double itsXmin;
  double itsXmax;
  double itsYmin;
  double itsYmax;
  double itsMmin;
  double itsMmax;
  double itsZmin;
  double itsZmax;
};

}  // namespace Imagine


// ======================================================================
