// ======================================================================
/*!
 * \file NFmiArea.h
 * \brief Interface of class NFmiArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiRect.h"
#include "NFmiSaveBaseFactory.h"
#include <string>

struct PacificPointFixerData
{
  PacificPointFixerData(void) : itsBottomLeftLatlon(), itsTopRightLatlon(), fIsPacific(false) {}
  PacificPointFixerData(const NFmiPoint &theBottomLeftLatlon,
                        const NFmiPoint &theTopRightLatlon,
                        bool isPacific)
      : itsBottomLeftLatlon(theBottomLeftLatlon),
        itsTopRightLatlon(theTopRightLatlon),
        fIsPacific(isPacific)
  {
  }

  NFmiPoint itsBottomLeftLatlon;
  NFmiPoint itsTopRightLatlon;
  bool fIsPacific;
};

//! Undocumented
class _FMI_DLL NFmiArea
{
 public:
  virtual ~NFmiArea();
  NFmiArea();
  NFmiArea(const NFmiArea &theArea);
  NFmiArea(const NFmiPoint &theTopLeftCorner,
           const NFmiPoint &theBottomRightCorner,
           bool usePacificView = false);
  NFmiArea(double theLeft,
           double theTop,
           double theRight,
           double theBottom,
           bool usePacificView = false);

  NFmiArea &operator=(const NFmiArea &theArea);

  bool IsInside(const NFmiPoint &theLatLonPoint) const;
  bool IsInside(const NFmiArea &theArea) const;

  const NFmiPoint TopLeftLatLon() const;
  const NFmiPoint TopRightLatLon() const;
  const NFmiPoint BottomLeftLatLon() const;
  const NFmiPoint BottomRightLatLon() const;
  const NFmiPoint CenterLatLon() const;

  const NFmiPoint TopLeft() const;
  const NFmiPoint BottomRight() const;
  const NFmiPoint TopRight() const;
  const NFmiPoint BottomLeft() const;

  void Place(const NFmiPoint &newPlace);
  void Size(const NFmiPoint &newSize);

  double Top() const;
  double Bottom() const;
  double Left() const;
  double Right() const;
  double Height() const;
  double Width() const;
  const NFmiRect &XYArea() const;

  virtual NFmiArea *Clone() const;
  virtual NFmiArea *CreateNewArea(const NFmiRect &theRect) const;

  virtual void Init(bool fKeepWorldRect = false);
  virtual void SetXYArea(const NFmiRect &newArea);
  virtual const NFmiRect XYArea(const NFmiArea *theArea) const;
  virtual const NFmiPoint ToLatLon(const NFmiPoint &theXYPoint) const = 0;
  virtual const NFmiPoint ToXY(const NFmiPoint &theLatLonPoint) const = 0;
  virtual const NFmiPoint XYToWorldXY(const NFmiPoint &theXYPoint) const = 0;
  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint &theXYPoint) const = 0;
  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const = 0;

  virtual NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                            const NFmiPoint &theTopRightLatLon,
                            bool allowPacificFix = true) const = 0;
  virtual NFmiArea *CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon) const;
  //  virtual NFmiArea * CreateNewArea(const NFmiRect & theRect);
  virtual NFmiArea *CreateNewAreaByWorldRect(const NFmiRect &theWorldRect);
  virtual NFmiArea *CreateNewArea(double theNewAspectRatioXperY,
                                  FmiDirection theFixedPoint,
                                  bool fShrinkArea);

  virtual const NFmiRect WorldRect() const = 0;

  virtual const NFmiPoint WorldXYPlace() const;
  virtual const NFmiPoint WorldXYSize() const;
  virtual double WorldXYWidth() const;
  virtual double WorldXYHeight() const;
  virtual double WorldXYAspectRatio() const;

  virtual const NFmiAngle TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                                           double theLatitudeEpsilon = 0.001) const;

  virtual unsigned long ClassId() const;
  virtual const char *ClassName() const;
  virtual const std::string AreaStr() const = 0;
  virtual const std::string WKT() const = 0;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual bool operator==(const NFmiArea &theArea) const;
  virtual bool operator!=(const NFmiArea &theArea) const;
  bool PacificView(void) const { return fPacificView; }
  void PacificView(bool newValue) { fPacificView = newValue; }
  void CheckForPacificView(void);

  static PacificPointFixerData PacificPointFixer(const NFmiPoint &theBottomLeftLatlon,
                                                 const NFmiPoint &theTopRightLatlon);
  static bool IsPacificView(const NFmiPoint &bottomleftLatlon, const NFmiPoint &toprightLatlon);
  static bool IsPacificLongitude(double theLongitude);
  NFmiArea *DoPossiblePacificFix(void) const;
  NFmiArea *DoForcePacificFix(void) const;

  std::size_t HashValue() const;

 protected:
  int Sign(double theValue) const;
  double FixLongitude(double theLongitude) const;

 private:
  NFmiRect itsXYRectArea;
  bool fPacificView;

};  // class NFmiArea

//! Undocumented, should be removed

typedef NFmiArea *PNFmiArea;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiArea::~NFmiArea() {}
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiArea::NFmiArea() : itsXYRectArea(), fPacificView(false) {}
// ----------------------------------------------------------------------
/*!
 * Constructor based on corner points
 *
 * \param theTopLeftCorner The top left corner coordinates
 * \param theBottomRightCorner The bottom right corner coordinates
 */
// ----------------------------------------------------------------------

inline NFmiArea::NFmiArea(const NFmiPoint &theTopLeftCorner,
                          const NFmiPoint &theBottomRightCorner,
                          bool usePacificView)
    : itsXYRectArea(theTopLeftCorner, theBottomRightCorner), fPacificView(usePacificView)
{
}

// ----------------------------------------------------------------------
/*!
 * Constuctor based on edge coordinates
 *
 * \param theLeft The left edge X-coordinate
 * \param theTop The top edge Y-coordinate
 * \param theRight The right edge X-coordinate
 * \param theBottom The bottom edge Y-coordinate
 */
// ----------------------------------------------------------------------

inline NFmiArea::NFmiArea(
    double theLeft, double theTop, double theRight, double theBottom, bool usePacificView)
    : itsXYRectArea(theLeft, theTop, theRight, theBottom), fPacificView(usePacificView)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiArea::TopLeft() const { return itsXYRectArea.TopLeft(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiArea::BottomRight() const { return itsXYRectArea.BottomRight(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiArea::TopRight() const { return itsXYRectArea.TopRight(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiArea::BottomLeft() const { return itsXYRectArea.BottomLeft(); }
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theArea The object to copy
 */
// ----------------------------------------------------------------------

inline NFmiArea::NFmiArea(const NFmiArea &theArea)
    : itsXYRectArea(theArea.TopLeft(), theArea.BottomRight()), fPacificView(theArea.fPacificView)
{
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theArea The object to copy
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

inline NFmiArea &NFmiArea::operator=(const NFmiArea &theArea)
{
  if (this != &theArea)
  {
    itsXYRectArea = theArea.itsXYRectArea;
    fPacificView = theArea.fPacificView;
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiArea::IsInside(const NFmiPoint &theLatLonPoint) const
{
  NFmiPoint xyPoint = ToXY(theLatLonPoint);
  if (xyPoint == NFmiPoint::gMissingLatlon)
  {
    return false;
  }
  return itsXYRectArea.IsInside(xyPoint);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiArea::TopLeftLatLon() const { return ToLatLon(TopLeft()); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiArea::TopRightLatLon() const { return ToLatLon(TopRight()); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiArea::BottomLeftLatLon() const { return ToLatLon(BottomLeft()); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiArea::BottomRightLatLon() const { return ToLatLon(BottomRight()); }
// ----------------------------------------------------------------------
/*!
 * \param theArea Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiArea::IsInside(const NFmiArea &theArea) const
{
  return IsInside(theArea.TopLeftLatLon()) && IsInside(theArea.TopRightLatLon()) &&
         IsInside(theArea.BottomLeftLatLon()) && IsInside(theArea.BottomRightLatLon());
}

// ----------------------------------------------------------------------
/*!
 * \param newPlace Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiArea::Place(const NFmiPoint &newPlace) { itsXYRectArea.Place(newPlace); }
// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiArea::Size(const NFmiPoint &newSize) { itsXYRectArea.Size(newSize); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiArea::Top() const { return itsXYRectArea.Top(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiArea::Bottom() const { return itsXYRectArea.Bottom(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiArea::Left() const { return itsXYRectArea.Left(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiArea::Right() const { return itsXYRectArea.Right(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiArea::Height() const { return itsXYRectArea.Height(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiArea::Width() const { return itsXYRectArea.Width(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect &NFmiArea::XYArea() const { return itsXYRectArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiArea *NFmiArea::Clone() const { return 0; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiArea::ClassId() const { return kNFmiArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiArea::ClassName() const { return "NFmiArea"; }
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The object to compare against
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

inline bool NFmiArea::operator==(const NFmiArea &theArea) const
{
  return itsXYRectArea == theArea.itsXYRectArea;
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The object to compare against
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

inline bool NFmiArea::operator!=(const NFmiArea &theArea) const
{
  return itsXYRectArea != theArea.itsXYRectArea;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiArea::Sign(double theValue) const { return theValue < 0 ? -1 : 1; }
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiArea
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiArea &ob) { return ob.Write(file); }
// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiArea
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiArea &ob) { return ob.Read(file); }

// ======================================================================
