//**********************************************************
// C++ Class Name : NFmiSortedPtrList
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiSortedPtrList.h
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_TemplateClass
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : ptrlist
//  - GD View Type      : Class Diagram
//  - GD View Name      : Class Diagram
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Tues - Feb 2, 1999
//
//
//  Description:
//   Pointeri lista jossa itemit ovat järjestetty
//   haluttuun järjestykseen.
//   Tähän luokkaan laitettavilla olioiden luokilla
//   on oltava operaattorit < ja ==
//
//  Change Log:
//
//**********************************************************
#pragma once

#include <newbase/NFmiPtrList.h>

template <class Type>
class NFmiSortedPtrList : public NFmiPtrList<Type>
{
 public:
  NFmiSortedPtrList(bool fAscendingOrder = true)
      : NFmiPtrList<Type>(), fAscending(fAscendingOrder){};

  virtual ~NFmiSortedPtrList(void){};

  bool InsertionSort(Type* theItem)
  {
    if (theItem)
    {
      typename NFmiPtrList<Type>::Iterator tempIter = this->Start();
      while (tempIter.Next())
      {
        if (*theItem < tempIter.Current())
          return tempIter.AddBefore(theItem);
      }
      return this->AddEnd(theItem);
    }
    return false;
  };

  bool Sort(bool fAscendingOrder = true)
  {
    typename NFmiPtrList<Type>::Iterator theIter1 = this->Start();
    typename NFmiPtrList<Type>::Iterator theIter2;
    while (theIter1.Next())
    {
      theIter2 = theIter1;
      while (theIter2.Next())
      {
        if ((theIter2.Current() < theIter1.Current() && fAscendingOrder) ||
            (theIter1.Current() < theIter2.Current() && !fAscendingOrder))
        {
          Swap(theIter1, theIter2);
        }
      }
    }
    return true;
  };

  typename NFmiPtrList<Type>::Iterator Find(Type* theItem)
  {
    typename NFmiPtrList<Type>::Iterator theIter = this->Start();
    while (theIter.Next())
    {
      if (theIter.Current() == *theItem)
        return theIter;
    }
    return theIter;
  };

 private:
 private:
  bool fAscending;  //   Määrää listan järjestyksen (true = nouseva).
};

