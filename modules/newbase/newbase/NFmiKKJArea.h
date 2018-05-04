// ======================================================================
/*!
 * \file NFmiKKJArea.h
 * \brief Interface of class NFmiKKJArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiArea.h"

//! Undocumented
class _FMI_DLL NFmiKKJArea : public NFmiArea
{
 public:
  virtual ~NFmiKKJArea(void);
  NFmiKKJArea(void);
  NFmiKKJArea(const NFmiKKJArea& theKKJArea);

  NFmiKKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopRightLatLon,
              const NFmiPoint& theTopLeftXY,
              const NFmiPoint& theBottomRightXY,
              bool usePacificView = false);

  NFmiKKJArea(double theRadialRangeInMeters,
              const NFmiPoint& theCenterLatLon,
              const NFmiPoint& theTopLeftXY,
              const NFmiPoint& theBottomRightXY);

  NFmiKKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopLeftXY,
              const NFmiPoint& theBottomRightXY);

  NFmiKKJArea(const NFmiPoint& theTopLeftXY, const NFmiPoint& theBottomRightXY);

  virtual void Init(bool fKeepWorldRect = false);
  virtual const NFmiRect WorldRect(void) const;

  virtual const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const;
  virtual const NFmiPoint ToWorldXY(const NFmiPoint& theLatLonPoint) const;
  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const;
  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const;

  NFmiKKJArea& operator=(const NFmiKKJArea& theArea);

  virtual bool operator==(const NFmiKKJArea& theArea) const;
  virtual bool operator!=(const NFmiKKJArea& theArea) const;

  virtual bool operator==(const NFmiArea& theArea) const;
  virtual bool operator!=(const NFmiArea& theArea) const;

  virtual unsigned long ClassId(void) const;
  virtual const char* ClassName(void) const;
  const std::string AreaStr(void) const;
  virtual const std::string WKT() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  void SetupGaussKruger(void);
  void SetupAnyEllipsoid(double theA, double theP);

 protected:
  NFmiPoint itsTopRightLatLon;
  NFmiPoint itsBottomLeftLatLon;

  virtual int ZoneNumberByLongitude(const double lon) const = 0;
  virtual int ZoneNumberByEasting(const double easting) const = 0;

  double Rad(double x) const;
  double Deg(double x) const;
  double Arsinh(double x) const;
  double Artanh(double x) const;

  double a, p, b, e, dn, dn2, dn3, dn4, a1, h1[4], h2[4];

  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;

 private:
};  // class NFmiKKJArea

//! Undocumented, should be removed
typedef NFmiKKJArea* PNFmiKKJArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special.
 */
// ----------------------------------------------------------------------

inline NFmiKKJArea::~NFmiKKJArea(void) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiKKJArea::WorldRect(void) const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiKKJArea::ClassId(void) const { return kNFmiKKJArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiKKJArea::ClassName(void) const { return "NFmiKKJArea"; }
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiKKJArea::Rad(double x) const { return x * kPii / 180.0; }
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiKKJArea::Deg(double x) const { return x * 180.0 / kPii; }
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiKKJArea::Arsinh(double x) const
{
  using namespace std;
  return log(x + sqrt(1.0 + x * x));
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiKKJArea::Artanh(double x) const
{
  using namespace std;
  return 0.5 * log((1.0 + x) / (1.0 - x));
}

// ======================================================================
