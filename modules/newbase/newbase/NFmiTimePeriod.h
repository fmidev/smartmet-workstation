// ======================================================================
/*!
 * \file NFmiTimePeriod.h
 * \brief Interface of class NFmiTimePeriod
 *
 * \todo Either rename the class or rename the file
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <iostream>

//! Undocumented
class _FMI_DLL NFmiTimePerioid
{
 public:
  virtual ~NFmiTimePerioid() {}
  NFmiTimePerioid(void);
  NFmiTimePerioid(const NFmiTimePerioid &theTimePerioid);
  NFmiTimePerioid(long minutes);
  NFmiTimePerioid(long years,
                  long month,
                  long days,
                  long hours,
                  long minutes,
                  long seconds,
                  long microSeconds = 0);

  void TimePerioid(long years,
                   long month,
                   long days,
                   long hours,
                   long minutes,
                   long seconds,
                   long microSeconds = 0);
  void TimePerioid(long seconds, long microSeconds = 0);

  long Year(void) const;
  long Month(void) const;
  long Day(void) const;
  long Hour(void) const;
  long Minute(void) const;
  long Second(void) const;
  long MicroSecond(void) const;

  void Year(long theYears);
  void Month(long theMonths);
  void Day(long theDays);
  void Hour(long theHours);
  void Minute(long theMinutes);
  void Second(long theSecond);
  void MicroSecond(long theMicroSecond);

  bool IsValue(void) const;

  NFmiTimePerioid &operator=(const NFmiTimePerioid &theTimePerioid);
  NFmiTimePerioid &operator+=(const NFmiTimePerioid &theTimePerioid);

  operator long(void) const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual const char *ClassName(void) const;

 protected:
  // used only NFmiMetTime ..

  bool IsDate(void) const;
  NFmiTimePerioid &operator=(const long theMinutes);
  void CalculatePerioid(void);

 private:
  long itsYears;
  long itsMonths;
  long itsDays;
  long itsHours;
  long itsMinutes;
  long itsSeconds;
  long itsMicroSeconds;

  friend class NFmiMetTime;
  friend class NFmiTimeBag;
  friend class NFmiTimeDescriptor;

};  // class NFmiTimePeriod

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTimePerioid::Year(void) const { return itsYears; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTimePerioid::Month(void) const { return itsMonths; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTimePerioid::Day(void) const { return itsDays; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTimePerioid::Hour(void) const { return itsHours; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTimePerioid::Minute(void) const { return itsMinutes; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTimePerioid::Second(void) const { return itsSeconds; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTimePerioid::MicroSecond(void) const { return itsMicroSeconds; }
// ----------------------------------------------------------------------
/*!
 * \param theYears Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimePerioid::Year(long theYears) { itsYears = theYears; }
// ----------------------------------------------------------------------
/*!
 * \param theMonths Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimePerioid::Month(long theMonths) { itsMonths = theMonths; }
// ----------------------------------------------------------------------
/*!
 * \param theDays Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimePerioid::Day(long theDays) { itsDays = theDays; }
// ----------------------------------------------------------------------
/*!
 * \param theHours Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimePerioid::Hour(long theHours) { itsHours = theHours; }
// ----------------------------------------------------------------------
/*!
 * \param theMinutes Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimePerioid::Minute(long theMinutes) { itsMinutes = theMinutes; }
// ----------------------------------------------------------------------
/*!
 * \param theSecond Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimePerioid::Second(long theSecond) { itsSeconds = theSecond; }
// ----------------------------------------------------------------------
/*!
 * \param theMicroSecond Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimePerioid::MicroSecond(long theMicroSecond) { itsMicroSeconds = theMicroSecond; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimePerioid::IsValue(void) const
{
  return (itsYears != 0 || itsMonths != 0 || itsDays != 0 || itsHours != 0 || itsMinutes != 0 ||
          itsSeconds != 0 || itsMicroSeconds != 0);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTimePerioid::operator long(void) const
{
  return (itsDays * 24 * 60 + itsHours * 60 + itsMinutes);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiTimePerioid::ClassName(void) const { return "NFmiTimePerioid"; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimePerioid::IsDate(void) const
{
  return (itsYears != 0 || itsMonths != 0 || itsDays != 0);
}

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiTimePeriod
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiTimePerioid &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiTimePeriod
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiTimePerioid &ob) { return ob.Read(file); }

// ======================================================================
