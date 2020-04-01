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
,itsPoints()
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
,itsPoints()
,itsBackGroundMode(theBackGroundMode)
,itsHatchMode(theHatchMode)
{
	if(theHatchMode == -1 && theEnvironment) // jos annettu defaultti arvo, katsotaan jos hatch-patternin saisi environmentilta
		itsHatchMode = theEnvironment->GetHatchPattern();
}

NFmiPolyline::~NFmiPolyline() = default;

//---------------------------------------------------------------------- AddPoint
void NFmiPolyline::AddPoint (const NFmiPoint &newPoint)
{
   itsPoints.push_back(newPoint);
}
