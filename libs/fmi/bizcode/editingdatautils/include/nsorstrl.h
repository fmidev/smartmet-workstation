// ____________________________________________________________________________
// © Ilmatieteenlaitos/Marko							File: nsorstrl.h
//   Original		04.06.1997
//	 Last Update	04.06.1997
//
//   Documentation/Marko
//   Original		04.06.1997
//	 Last Update	04.06.1997
// ____________________________________________________________________________
// 
// CLASS:	NFmiSortedStringList
// 
// SYNOPSIS:	#include "nsorstrl.h" 
// 
// USED BY:	C++ -programs
// 
// GENERAL DESCRIPTION:
// 
// This pointer list is made for the objects of the class NFmiString. Strings
// are sorted in alfabethical order.
// It encapsulates the void pointer list and its iterator having pointers to 
// objects of the classes NFmiVoidPtrList and NFmiVoidPtrIterator as attributes
// of this class. Through casting the data of NFmiVoidPtrList-object classifyed
// by type void to the pointers of the NFmiString objects the methods of 
// classes NFmiVoidPtrList and NFmiVoidPtrIterator are available.
//  
// 
// INHERETANCE:
//
// NOTES:	xxx
// 
// SEE ALSO: NFmiVoidPtrList.h
// ____________________________________________________________________________
//
// -----------------------
// Contents of the Manual:
// -----------------------
// ____________________________________________________________________________
//
//   1. Constructors/Destructors
//
//		NFmiSortedStringList(void);
//		NFmiSortedStringList(NFmiSortedStringList& theList)
//		~NFmiSortedStringList(void)
//
//   2. Methods/Member functions
//
//		void			  Clear( void );	 
//		bool		  Reset( void );	 
//		int				  NumberOfItems( void );
//		NFmiString*		  Current( void ) const;
//		bool		  Next( NFmiString **theItem );
//		bool		  Next( void ) ;
//		void			  Add( NFmiString *theItem, bool fCreateCopy = false );  
//		void			  Add( NFmiString *theItem );	 
//		void			  CopyList( NFmiStringList &theList );  
//	
//
//   3. Operators
//
//		NFmiSortedStringList& operator = (const NFmiSortedStringList& theList);	
//		virtual ostream_withassign &Write(ostream_withassign &file);	
//		virtual istream_withassign &Read(istream_withassign &file); 
//
//	 4. Attributes/Data
//
//		NFmiSortedList *itsList;	 
//		NFmiVoidPtrIterator *itsIter;	 
//	
//
//   5. Examples
// 
// ---------------
// 1. CONSTRUCTORS
// --------------- 
// 
//		NFmiSortedStringList( void );
//			Creates an empty list with itsIter=0 and sets istTime to a current UTC-time 
//			ignoring the minutes and seconds.
//
//		NFmiSortedStringList(NFmiSortedStringList& theList);
//			The copyconstructor which  copyes the data and creates a new list for it.
//
//		~NFmiSortedStringList( void );
//			Destroys first the data, then the list and its iterator.
//
//
// 1.1. INPUT PARAMETERS/ARGUMENT LISTS 
//		
// 
// 1.2. NOTES
// 
// 
// 
// ---------------------------  
// 2. METHODS/MEMBER FUNCTIONS
// ---------------------------
//
// 2.1. public
//
// 2.1.1  void Clear( void );
//			Deletes all the list items of itsList but remains the list itself.	
//	 
// 2.1.2  bool Reset( void );
//			Resets the iterator itsIter; moves the cursor to point to the  firs item 
//			of the list.
//
// 2.1.3  int NumberOfItems( void );
//			Returns the number of items in the list itsList.
//
// 2.1.4  NFmiString* Current( void ) const;
//			Returns the pointer to the data (NFmiString object) of the current
//			item of the list itsList.
//
// 2.1.5  bool Next( void ) ;
//			Moves the iterators itsIter cursor to the next item of the list itsList.
//
// 2.1.6  bool Next( NFmiString **theItem );
//			Takes the pointer to the pointer theItem as input parameter.  The input  
//			pointer theItem retrieves the return pointer pointing to the data of 
//			the current item. Then calls Next() and returns the bool; true 
//			if pointer value of theItem exist, others false. 
//			WARNING: This method moves the cursor out of the list when the calling 
//					 takes place on the last item of the list.
//				
// 2.1.7  bool Find( NFmiString &theStationName, bool fResetFirst = true );
//			Takes the object of NFmiString theStationName as input parameter and seeks
//			the corresponding item in the list having the same station name (Icao). The
//			cursors remains pointing the item first finded and the returning bool  
//			is true. If the finding fails, the cursor returns to the starting point 
//			and returnig bool is false. The second input parameter bool
//			fResetFirst controls whether theList will be reset before seeking or not; 
//			in the latter case the finding procedure starts from the next item (if it exist). Use Current() for returning the pointer to the
//			data of the items finded. By repeating the this method with fResetFirst = false
//			(or use FindNext() indrodused in (2.1.10)) moves cursor pointing to the next 
//			item with same station name this case exist.
//
// 2.1.8  void Add( NFmiString *theItem, bool fCreateCopy = false );  
//			Adds the given parameter pointer of NFmiString-object theItem to the end of 
//			the list. The another input parameter bool fCreateCopy tells if the copy 
//			of data or the original data is added (the default case).   		
//
// 2.1.9  void Add( NFmiSortedStringList *theList );
//			Adds the contents of the list given as a parameter by iterating through theList
//			and calling Add( NFmiString *theItem, bool fCreateCopy = false ) to
//			every item. As result the same data pointer exist in both lists.
//			WARNING: don't delete the list added, please use Flush().			
//	 
//			 			
//
// 2.2. protected:
//
// 2.3. private:
//
// 2.3.1  bool Index( int index );
//			Sets the cursor pointing to the item with the same index.  
//
// 2.3.1  int Index( void );
//			Returns the index of current item. 
//  
//
// 
// ------------------  
// 3. ATTRIBUTES/DATA
// ------------------
//
//		  NFmiVoidPtrList *itsList;	 
//			The core of this class is this void pointer list, now for pointers of 
//			NFmiString objects. The Add-medhods are mostly used, the medhod
//			Add(NFmiStringList *theList) uses operator+= of itsList.
//
//		  NFmiVoidPtrIterator *itsIter;	 
//			The iterator of itsList itsIter is able to move in the list by Next-medhods,
//			the medhod Reset sets the cursor to point to the first item of the list
//			the Current-medhod returns the pointer to the data of the current item. 
// 
// 
// ------------
// 4. OPERATORS
// ------------
// 
// 2.4.1  NFmiSortedStringList& operator = (const NFmiSortedStringList& theList);
//			Copyes the contents of theList to the calling object.
//
// 2.4.2  virtual ostream_withassign &Write(ostream_withassign &file);	
//		  virtual istream_withassign &Read(istream_withassign &file); 
//			These I/O functions reads and writes data to and from files 
//			and devices by operator "<<" and ">>".  
// ____________________________________________________________________________
// 
// -----------
// 5. EXAMPLES
// ----------- 
// 
// ____________________________________________________________________________



#pragma once

#include "NFmiSortedList.h"

class NFmiString;

//_________________________________________________________ NFmiStringList
class NFmiSortedStringList
{
 public:

   NFmiSortedStringList(FmiSortOrder theSortOrder = kAscending); 
   NFmiSortedStringList(NFmiSortedStringList& theList, bool fMakeStringCopy = false); // 27.1.2000/Marko
   virtual ~NFmiSortedStringList(void);

	 void        Add(NFmiString *theItem, bool fCreateCopy = false ); 
	 void		 Add(NFmiSortedStringList *theList);
 	 bool  Reset(void);
	 bool  Next(NFmiString **theItem);
	 bool  Next(void) ;
	 NFmiString   *Current(void) const;
	 int          NumberOfItems(void) {return itsList?itsList->NumberOfItems():0;};
	 void         Clear(bool fDeleteData=false);
	 NFmiSortedStringList& operator = (NFmiSortedStringList& theList);
	 bool NextExist(void);
     bool			  Index( int index );  
     int				  Index( void );	   
 
private:
	 NFmiSortedList*	  itsList;
	 NFmiVoidPtrIterator* itsIter; 	 
};

typedef NFmiSortedStringList* PNFmiSortedStringList;


