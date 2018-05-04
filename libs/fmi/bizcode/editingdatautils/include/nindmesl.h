/*----------------------------------------------------------------------------//
// ____________________________________________________________________________
// © Ilmatieteenlaitos/Marko				File: nindmesl.h
//   Original		27.07.1997
//	 Last Update	27.07.1997
//
//   Documentation/Marko
//   Original		27.07.1997
//	 Last Update	27.07.1997
//
//	 Update 01.09.1997/Marko	Changed to use NFmiStringList to store statusStrings.
//			28.09.1999/Laura	Lis‰sin bool  Find(const unsigned long& status):n, etsit‰‰n staringi‰ statuksen avulla
// ____________________________________________________________________________
// 
// CLASS:	NFmiIndexMessageList
// 
// SYNOPSIS:	#include "nindmesl.h" 
// 
// USED BY:	C++ -programs
// 
// GENERAL DESCRIPTION:
// 
// Holds messages with indexies. Reads given file with messages and their
// indexies. Message(index)-method returns in NFmiString-instance.
// 
// INHERETANCE:	Derivates from - 
//
// NOTES:	xxx
// 
// SEE ALSO: 
// ____________________________________________________________________________
//
// ____________________________________________________________________________
//--------------------------------------------------------------------------*/

#pragma once

#include "NFmiStringList.h"

class NFmiStatusString;

class NFmiIndexMessageList
{
 public:

//   1. Constructors/Destructors

		NFmiIndexMessageList(void);

		~NFmiIndexMessageList(void);

//   2. Methods/Member functions
 
		void Add(const NFmiStatusString& theStatusString);
		NFmiString Message(long theMessageIndex);

		bool  Reset(void){return itsStringList.Reset();};
		bool  Next(void){return itsStringList.Next();};
		bool  Find(const NFmiString& string);
		bool  Find(const unsigned long& status);
		NFmiString  Current(void) const{return *(itsStringList.Current());};
		long CurrentStringStatus(void); // use after after Find(str)

		std::ostream &Write(std::ostream &file) const;
		std::istream &Read(std::istream &file);

//   3. Operators

 private:

//   1. Constructors/Destructors
//   2. Methods/Member functions
//   3. Operators
//   4. Data
	 NFmiStringList itsStringList;
};

//@{ \name Globaalit NFmiIndexMessageList-luokan uudelleenohjaus-operaatiot
inline std::ostream& operator<<(std::ostream& os, const NFmiIndexMessageList& item){return item.Write(os);}
inline std::istream& operator>>(std::istream& is, NFmiIndexMessageList& item){return item.Read(is);}
//@}

//-----------------
// inline functions
//-----------------

