//**********************************************************
// C++ Class Name : NFmiValueLineList 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiValueLineList.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : multipolyline 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Fri - Sep 17, 1999 
// 
// 
//  Description: 
//   Lista NFmiValueLine:j‰. Viivat ovat sekaisin 
//   yhdess‰ listassa. Luokalta voidaan
//   pyyt‰‰ arvo listaa ja vastaavaa listaa, miss‰ 
//   on tietyll‰ arvolla olevia viivoja.
//   Listassa olevia viivoja ei saa tuhoa, koska 
//   niiss‰ on kopioitu vain osoitteet.
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiPtrList.h"
#include "NFmiString.h"				//Mikael lis‰si 27.9.99

class NFmiValueLine;
class NFmiPolyline;
class NFmiRect;

class NFmiValueLineList 
{

 public:
	NFmiValueLineList(void);
	NFmiValueLineList(NFmiValueLineList& theList);
	~NFmiValueLineList(void);
	bool Add(NFmiValueLine* theLine);
	bool PolyAdd(NFmiPolyline* theLine);
	void Clear(bool fDeleteLines = false);
	NFmiPtrList<NFmiValueLine> LineList(double theValue, bool fCopyLines = false);
	bool      Reset(void);
	bool      Next(void);
	bool      Previous(void);
	NFmiValueLine*  Current(void);
	bool      PolyReset(void);
	bool      PolyNext(void);
	bool      PolyPrevious(void);
	NFmiPolyline*  PolyCurrent(void);
	bool ReadData(const NFmiString& theFileName);	//Mikael lis‰si 27.9.99
	bool ReadData2(const NFmiString& theFileName, const NFmiRect& theRect);	//Marko lis‰si 25.10.99

 private:
	NFmiPtrList<NFmiValueLine> itsLineList;
	NFmiPtrList<NFmiValueLine>::Iterator itsIter;

	NFmiPtrList<NFmiPolyline> itsPolylineList;
	NFmiPtrList<NFmiPolyline>::Iterator itsPolylineIter;

};
