
#pragma once

#include "NFmiSymbol.h"


//_________________________________________________________ NFmiFlash
class NFmiFlash : public NFmiSymbol
{
 public:

   NFmiFlash(const NFmiRect &theRect
            ,NFmiToolBox *theToolBox
            ,NFmiView *theEnclosure = 0
            ,NFmiDrawingEnvironment *theEnvironment = 0
            ,ShapeIdentifier theIdent = kShapeList); 

    virtual void Build(void); 
 private:
};

typedef NFmiFlash* PNFmiFlash;

