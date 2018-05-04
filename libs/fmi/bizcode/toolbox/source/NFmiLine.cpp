//© Ilmatieteenlaitos/Persa.
//3.4.1995   Persa
//
//Ver. 3.4.1995



#include "NFmiLine.h"
 

//_________________________________________________________ NFmiLine
NFmiLine::NFmiLine (const NFmiPoint& theStartingPoint
                   ,const NFmiPoint& theEndingPoint
                   ,const NFmiView *theEnclosure
                   ,NFmiDrawingEnvironment *theEnvironment)
          : NFmiShape(NFmiRect(theStartingPoint, theEndingPoint)
                     ,theEnclosure
                     ,theEnvironment
                     ,kLineShape)
          , itsStartingCorner(theStartingPoint.DirectionOfDifference(theEndingPoint))
{
}               
//_________________________________________________________ GetStartingPoint
NFmiPoint NFmiLine::GetStartingPoint (void) const
{
  return GetFrame().Corner(itsStartingCorner);
}               
//_________________________________________________________ GetEndingPoint
NFmiPoint NFmiLine::GetEndingPoint (void) const
{
  int leftRightPart = itsStartingCorner & 3;
  int topBottomPart = itsStartingCorner & 12;		//- leftRightPart;
  leftRightPart = kLeft + kRight - leftRightPart;
  topBottomPart = itsStartingCorner >= kTop ? kBottom + kTop - topBottomPart :
                                              kDown + kUp - topBottomPart;
  return GetFrame().Corner(FmiDirection(leftRightPart | topBottomPart));
}               
