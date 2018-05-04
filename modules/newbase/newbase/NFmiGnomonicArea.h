// ======================================================================
/*!
 * \file NFmiGnomonicArea.h
 * \brief Interface of class NFmiGnomonicArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiAzimuthalArea.h"

//! Undocumented
class _FMI_DLL NFmiGnomonicArea : public NFmiAzimuthalArea
{
 public:
  virtual ~NFmiGnomonicArea(void);

  NFmiGnomonicArea();

  NFmiGnomonicArea(const NFmiGnomonicArea& theGnomonicArea);

  NFmiGnomonicArea(const NFmiPoint& theBottomLeftLatLon,
                   const NFmiPoint& theTopRightLatLon,
                   const double theCentralLongitude = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   const double theCentralLatitude = 90.,
                   const double theTrueLatitude = 60.,  // see ref [2]
                   bool usePacificView = false);

  NFmiGnomonicArea(const NFmiPoint& theBottomLeftLatLon,
                   const double theWidthInMeters,
                   const double theHeightInMeters,
                   const double theCentralLongitude = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   const double theCentralLatitude = 90.,
                   const double theTrueLatitude = 60.);  // see ref [2]

  NFmiGnomonicArea(const double theRadialRange,
                   const double theCentralLongitude = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   const double theCentralLatitude = 90.,
                   const double theTrueLatitude = 60.);

  virtual void Init(bool fKeepWorldRect = false);
  virtual NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                            const NFmiPoint& theTopRightLatLon,
                            bool allowPacificFix = true) const;
  virtual NFmiArea* Clone(void) const;

  NFmiGnomonicArea& operator=(const NFmiGnomonicArea& theArea);

  using NFmiAzimuthalArea::operator==;
  using NFmiAzimuthalArea::operator!=;

  bool operator==(const NFmiGnomonicArea& theArea) const;
  bool operator!=(const NFmiGnomonicArea& theArea) const;

  bool operator==(const NFmiArea& theArea) const;
  bool operator!=(const NFmiArea& theArea) const;

  virtual unsigned long ClassId(void) const;
  virtual const char* ClassName(void) const;
  const std::string AreaStr(void) const;
  virtual const std::string WKT() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 protected:
  virtual double K(const double delta) const;
  virtual double CalcDelta(const double xyDistance) const;
  virtual double DistanceFromPerspectivePointToCenterOfEarth(void) const;

 private:
};  // class NFmiGnomonicArea

//! Undocumented, should be removed
typedef NFmiGnomonicArea* PNFmiGnomonicArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiGnomonicArea::~NFmiGnomonicArea(void) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGnomonicArea::ClassId(void) const { return kNFmiGnomonicArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiGnomonicArea::ClassName(void) const { return "kNFmiGnomonicArea"; }

// ======================================================================
