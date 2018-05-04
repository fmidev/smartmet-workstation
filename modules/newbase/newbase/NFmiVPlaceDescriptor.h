// ======================================================================
/*!
 * \file NFmiVPlaceDescriptor.h
 * \brief Interface of class NFmiVPlaceDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiLevelBag.h"

//! Undocumented
class _FMI_DLL NFmiVPlaceDescriptor : public NFmiDataDescriptor
{
 public:
  virtual ~NFmiVPlaceDescriptor(void);
  NFmiVPlaceDescriptor(void);
  NFmiVPlaceDescriptor(const NFmiVPlaceDescriptor &theVPlaceDescriptor);
  NFmiVPlaceDescriptor(NFmiLevelBag &theLevelBag);

  NFmiVPlaceDescriptor &operator=(const NFmiVPlaceDescriptor &theVPlaceDescriptor);
  bool operator==(const NFmiVPlaceDescriptor &theVPlaceDescriptor) const;

  void Reset(void);
  bool Next(void);
  bool Previous(void);

  NFmiLevel *Level(void) const;
  NFmiLevel *Level(unsigned long theIndex) const;
  NFmiLevel *LevelMinValue(void) const;
  NFmiLevel *LevelMaxValue(void) const;
  bool Level(const NFmiLevel &theLevel);

  const NFmiLevelBag *Levels(void) const;

  bool IsStep(void) const;   // 'askel' määritelty? false jos ei.
  bool IsLevel(void) const;  // LevelBag määritelty?

  void LevelBag(NFmiLevelBag &theLevelBag);
  const NFmiLevelBag &LevelBag() const { return *itsLevelBag; }
  void Destroy(void);

  virtual unsigned long Index(void) const;
  bool Index(unsigned long theIndex);
  virtual unsigned long Size(void) const;

  virtual bool NextActive(void);
  virtual bool IsActive(void) const;
  virtual bool SetActivity(bool theActivityState);

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual const char *ClassName(void) const;

 private:
  NFmiLevelBag *itsLevelBag;
  bool *itsActivity;

};  // class NFmiVPlaceDescriptor

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiLevelBag *NFmiVPlaceDescriptor::Levels(void) const { return itsLevelBag; }
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiVPlaceDescriptor::Reset(void)
{
  if (itsLevelBag) itsLevelBag->Reset();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiVPlaceDescriptor::IsStep(void) const { return itsLevelBag->IsStep(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiVPlaceDescriptor::IsActive(void) const { return (itsActivity[Index()]); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiVPlaceDescriptor::ClassName(void) const { return "NFmiVPlaceDescriptor"; }
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiVPlaceDescriptor
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiVPlaceDescriptor &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiVPlaceDescriptor
 *
 * \param file The input stream to read form
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiVPlaceDescriptor &ob)
{
  return ob.Read(file);
}

// ======================================================================
