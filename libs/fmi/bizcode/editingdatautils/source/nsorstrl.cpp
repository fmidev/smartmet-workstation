// ____________________________________________________________________________
// © Ilmatieteenlaitos/ Marko	  		         File: nsorstrl.cpp
//   Original		04.06.1997 
//	 Last Update	04.06.1997 
//
//	 Update xx.xx.xxxxx/Viljo text......
//
// The implementation of CLASS:	NFmiSortedStringList
//----------------------------------------------------------------------------



#include  "nsorstrl.h"
#include "NFmiString.h"


//____________________________NFmiSortedStringList(NFmiSortedStringList& theList)

NFmiSortedStringList::NFmiSortedStringList(NFmiSortedStringList& theList, bool fMakeStringCopy) // 27.1.2000/Marko
: itsList(new NFmiSortedList(theList.itsList->SortOrder()))
			   , itsIter(0)
{
	theList.Reset();
	NFmiString *temp =0;
	while(theList.Next(&temp))
	{
		if(fMakeStringCopy) // 27.1.2000/Marko
			Add(temp->Clone()); // 27.1.2000/Marko
		else // 27.1.2000/Marko
			Add(temp);					// taas kopioidaan sisältö
	}
}

NFmiSortedStringList::NFmiSortedStringList(FmiSortOrder theSortOrder)
: itsList(new NFmiSortedList(theSortOrder))
, itsIter(0)
{
} 

NFmiSortedStringList::~NFmiSortedStringList(void) 
{
   if(itsList)
   {
		NFmiVoidPtrIterator iter(itsList);  
		void *vPt;					// iter.Reset() ??				 
		while(iter.Next(vPt))				 	
			 delete (reinterpret_cast<NFmiString *>(vPt));
		delete itsList;					 
		delete itsIter;
   }
}

//__________________________________________Next(NFmiString *theItem)

bool NFmiSortedStringList::Next(NFmiString **theItem) 
{
	*theItem=Current();
	if(*theItem)
	{					// VAROITUS !
		itsIter->Next();// Viimeisen alkion jälkeen mennään listan ulkopuolelle true:lla;
		return true;	// vasta seuraavalla kerralla tämä Next palauttaa false !!	
//		return Next(); <-- Näin kursori jääsi osoittamaan viimeistä itemiä, mutta toisaalta nyt
	}					// return false on harhaan johtava, sillä onhan saatu mielekäs theItem.

	return false;		// Suosittelen metodien Next(void) & Current() käyttöä, jolloin ei voi joutua ulos listalta
}						// viljo 12.05.-97


//______________________________________________________Next(void) 

bool NFmiSortedStringList::Next(void) 
{
	if( itsIter )		
	{
		itsIter->Next();
		if(/*itsIter->Next() &&*/itsIter->Current()) 	      //...ylivuodon esto
		{
			return true;
		}
	}
	return false;
}



//___________________________________________________ Current(void) const

NFmiString *NFmiSortedStringList::Current(void) const
{
	if(itsIter)
		return reinterpret_cast<NFmiString *>(itsIter->Current());
	return 0;
}


//____________________________________________________________ Reset(void)

bool NFmiSortedStringList :: Reset(void)
{ 
  if(itsIter)
  {
	itsIter->Reset();
	return NumberOfItems() != 0;
  }
  return 0;
}

//___________________________________________ Add(NFmiString *theItem)

void NFmiSortedStringList::Add(NFmiString *theItem, bool fCreateCopy )
{
	NFmiString* aItem;
	if( fCreateCopy == true )
	{
 		aItem = new NFmiString;
		*aItem = *theItem;
	}
	else 
	{
		aItem = theItem;		
	}
	itsList->Add(aItem);

	delete itsIter;
	itsIter = new NFmiVoidPtrIterator(itsList);
	itsIter->Reset();
}


//___________________________________________ Add(NFmiSortedStringList *theList)

void NFmiSortedStringList::Add(NFmiSortedStringList *theList)
{
	*itsList+=(*theList->itsList);
	delete itsIter;
	itsIter = new NFmiVoidPtrIterator(itsList);
	itsIter->Reset();
}



//_____________________________________________________ Clear( void )

void NFmiSortedStringList :: Clear(bool fDeleteData)
{
	if( fDeleteData )
	{
		NFmiString* aItem = 0;
		Reset();
		while(Next(&aItem))
			delete aItem;
	}
	if(itsList)
		itsList->Clear();
	Reset();
}

bool NFmiSortedStringList::NextExist(void)
{
	return NumberOfItems() ? NumberOfItems() > Index() : false;
}

//______________________________ & operator = (const NFmiSortedStringList& theList)

NFmiSortedStringList& NFmiSortedStringList::operator = (NFmiSortedStringList& theList)
{
	Clear(true);// 27.1.2000/Marko
	delete itsList;
	delete itsIter;
	itsList = new NFmiSortedList(theList.itsList->SortOrder());
	if(theList.Reset())
	{
		do
		{
			itsList->Add(theList.Current()->Clone());
		}while(theList.Next());
	}
	itsIter = new NFmiVoidPtrIterator(itsList);
	return *this;
}


//_____________________________________________________ Index( int index ) 

bool NFmiSortedStringList::Index( int index )
{
	return itsIter?itsIter->Index(index):false;	
}

//______________________________________________________ Index( void )

int NFmiSortedStringList::Index( void )
{
	return itsIter?itsIter->Index():false;		// index = -1 out of list
}


