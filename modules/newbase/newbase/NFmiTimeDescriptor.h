// ======================================================================
/*!
 * \file NFmiTimeDescriptor.h
 * \brief Interface of class NFmiTimeDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiGlobals.h"
#include "NFmiMetTime.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiTimeBag.h"

class NFmiTimeList;

//! Undocumented unnamed enum
enum
{
  kFmiLocalTime,
  kFmiUTCTime
};

//! Undocumented
class _FMI_DLL NFmiTimeDescriptor : public NFmiDataDescriptor
{
 public:
  virtual ~NFmiTimeDescriptor(void);

  NFmiTimeDescriptor(void);

  NFmiTimeDescriptor(const NFmiTimeDescriptor &theTimeDescriptor);

  NFmiTimeDescriptor(const NFmiMetTime &theOriginTime,
                     const NFmiTimeBag &theValidTimeBag,
                     FmiTimeLocalzation theIsLocalTime = kUTC,
                     bool theIsInterpolation = false);

  NFmiTimeDescriptor(const NFmiMetTime &theOriginTime,
                     const NFmiTimeList &theTimeList,
                     FmiTimeLocalzation theIsLocalTime = kUTC,
                     bool theIsInterpolation = false);

  NFmiTimeDescriptor(const NFmiTimeBag &theOriginTimeBag, const NFmiMetTime &theValidTime);

  NFmiTimeDescriptor(const NFmiTimeBag &theOriginTimeBag, unsigned long theForecastPeriod);

  NFmiTimeDescriptor(const NFmiMetTime &theOriginTimeBag,
                     unsigned long theForecastPeriodMin,
                     unsigned long theForecastPeriodMax);

  // -----------------------------------------------------------------
  // Following methods operate in the list part of the TimeDescriptor;
  // it can be either OriginTimeBag, or ValidTimeBag, depending on how
  // the TimeDescriptor was constructed
  // -----------------------------------------------------------------

  virtual bool Next(void);
  virtual bool Previous(void);
  virtual void Reset(void);
  void First(void)
  {
    Reset();
    Next();
  };

  const NFmiMetTime &Time(void) const;
  bool Time(const NFmiMetTime &theTime);
  bool TimeToNearestStep(const NFmiMetTime &theTime,
                         FmiDirection theDirection = kCenter,
                         unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);
  bool Time(const unsigned long &theIndex);
  const NFmiMetTime &FirstTime(void) const;
  const NFmiMetTime &LastTime(void) const;

  // -------------------------------------------------
  // ForecastPeriod(): returns current forecast length
  // -------------------------------------------------

  unsigned long ForecastPeriod(void) const;

  // ------------------------------------------------------------------------------------
  // If OriginTime (or ValidTime) is a list, method OriginTime() (or ValidTime() ) is
  // equivalent with Time(); it returns the current list item.
  // If OriginTime (or ValidTime) is'nt a list, it always returns the constant time
  // ------------------------------------------------------------------------------------

  const NFmiMetTime &OriginTime(void) const;
  void OriginTime(const NFmiMetTime &newTime);
  const NFmiMetTime &ValidTime(void) const;

  bool IsOriginTime(void) const;
  bool IsValidTime(void) const;

  // ---------------------------------------------------------------------
  // Following methods operate just in TimeDescriptor's list part
  // ---------------------------------------------------------------------

  virtual unsigned long Index(void) const;
  virtual unsigned long Size(void) const;
  bool IsEmpty(void) const;  // Jouduin tekemään IsEmpty -metodin, koska Size -metodi palauttaa
  // joskus tyhjänä 1:n (NFmiTimeBag palauttaa tyhjänä 1:n kun resolution
  // on 0 jostain historiallisista v. 1998 syistä)
  virtual unsigned long SizeActive(void) const;

  virtual bool NextActive(void);
  virtual bool PreviousActive(void);
  virtual bool FirstActive(void);
  virtual bool LastActive(void);
  virtual bool IsActive(void) const;
  virtual bool SetActivity(bool theActivityState);
  virtual bool SetActivePeriod(bool theActivityState, const NFmiTimeBag &thePeriod);
  virtual NFmiTimeBag GetActivePeriod(void);

  void SetLocalTimes(const float theLongitude);  // Muuttaa ajan paikalliseksi
  FmiTimeLocalzation TimeLocalzation(void) const;
  bool IsInterpolation(void) const;
  bool IsOriginLastest(void) const;
  void OriginLastest(bool theLastest);
  const NFmiTimePerioid Resolution(void) const;
  const NFmiTimePerioid ActiveResolution(void);

  NFmiTimeBag *ValidTimeBag(void) const;
  NFmiTimeBag *OriginTimeBag(void) const;
  NFmiTimeList *ValidTimeList(void) const;
  bool UseTimeList(void) const;

  NFmiTimeDescriptor &operator=(const NFmiTimeDescriptor &theTimeDescriptor);
  bool operator==(const NFmiTimeDescriptor &theTimeDescriptor) const;

  virtual NFmiTimeDescriptor Combine(const NFmiTimeDescriptor &theCombine,
                                     int theStartTimeFunction = 0,
                                     int theEndTimeFunction = 0) const;
  NFmiTimeDescriptor GetIntersection(const NFmiMetTime &theStartLimit,
                                     const NFmiMetTime &theEndLimit);
  void PruneTimes(int theMaxTimeCount, bool fFromEnd = true);

  virtual unsigned long ClassId(void) const;
  virtual const char *ClassName(void) const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  unsigned long LocalTimeStep(void);
  void ReduseTimeBag(void);
  void ExtendTimeBag(void);
  //  unsigned long TimeResolution(void); // poistettu, käytä Resolution-metodia
  bool IsInside(const NFmiMetTime &theTime) const;
  bool FindNearestTime(const NFmiMetTime &theTime,
                       FmiDirection theDirection = kCenter,
                       unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);
  void SetNewStartTime(const NFmiMetTime &theTime);

 protected:
  void Destroy(void);

 private:
  // ----
  // data
  // ----

  NFmiTimeBag *itsOriginTimeBag;
  NFmiTimeBag *itsValidTimeBag;
  NFmiTimeList *itsTimeList;

  // -------------------------------------------------------
  // ItsTimeBagIdent: True if validTime, False if originTime
  // -------------------------------------------------------

  bool itsTimeBagIdent;
  FmiTimeLocalzation itsIsLocalTime;
  bool itsIsInterpolation;
  bool itsIsOriginLastest;
  unsigned long itsLocalTimeStep;  // Kesken
  bool *itsActivity;

  // Hmm, should be removed??
  friend class NFmiDB;

};  // class NFmiTimeDescriptor

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiTimeLocalzation NFmiTimeDescriptor::TimeLocalzation(void) const { return itsIsLocalTime; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsInterpolation(void) const { return itsIsInterpolation; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsOriginLastest(void) const { return itsIsOriginLastest; }
// ----------------------------------------------------------------------
/*!
 * \param theLastest Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimeDescriptor::OriginLastest(bool theLastest) { itsIsOriginLastest = theLastest; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTimeBag *NFmiTimeDescriptor::ValidTimeBag(void) const { return itsValidTimeBag; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTimeBag *NFmiTimeDescriptor::OriginTimeBag(void) const { return itsOriginTimeBag; }
inline NFmiTimeList *NFmiTimeDescriptor::ValidTimeList(void) const { return itsTimeList; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTimeDescriptor::ForecastPeriod(void) const
{
  return itsTimeList ? 0 : (ValidTime().DifferenceInMinutes(OriginTime()));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsOriginTime(void) const { return !itsTimeBagIdent; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsValidTime(void) const { return itsTimeBagIdent; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsActive(void) const { return (itsActivity[Index()]); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::UseTimeList(void) const { return itsTimeList != 0; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTimeDescriptor::ClassId(void) const { return kNFmiTimeDescriptor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiTimeDescriptor::ClassName(void) const { return "NFmiTimeDescriptor"; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTimeDescriptor::LocalTimeStep(void) { return itsLocalTimeStep; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
inline
unsigned long NFmiTimeDescriptor::TimeResolution(void)
{
  return itsValidTimeBag->Resolution();
}
*/
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiTimeDescriptor
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiTimeDescriptor &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiTimeDescriptor
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiTimeDescriptor &ob)
{
  return ob.Read(file);
}

// ======================================================================
