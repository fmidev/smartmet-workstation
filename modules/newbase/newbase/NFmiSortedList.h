// ======================================================================
/*!
 * \file NFmiSortedList.h
 * \brief Interface of class NFmiSortedList
 */
// ======================================================================

#pragma once

#include "NFmiVoidPtrList.h"

//! Undocumented
enum FmiSortOrder
{
  kAscending = 1,
  kDescending = 2
};

class NFmiSortable;

//! Undocumented
class _FMI_DLL NFmiSortedList : public NFmiVoidPtrList
{
 public:
  virtual ~NFmiSortedList(void);
  NFmiSortedList(NFmiSortedList& theList);
  NFmiSortedList(FmiSortOrder theSortOrder = kAscending);

  using NFmiVoidPtrList::Add;
  virtual bool Add(NFmiSortable* theItem, bool fAddDuplicates = true);
  FmiSortOrder SortOrder(void);

 private:
  bool AddAscending(NFmiSortable* theItem, bool fAddDuplicates = true);
  bool AddDescending(NFmiSortable* theItem, bool fAddDuplicates = true);

  FmiSortOrder itsSortOrder;

};  // class NFmiSortedList

//! Undocumented, should be removed
typedef NFmiSortedList* PNFmiSortedList;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be const
 */
// ----------------------------------------------------------------------

inline FmiSortOrder NFmiSortedList::SortOrder(void) { return itsSortOrder; }
