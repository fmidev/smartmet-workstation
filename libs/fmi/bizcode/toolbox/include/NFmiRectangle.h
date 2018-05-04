//Ver. 25.10.1999 /Marko Lisäsin konstruktorin.

#pragma once

#include "NFmiShape.h"


//_________________________________________________________ NFmiRectangle
class NFmiRectangle : public NFmiShape
{
 public:

     NFmiRectangle(const NFmiPoint& theStartingPoint
             ,const NFmiPoint& theEndingPoint
             ,const NFmiView *theEnclosure = 0
             ,NFmiDrawingEnvironment *theEnvironment = 0); 

     NFmiRectangle(const NFmiRect& theRect
             ,const NFmiView *theEnclosure = 0
             ,NFmiDrawingEnvironment *theEnvironment = 0); 

 private:
};

typedef NFmiRectangle* PNFmiRectangle;


