//**********************************************************
// C++ Class Name : NFmiValueLineList 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiValueLineList.cpp 
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
#include "NFmiValueLineList.h"
#include "NFmiValueLine.h"
#include "NFmiPolyline.h"
#include <fstream>

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiValueLineList::NFmiValueLineList(void)
:itsLineList()
,itsIter()
,itsPolylineList()
,itsPolylineIter()
{
}
NFmiValueLineList::NFmiValueLineList(NFmiValueLineList& theList)
:itsLineList(theList.itsLineList)
,itsIter() // iteraattoria ei voi kopioida!!!
,itsPolylineList()
,itsPolylineIter()
{
}
NFmiValueLineList::~NFmiValueLineList(void)
{Clear();}

//--------------------------------------------------------
// Add 
//--------------------------------------------------------
bool NFmiValueLineList::Add(NFmiValueLine* theLine)
{return itsLineList.AddEnd(theLine);}
//--------------------------------------------------------
// Clear 
//--------------------------------------------------------
void NFmiValueLineList::Clear(bool fDeleteLines)
{
	itsLineList.Clear(fDeleteLines);
	itsPolylineList.Clear(fDeleteLines);
}
//--------------------------------------------------------
// LineList 
//--------------------------------------------------------

//   Palauttaa listan, miss‰ on kaikki sellaiset 
//   viivat joiden arvo on sama kuin parametrin.
//   Jos fCopyLines on true, tekee new:ll‰ kopiot alkuper‰isist‰, muuten kopioidaan vain osoitin
NFmiPtrList<NFmiValueLine> NFmiValueLineList::LineList(double theValue, bool fCopyLines)
{
   NFmiPtrList<NFmiValueLine> returnVal;
   return returnVal;
}

bool NFmiValueLineList::Reset(void)
{
	itsIter = itsLineList.Start();
	return true;
}

bool NFmiValueLineList::Next(void)
{return itsIter.Next();}

bool NFmiValueLineList::Previous(void)
{return itsIter.Previous();}

NFmiValueLine* NFmiValueLineList::Current(void)
{return itsIter.CurrentPtr();}

bool NFmiValueLineList::ReadData(const NFmiString& theFileName) //Mikael lis‰si 27.9.99
{
	int number;
	char diff;
	double lon1, lat1, lon2, lat2;
	bool found=false;

	std::ifstream in(theFileName, std::ios::binary);
	Clear();
	if(in && in.peek() != EOF)
	{
		in >> number >> diff >> lon1 >> diff >> lat1;
		found=true;
		NFmiValueLine* valueLine = 0;
		while (in && in.peek() != EOF)
		{
			in >> number >> diff >> lon2 >> diff >> lat2;
			if(number == 1)
			{
			}
			else
			{
				valueLine = new NFmiValueLine(lon1, lat1, lon2, lat2, 0);
				Add(valueLine);
			}
			lon1=lon2;
			lat1=lat2;
		}
	}
	in.close();
	return found;
}

bool NFmiValueLineList::ReadData2(const NFmiString& theFileName, const NFmiRect& theRect)	//Marko lis‰si 25.10.99
{
	int number;
	char diff; // erotin merkin luku
	double lon, lat;
	bool found=false;

	std::ifstream in(theFileName, std::ios::binary);
	Clear(true);
	if(in && in.peek() != EOF)
	{
		found=true;
		NFmiPolyline* polyLine = 0;
		while (in && in.peek() != EOF)
		{
			in >> number >> diff >> lon >> diff >> lat;
			if (number == 1) // polylinen 1. piste
			{
				PolyAdd(polyLine);
				polyLine = new NFmiPolyline(theRect, 0, 0);
				polyLine->AddPoint(NFmiPoint(lon, lat));
			}
			else 
				polyLine->AddPoint(NFmiPoint(lon, lat));
		}
		in.close();
	}
	return found;
}

bool NFmiValueLineList::PolyAdd(NFmiPolyline* theLine)
{return itsPolylineList.AddEnd(theLine);}

bool NFmiValueLineList::PolyReset(void)
{
	itsPolylineIter = itsPolylineList.Start();
	return true;
}

bool NFmiValueLineList::PolyNext(void)
{return itsPolylineIter.Next();}

bool NFmiValueLineList::PolyPrevious(void)
{return itsPolylineIter.Previous();}

NFmiPolyline* NFmiValueLineList::PolyCurrent(void)
{return itsPolylineIter.CurrentPtr();}


/*
NFmiPolyline(const NFmiPoint& theTopLeftCorner
           ,const NFmiPoint& theBottomRightCorner
           ,const NFmiView *theEnclosure = 0
           ,NFmiDrawingEnvironment *theEnvironment = 0); */

	

