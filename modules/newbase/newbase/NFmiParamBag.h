// ======================================================================
/*!
 * \file NFmiParamBag.h
 * \brief Interface for class NFmiParamBag
 */
// ======================================================================

#pragma once

#include "NFmiDataIdent.h"
#include "NFmiDataMatrix.h"
#include "NFmiParameterName.h"
#include "NFmiSize.h"

//! Undocumented
class _FMI_DLL NFmiParamBag : public NFmiSize
{
 public:
  virtual ~NFmiParamBag(void);
  void Destroy(void);

  NFmiParamBag(void);
  NFmiParamBag(FmiParameterName *theParamArray, unsigned long numberOfParams);
  NFmiParamBag(NFmiDataIdent *theParamArray, unsigned long numberOfParams);
  NFmiParamBag(const NFmiParamBag &theBag);

  NFmiParamBag &operator=(const NFmiParamBag &theParam);
  bool operator==(const NFmiParamBag &theParam) const;

  virtual void Reset(FmiDirection directionToIter = kForward);
  virtual bool First(void);
  bool Next(void);
  bool Next(bool fIgnoreSubParam);  // = true);
  bool Previous(void);
  bool Previous(bool fIgnoreSubParam);  // =true);

  FmiParameterName CurrentParam(void) const;
  const NFmiString CurrentParamName(void) const;
  bool SetCurrent(FmiParameterName theParam, bool fIgnoreSubParam = true);
  bool SetCurrent(unsigned long theParam, bool fIgnoreSubParam = true);

  bool Add(const NFmiDataIdent &theParam, bool fCheckNoDuplicateParams = false);
  bool Remove(void);

  const NFmiDataIdent *GetFirst(void) const;
  const NFmiDataIdent *GetLast(void) const;

  NFmiDataIdent *Current(bool fIgnoreSubParam = true) const;
  NFmiDataIdent *Param(unsigned long theIndex, bool fIgnoreSubParam = true) const;
  NFmiDataIdent *EditParam(bool fIgnoreSubParam = true);
  NFmiDataIdent *EditParam(unsigned long theIndex, bool fIgnoreSubParam = true);

  bool Current(const NFmiDataIdent &theParam, bool fIgnoreSubParam = true);
  bool Current(const NFmiParam &theParam, bool fIgnoreSubParam = true);

  void SetProducer(const NFmiProducer &newProducer);

  bool IsSubParamUsed(void) const;
  bool FindSubParam(const NFmiParam &theParam);
  bool FindSubParam(const NFmiDataIdent &theDataIdent);

  virtual const NFmiParamBag Combine(const NFmiParamBag &theBag);

  bool NextActive(bool fIgnoreSubParam = true);
  bool NextData(void);
  unsigned long SizeOfActive(void);
  bool SetActive(const NFmiParam &theParam, bool isActive, bool fIgnoreSubParam = true);
  bool SetActive(unsigned long theIndex, bool isActive, bool fIgnoreSubParam = true);
  bool SetCurrentActive(bool isActive, bool fIgnoreSubParam = true);
  bool SetActivities(NFmiParamBag &theParams,
                     bool fDefaultActivity,
                     bool fUseDefaultActivity = true,
                     int theBinaryOperation = 0);
  void SetActivities(bool newState);
  virtual bool IsActive(bool fIgnoreSubParam = true) const;
  virtual bool IsActive(unsigned long index, bool fIgnoreSubParam = true) const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual const char *ClassName(void) const { return "NFmiParamBag"; }
  const checkedVector<NFmiDataIdent>& ParamsVector() const { return itsParamsVector; }

 private:
  checkedVector<NFmiDataIdent> itsParamsVector;
  bool fIsSubParamUsed;
  NFmiDataIdent *itsUsedSubParam;

};  // class NFmiParamBag

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamBag::First(void)
{
  fIsSubParamUsed = false;
  return NFmiSize::First();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiString NFmiParamBag::CurrentParamName(void) const
{
  return itsParamsVector[itsIndex].GetParamName();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiDataIdent *NFmiParamBag::GetFirst(void) const
{
  return GetSize() ? &itsParamsVector[0] : 0;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiDataIdent *NFmiParamBag::GetLast(void) const
{
  return GetSize() ? &itsParamsVector[GetSize() - 1] : 0;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamBag::IsSubParamUsed(void) const { return fIsSubParamUsed; }
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiParamBag
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiParamBag &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiParamBag &ob) { return ob.Read(file); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiParameterName NFmiParamBag::CurrentParam(void) const
{
  return FmiParameterName(Current()->GetParam()->GetIdent());
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamBag::SetCurrent(unsigned long theParam, bool fIgnoreSubParam)
{
  return SetCurrent(FmiParameterName(theParam), fIgnoreSubParam);
}

// ======================================================================
