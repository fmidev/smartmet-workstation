// ======================================================================
/*!
 * \file NFmiVoidPtrList.h
 * \brief Interface to class NFmiVoidPtrList
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"

const int kDelete = true;
const int kNoDelete = false;
const long kNotInList = -1;

// ----------------------------------------------------------------------
/*!
 * \class NFmiVoidPtrData
 *
 * Ignore it, do not use it.
 */
// ----------------------------------------------------------------------

class _FMI_DLL NFmiVoidPtrData
{
  friend class NFmiVoidPtrList;
  friend class NFmiVoidPtrItem;
  friend class NFmiVoidPtrIterator;

 public:
  NFmiVoidPtrData(void* value) : itsDataValue(value), itsReferenceCount(0) {}
  void* GetVoidPtr(void) { return itsDataValue; }
  bool Remove(void) { return --itsReferenceCount ? false : true; }

 private:
  NFmiVoidPtrData(const NFmiVoidPtrData& theData);
  NFmiVoidPtrData& operator=(const NFmiVoidPtrData& theData);

  void* itsDataValue;
  int itsReferenceCount;
};

// ----------------------------------------------------------------------
/*!
 * \class NFmiVoidPtrItem
 *
 * Ignore it, do not use it
 *
 */
// ----------------------------------------------------------------------

class _FMI_DLL NFmiVoidPtrItem
{
  friend class NFmiVoidPtrList;
  friend class NFmiVoidPtrIterator;

 public:
  ~NFmiVoidPtrItem(void)
  {
    if (itsValue->Remove()) delete itsValue;
  }

  NFmiVoidPtrItem(NFmiVoidPtrData* value, NFmiVoidPtrItem* item = 0) : itsNextItem(item), itsValue()
  {
    if (value->itsReferenceCount)
    {
      itsValue = value;
    }
    else
      itsValue = value;
    itsValue->itsReferenceCount++;
  }

 private:
  NFmiVoidPtrItem(const NFmiVoidPtrItem& theItem);
  NFmiVoidPtrItem& operator=(const NFmiVoidPtrItem& theItem);

  NFmiVoidPtrItem* itsNextItem;
  NFmiVoidPtrData* itsValue;
};

// ----------------------------------------------------------------------
/*!
 * \class NFmiVoidPtrList
 *
 * Your risk your life for using this for anything new! Use
 * std::list instead.
 */
// ----------------------------------------------------------------------

class _FMI_DLL NFmiVoidPtrList
{
  friend class NFmiVoidPtrIterator;

 public:
  virtual ~NFmiVoidPtrList(void) { Clear(0); }
  NFmiVoidPtrList(void);
  NFmiVoidPtrList(const NFmiVoidPtrList& listItem);

  virtual void Add(void* value) { Add(new NFmiVoidPtrData(value)); }
  virtual void AddStart(void* value) { AddStart(new NFmiVoidPtrData(value)); }
  virtual void AddEnd(void* value) { AddEnd(new NFmiVoidPtrData(value)); }
  virtual void AddBefore(void* value) { AddBefore(new NFmiVoidPtrData(value)); }
  virtual void Add(NFmiVoidPtrData* value);
  virtual void AddStart(NFmiVoidPtrData* value);

  virtual void AddEnd(NFmiVoidPtrData* value);
  virtual void AddBefore(NFmiVoidPtrData* value);
  virtual void Remove(NFmiVoidPtrData* removeValue);
  virtual void Clear(bool doDelete = kNoDelete);

  int NumberOfItems(void) { return itsNumberOffItems; }
  NFmiVoidPtrItem* FirstItem(void) { return itsFirstItem; }
  void operator+=(const NFmiVoidPtrList& listItem);

  virtual void CopyList(const NFmiVoidPtrList& listItem);
  virtual void DeleteItem(void);

 protected:
  NFmiVoidPtrItem* itsFirstItem;
  NFmiVoidPtrItem* itsCurrentItem;
  NFmiVoidPtrItem* itsPreviousItem;
  int itsNumberOffItems;

 private:
  NFmiVoidPtrList& operator=(const NFmiVoidPtrList& theList);
};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
//                            Iterator                          II
// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

// ----------------------------------------------------------------------
/*!
 * \class NFmiVoidPtrIterator
 *
 * Do not use this
 *
 */
// ----------------------------------------------------------------------

class _FMI_DLL NFmiVoidPtrIterator
{
 public:
  virtual ~NFmiVoidPtrIterator() {}
  NFmiVoidPtrIterator(NFmiVoidPtrList* listItem);
  NFmiVoidPtrIterator(NFmiVoidPtrList& listItem);

  void Reset(void);
  virtual void* Next(void);
  virtual NFmiVoidPtrData* NextPtr(void);
  virtual NFmiVoidPtrData* CurrentPtr(void);
  virtual bool Next(void*& theItem);
  virtual void NextPreviousPtr(void);  // tämä on todella tyhmää
  virtual bool NextPtr(NFmiVoidPtrData*& theItem);
  long Index(void) const;
  bool Index(long theNewValue);

  void* operator++(void) { return Next(); }
  virtual void* Current(void);

 private:
  //  NFmiVoidPtrIterator(const NFmiVoidPtrIterator & theIterator);
  //  NFmiVoidPtrIterator & operator=(const NFmiVoidPtrIterator & theIterator);

  bool CheckIndex(long theValue) const;
  long itsIndex;
  NFmiVoidPtrItem* itsCurrentItem;
  NFmiVoidPtrItem* itsPreviousItem;
  NFmiVoidPtrList* itsListItem;
};

// ======================================================================
