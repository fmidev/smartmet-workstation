
#pragma once

#include "NFmiShape.h"


//_________________________________________________________ TFmiLine
class NFmiLine : public NFmiShape
{
 public:

     NFmiLine(const NFmiPoint& theStartingPoint
             ,const NFmiPoint& theEndingPoint
             ,const NFmiView *theEnclosure = 0
             ,NFmiDrawingEnvironment *theEnvironment = 0); 


      NFmiPoint  GetStartingPoint (void) const;
      NFmiPoint  GetEndingPoint (void) const;

 private:
   FmiDirection itsStartingCorner;
};

typedef NFmiLine* PNFmiLine;


