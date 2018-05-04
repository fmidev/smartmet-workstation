//© Ilmatieteenlaitos/Persa.
//4.10.1995   Persa
//
//Ver. 4.10.1995
//Ver. 25.10.1999 /Marko Lisäsin konstruktorin.
//----------------------------------------------------------------------npolylin.cpp

#include "NFmiPolyline.h"
#include "NFmiVoidPtrList.h"
#include "NFmiDrawingEnvironment.h"

NFmiPolyline::NFmiPolyline (const NFmiPoint& theTopLeftCorner
                           ,const NFmiPoint& theBottomRightCorner
                           ,const NFmiView *theEnclosure
                           ,NFmiDrawingEnvironment *theEnvironment
						   ,int theBackGroundMode
						   ,int theHatchMode) 
:NFmiShape(NFmiRect(theTopLeftCorner, theBottomRightCorner)
			,theEnclosure
			,theEnvironment
			,kPolylineShape)
,itsPoints(new NFmiVoidPtrList)
,itsBackGroundMode(theBackGroundMode)
,itsHatchMode(theHatchMode)
{
	if(theHatchMode == -1 && theEnvironment) // jos annettu defaultti arvo, katsotaan jos hatch-patternin saisi environmentilta
		itsHatchMode = theEnvironment->GetHatchPattern();
}

NFmiPolyline::NFmiPolyline(const NFmiRect& theRect
						   ,const NFmiView *theEnclosure
						   ,NFmiDrawingEnvironment *theEnvironment
						   ,int theBackGroundMode
						   ,int theHatchMode) 
:NFmiShape(theRect
			,theEnclosure
			,theEnvironment
			,kPolylineShape)
,itsPoints(new NFmiVoidPtrList)
,itsBackGroundMode(theBackGroundMode)
,itsHatchMode(theHatchMode)
{
	if(theHatchMode == -1 && theEnvironment) // jos annettu defaultti arvo, katsotaan jos hatch-patternin saisi environmentilta
		itsHatchMode = theEnvironment->GetHatchPattern();
}

//---------------------------------------------------------------------- AddPoint
NFmiPolyline::~NFmiPolyline (void)
{
	NFmiVoidPtrIterator kissa(itsPoints);
	void *vPtr;
	while(kissa.Next(vPtr))
	{
		delete (static_cast<NFmiPoint *>(vPtr));
	}
	delete itsPoints;
}
//---------------------------------------------------------------------- AddPoint
void NFmiPolyline::AddPoint (const NFmiPoint &newPoint)
{
   itsPoints->Add(static_cast<void *>(new NFmiPoint(newPoint)));
}
