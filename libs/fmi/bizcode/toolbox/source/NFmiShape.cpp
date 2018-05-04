//© Ilmatieteenlaitos/Persa.
//4. 4.1995   Persa
//
//Ver. 5. 4.1995


#include "NFmiShape.h"
 

//_________________________________________________________ NFmiShape
NFmiShape::NFmiShape (const NFmiRect &theRect
                     ,const NFmiView *theEnclosure
                     ,NFmiDrawingEnvironment *theEnvironment
                     ,ShapeIdentifier theIdent)
          : NFmiView(theRect, 0, theEnclosure, theEnvironment, theIdent)
{
//  SetIdentifier(theIdent);
}               

NFmiShape& NFmiShape::operator=(const NFmiShape& theShape)
{
	NFmiView::operator=(theShape);
	itsIdenfier = theShape.itsIdenfier;

	return *this;
}
