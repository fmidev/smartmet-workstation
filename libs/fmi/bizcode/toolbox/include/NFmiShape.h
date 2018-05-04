
#pragma once

#include "NFmiView.h"


class NFmiDrawingEnvironment;
class NFmiRect;
//_________________________________________________________ NFmiShape
class NFmiShape : public NFmiView
{
 public:

                              NFmiShape(const NFmiRect &theRect
                                       ,const NFmiView *theEnclosure = 0
                                       ,NFmiDrawingEnvironment *theEnvironment = 0
                                       ,ShapeIdentifier theIdent = kShape);
                     virtual ~NFmiShape(void) {};
/*
              NFmiPoint &GlobalPlace(const float x,
                                     const float y,
                                     NFmiPoint &thePoint) const;
              NFmiPoint &GlobalSize(const float x,
                                    const float y,
                                    NFmiPoint &thePoint) const;
*/
            virtual void SwapX(void) {}; 
            virtual void SwapY(void) {}; 

			NFmiShape& operator=(const NFmiShape& theShape);

 private:

   ShapeIdentifier itsIdenfier;

};

typedef NFmiShape* PNFmiShape;


