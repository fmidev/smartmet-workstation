// ======================================================================
/*!
 * \file NFmiMetBox.h
 * \brief Interface of class NFmiMetBox
 */
// ======================================================================

#pragma once

#include "NFmiBox.h"
#include "NFmiLocationBag.h"
#include "NFmiParamBag.h"
#include "NFmiStation.h"
#include "NFmiTimeBag.h"

//! Undocumented, should use a const int instead
#define HEADERMAX 3

//! Undocumented
class _FMI_DLL NFmiMetBox
{
 public:
  virtual ~NFmiMetBox();

  NFmiMetBox();

  NFmiMetBox(const NFmiMetBox &theMetBox);

  NFmiMetBox(const NFmiTimeBag &theTimeDescriptor,
             const NFmiLocationBag &theStationDescriptor,
             const NFmiParamBag &theParamDescriptor);

  const NFmiTimeBag GetTimeDescriptor() const;
  const NFmiLocationBag GetStationDescriptor() const;
  const NFmiParamBag GetParamDescriptor() const;

  const NFmiString Header1() const;
  const NFmiString Header2() const;
  const NFmiString Header3() const;
  void Header1(const NFmiString &theHeader);
  void Header2(const NFmiString &theHeader);
  void Header3(const NFmiString &theHeader);

  void Fill(float theFillValue);

  float &operator[](long theIndex);
  float Value(unsigned long theIndex);

 protected:
  virtual long CalcSize();
  long CalcTimeAddition();
  long CalcStationAddition();

  long CurrentTimeIndex() const;
  long CurrentStationIndex() const;
  long CurrentParamIndex() const;

  virtual long CalcBoxIndex(unsigned long theTimeIndex,
                            unsigned long theStationIndex,
                            unsigned long theParamIndex);

  long GetSize() const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

 private:
  NFmiTimeBag *itsTimeDescriptor;
  NFmiParamBag *itsParamDescriptor;
  NFmiLocationBag *itsStationDescriptor;
  NFmiBox *itsData;

  NFmiString itsHeader[HEADERMAX];

 private:
  NFmiMetBox operator=(const NFmiMetBox &theBox);
  friend class NFmiMetBoxIterator;

};  // class NFmiMetBox

//! Undocumented
class _FMI_DLL NFmiMetBoxIterator
{
 public:
  virtual ~NFmiMetBoxIterator();
  NFmiMetBoxIterator(const NFmiMetBoxIterator &theIterator);
  NFmiMetBoxIterator(NFmiMetBox *theBox);
  NFmiMetBoxIterator(NFmiMetBox *theBox,
                     const NFmiTimeBag &theTimeDescriptor,
                     const NFmiLocationBag &theStationDescriptor,
                     const NFmiParamBag &theParamDescriptor);

 public:
  NFmiTimeBag *GetTimeDescriptor() const;
  NFmiLocationBag *GetStationDescriptor() const;
  NFmiParamBag *GetParamDescriptor() const;
  void ResetTime(FmiDirection directionToIter = kForward);
  void ResetStation(FmiDirection directionToIter = kForward);
  void ResetParam(FmiDirection directionToIter = kForward);
  void Reset(FmiDirection directionToIter = kForward);

  const TMetTime FirstTime();
  const TMetTime LastTime();
  unsigned long TimeFromStart();
  bool NextTime();
  bool NextStation();
  bool NextParam();
  bool NextActiveParam();
  bool NextDataParam();

  bool NextTimeValue(float &theBoxValue);
  bool NextStationValue(float &theBoxValue);
  bool NextParamValue(float &theBoxValue);

  bool PreviousTime();
  bool PreviousStation();
  bool PreviousParam();

  bool PreviousTimeValue(float &theBoxValue);
  bool PreviousStationValue(float &theBoxValue);
  bool PreviousParamValue(float &theBoxValue);

  float CurrentValue();

  const TMetTime CurrentTime();
  unsigned long CurrentStation();
  FmiParameterName CurrentParam();

  const TMetTime Time();
  const NFmiStation Station();
  const NFmiDataIdent Param();

  unsigned long TimeResolution();

  bool CurrentTime(const TMetTime &newTime);
  bool CurrentStation(unsigned long newStation);
  bool CurrentParam(FmiParameterName newParam);
  bool CurrentParam(const NFmiDataIdent &newParam);

  virtual long CalcBoxIndex();

  bool MapCursorFrom(const NFmiMetBoxIterator &theIterator);

 private:
  NFmiMetBoxIterator &operator=(const NFmiMetBoxIterator &theBox);
  NFmiMetBox *itsBox;
  NFmiTimeBag *itsTimeDescriptor;
  NFmiParamBag *itsParamDescriptor;
  NFmiLocationBag *itsStationDescriptor;

};  // class NFmiMetBoxIterator

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMetBox The object being copied
 * \todo Shouldn't be inline
 */
// ----------------------------------------------------------------------

inline NFmiMetBox::NFmiMetBox(const NFmiMetBox &theMetBox)
    : itsTimeDescriptor(new NFmiTimeBag(*theMetBox.itsTimeDescriptor)),
      itsParamDescriptor(new NFmiParamBag(*theMetBox.itsParamDescriptor)),
      itsStationDescriptor(new NFmiLocationBag(*theMetBox.itsStationDescriptor)),
      itsData(new NFmiBox(*theMetBox.itsData)),
      itsHeader()
{
  for (int i = 0; i < HEADERMAX; i++)
    itsHeader[i] = theMetBox.itsHeader[i];
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiTimeBag NFmiMetBox::GetTimeDescriptor() const { return *itsTimeDescriptor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiLocationBag NFmiMetBox::GetStationDescriptor() const
{
  return *itsStationDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiParamBag NFmiMetBox::GetParamDescriptor() const { return *itsParamDescriptor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiString NFmiMetBox::Header1() const { return itsHeader[0]; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiString NFmiMetBox::Header2() const { return itsHeader[1]; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiString NFmiMetBox::Header3() const { return itsHeader[2]; }
// ----------------------------------------------------------------------
/*!
 * \param theHeader Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetBox::Header1(const NFmiString &theHeader) { itsHeader[0] = theHeader; }
// ----------------------------------------------------------------------
/*!
 * \param theHeader Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetBox::Header2(const NFmiString &theHeader) { itsHeader[1] = theHeader; }
// ----------------------------------------------------------------------
/*!
 * \param theHeader Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetBox::Header3(const NFmiString &theHeader) { itsHeader[2] = theHeader; }
// ----------------------------------------------------------------------
/*!
 * \param theFillValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetBox::Fill(float theFillValue) { itsData->Fill(theFillValue); }
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float &NFmiMetBox::operator[](long theIndex) { return itsData->operator[](theIndex); }
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiMetBox::Value(unsigned long theIndex) { return itsData->Value(theIndex); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiMetBox::GetSize() const { return itsData->GetSize(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTimeBag *NFmiMetBoxIterator::GetTimeDescriptor() const { return itsTimeDescriptor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLocationBag *NFmiMetBoxIterator::GetStationDescriptor() const
{
  return itsStationDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiParamBag *NFmiMetBoxIterator::GetParamDescriptor() const { return itsParamDescriptor; }
// ----------------------------------------------------------------------
/*!
 * \param directionToIter Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetBoxIterator::ResetTime(FmiDirection directionToIter)
{
  itsTimeDescriptor->Reset(directionToIter);
}

// ----------------------------------------------------------------------
/*!
 * \param directionToIter Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetBoxIterator::ResetStation(FmiDirection directionToIter)
{
  itsStationDescriptor->Reset(directionToIter);
}

// ----------------------------------------------------------------------
/*!
 * \param directionToIter Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetBoxIterator::ResetParam(FmiDirection directionToIter)
{
  itsParamDescriptor->Reset(directionToIter);
}

// ----------------------------------------------------------------------
/*!
 * \param directionToIter Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetBoxIterator::Reset(FmiDirection directionToIter)
{
  ResetTime(directionToIter);
  ResetStation(directionToIter);
  ResetParam(directionToIter);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const TMetTime NFmiMetBoxIterator::FirstTime() { return itsTimeDescriptor->FirstTime(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const TMetTime NFmiMetBoxIterator::LastTime() { return itsTimeDescriptor->LastTime(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiMetBoxIterator::TimeFromStart()
{
  return itsTimeDescriptor->TimeFromStart();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const TMetTime NFmiMetBoxIterator::CurrentTime() { return itsTimeDescriptor->CurrentTime(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiMetBoxIterator::CurrentStation()
{
  return (static_cast<const NFmiStation *>(itsStationDescriptor->Location())->GetIdent());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiParameterName NFmiMetBoxIterator::CurrentParam()
{
  return itsParamDescriptor->CurrentParam();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const TMetTime NFmiMetBoxIterator::Time() { return itsTimeDescriptor->CurrentTime(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiStation NFmiMetBoxIterator::Station()
{
  return *(const_cast<NFmiStation *>(
      static_cast<const NFmiStation *>(itsStationDescriptor->Location())));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiDataIdent NFmiMetBoxIterator::Param() { return *(itsParamDescriptor->Current()); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiMetBoxIterator::TimeResolution()
{
  return itsTimeDescriptor->Resolution();
}

// ----------------------------------------------------------------------
/*!
 * \param newTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiMetBoxIterator::CurrentTime(const TMetTime &newTime)
{
  return itsTimeDescriptor->SetCurrent(newTime);
}

// ----------------------------------------------------------------------
/*!
 * \param newStation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiMetBoxIterator::CurrentStation(unsigned long newStation)
{
  return itsStationDescriptor->Location(NFmiStation(newStation));
}

// ----------------------------------------------------------------------
/*!
 * \param newParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiMetBoxIterator::CurrentParam(FmiParameterName newParam)
{
  return itsParamDescriptor->SetCurrent(newParam);
}

// ----------------------------------------------------------------------
/*!
 * \param newParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiMetBoxIterator::CurrentParam(const NFmiDataIdent &newParam)
{
  return itsParamDescriptor->Current(newParam);
}

// ======================================================================
