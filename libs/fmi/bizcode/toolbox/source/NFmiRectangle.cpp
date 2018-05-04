//© Ilmatieteenlaitos/Persa.
//16.9.1995   Persa
//
//Ver. 16.9.1995
//Ver. 25.10.1999 /Marko Lisäsin konstruktorin.

#include "NFmiRectangle.h"


NFmiRectangle::NFmiRectangle (const NFmiPoint& theTopLeftCorner
                             ,const NFmiPoint& theBottomRightCorner
                             ,const NFmiView *theEnclosure
                             ,NFmiDrawingEnvironment *theEnvironment) 
:NFmiShape(NFmiRect(theTopLeftCorner, theBottomRightCorner)
,theEnclosure
,theEnvironment
,kRectangleShape)
{
}
               
NFmiRectangle::NFmiRectangle (const NFmiRect& theRect
                             ,const NFmiView *theEnclosure
                             ,NFmiDrawingEnvironment *theEnvironment) 
:NFmiShape(theRect, theEnclosure, theEnvironment, kRectangleShape)
{
}
