// ======================================================================
/*!
 * \file NFmiEditParam.h
 * \brief Interface of class NFmiEditParam
 */
// ======================================================================

#pragma once

#include "NFmiParam.h"

enum FmiMaskMethod
{
  kFmiIsLessThan = 0,
  kFmiIsGreaterThan,
  kFmiIsEqual
};

//! Undocumented class, with stupid interface
class _FMI_DLL NFmiEditParam : public NFmiParam
{
 public:
  virtual ~NFmiEditParam();
  NFmiEditParam();

 private:
  FmiMaskMethod itsMaskMethod;
  double itsMaskValue;

};  // class NFmiEditParam

// ======================================================================
