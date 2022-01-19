// ======================================================================
/*!
 * \file NFmiMetBoxQueryData.h
 * \brief Interface of class NFmiMetBoxQueryData
 */
// ======================================================================

#pragma once

#include "NFmiMetBox.h"
#include "NFmiQueryData.h"

//! Undocumented
class NFmiMetBoxQueryData : public NFmiQueryData
{
 public:
  ~NFmiMetBoxQueryData();
  NFmiMetBoxQueryData();
  NFmiMetBoxQueryData(const NFmiMetBoxQueryData& theData);
  NFmiMetBoxQueryData(NFmiQueryInfo& theInfo);

  virtual NFmiMetBox* Value();

 private:
  NFmiMetBoxQueryData& operator=(const NFmiMetBoxQueryData& theData);

  NFmiMetBox* itsMetBox;

};  // class NFmiMetBoxQueryData

// ======================================================================
