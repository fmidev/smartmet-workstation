// ======================================================================
/*!
 * \file NFmiSize.h
 * \brief Interface of class NFmiSize
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <iostream>

//! Undocumenetd
class _FMI_DLL NFmiSize
{
 public:
  virtual ~NFmiSize(void) {}
  NFmiSize(void);
  NFmiSize(const NFmiSize& theSize);
  NFmiSize(unsigned long theSize);
  NFmiSize(unsigned long theIndex, unsigned long theSize);

  virtual void Reset(FmiDirection directionToIter = kForward);
  virtual bool First(void);
  virtual bool Next(void);
  virtual bool Previous(void);
  virtual long CurrentIndex(void) const;

  virtual unsigned long GetSize(void) const;
  virtual void SetSize(unsigned long newSize);
  bool SetCurrentIndex(unsigned long theIndex);

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  virtual const char* ClassName(void) const;

 protected:
  unsigned long itsSize;
  long itsIndex;

};  // class NFmiSize

// ======================================================================
