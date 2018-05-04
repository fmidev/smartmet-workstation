
#pragma once

#include "NFmiShape.h"

//_________________________________________________________ TFmiShapeList
class NFmiShapeList : public NFmiShape
{
 public:

   NFmiShapeList(const NFmiRect &theRect
                ,NFmiView *theEnclosure = 0
                ,NFmiDrawingEnvironment *theEnvironment = 0
                ,ShapeIdentifier theIdent = kShapeList); 

   virtual ~NFmiShapeList(void);
    NFmiVoidPtrList  *GetShapes (void) const {return itsShapes;};
                 void AddShape(NFmiShape* newShape); 

     virtual void SwapX(void); 
     virtual void SwapY(void); 
 private:
   NFmiVoidPtrList *itsShapes;
};

typedef NFmiShapeList* PNFmiShapeList;

