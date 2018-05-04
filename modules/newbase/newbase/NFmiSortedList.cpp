// ======================================================================
/*!
 * \file NFmiSortedList.cpp
 * \brief Implementation of class NFmiSortedList
 */
// ======================================================================
/*!
 * \class NFmiSortedList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiSortedList.h"
#include "NFmiMetTime.h"
#include "NFmiSortable.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiSortedList::~NFmiSortedList() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSortOrder Undocumented
 */
// ----------------------------------------------------------------------

NFmiSortedList::NFmiSortedList(FmiSortOrder theSortOrder)
    : NFmiVoidPtrList(), itsSortOrder(theSortOrder)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theList The other list being copied
 */
// ----------------------------------------------------------------------

NFmiSortedList::NFmiSortedList(NFmiSortedList &theList)

    = default;

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \param fAddDuplicates Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSortedList::Add(NFmiSortable *theItem, bool fAddDuplicates)
{
  if (itsSortOrder == kAscending)
    return AddAscending(theItem, fAddDuplicates);
  else
    return AddDescending(theItem, fAddDuplicates);
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \param fAddDuplicates Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSortedList::AddAscending(NFmiSortable *theItem, bool fAddDuplicates)
{
  if (!itsNumberOffItems)
  {
    AddEnd(theItem);
    return true;
  }
  void *anItem;
  NFmiVoidPtrIterator it(this);
  it.Reset();
  while (it.Next(anItem))
  {
    if (!fAddDuplicates && (*static_cast<NFmiSortable *>(anItem) == *theItem))  // 25.02.1998/Marko
      return false;
    if (*theItem < (*static_cast<NFmiSortable *>(anItem)))  // 11.02.1998/Marko
    // new item with same 'order'-value as a item already in list will be placed last of the same
    // order-values
    {
      AddBefore(theItem);
      return true;
    }
  }
  AddEnd(theItem);
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \param fAddDuplicates Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSortedList::AddDescending(NFmiSortable *theItem, bool fAddDuplicates)
{
  if (!itsNumberOffItems)
  {
    AddEnd(theItem);
    return true;
  }
  void *anItem;
  NFmiVoidPtrIterator it(this);
  it.Reset();
  while (it.Next(anItem))
  {
    if (!fAddDuplicates && (*static_cast<NFmiSortable *>(anItem) == *theItem))  // 25.02.1998/Marko
      return false;
    if (*static_cast<NFmiSortable *>(anItem) < *theItem)
    {
      AddBefore(theItem);
      return true;
    }
  }
  AddEnd(theItem);
  return true;
}

// ======================================================================
