// ======================================================================
/*!
 * \file NFmiLatLonArea.h
 * \brief Interface of NFmiLatLonArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiArea.h"

//! Undocumented
class _FMI_DLL NFmiLatLonArea : public NFmiArea
{
 public:
  virtual ~NFmiLatLonArea(void);
  NFmiLatLonArea(void);
  NFmiLatLonArea(const NFmiLatLonArea& theLatLonArea);
  NFmiLatLonArea(const NFmiPoint& theBottomLeftLatLon,
                 const NFmiPoint& theTopRightLatLon,
                 const NFmiPoint& theTopLeftXY = NFmiPoint(0., 0.),
                 const NFmiPoint& theBottomRightXY = NFmiPoint(1., 1.),
                 bool usePacificView = false);

  virtual NFmiArea* Clone(void) const;
  virtual const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const;
  virtual double XScale(void) const;
  virtual double YScale(void) const;
  virtual void Init(bool fKeepWorldRect = false);

  virtual const NFmiRect WorldRect(void) const;
  virtual const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const;
  virtual NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                            const NFmiPoint& theTopRightLatLon,
                            bool allowPacificFix = true) const;
  virtual unsigned long ClassId(void) const;
  virtual const char* ClassName(void) const;
  const std::string AreaStr(void) const;
  virtual const std::string WKT() const;

  virtual bool operator==(const NFmiLatLonArea& theArea) const;
  virtual bool operator==(const NFmiArea& theArea) const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 protected:
  NFmiPoint itsBottomLeftLatLon;
  NFmiPoint itsTopRightLatLon;
  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;

};  // class NFmiLatLonArea

//! Undocumented, should be removed
typedef NFmiLatLonArea* PNFmiLatLonArea;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiLatLonArea::WorldRect(void) const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiLatLonArea::WorldXYToLatLon(const NFmiPoint& theXYPoint) const
{
  return NFmiPoint(180. * theXYPoint.X() / (kPii * kRearth),
                   180.0 * theXYPoint.Y() / (kPii * kRearth));
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiLatLonArea::LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const
{
  return NFmiPoint(kRearth * FmiRad(theLatLonPoint.X()), kRearth * FmiRad(theLatLonPoint.Y()));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiLatLonArea::ClassId(void) const { return kNFmiLatLonArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiLatLonArea::ClassName(void) const { return "NFmiLatLonArea"; }

// ======================================================================
